#ifndef MLP_BPF_H
#define MLP_BPF_H

#include <vmlinux.h>

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

// x: N
// W: MxN

static inline void linear_layer(const int32_t *W, const int32_t *x, int32_t *y, const int32_t M, const int32_t N)
{
    int64_t sum;
    #pragma clang loop unroll(full)
    for (int32_t i = 0; i < M; ++i)
    {
        sum = 0;
        for (int32_t j = 0; j < N; ++j)
        {
            sum += ((int64_t)(W[i * N + j])) * ((int64_t)(x[j]));
        }
        y[i] = sum >> 16;
    }
}

static inline void relu(int32_t *tensor, const int32_t size)
{
    #pragma clang loop unroll(full)
    for (int32_t i = 0; i < size; i++)
        tensor[i] = MAX(tensor[i], 0);
}

static inline void standard_scaler(int64_t *x, int32_t *y, int64_t *mean, int64_t *scale, int32_t N)
{
    #pragma clang loop unroll(full)
    for (int32_t i = 0; i < N; ++i)
    {
        if (mean[i] > x[i]) 
            y[i] = -((int32_t)(((uint64_t)mean[i] - (uint64_t)x[i]) * (1 << 16) / (uint64_t)scale[i]));
        else
            y[i] = (int32_t)(((uint64_t)x[i] - (uint64_t)mean[i]) * (1 << 16) / (uint64_t)scale[i]);
    }
}

#endif