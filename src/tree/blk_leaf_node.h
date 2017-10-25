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

        void serialize(void* buffer) {

//            assert(sizeof(uint32_t) * 2 + CAPACITY * sizeof(LeafNode<K, V, CAPACITY>::Entry) <= this->blk_accessor_->block_size);
//
//            // write node type
//            *static_cast<uint32_t*>(buffer) = LEAF_NODE;
//
//            LeafNode<K, V, CAPACITY> *leaf_node = dynamic_cast<LeafNode<K, V, CAPACITY>*>(this->node_);
//
//            // write size
//            *(static_cast<uint32_t*>(buffer) + 1) = leaf_node->size();
//
//            // copy entries
//            memcpy((char*)buffer + sizeof(uint32_t) * 2, &leaf_node->entries_, CAPACITY * sizeof(LeafNode<K, V, CAPACITY>::Entry));

        }

        void deserialize(void* buffer) {

//            LeafNode<K, V, CAPACITY> *leaf_node = dynamic_cast<LeafNode<K, V, CAPACITY>*>(this->node_);
//
//            // restore size
//            leaf_node->size_ = *(static_cast<uint32_t*>(buffer) + 1);
//
//            // restore entries
//            memcpy(&leaf_node->entries_, (char*)buffer + sizeof(uint32_t) * 2, CAPACITY * sizeof(LeafNode<K, V, CAPACITY>::Entry));
        }

        blk_address get_blk_address() const {
            return this->blk_address_;
        }

    };
}
#endif //NVM_BLK_LEAF_NODE_H
