#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	int no1, no2, result;
	char op;
	if (argc != 4)
	{
		printf("Usage: calc no1 op no2 (For op=*, escape * while entering\n");
		exit(-1);
	}
	sscanf(argv[1], "%d", &no1);
	sscanf(argv[2], "%c", &op);
	sscanf(argv[3], "%d", &no2);
        switch (op) {
	     case '+': result = no1 + no2;
		       break;
	     case '-': result = no1 - no2;
	               break;
	     case '*': result = no1 * no2;
		       break;
	     case '/': result = no1/no2; // truncation possible
                       break;
             default:  printf("Wrong operator\n");
                       exit(-1);
        }
        printf("%d %c %d = %d\n", no1, op, no2, result);
        exit(1);
}
