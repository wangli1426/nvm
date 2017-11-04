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
    BTree<int, int>::Iterator *it = tree.range_search(INT_MIN, INT_MAX);
    int k, v;
    it->next(k, v);
    delete it;
}