//
// Created by robert on 24/10/17.
//

#ifndef NVM_FILE_BLK_ACCESSOR_H
#define NVM_FILE_BLK_ACCESSOR_H
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

namespace tree {
    template<typename K, typename V, int CAPACITY>
    class blk_node_reference;
}

using namespace std;
template<typename K, typename V, int CAPACITY>
class file_blk_accessor: public blk_accessor<K, V>{
public:
    explicit file_blk_accessor(const char* path, const uint32_t& block_size) : path_(path), blk_accessor<K, V>(block_size),
                                                                               cursor_(0) {
        writes_ = 0;
        write_cycles_ = 0;
        reads_ = 0;
        read_cycles_ = 0;
//        cache_ = new blk_cache(block_size, 10000);
        cache_ = nullptr;
    }

    ~file_blk_accessor() {
        delete cache_;
    }

    int open() override{
#ifdef __APPLE__
        fd_ = ::open(path_, O_CREAT|O_TRUNC|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
//        fd_ = ::open(path_, O_CREAT|O_TRUNC|O_RDWR|F_NOCACHE, S_IRWXU|S_IRWXG|S_IRWXO);
#else
//        fd_ = ::open(path_, O_CREAT|O_TRUNC|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
        fd_ = ::open(path_, O_CREAT|O_TRUNC|O_RDWR|O_DIRECT, S_IRWXU|S_IRWXG|S_IRWXO);
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

        if (cache_ && cache_->read(address, buffer)) {
            read_cycles_ += ticks() - start;
            reads_++;
            return this->block_size;
        }

        int status = (int)::pread(fd_, buffer, this->block_size, address * this->block_size);
        if (status < 0) {
            printf("read error: %s\n", strerror(errno));
        }
        read_cycles_ += ticks() - start;
        reads_++;

        if (cache_) {
            blk_cache::cache_unit unit;
            bool evicted = cache_->write(address, buffer, unit);
            if (evicted) {
                int write_status = ::pwrite(unit.id, unit.data, this->block_size, address * this->block_size);
                if (write_status) {
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
            if (cache_->write(address, buffer, unit)) {
                int write_status = ::pwrite(unit.id, unit.data, this->block_size, address * this->block_size);
                if (write_status < 0) {
                    printf("error in write: %s\n", strerror(errno));
                }
                free(unit.data);
            }
            return this->block_size;
        }

        int status = (int)::pwrite(fd_, buffer, this->block_size, address * this->block_size);
        if (status < 0) {
            printf("write error: %s\n", strerror(errno));
        }
        write_cycles_ += ticks() - start;
        writes_++;
        return status;
    }

    int close() override {
        if (reads_ > 0)
            printf("[DISK:] total reads: %ld, average: %.2f us\n", reads_, cycles_to_microseconds(read_cycles_ / reads_));
        if (writes_ > 0)
            printf("[DISK:] total writes: %ld, average: %.2f us\n", writes_, cycles_to_microseconds(write_cycles_ / writes_));
        return ::close(fd_);
    }

    node_reference<K, V>* allocate_ref() override {
        blk_address addr = allocate();
        return new blk_node_reference<K, V, CAPACITY>(addr);
    }

    node_reference<K, V>* create_null_ref() {
        return new blk_node_reference<K, V, CAPACITY>(-1);
    };

    void flush() {
//        fsync(fd_);
    }

    void asynch_read(const blk_address& blk_addr, void* buffer, call_back_context* context) {
        read(blk_addr, buffer);
        completed_callbacks_.push(context);
    }

    void asynch_write(const blk_address& blk_addr, void* buffer, call_back_context* context) {
        write(blk_addr, buffer);
        completed_callbacks_.push(context);
    }

    int process_completion(int max = 1) {
        int processed = 0;
        for (int i = 0; i < max; i++) {
            if (completed_callbacks_.size() > 0) {
                call_back_context* callback = completed_callbacks_.front();
                completed_callbacks_.pop();
                callback->transition_to_next_state();
                if (callback->run() == CONTEXT_TERMINATED)
                    processed++;
            }
        }
        return processed;
    }
private:
    bool is_address_valid(const blk_address& address) const {
        return address < cursor_ && freed_blk_addresses_.find(address) == freed_blk_addresses_.cend();
    }

private:
    const char* path_;
    int fd_;
    std::unordered_set<blk_address> freed_blk_addresses_;
    uint32_t cursor_;
    uint64_t read_cycles_;
    uint64_t write_cycles_;
    uint64_t reads_;
    uint64_t writes_;
    std::queue<call_back_context*> completed_callbacks_;
    blk_cache *cache_;
};


#endif //NVM_FILE_BLK_ACCESSOR_H
