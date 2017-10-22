//
// Created by Li Wang on 10/22/17.
//

#ifndef NVM_SINGLE_THREAD_BENCHMARK_H
#define NVM_SINGLE_THREAD_BENCHMARK_H

#include <stdio.h>
#include "common.h"
#include "sequential_access.h"
#include "random_access.h"
#include "access_pattern.h"
#include "io_task.h"

namespace nvm{
    void run_single_thread_benchmark(const int number_of_accesses,
                                     const int bytes_per_access,
                                     const int queue_length, access_mode access, workload load, io_mode mode) {
        total_cycles = 0;
        cout << "perf test" << endl;

        access_pattern *pattern;
        if (access == seq_access)
            pattern = new sequential_access(0, nvm_utility::get_sector_number());
        else
            pattern = new random_access(0, nvm_utility::get_sector_number());
        QPair* qpair = nvm_utility::allocateQPair(queue_length);
        uint64_t cycles;
        run_task_with_timing(number_of_accesses, bytes_per_access, load, pattern, mode, qpair, cycles);
        delete qpair;
        printf("total cycles: %ld, %.3f us per I/O,  %.3f IOPS.\n",
               cycles,
               cycles_to_microseconds(cycles / number_of_accesses),
               1000000000 / cycles_to_nanoseconds(cycles / number_of_accesses));
        printf("complete latency per IO: %.3f us\n", cycles_to_microseconds(total_cycles / number_of_accesses));
    }
}
#endif //NVM_SINGLE_THREAD_BENCHMARK_H
