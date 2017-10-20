//
// Created by robert on 20/10/17.
//

#ifndef NVM_ACCESS_PATTERN_H
#define NVM_ACCESS_PATTERN_H
class access_pattern {
public:
    virtual uint64_t next_access() = 0;
};
#endif //NVM_ACCESS_PATTERN_H
