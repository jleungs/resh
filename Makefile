CC = cc
CFLAGS =-Wall -std=c99 -pedantic
LDFLAGS=-pthread

resh: src/resh.c src/srv.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
