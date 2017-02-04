#include <stdio.h>
#include  <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include  <sys/types.h>
#include <sys/sem.h>
/*
* Assignment 3 part 1
* 14CS30011 : Hiware Kaustubh Narendra
*/
/* shared circular buffers
*  m - producer, n - consumer problem
*
* sum must ideally be m * 50 * 51 / 2
*/

int main(int argc,char* argv[])
{

    // create a shared buffer

    // child process will be the consumer

    // parent has the producer

    // producer must wait long enough for consumer to consume else no point

    // refer to signal / shm.c


}
