import torch
import sys
import numpy as np

def write_params_to_file(f, name, params):
    f.write(f'static const int32_t {name}[{params.shape[0]}] = {{')
    for idx in range(params.shape[0]):
        if idx == 0:
            f.write(f'{params[idx]}')
        else:
            f.write(f', {params[idx]}')
    f.write(f'}};\n\n')

saved_stats = torch.load(sys.argv[1])
state_dict = saved_stats['state_dict']
mean = saved_stats['mean']
scale = saved_stats['scale']

# order dictionary 按顺序排序
with open('mlp_params.bpf.h', 'w') as f:
    f.write('#ifndef MLP_PARAMS_BPF_H\n')
    f.write('#define MLP_PARAMS_BPF_H\n\n')
    f.write(f'#define FIX_POINT {sys.argv[2]}\n\n')

    for i, (name, params) in enumerate(state_dict.items()):
        if i == 0:
            f.write(f'#define N{i} {params.shape[1]}\n')
        
        f.write(f'#define N{i+1} {params.shape[0]}\n')

        params = params.flatten()
        # fixed
        params = params * (2 ** int(sys.argv[2]))
        # round
        params = params.round().int()

        write_params_to_file(f, f'layer_{i}_weight', params)

    mean = (saved_stats['mean']).round().astype(np.int64)
    f.write(f"// mean\n")
    f.write(f"static const int64_t mean[{len(mean)}] = {{")
    for idx in range(len(mean)):
        f.write(f"{mean[idx]}")
        if idx < len(mean) - 1:
            f.write(", ")
    f.write("};\n\n")  

    scale = (saved_stats['scale']).round().astype(np.int64)
    f.write(f"// scale\n")
    f.write(f"static const int64_t scale[{len(scale)}] = {{")
    for idx in range(len(scale)):
        f.write(f"{scale[idx]}")
        if idx < len(scale) - 1:
            f.write(", ")
    f.write("};\n\n")  

    print('\n#endif', file=f)
