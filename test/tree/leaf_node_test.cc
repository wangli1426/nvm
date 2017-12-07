//
// Created by Li Wang on 6/2/17.
//
#include <gtest/gtest.h>
#include <iostream>
#include "../../src/tree/leaf_node.h"
#include "../../src/blk/void_blk_accessor.h"

using namespace tree;
TEST(LeafNode, InsertionWithOverflow) {
    blk_accessor<int, int>* blk_accessor = new void_blk_accessor<int, int, 5>(512);
    blk_accessor->open();
    LeafNode<int, int, 5> leafNode(blk_accessor);
    leafNode.insert(1, 1);
    leafNode.insert(3, 2);
    leafNode.insert(2, 5);
    leafNode.insert(-1, 5);
    leafNode.insert(6, 2);
    EXPECT_EQ("(-1,5) (1,1) (2,5) (3,2) (6,2)", leafNode.toString());
    blk_accessor->close();
    delete blk_accessor;
}

TEST(LeafNode, InsertionWithOverflowDouble) {
    blk_accessor<double, int>* blk_accessor = new void_blk_accessor<double, int, 5>(512);
    blk_accessor->open();
    LeafNode<double, int, 5> leafNode(blk_accessor);
    leafNode.insert(1.3, 1);
    leafNode.insert(3.2, 2);
    leafNode.insert(2.5, 5);
    leafNode.insert(-1.0, 5);
    leafNode.insert(-6.4, 2);
    EXPECT_EQ("(-6.4,2) (-1,5) (1.3,1) (2.5,5) (3.2,2)", leafNode.toString());
    blk_accessor->close();
    delete blk_accessor;
}

TEST(LeafNode, InsertionAndUpdate) {
    blk_accessor<double, int>* blk_accessor = new void_blk_accessor<double, int, 5>(512);
    blk_accessor->open();
    LeafNode<double, int, 5> leaf_node(blk_accessor);
    leaf_node.insert(1, 2);
    leaf_node.insert(1, 3);
    EXPECT_EQ("(1,3)", leaf_node.toString());
    blk_accessor->close();
    delete blk_accessor;
}

TEST(LeafNode, Search) {
    blk_accessor<double, int>* blk_accessor = new void_blk_accessor<double, int, 5>(512);
    blk_accessor->open();
    LeafNode<double, int, 5> leaf_node(blk_accessor);
    leaf_node.insert(1, 5);
    leaf_node.insert(5, 8);
    leaf_node.insert(-1, 222);
    int result;
    EXPECT_EQ(true, leaf_node.search(1, result));
    EXPECT_EQ(5, result);

    EXPECT_EQ(true, leaf_node.search(5, result));
    EXPECT_EQ(8, result);

    EXPECT_EQ(true, leaf_node.search(-1, result));
    EXPECT_EQ(222, result);
    blk_accessor->close();
    delete blk_accessor;
}

TEST(LeafNode, Update) {
    blk_accessor<double, int>* blk_accessor = new void_blk_accessor<double, int, 5>(512);
    blk_accessor->open();
    LeafNode<double, int, 5> leaf_node(blk_accessor);
    leaf_node.insert(5, 5);
    leaf_node.insert(9, 10);
    leaf_node.insert(3, 100);

    int value;

    EXPECT_EQ(true, leaf_node.update(5, 100));
    leaf_node.search(5, value);
    EXPECT_EQ(100, value);

    EXPECT_EQ(true, leaf_node.update(3, 20));
    leaf_node.search(3, value);
    EXPECT_EQ(20, value);

    EXPECT_EQ(true, leaf_node.update(9, 44));
    leaf_node.search(9, value);
    EXPECT_EQ(44, value);
    blk_accessor->close();
    delete blk_accessor;
}

TEST(LeafNode, Delete) {
    blk_accessor<double, int>* blk_accessor = new void_blk_accessor<double, int, 5>(512);
    blk_accessor->open();
    LeafNode<double, int, 5> leaf_node(blk_accessor);
    leaf_node.insert(5, 5);
    leaf_node.insert(6, 6);
    leaf_node.insert(7, 7);
    leaf_node.insert(3, 3);

    EXPECT_EQ(true, leaf_node.delete_key(6));
    EXPECT_EQ("(3,3) (5,5) (7,7)", leaf_node.toString());

    EXPECT_EQ(false, leaf_node.delete_key(6));

    EXPECT_EQ(true, leaf_node.delete_key(3));
    EXPECT_EQ("(5,5) (7,7)", leaf_node.toString());

    EXPECT_EQ(true, leaf_node.delete_key(7));
    EXPECT_EQ("(5,5)", leaf_node.toString());

    EXPECT_EQ(true, leaf_node.delete_key(5));
    EXPECT_EQ("", leaf_node.toString());

    blk_accessor->close();
    delete blk_accessor;
}

TEST(LeafNode, SplitInsertLeft) {
    blk_accessor<int, int>* blk_accessor = new void_blk_accessor<int, int, 4>(512);
    blk_accessor->open();
    LeafNode<int, int, 4> *leaf = new LeafNode<int, int, 4>(blk_accessor);
    Split<int, int> split;
    EXPECT_EQ(false, leaf->insert_with_split_support(3, 3, split));
    EXPECT_EQ(false, leaf->insert_with_split_support(4, 4, split));
    EXPECT_EQ(false, leaf->insert_with_split_support(6, 6, split));
    EXPECT_EQ(false, leaf->insert_with_split_support(5, 5, split));

    EXPECT_EQ(true, leaf->insert_with_split_support(1, 1, split));
    EXPECT_EQ("(1,1) (3,3) (4,4)", split.left->toString());
    EXPECT_EQ("(5,5) (6,6)", split.right->toString());
    delete split.left;
    delete split.right;
    blk_accessor->close();
    delete blk_accessor;
}

TEST(LeafNode, SplitInsertRight) {
    blk_accessor<int, int>* blk_accessor = new void_blk_accessor<int, int, 4>(512);
    blk_accessor->open();
    LeafNode<int, int, 4> *leaf = new LeafNode<int, int, 4>(blk_accessor);
    Split<int, int> split;
    EXPECT_EQ(false, leaf->insert_with_split_support(3, 3, split));
    EXPECT_EQ(false, leaf->insert_with_split_support(4, 4, split));
    EXPECT_EQ(false, leaf->insert_with_split_support(6, 6, split));
    EXPECT_EQ(false, leaf->insert_with_split_support(5, 5, split));

    EXPECT_EQ(true, leaf->insert_with_split_support(7, 7, split));
    EXPECT_EQ("(3,3) (4,4)", split.left->toString());
    EXPECT_EQ("(5,5) (6,6) (7,7)", split.right->toString());
    EXPECT_EQ(5, split.boundary_key);
    delete split.left;
    delete split.right;
    blk_accessor->close();
    delete blk_accessor;
}