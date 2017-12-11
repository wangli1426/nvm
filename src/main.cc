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

    disk_optimized_b_plus_tree<int, int, 32> disk_optimized("tree.dat", 256);
    disk_optimized.init();
    multithread_benchmark_mixed_workload(&disk_optimized, "disk_optimized_b_plus_tree", 1, 10000, 10000, 0.5, 0, 2);
    disk_optimized.close();
}