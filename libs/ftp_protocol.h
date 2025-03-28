#ifndef FTP_PROTOCOL_H
#define FTP_PROTOCOL_H

#include <time.h>
#include "csapp.h"
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_FILENAME 256

#define PORT 9919 // A CHANGER
#define MESSAGE_SIZE 1024
#define STORAGE_PATH "./storage/"
#define STORAGE_PATH_LEN 10

// This structure is used to define the request sent by the client
typedef enum {
    GET,    // Only GET is processed in this phase
    // PUT, // TO DO LATER
    // LS  // TO DO LATER
} typereq_t;


// The request structure for the FTP protocol
typedef struct {
    typereq_t type;              // The request type, should be one of typereq_t values
    char filename[MAX_FILENAME]; // The file name requested by the client
} request_t;


// The response code for the FTP protocol 
typedef enum {
    SUCCESS,
    ERROR_FILE_NOT_FOUND,
    ERROR_INVALID_REQUEST
} response_code_t;


// The response structure for the FTP protocol
typedef struct {
    response_code_t code;       // The response code, should be one of response_code_t values
    uint64_t file_size;           // The size of the file in bytes
} response_t;

#endif // FTP_PROTOCOL_H