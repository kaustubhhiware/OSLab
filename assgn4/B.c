#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
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
* 14CS30011 : Hiware Kaustubh Narendra
*/
/*
* B is the consumer - recieves requests
* per request, create thread, check for another threads
* per request, call book_ticket()
*/
//
// run with gcc -pthread B.c -o B

typedef struct thread_data_ {
    int request;
}thread_data;

#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)

// keep track of # requests, thread started but not finished
int Ticket = 100, *isBrunning, numthreads;
//
// this stuff as well, Brunning can be changed
//

pthread_mutex_t mutex1;
pthread_cond_t cond1;

void closeandexit();
// 2 error functions because sem+shm have different errors from pthreads
void non0_error(const char* , int , int*);
void minus1_error(const char* , int , int*);

//
// generalise after sure that specific works
// changed = 1; if changed, ticket
// can I detect useless includes in C
// isBrunning must go to 0 when dead
//
// function each request will call
void *book_ticket(void *param)
{
    thread_data *t_param = (thread_data *) param;
    int x = (*t_param).request;

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
        Ticket = Ticket - x;
        if(Ticket > 100) Ticket = 100; // min (Ticket -x, 100)

        // unlock so that others don't mind me sleeping
        c1 = pthread_mutex_unlock(&mutex1);
        non0_error("Error in mutex unlock (book_ticket)", 1, (int[]){ c1 });

        sleep(rand()%3); // sleep for 0 - 2 s
        c1 = pthread_mutex_lock(&mutex1); // I'm awake, I need to lock
        non0_error("Error in mutex lock (book_ticket)", 1, (int[]){ c1 });

        printf("+--- Ticket : %d\n", Ticket);
        numthreads--;
        if(numthreads < 6)
        {
            // wake up only one process at a time, not multiple
            c1 = pthread_cond_signal(&cond1);
            non0_error("Error in cond signal (book_ticket)", 1, (int[]){ c1 });
        }

        c1 = pthread_mutex_unlock(&mutex1);
        non0_error("Error in mutex unlock (book_ticket)", 1, (int[]){ c1 });
        pthread_exit(1); // returns 1

    }
    else
    {
        c1 = pthread_mutex_unlock(&mutex1);
        non0_error("Error in mutex unlock (book_ticket)", 1, (int[]){ c1 });

        sleep(rand()%3);
        c1 = pthread_mutex_lock(&mutex1); // I'm awake, I need to lock
        non0_error("Error in mutex lock (book_ticket)", 1, (int[]){ c1 });

        printf("+--- Request could not be fulfilled :/\n" );
        numthreads--;
        if(numthreads < 6)
        {
            // wake up only one process at a time, not multiple
            c1 = pthread_cond_signal(&cond1);
            non0_error("Error in cond signal (book_ticket)", 1, (int[]){ c1 });
        }

        c1 = pthread_mutex_unlock(&mutex1);
        non0_error("Error in mutex unlock (book_ticket)", 1, (int[]){ c1 });
        pthread_exit(0); // returns 0

    }
}

// need to declare globally to handle Ctrl+C
int shmb, shmq, shmin, shmout, full, empty, mutex;
key_t keyb = 0;
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
    shmb = shmget(keyb, sizeof(int), 0777|IPC_CREAT);
    shmq = shmget(keyq, 10*sizeof(int), 0777|IPC_CREAT);
    shmin = shmget(keyi, sizeof(int), 0777|IPC_CREAT);
    shmout = shmget(keyo, sizeof(int), 0777|IPC_CREAT);
    minus1_error("Error in shmget", 4, (int[]){ shmb, shmq, shmin, shmout });

    // create semaphores as indicatives for full, empty and operative buffer
    full = semget(keyf, 1, 0777|IPC_CREAT);
    empty = semget(keye, 1, 0777|IPC_CREAT);
    mutex = semget(keym, 1, 0777|IPC_CREAT);
    minus1_error("Error in semget", 3, (int[]){ full, empty, mutex });

    c1 = semctl(full, 0, SETVAL, 0);
    c2 = semctl(empty, 0, SETVAL, 10);
    int c3 = semctl(mutex, 0, SETVAL, 1);
    minus1_error("Error in semctl(SETVAL)", 3, (int[]){ c1, c2, c3 });


    isBrunning = (int *) shmat(shmb, 0, 0 );
    int *q, *in, *out;
    q = (int *) shmat(shmq, 0, 0);
    in = (int *) shmat(shmin, 0, 0);
    out = (int *) shmat(shmout, 0, 0);
    minus1_error("Error in shmat", 3, (int[]){ *q, *in, *out });

    int request;
    while(1) // until a Ctrl+C is pressed
    {
        pthread_t id;
        c1 = pthread_mutex_lock(&mutex1);
        non0_error("Error in thread var init", 1, (int[]){ c1, c2 });

        while(numthreads > 10)
        {
            c1 = pthread_cond_wait(&cond1, &mutex1);
            non0_error("Error in cond wait (book_ticket)", 1, (int[]){ c1 });
        }

        c1 = pthread_mutex_unlock(&mutex1);
        non0_error("Error in mutex unlock (book_ticket)", 1, (int[]){ c1 });

        P(full);
        P(mutex);
// optimise this stuff. Get rid of request, out[0] ko *out kar
        request = q[out[0]];
        out[0] = (out[0] + 1) % 10;

        thread_data param;
        param.request = request;
        pthread_create(&id, NULL, book_ticket, (void *) &param);

        V(mutex);
        V(empty);
    }

}


void closeandexit()
{
    // clean up shm, sem, thread mutex, condition, etc
    printf("+--- Exitting ---+\n");
// set to 0
    isBrunning[0] = 1;
    int c1 = pthread_mutex_destroy(&mutex1);
    int c2 = pthread_cond_destroy(&cond1);
    non0_error("Error in thread var destroy while exit", 2, (int[]){ c1, c2 });

    c1 = shmctl(shmb, IPC_RMID, 0);
    c2 = shmctl(shmq, IPC_RMID, 0);
    int c3 = shmctl(shmin, IPC_RMID, 0);
    int c4 = shmctl(shmout, IPC_RMID, 0);
    int c5 = semctl(full, IPC_RMID, 0);
    int c6 = semctl(empty, 0, IPC_RMID, 0);
    int c7 = semctl(mutex, 0, IPC_RMID, 0);
    minus1_error("Error in shmctl(remove)", 4, (int[]){ c1, c2, c3, c4 });
    minus1_error("Error in semctl(remove)", 3, (int[]){ c5, c6, c7 });

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
