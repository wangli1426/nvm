//
// Created by robert on 7/11/17.
//

#ifndef NVM_NVME_BLK_ACCESSOR_H
#define NVM_NVME_BLK_ACCESSOR_H

#include <unordered_set>
#include "blk.h"
#include "../tree/blk_node_reference.h"
#include "../accessor/ns_entry.h"
#include "../accessor/qpair_context.h"

namespace tree {
    template<typename K, typename V, int CAPACITY>
    class blk_node_reference;
}

using namespace nvm;

template<typename K, typename V, int CAPACITY>
class nvme_blk_accessor: public blk_accessor<K, V> {
public:
    nvme_blk_accessor(const int& block_size): blk_accessor<K, V>(block_size) {};
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
        qpair_ = nvm_utility::allocateQPair(8);
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
    }
    virtual int read(const blk_address & blk_addr, void* buffer) {
        qpair_->synchronous_read(buffer, this->block_size, blk_addr);
    }
    virtual int write(const blk_address & blk_addr, void* buffer) {
        qpair_->synchronous_write(buffer, this->block_size, blk_addr);
    }

private:
    std::unordered_set<blk_address> freed_blk_addresses_;
    uint32_t cursor_;
    QPair* qpair_;
};

#endif //NVM_NVME_BLK_ACCESSOR_H
