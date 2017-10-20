//
// Created by Li Wang on 10/15/17.
//

#ifndef NVM_RDTSC_H
#define NVM_RDTSC_H

#define CPU_FREQUENCY (2697)

inline uint64_t ticks() {
    uint32_t lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

inline double cycles_to_nanoseconds(uint64_t cycles) {
    return (uint64_t)((double) cycles / CPU_FREQUENCY * 1000);
}

inline double cycles_to_microseconds(uint64_t cycles) {
    return cycles_to_nanoseconds(cycles) / 1000;
}

#endif //NVM_RDTSC_H
