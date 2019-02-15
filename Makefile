CFLAGS = -Wall -std=c99 -pedantic

resh: src/resh.c src/arg.c
	$(CC) $(CFLAGS) -o $@ $^
