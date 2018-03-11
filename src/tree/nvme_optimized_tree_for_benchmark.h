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
            pending_operations_ = 0;
        }

        ~nvme_optimized_tree_for_benchmark() {
//            printf("%d found!\n", found);
        }

        bool search(const K& key, V & value) {
//            search_request<K,V>* request =
//                    new search_request<K,V>;
//            bool *found = new bool;
//            request->key = key;
//            request->value = &value;
//            request->found = found;
//            request->semaphore = semaphore;
//            request->cb_f = &callback;
//            request->args = request;
//            semaphore->wait();
//            this->asynchronous_search_with_callback(request);
//            return *found;
            bool* found = new bool;
            pending_operations_++;
            search_request<K,V>* request = new search_request<K,V>;
            request->ownership = true;
            request->start = ticks();
            request->key = key;
            request->value = &value;
            request->found = found;
            request->semaphore = nullptr;
            request->cb_f = &search_callback;
            search_call_back_para* para = new search_call_back_para;
            para->request = request;
            para->tree = this;
            request->args = para;
            this->asynchronous_search_with_callback(request);
            return *found;
        }

        void insert(const K& key, const V & value) {
            insert_request<K, V>* request = new insert_request<K, V>;
            pending_operations_++;
            request->ownership = true;
            request->key = key;
            request->start = ticks();
            request->value = value;
            request->semaphore = nullptr;
            request->cb_f = &insert_callback;
            insert_call_back_para* para = new insert_call_back_para;
            para->request = request;
            para->tree = this;
            request->args = para;
            this->asynchronous_insert_with_callback(request);
        }

        struct insert_call_back_para {
            nvme_optimized_tree_for_benchmark<K,V, CAPACITY>* tree;
            insert_request<K, V>* request;
        };

        struct search_call_back_para {
            nvme_optimized_tree_for_benchmark<K,V, CAPACITY>* tree;
            search_request<K, V>* request;
        };

        void sync() override {
            while(pending_operations_.load()) {
                usleep(1000);
            }
        }

        static void insert_callback(void* args) {
            insert_call_back_para* para = reinterpret_cast<insert_call_back_para*>(args);
            para->tree->pending_operations_--;
            para->tree->metrics_.add_write_latency(ticks() - para->request->start);
            delete para;
        }

        static void search_callback(void* args) {
            search_call_back_para* para = reinterpret_cast<search_call_back_para*>(args);
            para->tree->pending_operations_--;
            para->tree->metrics_.add_read_latency(ticks() - para->request->start);
            delete para;
        }

        static void callback(void* args) {
//            search_request<K,V>* context =
//                    reinterpret_cast<search_request<K,V>*>(args);
////            if (context->key != context->value)
////                printf("%d -> %d\n", context->key, context->value);
//            if (context->found)
//                found++;
            nvme_optimized_tree_for_benchmark<K,V, CAPACITY>* pthis = reinterpret_cast<nvme_optimized_tree_for_benchmark<K,V, CAPACITY>*>(args);
            pthis->pending_operations_--;
        }

    struct search_context {
        K key;
        V value;
        Semaphore* sema;
    };
    private:
        atomic<long> pending_operations_;
    };
}
#endif //NVM_NVME_OPTIMIZED_TREE_FOR_BENCHMARK_H
