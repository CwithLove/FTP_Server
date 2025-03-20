/*
 * echoclient.c - An echo client
 */
#include "csapp.h"
#include "ftp_protocol.h"

void file_transfer_client(int clientfd, char *filename) {
    char buf[MAXLINE];
    rio_t rio;
    int n;

    Rio_readinitb(&rio, clientfd);
    sprintf(buf, "GET %s\r\n", filename);
    Rio_writen(clientfd, buf, strlen(buf));
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) > 0) {
        Fputs(buf, stdout);
    }
}



int main(int argc, char **argv) {
    int clientfd;
    char *host, buf[MAXLINE];
    rio_t rio;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }
    host = argv[1];

    /*
     * Note that the 'host' can be a name or an IP address.
     * If necessary, Open_clientfd will perform the name resolution
     * to obtain the IP address.
     */
    clientfd = Open_clientfd(host, PORT);
    
    /*
     * At this stage, the connection is established between the client
     * and the server OS ... but it is possible that the server application
     * has not yet called "Accept" for this connection
     */
    printf("client connected to server OS\n"); 
    
    Rio_readinitb(&rio, clientfd);

    while (Fgets(buf, MAXLINE, stdin) != NULL) {
        Rio_writen(clientfd, buf, strlen(buf));
        if (Rio_readlineb(&rio, buf, MAXLINE) > 0) {
            Fputs(buf, stdout);
        } else { /* the server has prematurely closed the connection */
            break;
        }
    }
    Close(clientfd);
    exit(0);
}