//
// Created by robert on 10/11/17.
//

#ifndef NVM_CALL_BACK_H
#define NVM_CALL_BACK_H
#include <stdio.h>
#include <assert.h>
#include <queue>
#include <deque>
#include "../utils/sync.h"
//#include "context_barrier.h"

class barrier_token;
class context_barrier;
class call_back_context;

static std::queue<call_back_context*> submission_queue;

static std::queue<call_back_context*> ready_queue;

static volatile int pending_context = 0;

static SpinLock lock;
static void process_logic(volatile bool*);
static void add_to_queue(call_back_context* context);

#define CONTEXT_TERMINATED 0
#define CONTEXT_TRANSIT 1

class call_back_context {
public:
    call_back_context(const char* name = "unnamed"): status(0), current_state(0), next_state(-1), name_(std::string(name)) {};

    virtual ~call_back_context(){};
    int current_state;
    int next_state;
    int status;

    virtual int run() {
        if (status == 0) {
            printf("[%s]: status %d\n", name_.c_str(), status);
            add_to_queue(this);
            set_next_state(1);
            return CONTEXT_TRANSIT;
        } else {
            printf("[%s]: status %d, I am done!\n", name_.c_str(), status);
            return CONTEXT_TERMINATED;
        }
    }

    void transition_to_state(int status) {
        this->status = status;
    }

    void set_next_state(int state) {
        next_state = state;
    }

    void transition_to_next_state() {
        assert(next_state >= 0);
        current_state = next_state;
    }

    void add_barrier_token(barrier_token* token) {
        obtained_barriers_.push_back(token);
    }

    const char* get_name() const {
        return name_.c_str();
    }

protected:
    std::deque<barrier_token*> obtained_barriers_;
    std::string name_;
};

static void process_logic(volatile bool *terminate) {
    printf("thread starts!\n");
    while (!*terminate) {
        lock.acquire();
        if (submission_queue.size() > 0) {
            call_back_context *context = submission_queue.front();
            submission_queue.pop();
            ready_queue.push(context);
            lock.release();
        } else {
            lock.release();
        }
    }
    printf("thread terminates!\n");
}

static void submit_context(call_back_context* context) {
    lock.acquire();
    submission_queue.push(context);
    pending_context ++;
    lock.release();
}

static void add_to_queue(call_back_context* context) {
    lock.acquire();
    submission_queue.push(context);
    lock.release();
}

static int process_completion(int max = 1) {
    int processed = 0;
    for(int i = 0; i < max; i++) {
        lock.acquire();
        if (ready_queue.size() > 0) {
            call_back_context* context = ready_queue.front();
            ready_queue.pop();
            lock.release();
            const int status = context->run();
            if (status == CONTEXT_TERMINATED) {
                lock.acquire();
                pending_context--;
                lock.release();
                printf("terminated: %d -> %d\n", pending_context + 1, pending_context);
            }
            processed++;
        } else {
            lock.release();
        }
    }
    return processed;
}

static int pending_context_size() {
    lock.acquire();
    int ret = pending_context;
    lock.release();
    return ret;
}

#endif //NVM_CALL_BACK_H
