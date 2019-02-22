CC = cc
CFLAGS =-Wall -std=gnu99 -pedantic
LDFLAGS=-pthread -lssl -lcrypt

all: resh

resh: src/resh.c src/srv.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

cert:
	mkdir -p ./certs
	openssl req -newkey rsa:2048 -new -nodes -x509 -days 3650 -keyout certs/key.pem -out certs/cert.pem\
		-subj "/C=NA/ST=resh/L=resh/O=resh/OU=resh/CN=resh"
