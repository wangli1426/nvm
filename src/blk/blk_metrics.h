//
// Created by robert on 7/12/17.
//

#ifndef NVM_BLK_METRICS_H
#define NVM_BLK_METRICS_H

#include <atomic>
#include <string>
#include "../utils/rdtsc.h"
using namespace std;
class blk_metrics {
public:
    atomic<uint64_t> read_cycles_;
    atomic<uint64_t> write_cycles_;
    atomic<uint64_t> reads_;
    atomic<uint64_t> writes_;
    int64_t total_cycles_;
    string name_;

    blk_metrics() {
        read_cycles_ = 0;
        write_cycles_ = 0;
        reads_ = 0;
        writes_ = 0;
        total_cycles_ = 0;
    }

    blk_metrics& operator=(blk_metrics&& r) {
        this->read_cycles_.store(r.read_cycles_.load());
        this->write_cycles_.store(r.write_cycles_.load());
        this->reads_.store(r.reads_.load());
        this->writes_.store(r.writes_.load());
        this->total_cycles_ = r.total_cycles_;
        this->name_ = r.name_;
        return *this;
    }

    blk_metrics(const blk_metrics& r) {
        this->read_cycles_.store(r.read_cycles_.load());
        this->write_cycles_.store(r.write_cycles_.load());
        this->reads_.store(r.reads_.load());
        this->writes_.store(r.writes_.load());
        this->total_cycles_ = r.total_cycles_;
        this->name_ = r.name_;
    }

    void merge(const blk_metrics & metrics) {
        this->read_cycles_ += metrics.read_cycles_;
        this->write_cycles_ += metrics.write_cycles_;
        this->reads_ += metrics.reads_;
        this->writes_ += metrics.writes_;
        this->total_cycles_ += metrics.total_cycles_;
        this->name_ = metrics.name_;
    }

    void print() {
        double duration_in_seconds = cycles_to_seconds(total_cycles_);
        if (reads_ > 0)
            printf("[%s:] total reads: %ld, average: %.2f us, IOPS: %.0f\n", name_.c_str(), reads_.load(),
                   cycles_to_microseconds(read_cycles_.load() / reads_.load()), reads_.load() / duration_in_seconds);
        if (writes_ > 0)
            printf("[%s:] total writes: %ld, average: %.2f us, IOPS %.0f\n", name_.c_str(), writes_.load(),
                   cycles_to_microseconds(write_cycles_.load() / writes_.load()), writes_.load() / duration_in_seconds);
        if (writes_ || reads_)
            printf("[%s:] total IO: %ld, average: %.2f us, IOPS %.0f\n", name_.c_str(), reads_.load() + writes_.load(),
                   cycles_to_microseconds((read_cycles_.load() + write_cycles_.load()) / (reads_.load() + writes_.load())),
                   (reads_.load() + writes_.load()) / duration_in_seconds);
    }
};
#endif //NVM_BLK_METRICS_H
