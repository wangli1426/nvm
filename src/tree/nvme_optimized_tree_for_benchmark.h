//
// Created by Li Wang on 11/12/17.
//

#ifndef NVM_NVME_OPTIMIZED_TREE_FOR_BENCHMARK_H
#define NVM_NVME_OPTIMIZED_TREE_FOR_BENCHMARK_H

#include <stdio.h>
#include "nvme_optimized_b_plus_tree.h"

static volatile int found = 0;
namespace tree{
    template <typename K, typename V, int CAPACITY>
    class nvme_optimized_tree_for_benchmark: public nvme_optimized_b_plus_tree<K, V, CAPACITY> {

    public:
        nvme_optimized_tree_for_benchmark(const int block_size, int queue_length): nvme_optimized_b_plus_tree<K, V, CAPACITY>(block_size, queue_length) {
            semaphore = new Semaphore(queue_length);
        }

        ~nvme_optimized_tree_for_benchmark() {
            printf("%d found!\n", found);
        }

        bool search(const K& key, V & value) {
            search_request<K,V>* request =
                    new search_request<K,V>;
            request->key = key;
            request->value = value;
            request->semaphore = semaphore;
            request->cb_f = &callback;
            request->args = request;
            semaphore->wait();
            this->asynchronous_search_with_callback(request);
        }

        void insert(const K& key, const V & value) {
            insert_request<K, V>* request = new insert_request<K, V>;
            request->key = key;
            request->value = value;
            request->semaphore = semaphore;
            request->cb_f = &insert_callback;
            request->args = request;
            semaphore->wait();
            this->asynchronous_insert_with_callback(request);
        }

        static void callback(void* args) {
            search_request<K,V>* context =
                    reinterpret_cast<search_request<K,V>*>(args);
//            if (context->key != context->value)
//                printf("%d -> %d\n", context->key, context->value);
            if (context->found)
                found++;
        }

        static void insert_callback(void* args) {
//            search_request<K,V>* context =
//                    reinterpret_cast<search_request<K,V>*>(args);
//                printf("[%d,%d] is inserted!\n", context->key, context->value);
        }
    struct search_context {
        K key;
        V value;
        Semaphore* sema;
    };
    private:
        Semaphore* semaphore;
    };
}
#endif //NVM_NVME_OPTIMIZED_TREE_FOR_BENCHMARK_H
