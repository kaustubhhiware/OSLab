#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include  <fcntl.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>
/*
* Assignment 4 part 2
* 14CS30011 : Hiware Kaustubh Narendra
*/
/*
* A is the producer - produce requests
* queue of size 10
* A dies if B dies
*/
//
// run with gcc A.c -o A

#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)

void minus1_error(const char* , int , int*);
void didBstop();

int shmq, shmin, full, empty, mutex;
key_t keyq = 1; // Agrees with the keys of B
key_t keyi = 2;
key_t keyo = 3;
key_t keyf = 10;
key_t keye = 11;
key_t keym = 12;

int main(int argc, char* argv[])
{
    srand(time(NULL));
    didBstop(); // exit if B not active

    // initialize sembufs
    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1 ;   // value removed from semaphore
    vop.sem_op = 1 ;    // value added to semaphore

    // shmq is our queue
    shmq = shmget(keyq, 10*sizeof(int), 0777|IPC_CREAT);
    shmin = shmget(keyi, sizeof(int), 0777|IPC_CREAT);
    minus1_error("Error in shmget", 2, (int[]){ shmq, shmin });

    // create semaphores as indicatives for full, empty and operative buffer
    full = semget(keyf, 1, 0777|IPC_CREAT);
    empty = semget(keye, 1, 0777|IPC_CREAT);
    mutex = semget(keym, 1, 0777|IPC_CREAT);
    minus1_error("Error in semget", 3, (int[]){ full, empty, mutex });

    int c1 = semctl(full, 0, SETVAL, 0);
    int c2 = semctl(empty, 0, SETVAL, 10);
    int c3 = semctl(mutex, 0, SETVAL, 1);
    minus1_error("Error in semctl(SETVAL)", 3, (int[]){ c1, c2, c3 });

    int *q, *in;
    q = (int *) shmat(shmq, 0, 0);
    in = (int *) shmat(shmin, 0, 0);
    minus1_error("Error in shmat", 2, (int[]){ *q, *in });

    while(1)
    {
        didBstop();
        sleep(rand()%3); // sleep for 0 - 2 s
        int request = rand()%11 - 5; // in -5 to +5

        P(empty);
        P(mutex);

        // add number to queue, this is a request
        q[*in] = request;
        printf("+--- Request : %d\n", q[*in]);
        *in = (*in + 1) % 10;

        V(mutex);
        V(full);
    }

    // get rid of sems and shms
    c1 = shmdt(q);
    c2 = shmdt(in);
    minus1_error("Error in shmdt", 2, (int[]){ c1, c2 });

    c1 = semctl(full, 0, IPC_RMID, 0);
    c2 = semctl(empty, 0, IPC_RMID, 0);
    c3 = semctl(mutex, 0, IPC_RMID, 0);
    minus1_error("Error in semctl", 3, (int[]){ c1, c2, c3 });

    c1 = shmctl(shmq, IPC_RMID, 0);
    c2 = shmctl(shmin, IPC_RMID, 0);
    minus1_error("Error in shmctl", 2, (int[]){ c1, c2 });

}

// asking a question crucial to its own survival
void didBstop()
{
    int isBthere = shmget(keyq, sizeof(int), 0);
    if(isBthere==-1)
    {
        printf("No B active! Exitting...\n");
        exit(0);
    }
}

// standardised function to check for errors, no repetition for shm and sem
void minus1_error(const char* printmsg, int argcount, int* argval)
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
