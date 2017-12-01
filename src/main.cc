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

    vector<int> keys;
    const int tuples = 5;

    for(int i = 0; i < tuples; ++i) {
        keys.push_back(i);
    }


    concurrent_in_disk_b_plus_tree<int, int, 4> tree;
    tree.init();

    for(auto it = keys.cbegin(); it != keys.cend(); ++it) {
        tree.insert(*it, *it);
    }


    for (auto it = keys.cbegin(); it != keys.cend(); ++it) {
        int value = -1;
        tree.search(*it, value);
        printf("%d: %d\n", *it, value);
    }

}