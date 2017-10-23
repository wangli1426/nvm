//
// Created by Li Wang on 10/22/17.
//

#ifndef NVM_SINGLE_THREAD_BENCHMARK_H
#define NVM_SINGLE_THREAD_BENCHMARK_H

#include <stdio.h>
#include <string>
#include "common.h"
#include "sequential_access.h"
#include "random_access.h"
#include "access_pattern.h"
#include "io_task.h"

using namespace std;
namespace nvm {
    void run_single_thread_benchmark(const int number_of_accesses,
                                     const int bytes_per_access,
                                     const int queue_length, access_mode access, workload load, io_mode mode) {
        total_cycles = 0;
        string access_string, workload_string, mode_string;
        access_pattern *pattern;
        if (access == seq_access) {
            pattern = new sequential_access(0, nvm_utility::get_sector_number());
            access_string = "seq";
        } else {
            pattern = new random_access(0, nvm_utility::get_sector_number());
            access_string = "rand";
        }

        workload_string = load == read_load ? "read" : "write";

        mode_string = mode == synch ? "synch" : "asynch";

        printf("settings: accesses = %d, bytes per access = %d, queue length = %d, %s, %s, %s\n", number_of_accesses,
               bytes_per_access, queue_length, access_string.c_str(), workload_string.c_str(), mode_string.c_str());

        QPair *qpair = nvm_utility::allocateQPair(queue_length);
        uint64_t cycles;
        run_task_with_timing(number_of_accesses, bytes_per_access, load, pattern, mode, qpair, cycles);
        delete qpair;
        printf("total cycles: %ld, %.3f us per I/O,  %.3f IOPS, ",
               cycles,
               cycles_to_microseconds(cycles / number_of_accesses),
               1000000000 / cycles_to_nanoseconds(cycles / number_of_accesses));
        printf("complete latency per IO: %.3f us\n", cycles_to_microseconds(total_cycles / number_of_accesses));
    }
}
#endif //NVM_SINGLE_THREAD_BENCHMARK_H
