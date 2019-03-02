#ifndef SRV_H
#define SRV_H
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

#define MAXSHELLS 100

/* To keep track on the agents/clients */
typedef struct {
	char ip[16];
	int index;
	int fd;
	unsigned alive	:	1;
	unsigned ssl	:	1;
} Agents;

struct largs {
	unsigned short p0;
	unsigned short p1;
	Agents **pa;
	char *cert;
	char *key;
};

//void *listener(unsigned, unsigned, Agents *, char *, char *);
void *listener(void *);
int interact(Agents *);
#endif
