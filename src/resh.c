#include <ctype.h>
#include <pthread.h>

#include "resh.h"
#include "srv.h"

/* Checks if the argument is the last given */
#define LASTARG(i) if ((i+1) >= argc) { die("No port specified\n"); }
/* clears screen, unix */
#define CLEARSCREEN printf("\033[H\033[J")

void
prompt(Agents **pa)
{
	int i;
	char cmd[2048]; /* Zero out array, used by foor loop */
	sleep(1); /* Sleep 1 sec */

	while (1) {
		printf("> ");
		if (!fgets(cmd, sizeof(cmd), stdin))
			die("Failed to read command\n");
		for (i = 0; cmd[i] != '\0'; i++)
			cmd[i] = tolower(cmd[i]);
		if (!strcmp(cmd, "ls"))
			for (i = 0; i < MAXSHELLS; i++)
				if (pa[i]->alive)
					;
		printf("%d\t%s\n", pa[0]->alive,pa[0]->ip);
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
	pthread_t listen_thread;

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

	/* Setup struct for agents */
	Agents *agents[MAXSHELLS];
	for (i = 0; i < MAXSHELLS; i++) {
		agents[i] = malloc(sizeof(Agents));
		agents[i]->ip = malloc(sizeof(char) * 16); /* max chars in ip */
	}

	/* structure for listener() args, used by pthread_create */
	struct listener_args args = { port, sslport, agents };
	if (pthread_create(&listen_thread, NULL, listener, &args))
		die("Failed to create listener thread\n");

	banner();
	prompt(agents);
	/* free everything */
	for (i = 0; i < MAXSHELLS; i++)
		free(agents[i]);
	if (pthread_join(listen_thread, NULL))
		die("Error joining threads\n");
	return 0;
}
