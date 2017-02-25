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
#define MAX 1000000
/*
* Assignment 3 part 2
* 14CS30011 : Hiware Kaustubh Narendra
*/

// view shm -  ipcs -m
// remove all shared memory created by you ->
// ipcs -m | grep 777 | awk '{print $2}' | xargs -n 1 ipcrm -m

/* Loads all records in shm from file as argv
* Check periodically for mods
* No new additions allowed whatsoever
* ONLY I, LRRR of OMICRON PERCEI 8; CREATE ALL SHMs/ SEMs
*/

typedef struct records_ {
    char first[20];
    char last[20];
    int roll;
    float cg;
} records;

#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)

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
    int numstudents = numlines();
    int shmfirst = shmget(IPC_PRIVATE, numstudents*sizeof(char), 0777|IPC_CREAT);
    int shmlast = shmget(IPC_PRIVATE, numstudents*sizeof(char), 0777|IPC_CREAT);
    int shmroll = shmget(IPC_PRIVATE, numstudents*sizeof(int), 0777|IPC_CREAT);
    int shmcg = shmget(IPC_PRIVATE, numstudents*sizeof(float), 0777|IPC_CREAT);
/*
    // Read file from data
    FILE *fp = fopen("records.txt", "r");
    if(fp==NULL)
    {
        printf("+--- records.txt not found!\n");
        exit(0);
    }
    char *data, *bits;
    int i = 0;
    while(fgets(data, MAX, fp)!=NULL)
    printf("%s",data );
*/    /*while((bits = strsep(&data, " \t\n")) != NULL && i < 4*numstudents)
    {
        printf("hi\n");
    }*/




    //fclose(fp);
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
