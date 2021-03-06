//
// Created by robert on 12/9/17.
//

#ifndef CPU_INSTRUCTIONS_SYNC_H
#define CPU_INSTRUCTIONS_SYNC_H
/*
 * lock.h
 *
 *  Created on: Feb 20, 2014
 *      Author: wangli
 */

#include <semaphore.h>
#include <sstream>
#include <string>
#include <pthread.h>
#include <stdio.h>
#include <atomic>

//#define PTHREAD_SPIN_LOCK

using namespace std;

class Lock {
    pthread_mutex_t m;

public:
    Lock() { pthread_mutex_init(&m, NULL); }

    ~Lock() { pthread_mutex_destroy(&m); }

    inline void acquire() { pthread_mutex_lock(&m); }

    bool try_acquire() { return pthread_mutex_trylock(&m) == 0; }
    inline void release() { pthread_mutex_unlock(&m); }

    void destroy() { pthread_mutex_destroy(&m); }
};

/**
 * Non-recursive spinlock. Using `xchg` and `ldstub` as in PostgresSQL.
 */
class SpinLock {
public:
#ifndef PTHREAD_SPIN_LOCK
    SpinLock() : _l(0) {

    }
#else
    SpinLock() {
        int ret = pthread_spin_init(&t, 0);
        assert(ret == 0);
    }
    ~SpinLock() {
        int ret = pthread_spin_destroy(&t);
        assert(ret == 0);
    }
#endif

    /** Call blocks and retunrs only when it has the lock. */
    inline void acquire() {
#ifdef PTHREAD_SPIN_LOCK
        int ret = pthread_spin_lock(&t);
        assert(ret == 0);
#else
        while (tas(&_l)) {
#if defined(__i386__) || defined(__x86_64__)
            __asm__ __volatile__("pause\n");
#endif
        }
#endif
    }
    inline bool try_lock() {
#ifdef PTHREAD_SPIN_LOCK
        return pthread_spin_trylock(&t) == 0;
#else
        return !tas(&_l);
#endif
    }
    inline int fakelock() { return _l == 0 ? 0 : 1; }
    inline int getvalue() {
        // return _l==0?0:1;
        return 0;
    }

    /** Unlocks the lock object. */
    inline void release() {
#ifdef PTHREAD_SPIN_LOCK
        int ret = pthread_spin_unlock(&t);
        assert(ret == 0);
#else
        _l = 0;
#endif
    }

private:
    inline int tas(volatile char* lock) {
        register char res = 1;
#if defined(__i386__) || defined(__x86_64__)
        __asm__ __volatile__("lock xchgb %0, %1\n"
        : "+q"(res), "+m"(*lock)
        :
        : "memory", "cc");
#elif defined(__sparc__)
        __asm__ __volatile__("ldstub [%2], %0"
                         : "=r"(res), "+m"(*acquire)
                         : "r"(acquire)
                         : "memory");
#else
#error TAS not defined for this architecture.
#endif
        return res;
    }
#ifdef PTHREAD_SPIN_LOCK
    pthread_spinlock_t t;
#endif
    volatile char _l;  //__attribute__((aligned(64)))

};

static atomic<long> sema_id;
class Semaphore {
private:
    sem_t* sem;
#ifdef __APPLE__
    std::string name;
#endif

public:
    Semaphore(int initialValue = 0) {
#ifdef __APPLE__
        std::ostringstream os;
        os << "/" << sema_id++;
        name = os.str();
        if ((sem = sem_open(name.c_str(), O_CREAT|O_TRUNC, 0666, initialValue)) == SEM_FAILED)
            printf("fail to create semaphore %s\n", os.str().c_str());

//        printf("sema[%s] is created!\n", os.str().c_str());
        while(!sem_trywait(sem));
        post(initialValue);
#else
        sem = new sem_t();
        sem_init(sem, 0, initialValue);
#endif
    }

    ~Semaphore() {
#ifdef __APPLE__
        sem_close(sem);
        sem_unlink(name.c_str());
//        printf("sema is closed!\n");
#else
        sem_destroy(sem);
//        delete sem;
#endif
    }

    void set_value(int value) { sem_init(sem, 0, value); }

    inline void post() {
        sem_post(sem);
    }

    inline void post(int times) {
        for (int i = 0; i < times; i++) {
            post();
        }
    }

    inline void wait() { sem_wait(sem); }

    /*
     * The timed_wait() shall return true if the calling process/thread
     * successfully
     * performed the semaphore lock operation on the semaphore. If the calling is
     * unsuccessful, the state of the lock shall be unchanged and the function
     * shall
     * return false.
     */
    bool timed_wait(int millisecond) {
#ifdef __APPLE__
        timespec time;
        time.tv_sec = millisecond / 1000;
        millisecond = millisecond % (1000);
        time.tv_nsec = millisecond * (long)1000 * 1000;
        return false;
//        return sem_timedwait(sem, &time) == 0;
#else
        return false;
#endif
    }

    inline bool try_wait() {
        return sem_trywait(sem) ==
               0;  // if trywait() is successful, return 0, otherwise return -1.
    }

    void destroy() { sem_destroy(sem); }
    int get_value() {
        int ret;
        sem_getvalue(sem, &ret);
        return ret;
    }

};

class Barrier {
public:
    Barrier(int nThreads = 0) {
        m_nThreads = nThreads;
        int ret;
        ret = pthread_mutex_init(&m_l_SyncLock, NULL);
        if (ret != 0) printf("pthread_mutex_init failed at barrier creation.\n");
        //		printf("the init of barrier\n");
        ret = pthread_cond_init(&m_cv_SyncCV, NULL);
        if (ret != 0) printf("pthread_cond_init failed at barrier creation.\n");

        m_nSyncCount = 0;
    }
    void RegisterOneThread() {
        pthread_mutex_lock(&m_l_SyncLock);
        m_nThreads++;
        pthread_mutex_unlock(&m_l_SyncLock);
        //		printf("Barrier:: new thread registered!\n\n\n");
    }
    void UnregisterOneThread() {
        pthread_mutex_lock(&m_l_SyncLock);
        m_nThreads--;
        if (m_nThreads <= m_nSyncCount) {
            pthread_cond_broadcast(&m_cv_SyncCV);
        }
        pthread_mutex_unlock(&m_l_SyncLock);
    }

    virtual ~Barrier() {
        pthread_mutex_destroy(&m_l_SyncLock);
        pthread_cond_destroy(&m_cv_SyncCV);
    }
    void setEmpty() {
        m_nThreads = 0;
        m_nSyncCount = 0;
    }

    void reset() { m_nSyncCount = 0; }
    /*
     * One must call setEmpty() before the second's calling of Arrive()
     */
    void Arrive() {
        pthread_mutex_lock(&m_l_SyncLock);
        m_nSyncCount++;
        //		printf("cpu processor test: %d\n",m_nSyncCount);
        if (m_nSyncCount >= m_nThreads) {
            //			printf("arrive the nthreads\n");
            //			printf("arrive the broadcast\n");
            pthread_cond_broadcast(&m_cv_SyncCV);
        } else {
            //			printf("arrive the wait\n");
            pthread_cond_wait(&m_cv_SyncCV, &m_l_SyncLock);
        }

        pthread_mutex_unlock(&m_l_SyncLock);
    }

private:
    int m_nThreads;
    pthread_mutex_t m_l_SyncLock;
    pthread_cond_t m_cv_SyncCV;
    volatile int m_nSyncCount;
};

#endif //CPU_INSTRUCTIONS_SYNC_H
