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
        
        blk_address get_blk_address() const {
            return this->blk_address_;
        }

    };
}
#endif //NVM_BLK_LEAF_NODE_H
