//
// Created by Li Wang on 12/2/17.
//

#ifndef NVM_OPERATION_H
#define NVM_OPERATION_H
#define READ_OP 1
#define WRITE_OP 2

template <typename K, typename V>
struct operation{
    K key;
    V val;
    int type;
};
#endif //NVM_OPERATION_H
