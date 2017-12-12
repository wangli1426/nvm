//
// Created by Li Wang on 11/19/17.
//

#ifndef NVM_BARRIER_MANAGER_H
#define NVM_BARRIER_MANAGER_H

#include <unordered_map>
#include "context_barrier.h"
#include "call_back.h"

using namespace std;
class barrier_manager {
public:

    ~ barrier_manager() {
        print_uncleared_barriers();
    }

    void print_uncleared_barriers() {
        bool all_cleared = true;
        for (auto it = barriers_.begin(); it != barriers_.end(); ++it) {
            if (!it->second->is_clear()) {
                all_cleared = false;
                printf("[%ld]: %s\n", it->first, it->second->to_string().c_str());
            }
        }
    }
    void request_write_barrier(int64_t node_id, call_back_context* context) {
        context_barrier* barrier = get_or_create_barrier(node_id);
        barrier->set_write_barrier(context);
    }

    void request_read_barrier(int64_t node_id, call_back_context* context) {
        context_barrier* barrier = get_or_create_barrier(node_id);
        barrier->set_read_barrier(context);
    }

    void remove_read_barrier(int64_t node_id) {
        context_barrier* barrier = get_or_create_barrier(node_id);
        barrier->remove_read_barrier();
    }

    void remove_write_barrier(int64_t node_id) {
        context_barrier* barrier = get_or_create_barrier(node_id);
        barrier->remove_write_barrier();
    }

    void process_ready_context(int32_t max) {
        while(ready_contexts_.size() > 0 && max > 0) {
            call_back_context* context = ready_contexts_.front();
            ready_contexts_.pop();
            if (context->run() == CONTEXT_TERMINATED) {
                delete context;
            }
            max--;
        }
    }

private:
    context_barrier* get_or_create_barrier(int64_t node_id) {
        unordered_map<int64_t, context_barrier*>::const_iterator it = barriers_.find(node_id);
        if (it == barriers_.cend()) {
            context_barrier* ret = new context_barrier(node_id, ready_contexts_);
            barriers_[node_id] = ret;
            return ret;
        }
        return it->second;
    }

private:
    unordered_map<int64_t, context_barrier*> barriers_;
    std::queue<call_back_context*> ready_contexts_;
};

#endif //NVM_BARRIER_MANAGER_H
