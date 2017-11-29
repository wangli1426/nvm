#include <stdio.h>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include "tree/in_disk_b_plus_tree.h"
#include "tree/vanilla_b_plus_tree.h"
#include "tree/disk_optimized_b_plus_tree.h"
#include "utils/generator.h"
#include "tree/disk_optimized_tree_for_benchmark.h"
#include "utils/sync.h"
#include "utils/rdtsc.h"
#include "tree/nvme_optimized_tree_for_benchmark.h"
#include <set>

using namespace tree;

template <typename K, typename V>
struct operation{
    K key;
    V val;
    int type;
};


#define READ_OP 1
#define WRITE_OP 2

struct char512 {
//    int32_t i1[32];
//    int32_t i2[32];
//    double d[32];
    char data[8];
};


int main() {
    {
        const int number_of_tuples = 20;
        std::vector<int> tuples;
        for (int i = 0; i < number_of_tuples; ++i) {
            tuples.push_back(i);
        }
        std::random_shuffle(tuples.begin(), tuples.end());


        VanillaBPlusTree<int, int, 4> tree;
        tree.init();
        for (std::vector<int>::const_iterator it = tuples.cbegin(); it != tuples.cend(); ++it) {
            tree.insert(*it, *it);
        }

        printf("tree: %s\n", tree.toString().c_str());


        std::random_shuffle(tuples.begin(), tuples.end());
        for (std::vector<int>::const_iterator it = tuples.cbegin(); it != tuples.cend(); ++it) {
            if (*it == 0) {
                printf("0!\n");
            }
            tree.delete_key(*it);
            printf("tree: %s after deleting %d\n", tree.toString().c_str(), *it);
        }
        printf("%s\n", tree.toString().c_str());
        exit(0);
    }


    const int order = 32;
    const int queue_length = 2;
    const int tuples = 8000;
    vector<int> keys;
    disk_optimized_tree_for_benchmark<int, int, order> tree(queue_length);
    tree.init();

    for (int i = 0; i < tuples; i++) {
        keys.push_back(i);
    }

    std::random_shuffle(&keys[0], &keys[tuples]);

    for (auto it = keys.begin(); it != keys.cend(); ++it) {
        tree.insert(*it, *it);
    }

    tree.sync();
//
//    for (auto it = keys.begin(); it != keys.cend(); ++it) {
//        int value;
//        tree.search(*it, value);
//    }




    tree.sync();

    printf("tree height is %d\n", tree.get_height());

    tree.close();






//    const int order = 16;
//    const int queue_length = 32;
//    nvme_optimized_tree_for_benchmark<int, int, order> tree(512, queue_length);
//    tree.init();
//
//    double build_time = 0, search_time = 0, update_time = 0;
//    int runs = 1;
//    int run = 1;
//    int founds = 0;
//    int errors = 0;
//    int ntuples = 10000, noperations = 10000;
//    double skewness = 0.5;
//    const double write_rate = 0.5;
//    uint64_t search_cycles = 0;
//    ZipfGenerator generator(ntuples, skewness);
//
////    int *tuples = new int[ntuples];
//
//    vector<pair<int, int>> tuples;
//    vector<operation<int, int>> operations;
//
//    for (int i = 0; i < ntuples; ++i) {
//        tuples.push_back(make_pair(i, i));
//    }
//    random_shuffle(tuples.begin(), tuples.end());
//
//    for (int i = 0; i < noperations; ++i) {
//        const int key = generator.gen();
//        operation<int, int> op;
//        op.key = key;
//        op.val = key;
//        if (rand() / (double)RAND_MAX < write_rate) {
//            op.type = WRITE_OP;
//        } else {
//            op.type = READ_OP;
//        }
//        operations.push_back(op);
//    }
//
//    printf("begin to run benchmark\n");
//    while (run--) {
////        if (run != runs)
//        tree.clear();
//        std::set<int> s;
//
//        uint64_t begin = ticks();
//        for (auto it = tuples.begin(); it != tuples.cend(); ++it) {
//            tree.insert(it->first, it->second);
//        }
//        uint64_t end = ticks();
//        double elapsed_secs = cycles_to_seconds(end - begin);
//        build_time += elapsed_secs;
//
//        printf("inserted...\n");
//
//
//        begin = ticks();
//
//        for (int i = 0; i < noperations; ++i) {
//            operation<int ,int> op = operations[i];
//            if (op.type == WRITE_OP) {
//                tree.insert(op.key, op.val);
//            } else {
//                tree.search(op.key, op.val);
//            }
//        }
//        tree.sync();
//        end = ticks();
//        search_time += cycles_to_seconds(end - begin);;
//        printf("searched...\n");
//
//    }
//
//    cout << ntuples << " tuples." << endl;
//
//    cout << "[main]: " << "#. of runs: " << runs << ", #. of tuples: " << ntuples
//         << ", Insert: " << ntuples * runs / build_time / 1000 << " K tuples / s"
//         << ", Mix(" << write_rate * 100 <<"% write): " << noperations * runs / search_time / 1000 << " K tuples / s"
//         << endl;

}