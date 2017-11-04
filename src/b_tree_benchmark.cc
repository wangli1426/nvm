//
// Created by Li Wang on 11/4/17.
//

#include <stdio.h>
#include "perf/perf_test.h"
#include "tree/vanilla_b_plus_tree.h"
#include "tree/in_disk_b_plus_tree.h"

using namespace tree;
int main() {
//    VanillaBPlusTree<int, int, 16> tree;
    in_disk_b_plus_tree<int, int, 128> in_disk_tree("tree.dat", 2048);
//    tree.init();
    in_disk_tree.init();
//    benchmark<int, int>(&tree, "in-memory", 2, 1000000, 1000000, 1000000, 1);
    benchmark<int, int>(&in_disk_tree, "on-disk", 2, 10000, 10000, 10000, 1);
}