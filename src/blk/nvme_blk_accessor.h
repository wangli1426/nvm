//
// Created by robert on 7/11/17.
//

#ifndef NVM_NVME_BLK_ACCESSOR_H
#define NVM_NVME_BLK_ACCESSOR_H

#include <unordered_set>
#include <stdio.h>
#include "blk.h"
#include "../tree/blk_node_reference.h"
#include "../accessor/ns_entry.h"
#include "../accessor/qpair_context.h"
#include "../utils/rdtsc.h"
#include "../context/call_back.h"
#include "asynchronous_accessor.h"

namespace tree {
    template<typename K, typename V, int CAPACITY>
    class blk_node_reference;
}

using namespace nvm;

//#define __NVME_ACCESSOR_LOG__

template<typename K, typename V, int CAPACITY>
class nvme_blk_accessor: public blk_accessor<K, V> {
public:
    nvme_blk_accessor(const int& block_size): blk_accessor<K, V>(block_size) {
        writes_ = 0;
        write_cycles_ = 0;
        reads_ = 0;
        read_cycles_ = 0;
        cursor_ = 0;
        pending_commands_ = 0;
    };
    virtual node_reference<K, V>* allocate_ref() {
        blk_address addr = allocate();
        return new blk_node_reference<K, V, CAPACITY>(addr);
    };

    virtual node_reference<K, V>* create_null_ref() {
        return new blk_node_reference<K, V, CAPACITY>(-1);
    };

    virtual int open() {
        int status = nvm::nvm_utility::initialize_namespace();
        if (status != 0) {
            cout << "Errors in initialization of namespace." << endl;
            exit(1);
        } else {
            cout << "namespace is initialized." << endl;
        }
        qpair_ = nvm_utility::allocateQPair(1);
    }

    virtual blk_address allocate() {
        if (!freed_blk_addresses_.empty()) {
            auto it = freed_blk_addresses_.begin();
            blk_address blk_addr = *it;
            freed_blk_addresses_.erase(it);
            return blk_addr;
        } else {
            return blk_address(cursor_++);
        }
    }
    virtual void deallocate(const blk_address& address) {
        if (cursor_ == address - 1)
            cursor_ --;
        else {
            freed_blk_addresses_.insert(address);
        }
    }
    virtual int close() {
        qpair_->free_qpair();

        if (reads_ > 0)
            printf("[NVM:] total reads: %ld, average: %.2f us\n", reads_, cycles_to_microseconds(read_cycles_ / reads_));
        if (writes_ > 0)
            printf("[NVM:] total writes: %ld, average: %.2f us\n", writes_, cycles_to_microseconds(write_cycles_ / writes_));
    }
    virtual int read(const blk_address & blk_addr, void* buffer) {
        uint64_t start = ticks();
        qpair_->synchronous_read(buffer, this->block_size, blk_addr);
        read_cycles_ += ticks() - start;
        reads_++;
    }
    virtual int write(const blk_address & blk_addr, void* buffer) {
        uint64_t start = ticks();
        qpair_->synchronous_write(buffer, this->block_size, blk_addr);
        write_cycles_ += ticks() - start;
        writes_++;
    }

    virtual void* malloc_buffer() const {
        return spdk_dma_zmalloc(this->block_size, this->block_size, NULL);
    }
    virtual void free_buffer(void* buffer) const {
        spdk_dma_free(buffer);
    }

    void flush() {
    }

    void asynch_read(const blk_address& blk_addr, void* buffer, call_back_context* context) {
        nvme_callback_para* para = new nvme_callback_para;
        para->context = context;
        para->finished_context = &finished_contexts_;
        pending_commands_ ++;
        para->pending_command = &pending_commands_;
        int status = qpair_->submit_read_operation(buffer, this->block_size, blk_addr, context_call_back_function, para);
        if (status != 0) {
            printf("error in submitting read command\n");
            printf("blk_addr: %ld, block_size: %d\n", blk_addr, this->block_size);
            pending_commands_ --;
            return;
        }
#ifdef __NVME_ACCESSOR_LOG__
        printf("pending_commands_ added to %d.\n", pending_commands_);
#endif
    }

    int process_completion(int max = 1) {
        int32_t status = qpair_->process_completions(max);
        if (status < 0) {
            printf("errors in process_completions!\n");
            return status;
        }
#ifdef __NVME_ACCESSOR_LOG__
        printf("%d commands left.\n", pending_commands_);
        if (pending_commands_ < 0) {
            sleep(1);
        }
#endif
        int processed = finished_contexts_;
        finished_contexts_ = 0;
        return processed;
    }

    static void context_call_back_function(void* parms, const struct spdk_nvme_cpl *) {
        nvme_callback_para* para = reinterpret_cast<nvme_callback_para*>(parms);
        *para->pending_command -= 1;
        if (para->context->run() == CONTEXT_TERMINATED) {
//            *para->finished_context += 1;
            delete para->context;
            para->context = 0;
        }
        delete para;
    }

    struct nvme_callback_para {
        call_back_context* context;
        volatile int32_t* finished_context;
        volatile int32_t* pending_command;
    };
private:
    std::unordered_set<blk_address> freed_blk_addresses_;
    uint64_t cursor_;
    QPair* qpair_;
    uint64_t read_cycles_;
    uint64_t write_cycles_;
    uint64_t reads_;
    uint64_t writes_;
    volatile int32_t finished_contexts_;
    volatile int32_t pending_commands_;
};

#endif //NVM_NVME_BLK_ACCESSOR_H
