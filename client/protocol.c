#include "protocol.h"
#include "csapp.h"

void file_transfer_client(int clientfd, char *filename, typereq_t type) {
    request_t req;
    response_t res;
    char filepath[MAXLINE];
    int fd;
    ssize_t n;

    // Prepare the request
    req.type = type;
    strncpy(req.filename, filename, MAX_FILENAME);
    req.filename[strcspn(req.filename, "\n")] = '\0';  // Remove the newline character

    // Send the request to the server
    Rio_writen(clientfd, &req, sizeof(request_t));
    
    // Receive the response from the server
    while ((n = rio_readn(clientfd, &res, sizeof(response_t))) != sizeof(response_t)) {
        if (n < 0) {
            perror("Error reading response");
            return;
        } else if (n == 0) {
            fprintf(stderr, "Error: Server disconnected\n");
            return;
        }
        fprintf(stderr, "Error: Invalid response received!\n");
        return;
    }
    
    // Start timer
    struct timeval start, end;

    switch (res.code)
    {
        case SUCCESS:
        /* Construction du chemin de stockage du fichier */
        snprintf(filepath, STORAGE_PATH_LEN + MAX_FILENAME, "%s%s", STORAGE_PATH, req.filename);
        
        /* Ouverture (création) du fichier en écriture avec descripteur de fichier */
        if ((fd = open(filepath, O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0) {
            perror("open");
            return;
        }

        /* Réception du fichier par blocs */
        uint64_t total = res.file_size;
        uint64_t received = 0;
        char buffer[MESSAGE_SIZE];

        
        gettimeofday(&start, NULL);
        while (received < total) {
            uint64_t to_read = (total - received < MESSAGE_SIZE) ? (total - received) : MESSAGE_SIZE;
            uint64_t n = rio_readn(clientfd, buffer, to_read);
            if (n <= 0) {
                fprintf(stderr, "Error: reading file data\n");
                break;
            }
            uint64_t written = write(fd, buffer, n);
            if (written != n) {
                perror("write");
                break;
            }
            received += n;
        }
        close(fd);

        gettimeofday(&end, NULL);
        double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
        fprintf(stdout, "Transfer successfully complete.\n");
        fprintf(stdout, "%ld bytes received in %.6f seconds (%.3f Kbytes/s).\n",
                res.file_size, time_taken, res.file_size / time_taken / 1024);
        break;
    
    case ERROR_FILE_NOT_FOUND:
        fprintf(stderr, "Error: File not found\n");
        return;

    case ERROR_INVALID_REQUEST:
        fprintf(stderr, "Error: Invalid request\n");
        return;

    default:
        fprintf(stderr, "Error: Unknown error\n");
        return;
    }
}

int analyze_command(char *buf, char *cmd, char *filename) {
    int tokenCount = 0;
    int i = 0, j = 0;
    int len = strlen(buf);

    // Supprimer le caractère de fin de ligne éventuel
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    }

    // Ignorer les espaces initiaux
    while (buf[i] == ' ')
        i++;
    if (buf[i] == '\0')
        return -1;  // Chaîne vide après espaces

    // Extraction du premier token (commande)
    tokenCount++;
    j = 0;
    while (buf[i] != '\0' && buf[i] != ' ') {
        cmd[j++] = buf[i++];
    }
    cmd[j] = '\0';

    // Ignorer les espaces entre la commande et le filename
    while (buf[i] == ' ')
        i++;

    if (buf[i] == '\0') {
        // Seul le token commande est présent
        return 0;
    }

    // Extraction du second token (filename)
    tokenCount++;
    j = 0;
    while (buf[i] != '\0' && buf[i] != ' ') {
        filename[j++] = buf[i++];
    }
    filename[j] = '\0';

    // Ignorer les espaces éventuels en fin de chaîne
    while (buf[i] == ' ')
        i++;

    // S'il reste des caractères, cela signifie qu'il y a trop de tokens
    if (buf[i] != '\0')
        return -1;

    return tokenCount - 1;
}
