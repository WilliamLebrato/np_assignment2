#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

/* You will to add includes here */
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <math.h> // Required for floating-point comparison
#include <time.h>
#include <errno.h>

// Included to get the support library
#include <calcLib.h>

#include "protocol.h"

#define DEBUG
#define BUFFER_SIZE 128
#define MAXCLIENTS 128



using namespace std;

struct assignment {
    int int1;             // First integer value
    int int2;             // Second integer value
    float float1;         // First floating-point value
    float float2;         // Second floating-point value
    char operation[4];    // Operation (e.g., "add", "fdiv")
    int int_result;       // Result for integer operations
    float float_result;   // Result for floating-point operations
};

typedef struct {
    struct sockaddr_storage addr; // Client's address (can hold both IPv4 and IPv6)
    socklen_t addr_len;           // Length of the address
    struct assignment task;       // Current assignment (if any)
    time_t timestamp;             // Time when the assignment was sent
    int is_active;                // Flag to indicate if the client is active
} client_t;

client_t clients[MAXCLIENTS]; // Array to hold client information
// Function to generate a random assignment
void generate_assignment(struct assignment *task) {
    // List of possible operations
    const char *operations[] = {"add", "sub", "mul", "div", "fadd", "fsub", "fmul", "fdiv"};
    int operation_count = 8;

    // Seed the random number generator
    srand(time(NULL));

    // Select a random operation
    int index = rand() % operation_count;
    strcpy(task->operation, operations[index]);

    // Generate random values based on the operation type
    if (task->operation[0] == 'f') {
        // Floating-point operation
        task->float1 = (float)(rand() % 1000) / 100.0;  // Random float between 0 and 10
        task->float2 = (float)(rand() % 1000) / 100.0;  // Random float between 0 and 10

        // Calculate the expected result
        if (strcmp(task->operation, "fadd") == 0) {
            task->float_result = task->float1 + task->float2;
        } else if (strcmp(task->operation, "fsub") == 0) {
            task->float_result = task->float1 - task->float2;
        } else if (strcmp(task->operation, "fmul") == 0) {
            task->float_result = task->float1 * task->float2;
        } else if (strcmp(task->operation, "fdiv") == 0) {
            task->float_result = task->float1 / task->float2;
        }
    } else {
        // Integer operation
        task->int1 = rand() % 100;  // Random integer between 0 and 99
        task->int2 = rand() % 100;  // Random integer between 0 and 99

        // Calculate the expected result
        if (strcmp(task->operation, "add") == 0) {
            task->int_result = task->int1 + task->int2;
        } else if (strcmp(task->operation, "sub") == 0) {
            task->int_result = task->int1 - task->int2;
        } else if (strcmp(task->operation, "mul") == 0) {
            task->int_result = task->int1 * task->int2;
        } else if (strcmp(task->operation, "div") == 0) {
            task->int_result = task->int1 / task->int2; // Integer division
        }
    }
}

// Function to check the client's response against the expected result
const char* check_task(struct assignment *task, char *client_response) {
    // Remove the newline character from the client's response
    client_response[strcspn(client_response, "\n")] = '\0';

    if (task->operation[0] == 'f') {
        // Floating-point comparison
        float client_result = atof(client_response); // Convert client response to float
        float difference = fabs(task->float_result - client_result); // Calculate the difference

        // If the difference is within the acceptable range, return "OK"
        if (difference < 0.0001) {
            return "OK\n";
        } else {
            return "ERROR\n";
        }
    } else {
        // Integer comparison
        int client_result = atoi(client_response); // Convert client response to int

        // Compare integer results directly
        if (task->int_result == client_result) {
            return "OK\n";
        } else {
            return "ERROR\n";
        }
    }
}


int find_client(struct sockaddr_storage *addr, socklen_t addr_len) {
    for (int i = 0; i < MAXCLIENTS; i++) {
        if (clients[i].is_active &&
            clients[i].addr_len == addr_len &&
            memcmp(&clients[i].addr, addr, addr_len) == 0) {
            return i; // Return the index of the client
        }
    }
    return -1; // Client not found
}

