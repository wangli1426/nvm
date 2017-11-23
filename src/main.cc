#include <stdio.h>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include "tree/in_disk_b_plus_tree.h"
#include "tree/vanilla_b_plus_tree.h"
#include "tree/disk_optimized_b_plus_tree.h"
#include "tree/disk_optimized_tree_for_benchmark.h"
#include "utils/sync.h"
#include "utils/rdtsc.h"
#include "tree/nvme_optimized_tree_for_benchmark.h"

using namespace tree;

int main() {


    const int tuples = 200;
    std::vector<int> keys;
//
    disk_optimized_tree_for_benchmark<int, int, 4> tree(1, "tmp.dat");
//    in_nvme_b_plus_tree<int, int, 32> tree(512);
    tree.init();
//    tree.clear();

    for (int i = 0; i < tuples; i++) {
        keys.push_back(tuples - i);
    }
    std::random_shuffle(&keys[0], &keys[tuples - 1]);

//    std::reverse(&keys[0], &keys[tuples]);

    for(auto it = keys.begin(); it != keys.end(); ++it) {

        tree.insert(*it, *it);
        tree.sync();
//        printf("inserted %d\n", *it);
    }


    tree.sync();

//    BTree<int, int>::Iterator* it =  tree.get_iterator();
//    int k, v;
//    while (it->next(k, v)) {
//        printf("%d -----> %d \n", k, v);
//    }



    uint64_t start = ticks();
    for (int i = 0; i < tuples; i++) {
        int value = -1024;
        tree.search(keys[i], value);
        printf("[%d]: search operator for %d is submitted\n", i, keys[i]);
    }

//    while(tree.get_pending_requests()!=0){
//        usleep(1);
//    }
    uint64_t end = ticks();

    tree.close();
    printf("search throughput: %.2f K tuples / s\n", tuples / cycles_to_seconds(end - start) / 1000);

//    for (int i = 0; i < tuples; i++) {
//        int value;
//        bool found = tree.asynchronous_search(i, value);
//        printf("%d -> %d (%d)\n", i, value, found);
//    }



//    Semaphore semaphore(4);
//    for (int i = 0; i < tuples; i++) {
//        search_callback_arg* arg = new search_callback_arg();
//        arg->key = i;
//        arg->value = -1;
//        arg->sema = & semaphore;
//        semaphore.wait();
//        tree.asynchronous_search_with_callback(i, arg->value, update_concurrency, arg);
//    }

//    sleep(5);

//    sleep(2);
//    Semaphore s, s2;
//    std::thread t = std::thread(dosometing, &s);
//    std::thread t2 = std::thread(dosometing, &s2);
//    printf("wait!\n");
//    s.wait();
//    printf("waited!\n");
//    t.join();
//    t2.join();
}