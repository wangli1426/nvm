//
// Created by robert on 24/10/17.
//

#ifndef NVM_BLK_H
#define NVM_BLK_H


struct blk_address {
    uint32_t id;
};

class blk_accessor_abstract {

    virtual int open() = 0;
    virtual blk_address allocate() = 0;
    virtual int read(blk_address, void* buffer) = 0;
    virtual int write(blk_address, void* buffer) = 0;
    virtual int close() = 0;

};

#endif //NVM_BLK_H
