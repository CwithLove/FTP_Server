#include "csapp.h"
#include "ftp_protocol.h"

#define MAX_NAME_LEN 256

#define MAX_PROCESS 3

pid_t* childs;

void echo(int connfd);

void sigchild_handler(int sig) {
    pid_t pid;
    while ((pid = waitpid(-1, 0, WNOHANG)) > 0) {
        fprintf(stdout, "child %d reaped", pid);
        fflush(stdout);
    }
}

void sigint_handler(int sig) {
    printf("\nArrêt du serveur...\n");
    for (int i = 0; i < MAX_PROCESS; i++) {
        kill(childs[i], SIGINT);
    }
    exit(0);
}

void work_client(int listenfd) {
    int connfd;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];

    clientlen = (socklen_t)sizeof(clientaddr);

    while (1) {
        connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
        if (connfd == -1) {
            break;
        }
        /* determine the name of the client */
        Getnameinfo((SA *) &clientaddr, clientlen,
                    client_hostname, MAX_NAME_LEN, 0, 0, 0);
        /* determine the textual representation of the client's IP address */
        Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,
                  INET_ADDRSTRLEN);
        printf("server connected to %s (%s)\n", client_hostname,
               client_ip_string);
        //echo(connfd);
        
        Close(connfd);
    }
}

/* 
 * Note that this code only works with IPv4 addresses
 * (IPv6 is not supported)
 */
int main(int argc, char **argv)
{
    int listenfd;
    pid_t pid;


    if (argc != 1) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    // port = atoi(argv[1]);

    listenfd = Open_listenfd(PORT);

    childs = (pid_t*)malloc(MAX_PROCESS * sizeof(pid_t));

    for (int i = 0; i < MAX_PROCESS; i++) {
        pid = fork();
        if (pid == 0) {
            work_client(listenfd);
        }
        childs[i] = pid;
    }

    Signal(SIGCHLD, sigchild_handler);
    Signal(SIGINT, sigint_handler);

    // work_client(listenfd);
    while(1);
}