int add_client(struct sockaddr_storage *addr, socklen_t addr_len) {
    for (int i = 0; i < MAXCLIENTS; i++) {
        if (!clients[i].is_active) {
            clients[i].is_active = 1;
            clients[i].addr = *addr;
            clients[i].addr_len = addr_len;
            generate_assignment(&clients[i].task);
            clients[i].timestamp = time(NULL);
            return i; // Return the index where the client was added
        }
    }
    return -1; // No space for new client
}

void remove_client(int index) {
    if (index >= 0 && index < MAXCLIENTS) {
        clients[index].is_active = 0;
    }
}
/* Needs to be global, to be rechable by callback and main */
int loopCount=0;
int terminate=0;

void handle_message(char *buffer, ssize_t bytes_received, struct sockaddr_storage *client_addr, socklen_t client_addr_len) {
    // Step 1: Interpret the received buffer as a calcMessage
    struct calcMessage *msg = (struct calcMessage *)buffer;

    // Step 2: Check if the message is a protocol message
    if (ntohs(msg->type) == 22 && ntohl(msg->message) == 0) { // Protocol message type and initial message
        // This is a protocol message, we need to send an assignment
        printf("Received protocol message from client. Preparing assignment...\n");

        // TODO: Prepare and send an assignment to the client

    } 
    // Step 3: Check if the message is an answer message
    else if (ntohs(msg->type) == 2) { // Type 2 assumed to be answer message, adjust as per your protocol
        // This is an answer message
        printf("Received answer message from client. Validating answer...\n");
  
        // TODO: Validate the answer and respond accordingly

    } 
    // Step 4: Handle unknown message types
    else {
        printf("Received unknown message type from client.\n");
        // Optionally, send an error response back to the client
    }
}

/* Call back function, will be called when the SIGALRM is raised when the timer expires. */
void checkJobbList(int signum){
  // As anybody can call the handler, its good coding to check the signal number that called it.

  printf("Let me be, I want to sleep, loopCount = %d.\n", loopCount);

  if(loopCount>20){
    printf("I had enough.\n");
    terminate=1;
  }
  
  return;
}

int main(int argc, char *argv[]){
  
  /* Do more magic */
  char delim[]=":";
  char *Desthost=strtok(argv[1],delim);
  char *Destport=strtok(NULL,delim);

  /* 
     Prepare to setup a reoccurring event every 10s. If it_interval, or it_value is omitted, it will be a single alarm 10s after it has been set. 
  */
  struct itimerval alarmTime;
  alarmTime.it_interval.tv_sec=10;
  alarmTime.it_interval.tv_usec=10;
  alarmTime.it_value.tv_sec=10;
  alarmTime.it_value.tv_usec=10;

  /* Regiter a callback function, associated with the SIGALRM signal, which will be raised when the alarm goes of */
  signal(SIGALRM, checkJobbList);
  setitimer(ITIMER_REAL,&alarmTime,NULL); // Start/register the alarm. 

#ifdef DEBUG
  printf("DEBUGGER LINE ");
#endif
  
    struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;        // Allow IPv4 or IPv6
  hints.ai_socktype = SOCK_DGRAM;     // UDP datagram sockets, instead of SOCK_STREAM
  hints.ai_flags = AI_PASSIVE;        // All available interfaces

  // Get address info for the provided IP and port
  if (getaddrinfo(Desthost, Destport, &hints, &res) != 0) {
      perror("getaddrinfo() failed");
      return -1;
  }

  // Use the address info returned by getaddrinfo to create the socket
  int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sockfd < 0) {
      perror("socket() failed");
      freeaddrinfo(res); // Free the linked list
      return -1;
  }

  // Bind the socket to the provided IP and port
  if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
      perror("bind() failed");
      close(sockfd);
      return -1;
  }
  freeaddrinfo(res); // Free the linked list

  char buffer[BUFFER_SIZE];

  while(terminate==0){
    printf("This is the main loop, %d time.\n",loopCount);
    memset(buffer, 0, BUFFER_SIZE);
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len);
    if (bytes_received < 0) {
        if (errno == EINTR) {
            // Interrupted by signal, continue the loop
            continue;
        } else {
            // Print error but do not close the socket
            perror("recvfrom() failed");
            continue; // Keep the server running
        }
    }
    // Handle the received message
    handle_message(buffer, bytes_received, &client_addr, client_addr_len);    

    loopCount++;
    
  }
  close(sockfd);
  printf("done.\n");
  return(0);
  }
