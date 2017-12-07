//
// Created by robert on 1/12/17.
//

#ifndef NVM_LOCK_MANAGER_H
#define NVM_LOCK_MANAGER_H

#include <unordered_map>
#include <boost/thread/shared_mutex.hpp>
#include "../utils/sync.h"
using namespace std;
using namespace boost;

#define READ_LOCK 0
#define WRITE_LOCK 1


struct lock_descriptor {
    int64_t id;
    uint8_t type;
};

class lock_manager {
public:

    ~lock_manager() {
        for (auto it = id_to_mutex_.cbegin(); it != id_to_mutex_.cend(); ++it) {
            if(!it->second->try_lock()) {
                if (it->second->try_lock_shared()) {
                    printf("warning: mutex [%ld] is not fully released (shared)!\n", it->first);
                } else {
                    printf("warning: mutex [%ld] is not fully released (write)!\n", it->first);
                }
            }
            delete it->second;
        }
    }

    lock_descriptor get_read_lock(int64_t id) {
        lock_descriptor l;
        l.type = READ_LOCK;
        l.id = id;
        get_or_create(id)->lock_shared();
        return l;
    }

    bool try_get_read_lock(int64_t id, lock_descriptor &l) {
        bool ret = get_or_create(id)->try_lock_shared();
        if (!ret)
            return false;
        l.type = READ_LOCK;
        l.id = id;
        return true;
    }

    lock_descriptor get_write_lock(int64_t id) {
        lock_descriptor l;
        l.type = WRITE_LOCK;
        l.id = id;
        get_or_create(id)->lock();
        return l;
    }

    bool try_get_write_lock(int64_t id, lock_descriptor& l) {
        bool ret = get_or_create(id)->try_lock();
        if (!ret)
            return false;
        l.id = id;
        l.type = WRITE_LOCK;
        return true;
    }

    void promote_to_write_lock(lock_descriptor &l) {
        shared_mutex* lock = get_or_create(l.id);
        lock->lock_upgrade();
        lock->unlock_shared();
        lock->unlock_upgrade_and_lock();
        l.type = WRITE_LOCK;
    }

    void release_lock(lock_descriptor& lock) {
        if (lock.type == WRITE_LOCK) {
            get_or_create(lock.id)->unlock();
        } else if (lock.type == READ_LOCK) {
            get_or_create(lock.id)->unlock_shared();
        }
    }

private:

    shared_mutex* get_or_create(int64_t id) {
        shared_mutex* ret;
        spin_lock_.acquire();
        if (id_to_mutex_.find(id) == id_to_mutex_.cend()) {
            ret = new shared_mutex;
            id_to_mutex_[id] = ret;
        } else {
            ret = id_to_mutex_[id];
        }
        spin_lock_.release();
        return ret;
    }

private:
    unordered_map<int64_t, shared_mutex*> id_to_mutex_;
    SpinLock spin_lock_;
};
#endif //NVM_LOCK_MANAGER_H
