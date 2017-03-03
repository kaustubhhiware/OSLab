
/***
Program to illustarte the the use of condition variables
***/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_THREADS 10

struct thread_data {
	int t_index;
};

int global_count = 0;

pthread_t thread_id[NUM_THREADS];

pthread_mutex_t cnt_mutex;
pthread_cond_t cnt_cond;


// Function executed by threads A which increment global count

void *thread_start_A(void *param)
{
	struct thread_data *t_param = (struct thread_data *) param;
	int tid = (*t_param).t_index;
	int sleep_time;

	// sleep for a random time (1 or 2 seconds) to test that
	// the waiting thread actually waits

	sleep_time = (rand() % 2) + 1;
	sleep(sleep_time);

	// condition variables are always accessed with a semaphore,
	// lock the semaphore

	pthread_mutex_lock(&cnt_mutex);
	global_count++;
	printf("Thread A (id %ld) finished incrementing\n", thread_id[tid]);

	// signal to a waiting process if the condition is successful.
	// If there were multiple waiting processes and you want to 
	// wake them all up, use pthread_cond_broadcast()

	if(global_count == NUM_THREADS -1 )
		pthread_cond_signal(&cnt_cond);

	// Unlock the mutex

	pthread_mutex_unlock(&cnt_mutex);

	pthread_exit(NULL);
}


// Function executed by threade B which waits for the count value
// to reach NUM_THREADS - 1 (all threads have incremented)

void *thread_start_B(void *param)
{
	pthread_t *tid = (pthread_t *) param;

	// lock the mutex associated with the condition variable
	pthread_mutex_lock(&cnt_mutex);

	// wait if the condition is not satisfied.
	// while is used instead of just if as Posix standard
	// allows spurious wakeups even if the condition is \
	// not satisfied.
	// By property of condition variables, cnt_mutex will be
	// automatically unlocked if the thread blocks

	while (global_count != NUM_THREADS - 1)
		pthread_cond_wait(&cnt_cond, &cnt_mutex);
	
	printf("Thread B (id %ld): global count = %d \n", *tid, global_count);

	// cnt_mutex is automatically locked once the wait is over,
	// so need to unlock

	pthread_mutex_unlock(&cnt_mutex);

	pthread_exit(NULL);
}


int main()
{
	int no_of_threads, i;
	struct thread_data param[NUM_THREADS];
	pthread_t tid;
	
	// initialize the mutex and the condition variable

	pthread_mutex_init(&cnt_mutex, NULL);
	pthread_cond_init(&cnt_cond, NULL);

	// create thread B

	pthread_create(&tid, NULL, thread_start_B, (void *) &tid);

	// create (NUM_THREADS - 1) thread A's

	for(i=0; i<NUM_THREADS-1; i++)
	{
		 param[i].t_index = i;
		 pthread_create(&thread_id[i], NULL, thread_start_A, (void *) &param[i]);
	}

	// wait for all thread A's to finish

	for(i=0; i<NUM_THREADS-1; i++)
	{
		pthread_join(thread_id[i], NULL);
	}

	// wait for thread B to finish

	pthread_join(tid, NULL);

	printf("All threads terminated\n");

	// clean up the mutex and condition variable
	pthread_mutex_destroy(&cnt_mutex);
	pthread_cond_destroy(&cnt_cond);
} 

	
	
	 

