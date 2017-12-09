//
// Created by robert on 31/10/17.
//

#ifndef NVM_VOID_BLK_ACCESSOR_H
#define NVM_VOID_BLK_ACCESSOR_H


#include "blk.h"
#include "../tree/in_memory_node_reference.h"

namespace tree {
    template<typename K, typename V>
    class in_memory_node_ref;
}

template<typename K, typename V, int CAPACITY>
class void_blk_accessor: public blk_accessor<K, V> {
public:
    void_blk_accessor(const uint32_t size) : blk_accessor<K, V>(size) {}

    int open() override {
        return 0;
    }

    blk_address allocate() override {
        return 0;
    }

    void deallocate(const blk_address &address) override {

    }

    int close() override {
        return 0;
    }

    int read(const blk_address &address, void *buffer) override {
        return 0;
    }

    int write(const blk_address &address, void *buffer) override {
        return 0;
    }

    node_reference<K, V> *allocate_ref() override {
        return new in_memory_node_ref<K, V>(0);
    }

    node_reference<K, V>* create_null_ref() {
        return allocate_ref();
    };

    void flush() {
    }

    virtual void asynch_read(const blk_address& blk_addr, void* buffer, call_back_context* context) {

    }

    void asynch_write(const blk_address& blk_addr, void* buffer, call_back_context* context) {

    }

    int process_completion(int max = 1) {
        return 0;
    }
};


#endif //NVM_VOID_BLK_ACCESSOR_H
