//
// Created by Li Wang on 11/12/17.
//

#ifndef NVM_PULL_BASED_B_PLUS_TREE_H
#define NVM_PULL_BASED_B_PLUS_TREE_H
//
// Created by robert on 9/11/17.
//

#include <queue>
#include <thread>
#include <pthread.h>
#include <assert.h>
//#include "in_nvme_b_plus_tree.h"
#include "vanilla_b_plus_tree.h"
#include "../utils/sync.h"
#include "../context/call_back.h"
#include "../tree/blk_node_reference.h"
#include "../context/barrier_manager.h"

using namespace std;

typedef void (*callback_function)(void*);

namespace tree {

    template<typename K, typename V>
    class search_request {
    public:
        K key;
        V value;
        bool found;
        Semaphore *semaphore;
        callback_function cb_f;
        void* args;
    };

    template<typename K, typename V, int CAPACITY>
    class pull_based_b_plus_tree : public VanillaBPlusTree<K, V, CAPACITY> {
    public:


//        class search_request_with_semaphore: public search_request {
//        public:
//            Semaphore semaphore;
//        };

    public:
        pull_based_b_plus_tree(uint32_t queue_length) : VanillaBPlusTree<K, V, CAPACITY>(0) {
            queue_free_slots_ = Semaphore(queue_length);
            free_context_slots_ = queue_length;
            pending_request_ = 0;
            queue_length_ = queue_length;
        };

        ~pull_based_b_plus_tree() {
            working_thread_terminate_flag_ = true;
        }

        virtual void init() {
            create_and_init_blk_accessor();
            VanillaBPlusTree<K, V, CAPACITY>::init();
            working_thread_terminate_flag_ = false;
            pthread_create(&thread_handle_, NULL, pull_based_b_plus_tree::context_based_process, this);
        }

        virtual void create_and_init_blk_accessor() = 0;

        virtual void close() {

            working_thread_terminate_flag_ = true;
            if (pthread_join(thread_handle_, NULL) != 0) {
                printf("failed to join the thread!\n");
            }
            VanillaBPlusTree<K, V, CAPACITY>::close();
        }

