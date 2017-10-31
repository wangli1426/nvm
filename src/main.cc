#include <stdio.h>
#include "tree/in_disk_b_plus_tree.h"

using namespace tree;

int main() {
    in_disk_b_plus_tree<int, int, 16> tree("./tree_file");
    tree.init();
    tree.insert(1, 1);
    tree.insert(2, 2);


}