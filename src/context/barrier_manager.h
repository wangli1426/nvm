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
                printf("[%lld]: %s\n", it->first, it->second->to_string().c_str());
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

private:
    context_barrier* get_or_create_barrier(int64_t node_id) {
        if (barriers_.find(node_id) == barriers_.cend()) {
            barriers_[node_id] = new context_barrier(node_id);
        }
        return barriers_[node_id];
    }

private:
    unordered_map<int64_t, context_barrier*> barriers_;
};

#endif //NVM_BARRIER_MANAGER_H
