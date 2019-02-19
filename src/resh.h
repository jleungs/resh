#ifndef RESH_H
#define RESH_H
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void rtrim(char *);
void usage(char *);
unsigned grab_port(char *);
void die(const char *, ...);
#endif
