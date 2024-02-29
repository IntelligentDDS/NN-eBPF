import os
import random

command = 'iperf -c 33.33.33.220 -n %dM -P %d'
num_iterations = 1

for i in range(num_iterations):
    run_command = command % (random.randint(10, 100), random.randint(1, 6))
    os.system(run_command)
    print(f'[{i+1}/{num_iterations}]: {run_command}')
