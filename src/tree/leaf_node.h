//
// Created by Li Wang on 6/1/17.
//

#ifndef B_PLUS_TREE_LEAFNODE_H
#define B_PLUS_TREE_LEAFNODE_H

#include <sstream>
#include <iostream>
#include <string>
#include <memory.h>
#include <assert.h>
#include "node.h"
#include "node_reference.h"
#include "in_memory_node_reference.h"
#include "../blk/blk.h"
#include "../utils/rdtsc.h"

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
//            friend class boost::serialization::access;
//            template<class Archive>
//            void serialize(Archive & ar, const unsigned int version) {
//                ar & key & val;
//            }
        };

    public:

        LeafNode(blk_accessor<K, V>* blk_accessor = 0, bool allocate_blk_ref = true) : size_(0), right_sibling_(0), blk_accessor_(blk_accessor) {
            if (blk_accessor) {
                if (allocate_blk_ref)
                    self_ref_ = blk_accessor_->allocate_ref();
                else
                    self_ref_ = blk_accessor_->create_null_ref();
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
            this->mark_modified();
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
            this->mark_modified();
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
                right->mark_modified();
                node_reference<K, V>* right_ref = right->get_self_ref();

                right->update_right_sibling(left->right_sibling_);
                left->update_right_sibling(right_ref);
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
//                right_ref->close(blk_accessor_);
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

        bool locate_key(const K &k, node_reference<K, V> *&child, int &position) {
            const bool found = search_key_position(k, position);
            child = self_ref_;
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

            this->mark_modified();
            for (int i = position; i < size_ - 1; ++i) {
                entries_[i] = entries_[i + 1];
            }
            --size_;
            underflow = size_ < (CAPACITY + 1) / 2;
            return true;
        }

        std::string toString() {
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
            this->mark_modified();
            right_sibling_node->mark_modified();
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
//            delete right;
            return true;
        }

        node_reference<K, V> *get_leftmost_leaf_node() {
            node_reference<K, V>* ret = blk_accessor_->create_null_ref();
            ret->copy(self_ref_);

            return ret;
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
            uint64_t start = ticks();
            char* write_offset = static_cast<char*>(buffer);

            *(reinterpret_cast<uint32_t*>(write_offset)) = LEAF_NODE;
            write_offset += sizeof(uint32_t);
            // write size
            *(reinterpret_cast<uint32_t*>(write_offset)) = size_;
            write_offset += sizeof(uint32_t);

            // write self_ref_
            *(reinterpret_cast<int64_t*>(write_offset)) = self_ref_->get_unified_representation();
            write_offset += sizeof(int64_t);

            // write right_sibling_ref_;
            if (right_sibling_)
                *(reinterpret_cast<int64_t*>(write_offset)) = right_sibling_->get_unified_representation();
            else
                *(reinterpret_cast<int64_t*>(write_offset)) = -1;
            write_offset += sizeof(int64_t);

            // copy entries
            memcpy(write_offset, &entries_, size_ * sizeof(Entry));
            write_offset += size_ * sizeof(Entry);
            assert(write_offset - static_cast<char*>(buffer) <= blk_accessor_->block_size);
//            printf("%.2f ns to serialize leaf node.\n", cycles_to_nanoseconds(ticks() - start));
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
            self_ref_->restore_by_unified_representation(value);
            self_ref_->bind(this);
            read_offset += sizeof(int64_t);

            // restore right_child_ref_
            value = * reinterpret_cast<int64_t*>(read_offset);
            update_right_sibling(value);
            read_offset += sizeof(int64_t);

            // restore entries
            memcpy(&entries_, read_offset, size_ * sizeof(Entry));
//            printf("%.2f ns to deserialize leaf node.\n", cycles_to_nanoseconds(ticks() - start));
        }

        friend std::ostream &operator<<(std::ostream &os, LeafNode<K, V, CAPACITY> const &m) {
            for (int i = 0; i < m.size_; i++) {
                os << "(" << m.entries_[i].key << "," << m.entries_[i].val << ")";
                if (i != m.size_ - 1)
                    os << " ";
            }
            return os;
        }


        void update_right_sibling(int64_t value) {
            if (!right_sibling_) {
                right_sibling_ = blk_accessor_->create_null_ref();
            }
            right_sibling_->restore_by_unified_representation(value);
        }

        void update_right_sibling(node_reference<K, V>* new_ref) {
            if (!right_sibling_) {
                right_sibling_ = blk_accessor_->create_null_ref();
////                right_sibling_ = new in_memory_node_ref<K, V>(0);
            }
            if (new_ref)
                right_sibling_->copy(new_ref);
        }

        node_reference<K, V>* get_self_ref() {
            return self_ref_;
        };

        int64_t get_self_rep() {
            return self_ref_->get_unified_representation();
        }

        void set_blk_accessor(blk_accessor<K, V>* blk_accessor) {
            this->blk_accessor_ = blk_accessor;
        }

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
        blk_accessor<K, V>* blk_accessor_;
//    private:
//        friend class boost::serialization::access;
//
//        template<class Archive>
//        void save(Archive & ar, const unsigned int version) const {
//            ar & boost::serialization::base_object<Node<K, V>>(*this) & size_ & entries_ & right_sibling_ & self_ref_;
//        }
//        template<class Archive>
//        void load(Archive & ar, const unsigned int version) {
//            ar & boost::serialization::base_object<Node<K, V>>(*this) & size_ & entries_ & right_sibling_ & self_ref_;
//            self_ref_->bind(this);
//        }
//        BOOST_SERIALIZATION_SPLIT_MEMBER()
    };
}

#endif //B_PLUS_TREE_LEAFNODE_H
