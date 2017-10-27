//
// Created by Li Wang on 10/27/17.
//

#ifndef NVM_BLK_INNER_H
#define NVM_BLK_INNER_H

#include "inner_node.h"
#include "../blk/blk.h"

namespace tree {
    template<typename K, typename V, int CAPACITY>
    class BlkInner : public InnerNode<K, V, CAPACITY> {
    public:
        BlkInner(blk_address address, blk_accessor* accessor): blk_address_(address), blk_accessor_(accessor) {};


        bool insert_with_split_support(const K &key, const V &val, Split<K, V> &split) {
            bool is_split = InnerNode<K, V, CAPACITY>::insert_with_split_support(key, val, split);
            if (is_split) {
                blk_address right_sibling_address = blk_accessor_->allocate();
                void* write_buffer = malloc(blk_accessor_->block_size);
                serialize(write_buffer);
                blk_accessor_->write(right_sibling_address, write_buffer);
                delete write_buffer;
                split.left = (Node<K, V>*)blk_address_;
                split.right = (Node<K, V>*)right_sibling_address;
            }
        }
    private:
        blk_address blk_address_;
        blk_accessor* blk_accessor_;
    };
}
#endif //NVM_BLK_INNER_H
