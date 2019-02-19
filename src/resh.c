#include <ctype.h>

#include "resh.h"
#include "srv.h"

/* Checks if the argument is the last given */
#define LASTARG(i) if ((i+1) >= argc) { die("No port specified\n"); }
/* clears screen, unix */
#define CLEARSCREEN printf("\033[H\033[J")

void
prompt(Agents *pa)
{
	char cmd[2048];

	while (1) {
		printf("> ");
		printf("%s\n", pa->ip);
		if (fgets(cmd, sizeof(cmd), stdin))
			printf("%s\n", cmd);
		else
			die("Failed to read command\n");
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
  REverse Shell Handler \n";
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
	Agents agents[MAXSHELLS], *pagents;
	pagents = &agents[0]; /* copy of struct */
	pagents = malloc(sizeof(Agents));

	if (!fork()) { /* child */
		listener(port, sslport, pagents);
	} else { /* parent */
		CLEARSCREEN;
		banner();

		prompt(pagents);
	}
	free(pagents);
	return 0;
}
