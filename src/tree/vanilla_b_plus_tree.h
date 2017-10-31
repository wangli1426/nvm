//
// Created by Li Wang on 6/1/17.
//

#ifndef B_PLUS_TREE_BPLUSTREE_H
#define B_PLUS_TREE_BPLUSTREE_H

#include <iostream>
#include "leaf_node.h"
#include "b_tree.h"
#include "inner_node.h"
#include "node.h"
#include "in_memory_node_reference.h"
#include "../blk/void_blk_accessor.h"

template <typename K, typename V, int CAPACITY>
class blk_accessor;

namespace tree {


    template<typename K, typename V, int CAPACITY>
    class VanillaBPlusTree : public BTree<K, V> {
    public:
        VanillaBPlusTree() {
            set_blk_accessor();
            init();
        }

        ~VanillaBPlusTree() {
            root_->close();
            delete root_->get();
            delete root_;
            delete blk_accessor_;
        }

        void clear() {
            root_->close();
            delete root_->get();
            delete root_;
            init();
        }

        // Insert a k-v pair to the tree.
        void insert(const K &k, const V &v) {
            Split<K, V> split;
            bool is_split;

            is_split = root_->get()->insert_with_split_support(k, v, split);
            if (is_split) {
                InnerNode<K, V, CAPACITY> *new_inner_node = new InnerNode<K, V, CAPACITY>(split.left, split.right, blk_accessor_);
                root_->copy(new_inner_node->get_self_ref());
//                delete root_;
//                root_ = new in_memory_node_ref<K, V>(new_inner_node);
                ++depth_;
            }

        }

        // Delete the entry from the tree. Return true if the key exists.
        bool delete_key(const K &k) {
            bool underflow = false;
            Node<K, V>* root_node = root_->get();
            bool ret = root_node->delete_key(k, underflow);
            if (underflow && root_node->type() == INNER && root_node->size() == 1) {
                InnerNode<K, V, CAPACITY> *widow_inner_node = static_cast<InnerNode<K, V, CAPACITY> *>(root_node);
                root_node = widow_inner_node->child_[0]->get();
                delete widow_inner_node->child_[0];
                widow_inner_node->size_ = 0;

                delete widow_inner_node;
                root_->remove();

                // refer to the updated root_node
                root_->copy(root_node->get_self_ref());
//                delete root_;
//                root_ = new in_memory_node_ref<K, V>(root_node);
                --depth_;
            }
            return ret;
        }

        // Search for the value associated with the given key. If the key was found, return true and the value is stored
        // in v.
        bool search(const K &k, V &v) {
            bool ret = root_->get()->search(k, v);
            root_->close();
            return ret;
        }

        // Return the string representation of the tree.
        std::string toString() const {
            return root_->get()->toString();
        }

        friend std::ostream &operator<<(std::ostream &os, VanillaBPlusTree<K, V, CAPACITY> const &m) {
            return os << m.root_->toString();
        }

        typename BTree<K, V>::Iterator *get_iterator() {
            LeafNode<K, V, CAPACITY> *leftmost_leaf_node =
                    dynamic_cast<LeafNode<K, V, CAPACITY> *>(root_->get()->get_leftmost_leaf_node());

            return new Iterator(leftmost_leaf_node, 0);
        }

        typename BTree<K, V>::Iterator *range_search(const K &key_low, const K &key_high) {
            Node<K, V> *leaf_node;
            int offset;
            root_->get()->locate_key(key_low, leaf_node, offset);
            return new Iterator(dynamic_cast<LeafNode<K, V, CAPACITY> *>(leaf_node), offset, key_high);
        };

        class Iterator : public BTree<K, V>::Iterator {
        public:
            Iterator(LeafNode<K, V, CAPACITY> *leaf_node, int offset) : offset_(offset),
                                                                        upper_bound_(false) {
                if (leaf_node) {
                    leaf_node_ref_ = new in_memory_node_ref<K, V>(0);
                    leaf_node_ref_->copy(leaf_node->get_self_ref());
                } else
                    leaf_node_ref_ = 0;
            };

            Iterator(LeafNode<K, V, CAPACITY> *leaf_node, int offset, K key_high) : offset_(offset),
                                                                                    upper_bound_(true),
                                                                                    key_high_(key_high) {
                if (leaf_node) {
                    leaf_node_ref_ = new in_memory_node_ref<K, V>(0);
                    leaf_node_ref_->copy(leaf_node->get_self_ref());
                } else
                    leaf_node_ref_ = 0;
            };

            ~Iterator() {
                delete leaf_node_ref_;
            }

            virtual bool next(K &key, V &val) {
                if (!leaf_node_ref_)
                    return false;
                LeafNode<K, V, CAPACITY>* leaf_node = dynamic_cast<LeafNode<K, V, CAPACITY>*>(leaf_node_ref_->get());
                if (leaf_node->getEntry(offset_, key, val)) {
                    offset_++;
                    leaf_node_ref_->close();
                    return upper_bound_ ? key < key_high_ || key == key_high_ : true;
                } else if (leaf_node->right_sibling_ != 0 && !leaf_node->right_sibling_->is_null_ptr()) {
                    leaf_node_ref_->close();
//                    delete leaf_node_ref_;
                    leaf_node_ref_->copy(leaf_node->right_sibling_);
                    offset_ = 0;
                    return next(key, val);
                } else {
                    return false;
                }
            }

        private:
            node_reference<K, V> *leaf_node_ref_;
            int offset_;
            bool upper_bound_;
            K key_high_;
        };

    protected:
        void set_blk_accessor() {
            blk_accessor_ = new void_blk_accessor<K, V, CAPACITY>(512);
        }
    private:
        void init() {
            Node<K, V>* leaf_node = new LeafNode<K, V, CAPACITY>(blk_accessor_);
            root_ = new in_memory_node_ref<K, V>();
            root_->copy(leaf_node->get_self_ref());
            depth_ = 1;
        }

    protected:
        node_reference<K, V>* root_;
        int depth_;
        blk_accessor<K, V, CAPACITY>* blk_accessor_;
    };
}
#endif //B_PLUS_TREE_BPLUSTREE_H
