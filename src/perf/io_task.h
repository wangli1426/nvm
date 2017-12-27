//
// Created by robert on 20/10/17.
//

#ifndef NVM_IO_TASK_H
#define NVM_IO_TASK_H
#include "../accessor/ns_entry.h"
#include "access_pattern.h"
namespace nvm {

    void access(char* buffer, int number_of_accesses, int buffer_size, workload load, access_pattern* pattern, io_mode mode,
                QPair* qpair) {
        bool is_complete = false;
        for (int i = 0; i < number_of_accesses; i++) {

            if (mode == asynch) {
                if (i == number_of_accesses - 1) {
                    if (rand() % 100 >= load)
                        qpair->asynchronous_read(buffer, buffer_size, pattern->next_access(), &is_complete);
                    else
                        qpair->asynchronous_write(buffer, buffer_size, pattern->next_access(), &is_complete);
                } else {
                    if (rand() % 100 >= load)
                        qpair->asynchronous_read(buffer, buffer_size, pattern->next_access(), 0);
                    else
                        qpair->asynchronous_write(buffer, buffer_size, pattern->next_access(), 0);
                }
            } else {
                if (rand() % 100 >= load)
                    qpair->synchronous_read(buffer, buffer_size, pattern->next_access());
                else
                    qpair->synchronous_write(buffer, buffer_size, pattern->next_access());
            }
        }
        if (mode == asynch) {
            while (!is_complete) {
                qpair->process_completions();
            }
        }
    }

    void run_task(int number_of_accesses, int io_block_size, workload load, access_pattern* pattern, io_mode mode,
                  QPair* qpair) {

        int number_of_sectors = io_block_size / nvm_utility::get_sector_size();
        const uint64_t sector_number = nvm_utility::get_sector_number();
        uint32_t buffer_size = io_block_size;

        char *buffer = (char *) spdk_dma_zmalloc(io_block_size, 0x1000, NULL);
        snprintf(buffer, 0x1000, "%s", "Hello world!\n");

        access(buffer, number_of_accesses, io_block_size, load, pattern, mode, qpair);
        spdk_dma_free(buffer);
    }

    void run_task_with_timing(int number_of_accesses, int io_block_size, workload load, access_pattern* pattern, io_mode mode,
                  QPair* qpair, uint64_t& duration) {

        int number_of_sectors = io_block_size / nvm_utility::get_sector_size();
        const uint64_t sector_number = nvm_utility::get_sector_number();
        uint32_t buffer_size = io_block_size;

        char *buffer = (char *) spdk_dma_zmalloc(io_block_size, 0x1000, NULL);
        snprintf(buffer, 0x1000, "%s", "Hello world!\n");

        uint64_t start = ticks();
        access(buffer, number_of_accesses, io_block_size, load, pattern, mode, qpair);
        uint64_t  end = ticks();
        duration = end - start;
        spdk_dma_free(buffer);
    }
}

#endif //NVM_IO_TASK_H
