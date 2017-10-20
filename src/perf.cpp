//
// Created by Li Wang on 10/17/17.
//

#include <iostream>
#include <thread>
#include "accessor/ns_entry.h"
#include "perf/io_task.h"
#include "perf/sequential_access.h"
#include "accessor/qpair_context.h"
#include "utils/rdtsc.h"

using namespace std;
using namespace nvm;
int main() {
    total_cycles = 0;
    cout << "perf test" << endl;
    int status = nvm::nvm_utility::initialize_namespace();
    if (status != 0) {
        cout << "Errors in initialization of namespace." << endl;
        exit(1);
    } else {
        cout << "namespace is initialized." << endl;
    }

    const int number_of_accesses = 100000;
    const int queue_length = 8;
    const int number_of_threads = 2;
    const uint64_t number_of_sectors = nvm_utility::get_sector_number();
    const uint64_t sectors_per_thread = number_of_sectors / number_of_threads;
    QPair* qpairs[number_of_threads];
    std::thread threads[number_of_threads];
    access_pattern* patterns[number_of_threads];

    for (int i = 0; i < number_of_threads; i++) {
        qpairs[i] = nvm_utility::allocateQPair(queue_length);
        patterns[i] = new sequential_access(sectors_per_thread * i, sectors_per_thread * (i + 1));
    }

    uint64_t start = ticks();
    for (int i = 0; i < number_of_threads; i++) {
        threads[i] = std::thread(run_task, number_of_accesses, 512, read_load, patterns[i], asynch, qpairs[i]);
    }

    for (int i = 0; i < number_of_threads; i++) {
        threads[i].join();
    }
    uint64_t cycles = ticks() - start;

    for (int i = 0 ; i < number_of_threads; i++) {
        delete patterns[i];
    }

    nvm_utility::detach();

    printf("total cycles: %ld, %.3f us per I/O,  %.3f IOPS.\n",
           cycles,
           cycles_to_microseconds(cycles / number_of_accesses),
           1000000000 / cycles_to_nanoseconds(cycles / number_of_accesses));
    printf("complete latency per IO: %.3f us\n", cycles_to_microseconds(total_cycles / number_of_accesses));

}
