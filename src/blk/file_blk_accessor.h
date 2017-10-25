//
// Created by robert on 24/10/17.
//

#ifndef NVM_FILE_BLK_ACCESSOR_H
#define NVM_FILE_BLK_ACCESSOR_H
#include <stdio.h>
#include <unordered_set>
#include "blk.h"

using namespace std;
class file_blk_accessor: public blk_accessor {
public:
    explicit file_blk_accessor(const char* path, const uint32_t block_size);
    int open() override;

    blk_address allocate() override;

    void deallocate(const blk_address& address) override;

    int read(const blk_address& address, void *buffer) override;

    int write(const blk_address& address, void *buffer) override;

    int close() override;
private:
    bool is_address_valid(const blk_address& address) const;

private:
    const char* path_;
    FILE* file_;
    std::unordered_set<blk_address> freed_blk_addresses_;
    uint32_t cursor_;
};


#endif //NVM_FILE_BLK_ACCESSOR_H
