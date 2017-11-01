#include <stdio.h>
#include <fstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include "tree/in_disk_b_plus_tree.h"



int main() {
    in_disk_b_plus_tree<int, int, 16> tree;
    tree.init();
    tree.insert(1, 2);
    tree.insert(2, 3);
}