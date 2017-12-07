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
        writes_ = 0;
        write_cycles_ = 0;
        reads_ = 0;
        read_cycles_ = 0;
        cursor_ = 0;
        pending_commands_ = 0;
        qpair_ = 0;
    };

    ~nvme_blk_accessor() {
        delete qpair_;
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
        open_time_ = ticks();
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
            print_metrics();
            closed_ = true;
        }
    }
    virtual int read(const blk_address & blk_addr, void* buffer) {
        uint64_t start = ticks();
        spin_lock_.acquire();
        qpair_->synchronous_read(buffer, this->block_size, blk_addr);
        spin_lock_.release();
        read_cycles_ += ticks() - start;
        reads_++;
    }
    virtual int write(const blk_address & blk_addr, void* buffer) {
        uint64_t start = ticks();
        spin_lock_.acquire();
        qpair_->synchronous_write(buffer, this->block_size, blk_addr);
        spin_lock_.release();
        write_cycles_ += ticks() - start;
        writes_++;
    }

    virtual void* malloc_buffer() const {
        return spdk_dma_zmalloc(this->block_size, this->block_size, NULL);
    }
    virtual void free_buffer(void* buffer) const {
        spdk_dma_free(buffer);
    }

    void flush() {
    }

    virtual void asynch_read(const blk_address& blk_addr, void* buffer, call_back_context* context) {
        assert((uint64_t)buffer % 512 == 0);
        nvme_callback_para* para = new nvme_callback_para;
        para->start_time = ticks();
        para->type = NVM_READ;
        para->context = context;
        para->finished_context = &finished_contexts_;
        pending_commands_ ++;
        para->pending_command = &pending_commands_;
        para->id = blk_addr;
        para->accessor = this;
        int status = qpair_->submit_read_operation(buffer, this->block_size, blk_addr, context_call_back_function, para);
        if (status != 0) {
            printf("error in submitting read command\n");
            printf("blk_addr: %ld, block_size: %d\n", blk_addr, this->block_size);
            pending_commands_ --;
            return;
        }
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

    int process_completion(int max = 1) {
        return process_completion(qpair_, max);
    }

    virtual void asynch_write(const blk_address& blk_addr, void* buffer, call_back_context* context) {
        assert((uint64_t)buffer % 512 == 0);
        nvme_callback_para* para = new nvme_callback_para;
        para->start_time = ticks();
        para->type = NVM_WRITE;
        para->context = context;
        para->finished_context = &finished_contexts_;
        pending_commands_ ++;
        para->pending_command = &pending_commands_;
        para->id = blk_addr;
        para->accessor = this;
//        printf("%s to submit asynch write on %lld with address %llx\n", context->get_name(), blk_addr, buffer);
//        printf("pending ios: %s\n", pending_ios_to_string(&pending_io_).c_str());
        int status = qpair_->submit_write_operation(buffer, this->block_size, blk_addr, context_call_back_function, para);
        if (status != 0) {
            printf("error in submitting read command\n");
            printf("blk_addr: %ld, block_size: %d\n", blk_addr, this->block_size);
            pending_commands_ --;
            return;
        }
#ifdef __NVME_ACCESSOR_LOG__
        printf("pending_commands_ added to %d.\n", pending_commands_);
#endif
    }

    static void context_call_back_function(void* parms, const struct spdk_nvme_cpl *) {
        nvme_callback_para* para = reinterpret_cast<nvme_callback_para*>(parms);
        *para->pending_command -= 1;
        if (para->type == NVM_READ) {
            para->accessor->read_cycles_ += ticks() - para->start_time;
            para->accessor->reads_.fetch_add(1);
        } else {
            para->accessor->write_cycles_ += ticks() - para->start_time;
            para->accessor->writes_.fetch_add(1);
        }
        para->context->transition_to_next_state();
//        printf("%d(%s) is completed!\n", para->id, para->type.c_str());
//        printf("pending ios: %s\n", pending_ios_to_string(para->pending_io_).c_str());
//        printf("to process context[%s]\n", para->context->get_name());
        if (para->context->run() == CONTEXT_TERMINATED) {
//            *para->finished_context += 1;
            delete para->context;
            para->context = 0;
        }
        delete para;
    }

    struct nvme_callback_para {
        call_back_context* context;
        volatile int32_t* finished_context;
        volatile int32_t* pending_command;
        int64_t id;
        int64_t start_time;
        nvme_blk_accessor* accessor;
        int32_t type;
    };

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
        if (pending_commands_ < 0) {
            sleep(1);
        }
#endif
        int processed = finished_contexts_;
        finished_contexts_ = 0;
        return processed;
    }

protected:
    void print_metrics(const char* name = "NVM") {
        double duration_in_seconds = cycles_to_seconds(ticks() - open_time_);
        if (reads_ > 0)
            printf("[%s:] total reads: %ld, average: %.2f us, IOPS: %.0f\n", name, reads_.load(),
                   cycles_to_microseconds(read_cycles_.load() / reads_.load()), reads_.load() / duration_in_seconds);
        if (writes_ > 0)
            printf("[%s:] total writes: %ld, average: %.2f us, IOPS %.0f\n", name, writes_.load(),
                   cycles_to_microseconds(write_cycles_.load() / writes_.load()), writes_.load() / duration_in_seconds);
        if (writes_ || reads_)
            printf("[%s:] total IO: %ld, average: %.2f us, IOPS %.0f\n", name, reads_.load() + writes_.load(),
                   cycles_to_microseconds((read_cycles_.load() + write_cycles_.load()) / (reads_.load() + writes_.load())),
                   (reads_.load() + writes_.load()) / duration_in_seconds);
    }

protected:

    atomic<uint64_t> read_cycles_;
    atomic<uint64_t> write_cycles_;
    atomic<uint64_t> reads_;
    atomic<uint64_t> writes_;
    QPair* qpair_;
    int64_t open_time_;
    bool closed_;
private:
    std::unordered_set<blk_address> freed_blk_addresses_;
    uint64_t cursor_;
    volatile int32_t finished_contexts_;
    volatile int32_t pending_commands_;
    unordered_map<int64_t, string> pending_io_;
    SpinLock spin_lock_;
};

#endif //NVM_NVME_BLK_ACCESSOR_H
