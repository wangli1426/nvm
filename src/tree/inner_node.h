//
// Created by Li Wang on 6/3/17.
//

#ifndef B_TREE_INNER_NODE_H
#define B_TREE_INNER_NODE_H

#include <iostream>
#include <sstream>
#include <string>
#include "node.h"
#include "leaf_node.h"
#include "in_memory_node_reference.h"
#include "../blk/blk.h"
#include "../utils/rdtsc.h"

namespace tree {

    template<typename K, typename V, int CAPACITY>
    class VanillaBPlusTree;

    template<typename K, typename V, int CAPACITY>
    class InnerNode : public Node<K, V> {
        friend class VanillaBPlusTree<K, V, CAPACITY>;

    public:
        InnerNode(blk_accessor<K, V>* blk_accessor = 0, bool allocate_blk_ref = true) : size_(0), blk_accessor_(blk_accessor) {
            initialize_child_refs();
            if (blk_accessor_) {
                if (allocate_blk_ref) {
                    self_ref_ = blk_accessor_->allocate_ref();
                    self_rep_ = self_ref_->get_unified_representation();
                    self_ref_->bind(this);
                } else {
//                    self_ref_ = blk_accessor_->create_null_ref();
//                    self_rep_ = self_ref_->get_unified_representation();
                    self_ref_ = nullptr;
                    self_rep_ = -1;
                }
            } else {
                self_ref_ = nullptr;
                self_rep_ = -1;
            }
        };

        InnerNode(Node<K, V> *left, Node<K, V> *right, blk_accessor<K, V>* blk_accessor = 0): blk_accessor_(blk_accessor) {
            initialize_child_refs();
            if (blk_accessor_) {
                self_ref_ = blk_accessor_->allocate_ref();
                self_rep_ = self_ref_->get_unified_representation();
                self_ref_->bind(this);
            } else {
                self_ref_ = 0;
                self_rep_ = -1;
            }


            size_ = 2;
            key_[0] = left->get_leftmost_key();
            child_[0] = blk_accessor_->create_null_ref();
            child_[0]->copy(left->get_self_ref());
            child_rep_[0] = child_[0]->get_unified_representation();
            key_[1] = right->get_leftmost_key();
            child_[1] = blk_accessor_->create_null_ref();
            child_[1]->copy(right->get_self_ref());
            child_rep_[1] = child_[1]->get_unified_representation();
        }

        virtual ~InnerNode() {
            for (int i = 0; i < size_; ++i) {
//                delete get_child_reference(i)->get(blk_accessor_);
//                child_[i]->remove(blk_accessor_);
                if (child_[i]) {
                    delete child_[i];
                    child_[i] = 0;
                    child_rep_[i] = -1;
                }
            }
            delete self_ref_;
            self_rep_ = -1;
        }

        bool insert(const K &key, const V &val) {
            const int insert_position = locate_child_index(key);
            Node<K, V> *targetNode = get_child_reference(insert_position)->get(blk_accessor_);
            bool ret = targetNode->insert(key, val);
            get_child_reference(locate_child_index(key))->close(blk_accessor_);
            return ret;
        }

        bool search(const K &k, V &v) {
            const int index = locate_child_index(k);
            if (index < 0) return false;
            Node<K, V> *targeNode = get_child_reference(index)->get(blk_accessor_);
            bool ret = targeNode->search(k, v);
            child_[index]->close(blk_accessor_, READONLY);
            return ret;
        }


        bool locate_key(const K &k, node_reference<K, V> *&child, int &index) {
            int local_index = locate_child_index(k);
            if (local_index < 0) return false;
            Node<K, V> *targeNode = get_child_reference(local_index)->get(blk_accessor_);
            bool ret = targeNode->locate_key(k, child, index);
            child_[local_index]->close(blk_accessor_);
            return ret;
        }

        bool update(const K &k, const V &v) {
            return true;
        }

        bool delete_key(const K &k) {

        }

