// A program to show how a parent process can wait
// for child to terminate without using wait() call
// Uses signals and pause() call to simulate wait().
// Child's exit status is not handled though (so
// not waitpid(), jst wait())

// Uses sigprocmask, sigfillset, and sigdelset. Also
// see other signal set handling functions by doing
// "man sigsetops". Think how you can use them

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>


// Child termination sends SIGCHLD, write a handler for
// SIGCHLD to print a line just to check SIGCHLD is received

void child_handler(int signum)
{
	if (signum == SIGCHLD)
	{
		printf("PROCESS %d: Received SIGCHILD\n", getpid());
	}
}


int main()
{
	int id, signum;

	// sigset_t has one bit for each signal
	sigset_t st, old_st;

	id = fork();
	if (id == -1)
	{
		perror("fork failed: ");
		exit(-1);
	}
	if (id != 0)
	{	
		// parent registers handler for SIGCHILD
		signal(SIGCHLD, child_handler);


		// Add all signals to the signal set st 

		sigfillset(&st);

		// Delete just the SIGCHLD signal from the set st
		// so now all signals other than SIGCHLD are there

		sigdelset(&st, SIGCHLD);

		// Set the signal mask. The first parameter says set
		// the mask, the second one provides the mask.
		// The signal mask is to "mask", i.e., to block
		// all signals that are specified in the signal set
		// st. Note that this means all except SIGCHLD will
		// be blocked, as it is the only signal not in the mask
		// (except SIGKILL and SIGSTOP, for which the mask
		// has no effect as they cannot be blocked)

		sigprocmask(SIG_SETMASK, &st, &old_st);

		printf("PARENT = %d, CHILD = %d\n", getpid(), id);

		// Now the pause can come out only if a SIGCHLD is
		// received, as all other signals are blcoked
		// and will not be handled even by default handler.

		pause();

		// You could have also used the sigwait() function
		// to wait just for SIGCHLD specifically instead of
		// pause() above. Read up the function and
		// try to think what the parameters should be

		printf("PARENT: Exiting\n");
	}
	else {

		// Give the parent time to set mask etc.
		sleep(2);

		// Send a SIGUSR1 signal (its default handler behavior
		// is to terminate the process) to check that it has
		// no effect as it is blocked in parent

		kill(getppid(), SIGUSR1);	

		// Now sleep to make the parent wait, and then exit
		// after printing. This will send the SIGCHLD and make
		// the parent come out of pause
		sleep(5);
		printf("CHILD: Exiting\n");
	    } 	
}
