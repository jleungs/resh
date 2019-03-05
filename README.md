# resh
### REverse Shell Handler
A simple and minimalistic post-exploitation tool to handle and interact with reverse shells. The server listens on two ports, default 80 and 443. One for normal non-encrypted and one for SSL encrypted shells. This is to facilitate penetration tests, red teaming exercises or CTFs and handle multiple reverse shells.

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
### TODO
- [x] Re-use shell index if not alive
- [x] Tab function in shell