        bool delete_key(const K &k, bool &underflow) {
            int child_index = locate_child_index(k);
            if (child_index < 0)
                return false;

            Node<K, V> *child = get_child_reference(child_index)->get(blk_accessor_);
            bool deleted = child->delete_key(k, underflow);
            if (!deleted) {
                child_[child_index]->close(blk_accessor_);
                return false;
            }

            if (!underflow || size_ < 2) {
                child_[child_index]->close(blk_accessor_);
                return true;
            }

            Node<K, V> *left_child, *right_child;
            int left_child_index, right_child_index;
            int deleted_child_index = -1;
            K boundary;
            if (child_index >= 1) {
                left_child_index = child_index - 1;
                right_child_index = child_index;
            } else {
                left_child_index = child_index;
                right_child_index = child_index + 1;
            }
            left_child = get_child_ref(left_child_index)->get(blk_accessor_);
            right_child = get_child_ref(right_child_index)->get(blk_accessor_);


            // try to borrow an entry from the left. If no additional entry is available in the left, the two nodes will
            // be merged with the right one being deleted.
            bool merged = left_child->balance(right_child, boundary);
            this->mark_modified();

            if (!merged) {
                // if borrowed (not merged), update the boundary
                key_[right_child_index] = boundary;
                underflow = false;
                get_child_reference(left_child_index)->close(blk_accessor_);
                get_child_reference(right_child_index)->close(blk_accessor_);
                return true;
            }

            // merged
            get_child_reference(left_child_index)->close(blk_accessor_);

//            child_[right_child_index]->close(blk_accessor_);
            child_[right_child_index]->remove(blk_accessor_); // a potential bug here, because the instance of the right node may be freed, in the balance function.
            delete child_[right_child_index];
            child_[right_child_index] = 0;
            child_rep_[right_child_index] = -1;


            // remove the reference to the deleted child, i.e., right_child
            for (int i = right_child_index + 1; i < size_; ++i) {
                this->key_[i - 1] = this->key_[i];
                this->child_rep_[i - 1] = this->child_rep_[i];
                this->child_[i - 1] = this->child_[i];
            }
            --this->size_;

            underflow = this->size_ < UNDERFLOW_BOUND(CAPACITY);

            return true;
        }

        virtual bool balance(Node<K, V> *sibling_node, K &boundary) {
            this->mark_modified();
            sibling_node->mark_modified();
            const int underflow_bound = UNDERFLOW_BOUND(CAPACITY);
            InnerNode<K, V, CAPACITY> *right = static_cast<InnerNode<K, V, CAPACITY> *>(sibling_node);
            if (this->size_ < underflow_bound) {
                if (right->size_ > underflow_bound) {
                    // this node will borrow one child node from the right sibling node.
                    this->key_[this->size_] = right->key_[0];
                    this->child_rep_[this->size_] = right->child_rep_[0];
                    this->child_[this->size_] = nullptr;
                    ++this->size_;

                    // remove the involved child in the right sibling node
                    delete right->child_[0];
                    for (int i = 0; i < right->size_; ++i) {
                        right->key_[i] = right->key_[i + 1];
                        right->child_rep_[i] = right->child_rep_[i + 1];
                        right->child_[i] = right->child_[i + 1];
                    }
                    --right->size_;

                    // update the boundary
                    boundary = right->key_[0];
                    return false;
                }
            }

            if (right->size_ < underflow_bound) {
                if (this->size_ > underflow_bound) {
                    // make an empty slot for the entry to borrow.
                    for (int i = right->size_; i >= 0; --i) {
                        right->key_[i + 1] = right->key_[i];
                        right->child_rep_[i + 1] = right->child_rep_[i];
                        right->child_[i + 1] = right->child_[i];
                    }
//                    delete right->child_[0];
                    ++right->size_;

                    // copy the entry
                    right->key_[0] = this->key_[this->size_ - 1];
                    right->child_rep_[0] = this->child_rep_[this->size_ - 1];
                    right->child_[0] = this->child_[this->size_ - 1];;
                    this->child_rep_[this->size_ - 1] = -1;
                    this->child_[this->size_ - 1] = nullptr;

                    --this->size_;

                    // update the boundary
                    boundary = right->key_[0];
                    return false;
                }
            }

            // otherwise, two nodes get merged.
            for (int l = this->size_, r = 0; r < right->size_; ++l, ++r) {
                this->key_[l] = right->key_[r];
                this->child_rep_[l] = right->child_rep_[r];
                this->child_[l] = right->child_[r];
            }
            this->size_ += right->size_;
            right->size_ = 0;
//            delete right;
            return true;
        }

        void insert_inner_node(Node<K, V> *innerNode, K boundary_key, int insert_position) {
            this->mark_modified();
            // make room for insertion.
            for (int i = size_ - 1; i >= insert_position; --i) {
                key_[i + 1] = key_[i];
                child_rep_[i + 1] = child_rep_[i];
                child_[i + 1] = child_[i];
            }

            key_[insert_position] = boundary_key;
//            child_[insert_position] = new in_memory_node_ref<K, V>();
//            child_[insert_position]->copy(innerNode->get_self_ref());

//            child_[insert_position] = blk_accessor_->create_null_ref();
//            child_[insert_position]->copy(innerNode->get_self_ref());
            child_rep_[insert_position] = innerNode->get_self_ref()->get_unified_representation();
            child_[insert_position] = nullptr;

            ++size_;
        }

