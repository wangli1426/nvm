//
// Created by robert on 9/11/17.
//

#ifndef NVM_NVME_OPTIMIZED_B_PLUS_TREE_H
#define NVM_NVME_OPTIMIZED_B_PLUS_TREE_H

#include <queue>
#include <thread>
#include <pthread.h>
#include "in_nvme_b_plus_tree.h"
#include "vanilla_b_plus_tree.h"
#include "../utils/sync.h"
#include "../context/call_back.h"
#include "../tree/blk_node_reference.h"
#include "pull_based_b_plus_tree.h"

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
        }


        virtual bool search(const K &key, V &value) override {
            search_request<K,V>* request = new search_request<K,V>;
            Semaphore semaphore;
            bool found;
            request->key = key;
            request->value = &value;
            request->found = &found;
            request->semaphore = &semaphore;
            request->cb_f = nullptr;
            request->args = 0;
            this->asynchronous_search_with_callback(request);
            semaphore.wait();
            return found;
        }

        virtual void insert(const K &key, const V &value) override {
            insert_request<K,V>* request = new insert_request<K,V>;
            Semaphore semaphore;
            request->key = key;
            request->value = value;
            request->semaphore = &semaphore;
            request->cb_f = nullptr;
            request->args = 0;
            this->asynchronous_insert_with_callback(request);
            semaphore.wait();
        }

    private:
        int block_size_;
    };
}

#endif //NVM_NVME_OPTIMIZED_B_PLUS_TREE_H
