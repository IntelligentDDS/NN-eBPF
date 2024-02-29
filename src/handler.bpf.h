#ifndef HANDLER_BPF_H
#define HANDLER_BPF_H

#include <vmlinux.h>
#include "common.h"
#include "mlp.bpf.h"

static inline void init_flow_attribute(struct flow_attribute *attr)
{
    attr->num_packet = 0;
    attr->min_packet_length = 0x3fffffffffffffffULL;
    attr->max_duration = 0;
    attr->max_packet_length = 0;
    attr->dst_port = 0;
    attr->header_length = 0;

    attr->last_packet_time = 0;
    attr->status = 0;
}

static inline void update_flow_attribute(struct flow *f, struct tcphdr *tcp, u64 packet_length)
{
    u64 packet_time = bpf_ktime_get_ns();

    u64 extraction_start_time = bpf_ktime_get_ns();

    struct flow_attribute *attr_ptr = (struct flow_attribute *)bpf_map_lookup_elem(&flow_map, f);

    if (!attr_ptr)
    {
        struct flow_attribute attr = {};
        init_flow_attribute(&attr);
        attr.last_packet_time = packet_time;
        attr.dst_port = bpf_ntohs(tcp->dest);
        bpf_map_update_elem(&flow_map, f, &attr, BPF_NOEXIST);
        attr_ptr = (struct flow_attribute *)bpf_map_lookup_elem(&flow_map, f);
        if (!attr_ptr)
            return;
    }

    attr_ptr->num_packet += 1;

    if (packet_length < attr_ptr->min_packet_length)
        attr_ptr->min_packet_length = packet_length;

    if (packet_length > attr_ptr->max_packet_length)
        attr_ptr->max_packet_length = packet_length;

    u64 duration = packet_time - attr_ptr->last_packet_time;
    if (duration > attr_ptr->max_duration)
        attr_ptr->max_duration = duration;
    attr_ptr->last_packet_time = packet_time;

    attr_ptr->header_length += tcp->doff * 4;

    attr_ptr->total_feature_extraction_time += bpf_ktime_get_ns() - extraction_start_time;

    if (tcp->fin || tcp->rst)
    {
        attr_ptr->detection_start_time = bpf_ktime_get_ns();
    }
}

#endif