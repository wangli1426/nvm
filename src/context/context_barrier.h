//
// Created by Li Wang on 11/19/17.
//

#ifndef NVM_CONTEXT_BARRIER_H
#define NVM_CONTEXT_BARRIER_H

#define READ_BARRIER 0
#define WRITE_BARRIER 1
#include <queue>
#include <string>
#include <sstream>
#include "call_back.h"

//class call_back_context;
//class call_back_context;

//class barrier_token {
//public:
//    barrier_token(int64_t barrier_id, int32_t type): barrier_id_(barrier_id), type_(type) {
//    }
//
//public:
//    int64_t barrier_id_;
//    int32_t type_;
//};

class context_barrier {
    struct pending_barrier_request {
        pending_barrier_request(call_back_context* context, int32_t type): context(context), type(type) {};
        call_back_context* context;
        int32_t type;
    };

public:
    context_barrier(int64_t id, std::queue<call_back_context*> &ready_contexts): id(id), ready_contexts_(ready_contexts), read_barriers(0), write_barriers(0){};

    void set_read_barrier(call_back_context* context) {
        pending_barrier_requests.push(pending_barrier_request(context, READ_BARRIER));
        offer_barrier();
    }

    void set_write_barrier(call_back_context* context) {
        pending_barrier_requests.push(pending_barrier_request(context, WRITE_BARRIER));
        offer_barrier();
    }

    void remove_read_barrier() {
        read_barriers --;
        offer_barrier();
    }

    void remove_write_barrier() {
        write_barriers --;
        offer_barrier();
    }

    std::string to_string() {
        ostringstream ost;
        ost << "write: " << write_barriers << " read: " << read_barriers;
        return ost.str();
    }

    bool is_clear() {
        return write_barriers == 0 && read_barriers ==0;
    }

private:

    void fire_context(call_back_context* context, int type) {
//        barrier_token* token = new barrier_token(id, type);
        context->add_barrier_token(barrier_token(id, type));
        context->transition_to_next_state();
        if (context->run() == CONTEXT_TERMINATED) {
            delete context;
        }
//        ready_contexts_.push(context);
    }

    // try to offer barrier
    void offer_barrier() {
        while (!pending_barrier_requests.empty()) {
            pending_barrier_request request = pending_barrier_requests.front();
            if (request.type == READ_BARRIER) {
                // an read barrier offer can be made when there is no existing write barrier.
                if (write_barriers == 0) {
                    read_barriers ++;
                    pending_barrier_requests.pop();
                    fire_context(request.context, READ_BARRIER);
                } else {
                    // There is an existing writing barrier. No new barrier offer can be made and thus we terminate.
                    break;
                }
            } else if (request.type == WRITE_BARRIER) {
                // an write barrier offer can be made when there is no exiting write barrier or read barriers.
                if (read_barriers == 0 && write_barriers == 0) {
                    write_barriers ++;
                    pending_barrier_requests.pop();
                    fire_context(request.context, WRITE_BARRIER);
//                    break;
                } else {
                    // We terminate the offering processing, to guarantee the fairness of the write and read barrier
                    // request.
                    break;
                }
            }
            break;
        }
    }

    int64_t id;
    int32_t read_barriers;
    int32_t write_barriers;
    std::queue<pending_barrier_request> pending_barrier_requests;
    std::queue<call_back_context*> &ready_contexts_;
};

#endif //NVM_CONTEXT_BARRIER_H
