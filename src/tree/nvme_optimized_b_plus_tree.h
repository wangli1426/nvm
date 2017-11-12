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

using namespace std;
namespace tree {

    template<typename K, typename V, int CAPACITY>
    class nvme_optimized_b_plus_tree : public VanillaBPlusTree<K, V, CAPACITY> {
    public:
        struct search_request {
            K key;
            V value;
            bool found;
            Semaphore semaphore;
        };

    public:
        nvme_optimized_b_plus_tree(uint32_t queue_length) : VanillaBPlusTree<K, V, CAPACITY>(0) {
            queue_free_slots_ = Semaphore(1);
            free_context_slots_ = 8;
            working_thread_terminate_flag_ = false;
            this->blk_accessor_ = new file_blk_accessor<K, V, CAPACITY>("./tree.dat", 512);
            this->blk_accessor_->open();
//            pthread_create(&thread_handle_, NULL, nvme_optimized_b_plus_tree::worker_thread_logic, this);
            pthread_create(&thread_handle_, NULL, nvme_optimized_b_plus_tree::context_based_process, this);
        };

        ~nvme_optimized_b_plus_tree() {
            working_thread_terminate_flag_ = true;
            pthread_join(thread_handle_, NULL);
        }

        void init() {
            VanillaBPlusTree<K, V, CAPACITY>::init();
        }

        void close() {
            working_thread_terminate_flag_ = true;

            VanillaBPlusTree<K, V, CAPACITY>::close();
        }

        bool asynchronous_search(K key, V &value) {
            queue_free_slots_.wait();
            search_request request;
            request.found = false;
            request.key = key;
            lock_.acquire();
            request_queue_.push(&request);
            lock_.release();
            request.semaphore.wait();
            queue_free_slots_.post();
            if (request.found) {
                value = request.value;
                return true;
            } else
                return false;
        }

        static void *worker_thread_logic(void *para) {
            nvme_optimized_b_plus_tree *tree = reinterpret_cast<nvme_optimized_b_plus_tree *>(para);
            while (!tree->working_thread_terminate_flag_) {
                search_request *request;
                while (!tree->lock_.try_lock()) {
                    if (tree->working_thread_terminate_flag_)
                        return nullptr;
                }
                if (tree->request_queue_.size() > 0) {
                    request = tree->request_queue_.front();
                    tree->request_queue_.pop();
                    tree->lock_.release();

                    // we get a new search request.
                    printf("search key: %d\n", request->key);
                    request->found = tree->search(request->key, request->value);
                    request->semaphore.post();
                } else {
                    tree->lock_.release();
                    continue;
                }


            }
        }

        static void *context_based_process(void* para) {
            nvme_optimized_b_plus_tree* tree = reinterpret_cast<nvme_optimized_b_plus_tree*>(para);
            while (!tree->working_thread_terminate_flag_) {
                search_request* request;
                while (!tree->lock_.try_lock()) {
                    if (tree->working_thread_terminate_flag_)
                        return nullptr;
                }
                if (tree->request_queue_.size() > 0 && tree->free_context_slots_ > 0) {
                    request = tree->request_queue_.front();
                    tree->request_queue_.pop();
                    tree->lock_.release();
                    ostringstream ost;
                    ost << "context for " << request->key;
                    search_context* context = new search_context(ost.str().c_str(), tree, request);
                    tree->free_context_slots_ --;
                    if (context->run() == CONTEXT_TERMINATED)
                        tree->free_context_slots_ ++;
                } else {
                    tree->lock_.release();
                    const int processed = tree->blk_accessor_->process_completion();
                    tree->free_context_slots_ ++;
                    continue;
                }
            }
        }

    private:
        class search_context : public call_back_context {
        public:
            search_context(const char *name, nvme_optimized_b_plus_tree *tree, search_request* request) : call_back_context(name),
                                                                                 tree_(tree), request_(request) {
                buffer_ = tree_->blk_accessor_->malloc_buffer();
                node_ref_ = reinterpret_cast<blk_node_reference<K, V, CAPACITY>*>(tree->root_);
            };
            ~search_context() {
                tree_->blk_accessor_->free_buffer(buffer_);
                buffer_ = 0;
            }

            int run() {
                switch (this->status) {
                    case 0:
                        tree_->blk_accessor_->asynch_read(node_ref_->get_unified_representation(), buffer_, this);
                        transition_to_state(1);
                        return CONTEXT_TRANSIT;
                    case 1:
                        uint32_t node_type = *reinterpret_cast<uint32_t*>(buffer_);
                        switch (node_type) {
                            case LEAF_NODE:
                                current_node_ = new LeafNode<K, V, CAPACITY>(tree_->blk_accessor_, false);
                                current_node_->deserialize(buffer_);
                                request_->found = current_node_->search(request_->key, request_->value);
                                delete current_node_;
                                current_node_ = 0;
                                request_->semaphore.post();
                                return CONTEXT_TERMINATED;
                            case INNER_NODE:
                                current_node_ = new InnerNode<K, V, CAPACITY>(tree_->blk_accessor_, false);
                                current_node_->deserialize(buffer_);
                                int child_index = dynamic_cast<InnerNode<K, V, CAPACITY>*>(current_node_)->locate_child_index(request_->key);
                                if (child_index < 0) {
                                    request_->found = false;
                                    delete current_node_;
                                    current_node_ = 0;
                                    request_->semaphore.post();
                                    tree_->free_context_slots_ ++;
                                    return CONTEXT_TERMINATED;
                                } else {
                                    node_ref_ = reinterpret_cast<blk_node_reference<K, V, CAPACITY>*>(tree_->blk_accessor_->create_null_ref());
                                    node_ref_->copy(dynamic_cast<InnerNode<K, V, CAPACITY>*>(current_node_)->get_child_reference(child_index));
//                                    delete current_node_;
                                    current_node_ = 0;
                                    transition_to_state(0);
                                    return run();
                                }
//                            default:
//                                assert(false);
                        }

                }
            }

        private:
            nvme_optimized_b_plus_tree *tree_;
            blk_node_reference<K, V, CAPACITY> *node_ref_;
            void *buffer_;
            Node<K, V>* current_node_;
            search_request* request_;
        };

    private:
        SpinLock lock_;
        Semaphore queue_free_slots_;
        queue<search_request *> request_queue_;
        pthread_t thread_handle_;
        volatile bool working_thread_terminate_flag_;
        volatile int free_context_slots_;
    };
}

#endif //NVM_NVME_OPTIMIZED_B_PLUS_TREE_H
