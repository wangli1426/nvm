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
using namespace std;
namespace tree {

    template<typename K, typename V, int CAPACITY>
    class nvme_optimized_b_plus_tree: public VanillaBPlusTree<K, V, CAPACITY> {
    public:
        struct search_request {
            K key;
            V value;
            bool found;
            Semaphore semaphore;
            search_request() {
                semaphore = Semaphore(0);
            }
        };
    public:
        nvme_optimized_b_plus_tree(uint32_t queue_length): VanillaBPlusTree<K, V, CAPACITY>(0) {
            queue_free_slots_ = Semaphore(1);
            working_thread_terminate_flag_ = false;
            pthread_create(&thread_handle_, NULL, nvme_optimized_b_plus_tree::worker_thread_logic, this);
        };

        ~nvme_optimized_b_plus_tree() {
            working_thread_terminate_flag_ = true;
            pthread_join(thread_handle_, NULL);
        }

        void init() {
            VanillaBPlusTree<K, V, CAPACITY>::init();
        }

        void close() {
            VanillaBPlusTree<K, V, CAPACITY>::close();
        }

        bool asynchronous_search(K key, V& value) {
            queue_free_slots_.wait();
            search_request request;
            request.key = key;
            lock_.acquire();
            request_queue_.push(&request);
            lock_.release();
            request.semaphore.wait();
            if(request.found) {
                value = request.value;
                return true;
            } else
                return false;
        }

        static void* worker_thread_logic(void* para) {
            nvme_optimized_b_plus_tree* tree = reinterpret_cast<nvme_optimized_b_plus_tree*>(para);
            while(!tree->working_thread_terminate_flag_) {
                search_request* request;
                while(!tree->lock_.try_lock()) {
                    if (tree->working_thread_terminate_flag_)
                        return;
                }
                if (tree->request_queue_.size() > 0) {
                    request = tree->request_queue_.front();
                    tree->request_queue_.pop();
                    tree->lock_.release();
                } else {
                    tree->lock_.release();
                    continue;
                }

                // we get a new search request.
                printf("search key: %d\n", request->key);
                request->found = tree->search(request->key, request->value);
                request->semaphore.post();

                tree->queue_free_slots_.post();
            }
        }

    private:
        SpinLock lock_;
        Semaphore queue_free_slots_;
        queue<search_request*> request_queue_;
        pthread_t thread_handle_;
        volatile bool working_thread_terminate_flag_;
    };
}

#endif //NVM_NVME_OPTIMIZED_B_PLUS_TREE_H
