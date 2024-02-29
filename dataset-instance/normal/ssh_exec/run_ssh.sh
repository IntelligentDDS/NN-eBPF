for ((i=1; i<=20; i++)); do
    echo "Executing upload.sh for the $i-th time"
    python3 ssh_exec.py
done