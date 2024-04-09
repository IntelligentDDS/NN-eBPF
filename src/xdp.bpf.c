// SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause)
/* Copyright (c) 2022 Hengqi Chen */
#include <vmlinux.h>
#include <bpf/bpf_endian.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include "params.bpf.h"
#include "common.h"
#include "handler.bpf.h"
#include "mlp.bpf.h"

#define ETH_P_IP 0x0800 /* Internet Protocol packet	*/

struct
{
    __uint(type, BPF_MAP_TYPE_PROG_ARRAY);
    __uint(max_entries, 1024);
    __type(key, u32);
    __type(value, u32);
} progs SEC(".maps");

static inline int flow_tuple(struct xdp_md *ctx, struct flow *f)
{
    void *data = (void *)(long)ctx->data;
    void *data_end = (void *)(long)ctx->data_end;

    struct ethhdr *eth = data;

    if (eth + 1 > (struct ethhdr *)data_end)
    {
        return -1;
    }

    if (eth->h_proto != bpf_htons(ETH_P_IP))
    {
        return -1;
    }

    struct iphdr *ip = data + sizeof(struct ethhdr);
    if (ip + 1 > (struct iphdr *)data_end)
    {
        return -1;
    }

    if (ip->protocol != IPPROTO_TCP)
    {
        return -1;
    }

    struct tcphdr *tcp = data + sizeof(struct ethhdr) + sizeof(struct iphdr);
    if (tcp + 1 > (struct tcphdr *)data_end)
    {
        return -1;
    }

    f->saddr = ip->saddr;
    f->daddr = ip->daddr;
    f->sport = tcp->source;
    f->dport = tcp->dest;

    return 0;
}

SEC("xdp")
int xdp_prog(struct xdp_md *ctx)
{
    struct flow f = {};

    if (flow_tuple(ctx, &f) < 0)
    {
        return XDP_PASS;
    }

    void *data = (void *)(long)ctx->data;
    void *data_end = (void *)(long)ctx->data_end;

    // 33.33.33.73 == 555819337

    // if ((bpf_ntohs(tcp->dest) == 5001))
    if ((bpf_ntohl(f.saddr) == 555819337))
    {
        struct iphdr *ip = data + sizeof(struct ethhdr);
        if (ip + 1 > (struct iphdr *)data_end)
        {
            return XDP_PASS;
        }

        struct tcphdr *tcp = data + sizeof(struct ethhdr) + sizeof(struct iphdr);
        if (tcp + 1 > (struct tcphdr *)data_end)
        {
            return XDP_PASS;
        }

        u64 packet_length = bpf_ntohs(ip->tot_len) - ip->ihl * 4;

        update_flow_attribute(&f, tcp, packet_length);

        if (tcp->fin || tcp->rst)
        {
            int32_t idx = 0;
            int32_t *idx_ptr = bpf_map_lookup_elem(&nn_idx, &idx);
            struct flow_attribute *attr_ptr = (struct flow_attribute *)bpf_map_lookup_elem(&flow_map, &f);

            if (idx_ptr && attr_ptr)
            {
                attr_ptr->nn_idx = *idx_ptr;
                bpf_tail_call(ctx, &progs, 0);
            }
        }
    }

    return XDP_PASS;
}

SEC("xdp")
int xdp_preprocessing(struct xdp_md *ctx)
{
    struct flow f = {};

    if (flow_tuple(ctx, &f) < 0)
    {
        return XDP_PASS;
    }

    struct flow_attribute *attr_ptr = (struct flow_attribute *)bpf_map_lookup_elem(&flow_map, &f);
    if (!attr_ptr)
        return XDP_PASS;

    // bpf_printk("[%d->%d] %lld,%lld,%lld,%lld,%lld,%lld",
    //            bpf_ntohs(f.sport),
    //            bpf_ntohs(f.dport),
    //            attr_ptr->max_packet_length,
    //            attr_ptr->max_duration,
    //            attr_ptr->min_packet_length,
    //            attr_ptr->dst_port,
    //            attr_ptr->header_length,
    //            attr_ptr->num_packet);
    // bpf_printk("[%d->%d] NN index: %d",
    //            bpf_ntohs(f.sport),
    //            bpf_ntohs(f.dport),
    //            attr_ptr->nn_idx);
    // ！！！！！！！
    // return XDP_PASS;

    struct Net *net = bpf_map_lookup_elem(&nn_parameters, &(attr_ptr->nn_idx));
    if (!net)
        return XDP_PASS;

    int64_t x[6] = {attr_ptr->max_packet_length,
                    attr_ptr->max_duration,
                    attr_ptr->min_packet_length,
                    attr_ptr->dst_port,
                    attr_ptr->header_length,
                    attr_ptr->num_packet};

    standard_scaler(x, attr_ptr->hidden2, net->mean, net->scale, 6);

    // bpf_printk("%d %d %d %d %d %d",
    //            attr_ptr->hidden2[0],
    //            attr_ptr->hidden2[1],
    //            attr_ptr->hidden2[2],
    //            attr_ptr->hidden2[3],
    //            attr_ptr->hidden2[4],
    //            attr_ptr->hidden2[5]);
    // return XDP_PASS;

    bpf_tail_call(ctx, &progs, 1);

    return XDP_PASS;
}

