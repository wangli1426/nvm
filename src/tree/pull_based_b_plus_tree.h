//
// Created by Li Wang on 11/12/17.
//

#ifndef NVM_PULL_BASED_B_PLUS_TREE_H
#define NVM_PULL_BASED_B_PLUS_TREE_H
//
// Created by robert on 9/11/17.
//

#include <queue>
#include <atomic>
#include <deque>
#include <thread>
#include <pthread.h>
#include <assert.h>
#include <boost/lockfree/queue.hpp>
//#include "in_nvme_b_plus_tree.h"
#include "vanilla_b_plus_tree.h"
#include "../utils/sync.h"
#include "../context/call_back.h"
#include "../tree/blk_node_reference.h"
#include "../context/barrier_manager.h"
#include "../utils/concurrent_queue.h"

#define SEARCH_REQUEST 1
#define INSERT_REQUEST 2
using namespace std;

typedef void (*callback_function)(void*);

namespace tree {


    template<typename K, typename V>
    class request {
    public:
        request(int t): type(t), ownership(true){};
        int type;
        K key;
        bool ownership;
    };

    template<typename K, typename V>
    class search_request: public request<K, V> {
    public:
        search_request(): request<K, V>(SEARCH_REQUEST) {};
    public:
        V *value;
        bool *found;
        Semaphore *semaphore;
        callback_function cb_f;
        void* args;
    };

    template<typename K, typename V>
    class insert_request: public request<K, V>  {
    public:
        insert_request(): request<K, V>(INSERT_REQUEST) {};
        V value;
        bool succeed;
        Semaphore* semaphore;
        callback_function  cb_f;
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
//            request_queue_ = moodycamel::ConcurrentQueue<request<K, V> *>(256);
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

        bool asynchronous_search_with_callback(search_request<K, V>* request) {
//            lock_.acquire();
//            request_queue_.push(request);
//            request_queue_size_++;
//            lock_.release();
//            pending_request_ ++;
//            return true;

//            bool succeed = request_queue_.enqueue(request);
//            assert(succeed);
//            pending_request_++;
//            return true;

            request_queue_.push(request);
            pending_request_++;
            return true;
        }

        bool asynchronous_insert_with_callback(insert_request<K, V>* request) {
//            lock_.acquire();
//            request_queue_.push(request);
//            request_queue_size_++;
//            lock_.release();
//            pending_request_++;
//            return true;

//            bool succeed = request_queue_.enqueue(request);
//            assert(succeed);
//            pending_request_++;
//            return true;

            request_queue_.push(request);
            pending_request_++;
            return true;
        }

        int get_pending_requests() const {
            return pending_request_.load();
        }

        request<K, V>* atomic_dequeue_request() {
//            request<K, V>* ret = nullptr;
//            lock_.acquire();
//            if (request_queue_.size() > 0) {
//                ret = request_queue_.front();
//                request_queue_.pop();
//                request_queue_size_--;
//            }
//            lock_.release();
//            return ret;


//            request<K, V>* ret;
//            if (request_queue_.try_dequeue(ret))
//                return ret;
//            else
//                return nullptr;

            request<K, V>* ret;
            if (request_queue_.pop(ret))
                return ret;
            else
                return nullptr;
        }

        static void *context_based_process(void* para) {
            pull_based_b_plus_tree* tree = reinterpret_cast<pull_based_b_plus_tree*>(para);
            while (!tree->working_thread_terminate_flag_ || tree->pending_request_.load() > 0) {
//                usleep(100000);
                request<K, V>* request;
                /**
                 TODO 1: if all the incoming operations can be completed by calling context->run only once, the pending
                 context will never be processed. One possible solution is to process_completion() in each round
                 regardless if new request arrives.

                 TODO 2: It seems to be more efficient in terms of both throughput and processing latency to process
                 multiple new requests at a time.

                 **/

                int64_t last = ticks();
                int32_t free = tree->free_context_slots_.load();
                while (free-- > 0 && (request = tree->atomic_dequeue_request()) != nullptr) {
//                while ((request = tree->atomic_dequeue_request()) != nullptr) {
                    call_back_context* context;
                    if (request->type == SEARCH_REQUEST) {
                        context = new search_context(tree, static_cast<search_request<K, V> *>(request));
                    } else {
                        context = new insert_context(tree, static_cast<insert_request<K, V> *>(request));
                    }
                    tree->free_context_slots_ --;
                    if (context->run() == CONTEXT_TERMINATED)
                        delete context;
                }
                if (tree->pending_request_.load() > 0) {
//                    const int processed = tree->blk_accessor_->process_completion(tree->queue_length_ / 8);
                    const int processed = tree->blk_accessor_->process_completion(tree->queue_length_ / 8);
                }
//                tree->manager.process_ready_context(tree->queue_length_);
            }
//            printf("context based process thread terminates!\n");
            return nullptr;
        }

