#include <stdio.h>
#include <vector>
#include "tree/vanilla_b_plus_tree.h"
#include <limits.h>
#include <algorithm>

using namespace tree;
int main() {

    const int number_of_tuples = 8;
    std::vector<int> tuples;
    for (int i = 0; i < number_of_tuples; ++i) {
        tuples.push_back(i);
    }
//    std::random_shuffle(tuples.begin(), tuples.end());


    VanillaBPlusTree<int, int, 4> tree;
    for (std::vector<int>::const_iterator it = tuples.cbegin(); it != tuples.cend(); ++it) {
        tree.insert(*it, *it);
    }

    printf("Tree: %s\n", tree.toString().c_str());
//    const int runs = 2;
//    for (int i = 0; i < runs; i++) {
//        int start = rand() % number_of_tuples;
//        int end = rand() % number_of_tuples;
    int start = rand() % number_of_tuples;
    int end = rand() % number_of_tuples;
    if (start > end) {
        int temp = start;
        start = end;
        end = temp;
    }
        BTree<int, int>::Iterator *it = tree.range_search(start, end);
        int key, value;
        int founds = 0;
        while (it->next(key, value)) {
//            EXPECT_EQ(founds + start, key);
            founds++;
        }
        delete it;
//    }
    printf("<%d, %d>\n", start, end);
    printf("%d -> %d\n", founds, end - start + 1);
}