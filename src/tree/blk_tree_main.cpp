//
// Created by Li Wang on 10/25/17.
//
#include "../blk/file_blk_accessor.h"
#include "blk_b_plus_tree.h"
using namespace tree;
int main() {
    blk_accessor* accessor = new file_blk_accessor("./temp", 512);
    accessor->open();
    BlkBPlusTree<int, int, 32> tree(accessor);
    tree.insert(1, 1);


}