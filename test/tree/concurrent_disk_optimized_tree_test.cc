//
// Created by Li Wang on 12/2/17.
//

#include <gtest/gtest.h>
#include <limits.h>
#include <vector>
#include <iostream>
#include <thread>
#include <set>
#include "../../src/tree/disk_optimized_b_plus_tree.h"
#include "../../src/perf/operation.h"
#include "../../src/perf/tree_operation_executor.h"

using namespace tree;
using namespace std;

TEST(ConcurrentDiskOptimizedBPlusTree, SingleThreadedInsertion) {
    printf("begin to execute SingleThreadedInsertion\n");
    disk_optimized_b_plus_tree<int, int, 16> tree("tree.dat", 256);
    tree.init();
    vector<int> keys;
    vector<operation<int, int> > operations;
    const int tuples = 10000;

    for(int i = 0; i < tuples; i++) {
        keys.push_back(i);
    }

    std::random_shuffle(&keys[0], &keys[tuples]);

    for (auto it = keys.cbegin(); it != keys.cend(); ++it) {
        operation<int, int> op;
        op.key = *it;
        op.val = *it;
        op.type = WRITE_OP;
        operations.push_back(op);
    }

    execute_operations<int, int>(&tree, operations.begin(), operations.end());

    tree.sync();

    for (auto it = keys.cbegin(); it != keys.cend(); ++it) {
        int value;
        tree.search(*it, value);
        ASSERT_EQ(*it, value);
    }
    tree.sync();
    tree.close();
}

TEST(ConcurrentDiskOptimizedBPlusTree, MultipleThreadedInsertion) {
    printf("begin to execute MultipleThreadedInsertion\n");
    disk_optimized_b_plus_tree<int, int, 16> tree("tree.dat", 256);
    tree.init();
    vector<int> keys;
    vector<operation<int, int> > operations;
    const int tuples = 10000;
    const int threads = 64;
    std::thread tid[threads];

    for(int i = 0; i < tuples; i++) {
        keys.push_back(i);
    }

    std::random_shuffle(&keys[0], &keys[tuples]);

    for (auto it = keys.cbegin(); it != keys.cend(); ++it) {
        operation<int, int> op;
        op.key = *it;
        op.val = *it;
        op.type = WRITE_OP;
        operations.push_back(op);
    }

    const int tuples_per_thread = tuples / threads;
    for (int i = 0; i < threads; i++) {
        tid[i] = std::thread(&execute_operations<int, int>, &tree, operations.begin() + i * tuples_per_thread, operations.begin() + (i + 1) * tuples_per_thread);
    }

    for (int i = 0; i < threads; i++) {
        tid[i].join();
    }

    tree.sync();

    for (auto it = keys.cbegin(); it != keys.cend(); ++it) {
        int value;
        tree.search(*it, value);
        ASSERT_EQ(*it, value);
    }
    tree.sync();
    tree.close();
}

TEST(ConcurrentDiskOptimizedBPlusTree, MultipleThreadedInsertionAndSearch) {
    printf("begin to execute MultipleThreadedInsertionAndSearch\n");
    disk_optimized_b_plus_tree<int, int, 16> tree("tree.dat", 256);
    tree.init();
    vector<int> keys;
    vector<operation<int, int> > operations;
    const int tuples = 100000;
    const int searches = 100000;
    const int threads = 64;
    std::thread tid[threads];

    for(int i = 0; i < tuples; i++) {
        keys.push_back(i);
    }

    std::random_shuffle(&keys[0], &keys[tuples]);

    for (auto it = keys.cbegin(); it != keys.cend(); ++it) {
        operation<int, int> op;
        op.key = *it;
        op.val = *it;
        op.type = WRITE_OP;
        operations.push_back(op);
    }

    // add search operations
    for (int i = 0; i < searches; i++) {
        operation<int, int> op;
        op.key = i;
        op.type = READ_OP;
        operations.push_back(op);
    }

    const int tuples_per_thread = operations.size() / threads;
    for (int i = 0; i < threads; i++) {
        tid[i] = std::thread(&execute_operations<int, int>, &tree, operations.begin() + i * tuples_per_thread, operations.begin() + (i + 1) * tuples_per_thread);
    }

    for (int i = 0; i < threads; i++) {
        tid[i].join();
    }

    tree.sync();

    for (auto it = keys.cbegin(); it != keys.cend(); ++it) {
        int value;
        tree.search(*it, value);
        ASSERT_EQ(*it, value);
    }
    tree.sync();
    tree.close();
}

TEST(ConcurrentDiskOptimizedBPlusTree, MultipleThreadedInsertionAndSearchWithLargeBlockSize) {
    printf("begin to execute MultipleThreadedInsertionAndSearchWithLargeBlockSize\n");
    disk_optimized_b_plus_tree<int, int, 128> tree("tree.dat", 256, 4096);
    tree.init();
    vector<int> keys;
    vector<operation<int, int> > operations;
    const int tuples = 10000;
    const int searches = 10000;
    const int threads = 64;
    std::thread tid[threads];

    for(int i = 0; i < tuples; i++) {
        keys.push_back(i);
    }

    std::random_shuffle(&keys[0], &keys[tuples]);

    for (auto it = keys.cbegin(); it != keys.cend(); ++it) {
        operation<int, int> op;
        op.key = *it;
        op.val = *it;
        op.type = WRITE_OP;
        operations.push_back(op);
    }

    // add search operations
    for (int i = 0; i < searches; i++) {
        operation<int, int> op;
        op.key = i;
        op.type = READ_OP;
        operations.push_back(op);
    }

    const int tuples_per_thread = operations.size() / threads;
    for (int i = 0; i < threads; i++) {
        tid[i] = std::thread(&execute_operations<int, int>, &tree, operations.begin() + i * tuples_per_thread, operations.begin() + (i + 1) * tuples_per_thread);
    }

    for (int i = 0; i < threads; i++) {
        tid[i].join();
    }

    tree.sync();

    for (auto it = keys.cbegin(); it != keys.cend(); ++it) {
        int value;
        tree.search(*it, value);
        ASSERT_EQ(*it, value);
    }
    tree.sync();
    tree.close();
}