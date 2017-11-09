#include <stdio.h>
#include <fstream>
#include <string>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include "tree/in_disk_b_plus_tree.h"
#include "tree/vanilla_b_plus_tree.h"
#include "tree/nvme_optimized_b_plus_tree.h"
#include "utils/sync.h"

using namespace tree;

int main() {

    const int tuples = 3;

    nvme_optimized_b_plus_tree<int, int, 16> tree(8);
    tree.init();

    for (int i = 0; i < tuples; i++) {
        tree.insert(i, i);
    }

    for (int i = 0; i < tuples; i++) {
        int value;
        tree.asynchronous_search(i, value);
        printf("%d -> %d\n", i, value);
    }
    Semaphore sema;
    printf("%d\n", sema.get_value());
//    sema.post();
//    sema.wait();
}