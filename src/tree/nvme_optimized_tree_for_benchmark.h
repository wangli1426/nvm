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
            search_request<K,V>* request =
                    new search_request<K,V>;
            request->key = key;
            request->value = value;
            request->semaphore = semaphore;
            request->cb_f = &callback;
            request->args = request;
            this->asynchronous_search_with_callback(request);
        }
        static void callback(void* args) {
            search_request<K,V>* context =
                    reinterpret_cast<search_request<K,V>*>(args);
//            context->semaphore->post();
            printf("%d -> %d\n", context->key, context->value);
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
