//
// Created by robert on 20/10/17.
//

#ifndef NVM_IO_TASK_H
#define NVM_IO_TASK_H
#include "../accessor/ns_entry.h"
namespace nvm {
    enum access_pattern {
        seq, random
    };

    enum io_mode {
        synch, asynch
    };

    enum workload {
        read_load, write_load
    };

    void run_task(int number_of_accesses, int io_block_size, workload load, access_pattern pattern, io_mode mode, QPair* qpair) {

        int number_of_sectors = io_block_size / nvm_utility::get_sector_size();
        const uint64_t sector_number = nvm_utility::get_sector_number();
        uint32_t buffer_size = io_block_size;

        char *buffer = (char *) spdk_dma_zmalloc(io_block_size, 0x1000, NULL);
        snprintf(buffer, 0x1000, "%s", "Hello world!\n");
        bool is_complete = false;
        uint64_t start = ticks();
        for (int i = 0; i < number_of_accesses; i++) {
            uint64_t offset;
            if (pattern == seq)
                offset = i;
            else {
                offset = rand() % sector_number;
            }

            if (mode == asynch) {
                if (i == number_of_accesses - 1) {
                    if (load == read_load)
                        qpair->asynchronous_read(buffer, buffer_size, rand() % sector_number, &is_complete);
                    else
                        qpair->asynchronous_write(buffer, buffer_size, rand() % sector_number, &is_complete);
                } else {
                    if (load == read_load)
                        qpair->asynchronous_read(buffer, buffer_size, rand() % sector_number, 0);
                    else
                        qpair->asynchronous_write(buffer, buffer_size, rand() % sector_number, 0);
                }
            } else {
                if (load == read_load)
                    qpair->synchronous_read(buffer, buffer_size, rand() % sector_number);
                else
                    qpair->synchronous_write(buffer, buffer_size, rand() % sector_number);
            }
        }

        if (mode == asynch) {
            while (!is_complete) {
                qpair->process_completions();
            }
        }
        spdk_dma_free(buffer);
    }
}

#endif //NVM_IO_TASK_H
