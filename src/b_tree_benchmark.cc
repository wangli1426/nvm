//
// Created by Li Wang on 11/4/17.
//

#include <stdio.h>
#include <algorithm>
#include "perf/perf_test.h"
#include "tree/vanilla_b_plus_tree.h"
#include "tree/in_disk_b_plus_tree.h"
#include "tree/in_nvme_b_plus_tree.h"
#include "tree/nvme_optimized_b_plus_tree.h"
#include "tree/nvme_optimized_tree_for_benchmark.h"
#include "tree/disk_optimized_tree_for_benchmark.h"
#include "utils/rdtsc.h"

#include <list>
#include <unordered_map>
using namespace std;
using namespace tree;
int main(int argc, char** argv) {

//
//    int writes = 100;
//    std::vector<int> addresses;
//    for (int i = 0; i < writes; i++) {
//        addresses.push_back(i);
//    }
//    std::random_shuffle(std::begin(addresses), std::end(addresses));
//    void* write_buffer;
//    posix_memalign(&write_buffer, 512, 512);
//    uint64_t start = ticks();
////    int fd = open("test.file", O_DIRECT);
//    int fd = open("test.file", O_CREAT|O_TRUNC|O_RDWR|O_DIRECT, S_IRWXU|S_IRWXG|S_IRWXO);
//    if (fd < 0) {
//        printf("failed to open the file!\n");
//        exit(1);
//    }
//    for (int i = 0; i < writes; i++) {
//        int status = (int)pwrite(fd, write_buffer, 512, 512 * addresses[i]);
//        if (status < 0) {
//            printf("[Write:] file access fails, error: %d\n", status);
//            printf("Reason: %s\n", strerror(errno));
//        }
//    }
//
//    for (int i = 0; i < writes; i++) {
//        int status = (int)pread(fd, write_buffer, 512, 512 * addresses[i]);
//        if (status < 0) {
//            printf("[Read:] file access fails, error: %d\n", status);
//            printf("Reason: %s\n", strerror(errno));
//        }
//    }
//    close(fd);
//    uint64_t duration = ticks() - start;
//    printf("AVG latency: %f ms\n", cycles_to_milliseconds(duration / writes));
//
//    exit(0);

//    void* write_buffer = malloc(512);
//    file_blk_accessor<int, int, 32> accessor("t", 512);
//    accessor.open();
//    int writes = 100;
//    std::vector<blk_address > addresses;
//    for (int i = 0; i < writes; i++ ) {
//        const blk_address addr = accessor.allocate();
//        addresses.push_back(blk_address(i));
//    }
//
////    std::random_shuffle(std::begin(addresses), std::end(addresses));
//
//    uint64_t start = ticks();
//    for (int i = 0; i < writes; i++ ) {
//        memset(write_buffer, i, 1);
//        accessor.write((blk_address)i, write_buffer);
//    }
//
//    free(write_buffer);
//    accessor.close();
//    uint64_t duration = ticks() - start;
//    printf("average: %.2f ms per write!!!\n", cycles_to_milliseconds(duration / writes));
//
//    exit(0);
//
//
    const int order = 32;
    const int size = 512;
    const int ntuples = 1000;
    const double write_rate = 0.5;
    const double key_skewness = 0.5;

//    in_disk_b_plus_tree<int, int, order> in_disk_tree("tree.dat1", size);
//    in_disk_tree.init();
//    benchmark<int, int>(&in_disk_tree, "in-disk", 1, ntuples, ntuples, ntuples, 1);

//    in_disk_b_plus_tree<int, int, order> in_disk_tree2("/media/nvme/tree.dat1", size);
//    in_disk_tree2.init();
//    benchmark<int, int>(&in_disk_tree2, "in-disk (nvme)", 1, ntuples, ntuples, ntuples, 1);
////
//
////
//    VanillaBPlusTree<int, int, order> tree;
//    tree.init();
//    benchmark<int, int>(&tree, "in-memory", 1, ntuples, ntuples, ntuples, 1);
//
    disk_optimized_tree_for_benchmark<int, int, order> tree_optimized(256);
    tree_optimized.init();
    benchmark<int, int>(&tree_optimized, "disk-optimized", 1, ntuples, ntuples, ntuples, 0);
//
//    benchmark_mixed_workload(&tree_optimized, "disk-optimized-mixed", 1, ntuples, ntuples, 0.5, 0);

////
//    in_nvme_b_plus_tree<int, int, order> in_nvme_tree(size);
//    in_nvme_tree.init();
//    benchmark<int, int>(&in_nvme_tree, "in-nvme", 1, ntuples, ntuples, 0, key_skewness);
//    benchmark_mixed_workload(&in_nvme_tree, "in-nvme", 1, ntuples, ntuples, 0, key_skewness);
//    benchmark_mixed_workload(&in_nvme_tree, "in-nvme", 1, ntuples, ntuples, 0.2, key_skewness);
//    benchmark_mixed_workload(&in_nvme_tree, "in-nvme", 1, ntuples, ntuples, 0.4, key_skewness);
//    benchmark_mixed_workload(&in_nvme_tree, "in-nvme", 1, ntuples, ntuples, 0.6, key_skewness);
//    benchmark_mixed_workload(&in_nvme_tree, "in-nvme", 1, ntuples, ntuples, 0.8, key_skewness);
//    benchmark_mixed_workload(&in_nvme_tree, "in-nvme", 1, ntuples, ntuples, 1, key_skewness);
////
//    nvme_optimized_tree_for_benchmark<int, int, order> nvme_optimized(size, 256);
//    nvme_optimized.init();
//    benchmark<int, int>(&nvme_optimized, "nvme-optimized", 1, ntuples, ntuples, 0, key_skewness);
//    benchmark_mixed_workload(&nvme_optimized, "nvme-optimized", 1, ntuples, ntuples, 0, key_skewness);
//    benchmark_mixed_workload(&nvme_optimized, "nvme-optimized", 1, ntuples, ntuples, 0.2, key_skewness);
//    benchmark_mixed_workload(&nvme_optimized, "nvme-optimized", 1, ntuples, ntuples, 0.4, key_skewness);
//    benchmark_mixed_workload(&nvme_optimized, "nvme-optimized", 1, ntuples, ntuples, 0.6, key_skewness);
//    benchmark_mixed_workload(&nvme_optimized, "nvme-optimized", 1, ntuples, ntuples, 0.8, key_skewness);
//    benchmark_mixed_workload(&nvme_optimized, "nvme-optimized", 1, ntuples, ntuples, 1, key_skewness);
//    printf("tree deeps: %d\n", nvme_optimized.get_height());
//

//    in_disk_b_plus_tree<int, int, 4> tree;
//    tree.init();
//    tree.insert(1, 1);
//    tree.insert(2, 2);
//    tree.insert(3, 3);
//    tree.insert(4, 4);
//    tree.insert(5, 5);
//    tree.insert(6, 6);
//
//    tree.delete_key(4);
//    tree.delete_key(3);
}