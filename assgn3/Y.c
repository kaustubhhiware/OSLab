#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/ipc.h>
#include <errno.h>
#define QUERY 0
#define UPDATE 1
/*
* Assignment 3 part 2
* 14CS30011 : Hiware Kaustubh Narendra
*/

/* Querying process to do this :
* * search by roll and display
* * update CG
*
* Multiple copies supported
* Y must wait for X to start, only then
*/

key_t keyx = 30; // agrees with key of X
key_t keyu = 31;
#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)

int main(int argc, char* argv[])
{

    int didxstart = shmget(keyx, sizeof(int), 0);
    // Wait for X to start
    while(didxstart==-1)
    {
        printf("Wait for X to start\n");
        sleep(5);
        didxstart = shmget(keyx, sizeof(int), 0);
    }
    printf("X started! Can support operations now\n");

    int opt = 0;
    char *in;
    while(1)
    {
        printf("\n");
        printf("+--------------------------------------------------+\n");
        printf("+ Following operations are supported. What to do ? +\n");
        printf("+ 0 |   Query memory by roll numbers               +\n");
        printf("+ 1 |   Update CGPA of a student                   +\n");
        printf("+ 2 |   Exit                                       +\n");
        printf("+--------------------------------------------------+\n");

        scanf("%s", in);
        sscanf(in, "%d", &opt);
        if(opt==0)
        {
            printf("+--- You chose to query. Enter roll number : ");
        }
        else if(opt==1)
        {
            printf("+--- You chose to update CG. Enter roll number : ");
        }
        else
        {
            printf("Exitting...\n");
            exit(0);
        }
    }

}
