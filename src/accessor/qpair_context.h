//
// Created by Li Wang on 10/17/17.
//

#ifndef NVM_QPAIR_CONTEXT_H
#define NVM_QPAIR_CONTEXT_H

#include <assert.h>
#include <spdk/nvme.h>
#include "ns_entry.h"
#include "../utils/sync.h"


namespace nvm {
    class QPair {
        struct cb_parameters {
            cb_parameters(QPair* qp, bool* _is_complete) {
                qPair = qp;
                is_complete = _is_complete;
            }
            QPair* qPair;
            bool* is_complete;
        };
    public:
        QPair(struct spdk_nvme_ctrlr *ctrlr, struct spdk_nvme_ns *ns, struct spdk_nvme_qpair *qpair,
              int queue_length = 8): ctrlr_(ctrlr), ns_(ns), qpair_(qpair) {
            sector_size_ = spdk_nvme_ns_get_sector_size(ns);
            semaphore_ = Semaphore(queue_length);
        }

        uint32_t get_sector_size() const {
            return sector_size_;
        }

        int asynchronous_write(void* content, uint32_t size, uint64_t start_lba, bool *is_complete) {
            assert(size > 0 && size % sector_size_ == 0);
            cb_parameters* cba = new cb_parameters(this, is_complete);
            semaphore_.wait();
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
            cb_parameters* cba = new cb_parameters(this, is_complete);
            semaphore_.wait();
            spdk_nvme_ns_cmd_read(ns_,
                                  qpair_,
                                  buffer,
                                  start_lba,
                                  size / sector_size_,
                                  QPair::cb_function,
                                  cba,
                                  0);
        }

        int process_completions() {
            spdk_nvme_qpair_process_completions(qpair_, 0);
        }

        void detach() {
            spdk_nvme_detach(ctrlr_);
        }

        static void cb_function(void * pars, const struct spdk_nvme_cpl *) {
            cb_parameters* para = static_cast<cb_parameters*>(pars);
            para->qPair->semaphore_.post();
            if (para->is_complete) {
                *para->is_complete = true;
            }
        }

    private:
        struct spdk_nvme_ctrlr	*ctrlr_;
        struct spdk_nvme_ns	*ns_;
        struct spdk_nvme_qpair *qpair_;
        Semaphore semaphore_;
        uint32_t sector_size_;
    };
}
#endif //NVM_QPAIR_CONTEXT_H
