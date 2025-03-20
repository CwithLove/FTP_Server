// ftp_protocol.h

#ifndef FTP_PROTOCOL_H
#define FTP_PROTOCOL_H

#define MAX_FILENAME 256

// This structure is used to define the request sent by the client
typedef enum {
    GET    // Only GET is processed in this phase
    // PUT, can be added later as needed
    // LS  TO DO LATER
} typereq_t;


// The request structure for the FTP protocol
typedef struct {
    int type;          // The request type, should be one of typereq_t values
    char filename[MAX_FILENAME]; // The file name requested by the client
} request_t;

#endif // FTP_PROTOCOL_H