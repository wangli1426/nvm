//
// Created by robert on 7/11/17.
//

#ifndef NVM_NVME_BLK_ACCESSOR_H
#define NVM_NVME_BLK_ACCESSOR_H

#include <unordered_set>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <atomic>
#include "blk.h"
#include "blk_cache.h"
#include "../tree/blk_node_reference.h"
#include "../accessor/ns_entry.h"
#include "../accessor/qpair_context.h"
#include "../utils/rdtsc.h"
#include "../utils/sync.h"
#include "../context/call_back.h"
#include "asynchronous_accessor.h"

using namespace std;

namespace tree {
    template<typename K, typename V, int CAPACITY>
    class blk_node_reference;
}

using namespace nvm;

//#define __NVME_ACCESSOR_LOG__

#define NVM_READ 0
#define NVM_WRITE 1

template<typename K, typename V, int CAPACITY>
class nvme_blk_accessor: public blk_accessor<K, V> {
public:
    nvme_blk_accessor(const int& block_size): blk_accessor<K, V>(block_size) {
        cursor_ = 0;
        qpair_ = 0;
        cache_ = 0;
//        cache_ = new blk_cache(this->block_size, 10000);

        // measure the concurrency in the command queues
    };

    ~nvme_blk_accessor() {
        delete qpair_;
        if (cache_) {
            cache_->print();
        }
        delete cache_;
    }

    node_reference<K, V>* allocate_ref() override {
        blk_address addr = allocate();
        return new blk_node_reference<K, V, CAPACITY>(addr);
    };

    node_reference<K, V>* create_null_ref() override {
        return new blk_node_reference<K, V, CAPACITY>(-1);
    };

    virtual int open() {
        int status = nvm::nvm_utility::initialize_namespace();
        if (status != 0) {
            cout << "Errors in initialization of namespace." << endl;
            exit(1);
        } else {
            cout << "namespace is initialized." << endl;
        }
        qpair_ = nvm_utility::allocateQPair(1);
        closed_ = false;
    }

    virtual blk_address allocate() {
        if (!freed_blk_addresses_.empty()) {
            auto it = freed_blk_addresses_.begin();
            blk_address blk_addr = *it;
            freed_blk_addresses_.erase(it);
            return blk_addr;
        } else {
            return blk_address(cursor_++);
        }
    }
    virtual void deallocate(const blk_address& address) {
        if (cursor_ == address - 1)
            cursor_ --;
        else {
            freed_blk_addresses_.insert(address);
        }
    }
    virtual int close() {
        if (!closed_) {
            qpair_->free_qpair();
            closed_ = true;
        }
    }
    virtual int read(const blk_address & blk_addr, void* buffer) {
        uint64_t start = ticks();
        spin_lock_.acquire();
        if (cache_ && cache_->read(blk_addr, buffer)) {
        } else {
            qpair_->synchronous_read(buffer, this->block_size, blk_addr);
            if (cache_) {
                blk_cache::cache_unit unit;
                if (cache_->write(blk_addr, buffer, false, unit)) {
                    if (unit.dirty)
                        qpair_->synchronous_write(unit.data, this->block_size, unit.id);
                    free(unit.data);
                }
            }
        }
        spin_lock_.release();
        this->metrics_.read_cycles_ += ticks() - start;
        this->metrics_.reads_++;
    }
    virtual int write(const blk_address & blk_addr, void* buffer) {
        uint64_t start = ticks();
        spin_lock_.acquire();
//        if (cache_) {
//            blk_cache::cache_unit evit_unit;
//            if (cache_->write(blk_addr, buffer, true, evit_unit)) {
//                if (evit_unit.dirty)
//                    qpair_->synchronous_write(evit_unit.data, this->block_size, evit_unit.id);
//                free(evit_unit.data);
//            }
//        } else {
            qpair_->synchronous_write(buffer, this->block_size, blk_addr);
//        }
        spin_lock_.release();
        this->metrics_.write_cycles_ += ticks() - start;
        this->metrics_.writes_++;
        return this->block_size;
    }

    void* malloc_buffer() const override {
        return spdk_dma_zmalloc(this->block_size, this->block_size, NULL);
    }
    void free_buffer(void* buffer) const override {
        spdk_dma_free(buffer);
    }

    void flush() {
    }

    virtual void asynch_read(const blk_address& blk_addr, void* buffer, call_back_context* context) {
        int64_t start = ticks();
        if (cache_ && cache_->read(blk_addr, buffer)) {
            // we read the data from in-memory cache, so asynchronous io will be omitted.
            // As such, we just forward the context to the ready context queue.
            context->transition_to_next_state();
            ready_contexts_.push(context);
            this->metrics_.add_read_latency(ticks() - start);
            return;
        }

        nvme_callback_para* para = new nvme_callback_para;
        para->start_time = start;
        para->type = NVM_READ;
        para->context = context;
        para->id = blk_addr;
        para->accessor = this;
        para->buffer = buffer;
        int status = qpair_->submit_read_operation(buffer, this->block_size, blk_addr, context_call_back_function, para);
        if (status != 0) {
            printf("error in submitting read command\n");
            printf("blk_addr: %ld, block_size: %d\n", blk_addr, this->block_size);
            return;
        }
        this->metrics_.pending_commands_ ++;
#ifdef __NVME_ACCESSOR_LOG__
        printf("pending_commands_ added to %d.\n", pending_commands_);
#endif
    }

