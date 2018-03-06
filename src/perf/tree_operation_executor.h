//
// Created by Li Wang on 12/2/17.
//

#ifndef NVM_TREE_OPERATION_EXECUTOR_H
#define NVM_TREE_OPERATION_EXECUTOR_H

#include <vector>
#include "operation.h"
#include "../tree/b_tree.h"

using namespace tree;
using namespace std;

template<typename K, typename V>
void execute_operations(BTree<K, V> *tree, typename vector<operation<K, V> >::iterator begin_iterator, typename vector<operation<K,V> >::iterator end_iterator) {
    int count = 0;

    for (auto it = begin_iterator; it < end_iterator; ++it) {
        switch(it->type) {
            case READ_OP:
                tree->search(it->key, it->val);
                break;
            case WRITE_OP:
                tree->insert(it->key, it->val);
                break;
        }
        count ++;
    }
    tree->deregister_thread();
}

#endif //NVM_TREE_OPERATION_EXECUTOR_H
