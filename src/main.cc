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
#include <set>

using namespace tree;


int main() {

    disk_optimized_b_plus_tree<int, int, 16> tree("tree.dat", 8);
    tree.init();
    vector<int> keys;
    vector<operation<int, int> > operations;
    const int tuples = 500;

    for(int i = 0; i < tuples; i++) {
        keys.push_back(i);
    }

    std::random_shuffle(&keys[0], &keys[tuples]);

    printf("start insertion\n");
    for (auto it = keys.cbegin(); it != keys.cend(); ++it) {
        tree.insert(*it, *it);
    }

    printf("all insertion commands are submitted!\n");

    printf("insertion finished!\n");

    for (auto it = keys.cbegin(); it != keys.cend(); ++it) {
        int value;
        tree.search(*it, value);
    }
    printf("all search commands are submitted!\n");

    tree.close();
    printf("done!\n");
}