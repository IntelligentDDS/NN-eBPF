import paramiko
import time

hostname = '33.33.33.220'
username = 'ubuntu'
password = 'ubuntu'

with open('command.txt', 'r') as f:
    commands = [line[:-1] for line in f.readlines()]

for i, command in enumerate(commands):
    print(f'{i}/{len(commands)}: {command}')
    # build ssh connection
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    ssh.connect(hostname=hostname, username=username, password=password)
    # execute command
    _, stdout, _ = ssh.exec_command(command)
    print(stdout.read().decode())
    # close ssh connection
    ssh.close()
    time.sleep(1)