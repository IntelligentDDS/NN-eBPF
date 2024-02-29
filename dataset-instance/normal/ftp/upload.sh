#从本地向FTP批量上传文档
#!/bin/sh
ftp -v -n 33.33.33.220<<EOF
user ubuntu ubuntu
binary
hash
cd /tmp
lcd /home/ubuntu/figure
prompt
mput *
bye
#here document
EOF
echo "commit to ftp successfully"