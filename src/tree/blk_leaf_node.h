//
// Created by Li Wang on 10/25/17.
//

#ifndef NVM_BLK_LEAF_NODE_H
#define NVM_BLK_LEAF_NODE_H

#include <memory.h>
#include <assert.h>
#include "../blk/blk.h"
#include "leaf_node.h"
#include "blk_node.h"

namespace tree{
    template<typename K, typename V, int CAPACITY>
    class BlkLeafNode: public BlkNode<K, V, CAPACITY> {
    public:
        BlkLeafNode(blk_address blk_address, blk_accessor* blk_accessor): BlkNode<K, V, CAPACITY>(blk_address, blk_accessor) {
            this->node_ = new LeafNode<K, V, CAPACITY>();
        };

        BlkLeafNode(blk_address blk_address, blk_accessor* blk_accessor, Node<K, V>* node): BlkNode<K, V, CAPACITY>(blk_address, blk_accessor, node){

        };

        blk_address get_blk_address() const {
            return this->blk_address_;
        }

        bool insert_with_split_support(const K& key, const V& value, Blk_Split<K, V>& blk_split) {
            Split<K, V> split;
            bool is_split = this->node_->insert_with_split_support(key, value, split);
            if (is_split) {
                blk_address new_blk_address = this->blk_accessor_->allocate();
                BlkLeafNode* right_leaf_node = new BlkLeafNode(new_blk_address, this->blk_accessor_, split.right);
                blk_split.right = right_leaf_node->get_blk_address();
                right_leaf_node->flush();
                delete right_leaf_node;
            }
            blk_split.left = this->get_blk_address();
            this->flush();
            return is_split;
        };
    };
}
#endif //NVM_BLK_LEAF_NODE_H
