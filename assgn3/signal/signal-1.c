
// A program that does something on a Ctrl-C before exiting

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>


void my_handler(int signum)
{
	printf("Received a SIGINT, exitting\n");
	exit(-1);
}

int main()
{
	// register signal handler fo Ctrl-C
	signal(SIGINT, my_handler);
	while(1);
}
