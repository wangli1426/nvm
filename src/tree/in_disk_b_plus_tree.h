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
        in_disk_b_plus_tree(const char* file_name): VanillaBPlusTree<K, V, CAPACITY>(), file_name_(file_name) {
        }

        virtual ~in_disk_b_plus_tree() {
        }

    private:

        void set_blk_accessor() {
            this->blk_accessor_ = new file_blk_accessor<K, V, CAPACITY>(file_name_, 512);
        }
        const char* file_name_;
    };
}
#endif //NVM_IN_DISK_B_PLUS_TREE_H
