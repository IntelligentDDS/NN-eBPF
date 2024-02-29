# 通过zipf分布生成网络流量
import numpy as np

a = 1.1
num_flow = 256
num_core = 6

flow_set = np.random.zipf(a, num_flow) % 4096

print(":<<!")
print(np.sort(flow_set)[::-1])
print("!")

for flow in flow_set:
    num_instance = np.random.randint(num_core) + 1
    print(f"iperf -c 33.33.33.220 -n 10M -P {num_instance}")

