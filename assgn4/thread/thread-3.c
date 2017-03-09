
/***
Simple program to illustrate the use of returning status from posix threads.
We pass back an int and a string, but you can pass back anything
you want in the structure.

Also, we show the of thread attributes by setting the state of each
thread as JOINABLE at creation, so that another thread can wait for it
with pthread_join. Though in many cases this is default, it is a good
practice to make your code portable. Also sometimes you may want the
thread to be non-joinable (detached).

***/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_THREADS 5

struct thread_data {
	int t_index;
};

int global_count = 0;

pthread_t thread_id[NUM_THREADS];
pthread_attr_t thread_attr;

// structure to pass status infor, as only a single void * can be passed
struct thread_status {
	int status;
	char msg[25];
};


// Function executed by all threads created
void *thread_start(void *param)
{
	// Take out the parameter
	struct thread_data *t_param = (struct thread_data *) param;
	struct thread_status *stat;
	int tid = (*t_param).t_index;

	// allocate structure to pass sttaus back
	stat = (struct thread_status *)malloc(sizeof(struct thread_status));

	// do nothing, just wat for a random time
	sleep(rand()%2 + 1);

	// fill up status and exit passing status
	stat->status = tid;  // just an arbitrary sttaus we choose
	strcpy(stat->msg,"Thread Exited Normally");
	pthread_exit((void *) stat);
}


int main()
{
	int no_of_threads, i, id;
	struct thread_data param[NUM_THREADS];
	struct thread_status *stat;

	// initialize and set the attribute structure
	pthread_attr_init(&thread_attr);
	pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);

	// Create the threads, passing the attribute paramter
	for(i=0; i<NUM_THREADS; i++)
	{
		 param[i].t_index = i;
		 pthread_create(&thread_id[i], &thread_attr, thread_start, (void *) &param[i]);
	}


	// Wait for all threads to terminate
	for(i=0; i<NUM_THREADS; i++)
	{

		// sleep for 1 second between ecah join, just to test that
		// join can occur even if all threads have already terminated
		sleep(1);

		pthread_join(thread_id[i], (void **) &stat);
		// print the information in sttaus
		printf("Thread No. %d exited with message: %s\n", stat->status, stat->msg);
		// clean up the stat variable, as it has been malloc'ed
		free(stat);
	}

}
