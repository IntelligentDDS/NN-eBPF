#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <signal.h>
#include <unistd.h>
#include "xdp.skel.h"

static volatile bool exiting = false;

static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
{
    return vfprintf(stderr, format, args);
}

static void sig_handler(int sig)
{
    exiting = true;
}

struct bpf_progs_desc
{
    char name[256];
    enum bpf_prog_type type;
    int map_prog_idx;
    struct bpf_program *prog;
};

static struct bpf_progs_desc progs[] = {
    {"xdp_preprocessing", BPF_PROG_TYPE_XDP, 0, NULL},
    {"xdp_input_linear", BPF_PROG_TYPE_XDP, 1, NULL},
    {"xdp_input_relu", BPF_PROG_TYPE_XDP, 2, NULL},
    {"xdp_hidden_linear", BPF_PROG_TYPE_XDP, 3, NULL},
    {"xdp_hidden_relu", BPF_PROG_TYPE_XDP, 4, NULL},
    {"xdp_output_linear", BPF_PROG_TYPE_XDP, 5, NULL},
};

int main(int argc, char **argv)
{
    const char *interface = "ens3";
    unsigned int ifindex = if_nametoindex(interface);

    /* Set up libbpf errors and debug info callback */
    libbpf_set_print(libbpf_print_fn);

    /* Cleaner handling of Ctrl-C */
    signal(SIGINT, sig_handler);
    // signal(SIGTERM, sig_handler);

    struct xdp_bpf *skel;
    int map_progs_fd, prog_count;
    int err;

    skel = xdp_bpf__open();
    if (!skel)
    {
        fprintf(stderr, "Failed to open BPF skeleton\n");
        return 1;
    }

    err = xdp_bpf__load(skel);
    if (err)
    {
        fprintf(stderr, "Failed to load and verify BPF skeleton\n");
        goto cleanup;
    }

    map_progs_fd = bpf_object__find_map_fd_by_name(skel->obj, "progs");
    prog_count = sizeof(progs) / sizeof(progs[0]);
    for (int i = 0; i < prog_count; i++)
    {
        progs[i].prog = bpf_object__find_program_by_name(skel->obj, progs[i].name);
        if (!progs[i].prog)
        {
            fprintf(stderr, "Error: bpf_object__find_program_by_name failed\n");
            return 1;
        }
        bpf_program__set_type(progs[i].prog, progs[i].type);
    }

    for (int i = 0; i < prog_count; i++)
    {
        int prog_fd = bpf_program__fd(progs[i].prog);
        if (prog_fd < 0)
        {
            fprintf(stderr, "Error: Couldn't get file descriptor for program %s\n", progs[i].name);
            return 1;
        }

        unsigned int map_prog_idx = progs[i].map_prog_idx;
        if (map_prog_idx < 0)
        {
            fprintf(stderr, "Error: Cannot get prog fd for bpf program %s\n", progs[i].name);
            return 1;
        }
        // 给 progs map 的 map_prog_idx 插入 prog_fd
        err = bpf_map_update_elem(map_progs_fd, &map_prog_idx, &prog_fd, 0);
        if (err)
        {
            fprintf(stderr, "Error: bpf_map_update_elem failed for prog array map\n");
            return 1;
        }
    }

    //  update nn index
    int32_t nn_idx = 0;
    int32_t nn_idx_value = 1;

    err = bpf_map__update_elem(skel->maps.nn_idx, &nn_idx, sizeof(int32_t), &nn_idx_value, sizeof(int32_t), BPF_ANY);
    if (err)
    {
        fprintf(stderr, "Error: updating nn index\n");
        return 1;
    }

    struct bpf_link *link = bpf_program__attach_xdp(skel->progs.xdp_prog, ifindex);
    if (link == NULL)
    {
        fprintf(stderr, "Error: bpf_program__attach_xdp failed\n");
        return 1;
    }

    printf("Successfully started! Please run `sudo cat /sys/kernel/debug/tracing/trace_pipe` "
           "to see output of the BPF programs.\n");

    printf("pid: %d\n", getpid());

    while (!exiting)
    {
        fprintf(stderr, ".");
        sleep(60);
    }

cleanup:
    // delete pinned map
    remove("/sys/fs/bpf/nn_parameters");
    remove("/sys/fs/bpf/nn_idx");
    xdp_bpf__destroy(skel);

    return err < 0 ? -err : 0;
}