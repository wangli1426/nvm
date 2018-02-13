//
// Created by robert on 12/2/18.
//

#ifndef NVM_TREE_PERF_METRICS_H
#define NVM_TREE_PERF_METRICS_H

#include <string>
#include <ostream>
#include "../utils/rdtsc.h"
#include "../utils/aggregator.h"

using namespace std;

class tree_perf_metrics {
public:
    void add_write_latency(const uint64_t &latency) {
        write_latencies.record(latency);
    }

    void add_read_latency(const uint64_t &latency) {
        read_latencies.record(latency);
    }

    void clear() {
        write_latencies.clear();
        read_latencies.clear();
    }

    string get_string() {
        ostringstream ost;
        if (!read_latencies.empty())
            ost << "avg read latency: " << cycles_to_microseconds(read_latencies.avg()) << " us" << ", ";
        if (!write_latencies.empty())
            ost << "avg write latency: " << cycles_to_microseconds(write_latencies.avg()) << " us";
        return ost.str();
    }

    void print() {
        printf("%s\n", get_string().c_str());
    }

private:
    aggregator<uint64_t> write_latencies;
    aggregator<uint64_t> read_latencies;
};
#endif //NVM_TREE_PERF_METRICS_H
