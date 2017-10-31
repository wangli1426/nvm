////
//// Created by Li Wang on 10/25/17.
////
//
//#ifndef NVM_BLK_NODE_H
//#define NVM_BLK_NODE_H
//#include <assert.h>
//#include "../blk/blk.h"
//#include "node.h"
////#include "blk_inner_node.h"
////#include "blk_leaf_node.h"
//namespace tree{
//
//    template<typename K, typename V>
//    struct Blk_Split {
//        blk_address left;
//        blk_address right;
//        K boundary;
//    };
//
//    template <typename K, typename V, int CAPACITY>
//    class BlkInnerNode;
//
//    template <typename K, typename V, int CAPACITY>
//    class BlkLeafNode;
//
//    template <typename K, typename V, int CAPACITY>
//    class BlkNode {
//    public:
//        BlkNode(blk_address address, blk_accessor* accessor): blk_address_(address), blk_accessor_(accessor){};
//        BlkNode(blk_address address, blk_accessor* accessor, Node<K, V> * node): blk_address_(address), blk_accessor_(accessor), node_(node) {};
//        virtual ~BlkNode() {
//            delete node_;
//        }
//        void flush() {
//            void* buffer = malloc(blk_accessor_->block_size);
//            node_->serialize(buffer);
//            blk_accessor_->write(blk_address_, buffer);
//            free(buffer);
//        }
//
//        static BlkNode* create_leaf_node(blk_accessor* accessor) {
//            blk_address address = accessor->allocate();
////            new BlkInnerNode<K, V, CAPACITY>(address, accessor);
//            return new BlkLeafNode<K, V, CAPACITY>(address, accessor);
//        }
//
//        blk_address get_blk_address() {
//            return blk_address_;
//        }
//
//        virtual bool insert_with_split_support(const K& key, const V& value, Blk_Split<K, V>& split) = 0;
//
//        Node<K, V> *node_;
//    protected:
//        blk_address blk_address_;
//        blk_accessor* blk_accessor_;
//    };
//}
//#endif //NVM_BLK_NODE_H
