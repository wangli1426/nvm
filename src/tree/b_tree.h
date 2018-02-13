//
// Created by robert on 14/9/17.
//

#ifndef B_TREE_B_TREE_H
#define B_TREE_B_TREE_H

#include "tree_perf_metrics.h"

template <typename K, typename V>
class blk_accessor;

namespace tree {
    template<typename K, typename V>
    class BTree {
    public:
        virtual void insert(const K &k, const V &v) = 0;

        virtual bool delete_key(const K &k) = 0;

        virtual bool search(const K &k, V &v) = 0;

        virtual void clear() = 0;

        virtual void close() {};

        virtual void sync() {};

        virtual int height() {
            return -1;
        };

        class Iterator {
        public:
            virtual ~Iterator(){};
            virtual bool next(K &key, V &val) {
                return false;
            };
        };

        virtual Iterator *range_search(const K &key_low, const K &key_high) = 0;

        virtual blk_accessor<K, V>* get_accessor() {};

        tree_perf_metrics &get_metrics() {
            return metrics_;
        };

        void reset_metrics() {
            metrics_.clear();
        }

    protected:
        tree_perf_metrics metrics_;
    };
}
#endif //B_TREE_B_TREE_H
