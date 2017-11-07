//
// Created by Li Wang on 11/4/17.
//

#include <stdio.h>
#include "perf/perf_test.h"
#include "tree/vanilla_b_plus_tree.h"
#include "tree/in_disk_b_plus_tree.h"
#include "tree/in_nvme_b_plus_tree.h"

using namespace tree;
int main(int argc, char** argv) {

    const int order = 32;
    const int size = 512;
    const int ntuples = 100000;

    in_disk_b_plus_tree<int, int, order> in_disk_tree("tree.dat", size);
    in_disk_tree.init();
    benchmark<int, int>(&in_disk_tree, "in-disk", 1, ntuples, ntuples, ntuples, 1);

    in_nvme_b_plus_tree<int, int, order> in_nvme_tree(size);
    in_nvme_tree.init();
    benchmark<int, int>(&in_nvme_tree, "in-nvme", 1, ntuples, ntuples, ntuples, 1);

    VanillaBPlusTree<int, int, order> tree;
    tree.init();
    benchmark<int, int>(&tree, "in-memory", 2, ntuples, ntuples, ntuples, 1);
}