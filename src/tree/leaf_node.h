//
// Created by Li Wang on 6/1/17.
//

#ifndef B_PLUS_TREE_LEAFNODE_H
#define B_PLUS_TREE_LEAFNODE_H

#include <sstream>
#include <iostream>
#include <string>
#include <memory.h>
#include "node.h"
#include "node_reference.h"
#include "in_memory_node_reference.h"
#include "../blk/blk.h"

namespace tree {

    template<typename K, typename V, int CAPACITY>
    class VanillaBPlusTree;

    template<typename K, typename V, int CAPACITY>
    class LeafNode : public Node<K, V> {
        friend class VanillaBPlusTree<K, V, CAPACITY>;

    public:
        struct Entry {
            K key;
            V val;

            Entry() {};

            Entry(K k, V v) : key(k), val(v) {};

            Entry &operator=(const Entry &r) {
                this->key = r.key;
                this->val = r.val;
                return *this;
            }
            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive & ar, const unsigned int version) {
                ar & key & val;
            }
        };

    public:

        LeafNode(blk_accessor<K, V, CAPACITY>* blk_accessor = 0) : size_(0), right_sibling_(0), blk_accessor_(blk_accessor) {
            if (blk_accessor) {
                self_ref_ = blk_accessor_->allocate_ref();
                self_ref_->bind(this);
            } else {
                self_ref_ = 0;
            }
        };

        ~LeafNode() {
            delete right_sibling_;
            delete self_ref_;
        }

        bool insert(const K &key, const V &val) {

            int insert_position;
            const bool found = search_key_position(key, insert_position);

            if (found) {
                // update the entry.
                entries_[insert_position].val = val;
                return true;
            } else {

                if (size_ >= CAPACITY) {
                    return false;
                }

                // make an empty slot for new entry
                for (int i = size_ - 1; i >= insert_position; i--) {
                    entries_[i + 1] = entries_[i];
                }

                // insert the new entry.
                entries_[insert_position] = Entry(key, val);
                size_++;
                return true;
            }
        }

        bool insert_with_split_support(const K &key, const V &val, Split<K, V> &split) {
            int insert_position;
            const bool found = search_key_position(key, insert_position);

            if (found) {
                // update the entry.
                entries_[insert_position].val = val;
                return false;
            }

            if (size_ < CAPACITY) {
                // make an empty slot for the new entry
                for (int i = size_ - 1; i >= insert_position; i--) {
                    entries_[i + 1] = entries_[i];
                }

                // insert the new entry.
                entries_[insert_position] = Entry(key, val);
                size_++;
                return false;
            } else {

                // split
                bool insert_to_first_half = insert_position < CAPACITY / 2;

                int entry_index_for_right_node = CAPACITY / 2;
                LeafNode<K, V, CAPACITY> *const left = this;
                LeafNode<K, V, CAPACITY> *const right = new LeafNode<K, V, CAPACITY>(blk_accessor_);

                right->update_right_sibling(left->right_sibling_);
                node_reference<K, V>* right_node_ref = right->get_self_ref();
                left->update_right_sibling(right_node_ref);
//                delete right_node_ref;

                // move entries to the right node
                for (int i = entry_index_for_right_node, j = 0; i < CAPACITY; ++i, ++j) {
                    right->entries_[j] = left->entries_[i];
                }

                const int moved = CAPACITY - entry_index_for_right_node;
                left->size_ -= moved;
                right->size_ = moved;

                // insert
                if (insert_to_first_half)
                    left->insert(key, val);
                else
                    right->insert(key, val);

                split.left = left;
                split.right = right;
                split.boundary_key = right->entries_[0].key;
                return true;
            }

        }

        bool search(const K &k, V &v) {
            int position;
            const bool found = search_key_position(k, position);
            if (found)
                v = entries_[position].val;
            return found;
        }

        bool locate_key(const K &k, Node<K, V> *&child, int &position) {
            const bool found = search_key_position(k, position);
            child = this;
            return found;
        };

        bool update(const K &k, const V &v) {
            int position;
            const bool found = search_key_position(k, position);
            if (found) {
                entries_[position].val = v;
                return true;
            } else {
                return false;
            }

        }

        bool delete_key(const K &k) {
            int position;
            const bool found = search_key_position(k, position);
            if (!found)
                return false;

            for (int i = position; i < size_ - 1; ++i) {
                entries_[i] = entries_[i + 1];
            }
            --size_;
            return true;
        }

        bool delete_key(const K &k, bool &underflow) {
            int position;
            const bool found = search_key_position(k, position);
            if (!found)
                return false;

            for (int i = position; i < size_ - 1; ++i) {
                entries_[i] = entries_[i + 1];
            }
            --size_;
            underflow = size_ < (CAPACITY + 1) / 2;
            return true;
        }

