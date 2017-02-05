#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sem.h>
/*
* Assignment 3 part 1
* 14CS30011 : Hiware Kaustubh Narendra
*/
/* shared circular buffers
*  m - producer, n - consumer problem
*
* sum must ideally be m * 50 * 51 / 2 = m * 1275
*/
#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)

void consumer(int, int, int, int, int, int, struct sembuf, struct sembuf );
void producer(int, int, int ,int , int, int, int, struct sembuf, struct sembuf);

int main()
{
	int m, n, *sum;
	printf("+--- Enter values of m and n : ");
    scanf("%d %d\n",&m,&n );
    printf("\n"); // for good measure

	struct sembuf pop, vop ;
    // create a shared buffer of 20 ints
	int shmid = shmget(IPC_PRIVATE, 20*sizeof(int), 0777|IPC_CREAT);
	int shmin = shmget(IPC_PRIVATE, 1*sizeof(int), 0777|IPC_CREAT);
	int shmout = shmget(IPC_PRIVATE, 1*sizeof(int), 0777|IPC_CREAT);
	int shmsum = shmget(IPC_PRIVATE, 1*sizeof(int), 0777|IPC_CREAT);
    if (shmid==-1 || shmsum==-1 || shmin==-1 || shmout==-1)
    {
        perror("+--- Error in shmget ");
    }

    // create semaphores as indicatives for full, empty and operative buffer
	int full = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
	int empty = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
	int mutEx = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
    if(full==-1 || empty==-1 || mutEx==-1)
    {
        perror("+--- Error in semget ");
    }

    int c1 = semctl(full, 0, SETVAL, 0);
    int c2 = semctl(empty, 0, SETVAL, 20);
    int c3 = semctl(mutEx, 0, SETVAL, 1);
    if(c1==-1 ||c2==-1 || c3==-1)
    {
        perror("+--- Error in semctl(SETVAL) ");
    }

    // initialize sembufs
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1 ;   // value removed from semaphore
    vop.sem_op = 1 ;    // value added to semaphore

	int status, id = fork();
	if (id==0)     // child process will call the consumer
	{
		int i, aim = m * 1275;
		for(i = 0; i < n; i++)
        {
			int id1 = fork();
			// need to call consumer function via child processes
			if(id1==0)
            {
				sum = (int *) shmat(shmsum, 0, 0);
				if(*sum==-1)
                {
                    perror("+--- Error in shmat(child) ");
                }

				while(1)
				{
					consumer(shmid, shmsum, shmout,  full, empty, mutEx, pop, vop);
					//printf("consumer %d : %d\n",j,sum[0]);
					//fflush(0);
					if (sum[0]==aim) // this is brute force exit, rectify
                    {
                        exit(0);
                    }
				}

				c1 = shmdt(sum);
                if(c1==-1)
                {
                    perror("+--- Error in shmdt(child) ");
                }
			}
		}
		wait(NULL);
		exit(0);
    }
	else        // parent calls the producer
	{
		int i,j;
		for(i = 0;i < m; i++)
		{
			int id1 = fork();
			if(id1==0)
			{
				sum = (int *) shmat(shmsum, 0, 0);
				if(*sum==-1)
                {
                    perror("+--- Error in shmat(parent) ");
                }

				for(j = 0; j < 50; j++)
                {
					producer(j+1, shmid, shmsum, shmin, full, empty, mutEx, pop, vop);
					//printf("producer %d : %d\n",j,sum[0]);
					//fflush(0);
				}

				c1 = shmdt(sum);
                if(c1==-1)
                {
                    perror("+--- Error in shmdt(parent) ");
                }
				exit(0);
			}
		}
	}

	int completion = wait(&status);
	while (completion > 0)
	{
	    completion = wait(&status);
	}

	sum = (int *) shmat(shmsum, 0, 0);
    if (*sum==-1)
    {
        perror("+--- Error in shmat ");
    }

	printf("+--- Value of sum : %d\n",sum[0]);
	fflush(0);

	c1 = shmdt(sum);
    if(c1==-1)
    {
        perror("+--- Error in shmdt ");
    }

    c1 = semctl(full, 0, IPC_RMID, 0);
    c2 = semctl(empty, 0, IPC_RMID, 0);
    c3 = semctl(mutEx, 0, IPC_RMID, 0);
    if(c1==-1 || c2==-1 || c3==-1)
    {
        perror("+--- Error in semctl(RMID) ");
    }

	c1 = shmctl(shmid, IPC_RMID, 0);
    c2 = shmctl(shmsum, IPC_RMID, 0);
    c3 = shmctl(shmin, IPC_RMID, 0);
    int c4 = shmctl(shmout, IPC_RMID, 0);
    if(c1==-1 || c2==-1 || c3==-1 || c4==-1)
    {
        perror("+--- Error in shmctl ");
    }
}


void consumer (int shmid,int shmsum, int shmout,int full,int empty, int mutEx, struct sembuf pop,struct sembuf vop ){
	int *arr, *sum, *out;
	arr = (int *) shmat(shmid, 0, 0);
	sum = (int *) shmat(shmsum, 0, 0);
	out = (int *) shmat(shmout, 0, 0);
	if(*arr==-1 || *sum==-1 || *out==-1)
	{
	    perror("+--- Error in shmat(consumer) ");
	}

	P(full);
	P(mutEx);
	sum[0] += arr[out[0]];
//	printf("Consumer %d reads %d\n", k, arr[out[0]]);
//	fflush(0);
	out[0] = out[0] + 1;
	if(out[0]==20)  // reset value if all 20 elements consumed
	{
	    out[0]=0;
	}
	V(mutEx);
	V(empty);

    int c1 = shmdt(arr);
    int c2 = shmdt(sum);
    int c3 = shmdt(out);
	if(c1=-1 || c2==-1 || c3==-1)
    {
        perror("+--- Error in shmdt(consumer) ");
    }
}


void producer (int i,int shmid, int shmsum,int shmin, int full, int empty, int mutEx, struct sembuf pop,struct sembuf vop ){
	int *arr,*sum,*in;
	arr = (int *) shmat(shmid, 0, 0);
	sum = (int *) shmat(shmsum, 0, 0);
	in = (int *) shmat(shmin, 0, 0);
	if(*arr==-1 || *sum==-1 || *in==-1)
	{
	    perror("+--- Error in shmat(producer) ");
	}

	P(empty);
	P(mutEx);
	arr[in[0]] = i;
//	printf("Producer %d writes %d\n", k, l);
//	fflush(0);
	in[0] = in[0] + 1;
	if(in[0]==20)   // reset value if all 20 elements produced
	{
	    in[0]=0;
    }
	V(mutEx);
	V(full);

    int c1 = shmdt(arr);
    int c2 = shmdt(sum);
    int c3 = shmdt(in);
	if(c1=-1 || c2==-1 || c3==-1)
    {
        perror("+--- Error in shmdt(producer) ");
    }
}
