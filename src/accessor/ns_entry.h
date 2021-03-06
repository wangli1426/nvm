//
// Created by Li Wang on 10/17/17.
//

#ifndef NVM_NS_ENTRY_H
#define NVM_NS_ENTRY_H

#include <spdk/nvme.h>
#include <iostream>
#include "qpair_context.h"
#include "atomic_qpair_context.h"
#include "../utils/cpu_set.h"

struct ctrlr_entry;

namespace nvm {

    class QPair;
    class AtomicQPair;

    struct queue_pair {
        struct spdk_nvme_ctrlr	*ctrlr;
        struct spdk_nvme_ns	*ns;
        struct spdk_nvme_qpair	*qpair;
    };

    static struct spdk_nvme_ctrlr	*g_ctrlr;
    static struct spdk_nvme_ns	*g_ns;

    static bool spdk_initialized = false;

    struct ns {
        struct spdk_nvme_ctrlr	*ctrlr;
        struct spdk_nvme_ns	*ns;
    };




    class nvm_utility {
    public:
        static bool
        probe_cb(void *cb_ctx, const struct spdk_nvme_transport_id *trid,
                 struct spdk_nvme_ctrlr_opts *opts) {
            printf("Attaching to %s\n", trid->traddr);

            return true;
        }

        static void
        attach_cb(void *cb_ctx, const struct spdk_nvme_transport_id *trid,
                  struct spdk_nvme_ctrlr *ctrlr, const struct spdk_nvme_ctrlr_opts *opts) {
            int nsid, num_ns;
            struct ctrlr_entry *entry;
            struct spdk_nvme_ns *ns;
            const struct spdk_nvme_ctrlr_data *cdata = spdk_nvme_ctrlr_get_data(ctrlr);

            printf("Attached to %s\n", trid->traddr);

//        snprintf(entry->name, sizeof(entry->name), "%-20.20s (%-20.20s)", cdata->mn, cdata->sn);

            g_ctrlr = ctrlr;
//        next = g_controllers;
//        g_controllers = entry;

            /*
             * Each controller has one or more namespaces.  An NVMe namespace is basically
             *  equivalent to a SCSI LUN.  The controller's IDENTIFY data tells us how
             *  many namespaces exist on the controller.  For Intel(R) P3X00 controllers,
             *  it will just be one namespace.
             *
             * Note that in NVMe, namespace IDs start at 1, not 0.
             */
            num_ns = spdk_nvme_ctrlr_get_num_ns(ctrlr);
//        printf("Using controller %s with %d namespaces.\n", entry->name, num_ns);
            ns = spdk_nvme_ctrlr_get_ns(ctrlr, 1);
            if (ns == NULL) {
                std::cout << "fail to get namespace" << std::endl;
                return;
            }
            g_ctrlr = ctrlr;
            g_ns = ns;
        }

        static void initialize_spdk_env() {
            if (!spdk_initialized) {
                struct spdk_env_opts opts;
                spdk_env_opts_init(&opts);
//                printf("mask: %s\n", opts.core_mask);
                opts.core_mask = "0xff";
                opts.name = "nvm access interface";
                opts.shm_id = 0;

//                print_current_cpu_set();
//                set_cpu_set(32);
//                print_current_cpu_set();

                spdk_env_init(&opts);
//                spdk_unaffinitize_thread();
//                print_current_cpu_set();
//                set_cpu_set(32);
//                print_current_cpu_set();
//
//                spdk_unaffinitize_thread();
                spdk_initialized = true;
            }
        }

        static int initialize_namespace() {
            if (!g_ns) {
                initialize_spdk_env();
                printf("Initializing NVMe Controllers\n");
                return spdk_nvme_probe(NULL, NULL, probe_cb, attach_cb, NULL);
            } else {
                return 0;
            }
        }

        static QPair *allocateQPair(int length = 8) {
            spdk_nvme_qpair *qpair = spdk_nvme_ctrlr_alloc_io_qpair(g_ctrlr, NULL, 0);
            return new QPair(g_ctrlr, g_ns, qpair, length);
        }

        static AtomicQPair *allocateAtomicQPair(int length = 8) {
            spdk_nvme_qpair *qpair = spdk_nvme_ctrlr_alloc_io_qpair(g_ctrlr, NULL, 0);
            return new AtomicQPair(g_ctrlr, g_ns, qpair, length);
        }

        static uint32_t get_sector_size() {
            return spdk_nvme_ns_get_sector_size(g_ns);
        }

        static uint64_t get_sector_number() {
            return spdk_nvme_ns_get_num_sectors(g_ns);
        }

        static void detach() {
            spdk_nvme_detach(g_ctrlr);
            g_ctrlr = 0;
        }

    };
}

#endif //NVM_NS_ENTRY_H
