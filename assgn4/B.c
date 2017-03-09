#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>
#include <signal.h>
/*
* Assignment 4 part 2
* 14CS30011 : Hiware Kaustubh Narendra
*/
/*
* B is the consumer - recieves requests
* per request, create thread, check for another threads
* per request, call book_ticket()
*/
#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)

typedef struct thread_data_ {
    int index; // which thread is this
}thread_data;

// keep track of # requests, thread started but not finished
int Ticket = 100, *isBrunning, numthreads;

pthread_mutex_t mutex1;
pthread_cond_t cond1;

void closeandexit();
void non0_error(const char* , int , int*);

// function each request will call
void *book_ticket(void *param)
{
    thread_data *t_param = (thread_data *) param;
    int x = (*t_param).index;

    int c1 = pthread_mutex_lock(&mutex1);
    non0_error("Error in mutex lock (book_ticket)", 1, (int[]){ c1 });

    numthreads++;
    while(numthreads > 10)
    {
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
        if(numthreads <= 5)
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
        if(numthreads <= 5)
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


int main(int argc, char* argv[])
{
    srand(time(NULL));
    signal(SIGINT, closeandexit);
    // if # > 10, block B till # <= 5




}


void closeandexit()
{
    // clean up shm, sem, thread mutex, condition, etc
    *isBrunning = 1;
    int c1 = pthread_mutex_destroy(&mutex1);
    int c2 = pthread_cond_destroy(&cond1);
    non0_error("Error in thread vars destroy while exit", 2, (int[]){ c1, c2 });

    exit(0);
}


// standardised function to check for errors, no repetition
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
