//
// Created by Li Wang on 10/22/17.
//

#ifndef NVM_COMMON_H
#define NVM_COMMON_H
namespace nvm{
    enum io_mode {
        synch, asynch
    };

    enum workload {
        read_load, write_load
    };

    enum access_mode {
        seq_access, rand_access
    };
}
#endif //NVM_COMMON_H
