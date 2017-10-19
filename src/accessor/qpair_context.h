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


namespace nvm {

    static uint64_t total_cycles = 0;

    class QPair {
        struct cb_parameters {
            cb_parameters(QPair* qp, bool* _is_complete): cb_parameters(qp, _is_complete, 0){};
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
            completion_batch_size_ = queue_length / 8;
        }

        uint32_t get_sector_size() const {
            return sector_size_;
        }

        uint64_t get_sector_number() const {
            return spdk_nvme_ns_get_num_sectors(ns_);
        }

        void synchronous_write(void* content, uint32_t size, uint64_t start_lba) {

            bool is_complete = false;
            cb_parameters* cba = new cb_parameters(this, &is_complete, ticks());
            free_slots_ --;
            spdk_nvme_ns_cmd_write(ns_, qpair_, content, start_lba, size / sector_size_, QPair::cb_function,cba, 0);

            while(!is_complete) {
                process_completions();
            }
        }

        void synchronous_read(void* buffer, uint32_t size, uint64_t start_lba) {
            bool is_complete =  false;
            cb_parameters* cbp = new cb_parameters(this, &is_complete, ticks());
            free_slots_ --;
            spdk_nvme_ns_cmd_read(ns_, qpair_, buffer, start_lba, size / sector_size_, QPair::cb_function, cbp, 0);

            while(!is_complete) {
                process_completions();
            }
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

        inline int process_completions(uint32_t max_completions = 0) {
            spdk_nvme_qpair_process_completions(qpair_, max_completions);
        }

        void detach() {
            spdk_nvme_detach(ctrlr_);
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
