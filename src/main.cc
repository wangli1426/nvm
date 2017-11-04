#include <stdio.h>
#include <fstream>
#include <string>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include "tree/in_disk_b_plus_tree.h"
#include "tree/vanilla_b_plus_tree.h"


using namespace tree;

int main() {
    in_disk_b_plus_tree<int, int, 4> tree;
    tree.init();
    tree.insert(1, 1);
    tree.insert(3, 3);
    tree.insert(2, 2);
    tree.insert(6, 6);

    printf("%s\n", tree.toString().c_str());
    tree.insert(10, 10);
    printf("%s\n", tree.toString().c_str());
}