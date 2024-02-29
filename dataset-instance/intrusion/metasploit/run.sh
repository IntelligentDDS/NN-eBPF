#!/bin/bash

for ((i=1; i<=128; i++)); do
    echo "Executing upload.sh for the $i-th time"
    sudo nmap 33.33.33.220 -sS -p 21,22,80,3306,5001,8080,12865
    sleep 1
done