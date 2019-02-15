#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

void die(const char *errstr, ...);
void usage(char *argv0);
unsigned grab_port(char *s);
