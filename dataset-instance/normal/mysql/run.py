import os
import random

num_iteration = 100

command = ' '.join([
    'mysqlslap',
    '-h33.33.33.220',
    '-uroot',
    '-p12345678',
    '--concurrency=%d' ,
    '--number-int-cols=%d',
    '--number-char-cols=%d',
    '--number-of-queries=%d',
    '--auto-generate-sql',
    '--auto-generate-sql-add-autoincrement',
    '--auto-generate-sql-load-type=mixed',
    '--engine=innodb',
    '--verbose'])

for i in range(num_iteration):
    run_command = command % (random.randint(10, 100),
                             random.randint(50, 100),
                             random.randint(50, 100),
                             random.randint(1000, 2000),)
    os.system(run_command)
    print(f'[{i+1}/{num_iteration}]: {run_command}')

