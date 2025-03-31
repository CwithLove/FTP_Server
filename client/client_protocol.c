#include "client_protocol.h"

uint64_t get_file_size(char *filename) {
    struct stat st;
    if (!stat(filename, &st)) {
        return st.st_size;
    } else {
        return 0; 
    }
}

void file_transfer_client(int clientfd, char *filename, typereq_t type) {
    request_t req;
    response_t res;
    char filepath[MAXLINE];
    int fd;
    uint64_t n;
    uint64_t bytes_installed;

    /* Construction du chemin de stockage du fichier */
    snprintf(filepath, STORAGE_PATH_LEN + MAX_FILENAME, "%s%s", STORAGE_PATH, filename);

    /* Vérification de l'existence du fichier */
    bytes_installed = get_file_size(filepath);

    // Prepare the request
    req.type = type;
    req.offset = bytes_installed;
    strncpy(req.filename, filename, MAX_FILENAME);
    req.filename[strcspn(req.filename, "\n")] = '\0';  // Remove the newline character

    fprintf(stdout, "Sending request to server...\n");
    // Send the request to the server
    Rio_writen(clientfd, &req, sizeof(request_t));

    // Receive the response from the server
    if ((n = rio_readn(clientfd, &res, sizeof(response_t))) != sizeof(response_t)) {
        if (n < 0) {
            perror("Error reading response");
            return;
        } else if (n == 0) {
            fprintf(stderr, "Error: Server disconnected\n");
            return;
        }
        fprintf(stderr, "Error: Invxalid response received!\n");
        return;
    }
    
    // Convert the response to host byte order
    res.file_size = ntohl(res.file_size);

    // Start timer
    struct timeval start, end;

    switch (res.code)
    {
        case SUCCESS:

        if (bytes_installed == 0) {
            if ((fd = open(filepath, O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0) {
                perror("open");
                return;
            }
        } else {
            if ((fd = open(filepath, O_WRONLY | O_APPEND)) < 0) {
                perror("open");
                return;
            }

            if (lseek(fd, bytes_installed, SEEK_SET) < 0) {
                perror("lseek");
                close(fd);
                return;
            }
        }

        /* Réception du fichier par blocs */
        uint32_t missing_part = res.file_size - bytes_installed;
        uint32_t received = 0;
        char buffer[MESSAGE_SIZE];

        
        gettimeofday(&start, NULL);
        while (received < missing_part) {
            uint32_t to_read = (missing_part - received < MESSAGE_SIZE) ? (missing_part - received) : MESSAGE_SIZE;
            uint64_t n_read = rio_readn(clientfd, buffer, to_read);
            if (n <= 0) {
                fprintf(stderr, "Error: reading file data\n");
                break;
            }
            uint64_t written = write(fd, buffer, n_read);
            if (written != n_read) {
                perror("write");
                break;
            }
            received += n_read;
        }
        close(fd);

        gettimeofday(&end, NULL);
        double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
        fprintf(stdout, "Transfer successfully complete.\n");
        fprintf(stdout, "%d bytes received in %.6f seconds (%.3f Kbytes/s).\n",
                res.file_size, time_taken, res.file_size / time_taken / 1024.0);
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

int client_connect_to_slave(int masterfd) {
    slave_info_t slave_info;

    Rio_readn(masterfd, &slave_info, sizeof(slave_info_t));
    slave_info.slave_available = ntohl(slave_info.slave_available);
    slave_info.port = ntohl(slave_info.port);
    if (slave_info.slave_available == 0) {
        fprintf(stderr, "No slave available\n");
        return -1;
    }
    fprintf(stdout, "----------------------------------------\n");
    int fd = open_clientfd(slave_info.hostname, slave_info.port);
    if (fd < 0) {
        fprintf(stderr, "Error: Failed to connect to slave %s:%d\n", slave_info.hostname, slave_info.port);
        exit(0);
    }
    return fd;
}