        void sync() override {
            while(pending_request_.load()> 0) {
                usleep(1000);
            }
        }

    private:

        class insert_context: public call_back_context {
        public:
            insert_context(pull_based_b_plus_tree* tree, insert_request<K, V>* request):
                call_back_context(), tree_(tree), request_(request) {
                buffer_ = tree_->blk_accessor_->malloc_buffer();
                buffer_2 = tree_->blk_accessor_->malloc_buffer();
                node_ref_ = -1;
                current_state = 0;
                free_slot_available_in_parent_ = false;
                refer_to_root_ = false;
                optimistic_ = true;
                next_visit_is_leaf_node_ = tree_->depth_ == 1;
                current_node_level_ = tree_->get_height();
            }

            ~insert_context() {
                tree_->blk_accessor_->free_buffer(buffer_);
                tree_->blk_accessor_->free_buffer(buffer_2);
                buffer_ = 0;
                buffer_2 = 0;
            }

            int run() {
                while(true)
                switch (this->current_state) {
                    case 0: {
                        if (node_ref_ == -1) {
                            refer_to_root_ = true;
                            node_ref_ = tree_->root_->get_unified_representation();
                            current_node_level_ = tree_->get_height();
//                            printf("begin to insert [%d], root is %lld\n", request_->key,
//                                   tree_->root_->get_unified_representation());
                        }
                        set_next_state(10001);
//                        printf("[%d] --> <%lld>\n", request_->key, node_ref_->get_unified_representation());
                        bool obtained_barrier;
                        if (optimistic_ && !next_visit_is_leaf_node_)
                            obtained_barrier = tree_->manager.request_read_barrier(node_ref_, this);
                        else
                            obtained_barrier = tree_->manager.request_write_barrier(node_ref_, this);
                        if (obtained_barrier) {
                            transition_to_next_state();
//                            return run();
                            continue;
                        } else {
                            return CONTEXT_TRANSIT;
                        }
                    }
                    case 10001: {
                        if (refer_to_root_) {
                            if (obtained_barriers_.back().barrier_id_ != tree_->root_->get_unified_representation()) {
                                // root was updated
//                                printf("[%d]: detected root update!\n", request_->key);
                                release_all_barriers();
                                refer_to_root_ = true;
                                node_ref_ = -1;
                                set_next_state(0);
                                transition_to_next_state();
//                                return run();
                                continue;
                            } else {
                                refer_to_root_ = false;
                            }
                        }
                        if (optimistic_) {
                            barrier_token latest_token = obtained_barriers_.back();
                            obtained_barriers_.pop_back();
                            release_all_barriers();
                            obtained_barriers_.push_back(latest_token);
                        } else if (free_slot_available_in_parent_ && !parent_boundary_update_) {
                            //TODO release all the
                            barrier_token latest_token = obtained_barriers_.back();
                            obtained_barriers_.pop_back();
                            barrier_token latest_token_but_one = obtained_barriers_.back();
                            obtained_barriers_.pop_back();
                            release_all_barriers();
                            obtained_barriers_.push_back(latest_token_but_one);
                            obtained_barriers_.push_back(latest_token);
                        }
                        set_next_state(1);
                        transition_to_next_state();
//                        return run();
                        continue;
                    }

                    case 1: {
                        tree_->blk_accessor_->asynch_read(node_ref_, buffer_, this);
                        set_next_state(2);
                        return CONTEXT_TRANSIT;
                    }
                    case 2: {
                        uint32_t node_type = *reinterpret_cast<uint32_t*>(buffer_);
                        switch (node_type) {
                            case LEAF_NODE: {
                                assert(obtained_barriers_.back().type_ == WRITE_BARRIER);
                                current_node_ = new LeafNode<K, V, CAPACITY>(tree_->blk_accessor_, false);
                                current_node_->deserialize(buffer_);
                                if (optimistic_ && current_node_->size() == CAPACITY) {
                                    // leaf node is full, so the optimistic update fails.
                                    delete current_node_;
                                    current_node_ = 0;
                                    set_next_state(13);
                                    transition_to_next_state();
//                                    return run();
                                    continue;
                                }
                                child_node_split_ = current_node_->insert_with_split_support(request_->key, request_->value, split_);
                                if (!child_node_split_) {
                                    // the leaf node does not split, so we only need to flush the leaf node
                                    current_node_->serialize(buffer_);
                                    delete current_node_;
                                    current_node_ = 0;
                                    write_back_completion_target_ = 1;
                                    write_back_completion_count_ = 0;
                                    set_next_state(10);
                                    tree_->blk_accessor_->asynch_write(node_ref_, buffer_, this);
                                    return CONTEXT_TRANSIT;
                                } else {
                                    // the leaf node wat split and we need to flush both the leaf node and the new node.
                                    current_node_->serialize(buffer_);
                                    split_.right->serialize(buffer_2);
                                    if (current_node_->get_self_ref()->get_unified_representation() ==
                                            tree_->root_->get_unified_representation()) {
                                        set_next_state(12);
                                    } else {
                                        set_next_state(10);
                                    }
                                    write_back_completion_target_ = 2;
                                    write_back_completion_count_ = 0;
                                    tree_->blk_accessor_->asynch_write(node_ref_, buffer_, this);
                                    tree_->blk_accessor_->asynch_write(split_.right->get_self_ref()->get_unified_representation(), buffer_2, this);
                                    return CONTEXT_TRANSIT;
                                }
                            }
                            case INNER_NODE: {
                                current_node_ = new InnerNode<K, V, CAPACITY>(tree_->blk_accessor_, false);
                                current_node_->deserialize(buffer_);
                                InnerNode<K, V, CAPACITY>* inner_node = dynamic_cast<InnerNode<K, V, CAPACITY>*>(current_node_);
                                int target_node_index = inner_node->locate_child_index(request_->key);
                                const bool exceed_left_boundary = target_node_index < 0;
                                if (exceed_left_boundary && optimistic_) {
                                    // the insertion needs to update the inner node, so the optimistic insertion fails.
                                    delete current_node_;
                                    current_node_ = 0;
                                    set_next_state(13);
                                    transition_to_next_state();
//                                    return run();
                                    continue;
                                }
                                target_node_index = target_node_index < 0 ? 0 : target_node_index;
                                if (exceed_left_boundary) {
                                    inner_node->key_[0] = request_->key;
                                    inner_node->mark_modified();
                                }
                                store_parent_node(inner_node, target_node_index);
                                current_node_ = nullptr;
                                node_ref_ = inner_node->child_rep_[target_node_index];
                                free_slot_available_in_parent_ = inner_node->has_free_slot();
                                parent_boundary_update_ = exceed_left_boundary;
                                next_visit_is_leaf_node_ = current_node_level_ == 2;
                                current_node_level_ --;
                                set_next_state(0);
                                transition_to_next_state();
//                                return run();
                                continue;
                            }
                        }
                    }
                    case 9: {
                        // The insertion process goes into this state, when the tuple has been inserted into the
                        // child node. Depending on whether the child node was split during the insertion, the
                        // process logic varies. If the child node was split, we need to accommodate the new node;
                        // Otherwise, we only need to close the current node as well as the parent node(s).

                        if (child_node_split_) {
                            // child node was split

                            if (pending_parent_nodes_.empty()) {
                                // the root node was split.

                            } else {

                                parent_node_context parent_context = pending_parent_nodes_.back();
                                pending_parent_nodes_.pop_back();
                                InnerNode<K, V, CAPACITY> *parent_node = parent_context.node;
                                current_node_ = parent_node;
                                node_ref_ = current_node_->get_self_rep();

                                if (current_node_->size() < CAPACITY) {
                                    // the current node has free slot for the new node.
                                    parent_node->insert_inner_node(split_.right, split_.boundary_key,
                                                                   parent_context.position + 1);
                                    child_node_split_ = false;

                                    current_node_->serialize(buffer_);

                                    write_back_completion_count_ = 0;
                                    write_back_completion_target_ = 1;

                                    set_next_state(10);
                                    tree_->blk_accessor_->asynch_write(
                                            current_node_->get_self_ref()->get_unified_representation(), buffer_, this);
                                    delete split_.left;
                                    delete split_.right;
                                    delete parent_node;
                                    return CONTEXT_TRANSIT;
                                } else {
                                    // the current node need to split to accommodate the new node.
                                    bool insert_to_first_half = parent_context.position < CAPACITY / 2;

                                    //
                                    int start_index_for_right = CAPACITY / 2;
                                    InnerNode<K, V, CAPACITY> *left = reinterpret_cast<InnerNode<K, V, CAPACITY>*>(current_node_);
                                    InnerNode<K, V, CAPACITY> *right = new InnerNode<K, V, CAPACITY>(
                                            tree_->blk_accessor_);
                                    right->mark_modified();
                                    node_reference<K, V> *right_ref = right->get_self_ref();

                                    // move the keys and children to the right node
                                    for (int i = start_index_for_right, j = 0; i < left->size_; ++i, ++j) {
                                        right->key_[j] = left->key_[i];
                                        right->child_rep_[j] = left->child_rep_[i];
                                        right->child_[j] = left->child_[i];
                                    }

                                    const int moved = left->size_ - start_index_for_right;
                                    left->size_ -= moved;
                                    right->size_ = moved;
                                    left->mark_modified();

                                    // insert the new child node to the appropriate split node.
                                    InnerNode<K, V, CAPACITY> *host_for_node = insert_to_first_half ? left : right;
                                    int inner_node_insert_position = host_for_node->locate_child_index(
                                            split_.boundary_key);
                                    host_for_node->insert_inner_node(split_.right, split_.boundary_key,
                                                                     inner_node_insert_position + 1);
                                    delete split_.left;
                                    delete split_.right;

                                    // write the remaining content in the split data structure.
                                    split_.left = (left);
                                    split_.right = (right);
                                    split_.boundary_key = right->key_[0];

                                    child_node_split_ = true;

                                    left->serialize(buffer_);
                                    right->serialize(buffer_2);

                                    write_back_completion_count_ = 0;

                                    write_back_completion_target_ = 2;
                                    if (tree_->root_->get_unified_representation() !=
                                        left->get_self_ref()->get_unified_representation())
                                        set_next_state(10);
                                    else
                                        set_next_state(12);
                                    tree_->blk_accessor_->asynch_write(
                                            left->get_self_ref()->get_unified_representation(), buffer_, this);
                                    tree_->blk_accessor_->asynch_write(
                                            right->get_self_ref()->get_unified_representation(), buffer_2, this);
                                    return CONTEXT_TRANSIT;
                                }
                            }

                        } else {
                            while (!pending_parent_nodes_.empty()) {
                                parent_node_context parent_context = pending_parent_nodes_.back();
                                pending_parent_nodes_.pop_back();
                                InnerNode<K, V, CAPACITY>* parent_node = parent_context.node;
                                if (parent_node->is_modified()) {
                                    parent_node->serialize(buffer_);
                                    write_back_completion_target_ = 1;
                                    write_back_completion_count_ = 0;
                                    set_next_state(10);
                                    tree_->blk_accessor_->asynch_write(parent_node->get_self_ref()->get_unified_representation(), buffer_, this);
                                    delete parent_node;
                                    return CONTEXT_TRANSIT;
                                } else {
                                    delete parent_node;
                                }
                            }
                            set_next_state(11);
                            transition_to_next_state();
//                            return run();
                            continue;
                        }
                    }
                    case 10: {
                        write_back_completion_count_ ++;
                        if (write_back_completion_count_ == write_back_completion_target_) {
                            set_next_state(9);
                            transition_to_next_state();
//                            return run();
                            continue;
                        }
                        return CONTEXT_TRANSIT;
                    }
                    case 11: {
                        release_all_barriers(); // TODO: this can be done earlier.
                        if (request_->cb_f) {
                            (*request_->cb_f)(request_->args);
                        }
                        tree_->pending_request_--;
                        tree_->free_context_slots_++;
                        if (request_->ownership) {
                            request_->semaphore->post();
                            delete request_;
                        } else {
                            request_->semaphore->post();
                        }
                        return CONTEXT_TERMINATED;
                    }
                    case 12: {
                        // handle root node split
                        write_back_completion_count_ ++;
                        if (write_back_completion_count_ == write_back_completion_target_) {
                            InnerNode<K, V, CAPACITY> *new_inner_node = new InnerNode<K, V, CAPACITY>(split_.left, split_.right, tree_->blk_accessor_);
                            new_inner_node->mark_modified();
                            tree_->root_->copy(new_inner_node->get_self_ref());// the root_ reference which originally referred to a
                            // a leaf will refer to a inner node now. TODO: release the old root_
                            // reference and create a new one.
//                            tree_->root_->bind(new_inner_node);
                            tree_->depth_++;
                            new_inner_node->serialize(buffer_);
                            write_back_completion_count_ = 0;
                            write_back_completion_target_ = 1;
                            child_node_split_ = false;
                            set_next_state(11);
                            tree_->blk_accessor_->asynch_write(new_inner_node->get_self_ref()->get_unified_representation(), buffer_, this);
                            return CONTEXT_TRANSIT;
                        }
                        return CONTEXT_TRANSIT;
                    }

                    case 13: {
                        optimistic_ = false;
                        free_slot_available_in_parent_ = false;
                        refer_to_root_ = false;
                        next_visit_is_leaf_node_ = tree_->depth_ == 1;
                        current_node_level_ = tree_->get_height();
                        release_all_barriers();
                        set_next_state(0);
                        transition_to_next_state();
                        node_ref_ = -1;
//                        return run();
                        continue;
                    }

                }
                assert(false);
                return CONTEXT_TERMINATED;
            }

