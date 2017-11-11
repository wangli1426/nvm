//
// Created by robert on 10/11/17.
//

#ifndef NVM_CALL_BACK_H
#define NVM_CALL_BACK_H
#include <stdio.h>
#include <queue>
#include "../utils/sync.h"
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
    call_back_context(const char* name = "unnamed"): status(0), name_(name) {};
    int status;

    virtual int run() {
        if (status == 0) {
            printf("[%s]: status %d\n", name_, status);
            add_to_queue(this);
            transition_to_state(1);
            return CONTEXT_TRANSIT;
        } else {
            printf("[%s]: status %d, I am done!\n", name_, status);
            return CONTEXT_TERMINATED;
        }
    }

    void transition_to_state(int status) {
        this->status = status;
    }

protected:
    const char* name_;
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
