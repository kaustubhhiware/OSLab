
/***
Simple program to illustrate the use of posix semaphore
(unnamed semaphores, shared between threads of the same process
just as a global variable is. To share between processes, need 
to put in shared memory)
This example shows a non-mutual exclusion synchronization example
where each thraed signals the other. You can put sleep at different
places to test that this behaves as a true semaphore in that signals
are remembered even if waits are done latar.

Also note that if all of them wait, the single signal can wake up
any one of them. If you run it again and again, you will see different
order of prints

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

sem_t signal_sem;



// Function executed by all threads created
void *thread_start(void *param)
{
	// Take out the parameter
	struct thread_data *t_param = (struct thread_data *) param;
	int tid = (*t_param).t_index;


	// wait to be signalled to start
	sem_wait(&signal_sem);

	// Update count. Since by design only one thread will go at a time,
	// no mutual exclusion is needed
	global_count++;

	// Print updation is done. Actually should not print 
	// thread id with %ld, but ok here as the types match in Linux
	printf("Thread  No. %d with id %ld finished updating\n", tid, thread_id[tid]);

	// put a sleep to test that others actually wait
	sleep(2);

	// signal the next thread. But the last one should not signal
	// (though no harm really, but still)
	if(global_count < 5)
	{	
		printf("Thread No. %d signalling next thread\n", tid);
		sem_post(&signal_sem);
	}


	pthread_exit(NULL);
}


int main()
{
	int no_of_threads, i, id;
	struct thread_data param[NUM_THREADS];
	
	// initialize semaphore to 0 so that all threads are blocked initially
	sem_init(&signal_sem, 0, 0);

	// Create the threads
	for(i=0; i<NUM_THREADS; i++)
	{
		 param[i].t_index = i;
		 pthread_create(&thread_id[i], NULL, thread_start, (void *) &param[i]);
	}

	// Now signal one of the threads
	sem_post(&signal_sem);

	// Wait for all threads to terminate
	for(i=0; i<NUM_THREADS; i++)
	{
		pthread_join(thread_id[i], NULL);
	}

        // No lock needed here as all other threads have exited
	printf("Final Count is %d\n", global_count);

	// Clean up the mutex
	sem_destroy(&signal_sem);
} 

	
	
	 

