
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
	}
	else {
		     /* parent process */
                myid = getpid();
                parentid = getppid();
		     printf("PARENT: returned = %d, process id = %d, parent id = %d\n", id, myid, parentid);

	     }

     /* print val */ 
	if (id == 0)
		printf("CHILD: val = %d\n", val);
	else 
		printf("PARENT: val = %d\n", val);
}
			
