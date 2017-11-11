//
// Created by Li Wang on 11/11/17.
//

#ifndef NVM_ASYNCHRONOUS_ACCESSOR_H
#define NVM_ASYNCHRONOUS_ACCESSOR_H
class asynchronous_accessor {
    virtual void asynch_read(const blk_address& blk_addr, void* buffer, call_back_context* context) = 0;
};
#endif //NVM_ASYNCHRONOUS_ACCESSOR_H
