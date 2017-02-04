// A program that catches SIGPIPE when write to a
// pipe is attempted when read is closed


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

void pipe_handler(int signum)
{
	printf("PROCESS %d: pipe read end closed, no more data\n", getpid());
}


int main()
{
	int id, fd[2], n, err, i, j;
	char buffer[100];


	err = pipe(fd);
	if (err == -1)
	{
		perror("Pipe error: ");
		exit(-1);
	}
	id = fork();
	if (id == -1)
	{
		perror("fork failed: ");
		exit(-1);
	}
	if (id != 0)
	{	
		// parent registers handler for SIGPIPE
		signal(SIGPIPE, pipe_handler);

		close(fd[0]);
		printf("PARENT ID = %d,  CHILD ID = %d\n", getpid(), id);
		strcpy(buffer, "TEST_STRING");
		while(1)
		{
			// Write more than once with 5 second gap
			// Child will read once and exit, so second
			// write will cause SIGPIPE to be sent to parent

			n = write(fd[1], buffer, strlen(buffer)+1); 
			if (n == -1)
			{
				perror("write failed:");
				exit(-1);
			}
			sleep(5);
		}
	}
	else {
		close(fd[1]);
		// read once and exit, so parent will get SIGPIPE on write
		n = read(fd[0], buffer, 100);
		if (n == -1)
		{
			perror("read failed:");
			exit(-1);
		}
		if (n != 0)
			printf("CHILD: %d: %s\n", n, buffer);
	    } 	
}
