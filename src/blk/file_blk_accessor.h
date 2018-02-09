//
// Created by robert on 24/10/17.
//

#ifndef NVM_FILE_BLK_ACCESSOR_H
#define NVM_FILE_BLK_ACCESSOR_H
#include <deque>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <unordered_set>
#include <queue>
#include "../tree/blk_node_reference.h"
#include "blk.h"
#include "../utils/rdtsc.h"
#include "asynchronous_accessor.h"
#include "blk_cache.h"
#include "../utils/sync.h"
#include "../scheduler/ready_state_estimator.h"

namespace tree {
    template<typename K, typename V, int CAPACITY>
    class blk_node_reference;
}

using namespace std;
template<typename K, typename V, int CAPACITY>
class file_blk_accessor: public blk_accessor<K, V>{
public:
    explicit file_blk_accessor(const char* path, const uint32_t& block_size) : path_(path), blk_accessor<K, V>(block_size),
                                                                               cursor_(0), wait_for_completion_counts_(0),
                                                                               estimator_(1000, 2000), io_id_generator_(0) {
        cache_ = new blk_cache(block_size, 10000);
//        cache_ = nullptr;
    }

    ~file_blk_accessor() {
        if (cache_) {
            cache_->print();
        }
        delete cache_;
        cache_ = 0;
    }

