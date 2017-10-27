//
// Created by Li Wang on 10/25/17.
//

#ifndef NVM_BLK_INNER_NODE_H
#define NVM_BLK_INNER_NODE_H

#include <memory.h>
#include <assert.h>
#include "../blk/blk.h"
#include "inner_node.h"
#include "blk_node.h"

namespace tree{


    template<typename K, typename V, int CAPACITY>
    class BlkInnerNode: public BlkNode<K, V, CAPACITY> {
    public:
        BlkInnerNode(blk_address blk_address, blk_accessor* blk_accessor): BlkNode<K, V, CAPACITY>(blk_address, blk_accessor) {
            this->node_ = new InnerNode<K, V, CAPACITY>();
        };

        BlkInnerNode(blk_address blk_address, blk_accessor* blk_accessor, Node<K, V>* node): BlkNode<K, V, CAPACITY>(blk_address, blk_accessor, node){

        };

        blk_address get_blk_address() const {
            return this->blk_address_;
        }

        bool insert_with_split_support(const K& key, const V& value, Blk_Split<K, V>& blk_split) {
            Split<K, V> split;
            bool is_split = this->node_->insert_with_split_support(key, value, split);
            if (is_split) {
                blk_address new_blk_address = this->blk_accessor_->allocate();
                BlkInnerNode* right_inner_node = new BlkInnerNode(new_blk_address, this->blk_accessor_, split.right);
                blk_split.right = right_inner_node->get_blk_address();
                right_inner_node->flush();
                delete right_inner_node;
            }
            blk_split.left = this->get_blk_address();
            this->flush();
            return is_split;
        };
    };
}
#endif //NVM_BLK_INNER_NODE_H
