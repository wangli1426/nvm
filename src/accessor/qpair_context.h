//
// Created by Li Wang on 10/17/17.
//

#ifndef NVM_QPAIR_CONTEXT_H
#define NVM_QPAIR_CONTEXT_H

#include <assert.h>
#include <spdk/nvme.h>
#include "ns_entry.h"
#include "../utils/sync.h"
#include "../utils/rdtsc.h"

//#define __LOG__

namespace nvm {

    static uint64_t total_cycles = 0;

    class QPair {
        struct cb_parameters {
            cb_parameters(QPair* qp, bool* _is_complete): qPair(qp), is_complete(_is_complete), start(0){};
            cb_parameters(QPair* qp, bool* _is_complete, uint64_t start_ticks) {
                qPair = qp;
                is_complete = _is_complete;
                start = start_ticks;
            }
            QPair* qPair;
            bool* is_complete;
            uint64_t start;
        };
    public:
        QPair(struct spdk_nvme_ctrlr *ctrlr, struct spdk_nvme_ns *ns, struct spdk_nvme_qpair *qpair,
              int queue_length = 8): ctrlr_(ctrlr), ns_(ns), qpair_(qpair), free_slots_(queue_length),
                                     queue_length_(queue_length) {
            sector_size_ = spdk_nvme_ns_get_sector_size(ns);
            completion_batch_size_ = queue_length / 1;
        }

        ~QPair() {
            free_qpair();
        }

        void synchronous_write(void* content, uint32_t size, uint64_t start_lba) {

            bool is_complete = false;
            cb_parameters* cba = new cb_parameters(this, &is_complete, ticks());
            free_slots_ --;
#ifdef __LOG__
            printf("start writing %llu\n", start_lba);
#endif
            int status = spdk_nvme_ns_cmd_write(ns_, qpair_, content, start_lba, size / sector_size_, QPair::cb_function,cba, 0);
            if (status < 0) {
                printf("error in spdk_nvme_ns_cmd_write()\n");
            }
#ifdef __LOG__
            printf("waiting...\n");
#endif
            while(!is_complete) {
                int status = process_completions();
                if (status < 0) {
                    printf("error in processing_completions()\n");
                }
            }
#ifdef __LOG__
            printf("finished writing %llu\n\n", start_lba);
#endif
        }

        void synchronous_read(void* buffer, uint32_t size, uint64_t start_lba) {
            bool is_complete =  false;
            cb_parameters* cbp = new cb_parameters(this, &is_complete, ticks());
            free_slots_ --;
#ifdef __LOG__
            printf("start reading %llu\n", start_lba);
#endif
            int status = spdk_nvme_ns_cmd_read(ns_, qpair_, buffer, start_lba, size / sector_size_, QPair::cb_function, cbp, 0);
            if (status < 0) {
                printf("error in spdk_nvme_ns_cmd_read()\n");
            }
#ifdef __LOG__
            printf("waiting...\n");
#endif
            while(!is_complete) {
                int32_t status = process_completions();
                if (status < 0) {
                    printf("error in processing_completions()\n");
                }
            }
#ifdef __LOG__
            printf("finished reading %llu\n\n", start_lba);
#endif
        }

        int submit_read_operation(void* buffer, uint32_t size, uint64_t start_lba, spdk_nvme_cmd_cb cb, void* args) {
           return spdk_nvme_ns_cmd_read(ns_, qpair_, buffer, start_lba, size / sector_size_, cb, args, 0);
        }

        int submit_write_operation(void* buffer, uint32_t size, uint64_t start_lba, spdk_nvme_cmd_cb cb, void* args) {
            return spdk_nvme_ns_cmd_write(ns_, qpair_, buffer, start_lba, size / sector_size_, cb, args, 0);
        }

        int asynchronous_write(void* content, uint32_t size, uint64_t start_lba, bool *is_complete) {
            cb_parameters* cba = new cb_parameters(this, is_complete, ticks());

            while(free_slots_ == 0){
                process_completions(completion_batch_size_);
            }
            free_slots_ --;

            spdk_nvme_ns_cmd_write(ns_,
                                   qpair_,
                                   content,
                                   start_lba,
                                   size / sector_size_,
                                   QPair::cb_function,
                                   cba,
                                   0);
        }

        int asynchronous_read(void* buffer, uint32_t size, uint64_t start_lba, bool *is_complete) {
            assert(size > 0 && size % sector_size_ == 0);
            cb_parameters* cba = new cb_parameters(this, is_complete, ticks());

            while(free_slots_ == 0){
                process_completions(completion_batch_size_);
            }
            free_slots_ --;

            spdk_nvme_ns_cmd_read(ns_,
                                  qpair_,
                                  buffer,
                                  start_lba,
                                  size / sector_size_,
                                  QPair::cb_function,
                                  cba,
                                  0);
        }

        inline int32_t process_completions(uint32_t max_completions = 0) {
            return spdk_nvme_qpair_process_completions(qpair_, max_completions);
        }

        int get_number_of_free_slots() const {
            return free_slots_;
        }

        static void cb_function(void * pars, const struct spdk_nvme_cpl *) {
            cb_parameters* para = static_cast<cb_parameters*>(pars);
            para->qPair->free_slots_ ++;
            if (para->is_complete) {
                *para->is_complete = true;
            }
            total_cycles += ticks() - para->start;
            delete para;
        }

        int free_qpair() {
            if (qpair_) {
                spdk_nvme_ctrlr_free_io_qpair(qpair_);
                qpair_ = 0;
            }
        }

    private:
        struct spdk_nvme_ctrlr	*ctrlr_;
        struct spdk_nvme_ns	*ns_;
        struct spdk_nvme_qpair *qpair_;
        int free_slots_;
        uint32_t completion_batch_size_;
        uint32_t queue_length_;
        uint32_t sector_size_;
    };
}
#endif //NVM_QPAIR_CONTEXT_H
