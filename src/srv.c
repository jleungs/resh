#define _GNU_SOURCE
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>

#include "resh.h"
#include "srv.h"

/* client background */
static volatile int clbg;

int
setupsock(int p)
{
	int lfd;
	struct sockaddr_in adr;

	memset(&adr, 0, sizeof(adr)); /* zero out struct */

	adr.sin_family = AF_INET; /* IPv4 */
	adr.sin_addr.s_addr = htonl(INADDR_ANY); /* 0.0.0.0 */
	adr.sin_port = htons(p); /* port */

	if ((lfd = socket(AF_INET, SOCK_STREAM , 0)) < 0) /* IPv4, TCP, Default based on request */
		return 0;
	if (bind(lfd, (struct sockaddr*) &adr, sizeof(adr)) < 0)
		return 0;
	if (listen(lfd, 3) < 0)
		return 0;
	return lfd;
}

void
closecon(Agents *n)
{
	n->alive = 0;
	close(n->fd);
	printf("Connection closed: %s\n", n->ip);
}

void
sighandle(int dummy)
{
	clbg = 1;
}

int
interact(Agents *n)
{
	int r, s;
	char sbuf[2048], rbuf[1024], c;
	struct sigaction sigact;
	struct pollfd pfd;
	/* Handle CTRL-C and CTRL-Z*/
	sigact.sa_handler = sighandle;
	sigaction(SIGINT, &sigact, NULL);
	sigaction(SIGTSTP, &sigact, NULL);
	/* polling to recv data from socket descriptor */
	pfd.fd = n->fd;
	pfd.events = POLLIN;
	/* reset client background if set earlier */
	clbg = 0;

	while (!clbg) {
		switch (poll(&pfd, 1, 100)) { /* poll with .01 sec timeout */
		case -1:
			fprintf(stderr, "Failed to poll agent socket\n");
			closecon(n);
			return -1;
			break;
		case 0:
			if (fgets(sbuf, sizeof(sbuf), stdin) == NULL)
				fprintf(stderr, "Failed to read command\n");
			if (n->ssl)
				s = SSL_write(n->sslfd, sbuf, strlen(sbuf));
			else
				s = send(pfd.fd, sbuf, strlen(sbuf), 0);
			if (s <= 0) {
				closecon(n);
				return -1;
			}
			break;
		default:
			read:
			if (n->ssl)
				r = SSL_read(n->sslfd, rbuf, sizeof(rbuf));
			else
				r = recv(pfd.fd, rbuf, sizeof(rbuf), 0);
			if (r <= 0) { /* agent disconnected or error */
				closecon(n);
				return -1;
			}
			rtrim(rbuf);
			printf("%.*s", r, rbuf);
			if (n->ssl && SSL_has_pending(n->sslfd)) /* pending bytes to read */
				goto read;
			break;
		}
	}
	printf("\nDo you want to background? (Y/n): ");
	c = getchar();
	switch (c) {
	case EOF:
		fprintf(stderr, "Failed to read char\n");
		return 0;
		break;
	case 'n':
	case 'N':
		closecon(n);
		return 0;
		break;
	default:
		return 0;
		break;
	}
}

void
init_ssl(void)
{
	SSL_load_error_strings();
	OpenSSL_add_ssl_algorithms();
}

SSL_CTX *
ssl_ctx(char *c, char *k)
{
	const SSL_METHOD *m;
	SSL_CTX *tmp;

	m = SSLv23_server_method();
	if (!(tmp = SSL_CTX_new(m)))
		return 0;
	SSL_CTX_set_ecdh_auto(ctx, 1);
	if (SSL_CTX_use_certificate_file(tmp, c, SSL_FILETYPE_PEM) <= 0)
		return 0;
	if (SSL_CTX_use_PrivateKey_file(tmp, k, SSL_FILETYPE_PEM) <= 0 )
		return 0;

	return tmp;
}

void *
listener(void *args)
{
	int fd0, fd1, i = 0;
	SSL_CTX *ctx = NULL;
	struct pollfd fds[2];
	struct largs *a = args;

	if (!(fd0 = setupsock(a->p0)) || !(fd1 = setupsock(a->p1)))
		die("Failed to set up socket\n");

	init_ssl();
	if (!(ctx = ssl_ctx(a->cert, a->key))) {
		die("Failed to create SSL context, see -h\n");
	}

	fds[0].fd = fd0;
	fds[0].events = POLLIN;
	fds[1].fd = fd1;
	fds[1].events = POLLIN;

	while(1) {
		int afd; /* agent fd */
		SSL *sslfd;

		size_t inc_adr_len;
		struct sockaddr_in inc_adr; /* incomming */

		memset(&inc_adr, 0, sizeof(inc_adr));
		inc_adr_len = sizeof(inc_adr);

		if (poll(fds, 2, -1) < 0)
			die("Failed to poll\n");

		if (fds[0].revents & POLLIN) {
			if ((afd = accept(fd0, (struct sockaddr*) &inc_adr,
							  (socklen_t*) &inc_adr_len)) < 0)
				fprintf(stderr, "Failed to accept connection\n");
			else
				a->pa[i]->ssl = 0;
		} else if (fds[1].revents & POLLIN) {
			if ((afd = accept(fd1, (struct sockaddr*) &inc_adr,
						      (socklen_t*) &inc_adr_len)) < 0)
				fprintf(stderr, "Failed to accept connection\n");

			sslfd = SSL_new(ctx);
			SSL_set_fd(sslfd, afd);
			if (SSL_accept(sslfd) != 1)
				fprintf(stderr, "Failed to accept SSL connection\n");
			else {
				a->pa[i]->ssl = 1;
				a->pa[i]->sslfd = sslfd;
			}
		} else {
			continue;
		}

		printf("Connection from: %s\n", inet_ntoa(inc_adr.sin_addr));

		a->pa[i]->fd = afd;
		strcpy(a->pa[i]->ip, inet_ntoa(inc_adr.sin_addr));
		a->pa[i]->index = i;
		a->pa[i]->alive = 1;
		i++; /* Next agent gets next index */
	}
}
