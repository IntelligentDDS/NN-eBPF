import ftplib

class myFTP:
    def __init__(self, host, port=21):
        self.ftp = ftplib.FTP()
        self.ftp.set_pasv(False)
        self.ftp.connect(host, port)

    def login(self, user, passwd):
        self.ftp.login(user, passwd)
        # print(self.ftp.welcome)

    def download_file(self, local_path, remote_path):
        with open(local_path, 'wb') as f:
            self.ftp.retrbinary('RETR ' + remote_path, f.write)

    def upload_file(self, local_path, remote_path):
        with open(local_path, 'rb') as f:
            self.ftp.storbinary('STOR ' + remote_path, f, 1024)
            self.ftp.set_debuglevel(0)
    
    def close(self):
        self.ftp.quit()