        bool asynchronous_search(K key, V &value) {
            queue_free_slots_.wait();
            search_request<K, V> request;
            request.found = false;
            request.key = key;
            lock_.acquire();
            pending_request_ ++;
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

        // the logic of this function is identical to of asynchronous_search, except for calling callback function before
        // return
        bool asynchronous_search_with_callback(search_request<K, V>* request) {
//            queue_free_slots_.wait();
//            printf("cb: %llx, arg: %llx\n", cb_f, args);
            lock_.acquire();
            pending_request_ ++;
            request_queue_.push(request);
            lock_.release();
//            request.semaphore.wait();
//            queue_free_slots_.post();
//            if (request.found) {
//                value = request.value;
//                cb_f(args);
//                return true;
//            } else
//                cb_f(args);
//            return false;
        }

        int get_pending_requests() const {
            return pending_request_;
        }

        static void *worker_thread_logic(void *para) {
            pull_based_b_plus_tree *tree = reinterpret_cast<pull_based_b_plus_tree *>(para);
            while (!tree->working_thread_terminate_flag_) {
                search_request<K, V> *request;
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
            pull_based_b_plus_tree* tree = reinterpret_cast<pull_based_b_plus_tree*>(para);
            while (!tree->working_thread_terminate_flag_ || tree->pending_request_ > 0) {
                search_request<K, V>* request;
                while (!tree->lock_.try_lock()) {
                    if (tree->working_thread_terminate_flag_ && tree->pending_request_ == 0) {
                        printf("context based process thread terminates!\n");
                        return nullptr;
                    }
                }

                /**
                 TODO 1: if all the incoming operations can be completed by calling context->run only once, the pending
                 context will never be processed. One possible solution is to process_completion() in each round
                 regardless if new request arrives.

                 TODO 2: It seems to be more efficient in terms of both throughput and processing latency to process
                 multiple new requests at a time.

                 **/

                if (tree->request_queue_.size() > 0 && tree->free_context_slots_ > 0) {
                    request = tree->request_queue_.front();
                    tree->request_queue_.pop();
                    tree->lock_.release();
                    ostringstream ost;
                    ost << "context for " << request->key;
                    search_context* context = new search_context(ost.str(), tree, request);
                    tree->free_context_slots_ --;
                    if (context->run() == CONTEXT_TERMINATED)
                        delete context;
                } else {
                    tree->lock_.release();
                    if (tree->pending_request_ > 0) {
                        const int processed = tree->blk_accessor_->process_completion(tree->queue_length_);
                        if (processed > 0) {
//                        tree->free_context_slots_ += processed;
//                        tree->pending_request_ -= processed;
                        }
                    }
                    continue;
                }
            }
            printf("context based process thread terminates!\n");
        }

        void sync() {
            while(pending_request_> 0) {
                usleep(1);
            }
        }

    private:
        class search_context : public call_back_context {
        public:
            search_context(std::string name, pull_based_b_plus_tree *tree, search_request<K, V>* request) : name_(name), call_back_context(name_.c_str()),
                                                                                                          tree_(tree), request_(request) {
                buffer_ = tree_->blk_accessor_->malloc_buffer();
                node_ref_ = reinterpret_cast<blk_node_reference<K, V, CAPACITY>*>(tree->root_);
            };
            ~search_context() {
                tree_->blk_accessor_->free_buffer(buffer_);
                buffer_ = 0;
            }

            int run() {
                switch (this->current_state) {
                    case 0:
                        int64_t node_id;
                        node_id = node_ref_->get_unified_representation();
                        set_next_state(1);
                        tree_->manager.request_read_barrier(node_id, this);
                        return CONTEXT_TRANSIT;
                    case 1:
                        assert(this->obtained_barriers_.size() <= 2);
                        if (this->obtained_barriers_.size() == 2) {
                            auto front = this->obtained_barriers_.front();
                            tree_->manager.remove_read_barrier((*front).barrier_id_);
                            this->obtained_barriers_.pop_front();
                        }
                        tree_->blk_accessor_->asynch_read(node_ref_->get_unified_representation(), buffer_, this);
                        set_next_state(2);
                        return CONTEXT_TRANSIT;
                    case 2: {
                        uint32_t node_type = *reinterpret_cast<uint32_t *>(buffer_);
                        switch (node_type) {
                            case LEAF_NODE: {
                                current_node_ = new LeafNode<K, V, CAPACITY>(tree_->blk_accessor_, false);
                                current_node_->deserialize(buffer_);
                                request_->found = current_node_->search(request_->key, request_->value);
                                delete current_node_;
                                current_node_ = 0;
                                request_->semaphore->post();
                                if (request_->cb_f) {
                                    (*request_->cb_f)(request_->args);
                                }
                                tree_->pending_request_--;
                                tree_->free_context_slots_++;
                                delete request_;
                                request_ = 0;
                                for (auto it = this->obtained_barriers_.begin(); it != obtained_barriers_.end(); ++it) {
                                    tree_->manager.remove_read_barrier((*it)->barrier_id_);
                                }
                                obtained_barriers_.clear();
                                return CONTEXT_TERMINATED;
                            }
                            case INNER_NODE: {
                                current_node_ = new InnerNode<K, V, CAPACITY>(tree_->blk_accessor_, false);
                                current_node_->deserialize(buffer_);
                                int child_index = dynamic_cast<InnerNode<K, V, CAPACITY> *>(current_node_)->locate_child_index(
                                        request_->key);
                                if (child_index < 0) {
                                    request_->found = false;
                                    delete current_node_;
                                    current_node_ = 0;
                                    request_->semaphore->post();
                                    if (request_->cb_f) {
                                        (*request_->cb_f)(request_->args);
                                    }
                                    tree_->pending_request_--;
                                    tree_->free_context_slots_++;
                                    delete request_;
                                    request_ = 0;
                                    for (auto it = this->obtained_barriers_.begin();
                                         it != obtained_barriers_.end(); ++it) {
                                        tree_->manager.remove_read_barrier((*it)->barrier_id_);
                                    }
                                    obtained_barriers_.clear();
                                    return CONTEXT_TERMINATED;
                                } else {
                                    node_ref_ = reinterpret_cast<blk_node_reference<K, V, CAPACITY> *>(tree_->blk_accessor_->create_null_ref());
                                    node_ref_->copy(
                                            dynamic_cast<InnerNode<K, V, CAPACITY> *>(current_node_)->get_child_reference(
                                                    child_index));
//                                    delete current_node_;
                                    current_node_ = 0;
                                    set_next_state(0);
                                    transition_to_next_state();
                                    return run();
                                }
                            }
//                            default:
//                                assert(false);
                        }
                    }

                }
            }

        private:
            pull_based_b_plus_tree *tree_;
            blk_node_reference<K, V, CAPACITY> *node_ref_;
            void *buffer_;
            Node<K, V>* current_node_;
            search_request<K, V>* request_;
            std::string name_;
        };

    private:
        SpinLock lock_;
        Semaphore queue_free_slots_;
        queue<search_request<K, V> *> request_queue_;
        pthread_t thread_handle_;
        volatile bool working_thread_terminate_flag_;
        volatile int free_context_slots_;
        volatile int pending_request_;
        int queue_length_;
        barrier_manager manager;
    };
}

#endif //NVM_PULL_BASED_B_PLUS_TREE_H