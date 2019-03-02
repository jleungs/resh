#ifndef RESH_H
#define RESH_H
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ctype.h>

void rtrim(char *);
void usage(char *);
unsigned grab_port(char *);
void die(const char *, ...);
#endif
