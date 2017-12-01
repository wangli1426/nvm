//
// Created by Li Wang on 6/2/17.
//
#include <gtest/gtest.h>
#include <iostream>
#include <memory.h>
#include "../../src/sync/rwlock.h"
TEST(RWLOCK, SHARED) {
    rwlock lock;
    ASSERT_TRUE(lock.try_read_lock());
    ASSERT_TRUE(lock.try_read_lock());
    ASSERT_TRUE(lock.try_read_lock());
    ASSERT_FALSE(lock.try_write_lock());
    lock.read_unlock();
    lock.read_unlock();
    lock.read_unlock();
    ASSERT_TRUE(lock.try_write_lock());
    ASSERT_FALSE(lock.try_read_lock());
    ASSERT_FALSE(lock.try_write_lock());

}

TEST(RWLOCK, READLOCK_IDEMPOTENT) {
    rwlock lock;
    ASSERT_TRUE(lock.try_read_lock());
    ASSERT_TRUE(lock.try_read_lock());
    ASSERT_FALSE(lock.try_write_lock());
    lock.read_unlock();
    ASSERT_FALSE(lock.try_write_lock());
    lock.read_unlock();
    ASSERT_TRUE(lock.try_write_lock());
}

TEST(RWLOCK, UPGRADE) {
    rwlock lock;
    ASSERT_TRUE(lock.try_lock_upgrade());
    ASSERT_TRUE(lock.try_read_lock());
    ASSERT_FALSE(lock.try_unlock_and_write_lock());
    lock.read_unlock();
    ASSERT_TRUE(lock.try_unlock_and_write_lock());
}

