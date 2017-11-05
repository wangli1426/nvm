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

        in_memory_node_ref(): ref_(0) {}

        in_memory_node_ref(Node<K, V>* ref): ref_(ref) {

        }
        Node<K, V>* get(blk_accessor<K, V>* blk_accessor) override {
            return ref_;
        }

        void close(blk_accessor<K, V>* blk_accessor, bool read_only = false) override{

        }

        void remove(blk_accessor<K, V>* blk_accessor) override {
//            delete ref_;
//            ref_ = 0;
        }

        void copy(node_reference<K, V>* ref) {
            if (ref)
                this->ref_ = dynamic_cast<in_memory_node_ref<K, V>*>(ref)->ref_;
            else
                this->ref_ = nullptr;
        }

        bool is_null_ptr() const override {
            return !ref_;
        };

        void bind(Node<K, V>* node) {
            ref_ = node;
        }

        int64_t get_unified_representation() {
            return reinterpret_cast<int64_t>(ref_);
        }

        void restore_by_unified_representation(int64_t value) {
            ref_ = reinterpret_cast<Node<K, V>*>(value);
        }
    private:
        Node<K, V> *ref_;
    private:
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & boost::serialization::base_object<node_reference<K, V>>(*this);
        }
    };
}
#endif //NVM_IN_MEMORY_NODE_REFERENCE_H
