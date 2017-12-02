//
// Created by Li Wang on 6/2/17.
//
#include <gtest/gtest.h>
#include <iostream>
#include <memory.h>
#include "../../src/sync/lock_manager.h"
TEST(LOCK_MANAGER, SHARED_WRITE) {
    lock_manager manager;

    lock_descriptor s1, s2, w1;

    ASSERT_TRUE(manager.try_get_read_lock(0, s1));
    ASSERT_FALSE(manager.try_get_write_lock(0, w1));
    ASSERT_TRUE(manager.try_get_read_lock(0, s2));

    manager.release_lock(s1);
    manager.release_lock(s2);

    ASSERT_TRUE(manager.try_get_write_lock(0, w1));
    ASSERT_FALSE(manager.try_get_read_lock(0, s1));

}



