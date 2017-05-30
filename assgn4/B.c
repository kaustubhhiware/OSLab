#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>
#include <signal.h>
#include <assert.h>
/*
* Assignment 4 part 2
* written by @kaustubhhiware as a part of OS Lab
*/
/*
* B is the consumer - recieves requests
* per request, create thread, check for another threads, call book_ticket()
*/
// remove all shared things created by you ->
// ipcs -m | grep 777 | awk '{print $2}' | xargs -n 1 ipcrm -m
// ipcs -m | grep 777 | awk '{print $2}' | xargs -n 1 ipcrm -s
//
// run with gcc -pthread B.c -o B

typedef struct thread_data_ {
    int request;
}thread_data;

#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)

// keep track of # requests, thread started but not finished
int Ticket = 100, numthreads, status = 0;
pthread_t thread_id[10];
pthread_mutex_t mutex1;
pthread_cond_t cond1;

void closeandexit();
// 2 printerror functions because sem+shm have different errors from pthreads
void non0_error(const char* , int , int*);
void minus1_error(const char* , int , int*);

// function each request will call
void *book_ticket(void *param)
{
    thread_data *t_param = (thread_data *) param;
    int x = (*t_param).request;
    status = 0;
    int c1 = pthread_mutex_lock(&mutex1);
    non0_error("Error in mutex lock (book_ticket)", 1, (int[]){ c1 });

    numthreads++;
    while(numthreads > 10)
    {
        printf("+-- Queue too long! Wait for a while ~\n");
        c1 = pthread_cond_wait(&cond1, &mutex1);
        non0_error("Error in cond wait (book_ticket)", 1, (int[]){ c1 });
    }

    if((Ticket-x) >= 0)
    {
        status = 1;
        Ticket = Ticket - x;
        if(Ticket > 100) Ticket = 100; // min (Ticket -x, 100)
    }

    // unlock so that others don't mind me sleeping
    c1 = pthread_mutex_unlock(&mutex1);
    non0_error("Error in mutex unlock (book_ticket)", 1, (int[]){ c1 });

    sleep(rand()%3); // sleep for 0 - 2 s
    c1 = pthread_mutex_lock(&mutex1); // I'm awake, I need to lock
    non0_error("Error in mutex lock (book_ticket)", 1, (int[]){ c1 });

    if(status==1)
    {
        printf("+--- Ticket : %d,\n", Ticket);
    }
    else
    {
        printf("+--- Request could not be fulfilled :/\n" );
    }

    numthreads--;
    if(numthreads < 6)
    {
        // wake up only one process at a time, not multiple
        c1 = pthread_cond_signal(&cond1);
        non0_error("Error in cond signal (book_ticket)", 1, (int[]){ c1 });
    }

    c1 = pthread_mutex_unlock(&mutex1);
    non0_error("Error in mutex unlock (book_ticket)", 1, (int[]){ c1 });
    pthread_exit(0);
    // return status; // not needed, since global
    // return type of this function cannot be int, hence used global var
}

// need to declare globally to handle Ctrl+C
int shmq, shmout, full, empty, mutex;
key_t keyq = 1;
key_t keyi = 2;
key_t keyo = 3;
key_t keyf = 10;
key_t keye = 11;
key_t keym = 12;

int main(int argc, char* argv[])
{
    srand(time(NULL));
    signal(SIGINT, closeandexit);
    thread_data param[10];

    // initialize mutex and condition
    int c1 = pthread_mutex_init(&mutex1, NULL);
    int c2 = pthread_cond_init(&cond1, NULL);
    non0_error("Error in thread var init", 2, (int[]){ c1, c2 });

    // initialize sembufs
    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1 ;   // value removed from semaphore
    vop.sem_op = 1 ;    // value added to semaphore

    // shmq is our queue
    shmq = shmget(keyq, 10*sizeof(int), 0777|IPC_CREAT);
    shmout = shmget(keyo, sizeof(int), 0777|IPC_CREAT);
    minus1_error("Error in shmget", 2, (int[]){ shmq, shmout });

    // create semaphores as indicatives for full, empty and operative buffer
    full = semget(keyf, 1, 0777|IPC_CREAT);
    empty = semget(keye, 1, 0777|IPC_CREAT);
    mutex = semget(keym, 1, 0777|IPC_CREAT);
    minus1_error("Error in semget", 3, (int[]){ full, empty, mutex });

    c1 = semctl(full, 0, SETVAL, 0);
    c2 = semctl(empty, 0, SETVAL, 10);
    int c3 = semctl(mutex, 0, SETVAL, 1);
    minus1_error("Error in semctl(SETVAL)", 3, (int[]){ c1, c2, c3 });


    int *q, *out, i = 0;
    q = (int *) shmat(shmq, 0, 0);
    out = (int *) shmat(shmout, 0, 0);
    minus1_error("Error in shmat", 2, (int[]){ *q, *out });

    int request;
    while(1) // until a Ctrl+C is pressed
    {
        c1 = pthread_mutex_lock(&mutex1);
        non0_error("Error in thread var init", 1, (int[]){ c1, c2 });

        while(numthreads > 10)
        {
            c1 = pthread_cond_wait(&cond1, &mutex1);
            non0_error("Error in cond wait (book_ticket)", 1, (int[]){ c1 });
        }

        i = numthreads; // this ensures that i is unique for each request
        c1 = pthread_mutex_unlock(&mutex1);
        non0_error("Error in mutex unlock (book_ticket)", 1, (int[]){ c1 });

        P(full);
        P(mutex);

        request = q[*out];
        *out = (*out + 1) % 10;

        param[i].request = request;
        pthread_create(&thread_id[i], NULL, book_ticket, (void *) &param[i]);
        pthread_join(thread_id[i], NULL);
        printf(">>\t\t\t\t\tThread exitted with status %d\n",status );

        V(mutex);
        V(empty);
    }

}


void closeandexit()
{
    // clean up shm, sem, thread mutex, condition, etc
    printf("+--- Exitting ---+\n");
    int c1 = pthread_mutex_destroy(&mutex1);
    int c2 = pthread_cond_destroy(&cond1);
    non0_error("Error in thread var destroy while exit", 2, (int[]){ c1, c2 });

    c1 = shmctl(shmq, IPC_RMID, 0);
    c2 = shmctl(shmout, IPC_RMID, 0);
    int c3 = semctl(full, IPC_RMID, 0);
    int c4 = semctl(empty, 0, IPC_RMID, 0);
    int c5 = semctl(mutex, 0, IPC_RMID, 0);
    minus1_error("Error in shmctl(remove)", 2, (int[]){ c1, c2 });
    minus1_error("Error in semctl(remove)", 3, (int[]){ c3, c4, c5 });

    exit(0);
}

// standardised function to check for errors, no repetition for pthread
void non0_error(const char* printmsg, int argcount, int* argval)
{
    int i;
    for( i = 0; i < argcount; i++)
    {
        if(argval[i]!=0)
        {
            fprintf(stderr, "+--- %s : ",printmsg );
            perror("");
            exit(0);
        }
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
