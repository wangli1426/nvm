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

int main() {

//    Semaphore sema(4);
//    sema.wait();
//    sema.wait();
//    sema.wait();
//    sema.wait();
//    printf("waited!\n");
//    exit(0);

    const int tuples = 100;
//
    nvme_optimized_b_plus_tree<int, int, 16> tree(8);
    tree.init();

    for (int i = 0; i < tuples; i++) {
        tree.insert(i, i);
    }

//    for (int i = 0; i < tuples; i++) {
//        int value;
//        bool found = tree.asynchronous_search(i, value);
//        printf("%d -> %d (%d)\n", i, value, found);
//    }

    Semaphore semaphore(4);
    for (int i = 0; i < tuples; i++) {
        search_callback_arg* arg = new search_callback_arg();
        arg->key = i;
        arg->value = -1;
        arg->sema = & semaphore;
        semaphore.wait();
        tree.asynchronous_search_with_callback(i, arg->value, update_concurrency, arg);
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