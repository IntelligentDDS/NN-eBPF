#!/bin/bash

for ((i=1; i<=16; i++)); do
    echo "Executing upload.sh for the $i-th time"
    ./upload.sh
done