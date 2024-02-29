#include <bpf/bpf.h>
#include <stdio.h>
#include "mlp_params.bpf.h"

struct Net
{
	int32_t layer_0_weight[192];
	int32_t layer_1_weight[1024];
	int32_t layer_2_weight[64];
    int64_t mean[6];
    int64_t scale[6];
};

int main()
{
    const char *nn_parameters_path = "/sys/fs/bpf/nn_parameters";
    const char *nn_idx_path = "/sys/fs/bpf/nn_idx";

    int nn_parameters_fd = bpf_obj_get(nn_parameters_path);
    int nn_idx_fd = bpf_obj_get(nn_idx_path);

    int zero_idx = 0;
    int nn_idx = 0;

    int err = bpf_map_lookup_elem(nn_idx_fd, &zero_idx, &nn_idx);
    printf("load nn index error: %s\n", strerror(err));
    printf("NN index: %d\n", nn_idx);

    int new_nn_idx = (nn_idx + 1) % 2;
    struct Net net;

    // err = bpf_map_lookup_elem(nn_parameters_fd, &new_nn_idx, &net);
    // printf("load ready nn error: %s\n", strerror(err));

    // mean
    memcpy(net.mean, mean, sizeof(int64_t) * N0);
    // scale
    memcpy(net.scale, scale, sizeof(int64_t) * N0);
    // layer 0
    memcpy(net.layer_0_weight, layer_0_weight, sizeof(int32_t) * N1 * N0);
    // layer 1
    memcpy(net.layer_1_weight, layer_1_weight, sizeof(int32_t) * N2 * N1);
    // layer 2
    memcpy(net.layer_2_weight, layer_2_weight, sizeof(int32_t) * N3 * N2);

    printf("new NN index: %d\n", new_nn_idx);

    err = bpf_map_update_elem(nn_parameters_fd, &new_nn_idx, &net, BPF_ANY);
    printf("update nn error: %s\n", strerror(err));

    err = bpf_map_update_elem(nn_idx_fd, &zero_idx, &new_nn_idx, BPF_ANY);
    printf("update nn idx error: %s\n", strerror(err));
}