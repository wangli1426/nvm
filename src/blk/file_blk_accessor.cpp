//
// Created by robert on 24/10/17.
//

#include "file_blk_accessor.h"

file_blk_accessor::file_blk_accessor(const char *path): path_(path) {

}

int file_blk_accessor::open() {
    file_ = fopen(path_, "w+");
}

blk_address file_blk_accessor::allocate() {
    return nullptr;
}

int file_blk_accessor::read(blk_address address, void *buffer) {
    return 0;
}

int file_blk_accessor::write(blk_address address, void *buffer) {
    return 0;
}

int file_blk_accessor::close() {
    return 0;
}

