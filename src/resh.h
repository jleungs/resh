#ifndef RESH_H
#define RESH_H
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#define MAXSHELLS 500

/* To keep track on the agents/clients */
typedef struct {
        int fp;
        char *ip;
        int index;
        int alive;
} Agents;

struct listener_args {
	unsigned p0;
	unsigned p1;
	Agents **a;
};

void rtrim(char *);
void usage(char *);
unsigned grab_port(char *);
void die(const char *, ...);
#endif
