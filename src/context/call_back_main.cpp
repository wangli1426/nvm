//
// Created by robert on 10/11/17.
//
#include <stdio.h>
#include "call_back.h"

class my_context: public call_back_context {
public:
    explicit my_context(const char* name): call_back_context(name) {};
    void run() {
        if (status == 0) {
            printf("[%s]: status %d\n", name_, status);
            add_to_queue(this);
        } else if (status == 1) {
            printf("[%s]: status %d\n", name_, status);
            add_to_queue(this);
        } else {
            printf("[%s]: status %d, I am done!\n", name_, status);
        }
    }
};

int main() {
    call_back_context* context1 = new call_back_context("C1");
    call_back_context* context2 = new my_context("C2");
    add_to_queue(context1);
    add_to_queue(context2);
    process();
}
