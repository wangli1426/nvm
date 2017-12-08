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

        in_memory_node_ref(): rep_(-1), instance_(0) {}

        in_memory_node_ref(Node<K, V>* ref): instance_(ref) {
            if (ref)
                rep_ = reinterpret_cast<blk_address>(ref);
            else
                rep_ = -1;

        }
        Node<K, V>* get(blk_accessor<K, V>* blk_accessor) override {
            if (!instance_) {
                instance_ = reinterpret_cast<Node<K, V>*>(rep_);
            }
            return instance_;
        }

        void close(blk_accessor<K, V>* blk_accessor, bool read_only = false) override{
            instance_ = nullptr;
        }

        void remove(blk_accessor<K, V>* blk_accessor) override {
            instance_->close();
            delete instance_;
            instance_ = 0;
            rep_ = -1;
//            delete ref_;
//            ref_ = 0;
        }

        void copy(node_reference<K, V>* ref) {
            if (ref) {
                this->instance_ = dynamic_cast<in_memory_node_ref<K, V> *>(ref)->instance_;
                this->rep_ = dynamic_cast<in_memory_node_ref<K, V> *>(ref)->rep_;
            } else {
                this->instance_ = nullptr;
                this->rep_ = -1;
            }
        }

        bool is_null_ptr() const override {
            return rep_ == -1;
        };

        void bind(Node<K, V>* node) {
            instance_ = node;
            if (node)
                rep_ = reinterpret_cast<blk_address>(node);
        }

        int64_t get_unified_representation() {
            return rep_;
        }

        void restore_by_unified_representation(int64_t value) override {
            rep_ = value;
            instance_ = 0;
        }
    private:
        Node<K, V> *instance_;
        blk_address rep_;
    private:
//    private:
//        friend class boost::serialization::access;
//        template<class Archive>
//        void serialize(Archive & ar, const unsigned int version) {
//            ar & boost::serialization::base_object<node_reference<K, V>>(*this);
//        }
    };
}
#endif //NVM_IN_MEMORY_NODE_REFERENCE_H
