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

/* Needs to be global, to be rechable by callback and main */
int loopCount=0;
int terminate=0;

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
  
  
  while(terminate==0){
    printf("This is the main loop, %d time.\n",loopCount);
    sleep(1);
    loopCount++;
  }

  printf("done.\n");
  return(0);


  
}
