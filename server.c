#include "csapp.h"

#define MAX_NAME_LEN 256

#define MAX_PROCESS 3

void echo(int connfd);

void sigchild_handler(int sig) {
    pid_t pid;
    while ((pid = waitpid(-1, 0, WNOHANG)) > 0) {
        fprintf(stdout, "child %d reaped", pid);
    }
}

void sigint_handler(int sig) {
    printf("\nArrêt du serveur...\n");
    kill(-getpgrp(), SIGINT);
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
        echo(connfd);
        Close(connfd);
    }
    exit(0);
}

/* 
 * Note that this code only works with IPv4 addresses
 * (IPv6 is not supported)
 */
int main(int argc, char **argv)
{
    int listenfd, port;
    pid_t pid;

    Signal(SIGCHLD, sigchild_handler);
    if (Signal(SIGINT, sigint_handler) == SIG_ERR) {
        unix_error("Signal");
    }

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    port = atoi(argv[1]);

    listenfd = Open_listenfd(port);

    for (int i = 0; i < MAX_PROCESS; i++) {
        pid = fork();
        if (pid == 0) {
            work_client(listenfd);
        }
    }

    work_client(listenfd);
}