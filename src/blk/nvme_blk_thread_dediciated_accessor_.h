//
// Created by robert on 7/11/17.
//

#ifndef NVM_NVME_BLK_THREAD_DEDICATED_ACCESSOR_H
#define NVM_NVME_BLK_THREAD_DEDICATED_ACCESSOR_H

#include <unordered_set>
#include <stdio.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <pthread.h>
#include "blk_cache.h"
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
        cache_ = 0;
        cache_ = new blk_cache(block_size, 1000);
    };

    ~nvme_blk_thread_dedicated_accessor() {
        for(auto it = thread_to_qpair.cbegin(); it != thread_to_qpair.cend(); ++it) {
            delete it->second;
        }
        delete cache_;
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
        this->allocation_cur = 0;
        for(int i = 0; i < max_queue_pair; i++) {
            qpairs_.push_back(nvm::nvm_utility::allocateAtomicQPair(1));
        }
    }

   virtual int close() {

       if (!this->closed_) {
//           for (auto it = thread_to_qpair.cbegin(); it != thread_to_qpair.cend(); ++it) {
//               it->second->free_qpair();
//           }
           thread_to_qpair.clear();
           for(auto it = qpairs_.cbegin(); it != qpairs_.cend(); ++it) {
               delete *it;
           }
           qpairs_.clear();
           this->closed_ = true;
       }
    }
    virtual int read(const blk_address & blk_addr, void* buffer) {
        uint64_t start = ticks();

        cache_lock_.acquire();
        if (cache_ && cache_->read(blk_addr, buffer)) {
            cache_lock_.release();
            this->metrics_.read_cycles_ += ticks() - start;
            this->metrics_.reads_++;
            return this->block_size;
        }
        cache_lock_.release();

        QPair* dedicated_qpair = get_or_create_qpair(pthread_self());
        dedicated_qpair->synchronous_read(buffer, this->block_size, blk_addr);
        this->metrics_.read_cycles_ += ticks() - start;
        this->metrics_.reads_++;

        if (cache_) {
            blk_cache::cache_unit unit;
            cache_lock_.acquire();
            bool evicted = cache_->write(blk_addr, buffer, false, unit);
            cache_lock_.release();
            if (evicted && unit.dirty) {
                write(unit.id, unit.data);
                free(unit.data);
            }
        }
    }
    virtual int write(const blk_address & blk_addr, void* buffer) {
        uint64_t start = ticks();
        if (cache_) {
            cache_lock_.acquire();
            cache_->invalidate(blk_addr);
            cache_lock_.release();
        }
        QPair* dedicated_qpair = get_or_create_qpair(pthread_self());
        dedicated_qpair->synchronous_write(buffer, this->block_size, blk_addr);
        this->metrics_.write_cycles_ += ticks() - start;
        this->metrics_.writes_++;

//        if (cache_) {
//            blk_cache::cache_unit unit;
//            cache_lock_.acquire();
//            bool evicted = cache_->write(blk_addr, buffer, false, unit);
//            cache_lock_.release();
//            if (evicted) {
//                assert(!unit.dirty);
//                free(unit.data);
//            }
//        }
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

    bool deregister_thread(const pthread_t& tid) {
        auto it = thread_to_qpair.find(tid);
        if (it != thread_to_qpair.end()) {
//            delete it->seccache_ond;
            thread_to_qpair.erase(tid);
        }
    }

private:
    QPair* get_or_create_qpair(const pthread_t& tid) {
        QPair* qp;
        qpairs_lock_.acquire();
        unordered_map<pthread_t, QPair*>::const_iterator it = thread_to_qpair.find(tid);
        qpairs_lock_.release();
        if (it == thread_to_qpair.cend()) {
            qp = qpairs_[allocation_cur++ % max_queue_pair];
            qpairs_lock_.acquire();
            thread_to_qpair[tid] = qp;
            qpairs_lock_.release();
        } else {
            qp = it->second;
        }
        return qp;
    }

private:
    unordered_map<pthread_t, QPair*> thread_to_qpair;
    vector<QPair*> qpairs_;
    int allocation_cur;
    SpinLock qpairs_lock_;
    const int max_queue_pair = 31;

    blk_cache *cache_;
    SpinLock cache_lock_;
};

#endif //NVM_NVME_BLK_THREAD_DEDICATED_ACCESSOR_H
