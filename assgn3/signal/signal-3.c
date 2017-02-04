// A program to show the use of SIGUSR1 and SIGUSR2, which are
// general purpose signals that you can use.
// Parent sends child SIGUSR1 to take it out of
// an infinite loop; child sends SIGUSR2 from within
// the handler, whose handler terminates the parent


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>

#include <unistd.h>

// signal handler for SIGUSR1 and SIGUSER2
// Same handler can eb used for both, using
// signum to differentiate between the two

void my_handler(int signum)
{

	// action if SIGUSR1
	if (signum == SIGUSR1)
	{
		printf("PROCESS %d: Received SIGUSR1, exiting after sending SIGUSR2\n", getpid());

		// send SIGUSR2 to parent	
		kill(getppid(), SIGUSR2);

		// then exit	
		exit(0);
	}
	else if (signum == SIGUSR2)
	{
		// action for SIGUSR2
		printf("PROCESS %d: Received SIGUSR2, exiting\n", getpid());
		exit(0);
	}
}


int main()
{
	int id, fd[2], n, err, i, j;
	char buffer[100];

	id = fork();
	if (id == -1)
	{
		perror("fork failed: ");
		exit(-1);
	}
	if (id != 0)
	{	

		printf("PARENT ID = %d, CHILD ID = %d\n", getpid(), id);

		// register handler for SIGUSR2
		signal(SIGUSR2, my_handler);

		// infinite loop, will be terminated by SIGUSR2 receipt
		while(1)
		{
			// sleep for 5 seconds to give child time to run
			// then send it SIGUSR1
			sleep(5);
			kill(id, SIGUSR1);
		}
	}
	else {

		// child registers handler for SIGUSR1
		signal(SIGUSR1, my_handler);

		// Just run an infinite loop waiting for SIGUSR1
		while(1);
	     } 	
}
