/*
 * echoclient.c - An echo client
 */
#include "csapp.h"
#include "ftp_protocol.h"

#define COMMAND_LEN 15

void file_transfer_client(int clientfd, char *filename, typereq_t type) {
    request_t req;
    response_t res;
    char *buf;
    char filepath[MAXLINE];

    switch (type)
    {
    case GET:
        req.type = GET;
        break;
    }

    req.type = GET;
    strncpy(req.filename, filename, MAX_FILENAME);
    req.filename[strcspn(req.filename, "\n")] = '\0';  // Remove the newline character

    // Send the request to the server
    Rio_writen(clientfd, &req, sizeof(request_t));
    //start timer
    struct timeval start, end;
    gettimeofday(&start, NULL);

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
        //end timer
        gettimeofday(&end, NULL);
        double time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
        
        // Write the file to the storage directory
        snprintf(filepath, STORAGE_PATH_LEN + MAX_FILENAME, "%s%s", STORAGE_PATH, req.filename);
        FILE *file = Fopen(filepath, "wb");
        Fwrite(buf, 1, res.file_size, file);
        Fclose(file);
        free(buf);
        fprintf(stdout, "Transfer successfully complete.\n");
        //X bytes received in Y seconds (Z Kbytes/s).
        fprintf(stdout, "%ld bytes received in %.6f seconds (%.3f Kbytes/s).\n", res.file_size, time, res.file_size / time / 1024);
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
    typereq_t type;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }
    host = argv[1];

    // Open a connection to the server
    clientfd = Open_clientfd(host, PORT);
    while (1) {
        // Read the user input
        fprintf(stdout, "ftp> ");
        Fgets(buf, MAXLINE, stdin);

        // Analyze the user input, and extract the command and the filename
        if (sscanf(buf, "%3s %s", cmd, filename) != 2 ) {
            fprintf(stderr, "Error: Invalid command format. Use: <get/put/ls> <filename> | bye\n");
        }

        if (strcmp(cmd, "get") == 0) {
            type = GET;
        } else if (strcmp(cmd, "bye") == 0) {
            fprintf(stderr, "bybye !!\n");
            Close(clientfd);
            exit(0);
        // } else if (strcmp(cmd, "put") == 0) {
        //     type = PUT;
        // } else if (strcmp(cmd, "ls") == 0) {
        //     type = LS;
        } else {
            fprintf(stderr, "Error: Invalid command. Use: <get/put/ls> <filename> | bye\n");
            continue;
        }

        file_transfer_client(clientfd, filename, type);
    }
}