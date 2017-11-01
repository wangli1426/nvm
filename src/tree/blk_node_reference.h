//
// Created by Li Wang on 10/27/17.
//

#ifndef NVM_BLK_NODE_REFERENCE_H
#define NVM_BLK_NODE_REFERENCE_H

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <sstream>
#include "node_reference.h"
#include "../blk/blk.h"
#include "node.h"
#include "leaf_node.h"
#include "inner_node.h"
using namespace std;
namespace tree {
    template<typename K, typename V, int CAPACITY>
    class blk_node_reference : public node_reference<K, V> {
    public:
        blk_node_reference(blk_address blk_address, blk_accessor<K, V, CAPACITY>* blk_accessor) : blk_address_(blk_address),
                                                                                 blk_accessor_(blk_accessor),
                                                                                 instance_(0) {};


        Node<K, V> *get() {
            if (instance_)
                return instance_;
            void *read_buffer = malloc(blk_accessor_->block_size);
            blk_accessor_->read(blk_address_, read_buffer);

//            uint32_t *type = static_cast<uint32_t*>(read_buffer);
//            switch (*type) {
//                case LEAF_NODE:
//                    instance_ = new LeafNode<K, V, CAPACITY>(blk_accessor_);
//                    instance_->deserialize(read_buffer);
//                    break;
//                case INNER_NODE:
//                    instance_ = new InnerNode<K, V, CAPACITY>();
//                    instance_->deserialize(read_buffer);
//                    break;
//                default:
//                    return nullptr;
//            }

            ostringstream ostr;
            ostr.write(read_buffer, blk_accessor_->block_size);
            std::string tmp = ostr.str();
            istringstream istr(tmp);
            boost::archive::text_iarchive ia(istr);
            ia.register_type(static_cast<LeafNode<K, V, CAPACITY>*>(NULL));
            ia.register_type(static_cast<InnerNode<K, V, CAPACITY>*>(NULL));
            ia.register_type(static_cast<Node<K, V>*>(NULL));
            ia >> instance_;

            return instance_;

        };

        void close() {
            void *write_buffer = malloc(blk_accessor_->block_size);
//            instance_->serialize(write_buffer);

            ostringstream ostr;
            boost::archive::text_oarchive oa(ostr);
            oa.register_type(static_cast<LeafNode<K, V, CAPACITY>*>(NULL));
            oa.register_type(static_cast<InnerNode<K, V, CAPACITY>*>(NULL));
            oa.register_type(static_cast<Node<K, V>*>(NULL));
            oa << instance_;
            std::string str = ostr.str();
            memcpy(write_buffer, str.c_str(), str.size());

            blk_accessor_->write(blk_address_, write_buffer);
            delete instance_;
            instance_ = 0;
        }

        void remove() {
            delete instance_;
            instance_ = 0;
            blk_accessor_->deallocate(blk_address_);
        }

        void copy(node_reference<K, V>* ref) {
//            this->ref_ = dynamic_cast<blk_node_reference<K, V>*>(ref)->ref_;
            this->blk_address_ = dynamic_cast<blk_node_reference<K, V, CAPACITY>*>(ref)->blk_address_;
//            this->instance_ = 0;
        }
        bool is_null_ptr() const {
            return blk_address_ == 0;
        }

        void bind(Node<K, V>* node) {
            instance_ = node;
        }
    private:
        blk_address blk_address_;
        blk_accessor<K, V, CAPACITY>* blk_accessor_;
        Node<K, V>* instance_;
    };
}
#endif //NVM_BLK_NODE_REFERENCE_H
