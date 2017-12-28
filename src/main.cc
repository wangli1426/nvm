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
#include "utils/dummy.h"
#include "utils/cpu_set.h"
using namespace tree;

class A {
public:
    std::vector<int>& get_list() {
        return list;
    }
    std::vector<int> list;
};


int main() {
    disk_optimized_b_plus_tree<int, int, 16> tree("tree.dat", 256);
    tree.init();
    vector<int> keys;
    vector<operation<int, int> > operations;
    const int tuples = 10000;
    const int threads = 2;
    std::thread tid[threads];

    for(int i = 0; i < tuples; i++) {
        keys.push_back(i);
    }

    std::random_shuffle(&keys[0], &keys[tuples]);

    for (auto it = keys.cbegin(); it != keys.cend(); ++it) {
        operation<int, int> op;
        op.key = *it;
        op.val = *it;
        op.type = WRITE_OP;
        operations.push_back(op);
    }

    const int tuples_per_thread = tuples / threads;
    for (int i = 0; i < threads; i++) {
        tid[i] = std::thread(&execute_operations<int, int>, &tree, operations.begin() + i * tuples_per_thread, operations.begin() + (i + 1) * tuples_per_thread);
    }

    for (int i = 0; i < threads; i++) {
        tid[i].join();
    }

    tree.sync();

    for (auto it = keys.cbegin(); it != keys.cend(); ++it) {
        int value;
        tree.search(*it, value);
    }

    tree.close();
    printf("done!\n");

}