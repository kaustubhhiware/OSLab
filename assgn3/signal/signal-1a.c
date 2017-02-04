
// A program that cannot be terminated by Ctrl+C

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>


int main()
{
	signal(SIGINT, SIG_IGN);
	while(1);
}
