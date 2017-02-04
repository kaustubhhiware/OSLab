#include <stdio.h>
#include  <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include  <sys/types.h>
#include <sys/sem.h>
/*
* Assignment 3 part 1
* 14CS30011 : Hiware Kaustubh Narendra
*/
/* shared circular buffers
*  m - producer, n - consumer problem
*
* sum must ideally be m * 50 * 51 / 2 = m*1275
*/

#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)

int main(int argc,char* argv[])
{
    int m,n, *sum, status;
    printf("+--- Enter values of m and n : ");
    scanf("%d %d\n",&m,&n );

    struct sembuf pop, vop;
    // create a shared buffer of 20 ints
    int shmid = shmget(IPC_PRIVATE, 20*sizeof(int), 0777|IPC_CREAT);
    int shmsum = shmget(IPC_PRIVATE, 1*sizeof(int), 0777|IPC_CREAT);
    if (shmid==-1 | shmsum==-1)
    {
        perror("+--- Error in shmget ");
    }

    // create semaphores as indicatives for full, empty buffer
    int isFull = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
    int isEmpty = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
    if(isFull==-1 || isEmpty==-1)
    {
        perror("+--- Error in semget ");
    }

    int c1 = semctl(full, 0, SETVAL, 0);
    int c2 = semctl(semid2, 0, SETVAL, 1);
    if(c1==-1 ||c2==-1)
    {
        perror("+--- Error in semctl ");
    }

    // initialize sembufs
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1 ;   // calue removed from semaphore
    vop.sem_op = 1 ;    // value added to semaphore

    int id = fork();
    if(id==0)     // child process will be the consumer
    {
        int i,aim = m*1275;;
        for(i = 0;i < n; i++)
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
                    consumer(shmid,shmsum);
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
    else        // parent has the producer
    {
        int i,j;
        for(i = 0;i < m; i++)
        {
            int id1 = fork();
            if(id1==0)
            {
                sum = (int *) shmat(shmsum, 0, 0);
                if (*sum==-1)
                {
                    perror("+--- Error in shmat(parent) ");
                }

                for(j = 0; j < 50; j++)
                {
                    producer();
                    //printf("producer %d : %d\n",j,sum[0]);
                    //fflush(0);
                }

                c2 = shmdt(sum);
                if(c2==-1)
                {
                    perror("+--- Error in shmdt(parent) ");
                }
                exit(0);
            }
        }
    }
    // producer must wait long enough for consumer to consume else no point

    int waitsig = wait(&status);
    while (waitsig > 0)
    {
        waitsig = wait(&status);
    }

    sum = (int *) shmat(shmsum, 0, 0);
    if (*sum==-1)
    {
        perror("+--- Error in shmat ");
    }
    printf("+--- Sum : %d\n",sum[0]);
    fflush(0);

    c1 = shmdt(sum);
    if(c1==-1)
    {
        perror("+--- Error in shmdt(child) ");
    }

    c1=semctl(isFull, 0, IPC_RMID, 0);
    c2=semctl(isEmpty, 0, IPC_RMID, 0);
    if(c1==-1 || c2==-1)
    {
        perror("+--- Error in semctl ");
    }


    c1 = shmctl(shmid, IPC_RMID, 0);
    c2 = shmctl(shmsum, IPC_RMID, 0);
    if(c1==-1 || c2==-1)
    {
        perror("+--- Error in shmctl ");
    }
}
