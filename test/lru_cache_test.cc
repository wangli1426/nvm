//
// Created by Li Wang on 6/2/17.
//
#include <gtest/gtest.h>
#include <iostream>
#include <memory.h>
#include "../src/blk/blk_cache.h"
TEST(LRU_CACHE_TEST, WRITE_AND_GET) {
    blk_cache cache(512, 2);
    void* buffer = malloc(512);
    *reinterpret_cast<int*>(buffer) = 31485;
    blk_cache::cache_unit unit;
    cache.write(0, buffer, unit);

    void* read_buffer = malloc(512);
    cache.read(0, read_buffer);

    ASSERT_EQ(31485, *reinterpret_cast<int*>(read_buffer));

    ASSERT_FALSE(cache.read(1, read_buffer));

    free(buffer);
    free(read_buffer);
}

TEST(LRU_CACHE_TEST, EVIT) {
    blk_cache cache(512, 2);
    void* buffer = malloc(512);
    blk_cache::cache_unit unit;
    ASSERT_FALSE(cache.write(0, buffer, unit));
    ASSERT_FALSE(cache.write(1, buffer, unit));

    ASSERT_TRUE(cache.write(2, buffer, unit));
    ASSERT_EQ(0, unit.id);

    free(buffer);
}

TEST(LRU_CACHE_TEST, REORDER) {
    blk_cache cache(512, 4);
    void* buffer = malloc(512);
    blk_cache::cache_unit unit;
    ASSERT_FALSE(cache.write(0, buffer, unit));
    ASSERT_FALSE(cache.write(1, buffer, unit));
    ASSERT_FALSE(cache.write(2, buffer, unit));
    ASSERT_FALSE(cache.write(3, buffer, unit));

    ASSERT_EQ("{3, 2, 1, 0}", cache.keys_to_string());

    ASSERT_TRUE(cache.read(0, buffer));
    ASSERT_EQ("{0, 3, 2, 1}", cache.keys_to_string());

    ASSERT_TRUE(cache.read(2, buffer));
    ASSERT_EQ("{2, 0, 3, 1}", cache.keys_to_string());

    free(buffer);
}

