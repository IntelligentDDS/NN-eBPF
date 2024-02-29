#!/bin/bash

date -R >> timestamp.txt
cd ftp && ./download.sh && cd ..

date -R >> timestamp.txt
cd ftp && ./run_upload.sh && cd ..

date -R >> timestamp.txt
cd http && ./run.sh && cd ..

date -R >> timestamp.txt
cd iperf && ./run.sh && cd ..

date -R >> timestamp.txt
cd ssh_exec && ./run_ssh.sh && cd ..