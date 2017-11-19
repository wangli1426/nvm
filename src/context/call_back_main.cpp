//
// Created by robert on 10/11/17.
//
#include <stdio.h>
#include <thread>
#include <pthread.h>
#include <unistd.h>
#include "call_back.h"

class my_context: public call_back_context {
public:
    explicit my_context(const char* name): call_back_context(name) {};
    int run() {
        if (status == 0) {
            printf("[%s]: status %d\n", name_, status);
            add_to_queue(this);
            set_next_state(1);
            return CONTEXT_TRANSIT;
        } else if (status == 1) {
            printf("[%s]: status %d\n", name_, status);
            add_to_queue(this);
            set_next_state(2);
            return CONTEXT_TRANSIT;
        } else {
            printf("[%s]: status %d, I am done!\n", name_, status);
            return CONTEXT_TERMINATED;
        }
    }
};

int main() {
    pthread_t pid;
    bool terminate_flag;
    std::thread t = std::thread(process_logic, &terminate_flag);
    call_back_context* context1 = new call_back_context("C1");
    call_back_context* context2 = new my_context("C2");
    submit_context(context1);
    submit_context(context2);

    while(pending_context_size()) {
        process_completion();
    }

    terminate_flag = true;
    t.join();
}
