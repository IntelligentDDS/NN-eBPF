for ((i=1; i<=200; i++)); do
    echo "Executing upload.sh for the $i-th time"
    python3 -u xss.py http://33.33.33.220/dvwa
done
