CFLAGS = -Wall -std=c99 -pedantic

resh: src/resh.c src/srv.c
	$(CC) $(CFLAGS) -o $@ $^
