#!/bin/bash

for ((i=1; i<=100; i++)); do
    echo "Executing download.sh for the $i-th time"
    ./download.sh
done