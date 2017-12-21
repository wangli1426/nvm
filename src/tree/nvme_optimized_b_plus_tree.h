//
// Created by robert on 9/11/17.
//

#ifndef NVM_NVME_OPTIMIZED_B_PLUS_TREE_H
#define NVM_NVME_OPTIMIZED_B_PLUS_TREE_H

#include <queue>
#include <thread>
#include <pthread.h>
#include "../utils/dummy.h"
#include "in_nvme_b_plus_tree.h"
#include "vanilla_b_plus_tree.h"
#include "../utils/sync.h"
#include "../context/call_back.h"
#include "../tree/blk_node_reference.h"
#include "pull_based_b_plus_tree.h"
#include "../utils/cpu_set.h"
#include <spdk/nvme.h>

using namespace std;


namespace tree {

    template<typename K, typename V, int CAPACITY>
    class nvme_optimized_b_plus_tree : public pull_based_b_plus_tree<K, V, CAPACITY> {
    public:
        nvme_optimized_b_plus_tree(const int block_size, int queue_length):
                pull_based_b_plus_tree<K, V, CAPACITY>(queue_length), block_size_(block_size) {
        }

        void create_and_init_blk_accessor() {
            this->blk_accessor_ = new nvme_blk_accessor<K, V, CAPACITY>(block_size_);
            this->blk_accessor_->open();
            spdk_unaffinitize_thread();
            print_current_cpu_set();
            set_cpu_set(32);
            print_current_cpu_set();
        }


        virtual bool search(const K &key, V &value) override {
            search_request<K,V> request;
            request.ownership = false;
            SpinLock semaphore;
            semaphore.acquire();
            bool found = true;
            request.key = key;
            request.value = &value;
            request.found = &found;
            request.semaphore = &semaphore;
            request.cb_f = nullptr;
            request.args = 0;
            this->asynchronous_search_with_callback(&request);
            while(!semaphore.try_lock()) {
                usleep(1);
            }
            return found;
        }

        virtual void insert(const K &key, const V &value) override {
            insert_request<K,V> request;
            request.ownership = false;
            SpinLock semaphore;
            semaphore.acquire();
            request.key = key;
            request.value = value;
            request.semaphore = &semaphore;
            request.cb_f = nullptr;
            request.args = 0;
            this->asynchronous_insert_with_callback(&request);
            while(!semaphore.try_lock()) {
//                usleep(10);
            }
        }

    private:
        int block_size_;
    };
}

#endif //NVM_NVME_OPTIMIZED_B_PLUS_TREE_H
