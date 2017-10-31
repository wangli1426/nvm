//
// Created by robert on 31/10/17.
//

#ifndef NVM_VOID_BLK_ACCESSOR_H
#define NVM_VOID_BLK_ACCESSOR_H


#include "blk.h"
template<typename K, typename V, int CAPACITY>
class void_blk_accessor: public blk_accessor<K, V, CAPACITY> {
public:
    void_blk_accessor(const uint32_t size) : blk_accessor<K, V, CAPACITY>(size) {}

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

};


#endif //NVM_VOID_BLK_ACCESSOR_H
