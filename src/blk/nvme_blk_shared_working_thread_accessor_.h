//
// Created by robert on 7/11/17.
//

#ifndef NVM_NVME_BLK_SHARED_WORKING_THREAD_ACCESSOR_H
#define NVM_NVME_BLK_SHARED_WORKING_THREAD_ACCESSOR_H

#include <unordered_set>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <pthread.h>
#include <queue>
#include <atomic>
#include <thread>
#include "nvme_blk_accessor.h"
#include "../tree/blk_node_reference.h"
#include "../accessor/ns_entry.h"
#include "../accessor/qpair_context.h"
#include "../utils/rdtsc.h"
#include "../utils/sync.h"
#include "../context/call_back.h"
#include "asynchronous_accessor.h"

using namespace std;

namespace tree {
    template<typename K, typename V, int CAPACITY>
    class blk_node_reference;
}

using namespace nvm;

/**
 * This class create a working thread to handle the I/O request from one or more caller threads.
 * @tparam K key tupe
 * @tparam V value tuple
 * @tparam CAPACITY: the maximum number of children node for each inner node.
 */

template<typename K, typename V, int CAPACITY>
class nvme_blk_shared_working_thread_accessor : public nvme_blk_accessor<K, V, CAPACITY> {

    enum op_type {
        read_request, write_request
    };

    struct io_request {
        void *buffer;
        blk_address addr;
        op_type type;
        Semaphore *semaphore;
    };

public:
    nvme_blk_shared_working_thread_accessor(const int &block_size, const int queue_length) :
            io_queue_length_(queue_length), nvme_blk_accessor<K, V, CAPACITY>(block_size), terminated(false), pending_requests_(0) {
    };

    ~nvme_blk_shared_working_thread_accessor() {
        terminate_io_thread();
    }

    virtual int open() {
        int status = nvm::nvm_utility::initialize_namespace();
        if (status != 0) {
            cout << "Errors in initialization of namespace." << endl;
            exit(1);
        } else {
            cout << "namespace is initialized." << endl;
        }
        this->qpair_ = nvm_utility::allocateQPair(1);
        terminiating_flag_ = false;
        tid = std::thread(working_thread, this);
        this->open_time_ = ticks();
        this->closed_ = false;
    }

    virtual int close() {

        if (!this->closed_) {
            terminate_io_thread();
            this->print_metrics("NVM(io)");
            this->closed_ = true;
        }
    }

    virtual int read(const blk_address &blk_addr, void *buffer) {
        uint64_t start = ticks();
        Semaphore semaphore;
        io_request request;
        request.semaphore = &semaphore;
        request.addr = blk_addr;
        request.buffer = buffer;
        request.type = read_request;
        enqueue_io_request(request);
        semaphore.wait();
        this->read_cycles_ += ticks() - start;
        this->reads_++;
        return this->block_size;
    }

    virtual int write(const blk_address &blk_addr, void *buffer) {
        uint64_t start = ticks();

        Semaphore semaphore;
        io_request request;
        request.semaphore = &semaphore;
        request.addr = blk_addr;
        request.buffer = buffer;
        request.type = write_request;
        enqueue_io_request(request);
        semaphore.wait();

        this->write_cycles_ += ticks() - start;
        this->writes_++;
        return this->block_size;
    }

    static string pending_ios_to_string(unordered_map<int64_t, string> *pending_io_) {
        ostringstream ost;
        for (auto it = pending_io_->begin(); it != pending_io_->end(); ++it) {
            ost << it->first << "(" << it->second << ")" << " ";
        }
        return ost.str();
    }


    static void working_thread(nvme_blk_shared_working_thread_accessor *accessor) {
        printf("working thread is created!\n");
        while (!accessor->terminiating_flag_ || accessor->get_request_queue_size() > 0 || accessor->pending_requests_.load() > 0) {
            while (accessor->get_request_queue_size() > 0 && accessor->io_queue_length_ - accessor->pending_requests_.load() > 0) {
                io_request request = accessor->dequeue_io_request();
                accessor->pending_requests_.fetch_add(1);
                callback_para *para = new callback_para;
                para->semaphore = request.semaphore;
                para->pending_requests = &accessor->pending_requests_;
                para->addr = request.addr;
                if (request.type == read_request) {
                    accessor->qpair_->submit_read_operation(request.buffer, accessor->block_size,
                                                            request.addr, callback_function, para);
                } else {
                    accessor->qpair_->submit_write_operation(request.buffer, accessor->block_size,
                                                             request.addr, callback_function, para);
                }
            }
            accessor->qpair_->process_completions(accessor->io_queue_length_);
        }
        printf("working thread is terminated.\n");
    }

    struct callback_para {
        Semaphore *semaphore;
        atomic<int> *pending_requests;
        blk_address addr;
    };

    static void callback_function(void *arg, const struct spdk_nvme_cpl *) {
        callback_para *para = static_cast<callback_para *>(arg);
        para->pending_requests->fetch_sub(1);
        para->semaphore->post();
        delete para;
    }

private:
    int32_t get_request_queue_size() {
        request_queue_lock_.acquire();
        int32_t size = request_queue_.size();
        request_queue_lock_.release();
        return size;
    }

    io_request dequeue_io_request() {
        request_queue_lock_.acquire();
        io_request ret = request_queue_.front();
        request_queue_.pop();
        request_queue_lock_.release();
        return ret;
    }

    void enqueue_io_request(const io_request &request) {
        request_queue_lock_.acquire();
        request_queue_.push(request);
        request_queue_lock_.release();
    }

    void terminate_io_thread() {
        if (!terminated) {
            terminiating_flag_ = true;
            tid.join();
            terminated = true;
        }
    }

private:
    SpinLock request_queue_lock_;
    queue<io_request> request_queue_;
    Semaphore request_semaphore_;

    atomic<int> pending_requests_;
    int32_t io_queue_length_;

    volatile bool terminiating_flag_;
    bool terminated;
    std::thread tid;
};

#endif //NVM_NVME_BLK_SHARED_WORKING_THREAD_ACCESSOR_H
