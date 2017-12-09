//
// Created by Li Wang on 10/27/17.
//

#ifndef NVM_BLK_NODE_REFERENCE_H
#define NVM_BLK_NODE_REFERENCE_H

//#include <boost/archive/binary_iarchive.hpp>
//#include <boost/archive/binary_oarchive.hpp>
#include <stdio.h>
#include <sstream>
#include <string>
#include <assert.h>
#include "node_reference.h"
#include "../blk/blk.h"
#include "node.h"
#include "leaf_node.h"
#include "inner_node.h"
#include "../utils/rdtsc.h"
using namespace std;
namespace tree {


    template<typename K, typename V, int CAPACITY>
    class blk_node_reference : public node_reference<K, V> {
    public:

        blk_node_reference(): blk_address_(-1), instance_(0) {
        };

        blk_node_reference(blk_address blk_address) : blk_address_(blk_address), instance_(0) {
        };

        virtual ~blk_node_reference(){
        }

        Node<K, V> *get(blk_accessor<K, V>* blk_accessor) {
            if (blk_address_ < 0)
                return nullptr;
            if (instance_)
                return instance_;
            void *read_buffer = blk_accessor->malloc_buffer();
            blk_accessor->read(blk_address_, read_buffer);
            specific_deserialize(read_buffer, blk_accessor);
            blk_accessor->free_buffer(read_buffer);
            return instance_;
        };

        void close(blk_accessor<K, V>* blk_accessor, bool read_only = false) {
            if (!instance_)
                return;
            if (instance_->is_modified()) {
                void *write_buffer = blk_accessor->malloc_buffer();
                int serialized_size = specific_serialize(write_buffer, blk_accessor);
                blk_accessor->write(blk_address_, write_buffer);
                blk_accessor->free_buffer(write_buffer);
            }
            Node<K, V>* instance = instance_;
            instance_ = 0;
            delete instance;
        }

        void remove(blk_accessor<K, V>* blk_accessor) {
            instance_->close();
            blk_accessor->deallocate(blk_address_);
            Node<K, V>* instance = instance_;
            instance_ = 0;
            delete instance;
        }

        void copy(node_reference<K, V>* ref) {
//            this->ref_ = dynamic_cast<blk_node_reference<K, V>*>(ref)->ref_;
            if (ref) {
                this->blk_address_ = dynamic_cast<blk_node_reference<K, V, CAPACITY> *>(ref)->blk_address_;
                this->instance_ = dynamic_cast<blk_node_reference<K, V, CAPACITY> *>(ref)->instance_;
            } else {
                this->blk_address_ = -1;
                this->instance_ = 0;
            }
//            this->instance_ = 0;
        }
        bool is_null_ptr() const {
            return blk_address_ == -1;
        }

        void bind(Node<K, V>* node) {
            instance_ = node;
        }

        int64_t get_unified_representation() {
            return reinterpret_cast<int64_t>(blk_address_);
        }

        void restore_by_unified_representation(int64_t value) override {
            instance_ = nullptr;
            blk_address_ = value;
        }

    private:
        int specific_serialize(void* write_buffer, blk_accessor<K, V>* blk_accessor) {
            instance_->serialize(write_buffer);
        }

        int specific_deserialize(void* read_buffer, blk_accessor<K, V>* blk_accessor) {
            uint32_t *type = static_cast<uint32_t*>(read_buffer);
            switch (*type) {
                case LEAF_NODE:
                    instance_ = new LeafNode<K, V, CAPACITY>(blk_accessor, false);
                    instance_->deserialize(read_buffer);
                    break;
                case INNER_NODE:
                    instance_ = new InnerNode<K, V, CAPACITY>(blk_accessor, false);
                    instance_->deserialize(read_buffer);
                    break;
                default:
                    assert(false); // unexpected point of execution.
                    return -1;
            }
        }

        int boost_serialize(void* write_buffer, blk_accessor<K, V>* blk_accessor) {
//            ostringstream ostr;
//            boost::archive::binary_oarchive oa(ostr);
//            oa.register_type(static_cast<LeafNode<K, V, CAPACITY>*>(NULL));
//            oa.register_type(static_cast<InnerNode<K, V, CAPACITY>*>(NULL));
//            oa.register_type(static_cast<in_memory_node_ref<K, V>*>(NULL));
//            oa.register_type(static_cast<blk_node_reference<K, V, CAPACITY>*>(NULL));
//            oa << instance_;
//            ostr.flush();
//            std::string str = ostr.str();
//            int32_t length = str.length();
//            assert(length <= blk_accessor->block_size);
//            memcpy(write_buffer, str.c_str(), str.size());
//            memset((char*)write_buffer + str.size(), 0, blk_accessor->block_size - str.size());
//            return length;
        }

        void boost_deserialize(void* read_buffer, blk_accessor<K, V>* blk_accessor) {
//            ostringstream ostr;
//            ostr.write((char*)read_buffer, blk_accessor->block_size);
//            std::string tmp = ostr.str();
//            istringstream istr(tmp);
//            boost::archive::binary_iarchive ia(istr);
//            ia.register_type(static_cast<LeafNode<K, V, CAPACITY>*>(NULL));
//            ia.register_type(static_cast<InnerNode<K, V, CAPACITY>*>(NULL));
//            ia.register_type(static_cast<in_memory_node_ref<K, V>*>(NULL));
//            ia.register_type(static_cast<blk_node_reference<K, V, CAPACITY>*>(NULL));
//            ia >> instance_;
//            instance_->set_blk_accessor(blk_accessor);
        }

        blk_address get_blk_address() const {
            return blk_address_;
        }
    private:
        blk_address blk_address_;
        Node<K, V>* instance_;


//    private:
//        friend class boost::serialization::access;
//        template<class Archive>
//        void serialize(Archive & ar, const unsigned int version) {
//            ar & boost::serialization::base_object<node_reference<K, V>>(*this) & blk_address_;
//        }
    };
}
#endif //NVM_BLK_NODE_REFERENCE_H
