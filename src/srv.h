#ifndef SRV_H
#define SRV_H
#include <string.h>
#include <unistd.h>

#define MAXSHELLS 500

/* To keep track on the agents/clients */
typedef struct {
        int fp;
        char ip[16];
        int index;
        int alive;
		int ssl;
		pid_t pid;
} Agents;

void listener(unsigned, unsigned, Agents *, pid_t, char *, char *);
void interact(Agents *);
#endif
