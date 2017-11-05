#include <stdio.h>
#include <fstream>
#include <string>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include "tree/in_disk_b_plus_tree.h"
#include "tree/vanilla_b_plus_tree.h"


using namespace tree;

int main() {
    const int number_of_tuples = 200;
    std::vector<int> tuples;
    for (int i = 0; i < number_of_tuples; ++i) {
        tuples.push_back(i);
    }
    std::random_shuffle(tuples.begin(), tuples.end());


    in_disk_b_plus_tree<int, int, 16> tree;
    tree.init();
    for (std::vector<int>::const_iterator it = tuples.cbegin(); it != tuples.cend(); ++it) {
        printf("########\n");
        tree.insert(*it, *it);
        printf("########\n\n");
    }


}