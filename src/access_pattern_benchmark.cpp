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
int main(int argc, char* argv[]) {

    int status = nvm::nvm_utility::initialize_namespace();
    if (status != 0) {
        cout << "Errors in initialization of namespace." << endl;
        exit(1);
    } else {
        cout << "namespace is initialized." << endl;
    }

    spdk_unaffinitize_thread();
    print_current_cpu_set();
    set_cpu_set(8);
    print_current_cpu_set();

    cout << "++++++++++ WARMUP ++++++++++" << endl;
    const int number_of_threads = atoi(argv[1]);
    const int write_rate = 0;
    run_multiple_threads_benchmark(number_of_threads, 100000, 512, 1, rand_access, write_rate, asynch);
    run_multiple_threads_benchmark(number_of_threads, 100000, 512, 4, rand_access, write_rate, asynch);
    run_multiple_threads_benchmark(number_of_threads, 100000, 512, 16, rand_access, write_rate, asynch);
    run_multiple_threads_benchmark(number_of_threads, 100000, 512, 64, rand_access, write_rate, asynch);
    run_multiple_threads_benchmark(number_of_threads, 100000, 512, 256, rand_access, write_rate, asynch);
    run_multiple_threads_benchmark(number_of_threads, 100000, 512, 1024, rand_access, write_rate, asynch);
//    run_multiple_threads_benchmark(1, 1000000, 512, 16, rand_access, write_rate, asynch);
//    run_multiple_threads_benchmark(2, 1000000, 512, 16, rand_access, write_rate, asynch);
//    run_multiple_threads_benchmark(3, 100000, 512, 32, rand_access, write_rate, asynch);
//    run_multiple_threads_benchmark(4, 1000000, 512, 16, rand_access, write_rate, asynch);
//    run_multiple_threads_benchmark(5, 100000, 512, 32, rand_access, write_rate, asynch);
//    run_multiple_threads_benchmark(6, 100000, 512, 32, rand_access, write_rate, asynch);
//    run_multiple_threads_benchmark(7, 100000, 512, 32, rand_access, write_rate, asynch);
//    run_multiple_threads_benchmark(8, 1000000, 512, 16, rand_access, write_rate, asynch);
    cout << "++++++++++ WARMUP ++++++++++" << endl << endl;

    exit(0);
    sleep(1);

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
