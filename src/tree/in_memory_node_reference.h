//
// Created by Li Wang on 10/27/17.
//

#ifndef NVM_IN_MEMORY_NODE_REFERENCE_H
#define NVM_IN_MEMORY_NODE_REFERENCE_H
#include "node_reference.h"
namespace tree{
    template <typename K, typename V>
    class in_memory_node_ref: public node_reference<K, V> {
    public:
        in_memory_node_ref(Node<K, V>* ref): ref_(ref) {

        }
        Node<K, V>* get() {
            return ref_;
        }

        void close() {

        }

        void remove() {

        }
    private:
        Node<K, V> *ref_;
    };
}
#endif //NVM_IN_MEMORY_NODE_REFERENCE_H