    static string pending_ios_to_string(unordered_map<int64_t, string> *pending_io_) {
        ostringstream ost;
        for (auto it = pending_io_->begin(); it != pending_io_->end(); ++it) {
            ost << it->first << "(" << it->second << ")" << " ";
        }
        return ost.str();
    }


    std::queue<call_back_context*>& get_ready_contexts() override {
        return ready_contexts_;
    }

    int process_completion(int max = 1) {
        int processed =  process_completion(qpair_, max);
        this->metrics_.pending_commands_ -= processed;
        this->metrics_.pending_command_counts_.push_back(this->metrics_.pending_commands_);
        return processed;
    }

    virtual void asynch_write(const blk_address& blk_addr, void* buffer, call_back_context* context) {
        nvme_callback_para* para = new nvme_callback_para;
        para->start_time = ticks();
        para->type = NVM_WRITE;
        para->context = context;
        para->id = blk_addr;
        para->accessor = this;
        para->buffer = buffer;
//        printf("%s to submit asynch write on %lld with address %llx\n", context->get_name(), blk_addr, buffer);
//        printf("pending ios: %s\n", pending_ios_to_string(&pending_io_).c_str());
        int status = qpair_->submit_write_operation(buffer, this->block_size, blk_addr, context_call_back_function, para);
        if (status != 0) {
            printf("error in submitting read command\n");
            printf("blk_addr: %ld, block_size: %d\n", blk_addr, this->block_size);
            return;
        }
        this->metrics_.pending_commands_ ++;
#ifdef __NVME_ACCESSOR_LOG__
        printf("pending_commands_ added to %d.\n", pending_commands_);
#endif
    }

    static void context_call_back_function(void* parms, const struct spdk_nvme_cpl *) {
        nvme_callback_para* para = reinterpret_cast<nvme_callback_para*>(parms);
        if (para->type == NVM_READ) {
//            para->accessor->metrics_.read_cycles_ += ticks() - para->start_time;
//            para->accessor->metrics_.reads_.fetch_add(1);
            para->accessor->metrics_.add_read_latency(ticks() - para->start_time);
        } else {
//            para->accessor->metrics_.write_cycles_ += ticks() - para->start_time;
//            para->accessor->metrics_.writes_.fetch_add(1);
            para->accessor->metrics_.add_write_latency(ticks() - para->start_time);
        }

        if (para->accessor->cache_) {
            blk_cache::cache_unit unit;
            bool evicted = para->accessor->cache_->write(para->id, para->buffer, false, unit);
            if (evicted) {
                assert(!unit.dirty);
                free(unit.data);
            }
        }
//        para->context->transition_to_next_state();
//        printf("%d(%s) is completed!\n", para->id, para->type.c_str());
//        printf("pending ios: %s\n", pending_ios_to_string(para->pending_io_).c_str());
//        printf("to process context[%s]\n", para->context->get_name());
//        if (para->context->run() == CONTEXT_TERMINATED) {
//            delete para->context;
//            para->context = 0;
//        }

        para->context->transition_to_next_state();
        para->accessor->ready_contexts_.push(para->context);

        delete para;
    }

    int32_t process_ready_contexts(int32_t max = 1) {
        int32_t processed = 0;
        for (int i = 0; i < ready_contexts_.size(); i++) {
            call_back_context* context = ready_contexts_.front();
            ready_contexts_.pop();
            context->run();
            processed++;
        }
        return processed;
    }

    std::queue<call_back_context*>& get_ready_context_queue() override {
        return ready_contexts_;
    }

    struct nvme_callback_para {
        call_back_context* context;
        int64_t id;
        int64_t start_time;
        nvme_blk_accessor* accessor;
        int32_t type;
        void* buffer;
    };
    std::string get_name() const {
        return std::string("NVM");
    }

protected:
    int process_completion(QPair* qpair, int max = 1) {
//        printf("process_completion is called!\n");
        int32_t status = qpair->process_completions(max);
        if (status < 0) {
            printf("errors in process_completions!\n");
            return status;
        }
#ifdef __NVME_ACCESSOR_LOG__
        printf("%d commands left.\n", pending_commands_);
//        if (pending_commands_ < 0) {
//            sleep(1);
//        }
#endif
        return status;
    }

    QPair* qpair_;
    bool closed_;
private:
    std::unordered_set<blk_address> freed_blk_addresses_;
    uint64_t cursor_;
    unordered_map<int64_t, string> pending_io_;
    SpinLock spin_lock_;
    std::queue<call_back_context*> ready_contexts_;
    blk_cache *cache_;
};

#endif //NVM_NVME_BLK_ACCESSOR_H
