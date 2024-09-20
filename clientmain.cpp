#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* You will to add includes here */
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h> // For timeout handling

#define BUFFER_SIZE 1024
#define DEBUG


#include "protocol.h"

// client 127.0.0.1:5000

typedef struct calcMessage {
    uint16_t type;
    uint32_t message;
    uint16_t protocol;
    uint16_t major_version;
    uint16_t minor_version;
} calcMessage;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <IP:PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Split the input into host and port
    char *Desthost = strtok(argv[1], ":");
    char *Destport = strtok(NULL, ":");

    if (Desthost == NULL || Destport == NULL) {
        fprintf(stderr, "Invalid IP:PORT format.\n");
        exit(EXIT_FAILURE);
    }

    int port = atoi(Destport);

    #ifdef DEBUG 
      printf("Host %s, and port %d.\n", Desthost, port);
    #endif


    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;     // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM;  // UDP datagram sockets

    if ((rv = getaddrinfo(Desthost, Destport, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(EXIT_FAILURE);
    }
  
      // Loop through all the results and create a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            perror("socket");
            continue;
        }

        // For UDP, you typically don't need to bind the socket unless you have specific requirements
        // printf("Socket created successfully.\n");
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "Failed to create socket.\n");
        freeaddrinfo(servinfo);
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(servinfo); // No longer needed after socket creation

    return 0;
}