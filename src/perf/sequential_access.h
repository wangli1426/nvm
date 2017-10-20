//
// Created by robert on 20/10/17.
//

#ifndef NVM_SEQUENTIAL_ACCESS_H
#define NVM_SEQUENTIAL_ACCESS_H

#include "access_pattern.h"

class sequential_access: public access_pattern {
public:
    sequential_access(uint64_t min, uint64_t max): min_(min), max_(max), cur_(min) {};
    uint64_t next_access() {
        if (cur_ >= max_)
            cur_ = min_;
        return cur_++;
    }
private:
    uint64_t min_;
    uint64_t max_;
    uint64_t cur_;
};


#endif //NVM_SEQUENTIAL_ACCESS_H
