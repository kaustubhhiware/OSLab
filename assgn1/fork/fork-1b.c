#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
	int id, myid, parentid;
        int val = 5;

	id = fork();
	if (id == 0)
	{
		     /* Child Process */
                myid = getpid();
                parentid = getppid();
		     printf("CHILD: process id = %d, parent id = %d\n", myid, parentid);
		     /* Change the value of val */
                val = val + 5;
                printf("CHILD: val = %d\n", val); 
	}
	else {
		     /* parent process */
                myid = getpid();
                parentid = getppid();
		     printf("PARENT: returned = %d, process id = %d, parent id = %d\n", id, myid, parentid);

		     /* Make the parent sleep for 5 seconds so that 
                   the child changes val first */
                sleep(5);

                /* Now print val */
		      printf("PARENT: val = %d\n", val);

	     }
}
			
