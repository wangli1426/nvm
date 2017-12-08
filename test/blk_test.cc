//
// Created by Li Wang on 6/2/17.
//
#include <gtest/gtest.h>
#include <iostream>
#include <memory.h>
#include "../src/blk/file_blk_accessor.h"
TEST(FILE_BLK_ACCESSOR, OPEN_AND_CLOSE) {
    file_blk_accessor<int, int, 32> accessor("./tmp", 512);
    ASSERT_EQ(0, accessor.open());
    ASSERT_EQ(0, accessor.close());
}

TEST(FILE_BLK_ACCESSOR, ALLOCAT_WRITE_AND_READ) {
    file_blk_accessor<int, int, 32> accessor("./tmp", 512);
    accessor.open();
    blk_address addr = accessor.allocate();
    void* write_buffer = accessor.malloc_buffer();
    memset(write_buffer, 512, 1);
    void* read_buffer = accessor.malloc_buffer();

    ASSERT_EQ(512, accessor.write(addr, write_buffer));
    ASSERT_EQ(512, accessor.read(addr, read_buffer));

    ASSERT_TRUE(memcmp(write_buffer, read_buffer, 512) == 0);

    ASSERT_EQ(0, accessor.close());
    accessor.free_buffer(write_buffer);
    accessor.free_buffer(read_buffer);
}

TEST(FILE_BLK_ACCESSOR, INVALID_ACCESS) {
    file_blk_accessor<int, int, 32> accessor("./tmp", 512);
    accessor.open();
    blk_address address1 = accessor.allocate();

    blk_address address2 = accessor.allocate();

    accessor.deallocate(address1);

    void* write_buffer = accessor.malloc_buffer();
    memset(write_buffer, 512, 1);
    ASSERT_EQ(0, accessor.write(address1, write_buffer));

    accessor.close();
    accessor.free_buffer(write_buffer);
}

TEST(FILE_BLK_ACCESSOR, CONTEXT_VALIDATION) {
    file_blk_accessor<int, int, 32> accessor("./tmp", 512);
    accessor.open();
    blk_address  address = accessor.allocate();

    void* write_buffer = accessor.malloc_buffer();
    memset(write_buffer, 512, 1);
    void* read_buffer = accessor.malloc_buffer();
    * static_cast<uint32_t*>(write_buffer) = 100;

    accessor.write(address, write_buffer);
    accessor.read(address, read_buffer);

    ASSERT_EQ(100, * static_cast<uint32_t*>(read_buffer));

    accessor.free_buffer(write_buffer);
    accessor.free_buffer(read_buffer);

}