    int open() override{
#ifdef __APPLE__
        fd_ = ::open(path_, O_CREAT|O_TRUNC|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
//        fd_ = ::open(path_, O_CREAT|O_TRUNC|O_RDWR|F_NOCACHE, S_IRWXU|S_IRWXG|S_IRWXO);
#else
        fd_ = ::open(path_, O_CREAT|O_TRUNC|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
//        fd_ = ::open(path_, O_CREAT|O_TRUNC|O_RDWR|O_DIRECT, S_IRWXU|S_IRWXG|S_IRWXO);
#endif
//        fd_ = ::open(path_, O_CREAT|O_TRUNC|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
        return fd_ ? 0 : errno;
    }

    blk_address allocate() override {
        if (!freed_blk_addresses_.empty()) {
            auto it = freed_blk_addresses_.begin();
            blk_address blk_addr = *it;
            freed_blk_addresses_.erase(it);
            return blk_addr;
        } else {
            return blk_address(cursor_++);
        }
    }

    void deallocate(const blk_address& address) override {
        if (cursor_ == address - 1)
            cursor_ --;
        else {
            freed_blk_addresses_.insert(address);
        }
    }

    int read(const blk_address& address, void *buffer) override {
        uint64_t start = ticks();
        if (!is_address_valid(address))
            return 0;
        lock_.acquire();
        if (cache_ && cache_->read(address, buffer)) {
            lock_.release();
            this->metrics_.read_cycles_ += ticks() - start;
            this->metrics_.reads_++;
            return this->block_size;
        }
        lock_.release();

        int status = (int)::pread(fd_, buffer, this->block_size, address * this->block_size);
        if (status < 0) {
            printf("read error: %s\n", strerror(errno));
            assert(false);
        }
        this->metrics_.read_cycles_ += ticks() - start;
        this->metrics_.reads_++;

        if (cache_) {
            blk_cache::cache_unit unit;
            lock_.acquire();
            bool evicted = cache_->write(address, buffer, false, unit);
            lock_.release();
            if (evicted) {
                int write_status = ::pwrite(fd_, unit.data, this->block_size, unit.id * this->block_size);
                if (write_status < 0) {
                    printf("write error: %s\n", strerror(errno));
                }
                free(unit.data);
            }
        }

        return status;
    }

    int write(const blk_address& address, void *buffer) override {
        uint64_t start = ticks();
        if (!is_address_valid(address))
            return 0;
        if (cache_) {
            blk_cache::cache_unit unit;
            lock_.acquire();
            if (cache_->write(address, buffer, true, unit)) {
                lock_.release();
                if (unit.dirty) {
                    int write_status = ::pwrite(fd_, unit.data, this->block_size, unit.id * this->block_size);
                    if (write_status < 0) {
                        printf("error in write: %s\n", strerror(errno));
                    }
                }
                free(unit.data);
                unit.data = 0;
            } else {
                lock_.release();
            }
            return this->block_size;
        }

        int status = (int)::pwrite(fd_, buffer, this->block_size, address * this->block_size);
        if (status < 0) {
            printf("write error: %s\n", strerror(errno));
        }
        this->metrics_.write_cycles_ += ticks() - start;
        this->metrics_.writes_++;
        return status;
    }

    int close() override {
        int fd = fd_;
        fd_ = -1;
        return ::close(fd);
    }

    node_reference<K, V>* allocate_ref() override {
        blk_address addr = allocate();
        return new blk_node_reference<K, V, CAPACITY>(addr);
    }

    node_reference<K, V>* create_null_ref() override {
        return new blk_node_reference<K, V, CAPACITY>(-1);
    };

    void flush() override {
//        fsync(fd_);
    }

    void asynch_read(const blk_address& blk_addr, void* buffer, call_back_context* context) override {
        read(blk_addr, buffer);
        const uint64_t id = io_id_generator_++;
        estimator_.register_read_io(id, ticks());
        pending_io io;
        io.type = pending_io::read_io;
        io.id = id;
        pending_ids_.push_back(io);
        pending_contexts_.push_back(context);
        wait_for_completion_counts_++;
    }

    void asynch_write(const blk_address& blk_addr, void* buffer, call_back_context* context) override {
        write(blk_addr, buffer);
        const uint64_t id = io_id_generator_++;
        estimator_.register_write_io(id, ticks());
        pending_io io;
        io.type = pending_io::write_io;
        io.id = id;
        pending_ids_.push_back(io);
        pending_contexts_.push_back(context);
        wait_for_completion_counts_++;
    }

    std::deque<call_back_context*>& get_ready_contexts() override {
        return ready_contexts_;
    }

    int32_t process_ready_contexts(int32_t max = 1) override {
        int processed = 0;
        for(; processed < ready_contexts_.size() && processed < max; processed++) {
            call_back_context* context = ready_contexts_.front();
            ready_contexts_.pop_front();
            if (context->run() == CONTEXT_TERMINATED) {
//                delete context;
            }
        }
        return processed;
    }

    int process_completion(int max = 1) override {
//        int processed = 0;
//        for (int i = 0; i < max; i++) {
//            if (ready_contexts_.size() > 0) {
//                call_back_context* callback = ready_contexts_.front();
//                ready_contexts_.pop();
//                callback->transition_to_next_state();
////                printf("[blk:] before\n");
////                if (callback->run() == CONTEXT_TERMINATED) {
////                    processed++;
////                    delete callback;
////                }
////                printf("[blk:] after\n");
//
//                ready_contexts_.push(callback);
//            }
//        }
//        return processed;
//        int ret = wait_for_completion_counts_ < max ? wait_for_completion_counts_: max;
//        wait_for_completion_counts_ -= ret;
//        return ret;
        int processed = 0;
        random_shuffle(pending_contexts_.begin(), pending_contexts_.end());
        while (processed < max && pending_contexts_.size() > 0) {
            call_back_context* context = pending_contexts_.back();
            pending_io io = pending_ids_.back();
            pending_ids_.pop_back();
            if (io.type == pending_io::read_io) {
                estimator_.remove_read_io(io.id);
                context->set_tag(CONTEXT_READ_IO);
            } else {
                estimator_.remove_write_io(io.id);
                context->set_tag(CONTEXT_WRITE_IO);
            }
            pending_contexts_.pop_back();
            context->transition_to_next_state();
            ready_contexts_.push_back(context);
            processed++;
        }
        return processed;
    }

    ready_state_estimator& get_ready_state_estimator() override{
        return estimator_;
    }

    std::string get_name() const override {
        return std::string("Disk");
    }
private:
    bool is_address_valid(const blk_address& address) const {
        return address < cursor_ && freed_blk_addresses_.find(address) == freed_blk_addresses_.cend();
    }

private:

    struct pending_io {
        uint64_t id;
        enum {read_io, write_io} type;
    };

    const char* path_;
    int fd_;
    std::unordered_set<blk_address> freed_blk_addresses_;
    uint32_t cursor_;
    std::vector<call_back_context*> pending_contexts_;
    std::deque<call_back_context*> ready_contexts_;
    blk_cache *cache_;
    uint32_t wait_for_completion_counts_;
    SpinLock lock_;
    ready_state_estimator estimator_;
    uint64_t io_id_generator_;
    std::vector<pending_io> pending_ids_;
};


#endif //NVM_FILE_BLK_ACCESSOR_H