        bool insert_with_split_support(const K &key, const V &val, Split<K, V> &split) {
            int target_node_index = locate_child_index(key);
            const bool exceed_left_boundary = target_node_index < 0;
            target_node_index = std::max(0, target_node_index);
            Split<K, V> local_split;

            Node<K, V>* target_child_instance;

            node_reference<K, V>* node_ref;
            // Insert into the target leaf node.
            bool is_split;
            if (exceed_left_boundary) {
                key_[0] = key;
                this->mark_modified();
            }
            node_ref = get_child_reference(target_node_index);
            target_child_instance = node_ref->get(blk_accessor_);
            is_split = target_child_instance->insert_with_split_support(key, val, local_split);

            // The tuple was inserted without causing leaf node split.
            if (!is_split) {
                node_ref->close(blk_accessor_);
                return false;
            }

            this->mark_modified();

            // The child node was split, but the current node has free slot.
            if (size_ < CAPACITY) {
                insert_inner_node(local_split.right, local_split.boundary_key,
                                  target_node_index + 1);
                local_split.left->get_self_ref()->close(blk_accessor_);
                local_split.right->get_self_ref()->close(blk_accessor_);
                return false;
            }

            // child node was split but the current node is full. So we split the current node.
            bool insert_to_first_half = target_node_index < CAPACITY / 2;

            //
            int start_index_for_right = CAPACITY / 2;
            InnerNode<K, V, CAPACITY> *left = this;
            InnerNode<K, V, CAPACITY> *right = new InnerNode<K, V, CAPACITY>(blk_accessor_);
            right->mark_modified();
            node_reference<K, V>* right_ref = right->get_self_ref();

            // move the keys and children to the right node
            for (int i = start_index_for_right, j = 0; i < size_; ++i, ++j) {
                right->key_[j] = key_[i];
                right->child_rep_[j] = child_rep_[i];
                delete left->child_[i];
                left->child_[i] = 0;
            }

            const int moved = size_ - start_index_for_right;
            left->size_ -= moved;
            right->size_ = moved;

            // insert the new child node to the appropriate split node.
            InnerNode<K, V, CAPACITY> *host_for_node = insert_to_first_half ? left : right;
            int inner_node_insert_position = host_for_node->locate_child_index(local_split.boundary_key);
            host_for_node->insert_inner_node(local_split.right, local_split.boundary_key,
                                             inner_node_insert_position + 1);



            // write the remaining content in the split data structure.
            split.left = (left);
            split.right = (right);
            split.boundary_key = right->get_leftmost_key();
//            right_ref->close(blk_accessor_);
            local_split.left->get_self_ref()->close(blk_accessor_);
            local_split.right->get_self_ref()->close(blk_accessor_);
            return true;
        }

        node_reference<K, V> *get_leftmost_leaf_node() {
            node_reference<K, V> * ret = get_child_reference(0)->get(blk_accessor_)->get_leftmost_leaf_node();
            child_[0]->close(blk_accessor_);
            return ret;
        }

//        node_reference<K, V>* get_leftmost_leaf_node_ref() {
//            node_reference<K, V>* ret = child_[0]->get(blk_accessor_)->get_leftmost_leaf_node_ref();
//            child_[0]->close();
//            return ret;
//        };

        bool has_free_slot() const {
            return size_ < CAPACITY;
        }

        std::string toString() {
//        return std::to_string(this->id) + ": " + keys_to_string() + " " + nodes_to_string(); // for debug
            std::string key_string = keys_to_string();
            std::string node_string = nodes_to_string();
            return key_string + " " + node_string;
        }

        std::string keys_to_string() const {
            std::stringstream ss;
            for (int i = 1; i < size_; ++i) {
                ss << key_[i];
                if (i < size_ - 1)
                    ss << " ";
            }
            return ss.str();
        }

        std::string nodes_to_string() {
            std::stringstream ss;
            for (int i = 0; i < size_; ++i) {
                node_reference<K, V>* node_reference = get_child_reference(i);
                Node<K, V> *node = node_reference->get(blk_accessor_);
                ss << "[" << node->toString() << "]";
                node_reference->close(blk_accessor_);
                if (i != size_ - 1) {
                    ss << " ";
                }
            }
            return ss.str();
        }

        const K get_leftmost_key() {
//            K ret = child_[0]->get(blk_accessor_)->get_leftmost_key();
//            child_[0]->close(blk_accessor_);
//            return ret;
            return key_[0];
        }

        NodeType type() const {
            return INNER;
        }

        int size() const {
            return size_;
        }

        void close() {
            for (int i = 0; i < size_; i++) {
                get_child_ref(i)->get(this->blk_accessor_);
                get_child_ref(i)->remove(this->blk_accessor_);
            }
        }

        friend std::ostream &operator<<(std::ostream &os, InnerNode<K, V, CAPACITY> const &m) {
            for (int i = 0; i < m.size_; ++i) {
                os << "[" << m.get_child_reference(i)->toString() << "]";
                m.child_[i].close();
                if (i != m.size_ - 1)
                    os << " ";
            }
            return os;
        }

