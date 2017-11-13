//
// Created by robert on 24/10/17.
//

#ifndef NVM_BLK_H
#define NVM_BLK_H

#include <stdint.h>
#include "../tree/node_reference.h"
#include "../context/call_back.h"

//struct blk_address {
//    blk_address(uint32_t offset) {
//        id = offset;
//    }
//    uint32_t id;
//};

namespace tree {
    template<typename K, typename V>
    class node_reference;
}
using namespace tree;

typedef int64_t blk_address;
template <typename K, typename V>
class blk_accessor {

public:
    blk_accessor(const uint32_t size): block_size(size){
    }
    virtual ~blk_accessor() {};
    virtual node_reference<K, V>* allocate_ref() = 0;
    virtual node_reference<K, V>* create_null_ref() = 0;
    virtual int open() = 0;
    virtual blk_address allocate() = 0;
    virtual void deallocate(const blk_address& address) = 0;
    virtual int close() = 0;
    virtual int read(const blk_address &, void* buffer) = 0;
    virtual int write(const blk_address &, void* buffer) = 0;
    virtual void flush() = 0;

    virtual void* malloc_buffer() const {
        void* buffer;
        int status = posix_memalign(&buffer, block_size, block_size);
        if (status != 0) {
            printf("error in posix_memalign()\n");
        }
        return buffer;
    }
    virtual void free_buffer(void* buffer) const {
        free(buffer);
    }
    virtual void asynch_read(const blk_address& blk_addr, void* buffer, call_back_context* context) = 0;
    virtual int process_completion(int max = 1) = 0;
    const uint32_t block_size;
};

#endif //NVM_BLK_H
