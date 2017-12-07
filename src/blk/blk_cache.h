//
// Created by Li Wang on 11/25/17.
//

#ifndef NVM_BLK_CACHE_H
#define NVM_BLK_CACHE_H
#include <memory.h>
#include <unordered_map>
#include <list>
#include <string>
#include <sstream>
#include <stdio.h>

using namespace std;

class blk_cache {
public:
    struct cache_unit {
        int64_t id;
        bool dirty;
        void* data;
    };
public:
    blk_cache(const int32_t blk_size, const int32_t capacity): blk_size_(blk_size), size_(0), capacity_(capacity) {
        hits_ = 0;
        probes_ = 0;
    };

    ~blk_cache() {
        for(auto it = key_.begin(); it != key_.end(); ++it) {
            free(it->data);
        }
    }


    bool write(const int64_t &id, void* buffer, cache_unit & evict) {
        probes_++;
        cache_unit unit;
        bool evicted = false;
        if (cache_.find(id) == cache_.cend()) {
            unit.id = id;
            unit.data = malloc(blk_size_);
            insert_new_unit(unit);
            if (size_ > capacity_) {
                evicted = true;
                evict = evict_unit();
            }
        } else {
            hits_++;
            unit = get(id);
        }
        memcpy(unit.data, buffer, blk_size_);
        return evicted;
    }

    bool read(const int64_t &id, void* buffer) {
        probes_++;
        if (cache_.find(id) == cache_.cend()) {
            return false;
        }
        hits_++;
        cache_unit unit = get(id);
        memcpy(buffer, unit.data, blk_size_);
        return true;
    }

    string keys_to_string() const {
        ostringstream ost;
        ost << "{";
        int i = 0;
        for (auto it = key_.begin(); it != key_.cend(); ++it, ++i) {
            if (i != key_.size() - 1)
                ost << it->id << ", ";
            else
                ost << it->id << "}";
        }
        return ost.str();
    }

    void print() const {
        if (probes_)
            printf("blk cache hit rates: %2.2f\n", double(hits_) / double(probes_));
    }

private:
    void insert_new_unit(cache_unit& unit) {
        key_.push_front(unit);
        cache_[unit.id] = key_.begin();
        size_++;
    }

    cache_unit evict_unit() {
        cache_unit evict_unit = key_.back();
        key_.pop_back();
        cache_.erase(evict_unit.id);
        size_--;
        return evict_unit;
    }

    cache_unit get(const uint64_t &id) {
        auto it = cache_.find(id);
        key_.splice(key_.begin(), key_, it->second);
        return *it->second;
    }

private:
    const int32_t blk_size_;
    const int32_t capacity_;
    int32_t size_;
    unordered_map<int64_t, list<cache_unit>::iterator> cache_;
    list<cache_unit> key_;
    int32_t hits_;
    int32_t probes_;
};
#endif //NVM_BLK_CACHE_H
