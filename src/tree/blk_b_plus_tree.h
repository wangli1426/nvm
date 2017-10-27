//
// Created by Li Wang on 10/25/17.
//

#ifndef NVM_BLK_B_PLUS_TREE_H
#define NVM_BLK_B_PLUS_TREE_H
#include "../blk/blk.h"
#include "blk_node.h"
#include "blk_leaf_node.h"
#include "blk_tree_utility.h"
namespace tree {
    template <typename K, typename V, int CAPACITY>
    class BlkBPlusTree {
    public:
        BlkBPlusTree(blk_accessor* accessor): blk_accessor_(accessor){
            init();
        };

    public:
        bool insert(const K &key, const V& value) {
            BlkNode<K, V, CAPACITY> *blk_node = restore<K, V, CAPACITY>(blk_accessor_, root_);
            Blk_Split<K, V> blk_split;
            bool is_split = blk_node->insert_with_split_support(key, value, blk_split);

            bool ret = blk_node->node_->insert(key, value);
            blk_node->flush();
            delete blk_node;
            return ret;
        }

        bool search(const K& key, V& value) {
            BlkNode<K, V, CAPACITY> *blk_node = restore<K, V, CAPACITY>(blk_accessor_, root_);
            bool ret  = blk_node->node_->search(key, value);
            delete blk_node;
            return ret;
        }

    private:
        void init() {
            BlkNode<K, V, CAPACITY> *blk_node = BlkNode<K, V, CAPACITY>::create_leaf_node(blk_accessor_);
            root_ = blk_node->get_blk_address();
            blk_node->flush();
            delete blk_node;
        }
    private:
        blk_address root_;
        blk_accessor* blk_accessor_;
    };
}
#endif //NVM_BLK_B_PLUS_TREE_H
