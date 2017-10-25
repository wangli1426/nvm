//
// Created by robert on 24/10/17.
//

#include <sys/errno.h>
#include "file_blk_accessor.h"

file_blk_accessor::file_blk_accessor(const char *path, const uint32_t block_size) : path_(path), blk_accessor(block_size),
                                                                               cursor_(0) {
}

int file_blk_accessor::open() {
    file_ = fopen(path_, "w+");
    return file_ ? 0 : errno;
}

blk_address file_blk_accessor::allocate() {
    if (!freed_blk_addresses_.empty()) {
        auto it = freed_blk_addresses_.begin();
        blk_address blk_addr = *it;
        freed_blk_addresses_.erase(it);
        return blk_addr;
    } else {
        return blk_address(cursor_++);
    }
}

void file_blk_accessor::deallocate(const blk_address& address) {
    if (cursor_ == address - 1)
        cursor_ --;
    else {
        freed_blk_addresses_.insert(address);
    }
}

int file_blk_accessor::read(const blk_address& address, void *buffer) {
    if (!is_address_valid(address))
        return 0;
    if (fseek(file_, address * block_size, SEEK_SET) != 0)
        return 0;
    return (int)fread(buffer, 1, block_size, file_);
}

int file_blk_accessor::write(const blk_address& address, void *buffer) {
    if (!is_address_valid(address))
        return 0;
    if (fseek(file_, address * block_size, SEEK_SET) != 0)
        return 0;
    int status = (int)fwrite(buffer, 1, block_size, file_);
    fflush(file_);
    return status;
}

int file_blk_accessor::close() {
    return fclose(file_);
}

bool file_blk_accessor::is_address_valid(const blk_address& address) const {
    return address < cursor_ && freed_blk_addresses_.find(address) == freed_blk_addresses_.cend();
}