        std::string toString() const {
            std::string ret;
            std::stringstream ss;
            for (int i = 0; i < size_; i++) {
                ss << "(" << entries_[i].key << "," << entries_[i].val << ")";
                if (i != size_ - 1)
                    ss << " ";
            }
            return ss.str();
        }

        const K get_leftmost_key() {
            return entries_[0].key;
        }

        bool balance(Node<K, V> *right_sibling_node, K &boundary) {
            LeafNode<K, V, CAPACITY> *right = static_cast<LeafNode<K, V, CAPACITY> * >(right_sibling_node);
            const int underflow_bound = UNDERFLOW_BOUND(CAPACITY);
            if (size_ < underflow_bound) {
                // this node under-flows
                if (right->size_ > underflow_bound) {

                    // borrow an entry from the right sibling node
                    entries_[size_] = right->entries_[0];
                    ++size_;

                    // remove the entry from the right sibling node
                    for (int i = 0; i < right->size_ - 1; ++i) {
                        right->entries_[i] = right->entries_[i + 1];
                    }
                    --right->size_;

                    // update the boundary
                    boundary = right->entries_[0].key;
                    return false;
                }
            }

            if (right->size_ < underflow_bound) {
                // the right node under-flows
                if (this->size_ > underflow_bound) {

                    // make space for the entry borrowed from the left
                    for (int i = right->size_ - 1; i >= 0; --i) {
                        right->entries_[i + 1] = right->entries_[i];
                    }

                    // copy the entry and increase the size by 1
                    right->entries_[0] = this->entries_[size_ - 1];
                    ++right->size_;

                    // remove the entry from the left by reducing the size
                    --this->size_;

                    // update the boundary
                    boundary = right->entries_[0].key;
                    return false;
                }
            }


            // the sibling node has no additional entry to borrow. We merge the nodes.
            // move all the entries from the right to the left
            for (int l = this->size_, r = 0; r < right->size_; ++l, ++r) {
                this->entries_[l] = right->entries_[r];
            }
            this->size_ += right->size_;
            right->size_ = 0;
            this->update_right_sibling(right->right_sibling_);

            // delete the right
            delete right;
            return true;
        }

        Node<K, V> *get_leftmost_leaf_node() {
            return this;
        }
//
//        node_reference<K, V>* get_leftmost_leaf_node_ref() {
//            return new in_memory_node_ref(this);
//        };

        NodeType type() const {
            return LEAF;
        }

        int size() const {
            return size_;
        }

        void serialize(void* buffer) {
            *(static_cast<uint32_t*>(buffer)) = LEAF_NODE;

            // write size
            *(static_cast<uint32_t*>(buffer) + 1) = size_;

            // copy entries
            memcpy((char*)buffer + sizeof(uint32_t) * 2, &entries_, CAPACITY * sizeof(Entry));
        }

        void deserialize(void* buffer) {
            // restore size
            size_ = *(static_cast<uint32_t*>(buffer) + 1);

            // restore entries
            memcpy(&entries_, (char*)buffer + sizeof(uint32_t) * 2, CAPACITY * sizeof(Entry));
        }

        friend std::ostream &operator<<(std::ostream &os, LeafNode<K, V, CAPACITY> const &m) {
            for (int i = 0; i < m.size_; i++) {
                os << "(" << m.entries_[i].key << "," << m.entries_[i].val << ")";
                if (i != m.size_ - 1)
                    os << " ";
            }
            return os;
        }

        void update_right_sibling(node_reference<K, V>* new_ref) {
            if (!right_sibling_) {
                right_sibling_ = blk_accessor_->create_null_ref();
//                right_sibling_ = new in_memory_node_ref<K, V>(0);
            }
            if (new_ref)
                right_sibling_->copy(new_ref);
        }

        node_reference<K, V>* get_self_ref() const {
            return self_ref_;
        };

    protected:
        bool getEntry(int i, K &k, V &v) const {
            if (i >= size_)
                return false;
            k = entries_[i].key;
            v = entries_[i].val;
            return true;
        }

    private:

        bool search_key_position(const K &key, int &position) const {

            int l = 0, r = size_ - 1;
            int m = 0;
            bool found = false;
            while (l <= r) {
                m = (l + r) >> 1;
                if (entries_[m].key < key) {
                    l = m + 1;
                } else if (entries_[m].key == key) {
                    position = m;
                    return true;
                } else {
                    r = m - 1;
                }
            }
            position = l;
            return false;
        }

    protected:
        /**
         * TODO: store the keys and the values separately, to improve data access locality.
         */
        Entry entries_[CAPACITY];
        int size_;
        node_reference<K, V> *right_sibling_;
        node_reference<K, V> *self_ref_;
        blk_accessor<K, V, CAPACITY>* blk_accessor_;
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & boost::serialization::base_object<Node<K, V>>(*this) & size_ & entries_ & right_sibling_ & self_ref_;
        }
    };
}

#endif //B_PLUS_TREE_LEAFNODE_H
