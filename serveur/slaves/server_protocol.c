#include "server_protocol.h"

int listenfd = -1;
int masterfd = -1;

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

    // Convert the request to host byte order
    req.offset = ntohl(req.offset);

    switch (req.type) {
        case GET:
            /* Constitution du chemin complet du fichier */
            snprintf(filepath, STORAGE_PATH_LEN + MAX_FILENAME, "%s%s", STORAGE_PATH, req.filename);
            
            /* Ouverture du fichier */
            if ((fd = open(filepath, O_RDONLY)) < 0) {
                perror(filepath);
                res.code = ERROR_FILE_NOT_FOUND;
                res.file_size = htonl(0);
                rio_writen(connfd, &res, sizeof(response_t));
                return 1;
            }
            
            /* Récupération de la taille du fichier */
            if (fstat(fd, &st) < 0) {
                perror("Error getting file size");
                res.code = ERROR_FILE_NOT_FOUND;
                res.file_size = htonl(0);
                rio_writen(connfd, &res, sizeof(response_t));
                close(fd);
                return 1;
            }

            // Si le fichier est déjà à jour, on envoie un code de mise à jour
            if (req.offset >= st.st_size) {
                res.code = UPDATED;
                res.file_size = htonl(st.st_size);
                rio_writen(connfd, &res, sizeof(response_t));
                close(fd);
                return 1;
            }

            // Positionner le descripteur de fichier à l'offset
            if (lseek(fd, req.offset, SEEK_SET) < 0) {
                perror("lseek");
                res.code = ERROR_INVALID_REQUEST;
                res.file_size = htonl(st.st_size);
                rio_writen(connfd, &res, sizeof(response_t));
                close(fd);
                return 1;
            }


            res.file_size = htonl((uint32_t)st.st_size);
            res.code = SUCCESS;
            /* Envoi de l’en-tête de réponse (code + taille du fichier) */
            rio_writen(connfd, &res, sizeof(response_t));

            /* Envoi du fichier par blocs */
            uint32_t remaining = res.file_size;
            char buffer[MESSAGE_SIZE];
            while (remaining > 0) {
                uint32_t bytes_to_read = (remaining < MESSAGE_SIZE) ? remaining : MESSAGE_SIZE;
                uint64_t n_read = read(fd, buffer, bytes_to_read);
                if (n_read < 0) {
                    perror("Error reading file");
                    break;
                }
                if (n_read == 0) {
                    break; // EOF   
                }
                rio_writen(connfd, buffer, n_read);
                remaining -= n_read;
            }
            close(fd);
            break;

        // case LS:
        //     pid_t pid;
        //     if ((pid = fork()) == 0) {
        //         dup2(connfd, STDOUT_FILENO);
        //         dup2(connfd, STDERR_FILENO);
        //         if (execlp("ls", "ls", "-l", NULL) < 0) {
        //             perror("execlp ls");
        //             exit(EXIT_FAILURE);
        //         }
        //     } else {
        //         waitpid(pid, NULL, 0);
        //     }
        //     break;

        // case RM:
        //     break;

        // case PUT:
        //     break;

        default:
            res.code = ERROR_INVALID_REQUEST;
            res.file_size = htonl(0);
            rio_writen(connfd, &res, sizeof(response_t));
            break;
    }     
    return 1;
}