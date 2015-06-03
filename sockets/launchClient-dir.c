#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>


int max_concurrent = 0;
int current_concurrent = 0;

void trat_sigchld (int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0)
        current_concurrent --;
}

main(int argc, char *argv[]) {
    int i, nclients, pidh, fd;
    char buf[80], *dirName;
    int msec_elapsed;
    double avg_exeT, clientsperS;
    struct timeval init_t, end_t;
    struct stat st = {0};



    if (argc < 5) {
	sprintf(buf, "usage: %s num_clients num_it hostname port [FolderName]\n", argv[0]);
	write(1, buf, strlen(buf));
	exit(1);
    }

    signal(SIGCHLD,trat_sigchld);
    nclients = atoi(argv[1]);
    dirName = "execClient";
    if(argc == 6)
    {
        dirName = argv[5];
    }

    /* crea el directori exec */
    if(stat(dirName,&st) == -1)
        if ((mkdir(dirName,0700)) == -1)
        {
            perror("error creating dir\n");
            exit(1);
        }
    
    sprintf(buf,"%s/launch_info", dirName);
    fd = open(buf, O_CREAT|O_TRUNC|O_WRONLY, 0600);
    gettimeofday(&init_t, NULL);
    for (i=0; i<nclients;i++){
	pidh =fork();
	switch (pidh){
            case -1: perror("Error creating client process");
                exit(1);
            case 0:  
                sprintf(buf, "%s/client_%d", dirName, i);
                fd = open (buf, O_CREAT|O_TRUNC|O_WRONLY, 0600); 
                if (fd < 0) {
                    perror("Opening client results file");
                } else {
                    dup2(fd, 2);
                    close(fd);
                }

                execlp("./clientSocket", "clientSocket", argv[2], argv[3], argv[4],NULL);
		perror("Error loading clientSocket");
		exit(1);
	}
	current_concurrent++;
	if (current_concurrent > max_concurrent) {
            max_concurrent = current_concurrent;
            sprintf(buf,"CURRENT MAX CONCURRENT CLIENTS: %d\n", max_concurrent);
            write(fd, buf, strlen(buf));
	}
    }
    while (waitpid(-1,NULL,0) > 0);
    sprintf(buf,"MAX CONCURRENT CLIENTS: %d\n", max_concurrent);
    write(fd, buf, strlen(buf));
    gettimeofday(&end_t, NULL);
    msec_elapsed = (end_t.tv_sec*1000)+(end_t.tv_usec/1000) - (init_t.tv_sec*1000)+(init_t.tv_usec/1000);
    sprintf(buf, "Time %d\n", msec_elapsed);
    write (fd, buf,strlen(buf));
    close(fd);

}