#!/bin/bash

date -R >> timestamp.txt
cd GoldenEye && timeout 5m ./run.sh
cd ..

date -R >> timestamp.txt
cd DVMA/xss && ./run.sh && cd ../..

date -R >> timestamp.txt
cd DVMA/brute-force && ./run.sh && cd ../..

date -R >> timestamp.txt
cd metasploit && ./run.sh && cd ..

date -R >> timestamp.txt
cd patator && ./run_ssh.sh && cd ..

date -R >> timestamp.txt
cd patator && ./run_ftp.sh && cd ..

date -R >> timestamp.txt
cd slowloris && ./run.sh && cd ..
