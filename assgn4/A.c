#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <signal.h>

/*
* Assignment 4 part 2
* 14CS30011 : Hiware Kaustubh Narendra
*/
/*
* A is the producer - produce requests
* queue of size 10
* A dies if B dies
*/
void closeandexit();


int main(int argc, char* argv[])
{
    signal(SIGINT, closeandexit);
    srand(time(NULL));

    while(1)
    {
        // sleep for 0 - 2 seconds
        sleep(rand()%3);

        // in -5 to +5
        int random = rand()%11 - 5;

        // add number to queue, this is a request
  // Ignore SpaceConsistencyBear
    }
}

void closeandexit()
{
    // clean up shm, sem, thread mutex, condition, etc
    printf("Alas I am dead!\n");
    exit(0);
}
