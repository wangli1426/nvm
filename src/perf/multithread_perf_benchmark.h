//
// Created by Li Wang on 10/22/17.
//

#ifndef NVM_MULTITHREAD_PERF_BENCHMARK_H
#define NVM_MULTITHREAD_PERF_BENCHMARK_H

#endif //NVM_MULTITHREAD_PERF_BENCHMARK_H


#include <iostream>
#include <thread>
#include "../accessor/ns_entry.h"
#include "../accessor/qpair_context.h"
#include "../utils/rdtsc.h"
#include "io_task.h"
#include "sequential_access.h"
#include "random_access.h"
#include "common.h"

using namespace std;
namespace nvm {

    void run_multiple_threads_benchmark(const int number_of_threads, const int number_of_accesses,
                                        const int bytes_per_access,
                                        const int queue_length, access_mode access, workload load, io_mode mode) {
        total_cycles = 0;
//        cout << "perf test" << endl;

        const uint64_t number_of_sectors = nvm_utility::get_sector_number();
        const uint64_t sectors_per_thread = number_of_sectors / number_of_threads;
        QPair *qpairs[number_of_threads];
        std::thread threads[number_of_threads];
        access_pattern *patterns[number_of_threads];

        for (int i = 0; i < number_of_threads; i++) {
            qpairs[i] = nvm_utility::allocateQPair(queue_length);
            if (access == seq_access)
                patterns[i] = new sequential_access(sectors_per_thread * i, sectors_per_thread * (i + 1));
            else
                patterns[i] = new random_access(sectors_per_thread * i, sectors_per_thread * (i + 1));
        }

        uint64_t start = ticks();
        for (int i = 0; i < number_of_threads; i++) {
            threads[i] = std::thread(run_task, number_of_accesses / number_of_threads, bytes_per_access, load, patterns[i], mode,
                                     qpairs[i]);
        }

        for (int i = 0; i < number_of_threads; i++) {
            threads[i].join();
        }
        uint64_t cycles = ticks() - start;

        for (int i = 0; i < number_of_threads; i++) {
            delete patterns[i];
            delete qpairs[i];
        }

//        nvm_utility::detach();


//        printf("total cycles: %ld, %.3f us per I/O,  %.3f IOPS.\n",
//               cycles,
//               cycles_to_microseconds(cycles / number_of_accesses),
//               1000000000 / cycles_to_nanoseconds(cycles / number_of_accesses));
//        printf("complete latency per IO: %.3f us\n", cycles_to_microseconds(total_cycles / number_of_accesses));

        printf("%d\t%d\t%.3f\t%.3f\n", queue_length, load, 1000000000 / cycles_to_nanoseconds(cycles / number_of_accesses) / 1000.0, cycles_to_microseconds(total_cycles / number_of_accesses));

//        printf("%.1f\t", 1000000000 / cycles_to_nanoseconds(cycles / number_of_accesses) / 1000);
    }
}