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
#define MAX 1000000
#define QUERY 0
#define UPDATE 1
/*
* Assignment 3 part 2
* 14CS30011 : Hiware Kaustubh Narendra
*/

// view shm -  ipcs -m
// remove all shared memory created by you ->
// ipcs -m | grep 777 | awk '{print $2}' | xargs -n 1 ipcrm -m
// ipcs -m | grep 777 | awk '{print $2}' | xargs -n 1 ipcrm -s

/* Loads all records in shm from file as argv
* Check periodically for mods
* No new additions allowed whatsoever
* ONLY X CREATE ALL SHMs/ SEMs
*/
#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)

typedef struct records_ {
    char first[20];
    char last[20];
    int roll;
    float cg;
} records;

key_t keyx = 30; // chosen randomly
key_t keyu = 31;
int numlines();

// standardised function to check for errors, no repeatition
void printerror(const char* printmsg, int argcount, int* argval)
{
    int i;
    for( i = 0; i < argcount; i++)
    {
        if(argval[i]==-1)
        {
            fprintf(stderr, "+--- %s : ",printmsg );
            perror("");
        }
    }
}


int main(int argc, char* argv[])
{
    char filename[100];
    if(argc > 1)
    {
        sscanf(argv[1],"%s",filename);
    }
    else
    {
        printf("+--- Reference file not provided! Exitting...\n");
        exit(0);
    }
    struct sembuf pop, vop;
    // initialize sembufs
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1 ;   // value removed from semaphore
    vop.sem_op = 1 ;    // value added to semaphore

    int numstudents = numlines();

    int xstart = shmget(keyx, sizeof(int), 0777|IPC_CREAT);
    int update = semget(keyu, 1, 0777|IPC_CREAT);
    printerror("Error in semget", 1, (int[]){ xstart });

    int e1;
    // int e2 = semctl(update, 0, SETVAL, 0);
    printerror("Error in shmctl", 1, (int[]){ e1 });

    // Read file from data
    FILE *fp = fopen("records.txt", "r");
    if(fp==NULL)
    {
        printf("+--- records.txt not found!\n");
        exit(0);
    }
    printf("#students : %d\n",numstudents );

    int i;
    printf("Enter i : ");
    scanf("%d",&i);

    // move this down
    e1 = shmctl(xstart, IPC_RMID, 0);
    int e2 = semctl(update, 0, IPC_RMID, 0);
    printerror("Error in semctl(remove)", 2, (int[]){ e1, e2 });

}

int numlines()
{
    int lines = 0, ch = 0;
    FILE *fp = fopen("records.txt","r");
    if(fp==NULL)
    {
        printf("+--- records.txt not found!\n");
        exit(0);
    }

    while ((ch = fgetc(fp)) != EOF)
    {
        if (ch == '\n')
        {
            lines++;
        }
    }
    fclose(fp);
    return lines;
}
