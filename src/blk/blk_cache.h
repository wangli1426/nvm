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
#include <boost/unordered_map.hpp>
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
        read_hits_ = 0;
        read_probes_ = 0;
        write_hits_ = 0;
        write_probes_ = 0;
    };

    ~blk_cache() {
        for(auto it = key_.begin(); it != key_.end(); ++it) {
            free_block(it->data);
        }
        key_.clear();
    }


    bool write(const int64_t &id, void* buffer, bool dirty, cache_unit & evict) {
        write_probes_++;
        cache_unit unit;
        bool evicted = false;
        if (cache_.find(id) == cache_.cend()) {
            unit.id = id;
            unit.data = allocate_block();
            unit.dirty = dirty;
            insert_new_unit(unit);
            if (size_ > capacity_) {
                evicted = true;
                evict = evict_unit();
            }
        } else {
            write_hits_++;
            unit = get(id);
        }
//        rte_memcpy(unit.data, buffer, blk_size_);
//        rte_mov256((uint8_t*)unit.data, (uint8_t*)buffer);
//        rte_mov256((uint8_t*)(unit.data + 256), (uint8_t*)(buffer + 256));
        unit.dirty |= dirty;
        memcpy(unit.data, buffer, blk_size_);
        return evicted;
    }

    bool read(const int64_t &id, void* buffer) {
//        uint64_t start = ticks();
        read_probes_++;
        if (cache_.find(id) == cache_.cend()) {
            return false;
        }
        read_hits_++;
        cache_unit unit = get(id);
//        uint64_t copy_start = ticks();
//        rte_memcpy(buffer, unit.data, blk_size_);
        memcpy(buffer, unit.data, blk_size_);
//        rte_mov256((uint8_t*)buffer, (uint8_t*)unit.data);
//        rte_mov256((uint8_t*)(buffer + 256), (uint8_t*)(unit.data + 256));
//        uint64_t end = ticks();
//        printf("%.2f ns to read, %.2f ns to copy \n", cycles_to_nanoseconds(end- start), cycles_to_nanoseconds(end - copy_start));
        return true;
    }

    bool invalidate(const int64_t &id) {
        boost::unordered_map<int64_t, list<cache_unit>::iterator>::const_iterator it;
        if ((it = cache_.find(id)) == cache_.end())
            return false;
        free_block(it->second->data);
        key_.erase(it->second);
        cache_.erase(it);
        size_--;
        return true;
    }

    inline void* allocate_block() {
        if (free_blocks_.empty())
            return malloc(blk_size_);
        else {
            void* ret = free_blocks_.back();
            free_blocks_.pop_back();
            return ret;
        }
//        return malloc(blk_size_);
    }

    inline void free_block(void* block) {
        free_blocks_.push_back(block);
//        free(block);
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
        if (read_probes_)
            printf("blk cache read hit rates: %2.2f (%d / %d)\n", double(read_hits_) / double(read_probes_), read_hits_, read_probes_);

        if (write_probes_)
            printf("blk cache write hit rates: %2.2f (%d / %d)\n", double(write_hits_) / double(write_probes_), write_hits_, write_probes_);
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

    static void fastMemcpy(void *pvDest, void *pvSrc, size_t nBytes) {
//        const __m256i *pSrc = reinterpret_cast<const __m256i*>(pvSrc);
//        __m256i *pDest = reinterpret_cast<__m256i*>(pvDest);
//        int64_t nVects = nBytes / sizeof(*pSrc);
//        for (; nVects > 0; nVects--, pSrc++, pDest++) {
//            const __m256i loaded = _mm256_stream_load_si256(pSrc);
//            _mm256_stream_si256(pDest, loaded);
//        }
//        _mm_sfence();
    }


private:
    const int32_t blk_size_;
    const int32_t capacity_;
    int32_t size_;
    boost::unordered_map<int64_t, list<cache_unit>::iterator> cache_;
    list<cache_unit> key_;
    int32_t read_hits_;
    int32_t read_probes_;
    int32_t write_hits_;
    int32_t write_probes_;
    int32_t hits_;
    int32_t probes_;
    std::vector<void*> free_blocks_;
};
#endif //NVM_BLK_CACHE_H
