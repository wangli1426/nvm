//
// Created by Li Wang on 6/2/17.
//
#include <gtest/gtest.h>
#include "../../src/scheduler/ready_state_estimator.h"
TEST(READY_STATE_ESTIMATOR, ESTIMATE_READ) {
    ready_state_estimator estimator(10, 20);
    estimator.register_read_io(0, 0);
    ASSERT_EQ(0, estimator.estimate_number_of_ready_states(5));
    ASSERT_EQ(1, estimator.estimate_number_of_ready_states(10));
    ASSERT_EQ(1, estimator.estimate_number_of_ready_states(15));
    estimator.remove_read_io(0);
    ASSERT_EQ(0, estimator.estimate_number_of_ready_states(15));
}

TEST(READY_STATE_ESTIMATOR, ESTIMATE_MIXED) {
    ready_state_estimator estimator(10, 20);
    estimator.register_read_io(0, 0);
    ASSERT_EQ(0, estimator.estimate_number_of_ready_states(5));
    ASSERT_EQ(1, estimator.estimate_number_of_ready_states(10));
    ASSERT_EQ(1, estimator.estimate_number_of_ready_states(15));
    estimator.register_write_io(2, 1);
    estimator.register_read_io(3, 3);

    ASSERT_EQ(2, estimator.estimate_number_of_ready_states(15));
    ASSERT_EQ(3, estimator.estimate_number_of_ready_states(25));

    ASSERT_EQ(0, estimator.estimate_number_of_write_states(15));
    ASSERT_EQ(1, estimator.estimate_number_of_write_states(25));

    estimator.remove_write_io(2);
    ASSERT_EQ(2, estimator.estimate_number_of_ready_states(25));
    ASSERT_EQ(0, estimator.estimate_number_of_write_states(25));

    ASSERT_EQ(2, estimator.get_number_of_pending_state());
    ASSERT_EQ(0, estimator.get_number_of_pending_write_state());

    estimator.remove_read_io(0);
    estimator.remove_read_io(3);
    ASSERT_EQ(0, estimator.get_number_of_pending_state());
}

TEST(READY_STATE_ESTIMATOR, EXPECTED_DURATION_FOR_EXPECTED_READY_STATES) {
    ready_state_estimator estimator(10, 20);
    estimator.register_read_io(0, 0);
    estimator.register_read_io(1, 1);
    estimator.register_read_io(2, 2);
    estimator.register_read_io(3, 3);
    estimator.register_write_io(4, 4);

    ASSERT_EQ(13, estimator.estimate_the_time_to_get_desirable_ready_state(4, 0));
    ASSERT_EQ(13, estimator.estimate_the_time_to_get_desirable_ready_state(4, 5));

    ASSERT_EQ(24, estimator.estimate_the_time_to_get_desirable_ready_state(5, 20));
    ASSERT_EQ(24, estimator.estimate_the_time_to_get_desirable_ready_state(5, 2000));

    ASSERT_EQ(24, estimator.estimate_the_time_to_get_desirable_ready_write_state(1, 0));
    estimator.remove_write_io(4);
    ASSERT_EQ(INT64_MAX, estimator.estimate_the_time_to_get_desirable_ready_write_state(1, 0));
    ASSERT_EQ(0, estimator.get_number_of_pending_write_state());
}

