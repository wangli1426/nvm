//
// Created by robert on 30/11/17.
//

#ifndef NVM_READ_WRITE_LOCK_H
#define NVM_READ_WRITE_LOCK_H

#include <boost/thread/shared_mutex.hpp>

class rwlock {
public:
    void read_lock() {
        shared_mutex_.lock_shared();
    }

    void read_unlock() {
        shared_mutex_.unlock_shared();
    }

    bool try_read_lock() {
        return shared_mutex_.try_lock_shared();
    }

    void write_lock() {
        shared_mutex_.lock();
    }

    bool try_write_lock() {
        return shared_mutex_.try_lock();
    }

    bool try_lock_upgrade() {
        return shared_mutex_.try_lock_upgrade();
    }

    bool try_unlock_and_write_lock() {
        return shared_mutex_.try_unlock_upgrade_and_lock();
    }

private:
    int64_t id_;
    boost::shared_mutex shared_mutex_;
};
#endif //NVM_READ_WRITE_LOCK_H
