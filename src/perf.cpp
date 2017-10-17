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
    cout << "perf test" << endl;
    int status = nvm::initialize_namespace();
    if (status != 0) {
        cout << "Errors in initialization of namespace." << endl;
    } else {
        cout << "namespace is initialized." << endl;
    }
    QPair *qpair = allocateQPair(8);

    int number_of_sectors = 1;
    int number_of_writes = 100000;

    uint32_t sector_size = qpair->get_sector_size();

    void* buffer = (char*)spdk_dma_zmalloc(number_of_sectors * qpair->get_sector_size(), 0x1000, NULL);
    bool is_complete;
    uint64_t start = ticks();
    for (int i = 0; i < number_of_writes; i++) {
        if (i == number_of_writes - 1)
            qpair->asynchronous_write(buffer, number_of_sectors * sector_size, rand() % 32432 + (uint64_t)i, &is_complete);
        else
            qpair->asynchronous_write(buffer, number_of_sectors * sector_size, rand() % 32432 + (uint64_t)i, 0);
    }

    while(!is_complete) {
        qpair->process_completions();
    }

    uint64_t cycles = ticks() - start;
    printf("total cycles: %ld, %ld IOPS.\n", cycles, 1000000 / cycles_to_microseconds(cycles / number_of_writes));
    spdk_dma_free(buffer);
    qpair->detach();
}
