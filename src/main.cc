#include <stdio.h>
#include <fstream>
#include <string>
#include "tree/in_disk_b_plus_tree.h"
#include "tree/vanilla_b_plus_tree.h"
#include "tree/disk_optimized_b_plus_tree.h"
#include "tree/disk_optimized_tree_for_benchmark.h"
#include "utils/sync.h"
#include "tree/nvme_optimized_tree_for_benchmark.h"

using namespace tree;

static void dosometing(Semaphore* s) {
    sleep(2);
    s->post();
}

struct search_callback_arg {
    Semaphore* sema;
    int key;
    int value;
};

void update_concurrency(void* args) {
    search_callback_arg* arg = reinterpret_cast<search_callback_arg*>(args);
    arg->sema->post();
    printf("%d -> %d\n", arg->key, arg->value);
    delete arg;
}

typedef void (*p)(int i);

static void print(int i) {
    printf("%d\n", i);
}

int main() {
//    p pi = & print;
//    p p2 = pi;
//    (*p2)(1);
//    exit(0);
//    Semaphore sema(4);
//    sema.wait();
//    sema.wait();
//    sema.wait();
//    sema.wait();
//    printf("waited!\n");
//    exit(0);

    const int tuples = 100;
//
//    nvme_optimized_tree_for_benchmark<int, int, 32> tree(8);
    disk_optimized_tree_for_benchmark<int, int, 32> tree(1);
    tree.init();
    tree.clear();

    for (int i = 0; i < tuples; i++) {
        tree.insert(i, i);
    }


    for (int i = 0; i < tuples; i++) {
        int value;
        tree.search(i, value);
        printf("search operator for [%d] is submitted\n", i);
    }

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
    tree.close();

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