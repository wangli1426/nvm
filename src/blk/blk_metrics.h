//
// Created by robert on 7/12/17.
//

#ifndef NVM_BLK_METRICS_H
#define NVM_BLK_METRICS_H

#include <atomic>
#include <deque>
#include <string>
#include <vector>
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

    std::vector<int16_t> pending_command_counts_;
    int64_t pending_commands_;

    deque<uint64_t> recent_read_cycles_;
    deque<uint64_t> recent_write_cycles_;

    const int window_size = 1000;

    blk_metrics() {
        reset();
    }

    void reset() {
        read_cycles_ = 0;
        write_cycles_ = 0;
        reads_ = 0;
        writes_ = 0;
        total_cycles_ = 0;
        pending_command_counts_.clear();
        pending_commands_ = 0;
    }

    blk_metrics& operator=(blk_metrics&& r) {
        this->read_cycles_.store(r.read_cycles_.load());
        this->write_cycles_.store(r.write_cycles_.load());
        this->reads_.store(r.reads_.load());
        this->writes_.store(r.writes_.load());
        this->total_cycles_ = r.total_cycles_;
        this->name_ = r.name_;
        this->pending_command_counts_ = r.pending_command_counts_;
        this->pending_commands_ = r.pending_commands_;
        return *this;
    }

    blk_metrics(const blk_metrics& r) {
        this->read_cycles_.store(r.read_cycles_.load());
        this->write_cycles_.store(r.write_cycles_.load());
        this->reads_.store(r.reads_.load());
        this->writes_.store(r.writes_.load());
        this->total_cycles_ = r.total_cycles_;
        this->name_ = r.name_;
        this->pending_command_counts_ = r.pending_command_counts_;
        this->pending_commands_ = r.pending_commands_;
    }

    void merge(const blk_metrics & metrics) {
        this->read_cycles_ += metrics.read_cycles_;
        this->write_cycles_ += metrics.write_cycles_;
        this->reads_ += metrics.reads_;
        this->writes_ += metrics.writes_;
        this->total_cycles_ += metrics.total_cycles_;
        this->name_ = metrics.name_;
        this->pending_command_counts_ = metrics.pending_command_counts_;
        this->pending_commands_ = metrics.pending_commands_;
    }

    void add_read_latency(const int64_t &cycles) {
        this->read_cycles_ += cycles;
        this->reads_++;
        this->recent_read_cycles_.push_back(cycles);
        if (recent_read_cycles_.size() > window_size)
            this->recent_read_cycles_.pop_front();
    }

    void add_write_latency(const int64_t &cycles) {
        this->write_cycles_ += cycles;
        this->writes_++;
        this->recent_write_cycles_.push_back(cycles);
        if (recent_write_cycles_.size() > window_size)
            this->recent_write_cycles_.pop_front();
    }

    int get_avg_write_latency_in_cycles() const {
        return this->write_cycles_ / this->writes_;
    }

    int get_avg_read_latency_in_cycles() const {
        return this->read_cycles_ / this->reads_;
    }

    int get_recent_avg_write_latency_in_cycles() const {
        if (recent_write_cycles_.empty())
            return 10000;
        uint64_t sum = 0;
        for (auto it = recent_write_cycles_.begin(); it != recent_write_cycles_.end(); ++it) {
            sum += *it;
        }
        return sum / recent_write_cycles_.size();
    }

    int get_recent_avg_read_latency_in_cycles() const {
        if (recent_read_cycles_.empty())
            return 10000;
        uint64_t sum = 0;
        for (auto it = recent_read_cycles_.begin(); it != recent_read_cycles_.end(); ++it) {
            sum += *it;
        }
        return sum / recent_read_cycles_.size();
    }

    void print() {
        double duration_in_seconds = cycles_to_seconds(total_cycles_);
        if (reads_ > 0)
            printf("[%s:] total reads: %ld, average: %.2f us, amortized latency: %.2f us, IOPS: %.0f, concurrency: %.2f\n",
                   name_.c_str(),
                   reads_.load(),
                   cycles_to_microseconds(read_cycles_.load() / reads_.load()),
                   duration_in_seconds / reads_.load() * 1000000,
                   reads_.load() / duration_in_seconds,
                   cycles_to_seconds(read_cycles_.load()) / duration_in_seconds);
        if (writes_ > 0)
            printf("[%s:] total writes: %ld, average: %.2f us, amortized latency: %.2f us, IOPS %.0f, concurrency: %.2f\n",
                   name_.c_str(),
                   writes_.load(),
                   cycles_to_microseconds(write_cycles_.load() / writes_.load()),
                   duration_in_seconds / writes_.load() * 1000000,
                   writes_.load() / duration_in_seconds,
                   cycles_to_seconds(write_cycles_.load()) / duration_in_seconds);
        if (writes_ || reads_)
            printf("[%s:] total IO: %ld, average: %.2f us, amortized latency: %.2f us, IOPS %.0f, concurrency: %.2f\n",
                   name_.c_str(),
                   reads_.load() + writes_.load(),
                   cycles_to_microseconds((read_cycles_.load() + write_cycles_.load()) / (reads_.load() + writes_.load())),
                   duration_in_seconds / (reads_.load() + writes_.load()) * 1000000,
                   (reads_.load() + writes_.load()) / duration_in_seconds,
                   cycles_to_seconds(read_cycles_.load()  + write_cycles_.load()) / duration_in_seconds);

        if (pending_command_counts_.size() > 0) {
//            for (int i = 0; i < pending_command_counts_.size(); ++i) {
//                printf("%d ", pending_command_counts_[i]);
//            }

            long sum = 0;
            for (int i = 0; i < pending_command_counts_.size(); ++i) {
                sum += pending_command_counts_[i];
            }
            printf("NVMe: average queue length: %.2f\n", (double) sum / pending_command_counts_.size());
        }
    }
};
#endif //NVM_BLK_METRICS_H
