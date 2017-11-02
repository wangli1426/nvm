#include <stdio.h>
#include <fstream>
#include <string>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include "tree/in_disk_b_plus_tree.h"


using namespace tree;

int main() {

    in_disk_b_plus_tree<int, int, 4> tree;
    tree.init();
    tree.insert(1, 2);
    tree.insert(2, 3);
    tree.insert(3, 3);
    tree.insert(4, 3);
    tree.insert(5, 3);
    tree.insert(6, 3);
    tree.insert(7, 3);
    tree.insert(8, 3);
    tree.insert(9, 3);
}