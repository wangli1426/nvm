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

    const int number_of_tuples = 15;
    std::vector<int> tuples;
    for (int i = 0; i < number_of_tuples; ++i) {
        tuples.push_back(i);
    }
//    std::random_shuffle(tuples.begin(), tuples.end());

    std::reverse(tuples.begin(), tuples.end());
    VanillaBPlusTree<int, int, 4> tree;
    tree.init();
    for (std::vector<int>::const_iterator it = tuples.cbegin(); it != tuples.cend(); ++it) {
        tree.insert(*it, *it);
    }

    std::random_shuffle(tuples.begin(), tuples.end());
    for (std::vector<int>::const_iterator it = tuples.cbegin(); it != tuples.cend(); ++it) {
        printf("%d\n", *it);
        tree.delete_key(*it);
        printf("tree: %s\n", tree.toString().c_str());
    }
}