        private:
            void store_parent_node(InnerNode<K, V, CAPACITY>* node, int position) {
                pending_parent_nodes_.push_back(parent_node_context(node, position));
            }

            void release_all_barriers() {
                while (obtained_barriers_.size() > 0) {
                    barrier_token token = obtained_barriers_.back();
                    obtained_barriers_.pop_back();
                    if (token.type_ == READ_BARRIER) {
                        tree_->manager.remove_read_barrier(token.barrier_id_);
                    } else {
                        tree_->manager.remove_write_barrier(token.barrier_id_);
                    }
                }
            }

            struct parent_node_context {
                parent_node_context(InnerNode<K, V, CAPACITY>* n, int p) {
                    node = n;
                    position = p;
                }
                InnerNode<K, V, CAPACITY>* node;
                int position;
            };
        private:
            pull_based_b_plus_tree *tree_;
            blk_address node_ref_;
            void *buffer_, *buffer_2;
            Node<K, V>* current_node_;
            insert_request<K, V>* request_;
            std::deque<parent_node_context> pending_parent_nodes_;

            int write_back_completion_count_;
            int write_back_completion_target_;
            bool child_node_split_;
            Split<K, V> split_;

            bool free_slot_available_in_parent_;
            bool parent_boundary_update_;
            bool refer_to_root_;

