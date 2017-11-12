//
// Created by Li Wang on 11/12/17.
//

#ifndef NVM_NVME_OPTIMIZED_TREE_FOR_BENCHMARK_H
#define NVM_NVME_OPTIMIZED_TREE_FOR_BENCHMARK_H

#include <stdio.h>
#include "nvme_optimized_b_plus_tree.h"
namespace tree{
    template <typename K, typename V, int CAPACITY>
    class nvme_optimized_tree_for_benchmark: public nvme_optimized_b_plus_tree<K, V, CAPACITY> {

    public:
        nvme_optimized_tree_for_benchmark(int queue_length): nvme_optimized_b_plus_tree<K, V, CAPACITY>(queue_length) {
            semaphore = new Semaphore(queue_length);
        }

        bool search(const K& key, V & value) {
            search_context* context = new search_context();
            context->key = key;
            context->sema = semaphore;
            semaphore->wait();
            this->asynchronous_search_with_callback(key, context->value, callback, context);
            printf("submitted %d\n", key);
        }
        static void callback(void* args) {
            search_context* context = reinterpret_cast<search_context*>(args);
            context->sema->post();
            printf("%d -> %d\n", context->key, context->value);
            delete context;
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
