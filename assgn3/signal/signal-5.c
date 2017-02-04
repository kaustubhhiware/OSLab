// A program to show how to make a process sleep
// without using the sleep() call. Uses the
// alarm() and pause() call with signals to
// simulate sleep()

// Uses sigprocmask, sigfillset, and sigdelset. Also
// see other signal set handling functions by doing
// "man sigsetops". Think how you can use them

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>


// Write  a handler for the signal SIGALRM that we will use 
// just to check SIGALRM is received

void alarm_handler(int signum)
{
	if (signum == SIGALRM)
	{
		printf("PROCESS %d: Received SIGALRM\n", getpid());
	}
}


int main()
{

	// sigset_t has one bit for each signal
	sigset_t st, old_st;

	// parent registers handler for SIGALRM
	signal(SIGALRM, alarm_handler);


	// Add all signals to the signal set st 

	sigfillset(&st);

	// Delete just the SIGALRM signal from the set st
	// so now all signals other than SIGALRM are there

	sigdelset(&st, SIGALRM);

	// Set the signal mask. The first parameter says set
	// the mask, the second one provides the mask.
	// The signal mask is to "mask", i.e., to block
	// all signals that are specified in the signal set
	// st. Note that this means all except SIGALRM will
	// be blocked, as it is the only signal not in the mask
	// (except SIGKILL and SIGSTOP, for which the mask
	// has no effect as they cannot be blocked)

	sigprocmask(SIG_SETMASK, &st, &old_st);


	// Who will raise the SIGALRM signal?
	// The alarm(x) call raises a SIGALRM signal
	// after x seconds. Raise an alarm after 5 seconds

	alarm(5);

	// Now the pause can come out only if a SIGALRM is
	// received, as all other signals are blocked
	// and will not be handled even by default handler.
	// Since the SIGALRM will be raised by the alarm()
	// call after 5 seconds, the process pauses for 5 seconds

	printf("PROCESS %d: Just before pausing\n", getpid());
	pause();

	printf("PROCESS %d: Out of pause, exiting\n", getpid());

}
