//
// Created by Li Wang on 10/17/17.
//

#include <iostream>
#include "accessor/ns_entry.h"
#include "perf/common.h"
#include "perf/multithread_perf_benchmark.h"

using namespace std;
using namespace nvm;
int main() {
    run_multiple_threads_benchmark(1, 100000, 8, seq_access, read_load, asynch);
}
