#include "arg.h"

/* Checks if the argument is the last given */
#define LASTARG(i) if ((i+1) >= argc) { die("No port specified\n"); }

int
main(int argc, char **argv)
{
	/* If no arguments */
	if (argc < 2)
		usage(argv[0]);

	int i;
	unsigned port, sslport;

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
			case 'i':
				; /* Interact */
			case 'v': /* version & help fall through */
			case 'h':
			default:
				usage(argv[0]);
				break;
			}
		}
	}
	printf("%u - %u\n", port,sslport);
	return 0;
}
