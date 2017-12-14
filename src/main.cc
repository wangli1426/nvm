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
#include "perf/operation.h"
#include "perf/tree_operation_executor.h"
#include "perf/perf_test.h"
#include <set>

using namespace tree;


int main() {
//
//    disk_optimized_b_plus_tree<int, int, 256> disk_optimized("tree.dat", 256, 4096);
//    disk_optimized.init();
//    multithread_benchmark_mixed_workload(&disk_optimized, "disk_optimized_b_plus_tree", 1, 100000, 100000, 0.5, 0, 1);
////    multithread_benchmark_mixed_workload(&disk_optimized, "disk_optimized_b_plus_tree", 1, 100000, 100000, 0.5, 0, 2);
//    disk_optimized.close();
    VanillaBPlusTree<int, int, 4> tree;
    tree.init();
    tree.insert(1, 1);
    tree.insert(3, 3);
    tree.insert(2, 2);
    tree.insert(6, 6);


    tree.insert(10, 10);

    tree.insert(9, 9);

    tree.insert(8, 8);

    tree.insert(7, 7);
    tree.insert(4, 4);
    tree.insert(5, 5);

    tree.insert(11, 11);
    tree.insert(12, 12);
    tree.close();
}