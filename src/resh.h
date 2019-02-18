#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void usage(char *argv0);
unsigned grab_port(char *s);
void die(const char *errstr, ...);
