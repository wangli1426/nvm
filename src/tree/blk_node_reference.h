//
// Created by Li Wang on 10/27/17.
//

#ifndef NVM_BLK_NODE_REFERENCE_H
#define NVM_BLK_NODE_REFERENCE_H

#include "node_reference.h"
#include "../blk/blk.h"
#include "node.h"

namespace tree {
    template<typename K, typename V>
    class blk_node_reference : public node_reference {
    public:
        blk_node_reference(blk_address blk_address, blk_accessor blk_accessor) : blk_address_(blk_address),
                                                                                 blk_accessor_(blk_accessor),
                                                                                 instance_(0) {};

        Node<K, V> *get() const {
            if (instance_)
                return instance_;
            void *read_buffer = malloc(blk_accessor_.block_size);
            blk_accessor_.read(blk_address_, read_buffer);
            uint32_t *type = read_buffer;
            switch (*type) {
                case LEAF_NODE:
                    instance_ = new BlkLeaf();
                    instance_->deserialize(read_buffer);
                    break;
                default:
                    return;
            }
            return instance_;

        };

        void close() const {
            void *write_buffer = malloc(blk_accessor_.block_size);
            instance_.deserialize(write_buffer);
            blk_accessor_.write(blk_address_, write_buffer);
            delete instance_;
            instance_ = 0;
        }

        void remove() {
            delete instance_;
            instance_ = 0;
            blk_accessor_.deallocate(blk_address_);
        }

    private:
        blk_address blk_address_;
        blk_accessor blk_accessor_;
        Node<K, V> instance_;
    };
}
#endif //NVM_BLK_NODE_REFERENCE_H
