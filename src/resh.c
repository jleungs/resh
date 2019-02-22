#include <ctype.h>
#include <pthread.h>
#include <sys/mman.h>

#include "resh.h"
#include "srv.h"

/* Checks if the argument is the last given */
#define LASTARG(i) if ((i+1) >= argc) { die("No port specified\n"); }
/* clears screen, unix */
#define CLEARSCREEN printf("\033[H\033[J")

void
prompt(int p0, int p1, Agents *pa)
{
	char cmd[30], *arg0, *arg1 = NULL;
	int i;
	int l = sizeof(cmd);

	printf("Now listening on port\nNo SSL:\t%d\nSSL:\t%d\n", p0, p1);

	while (1) {
		printf("resh> ");
		if (!fgets(cmd, sizeof(cmd), stdin))
			die("Failed to read command\n");
		for (i = 0; cmd[i] != '\0'; i++) {
			cmd[i] = tolower(cmd[i]);
			if (cmd[i] == '\n') /* strip newline */
				cmd[i] = '\0';
		}
		arg0 = strtok(cmd, " ");
		arg1 = strtok(NULL, " ");

		if (!strncmp(arg0, "help", l) || !strncmp(arg0, "?", l)) {
			printf("Commands\n--------\n"
				   "help\tPrints this menu\n"
				   "agents\tList active agents\n"
				   "listeners\tList listener ports\n"
				   "interact\tInteract with and agent\n");
		} else if (!strncmp(arg0, "agents", l)) {
			printf("Active Agents\n");
			for (i = 0; i < MAXSHELLS; i++)
				if (pa[i].alive)
					printf("Index: %d\tSourceIP: %s\n", pa[i].index, pa[i].ip);
		} else if (!strncmp(arg0, "listeners", l)) {
			printf("Listening on port\nNo SSL:\t%d\nSSL:\t%d\n\n", p0, p1);
		} else if (!strncmp(arg0, "interact", l)) {
			if (!arg1) {
				printf("Specify an index, example:\n> interact 1\n\n");
				continue;
			}
			printf("conttest\n");
		}
	}
}

void
banner(void)
{
	const char *b =
"                  _     \n\
                 | |    \n\
  ____ _____  ___| |__  \n\
 / ___) ___ |/___)  _ \\\n\
| |   | ____|___ | | | |\n\
|_|   |_____|___/|_| |_|\n\
  REverse Shell Handler \n\n";
	CLEARSCREEN;
	printf("%s", b);
}

void
rtrim(char *s)
{
	int len = strlen(s);

	while (isspace(s[len  - 1]))
		--len;
	s[len] = '\n';
	s[len + 1] = '\0';
}

void
usage(char *argv0)
{
	die("usage: %s [-hv] [-l port] [-L port] [-i]\n\n"
		"-l port\t\tListen port for no encryption. Default 80\n"
		"-L port\t\tListen port for SSL encrypted traffic. Default 443\n"
		"-i\t\tInteractive mode, use after the listening daemon is started\n"
		, argv0);
}

unsigned
grab_port(char *s)
{
	int i;
	long p;

	for (i = 0; s[i] != '\0'; i++) {
		if (!isdigit(s[i]))
			die("Specify a valid port\n");
	}
	p = atol(s);
	if (p > 0xffff || p == 0)
		die("Specify a valid port\n");
	return p;
}

void
die(const char *errstr, ...)
{
        va_list ap;

        va_start(ap, errstr);
        vfprintf(stderr, errstr, ap);
        va_end(ap);
        exit(1);
}

int
main(int argc, char **argv)
{
	int i;
	unsigned port = 80, sslport = 443; /* Default ports */
	pid_t pid;

	for (i = 1; i < argc; i++) {
		if (*argv[i] == '-') {
			switch (*(++argv[i])) {
			case 'l': /* Listen */
				LASTARG(i);
				port = grab_port(argv[++i]);
				break;
			case 'L': /* SSL Listen */
				LASTARG(i);
				sslport = grab_port(argv[++i]);
				break;
			case 'v': /* version & help fall through */
			case 'h':
				usage(argv[0]);
				break;
			default:
				break;
			}
		}
	}
	if (port < 1024 && sslport < 1024 && getuid() != 0)
		die("%d or %d needs root privileges to listen on\n", port, sslport);

	/* Setup struct for agents and mmap for shared memory between forks */
	Agents agents[MAXSHELLS];
	Agents *pagents = mmap(agents, sizeof(agents),
						   PROT_READ|PROT_WRITE,
						   MAP_SHARED|MAP_ANONYMOUS,
						   -1, 0);
	if ((pid = fork()) < 0 ) {
		die("Failed to fork\n");
	} else if (!pid){ /* child */
		banner();
		prompt(port, sslport, pagents);
	} else { /* parent */
		listener(port, sslport, pagents, pid);
	}
	return 0;
}