SEC("xdp")
int xdp_input_linear(struct xdp_md *ctx)
{
    struct flow f = {};

    if (flow_tuple(ctx, &f) < 0)
    {
        return XDP_PASS;
    }

    struct flow_attribute *attr_ptr = (struct flow_attribute *)bpf_map_lookup_elem(&flow_map, &f);
    if (!attr_ptr)
        return XDP_PASS;

    struct Net *net = bpf_map_lookup_elem(&nn_parameters, &(attr_ptr->nn_idx));
    if (!net)
        return XDP_PASS;

    linear_layer(net->layer_0_weight, attr_ptr->hidden2, attr_ptr->hidden1, 32, 6);

    // bpf_printk("%d %d %d %d %d %d",
    //            attr_ptr->hidden2[0],
    //            attr_ptr->hidden2[1],
    //            attr_ptr->hidden2[2],
    //            attr_ptr->hidden2[3],
    //            attr_ptr->hidden2[4],
    //            attr_ptr->hidden2[5]);

    bpf_tail_call(ctx, &progs, 2);

    return XDP_PASS;
}

SEC("xdp")
int xdp_input_relu(struct xdp_md *ctx)
{
    struct flow f = {};

    if (flow_tuple(ctx, &f) < 0)
    {
        return XDP_PASS;
    }

    struct flow_attribute *attr_ptr = (struct flow_attribute *)bpf_map_lookup_elem(&flow_map, &f);
    if (!attr_ptr)
        return XDP_PASS;

    relu(attr_ptr->hidden1, 32);

    // bpf_printk("%d %d %d %d %d %d",
    //            attr_ptr->hidden1[0],
    //            attr_ptr->hidden1[1],
    //            attr_ptr->hidden1[2],
    //            attr_ptr->hidden1[3],
    //            attr_ptr->hidden1[4],
    //            attr_ptr->hidden1[5]);

    bpf_tail_call(ctx, &progs, 3);

    return XDP_PASS;
}

SEC("xdp")
int xdp_hidden_linear(struct xdp_md *ctx)
{
    struct flow f = {};

    if (flow_tuple(ctx, &f) < 0)
    {
        return XDP_PASS;
    }

    struct flow_attribute *attr_ptr = (struct flow_attribute *)bpf_map_lookup_elem(&flow_map, &f);
    if (!attr_ptr)
        return XDP_PASS;

    struct Net *net = bpf_map_lookup_elem(&nn_parameters, &(attr_ptr->nn_idx));
    if (!net)
        return XDP_PASS;

    linear_layer(net->layer_1_weight, attr_ptr->hidden1, attr_ptr->hidden2, 32, 32);
    // bpf_printk("%d %d %d %d %d %d",
    //            attr_ptr->hidden1[0],
    //            attr_ptr->hidden1[1],
    //            attr_ptr->hidden1[2],
    //            attr_ptr->hidden1[3],
    //            attr_ptr->hidden1[4],
    //            attr_ptr->hidden1[5]);

    bpf_tail_call(ctx, &progs, 4);

    return XDP_PASS;
}

SEC("xdp")
int xdp_hidden_relu(struct xdp_md *ctx)
{
    struct flow f = {};

    if (flow_tuple(ctx, &f) < 0)
    {
        return XDP_PASS;
    }

    struct flow_attribute *attr_ptr = (struct flow_attribute *)bpf_map_lookup_elem(&flow_map, &f);
    if (!attr_ptr)
        return XDP_PASS;

    relu(attr_ptr->hidden2, 32);

    // bpf_printk("%d %d %d %d %d %d",
    //            attr_ptr->hidden2[0],
    //            attr_ptr->hidden2[1],
    //            attr_ptr->hidden2[2],
    //            attr_ptr->hidden2[3],
    //            attr_ptr->hidden2[4],
    //            attr_ptr->hidden2[5]);

    bpf_tail_call(ctx, &progs, 5);

    return XDP_PASS;
}

SEC("xdp")
int xdp_output_linear(struct xdp_md *ctx)
{
    struct flow f = {};

    if (flow_tuple(ctx, &f) < 0)
    {
        return XDP_PASS;
    }

    struct flow_attribute *attr_ptr = (struct flow_attribute *)bpf_map_lookup_elem(&flow_map, &f);
    if (!attr_ptr)
        return XDP_PASS;

    struct Net *net = bpf_map_lookup_elem(&nn_parameters, &(attr_ptr->nn_idx));
    if (!net)
        return XDP_PASS;

    linear_layer(net->layer_2_weight, attr_ptr->hidden2, attr_ptr->hidden1, 2, 32);

    bpf_printk("avg feature extraction time: %lld, detection time: %lld, label: %d",
               attr_ptr->total_feature_extraction_time / attr_ptr->num_packet,
               bpf_ktime_get_ns() - attr_ptr->detection_start_time,
               (attr_ptr->hidden1[0] > attr_ptr->hidden1[1]) ? 0 : 1);

    return XDP_PASS;
}

char __license[] SEC("license") = "GPL";