            bool optimistic_;
            bool next_visit_is_leaf_node_;
            int current_node_level_;
        };


        class search_context : public call_back_context {
        public:

            uint64_t last;
            search_context(pull_based_b_plus_tree *tree, search_request<K, V>* request) : call_back_context(),
                                                                                                          tree_(tree), request_(request) {
                buffer_ = tree_->blk_accessor_->malloc_buffer();
                node_ref_ = tree->root_->get_unified_representation();
                last = ticks();
            };
            ~search_context() {
                tree_->blk_accessor_->free_buffer(buffer_);
                buffer_ = 0;
            }

            int run() {
//                int64_t duration = ticks() - last;
//                if (duration > 10000) {
//                    printf("during is %.2f ns, state: %d\n", cycles_to_nanoseconds(duration), current_state);
//                }
//                last = ticks();
                while(true)
                switch (this->current_state) {
                    case 0:
                        set_next_state(1);
                        if (tree_->manager.request_read_barrier(node_ref_, this)) {
                            transition_to_next_state();
//                            return run();
                            continue;
                        } else {
                            return CONTEXT_TRANSIT;
                        }
                    case 1:
                        assert(this->obtained_barriers_.size() <= 2);
                        if (this->obtained_barriers_.size() == 2) {
                            auto front = this->obtained_barriers_.front();
                            tree_->manager.remove_read_barrier(front.barrier_id_);
                            this->obtained_barriers_.pop_front();
                        }
                        set_next_state(2);
                        tree_->blk_accessor_->asynch_read(node_ref_, buffer_, this);
                        return CONTEXT_TRANSIT;
                    case 2: {
                        uint32_t node_type = *reinterpret_cast<uint32_t *>(buffer_);
                        switch (node_type) {
                            case LEAF_NODE: {
                                current_node_ = new LeafNode<K, V, CAPACITY>(tree_->blk_accessor_, false);
                                current_node_->deserialize(buffer_);
                                *request_->found = current_node_->search(request_->key, *request_->value);
                                delete current_node_;
                                current_node_ = 0;
                                if (request_->cb_f) {
                                    (*request_->cb_f)(request_->args);
                                }
                                tree_->pending_request_--;
                                tree_->free_context_slots_++;
                                if (request_->ownership) {
                                    request_->semaphore->post();
                                    delete request_;
                                    request_ = 0;
                                } else {
                                    request_->semaphore->post();
                                }
                                for (auto it = this->obtained_barriers_.begin(); it != obtained_barriers_.end(); ++it) {
                                    tree_->manager.remove_read_barrier((*it).barrier_id_);
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
                                    *request_->found = false;
                                    delete current_node_;
                                    current_node_ = 0;
                                    if (request_->cb_f) {
                                        (*request_->cb_f)(request_->args);
                                    }
                                    if (request_->ownership) {
                                        request_->semaphore->post();
                                        delete request_;
                                    } else {
                                        request_->semaphore->post();
                                    }
                                    request_ = 0;
                                    tree_->pending_request_--;
                                    tree_->free_context_slots_++;
                                    for (auto it = this->obtained_barriers_.begin();
                                         it != obtained_barriers_.end(); ++it) {
                                        tree_->manager.remove_read_barrier((*it).barrier_id_);
                                    }
                                    obtained_barriers_.clear();
                                    return CONTEXT_TERMINATED;
                                } else {
                                    node_ref_ = dynamic_cast<InnerNode<K, V, CAPACITY> *>(current_node_)->child_rep_[child_index];
                                    delete current_node_;
                                    current_node_ = 0;
                                    set_next_state(0);
                                    transition_to_next_state();
//                                    return run();
                                    continue;
                                }
                            }
                        }
                    }

                }
                assert(false);
                return CONTEXT_TERMINATED;
            }

        private:
            pull_based_b_plus_tree *tree_;
            blk_address node_ref_;
            void *buffer_;
            Node<K, V>* current_node_;
            search_request<K, V>* request_;
        };

    private:
        SpinLock lock_;
        Semaphore queue_free_slots_;
//        queue<request<K, V> *> request_queue_;
//        moodycamel::ConcurrentQueue<request<K, V> *> request_queue_;

        boost::lockfree::queue<request<K, V>*, boost::lockfree::capacity<512> > request_queue_;

        atomic<int> request_queue_size_;
        pthread_t thread_handle_;
        volatile bool working_thread_terminate_flag_;
        atomic<int> free_context_slots_;
        atomic<int> pending_request_;
        int queue_length_;
        barrier_manager manager;
    };
}

#endif //NVM_PULL_BASED_B_PLUS_TREE_H