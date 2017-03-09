
/***
Simple program to illustrate the use of posix semaphore
(unnamed semaphores, shared between threads of the same process
just as a global variable is. To share between processes, need 
to put in shared memory)
***/


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>

#define NUM_THREADS 5

struct thread_data {
	int t_index;
};

int global_count = 0;

pthread_t thread_id[NUM_THREADS];

sem_t mutex_sem;


// Function executed by all threads created
void *thread_start(void *param)
{
	// Take out the parameter
	struct thread_data *t_param = (struct thread_data *) param;
	int tid = (*t_param).t_index;

	// Update global count inside critical section
	sem_wait(&mutex_sem);
	global_count++;
	// put a sleep to test that others actually wait
	sleep(2);
	sem_post(&mutex_sem);

	// Print success and exit. Actually should not print 
	// thread id with %ld, but ok here as the types match in Linux
	printf("Thread  %ld finished updating\n", thread_id[tid]);
	pthread_exit(NULL);
}


int main()
{
	int no_of_threads, i, id;
	struct thread_data param[NUM_THREADS];
	
	// initialize semaphore to 1
	sem_init(&mutex_sem, 0, 1);

	// Create the threads
	for(i=0; i<NUM_THREADS; i++)
	{
		 param[i].t_index = i;
		 pthread_create(&thread_id[i], NULL, thread_start, (void *) &param[i]);
	}

	// Wait for all threads to terminate
	for(i=0; i<NUM_THREADS; i++)
	{
		pthread_join(thread_id[i], NULL);
	}

	// The lock is actually unnecessary here as all other threads
	// must have exited. But just good practice; in a general setting
	// there can be other threads accessing it other than the ones
	// this process waits for

	sem_wait(&mutex_sem);
	printf("Final Count is %d\n", global_count);
	sem_post(&mutex_sem);

	// Clean up the mutex
	sem_destroy(&mutex_sem);
} 

	
	
	 

