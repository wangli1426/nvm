//
// Created by Li Wang on 10/17/17.
//

#include <iostream>
#include "accessor/ns_entry.h"
#include "accessor/qpair_context.h"
#include "utils/rdtsc.h"

using namespace std;
using namespace nvm;
int main() {
    total_cycles = 0;
    cout << "perf test" << endl;
    int status = nvm::initialize_namespace();
    if (status != 0) {
        cout << "Errors in initialization of namespace." << endl;
    } else {
        cout << "namespace is initialized." << endl;
    }
    QPair *qpair = allocateQPair(64);

    int number_of_sectors = 1;
    int number_of_accesses = 100000;

    uint32_t sector_size = qpair->get_sector_size();
    const uint64_t sector_number = qpair->get_sector_number();

    uint32_t buffer_size = sector_size * number_of_sectors;

    char* buffer = (char*)spdk_dma_zmalloc(number_of_sectors * qpair->get_sector_size(), 0x1000, NULL);
    snprintf(buffer, 0x1000, "%s", "Hello world!\n");
    bool is_complete = false;
    uint64_t start = ticks();
    for (int i = 0; i < number_of_accesses; i++) {
        if (i == number_of_accesses - 1)
            qpair->asynchronous_read(buffer, buffer_size, rand() % sector_number, &is_complete);
        else
            qpair->asynchronous_read(buffer, buffer_size, rand() % sector_number, 0);
//        qpair->synchronous_write(buffer, buffer_size, rand() % sector_number);
    }

//    while(!is_complete) {
//        qpair->process_completions();
//    }

    printf("String: %c\n", buffer[3]);

    printf("Number of Free Slots: %d\n", qpair->get_number_of_free_slots());

    uint64_t cycles = ticks() - start;
    printf("total cycles: %ld, %ld us per I/O,  %ld IOPS.\n",
           cycles,
           cycles_to_microseconds(cycles / number_of_accesses),
           1000000000 / cycles_to_nanoseconds(cycles / number_of_accesses));
    printf("complete latency per IO: %ld us\n", cycles_to_microseconds(total_cycles / number_of_accesses));
    spdk_dma_free(buffer);
    qpair->detach();
}
