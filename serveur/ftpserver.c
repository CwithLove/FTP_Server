#include "csapp.h"
#include "ftp_protocol.h"

#define MAX_NAME_LEN 256
#define NB_PROC 3

pid_t* childs;

void file_transfer_server(int connfd){
    request_t req;
    response_t res;
    char *buf;
    char filepath[MAXLINE];
    FILE *file;
    
    // If the number of bytes read is different from the number of bytes expected
    if (Rio_readn(connfd, &req, sizeof(request_t)) != sizeof(request_t)) {
        fprintf(stderr, "Error: Invalid request received!\n");
        return;
    }

    switch (req.type)
    {
    case GET:
        // Create the file path
        snprintf(filepath, STORAGE_PATH_LEN + MAX_FILENAME, "%s%s", STORAGE_PATH, req.filename);
        
        // Open the file
        file = fopen(filepath, "rb");
        if (file == NULL) {
            res.code = ERROR_FILE_NOT_FOUND;
            res.file_size = 0;
            rio_writen(connfd, &res, sizeof(response_t));
            return;
        }

        // Determine the size of the file
        fseek(file, 0, SEEK_END);
        res.file_size = ftell(file); // Get the current position in the file
        rewind(file); // Go back to the beginning of the file

        // Send the response to the client
        buf = malloc(res.file_size);

        // If the number of bytes read is different from the number of bytes expected
        if (fread(buf, 1, res.file_size, file) != res.file_size) {
            fprintf(stderr, "Error: reading file\n");
            res.code = ERROR_FILE_NOT_FOUND;
            res.file_size = 0;
            Rio_writen(connfd, &res, sizeof(response_t));
            free(buf);
            Fclose(file);
            return;
        }
        Fclose(file);
        res.code = SUCCESS;
        Rio_writen(connfd, &res, sizeof(response_t));
        Rio_writen(connfd, buf, res.file_size);
        free(buf);
        return;

    default:
        res.code = ERROR_INVALID_REQUEST;
        res.file_size = 0;
        Rio_writen(connfd, &res, sizeof(response_t));
        return;
    }
}

void sigchild_handler(int sig) {
    pid_t pid;
    while ((pid = waitpid(-1, 0, WNOHANG)) > 0) {
        fprintf(stdout, "child %d reaped", pid);
    }
}

void sigint_handler(int sig) {
    printf("\nArrêt du serveur...\n");
    for (int i = 0; i < NB_PROC; i++) {
        kill(childs[i], SIGINT);
    }
    exit(0);
}

void work_client(int listenfd) {
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_hostname[MAX_NAME_LEN], client_ip_string[INET_ADDRSTRLEN];

    int connfd;

    clientlen = sizeof(struct sockaddr_in);

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
        file_transfer_server(connfd);
        Close(connfd);
    }
}

int main(void) {
    int listenfd;
    pid_t pid;

    childs = (pid_t*)malloc(NB_PROC * sizeof(pid_t));

    listenfd = Open_listenfd(PORT);

    for (int i = 0; i < NB_PROC; i++) {
        pid = Fork();
        if (pid == 0) {  // Child process
            work_client(listenfd);
            exit(0);
        }
        childs[i] = pid;
    }

    Signal(SIGCHLD, sigchild_handler);
    Signal(SIGINT, sigint_handler);

    while (1) 
        Pause(); 

    return 0;
}