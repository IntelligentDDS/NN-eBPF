#从FTP上批量下载文件到本地
#!/bin/sh
ftp -v -n 33.33.33.220<<EOF
user ubuntu ubuntu
binary
cd /home/ubuntu/figure
lcd /tmp
prompt
mget *
bye
EOF
echo "download from ftp successfully"