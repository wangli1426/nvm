//
// Created by robert on 7/11/17.
//

#ifndef NVM_IN_NVME_B_PLUS_TREE_H
#define NVM_IN_NVME_B_PLUS_TREE_H

#include "vanilla_b_plus_tree.h"
#include "../blk/nvme_blk_accessor.h"

namespace tree{
    template<typename K, typename V, int CAPACITY>
    class in_nvme_b_plus_tree: public VanillaBPlusTree<K, V, CAPACITY> {
    public:
        in_nvme_b_plus_tree(const int& block_size = 512): VanillaBPlusTree<K, V, CAPACITY>(
                new nvme_blk_accessor<K, V, CAPACITY>(block_size)), block_size_(block_size) {
            set_blk_accessor(block_size_);
        }

        virtual ~in_nvme_b_plus_tree() {
        }

        virtual void clear() {
            VanillaBPlusTree<K, V, CAPACITY>::close();
            set_blk_accessor(block_size_);
            VanillaBPlusTree<K, V, CAPACITY>::init();
        }
    private:

        void set_blk_accessor(const int & block_size) {
            if (this->blk_accessor_)
                delete this->blk_accessor_;
            this->blk_accessor_ = new nvme_blk_accessor<K, V, CAPACITY>(block_size_);
            this->blk_accessor_->open();
        }
        const int block_size_;
    };
}

#endif //NVM_IN_NVME_B_PLUS_TREE_H
