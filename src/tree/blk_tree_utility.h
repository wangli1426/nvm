//
// Created by Li Wang on 10/27/17.
//

#ifndef NVM_BLK_TREE_UTILITY_H
#define NVM_BLK_TREE_UTILITY_H

#include "blk_node.h"
#include "blk_inner_node.h"
#include "blk_leaf_node.h"
namespace tree{
    template<typename K, typename V, int CAPACITY>
    BlkNode<K, V, CAPACITY>* restore(blk_accessor* accessor, const blk_address& address) {
        void* read_buffer = malloc(accessor->block_size);
        if(accessor->read(address, read_buffer) <= 0)
            return nullptr;

        uint32_t node_type = *static_cast<uint32_t*>(read_buffer);
        assert(node_type == LEAF_NODE || node_type == INNER_NODE);
        BlkNode<K, V, CAPACITY>* node;
        switch (node_type) {
            case LEAF_NODE: node = new BlkLeafNode<K, V, CAPACITY>(address, accessor);
                node->node_->deserialize(read_buffer);
                break;
            case INNER_NODE: node = new BlkInnerNode<K, V, CAPACITY>(address, accessor);
                node->node_->deserialize(read_buffer);
                break;
            default:
                assert(false); // unexpected point of execution.
        }
        return node;
    }
}
#endif //NVM_BLK_TREE_UTILITY_H
