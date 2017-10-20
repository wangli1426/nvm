//
// Created by robert on 20/10/17.
//

#ifndef NVM_RANDOM_ACCESS_H
#define NVM_RANDOM_ACCESS_H
#include "access_pattern.h"

class random_access: public access_pattern{
public:
    random_access(uint64_t min, uint64_t max): min_(min), max_(max), range_(max - min) {};
    uint64_t next_access() {
        return rand() % range_ + min_;
    }
private:
    uint64_t range_;
    uint64_t min_;
    uint64_t max_;
};


#endif //NVM_RANDOM_ACCESS_H
