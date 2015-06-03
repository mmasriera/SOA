#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX_CONCURRENT_CHILDS 5

int currentChilds = 0,
	total  = 0;

doServiceFork(int fd) {

	int pid = fork();
	char buff2[80];

	if (pid != -1) ++currentChilds;

	if (!pid) { //child

		sprintf(buff2, " - - - child [%d] doin srvc, total: %d \n", getpid(), total);
		write(1, buff2, strlen(buff2));

		doService(fd);
		exit(0);
	}
}

doService(int fd) {

int i = 0;
char buff[80];
char buff2[80];
int ret;
int socket_fd = (int) fd;

	ret = read(socket_fd, buff, sizeof(buff));
	while(ret > 0) {
		buff[ret]='\0';
		ret = write(fd, "caracola ", 8);
		if (ret < 0) {
			perror ("Error writing to socket");
			exit(1);
		}
		ret = read(socket_fd, buff, sizeof(buff));
	}

	if (ret < 0) {
		perror ("Error reading from socket");

	}
	sprintf(buff2, " - - Server [%d] ends service\n", getpid());
	write(1, buff2, strlen(buff2));

}

void handleChildExit(int numSignal) {
	
	char buffer[80];

	sprintf(buffer, " - handle sig child, childs: %d \n", currentChilds);
	write(1, buffer, strlen(buffer));

	currentChilds--;
}

void serverReady (int socketFD) {

	int connectionFD;
	char buffer[80];

	while (1) {

		connectionFD = acceptNewConnections (socketFD);
		if (connectionFD < 0)
		{
			perror ("Error establishing connection \n");
			deleteSocket(socketFD);
			exit (1);
		}

		++total;

		if (currentChilds >= MAX_CONCURRENT_CHILDS) {

			sprintf(buffer, "WAIT\n");
			write(1, buffer, strlen(buffer));

			while(waitpid(-1, NULL, 0) > WNOHANG); //serverReady(socketFD); //espera qualsevol fill

			sprintf(buffer, "done waitin\n");
			write(1, buffer, strlen(buffer));


		}

		doServiceFork(connectionFD);

	}

}


main (int argc, char *argv[])
{
	int socketFD;
	char buffer[80];
	int ret;
	int port;


	if (argc != 2)
	{
		strcpy (buffer, "Usage: ServerSocket PortNumber\n");
		write (2, buffer, strlen (buffer));
		exit (1);
	}

	port = atoi(argv[1]);
	socketFD = createServerSocket (port);
	if (socketFD < 0)
	{
		perror ("Error creating socket\n");
		exit (1);
	}


	signal(SIGCHLD, handleChildExit);
	serverReady(socketFD);
}
