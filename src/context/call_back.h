//
// Created by robert on 10/11/17.
//

#ifndef NVM_CALL_BACK_H
#define NVM_CALL_BACK_H
#include <stdio.h>
#include <queue>
#include "../utils/sync.h"
class call_back_context;

static std::queue<call_back_context*> call_back_queue;
static SpinLock lock;
static void process_logic(volatile bool*);
static void add_to_queue(call_back_context* context);

class call_back_context {
public:
    call_back_context(const char* name = "unnamed"): status(0), name_(name) {};
    int status;

    virtual void run() {
        if (status == 0) {
            printf("[%s]: status %d\n", name_, status);
            add_to_queue(this);
            transition_to_state(1);
        } else {
            printf("[%s]: status %d, I am done!\n", name_, status);
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
        if (call_back_queue.size() > 0) {
            lock.acquire();
            call_back_context *context = call_back_queue.front();
            call_back_queue.pop();
            lock.release();
            context->run();
        }
    }
    printf("thread terminates!\n");
}

static void add_to_queue(call_back_context* context) {
    lock.acquire();
    call_back_queue.push(context);
    lock.release();
}

#endif //NVM_CALL_BACK_H
