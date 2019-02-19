#ifndef SRV_H
#define SRV_H
#include <string.h>

#define MAXSHELLS 500

/* To keep track on the agents/clients */
typedef struct {
        int fp;
        char *ip;
        int index;
        int alive;
} Agents;

void listener(unsigned, unsigned, Agents *);
#endif
