//
// Created by Li Wang on 11/12/17.
//

#ifndef NVM_DISK_OPTIMIZED_TREE_FOR_BENCHMARK_H
#define NVM_DISK_OPTIMIZED_TREE_FOR_BENCHMARK_H

#include <stdio.h>
#include "disk_optimized_b_plus_tree.h"
namespace tree{
    template <typename K, typename V, int CAPACITY>
    class disk_optimized_tree_for_benchmark: public disk_optimized_b_plus_tree<K, V, CAPACITY> {

    public:
        disk_optimized_tree_for_benchmark(int queue_length, const char* path = "temp.dat"):
                disk_optimized_b_plus_tree<K, V, CAPACITY>(path, queue_length){
            semaphore = new Semaphore(queue_length);
        }

        bool search(const K& key, V & value) override {
//            search_context* context = new search_context();
//            context->key = key;
//            context->sema = semaphore;
//            semaphore->wait();
//            this->asynchronous_search_with_callback(key, context->value, &callback, context);
            bool* found = new bool;
            search_request<K,V>* request = new search_request<K,V>;
            request->key = key;
            request->value = &value;
            request->found = found;
            request->semaphore = semaphore;
            request->cb_f = &callback;
            request->args = request;
            semaphore->wait();
            this->asynchronous_search_with_callback(request);
            return *found;
        }

        void insert(const K& key, const V & value) override {
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
            if (context->key != context->value)
                printf("%d -> %d\n", context->key, context->value);
        }

        static void insert_callback(void* args) {
//            search_request<K,V>* context =
//                    reinterpret_cast<search_request<K,V>*>(args);
//                printf("[%d,%d] is inserted!\n", context->key, context->value);
        }
//    struct search_context {
//        K key;
//        V value;
//        Semaphore* sema;
//    };
    private:
        Semaphore* semaphore;
    };
}
#endif //NVM_DISK_OPTIMIZED_TREE_FOR_BENCHMARK_H
