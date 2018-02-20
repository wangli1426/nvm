//
// Created by robert on 2/1/18.
//

#ifndef NVM_READY_STATE_ESTIMATOR_H
#define NVM_READY_STATE_ESTIMATOR_H

#include <algorithm>
#include <assert.h>
#include <vector>
#include <unordered_map>
#include <stdlib.h>
#include "../utils/rdtsc.h"

#define granularity 20

using namespace std;

/**
 * This class is used to estimated the number of ready command io in a nvme queue pair.
 */
class ready_state_estimator {
public:
    ready_state_estimator() {
        read_latency = 200000;
        write_latency = 400000;
    }
    ready_state_estimator(int ready_lat, int write_lat): read_latency(ready_lat), write_latency(write_lat) {};

    void register_write_io (uint64_t id, uint64_t current_timestamp) {
        write_time_array_.push_back(current_timestamp);
        id_to_time_[id] = current_timestamp;
    }

    void register_read_io (uint64_t id, uint64_t current_timestamp) {
        read_time_array_.push_back(current_timestamp);
        id_to_time_[id] = current_timestamp;
    }

    void remove_read_io(uint64_t id) {
        assert (id_to_time_.find(id) != id_to_time_.cend());
        uint64_t time = id_to_time_[id];
        id_to_time_.erase(id);
        auto it = read_time_array_.begin();
        while(*it != time) {
            ++it;
        }
        assert(it != read_time_array_.end());
        read_time_array_.erase(it);
    }

    void remove_write_io(uint64_t id) {
        assert (id_to_time_.find(id) != id_to_time_.cend());
        uint64_t time = id_to_time_[id];
        id_to_time_.erase(id);
        auto it = write_time_array_.begin();
        while(*it != time) {
            ++it;
        }
        assert(it != write_time_array_.end());
        write_time_array_.erase(it);
    }

    virtual int estimate_number_of_ready_reads(uint64_t timestamp) {
        int ready_count = 0;
        for (auto it = read_time_array_.begin(); it != read_time_array_.end(); ++it) {
            if (*it + read_latency <= timestamp)
                ready_count++;
            else
                break;
        }
        return ready_count;
    }

    virtual int estimate_number_of_ready_writes(uint64_t timestamp) {
        int ready_count = 0;
        for (auto it = write_time_array_.begin(); it != write_time_array_.end(); ++it) {
            if (*it + write_latency <= timestamp)
                ready_count++;
            else
                break;
        }
        return ready_count;
    }

    int estimate_number_of_ready_ios(uint64_t timestamp) {
        return estimate_number_of_ready_reads(timestamp) + estimate_number_of_ready_writes(timestamp);
    }

    int64_t estimate_the_time_to_get_desirable_ready_state(int expected, uint64_t timestamp) {
//        uint64_t start = ticks();
//        std::sort(read_time_array_.begin(), read_time_array_.end());
//        int64_t time = INT64_MAX;
//        int ready_count = 0;
//        for (auto it = ready_time_array_.begin(); it != ready_time_array_.end(); ++it) {
//            ready_count++;
//            if (ready_count == expected) {
//                time = *it;
//                break;
//            }
//        }
////        printf("time: %.2f ns length: %d\n", cycles_to_nanoseconds(ticks() - start), ready_time_array_.size());
//
//
//        auto read_it = read_time_array_.cbegin(), write_it = write_time_array_.cbegin();
//
//        while ()

        assert(false);
//        return time;
    }

    int64_t estimate_the_time_to_get_desirable_ready_write_state(int expected, uint64_t timestamp) {
        uint64_t start = ticks();
        int64_t time = INT64_MAX;
        int ready_count = 0;
        for (auto it = write_time_array_.begin(); it != write_time_array_.end(); ++it) {
            ready_count++;
            if (ready_count == expected) {
                time = *it;
                break;
            }
        }
        return time + write_latency;
    }

    int get_number_of_pending_state() const {
        return read_time_array_.size() + write_time_array_.size();
    }

    int get_number_of_pending_write_state() const {
        return write_time_array_.size();
    }

    void update_read_latency_in_cycles(const uint64_t &read_lat) {
        read_latency = read_lat;
//        printf("read latency -> %f us\n", cycles_to_microseconds(read_latency));
    }

    void update_write_latency_in_cycles(const uint64_t & write_lat) {
        write_latency = write_lat;
//        printf("write latency -> %f us\n", cycles_to_microseconds(read_latency));
    }

    uint64_t get_current_read_latency() const {
        return read_latency;
    }

    void print_reads() const {
        uint64_t now = ticks();
//        printf("[READ]");
//        for(int i = 0; i < read_time_array_.size(); i++) {
//            printf("%.2f ", cycles_to_microseconds(now - read_time_array_[i]));
//        }
//        printf("\n");

        unordered_map<int, int> counts;
        for(int i = 0 ; i < granularity; i++) {
            counts[i] = 0;
        }

        for(auto it = read_time_array_.cbegin(); it != read_time_array_.cend(); ++it) {
            int offset = max(0, (int)(width - cycles_to_microseconds(now - *it))) / (width / granularity);
            counts[offset]++;
        }
//
        for (int i = 0; i < granularity; i++) {
            printf("%d ", counts[i]);
        }
//        printf("\n");
    }

    void print_writes() const {
        uint64_t now = ticks();
//        printf("[WRITE]");
//        for(int i = 0; i < write_time_array_.size(); i++) {
//            printf("%.2f ", cycles_to_microseconds(now - write_time_array_[i]));
//        }
//        printf("\n");


        unordered_map<int, int> counts;
        for(int i = 0 ; i < granularity; i++) {
            counts[i] = 0;
        }

        for(auto it = write_time_array_.cbegin(); it != write_time_array_.cend(); ++it) {
            int offset = max(0, (int)(width - cycles_to_microseconds(now - *it))) / (width / granularity);
            counts[offset]++;
        }
//
        for (int i = 0; i < granularity; i++) {
            printf("%d ", counts[i]);
        }
//        printf("\n");

    }

protected:
    vector<uint64_t> read_time_array_;
    vector<uint64_t> write_time_array_;
    unordered_map<uint64_t, uint64_t> id_to_time_;
    volatile uint64_t write_latency = 100;
    volatile uint64_t read_latency = 10;


    const int width = 1000;
};

#endif //NVM_READY_STATE_ESTIMATOR_H
