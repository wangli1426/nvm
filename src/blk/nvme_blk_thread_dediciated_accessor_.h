//
// Created by robert on 7/11/17.
//

#ifndef NVM_NVME_BLK_THREAD_DEDICATED_ACCESSOR_H
#define NVM_NVME_BLK_THREAD_DEDICATED_ACCESSOR_H

#include <unordered_set>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <pthread.h>
#include "nvme_blk_accessor.h"
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

template<typename K, typename V, int CAPACITY>
class nvme_blk_thread_dedicated_accessor: public nvme_blk_accessor<K, V, CAPACITY> {
public:
    nvme_blk_thread_dedicated_accessor(const int& block_size): nvme_blk_accessor<K, V, CAPACITY>(block_size) {
    };

    ~nvme_blk_thread_dedicated_accessor() {

    }


    virtual int open() {
        int status = nvm::nvm_utility::initialize_namespace();
        if (status != 0) {
            cout << "Errors in initialization of namespace." << endl;
            exit(1);
        } else {
            cout << "namespace is initialized." << endl;
        }
    }

    virtual blk_address allocate() {
        this->spin_lock_.acquire();
        if (!freed_blk_addresses_.empty()) {
            auto it = freed_blk_addresses_.begin();
            blk_address blk_addr = *it;
            freed_blk_addresses_.erase(it);
            this->spin_lock_.release();
            return blk_addr;
        } else {
            this->spin_lock_.release();
            return blk_address(cursor_++);
        }
    }
    virtual void deallocate(const blk_address& address) {
        this->spin_lock_.acquire();
        if (cursor_ == address - 1)
            cursor_ --;
        else {
            freed_blk_addresses_.insert(address);
        }
        this->spin_lock_.release();
    }
    virtual int close() {
        qpair_->free_qpair();

        if (reads_ > 0)
            printf("[NVM:] total reads: %ld, average: %.2f us\n", reads_, cycles_to_microseconds(read_cycles_ / reads_));
        if (writes_ > 0)
            printf("[NVM:] total writes: %ld, average: %.2f us\n", writes_, cycles_to_microseconds(write_cycles_ / writes_));
    }
    virtual int read(const blk_address & blk_addr, void* buffer) {
        uint64_t start = ticks();
        QPair* dedicated_qpair = get_or_create_qpair(pthread_self());
        dedicated_qpair->synchronous_read(buffer, this->block_size, blk_addr);
        read_cycles_ += ticks() - start;
        reads_++;
    }
    virtual int write(const blk_address & blk_addr, void* buffer) {
        uint64_t start = ticks();
        QPair* dedicated_qpair = get_or_create_qpair(pthread_self());
        dedicated_qpair->synchronous_write(buffer, this->block_size, blk_addr);
        write_cycles_ += ticks() - start;
        writes_++;
    }

    void asynch_read(const blk_address& blk_addr, void* buffer, call_back_context* context) = delete;

    static string pending_ios_to_string(unordered_map<int64_t, string> *pending_io_) {
        ostringstream ost;
        for (auto it = pending_io_->begin(); it != pending_io_->end(); ++it) {
            ost << it->first << "(" << it->second << ")" << " ";
        }
        return ost.str();
    }

    int process_completion(int max = 1) = delete;

    void asynch_write(const blk_address& blk_addr, void* buffer, call_back_context* context) = delete;

private:
    QPair* get_or_create_qpair(pthread_t& tid) {
        qpairs_lock_.acquire();
        QPair* qp;
        if (dedicated_qpairs_.find(tid) == dedicated_qpairs_.cend()) {
            qp = nvm_utility::allocateQPair(1);
            dedicated_qpairs_[tid] = qp;
        } else {
            qp = nvm_utility::allocateQPair(1);
        }
        qpairs_lock_.release();
        return qp;
    }

private:
    unordered_map<pthread_t, Qpair*> dedicated_qpairs_;
    SpinLock qpairs_lock_;
};

#endif //NVM_NVME_BLK_THREAD_DEDICATED_ACCESSOR_H
