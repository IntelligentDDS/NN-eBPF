// SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause)
/* Copyright (c) 2022 Hengqi Chen */
#include <signal.h>
#include <unistd.h>
#include "tc.skel.h"

#define LO_IFINDEX 1

static volatile sig_atomic_t exiting = 0;

static void sig_int(int signo)
{
    exiting = 1;
}

static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
{
    return vfprintf(stderr, format, args);
}

int main(int argc, char **argv)
{
    // ingress
    DECLARE_LIBBPF_OPTS(bpf_tc_hook, tc_ingress_hook,
                        .ifindex = LO_IFINDEX,
                        .attach_point = BPF_TC_INGRESS);
    DECLARE_LIBBPF_OPTS(bpf_tc_opts, tc_ingress_opts, .handle = 1, .priority = 1);

    // egress
    DECLARE_LIBBPF_OPTS(bpf_tc_hook, tc_egress_hook,
                        .ifindex = LO_IFINDEX,
                        .attach_point = BPF_TC_EGRESS);
    DECLARE_LIBBPF_OPTS(bpf_tc_opts, tc_egress_opts, .handle = 1, .priority = 1);

    bool hook_ingress_created = false;
    bool hook_egress_created = false;

    struct tc_bpf *skel;
    int err;

    libbpf_set_print(libbpf_print_fn);

    skel = tc_bpf__open_and_load();
    if (!skel)
    {
        fprintf(stderr, "Failed to open BPF skeleton\n");
        return 1;
    }

    /* The hook (i.e. qdisc) may already exists because:
     *   1. it is created by other processes or users
     *   2. or since we are attaching to the TC ingress ONLY,
     *      bpf_tc_hook_destroy does NOT really remove the qdisc,
     *      there may be an egress filter on the qdisc
     */

    // 创建ingress hook
    err = bpf_tc_hook_create(&tc_ingress_hook);
    if (!err)
        hook_ingress_created = true;
    if (err && err != -EEXIST)
    {
        fprintf(stderr, "Failed to create TC hook: %d\n", err);
        goto cleanup;
    }

    tc_ingress_opts.prog_fd = bpf_program__fd(skel->progs.tc_ingress);
    err = bpf_tc_attach(&tc_ingress_hook, &tc_ingress_opts);
    if (err)
    {
        fprintf(stderr, "Failed to attach TC: %d\n", err);
        goto cleanup;
    }

    // 创建egress hook
    err = bpf_tc_hook_create(&tc_egress_hook);
    if (!err)
        hook_egress_created = true;
    if (err && err != -EEXIST)
    {
        fprintf(stderr, "Failed to create TC hook: %d\n", err);
        goto cleanup;
    }

    tc_egress_opts.prog_fd = bpf_program__fd(skel->progs.tc_egress);
    err = bpf_tc_attach(&tc_egress_hook, &tc_egress_opts);
    if (err)
    {
        fprintf(stderr, "Failed to attach TC: %d\n", err);
        goto cleanup;
    }

    if (signal(SIGINT, sig_int) == SIG_ERR)
    {
        err = errno;
        fprintf(stderr, "Can't set signal handler: %s\n", strerror(errno));
        goto cleanup;
    }

    printf("Successfully started! Please run `sudo cat /sys/kernel/debug/tracing/trace_pipe` "
           "to see output of the BPF program.\n");

    while (!exiting)
    {
        fprintf(stderr, ".");
        sleep(1);
    }

    tc_ingress_opts.flags = tc_ingress_opts.prog_fd = tc_ingress_opts.prog_id = 0;
    err = bpf_tc_detach(&tc_ingress_hook, &tc_ingress_opts);
    if (err)
    {
        fprintf(stderr, "Failed to detach TC ingress: %d\n", err);
        goto cleanup;
    }

    tc_egress_opts.flags = tc_egress_opts.prog_fd = tc_egress_opts.prog_id = 0;
    err = bpf_tc_detach(&tc_egress_hook, &tc_egress_opts);
    if (err)
    {
        fprintf(stderr, "Failed to detach TC egress: %d\n", err);
        goto cleanup;
    }

cleanup:
    if (hook_ingress_created)
        bpf_tc_hook_destroy(&tc_ingress_hook);
    if (hook_egress_created)
        bpf_tc_hook_destroy(&tc_ingress_hook);
    tc_bpf__destroy(skel);
    return -err;
}