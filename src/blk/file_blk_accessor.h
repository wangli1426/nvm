//
// Created by robert on 24/10/17.
//

#ifndef NVM_FILE_BLK_ACCESSOR_H
#define NVM_FILE_BLK_ACCESSOR_H
#include <stdio.h>
#include <unordered_set>
#include "../tree/blk_node_reference.h"
#include "blk.h"
namespace tree {
    template<typename K, typename V, int CAPACITY>
    class blk_node_reference;
}

using namespace std;
template<typename K, typename V, int CAPACITY>
class file_blk_accessor: public blk_accessor<K, V> {
public:
    explicit file_blk_accessor(const char* path, const uint32_t& block_size) : path_(path), blk_accessor<K, V>(block_size),
                                                                               cursor_(0) {
    }

    int open() override{
        file_ = fopen(path_, "w+");
        return file_ ? 0 : errno;
    }

    blk_address allocate() override {
        if (!freed_blk_addresses_.empty()) {
            auto it = freed_blk_addresses_.begin();
            blk_address blk_addr = *it;
            freed_blk_addresses_.erase(it);
            return blk_addr;
        } else {
            return blk_address(cursor_++);
        }
    }

    void deallocate(const blk_address& address) override {
        if (cursor_ == address - 1)
            cursor_ --;
        else {
            freed_blk_addresses_.insert(address);
        }
    }

    int read(const blk_address& address, void *buffer) override {
        if (!is_address_valid(address))
            return 0;
        if (fseek(file_, address * this->block_size, SEEK_SET) != 0)
            return 0;
        return (int)fread(buffer, 1, this->block_size, file_);
    }

    int write(const blk_address& address, void *buffer) override {
        if (!is_address_valid(address))
            return 0;
        if (fseek(file_, address * this->block_size, SEEK_SET) != 0)
            return 0;
        int status = (int)fwrite(buffer, this->block_size, 1, file_);
        fflush(file_);
        return status;
    }

    int close() override {
        return fclose(file_);
    }

    node_reference<K, V>* allocate_ref() override {
        blk_address addr = allocate();
        return new blk_node_reference<K, V, CAPACITY>(addr);
    }

    node_reference<K, V>* create_null_ref() {
        return new blk_node_reference<K, V, CAPACITY>(-1);
    };

private:
    bool is_address_valid(const blk_address& address) const {
        return address < cursor_ && freed_blk_addresses_.find(address) == freed_blk_addresses_.cend();
    }

private:
    const char* path_;
    FILE* file_;
    std::unordered_set<blk_address> freed_blk_addresses_;
    uint32_t cursor_;
};


#endif //NVM_FILE_BLK_ACCESSOR_H
