#!/bin/bash

for ((i=1; i<=12; i++)); do
    echo "Executing upload.sh for the $i-th time"
    ./ssh.sh
done