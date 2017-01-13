#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
	int id;
      int val = 5, status = -1;

	id = fork();
	if (id == 0)
	{
		     /* Child Process */
                val = val + 5;
                printf("CHILD: val = %d\n", val); 
                /* make the child sleep so that parent will wait */
		      sleep(5);
                /* exit with a value */
		     exit(1);
	}
	else {
		     /* parent process */
                /* Make the parent process wait for a specifc child to finish */
                waitpid(id, &status, 0);

                /* child has exited after changing val; 
                   print value to see what gets printed */

		     printf("PARENT: val = %d\n", val);

                /* print child exit status */
                printf("PARENT: Child exited with status %d\n", status);
		     /* Do you see the exit value of child printed? */

	     }
}
			
