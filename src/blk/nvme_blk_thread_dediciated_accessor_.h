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

template<typename K, typename V, int CAPACITY>
class nvme_blk_thread_dedicated_accessor: public nvme_blk_accessor<K, V, CAPACITY> {
public:
    nvme_blk_thread_dedicated_accessor(const int& block_size): nvme_blk_accessor<K, V, CAPACITY>(block_size) {
    };

    ~nvme_blk_thread_dedicated_accessor() {
        for(auto it = dedicated_qpairs_.cbegin(); it != dedicated_qpairs_.cend(); ++it) {
            delete it->second;
        }
    }

    virtual int open() {
        int status = nvm::nvm_utility::initialize_namespace();
        if (status != 0) {
            cout << "Errors in initialization of namespace." << endl;
            exit(1);
        } else {
            cout << "namespace is initialized." << endl;
        }
        this->open_time_ = ticks();
        this->closed_ = false;
    }

   virtual int close() {

       if (!this->closed_) {
           for (auto it = dedicated_qpairs_.cbegin(); it != dedicated_qpairs_.cend(); ++it) {
               it->second->free_qpair();
           }
           this->closed_ = true;
       }
    }
    virtual int read(const blk_address & blk_addr, void* buffer) {
        uint64_t start = ticks();
        QPair* dedicated_qpair = get_or_create_qpair(pthread_self());
        dedicated_qpair->synchronous_read(buffer, this->block_size, blk_addr);
        this->metrics_.read_cycles_ += ticks() - start;
        this->metrics_.reads_++;
    }
    virtual int write(const blk_address & blk_addr, void* buffer) {
        uint64_t start = ticks();
        QPair* dedicated_qpair = get_or_create_qpair(pthread_self());
        dedicated_qpair->synchronous_write(buffer, this->block_size, blk_addr);
        this->metrics_.write_cycles_ += ticks() - start;
        this->metrics_.writes_++;
    }

    static string pending_ios_to_string(unordered_map<int64_t, string> *pending_io_) {
        ostringstream ost;
        for (auto it = pending_io_->begin(); it != pending_io_->end(); ++it) {
            ost << it->first << "(" << it->second << ")" << " ";
        }
        return ost.str();
    }

    std::string get_name() const {
        return std::string("NVM(dedicated)");
    }

private:
    QPair* get_or_create_qpair(const pthread_t& tid) {
        QPair* qp;
        qpairs_lock_.acquire();
        unordered_map<pthread_t, QPair*>::const_iterator it = dedicated_qpairs_.find(tid);
        qpairs_lock_.release();
        if (it == dedicated_qpairs_.cend()) {
            qp = nvm_utility::allocateQPair(1);
            qpairs_lock_.acquire();
            dedicated_qpairs_[tid] = qp;
            qpairs_lock_.release();
        } else {
            qp = it->second;
        }
        return qp;
    }

private:
    unordered_map<pthread_t, QPair*> dedicated_qpairs_;
    SpinLock qpairs_lock_;
};

#endif //NVM_NVME_BLK_THREAD_DEDICATED_ACCESSOR_H
