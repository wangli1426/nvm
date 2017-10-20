//
// Created by Li Wang on 10/17/17.
//

#include <iostream>
#include "accessor/ns_entry.h"
#include "perf/io_task.h"
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
    } else {
        cout << "namespace is initialized." << endl;
    }

    const int number_of_accesses = 100000;
    const int queue_length = 16;
    QPair* qpair = nvm_utility::allocateQPair(queue_length);

    uint64_t start = ticks();
    run_task(number_of_accesses, 512, read_load, seq, asynch, qpair);
    uint64_t cycles = ticks() - start;

    qpair->detach();
    printf("total cycles: %ld, %.3f us per I/O,  %.3f IOPS.\n",
           cycles,
           cycles_to_microseconds(cycles / number_of_accesses),
           1000000000 / cycles_to_nanoseconds(cycles / number_of_accesses));
    printf("complete latency per IO: %.3f us\n", cycles_to_microseconds(total_cycles / number_of_accesses));

}
