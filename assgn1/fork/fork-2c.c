#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
	int i, id[2], status[2];

     // create 2 child processes and make them sleep
     for (i=0; i<2; i++)
	{	
	   id[i] = fork();
	   if (id[i] == 0)
	   {
		     /* Child Process */
                printf("CHILD: pid = %d\n", getpid()); 
                /* make the child sleep so that parent will wait */
		      sleep(5 + i);
                /* exit with a value */
		      exit(i);
	   }
      }
      /* Make the parent process wait for a specifc child to finish */
      waitpid(id[0], &status[0], 0);
      printf("PARENT: Child %d exited with status %d\n", id[0], status[0]);
      waitpid(id[1], &status[1], 0);
      printf("PARENT: Child %d exited with status %d\n", id[1], status[1]);

}
			
