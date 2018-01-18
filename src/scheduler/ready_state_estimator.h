//
// Created by robert on 2/1/18.
//

#ifndef NVM_READY_STATE_ESTIMATOR_H
#define NVM_READY_STATE_ESTIMATOR_H

#include <algorithm>
#include <assert.h>
#include <vector>
#include <unordered_map>
#include "../utils/rdtsc.h"

using namespace std;

/**
 * This class is used to estimated the number of ready command io in a nvme queue pair.
 */
class ready_state_estimator {
public:
    ready_state_estimator(int ready_lat, int write_lat): read_latency(ready_lat), write_latency(write_lat) {};

    void register_write_io (uint64_t id, uint64_t current_timestamp) {
        uint64_t ready_timestamp = current_timestamp + write_latency;
        write_ready_time_array_.push_back(ready_timestamp);
        ready_time_array_.push_back(ready_timestamp);
        id_to_time_[id] = ready_timestamp;
    }

    void register_read_io (uint64_t id, uint64_t current_timestamp) {
        uint64_t ready_timestamp = current_timestamp + read_latency;
        ready_time_array_.push_back(ready_timestamp);
        id_to_time_[id] = ready_timestamp;
    }

    void remove_read_io(uint64_t id) {
        assert (id_to_time_.find(id) != id_to_time_.cend());

        uint64_t time = id_to_time_[id];
        id_to_time_.erase(id);
        auto it = ready_time_array_.begin();
        while(*it != time) {
            ++it;
        }
        ready_time_array_.erase(it);
    }

    void remove_write_io(uint64_t id) {
        assert (id_to_time_.find(id) != id_to_time_.cend());

        uint64_t time = id_to_time_[id];
        id_to_time_.erase(id);
        auto it = write_ready_time_array_.begin();
        while(*it != time) {
            ++it;
        }
        write_ready_time_array_.erase(it);

        it = ready_time_array_.begin();
        while(*it != time) {
            ++it;
        }
        ready_time_array_.erase(it);
    }

    int estimate_number_of_ready_states(uint64_t timestamp) {
        int ready_count = 0;
        for (auto it = ready_time_array_.begin(); it != ready_time_array_.end(); ++it) {
            if (*it <= timestamp)
                ready_count++;
        }
        return ready_count;
    }

    int estimate_number_of_write_states(uint64_t timestamp) {
        int ready_count = 0;
        for (auto it = write_ready_time_array_.begin(); it != write_ready_time_array_.end(); ++it) {
            if (*it <= timestamp)
                ready_count++;
        }
        return ready_count;
    }

    int64_t estimate_the_time_to_get_desirable_ready_state(int expected, uint64_t timestamp) {
//        uint64_t start = ticks();
        std::sort(ready_time_array_.begin(), ready_time_array_.end());
        int64_t time = INT64_MAX;
        int ready_count = 0;
        for (auto it = ready_time_array_.begin(); it != ready_time_array_.end(); ++it) {
            ready_count++;
            if (ready_count == expected) {
                time = *it;
                break;
            }
        }
//        printf("time: %.2f ns length: %d\n", cycles_to_nanoseconds(ticks() - start), ready_time_array_.size());
        return time;
    }

    int64_t estimate_the_time_to_get_desirable_ready_write_state(int expected, uint64_t timestamp) {
        std::sort(write_ready_time_array_.begin(), write_ready_time_array_.end());
        int64_t time = INT64_MAX;
        int ready_count = 0;
        for (auto it = write_ready_time_array_.begin(); it != write_ready_time_array_.end(); ++it) {
            ready_count++;
            if (ready_count == expected) {
                time = *it;
                break;
            }
        }
        return time;
    }

    int get_number_of_pending_state() const {
        return ready_time_array_.size();
    }

    int get_number_of_pending_write_state() const {
        return write_ready_time_array_.size();
    }

    void update_read_latency_in_cycles(int read_lat) {
        read_latency = read_lat;
//        printf("read latency -> %f us\n", cycles_to_microseconds(read_latency));
    }

    void update_write_latency_in_cycles(int write_lat) {
        write_latency = write_lat;
//        printf("write latency -> %f us\n", cycles_to_microseconds(read_latency));
    }

private:
    vector<uint64_t> ready_time_array_;
    vector<uint64_t> write_ready_time_array_;
    unordered_map<uint64_t, uint64_t> id_to_time_;
    int write_latency = 100;
    int read_latency = 10;
};

#endif //NVM_READY_STATE_ESTIMATOR_H
