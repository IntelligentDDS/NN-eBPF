# generate user and password dictionary
import random

num_user = 32
num_password = 32

def gen_random_string(length):
    return ''.join(random.sample('abcdefghijklmnopqrstuvwxyz0123456789', length))

with open('user.txt', 'w') as f:
    for i in range(num_user):
        print(gen_random_string(random.randint(4, 16)), file=f)
    
with open('password.txt', 'w') as f:
    for i in range(num_password):
        print(gen_random_string(random.randint(4, 16)), file=f)
    
