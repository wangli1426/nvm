//
// Created by robert on 24/10/17.
//

#ifndef NVM_FILE_BLK_ACCESSOR_H
#define NVM_FILE_BLK_ACCESSOR_H
#include <stdio.h>
#include "blk.h"

class file_blk_accessor: public blk_accessor_abstract {
public:
    explicit file_blk_accessor(const char* path);
    int open() override;

    blk_address allocate() override;

    int read(blk_address address, void *buffer) override;

    int write(blk_address address, void *buffer) override;

    int close() override;

private:
    const char* path_;
    FILE* file_;
};


#endif //NVM_FILE_BLK_ACCESSOR_H
