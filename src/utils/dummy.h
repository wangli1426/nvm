//
// Created by robert on 14/12/17.
//

#ifndef NVM_DUMMY_H
#define NVM_DUMMY_H
#include <thread>
#include "rdtsc.h"
#include "sync.h"
inline int make_me_busy(const int64_t &nanoseconds) {
    int64_t start = ticks();
    int value = 0;
    while(cycles_to_nanoseconds(ticks() - start) < nanoseconds) {
        value = (value * 4 + 32) / 65 + 3;
    }
    return value;

}

inline void busy_for_ever(SpinLock *l) {
    while(!l->try_lock());
    printf("lock obtained!\n");
}

inline void create_one_cpu_consumer() {
    SpinLock *l = new SpinLock();
    l->acquire();
    new std::thread(busy_for_ever, l);
}

#endif //NVM_DUMMY_H
