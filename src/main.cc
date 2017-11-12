#include <stdio.h>
#include <fstream>
#include <string>
#include "tree/in_disk_b_plus_tree.h"
#include "tree/vanilla_b_plus_tree.h"
#include "tree/nvme_optimized_b_plus_tree.h"
#include "utils/sync.h"

using namespace tree;

static void dosometing(Semaphore* s) {
    sleep(2);
    s->post();
}

int main() {

    const int tuples = 100;
//
    nvme_optimized_b_plus_tree<int, int, 16> tree(8);
    tree.init();

    for (int i = 0; i < tuples; i++) {
        tree.insert(i, i);
    }

    for (int i = 0; i < tuples; i++) {
        int value;
        bool found = tree.asynchronous_search(i, value);
        printf("%d -> %d (%d)\n", i, value, found);
    }

    tree.close();

//    Semaphore s, s2;
//    std::thread t = std::thread(dosometing, &s);
//    std::thread t2 = std::thread(dosometing, &s2);
//    printf("wait!\n");
//    s.wait();
//    printf("waited!\n");
//    t.join();
//    t2.join();
}