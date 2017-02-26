#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#define MAX 200
/*
* Assignment 3 part 2
* 14CS30011 : Hiware Kaustubh Narendra
*/

// view shm -  ipcs -m
// remove all shared things created by you ->
// ipcs -m | grep 777 | awk '{print $2}' | xargs -n 1 ipcrm -m
// ipcs -m | grep 777 | awk '{print $2}' | xargs -n 1 ipcrm -s

/* Loads all records in shm from file as argv
* Check periodically for mods
* No new additions allowed whatsoever
* ONLY X CREATE ALL SHMs/ SEMs
* X and Y need not wait for each other, only let other know if operating
*/
#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)

typedef struct records_
{
    char first[21];
    char last[21];
    int roll;
    float cgpa;
} records;

key_t keyx = 0;
key_t keys = 1;
key_t keyd = 2;
key_t keyn = 3;
key_t keyu = 4;
key_t keyw = 10;
key_t keym = 11;
int numlines(char*);
void closeandexit();
// standardised function to check for errors, no repetition
void printerror(const char* printmsg, int argcount, int* argval)
{
    int i;
    for( i = 0; i < argcount; i++)
    {
        if(argval[i]==-1)
        {
            fprintf(stderr, "+--- %s : ",printmsg );
            perror("");
            exit(0);
        }
    }
}
// need to declare globally to handle Ctrl+C
int shmx, shmstud, shmdelta, shmnum, shmusers, wrt;


int main(int argc, char* argv[])
{
    signal(SIGINT, closeandexit);
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

    int numstudents = numlines(argv[1]), i = 0;

    shmx = shmget(keyx, sizeof(int), 0777|IPC_CREAT);
    shmstud = shmget(keys, numstudents*sizeof(int), 0777|IPC_CREAT);
    shmdelta = shmget(keyd, sizeof(int), 0777|IPC_CREAT);
    shmnum = shmget(keyn, sizeof(int), 0777|IPC_CREAT);
    shmusers = shmget(keyu, sizeof(int), 0777|IPC_CREAT);
    printerror("Error in shmget", 5, (int[]){ shmx, shmstud, shmdelta, shmnum, shmusers });

    wrt = semget(keyw, 1, 0777|IPC_CREAT);
    printerror("Error in semget", 1, (int[]){ wrt });

    int e1 = semctl(wrt, 0, SETVAL, 1);
    printerror("Error in semctl", 1, (int[]){ e1 });

    // Read file from data
    FILE *fp = fopen(argv[1], "r");
    if(fp==NULL)
    {
        printf("+--- File not found!\n");
        exit(0);
    }
    records* students = (records *) shmat(shmstud, 0, 0);
    while(!feof(fp))
    {
        fscanf(fp ,"%s" ,students[i].first);
		fscanf(fp ,"%s" ,students[i].last);
		fscanf(fp ,"%d" ,&students[i].roll);
		fscanf(fp ,"%f" ,&students[i].cgpa);
		i++;
    }
    fclose(fp);
    int *delta, *users, *num;
    delta = (int *) shmat(shmdelta, 0, 0);
    users = (int *) shmat(shmusers, 0, 0);
    num = (int *) shmat(shmnum, 0, 0);
    printerror("Error in shmat", 3, (int[]){ *delta, *users, *num });
    delta[0] = 0;
    num[0] = numstudents;
    users[0] = 0;

    struct sembuf pop, vop;
    // initialize sembufs
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1 ;   // value removed from semaphore
    vop.sem_op = 1 ;    // value added to semaphore

    int t = 0;
    while(1)
    {
        sleep(5);
        printf("+--- Changes from %4d to %4d :\t",t,t+5);
        t += 5;
        if(delta[0]==0)
        {
            printf("All good\n");
            continue; // no changes
        }

        users[0] += 1;
        if(users[0]==1)
        {
            P(wrt);
        }

        int s;
        fp = fopen(argv[1],"r+");
        for(i = 0; i < numstudents; i++)
        {
            s += fprintf(fp, "%s %s %d %f\n", students[i].first, students[i].last, students[i].roll, students[i].cgpa );
        }
        printf("Records have been updated\n");
        fclose(fp);
        delta[0] = 0;

        users[0] -= 1;
        if(users[0]==0)
        {
            V(wrt);
        }

    }

}


void closeandexit()
{
    int e1 = shmctl(shmx, IPC_RMID, 0);
    int e2 = shmctl(shmstud, IPC_RMID, 0);
    int e3 = shmctl(shmdelta, IPC_RMID, 0);
    int e4 = shmctl(shmnum, IPC_RMID, 0);
    int e5 = shmctl(shmusers, IPC_RMID, 0);
    int e6 = semctl(wrt, 0, IPC_RMID, 0);
    printerror("Error in shmctl(remove)", 5, (int[]){ e1, e2, e3, e4, e5 });
    printerror("Error in semctl(remove)", 1, (int[]){ e6 });
    exit(0);
}


int numlines(char* str)
{
    int lines = 0, ch = 0;
    FILE *fp = fopen(str,"r");
    if(fp==NULL)
    {
        printf("+--- File not found!\n");
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
