#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
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

key_t keyx = 0; // agrees with key of X
key_t keys = 1;
key_t keyd = 2;
key_t keyn = 3;
key_t keyu = 4;
key_t keyw = 10;
key_t keym = 11;
#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)

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

void letxstart();
int shmx, shmstud, shmdelta, shmnum, shmusers, wrt, mutex;


int main(int argc, char* argv[])
{
    letxstart();
    printf("X started! Can support operations now\n");

    int opt = 0;
    char *in;

    shmstud = shmget(keys, numstudents*sizeof(int), 0);
    shmdelta = shmget(keyd, sizeof(int), 0);
    shmnum = shmget(keyn, sizeof(int), 0);
    shmusers = shmget(keyu, sizeof(int), 0);
    printerror("Error in shmget", 4, (int[]){ shmstud, shmdelta, shmnum, shmusers });

    int *delta, *users, *num;
    delta = (int *) shmat(shmdelta, 0, 0);
    users = (int *) shmat(shmusers, 0, 0);
    num = (int *) shmat(shmnum, 0, 0);
    printerror("Error in shmat", 3, (int[]){ *delta, *users, *num });

    struct sembuf pop, vop;
    // initialize sembufs
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1 ;   // value removed from semaphore
    vop.sem_op = 1 ;    // value added to semaphore

    while(1)
    {
        letxstart();
        printf("\n");
        printf("+--------------------------------------------------+\n");
        printf("+ Following operations are supported. What to do ? +\n");
        printf("+ 0 |   Query memory by roll numbers               +\n");
        printf("+ 1 |   Update CGPA of a student                   +\n");
        printf("+ 2 |   Exit                                       +\n");
        printf("+--------------------------------------------------+\n");

        scanf("%s", in);
        sscanf(in, "%d", &opt);

        // migrate to separate functions later
        if(opt==0)
        {
            printf("+--- You chose to query. Enter roll number : ");
            int r;
            scanf("%s", in);
            sscanf(in, "%d", &r);

            P(mutex);


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


void letxstart()
{
    int didxstart = shmget(keyx, sizeof(int), 0);
    while(didxstart==-1)
    {
        printf("Wait for X to start\n");
        sleep(5);
        didxstart = shmget(keyx, sizeof(int), 0);
    }
}
