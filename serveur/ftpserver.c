#include "csapp.h"
#include "ftp_protocol.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_NAME_LEN 256
#define NB_PROC 3

pid_t* childs;

int file_transfer_server(int connfd) {
    request_t req;
    response_t res;
    char filepath[MAXLINE];
    struct stat st;
    int fd;
    ssize_t n;
    // Read the client's request.
    if ((n = rio_readn(connfd, &req, sizeof(request_t))) != sizeof(request_t)) {
        if (n == 0) {
            fprintf(stderr, "Bye bye! Client disconnected\n");
            return 0;
        } else if (n < 0) {
            perror("Error reading request");
            return 1;
        } else {
            fprintf(stderr, "Error: Invalid request received!\n");
            return 1;
        }
    }

    switch (req.type) {
        case GET:
            /* Constitution du chemin complet du fichier */
            snprintf(filepath, STORAGE_PATH_LEN + MAX_FILENAME, "%s%s", STORAGE_PATH, req.filename);
            
            /* Ouverture du fichier */
            if ((fd = open(filepath, O_RDONLY)) < 0) {
                perror(filepath);
                res.code = ERROR_FILE_NOT_FOUND;
                res.file_size = 0;
                rio_writen(connfd, &res, sizeof(response_t));
                return 1;
            }
            
            /* Récupération de la taille du fichier */
            if (fstat(fd, &st) < 0) {
                perror("Error getting file size");
                res.code = ERROR_FILE_NOT_FOUND;
                res.file_size = 0;
                rio_writen(connfd, &res, sizeof(response_t));
                close(fd);
                return 1;
            }
            res.file_size = st.st_size;
            res.code = SUCCESS;

            /* Envoi de l’en-tête de réponse (code + taille du fichier) */
            rio_writen(connfd, &res, sizeof(response_t));

            /* Envoi du fichier par blocs */
            uint64_t remaining = res.file_size;
            char buffer[MESSAGE_SIZE];
            while (remaining > 0) {
                uint64_t bytes_to_read = (remaining < MESSAGE_SIZE) ? remaining : MESSAGE_SIZE;
                uint64_t n = read(fd, buffer, bytes_to_read);
                if (n < 0) {
                    perror("Error reading file");
                    break;
                }
                rio_writen(connfd, buffer, n);
                remaining -= n;
            }
            close(fd);
            break;

        default:
            res.code = ERROR_INVALID_REQUEST;
            res.file_size = 0;
            rio_writen(connfd, &res, sizeof(response_t));
            break;
    }     
    return 1;
}

void sigchild_handler(int sig) {
    pid_t pid;
    while ((pid = waitpid(-1, 0, WNOHANG)) > 0) {
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
        Getnameinfo((SA *)&clientaddr, clientlen,
                    client_hostname, MAX_NAME_LEN, 0, 0, 0);
        /* determine the textual representation of the client's IP address */
        Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,
                  INET_ADDRSTRLEN);
        printf("server connected to %s (%s)\n", client_hostname, client_ip_string);
        while(file_transfer_server(connfd));
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
