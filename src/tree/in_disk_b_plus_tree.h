//
// Created by robert on 31/10/17.
//

#ifndef NVM_IN_DISK_B_PLUS_TREE_H
#define NVM_IN_DISK_B_PLUS_TREE_H

#include "vanilla_b_plus_tree.h"
#include "../blk/file_blk_accessor.h"

namespace tree{
    template<typename K, typename V, int CAPACITY>
    class in_disk_b_plus_tree: public VanillaBPlusTree<K, V, CAPACITY> {
    public:
        in_disk_b_plus_tree(const char* file_name = "./b_tree.dat", const int& block_size = 512): VanillaBPlusTree<K, V, CAPACITY>(
                new file_blk_accessor<K, V, CAPACITY>(file_name_, block_size)), file_name_(file_name), block_size_(block_size) {
            set_blk_accessor(block_size_);
        }

        virtual ~in_disk_b_plus_tree() {
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
            this->blk_accessor_ = new file_blk_accessor<K, V, CAPACITY>(file_name_, block_size_);
            this->blk_accessor_->open();
        }
        const char* file_name_;
        const int block_size_;
    };
}
#endif //NVM_IN_DISK_B_PLUS_TREE_H
