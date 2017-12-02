//
// Created by robert on 1/12/17.
//

#ifndef NVM_CONCURRENT_IN_NVME_B_PLUS_TREE_H
#define NVM_CONCURRENT_IN_NVME_B_PLUS_TREE_H

#include <deque>
#include <stack>
#include "inner_node.h"
#include "leaf_node.h"
#include "vanilla_b_plus_tree.h"
#include "../sync/lock_manager.h"
#include "../blk/nvme_blk_accessor.h"
#include "../utils/sync.h"
#include "concurrent_b_plus_tree.h"

using namespace std;

namespace tree{
    template<typename K, typename V, int CAPACITY>
    class concurrent_in_nvme_b_plus_tree: public concurrent_b_plus_tree<K, V, CAPACITY> {
    public:
        concurrent_in_nvme_b_plus_tree(int32_t block_size = 512): concurrent_b_plus_tree<K, V, CAPACITY>(
                block_size) {
            set_blk_accessor();
        }

        virtual ~concurrent_in_nvme_b_plus_tree() {
        }

    protected:
        void set_blk_accessor() {
            if (this->blk_accessor_)
                delete this->blk_accessor_;
            this->blk_accessor_ = new nvme_blk_accessor<K, V, CAPACITY>(this->block_size_);
            this->blk_accessor_->open();
        }

    private:
        const char* file_name_;
    };
}
#endif //NVM_CONCURRENT_IN_NVME_B_PLUS_TREE_H
