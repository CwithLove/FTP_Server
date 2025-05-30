#include "ftp_protocol.h"
#include "client_protocol.h"

int main(int argc, char **argv) {
    int clientfd;
    char *host;
    char buf[MAXLINE];
    char cmd[COMMAND_LEN];
    char filename[MAXLINE];
    typereq_t type;
    int status;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }
    host = argv[1];

    // Create a socket and connect to the server
    clientfd = Open_clientfd(host, PORT);
    
    fprintf(stdout, "----------------------------------------\n");
    fprintf(stdout, "Welcome to the FTP client!\n");
    fprintf(stdout, "----------------------------------------\n");
    fprintf(stdout, "Available commands:\n");
    fprintf(stdout, "1. get <filename>\n");
    fprintf(stdout, "2. bye\n");
    while (1) {
        // Prompt the user for a command
        fprintf(stdout, "ftp> ");
        if (fgets(buf, MAXLINE, stdin) == NULL) {
            fprintf(stderr, "Error: Failed to read input.\n");
            continue;
        }
    
        // Analyze the command
        status = analyze_command(buf, cmd, filename);
        if (status == -1) {
            fprintf(stderr, "Error: Invalid command format. Use: <get> <filename> or bye\n");
            continue;
        }
        
        if (status == 0) {
            if (strcmp(cmd, "bye") == 0) {
                fprintf(stdout, "Disconnecting...\n");
                Close(clientfd);
                exit(0);
            } else {
                fprintf(stderr, "Error: Unsupported single command. Use 'bye' or <get> <filename>.\n");
                continue;
            }
        }
    
        // Nếu có đủ 2 token, chỉ chấp nhận lệnh "get"
        if (strcmp(cmd, "get") == 0) {
            type = GET;
            file_transfer_client(clientfd, filename, type);
        } else {
            fprintf(stderr, "Error: Unsupported command.\n");
            continue;
        }
    }
    return 0;
}