        void serialize(void* buffer) {

            uint64_t start = ticks();

            char* write_offset = static_cast<char*>(buffer);

            // write node type info
            *(reinterpret_cast<uint32_t*>(write_offset)) = INNER_NODE;
            write_offset += sizeof(uint32_t);

            // write size
            *(reinterpret_cast<uint32_t*>(write_offset)) = size_;
            write_offset += sizeof(uint32_t);

            // write self_ref_
            *(reinterpret_cast<int64_t*>(write_offset)) = self_rep_;
            write_offset += sizeof(int64_t);

            // write valid keys
            memcpy(write_offset, &key_, size_ * sizeof(K));
            write_offset += size_ * sizeof(K);

            // write valid child references.
            memcpy(write_offset, &child_rep_[0], size_ * sizeof(blk_address));
            write_offset += size_ * sizeof(blk_address);
//            for (int i = 0; i < size_; i++) {
////                int64_t value = child_[i]->get_unified_representation();
////                * reinterpret_cast<int64_t*>(write_offset) = value;
//                * reinterpret_cast<int64_t*>(write_offset) = child_rep_[i];
//                write_offset += sizeof(int64_t);
//            }
            assert(write_offset - static_cast<char*>(buffer) <= blk_accessor_->block_size);
//            printf("%.2f ns to serialize inner node.\n", cycles_to_nanoseconds(ticks() - start));
        }

        void deserialize(void* buffer) {
            uint64_t start = ticks();
            char* read_offset = static_cast<char*>(buffer);

            // skip type
            read_offset += sizeof(uint32_t);

            // restore size
            size_ = *reinterpret_cast<uint32_t*>(read_offset);
            read_offset += sizeof(uint32_t);

            // restore self_ref_
            int64_t value = * reinterpret_cast<int64_t*>(read_offset);
            read_offset += sizeof(int64_t);
//            self_ref_->restore_by_unified_representation(value);
//            self_ref_->bind(this);
            self_rep_ = value;
            self_ref_ = nullptr;

            // restore valid keys
            memcpy(&key_, read_offset, size_ * sizeof(K));
            read_offset += size_ * sizeof(K);

            // restore valid child references
            memcpy(child_rep_, read_offset, size_ * sizeof(blk_address));
            read_offset += size_ * sizeof(blk_address);
//            for (int i = 0; i < size_; i++) {
//                int64_t value = * reinterpret_cast<int64_t*>(read_offset);
//                read_offset += sizeof(int64_t);
////                get_child_reference(i) = blk_accessor_->create_null_ref();
////                child_[i]->restore_by_unified_representation(value);
//                child_rep_[i] = value;
//            }
//            printf("%.2f ns to deserialize inner node.\n", cycles_to_nanoseconds(ticks() - start));
        }

        node_reference<K, V>* get_self_ref() {
            if (!self_ref_) {
                self_ref_ = blk_accessor_->create_null_ref();
                self_ref_->restore_by_unified_representation(self_rep_);
                self_ref_->bind(this);
            }
            return self_ref_;
        };

        int64_t get_self_rep() {
            return self_rep_;
        }

        node_reference<K, V>* get_child_ref(int i) {
            if (!child_[i]) {
                child_[i] = blk_accessor_->create_null_ref();
                child_[i]->restore_by_unified_representation(child_rep_[i]);
            }
            return child_[i];
        };

        void set_blk_accessor(blk_accessor<K, V>* blk_accessor) {
            blk_accessor_ = blk_accessor;
        }

        // Locate the node that might contain the particular key.
        int locate_child_index(K key) const {
            if (size_ == 0)
                return -1;
            int l = 0, r = size_ - 1;
            int m;
            bool found = false;
            while(l <= r) {
                m = (l + r) >> 1;
                if (key_[m] < key) {
                    l = m + 1;
                } else if (key < key_[m]) {
                    r = m - 1;
                } else {
                    found = true;
                    break;
                }
            }

            if (found) {
                return m;
            } else {
                return l -  1;
            }
        }

        node_reference<K, V>* get_child_reference(int index) {
            return get_child_ref(index);
        };

    public:
        K key_[CAPACITY]; // key_[0] is the smallest key for this inner node. The key boundaries start from index 1.
        int size_;
        blk_address child_rep_[CAPACITY];
        node_reference<K, V>* child_[CAPACITY];

    protected:
        blk_address self_rep_;
        blk_accessor<K, V>* blk_accessor_;
    private:
        node_reference<K, V>* self_ref_;

    private:
        void initialize_child_refs() {
            for (int i = 0 ; i < CAPACITY; i++) {
                child_[i] = nullptr;
                child_rep_[i] = -1;
            }
        }

    };
}
#endif //B_TREE_INNER_NODE_H
