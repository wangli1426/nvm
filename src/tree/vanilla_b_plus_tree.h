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

template <typename K, typename V>
class blk_accessor;

namespace tree {


    template<typename K, typename V, int CAPACITY>
    class VanillaBPlusTree : public BTree<K, V> {
    public:
        VanillaBPlusTree(blk_accessor<K, V>* blk_accessor = 0): blk_accessor_(blk_accessor) {
        }

        virtual ~VanillaBPlusTree() {
            close();
            delete blk_accessor_;
            blk_accessor_ = 0;
        }

        virtual void clear() {
            close();
            init();
        }

        void close() {
            if (root_) {
                Node<K, V> *root_instance = root_->get(blk_accessor_);
                root_->close(blk_accessor_);
//            delete root_instance;
                delete root_;
                root_ = 0;
            }
            blk_accessor_->close();
        }

        // Insert a k-v pair to the tree.
        void insert(const K &k, const V &v) {
            Split<K, V> split;
            bool is_split;
            Node<K, V>* root_instance = root_->get(blk_accessor_);
            is_split = root_instance->insert_with_split_support(k, v, split);
            if (is_split) {
                InnerNode<K, V, CAPACITY> *new_inner_node = new InnerNode<K, V, CAPACITY>(split.left, split.right, blk_accessor_);
                new_inner_node->mark_modified();
                root_->copy(new_inner_node->get_self_ref());// the root_ reference which originally referred to a
                                                            // a leaf will refer to a inner node now. TODO: release the old root_
                                                            // reference and create a new one.
                root_->bind(new_inner_node);

                split.left->get_self_ref()->close(blk_accessor_);
                split.right->get_self_ref()->close(blk_accessor_);


//                delete root_;
//                root_ = new in_memory_node_ref<K, V>(new_inner_node);
                ++depth_;
            }
            root_->close(blk_accessor_);
            blk_accessor_->flush();
        }

        // Delete the entry from the tree. Return true if the key exists.
        bool delete_key(const K &k) {
            bool underflow = false;
            Node<K, V>* root_node = root_->get(blk_accessor_);
            bool ret = root_node->delete_key(k, underflow);
            if (underflow && root_node->type() == INNER && root_node->size() == 1) {
//                InnerNode<K, V, CAPACITY> *widow_inner_node = static_cast<InnerNode<K, V, CAPACITY> *>(root_node);
//                root_node = widow_inner_node->child_[0]->get(blk_accessor_);
//                delete widow_inner_node->child_[0];
//                widow_inner_node->size_ = 0;
//
//                delete widow_inner_node;
//                root_->remove(blk_accessor_);
//
//                // refer to the updated root_node
//                root_->copy(root_node->get_self_ref());
//                --depth_;


                node_reference<K, V>* single_child_ref = blk_accessor_->create_null_ref();
                single_child_ref->copy(static_cast<InnerNode<K, V, CAPACITY> *>(root_node)->child_[0]);
                static_cast<InnerNode<K, V, CAPACITY> *>(root_node)->child_[0]->copy(0);
                root_->remove(blk_accessor_);
                delete root_;
                root_ = single_child_ref;
                --depth_;

            }
            root_->close(blk_accessor_);
            blk_accessor_->flush();
            return ret;
        }

        // Search for the value associated with the given key. If the key was found, return true and the value is stored
        // in v.
        bool search(const K &k, V &v) {
            bool ret = root_->get(blk_accessor_)->search(k, v);
            root_->close(blk_accessor_, READONLY);
            return ret;
        }

        // Return the string representation of the tree.
        std::string toString() const {
            return root_->get(blk_accessor_)->toString();
        }

        friend std::ostream &operator<<(std::ostream &os, VanillaBPlusTree<K, V, CAPACITY> const &m) {
            return os << m.root_->toString();
        }

        typename BTree<K, V>::Iterator *get_iterator() {
            node_reference<K, V> *leftmost_leaf_node = root_->get(blk_accessor_)->get_leftmost_leaf_node();

            return new Iterator(leftmost_leaf_node, 0, blk_accessor_);
        }

        void init() {
            if (!blk_accessor_)
                blk_accessor_ = new void_blk_accessor<K, V, CAPACITY>(512);
            Node<K, V>* leaf_node = new LeafNode<K, V, CAPACITY>(blk_accessor_);
            leaf_node->mark_modified();
            node_reference<K, V>* leaf_node_ref = leaf_node->get_self_ref();
            root_ = blk_accessor_->allocate_ref();
            root_->copy(leaf_node_ref);

            leaf_node_ref->close(blk_accessor_);
            blk_accessor_->flush();
//            root_ = leaf_node->get_self_ref();
//            root_->close(blk_accessor_);
            depth_ = 1;
        }

        typename BTree<K, V>::Iterator *range_search(const K &key_low, const K &key_high) {
            node_reference<K, V> *leaf_node_ref;
            int offset;
            root_->get(blk_accessor_)->locate_key(key_low, leaf_node_ref, offset);
            typename BTree<K, V>::Iterator * ret =  new Iterator(leaf_node_ref, offset, key_high, blk_accessor_);
            root_->close(blk_accessor_);
            return ret;
        };

        class Iterator : public BTree<K, V>::Iterator {
        public:
            Iterator(node_reference<K, V> *leaf_node_ref, int offset, blk_accessor<K, V>* blk_accessor):
                    offset_(offset), upper_bound_(false), blk_accessor_(blk_accessor), leaf_node_ref_(leaf_node_ref) {
            };

            Iterator(node_reference<K, V> *leaf_node_ref, int offset, K key_high, blk_accessor<K, V>* blk_accessor)
                    : offset_(offset), blk_accessor_(blk_accessor), upper_bound_(true), key_high_(key_high),
                      leaf_node_ref_(leaf_node_ref) {
            };

            ~Iterator() {
                delete leaf_node_ref_;
            }

            virtual bool next(K &key, V &val) {
                if (!leaf_node_ref_)
                    return false;
                LeafNode<K, V, CAPACITY>* leaf_node = dynamic_cast<LeafNode<K, V, CAPACITY>*>(leaf_node_ref_->get(blk_accessor_));
                if (leaf_node->getEntry(offset_, key, val)) {
                    offset_++;
                    leaf_node_ref_->close(blk_accessor_);
                    return upper_bound_ ? key < key_high_ || key == key_high_ : true;
                } else if (leaf_node->right_sibling_ != 0 && !leaf_node->right_sibling_->is_null_ptr()) {
                    leaf_node_ref_->close(blk_accessor_);
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
            blk_accessor<K, V>* blk_accessor_;
            int offset_;
            bool upper_bound_;
            K key_high_;
        };
    protected:
        virtual void set_blk_accessor() {
            blk_accessor_ = new void_blk_accessor<K, V, CAPACITY>(512);
        }
    private:

    protected:
        node_reference<K, V>* root_;
        int depth_;
        blk_accessor<K, V>* blk_accessor_;
    };
}
#endif //B_PLUS_TREE_BPLUSTREE_H
