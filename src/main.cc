#include <stdio.h>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include "tree/in_disk_b_plus_tree.h"
#include "tree/concurrent_in_disk_b_plus_tree.h"
#include "tree/vanilla_b_plus_tree.h"
#include "tree/disk_optimized_b_plus_tree.h"
#include "utils/generator.h"
#include "tree/disk_optimized_tree_for_benchmark.h"
#include "utils/sync.h"
#include "utils/rdtsc.h"
#include "tree/nvme_optimized_tree_for_benchmark.h"
#include "sync/rwlock.h"
#include "perf/operation.h"
#include "perf/tree_operation_executor.h"
#include "perf/perf_test.h"
#include <set>
#include "utils/dummy.h"
#include "utils/cpu_set.h"
using namespace tree;

class A {
public:
    std::vector<int>& get_list() {
        return list;
    }
    std::vector<int> list;
};


int main() {
//
    disk_optimized_b_plus_tree<int, int, 32> disk_optimized("tree.dat", 256, 512);
    disk_optimized.init();
    multithread_benchmark_mixed_workload(&disk_optimized, "disk_optimized_b_plus_tree", 1, 1000, 1000, 0, 0, 1);
//////    multithread_benchmark_mixed_workload(&disk_optimized, "disk_optimized_b_plus_tree", 1, 100000, 100000, 0.5, 0, 2);
    disk_optimized.close();
//    create_one_cpu_consumer();
//    create_one_cpu_consumer();
//    create_one_cpu_consumer();
//    sleep(10000);

//    A a;
//    a.list.push_back(1);
//    a.list.push_back(2);
//
//    std::vector<int> &list = a.get_list();
//    list.push_back(3);
//    printf("size is %d.\n", a.list.size());

}