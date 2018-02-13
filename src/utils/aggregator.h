//
// Created by robert on 12/2/18.
//

#ifndef NVM_AGGREGATOR_H
#define NVM_AGGREGATOR_H

#endif //NVM_AGGREGATOR_H

#include <vector>

#include "../utils/sync.h"

using namespace std;

template<typename T>
class aggregator {
public:
    void record(const T &t) {
        lock.acquire();
        values.push_back(t);
        lock.release();
    }

    T sum() const {
        T sum = 0;
        for (auto it = values.cbegin(); it != values.cend(); ++it) {
            sum += *it;
        }
        return sum;
    }

    T avg() const {
        return sum() / values.size();
    }

    void clear() {
        lock.acquire();
        values.clear();
        lock.release();
    }

    bool empty() const {
        return values.empty();
    }


private:
    vector<T> values;
    SpinLock lock;
};