#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "devices/timer.h"

void test_demo(void) {

int **Y=(int **)malloc(5*sizeof(int *));
int i,j;
for(i=0;i<5;++i)
{
Y[i]=(int *)malloc(sizeof(int)*5);
}
for(i=0;i<5;i++)
{
for(j=0;j<2;++j)
{
Y[i][j]=6;
printf("i= %d,\tj= %d\n",i,j);
}
printf("\n");
}

  
  printf("Here I am\n");
/*
int *X=(int *)malloc(sizeof(int)*2);
X[0]=0;
X[1]=1;
printf("%d  %d\n",X[0],X[1] );
*/

}
void test_demo2(void) {
	int i=10;
	while(i)
		printf("Yes\n");
}