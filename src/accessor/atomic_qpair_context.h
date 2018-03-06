//
// Created by robert on 6/3/18.
//

#ifndef NVM_ATOMIC_QPAIR_CONTEXT_H_H
#define NVM_ATOMIC_QPAIR_CONTEXT_H_H

#endif //NVM_ATOMIC_QPAIR_CONTEXT_H_H
#include "ns_entry.h"
#include "qpair_context.h"
#include "../utils/sync.h"


namespace nvm{
    class AtomicQPair: public QPair {
    public:
        AtomicQPair(struct spdk_nvme_ctrlr *ctrlr, struct spdk_nvme_ns *ns, struct spdk_nvme_qpair *qpair,
                    int queue_length = 8): QPair(ctrlr, ns, qpair, queue_length){}
        virtual ~AtomicQPair() {
        }
        void synchronous_write(void* content, uint32_t size, uint64_t start_lba) {
            lock_.acquire();
            QPair::synchronous_write(content, size, start_lba);
            lock_.release();
        }

        void synchronous_read(void* buffer, uint32_t size, uint64_t start_lba) {
            lock_.acquire();
            QPair::synchronous_read(buffer, size, start_lba);
            lock_.release();
        }
    private:
        SpinLock lock_;
    };
}