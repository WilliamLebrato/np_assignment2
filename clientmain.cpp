#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* You will to add includes here */
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h> // For timeout handling
#include <errno.h>
#define BUFFER_SIZE 1024
#define DEBUG


#include "protocol.h"

// ./client 13.53.76.30:5001
// Function to perform the arithmetic operation
int performOperation(struct calcProtocol *assignment) {
    uint32_t arith = ntohl(assignment->arith);
    int32_t inVal1 = ntohl(assignment->inValue1);
    int32_t inVal2 = ntohl(assignment->inValue2);
    double flVal1 = assignment->flValue1;
    double flVal2 = assignment->flValue2;

    switch(arith) {
        // Integer Operations
        case 1: // Addition
            assignment->inResult = htonl(inVal1 + inVal2);
            return 0;
        case 2: // Subtraction
            assignment->inResult = htonl(inVal1 - inVal2);
            return 0;
        case 3: // Multiplication
            assignment->inResult = htonl(inVal1 * inVal2);
            return 0;
        case 4: // Division
            if (inVal2 == 0) {
                fprintf(stderr, "Error: Division by zero.\n");
                return -1;
            }
            assignment->inResult = htonl(inVal1 / inVal2);
            return 0;

        // Floating-Point Operations
        case 5: // Floating-Point Addition
            assignment->flResult = flVal1 + flVal2;
            return 0;
        case 6: // Floating-Point Subtraction
            assignment->flResult = flVal1 - flVal2;
            return 0;
        case 7: // Floating-Point Multiplication
            assignment->flResult = flVal1 * flVal2;
            return 0;
        case 8: // Floating-Point Division
            if (flVal2 == 0.0) {
                fprintf(stderr, "Error: Floating-Point Division by zero.\n");
                return -1;
            }
            assignment->flResult = flVal1 / flVal2;
            return 0;
        default:
            fprintf(stderr, "Unknown arithmetic operation code: %u\n", arith);
            return -1;
    }
}

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

      // Construct the initial calcMessage
    struct calcMessage message;
    message.type = htons(22);
    message.message = htonl(0);
    message.protocol = htons(17);
    message.major_version = htons(1);
    message.minor_version = htons(0);

    // Send the message to the server
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, Desthost, &server_addr.sin_addr);

    // Set receive timeout
    struct timeval timeout;
    timeout.tv_sec = 2; // 2 seconds timeout
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  
    ssize_t sent_bytes;
    int attempts = 0;
    socklen_t server_len = sizeof(server_addr);
    struct calcProtocol response;

    while (attempts < 3) {
        // Send the message to the server
        sent_bytes = sendto(sockfd, &message, sizeof(message), 0, (struct sockaddr *)&server_addr, server_len);
        if (sent_bytes == -1) {
            perror("sendto");
            close(sockfd);
            return -1;
        }
        printf("Message sent, attempt %d.\n", attempts + 1);

        // Wait for the response
        ssize_t recv_bytes = recvfrom(sockfd, &response, sizeof(response), 0, (struct sockaddr *)&server_addr, &server_len);

        if (recv_bytes == -1) {
            // Check if the error is due to timeout
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("No response from server, retrying...\n");
                attempts++;
                continue; // Retry sending the message
            } else {
                perror("recvfrom");
                close(sockfd);
                return -1;
            }
        }
        break;
    }

    if (attempts == 3) {
        printf("Server did not respond after 3 attempts. Terminating.\n");
        close(sockfd);
        return 0;
    }

    // Assuming `response` is a buffer where the response is stored
    struct calcMessage *responseMessage = (struct calcMessage *)&response;
    if (ntohs(responseMessage->type) == 2) {
        printf("NOT OK\n");
        close(sockfd);
        return -1;
    }

    printf("OK\n");
    struct calcProtocol *protocolResponse = (struct calcProtocol *)&response;
    /*
    printf("Received calculation assignment:\n");
    printf("Type: %d\n", ntohs(protocolResponse->type));
    printf("Major Version: %d\n", ntohs(protocolResponse->major_version));
    printf("Minor Version: %d\n", ntohs(protocolResponse->minor_version));
    printf("ID: %d\n", ntohl(protocolResponse->id));
    printf("Arithmetic Operation: %d\n", ntohl(protocolResponse->arith));
    printf("Integer Value 1: %d\n", ntohl(protocolResponse->inValue1));
    printf("Integer Value 2: %d\n", ntohl(protocolResponse->inValue2));
    printf("Floating Point Value 1: %f\n", protocolResponse->flValue1);
    printf("Floating Point Value 2: %f\n", protocolResponse->flValue2);
    */

    // Perform the arithmetic operation
    if (performOperation(protocolResponse) == -1) {
        close(sockfd);
        return -1;
    }

    attempts = 0;
    while (attempts < 3) {
        // Send the response back to the server
        ssize_t sent_bytes = sendto(sockfd, protocolResponse, sizeof(*protocolResponse), 0, (struct sockaddr *)&server_addr, server_len);
        if (sent_bytes == -1) {
            perror("sendto");
            close(sockfd);
            return -1;
        }
        printf("Response sent, attempt %d.\n", attempts + 1);

        // Wait for the response
        ssize_t recv_bytes = recvfrom(sockfd, &response, sizeof(response), 0, (struct sockaddr *)&server_addr, &server_len);

        if (recv_bytes == -1) {
            // Check if the error is due to timeout
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("No response from server, retrying...\n");
                attempts++;
                continue; // Retry sending the message
            } else {
                perror("recvfrom");
                close(sockfd);
                return -1;
            }
        }
        break;
    }

    close(sockfd);
    
    return 0;
}