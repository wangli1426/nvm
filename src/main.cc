#include <stdio.h>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include "tree/in_disk_b_plus_tree.h"
#include "tree/concurrent_in_disk_b_plus_tree.h"
#include "tree/vanilla_b_plus_tree.h"
#include "tree/disk_optimized_b_plus_tree.h"
#include "utils/generator.h"
#include "tree/disk_optimized_tree_for_benchmark.h"
#include "utils/sync.h"
#include "utils/rdtsc.h"
#include "tree/nvme_optimized_tree_for_benchmark.h"
#include "sync/rwlock.h"
#include <set>

using namespace tree;


int main() {

    concurrent_in_disk_b_plus_tree<int, int, 4> tree;
    tree.init();
    tree.insert(1, 1);
    tree.insert(3, 3);
    tree.insert(2, 2);
    tree.insert(6, 6);

//    printf("%s\n", tree.toString().c_str());

    tree.insert(10, 10);
//    printf("%s", tree.toString().c_str());

    tree.insert(9, 9);
//    printf("%s", tree.toString().c_str());

    tree.insert(8, 8);
//    printf("%s", tree.toString().c_str());

    tree.insert(7, 7);
    tree.insert(4, 4);
    tree.insert(5, 5);
//    printf("%s", tree.toString().c_str());

    tree.insert(11, 11);
    tree.insert(12, 12);
    printf("%s", tree.toString().c_str());
}