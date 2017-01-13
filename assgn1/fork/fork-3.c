#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/* This program should be used in conjunction with the calculator
   program calc.c. The executable file calc should be present
*/

int main()
{
	int id, status = 0;
     int no1, no2;
     char op;
     printf("PARENT: Enter argument operator argument\n");
     scanf("%d %c%d", &no1, &op, &no2);

	id = fork();
	if (id == 0)
	{
		/* Child Process, make it run the calculator. */

		/* set up the parameters; total 4 parameters + end */
           char *argv[5];
           argv[0] = (char *)malloc(strlen("./calc")+1);
 		argv[1] = (char *)malloc(11);  // assuming max 10 digits
 		argv[2] = (char *)malloc(2);
 		argv[3] = (char *)malloc(11);
           strcpy(argv[0], "./calc");
		sprintf(argv[1], "%d", no1);
		sprintf(argv[2], "%c", op);
		sprintf(argv[3], "%d", no2);
		argv[4] = NULL;

          execvp("./calc", argv);
          /* The code should never be here if no error */ 
		perror("execvp failed:");
           exit(-1);
	}
	else {
		     /* parent process */
                /* Make the parent process wait for the child to finish */
                wait(&status);

                /* child has exited; so some  printing and exit */
                printf("PARENT: Child exited with status %d\n", status);

	     }
}
			
