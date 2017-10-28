//
// Created by Li Wang on 10/27/17.
//

#ifndef NVM_NODE_REFERENCE_H
#define NVM_NODE_REFERENCE_H

#include "node.h"
namespace tree {
    template<typename K, typename V>
    class node_reference {
    public:
        // get an instance of the node referred to.
        virtual Node<K, V>* get() const {
            return nullptr;
        };

        // flush the change.
        virtual void close() const {};

        // remove the reference as well as the referred node.
        virtual void remove() {};
    };
}
#endif //NVM_NODE_REFERENCE_H
