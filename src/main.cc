#include <stdio.h>
#include <fstream>
#include <string>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include "tree/in_disk_b_plus_tree.h"
#include "tree/vanilla_b_plus_tree.h"
#include "tree/nvme_optimized_b_plus_tree.h"


using namespace tree;

int main() {
    nvme_optimized_b_plus_tree<int, int, 32> tree(8);
    tree.init();
    tree.insert(8, 8);
    tree.insert(9, 9);

    int value;
    tree.asynchronous_search(8, value);
    printf("%d -> %d\n", 8, value);

    tree.asynchronous_search(9, value);
    printf("%d -> %d\n", 9, value);

}