//
// Created by Li Wang on 10/17/17.
//

#include <iostream>
#include "accessor/ns_entry.h"
#include "perf/common.h"
#include "perf/multithread_perf_benchmark.h"
#include "perf/single_thread_benchmark.h"

using namespace std;
using namespace nvm;
int main() {

    int status = nvm::nvm_utility::initialize_namespace();
    if (status != 0) {
        cout << "Errors in initialization of namespace." << endl;
        exit(1);
    } else {
        cout << "namespace is initialized." << endl;
    }

    cout << "++++++++++ WARMUP ++++++++++" << endl;
    run_multiple_threads_benchmark(1, 100000, 512, 8, seq_access, read_load, asynch);
    run_multiple_threads_benchmark(1, 100000, 512, 8, seq_access, write_load, asynch);
    cout << "++++++++++ WARMUP ++++++++++" << endl << endl;

    cout << "=========== Sequential Access ==========" << endl;

    cout << "[read:]" << endl;
    run_single_thread_benchmark(100, 512, 64, seq_access, read_load, asynch);
    run_single_thread_benchmark(100, 512, 8, seq_access, read_load, asynch);
    run_single_thread_benchmark(100, 512, 4, seq_access, read_load, asynch);
    run_single_thread_benchmark(100, 512, 1, seq_access, read_load, synch);


    cout << "[write:]" << endl;

    run_single_thread_benchmark(100, 512, 64, seq_access, write_load, asynch);
    run_single_thread_benchmark(100, 512, 8, seq_access, write_load, asynch);
    run_single_thread_benchmark(100, 512, 4, seq_access, write_load, asynch);
    run_single_thread_benchmark(100, 512, 1, seq_access, write_load, synch);


    cout << "=========== Random Access ===========" << endl;

    cout << "[read:]" << endl;
    run_single_thread_benchmark(100, 512, 64, rand_access, read_load, asynch);
    run_single_thread_benchmark(100, 512, 8, rand_access, read_load, asynch);
    run_single_thread_benchmark(100, 512, 4, rand_access, read_load, asynch);
    run_single_thread_benchmark(100, 512, 1, rand_access, read_load, synch);


    cout << "[write:]" << endl;

    run_single_thread_benchmark(100, 512, 64, rand_access, write_load, asynch);
    run_single_thread_benchmark(100, 512, 8, rand_access, write_load, asynch);
    run_single_thread_benchmark(100, 512, 4, rand_access, write_load, asynch);
    run_single_thread_benchmark(100, 512, 1, rand_access, write_load, synch);

}
