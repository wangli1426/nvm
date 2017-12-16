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
#include "../blk/nvme_blk_thread_dediciated_accessor_.h"
#include "../blk/nvme_blk_shared_working_thread_accessor_.h"
#include "../utils/sync.h"
#include "concurrent_b_plus_tree.h"

using namespace std;

#define SHARED 0
#define DEDICATED 1
#define SHARED_IO_THREAD 3


namespace tree{
    template<typename K, typename V, int CAPACITY>
    class concurrent_in_nvme_b_plus_tree: public concurrent_b_plus_tree<K, V, CAPACITY> {
    public:
        concurrent_in_nvme_b_plus_tree(int32_t mode = SHARED, int io_queue_length = 256, int32_t block_size = 512):
                mode_(mode), concurrent_b_plus_tree<K, V, CAPACITY>(
                block_size), io_queue_length_(io_queue_length) {
            set_blk_accessor();
        }

        virtual ~concurrent_in_nvme_b_plus_tree() {
        }

    protected:
        void set_blk_accessor() {
            if (this->blk_accessor_)
                delete this->blk_accessor_;
            switch(mode_) {
                case SHARED:
                    this->blk_accessor_ = new nvme_blk_accessor<K, V, CAPACITY>(this->block_size_);
                    break;
                case DEDICATED:
                    this->blk_accessor_ = new nvme_blk_thread_dedicated_accessor<K, V, CAPACITY>(this->block_size_);
                    break;
                case SHARED_IO_THREAD:
                    this->blk_accessor_ = new nvme_blk_shared_working_thread_accessor<K, V, CAPACITY>(this->block_size_, io_queue_length_);
                    break;
                default: {
                    assert(false);
                }
            }

            this->blk_accessor_->open();
            spdk_unaffinitize_thread();
            print_current_cpu_set();
            set_cpu_set(32);
            print_current_cpu_set();
        }

    private:
        const char* file_name_;
        int32_t mode_;
        int32_t io_queue_length_;
    };
}
#endif //NVM_CONCURRENT_IN_NVME_B_PLUS_TREE_H
