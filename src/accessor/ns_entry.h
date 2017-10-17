//
// Created by Li Wang on 10/17/17.
//

#ifndef NVM_NS_ENTRY_H
#define NVM_NS_ENTRY_H

#include <spdk/nvme.h>
#include "qpair_context.h"

namespace nvm {

    class QPair;

    struct ns_entry {
        struct spdk_nvme_ctrlr	*ctrlr;
        struct spdk_nvme_ns	*ns;
        struct spdk_nvme_qpair	*qpair;
    };

    static struct spdk_nvme_ctrlr	*g_ctrlr;
    static struct spdk_nvme_ns	*g_ns;

    struct ns {
        struct spdk_nvme_ctrlr	*ctrlr;
        struct spdk_nvme_ns	*ns;
    };

    static bool
    probe_cb(void *cb_ctx, const struct spdk_nvme_transport_id *trid,
             struct spdk_nvme_ctrlr_opts *opts)
    {
        printf("Attaching to %s\n", trid->traddr);

        return true;
    }

    static void
    attach_cb(void *cb_ctx, const struct spdk_nvme_transport_id *trid,
              struct spdk_nvme_ctrlr *ctrlr, const struct spdk_nvme_ctrlr_opts *opts)
    {
        int nsid, num_ns;
        struct ctrlr_entry *entry;
        struct spdk_nvme_ns *ns;
        const struct spdk_nvme_ctrlr_data *cdata = spdk_nvme_ctrlr_get_data(ctrlr);

        entry = reinterpret_cast<ctrlr_entry*>(malloc(sizeof(struct ctrlr_entry)));
        if (entry == NULL) {
            perror("ctrlr_entry malloc");
            exit(1);
        }

        printf("Attached to %s\n", trid->traddr);

        snprintf(entry->name, sizeof(entry->name), "%-20.20s (%-20.20s)", cdata->mn, cdata->sn);

        entry->ctrlr = ctrlr;
        entry->next = g_controllers;
        g_controllers = entry;

        /*
         * Each controller has one or more namespaces.  An NVMe namespace is basically
         *  equivalent to a SCSI LUN.  The controller's IDENTIFY data tells us how
         *  many namespaces exist on the controller.  For Intel(R) P3X00 controllers,
         *  it will just be one namespace.
         *
         * Note that in NVMe, namespace IDs start at 1, not 0.
         */
        num_ns = spdk_nvme_ctrlr_get_num_ns(ctrlr);
        printf("Using controller %s with %d namespaces.\n", entry->name, num_ns);
        ns = spdk_nvme_ctrlr_get_ns(ctrlr, nsid);
        if (ns == NULL) {
            return;
        }
        g_ctrlr = ctrlr;
        g_ns = ns;
    }


    static void initialize_namespace() {
        rc = spdk_nvme_probe(NULL, NULL, probe_cb, attach_cb, NULL);
    }

    static QPair allocateQPair(int length = 0) {
        spdk_nvme_qpair qpair = spdk_nvme_ctrlr_alloc_io_qpair(g_namespaces->ctrlr, NULL, 0);
        struct ns_entry ns_en;
        ns_en.ctrlr = g_ctrlr;
        ns_en.ns = g_ns;
        ns_en.qpair = qpair;
        return QPair(ns_en, length);
    }

}

#endif //NVM_NS_ENTRY_H
