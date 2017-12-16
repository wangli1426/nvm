//
// Created by robert on 14/12/17.
//

#ifndef NVM_CPU_SET_H
#define NVM_CPU_SET_H
#include <sched.h>
#include <pthread.h>

void print_current_cpu_set() {
#ifdef __APPLE__
    printf("WARNING: Mac Os does not support cpu affinity settings.\n");
    return;
#else
    cpu_set_t mask;
    sched_getaffinity(0, sizeof(cpu_set_t), &mask);
    printf("CPUs = %d\n", CPU_COUNT(&mask));
#endif
}

void set_cpu_set(const int32_t &cpus) {
#ifdef __APPLE__
    printf("WARNING: Mac Os does not support cpu affinity settings.\n");
    return;
#else
    cpu_set_t mask;
    CPU_ZERO(&mask);
    for (int i = 0; i < cpus; i++) {
        CPU_SET(i, &mask);
    }
    sched_setaffinity(0, sizeof(cpu_set_t), &mask);
#endif
}

#endif //NVM_CPU_SET_H
