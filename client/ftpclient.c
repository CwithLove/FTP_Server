/*
 * echoclient.c - An echo client
 */
#include "csapp.h"
#include "ftp_protocol.h"

#define COMMAND_LEN 3

void file_transfer_client(int clientfd, char *filename) {
    request_t req;
    response_t res;
    char *buf;
    char filepath[MAXLINE];

    req.type = GET;
    strncpy(req.filename, filename, MAX_FILENAME);
    req.filename[strcspn(req.filename, "\n")] = '\0';  // Remove the newline character

    // Send the request to the server
    Rio_writen(clientfd, &req, sizeof(request_t));

    // Receive the response from the server
    // If the number of bytes read is different from the number of bytes expected
    if (Rio_readn(clientfd, &res, sizeof(response_t)) != sizeof(response_t)) {
        fprintf(stderr, "Error: Invalid response received!\n");
        return;
    }

    switch (res.code)
    {
    case SUCCESS:
        buf = malloc(res.file_size);
        // If the number of bytes read is different from the number of bytes expected
        if (Rio_readn(clientfd, buf, res.file_size) != res.file_size) {
            fprintf(stderr, "Error: reading file\n");
            free(buf);
            return;
        }
        
        // Write the file to the storage directory
        snprintf(filepath, STORAGE_PATH_LEN + MAX_FILENAME, "%s%s", STORAGE_PATH, req.filename);
        FILE *file = Fopen(filepath, "wb");
        Fwrite(buf, 1, res.file_size, file);
        Fclose(file);
        free(buf);
        fprintf(stdout, "Transfer successfully complete.\n");
        fprintf(stdout, "%ld bytes received\n", res.file_size);
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

int main(int argc, char **argv) {
    int clientfd;
    char *host;
    char buf[MAXLINE];
    char cmd[COMMAND_LEN];
    char filename[MAXLINE];

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }
    host = argv[1];

    // Open a connection to the server
    clientfd = Open_clientfd(host, PORT);
    
    // Read the user input
    fprintf(stdout, "ftp> ");
    Fgets(buf, MAXLINE, stdin);

    // Analyze the user input, and extract the command and the filename
    if (sscanf(buf, "%3s %s", cmd, filename) != 2) {
        fprintf(stderr, "Error: Invalid command format. Use: <get/put/ls> <filename>\n");
        Close(clientfd);
        exit(0);
    }

    if (strcmp(cmd, "get") != 0) {
        fprintf(stderr, "Error: Only the get command is supported\n");
        Close(clientfd);
        exit(0);
    }

    file_transfer_client(clientfd, filename);

    Close(clientfd);
    exit(0);
}