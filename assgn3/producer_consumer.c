#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <stdbool.h>
/*
* Assignment 3 part 1
* written by @kaustubhhiware as a part of OS Lab
*/
/* shared circular buffers
*  m - producer, n - consumer problem
*
* sum must ideally be m * 50 * 51 / 2 = m * 1275
*/
// view shm -  ipcs -m
// remove all shared memory created by you ->
// ipcs -m | grep 777 | awk '{print $2}' | xargs -n 1 ipcrm -m
// Usage - ./a.out gives direct output
//          ./a.out 1 print every detail
//
#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)

int consumer(int, int, int, int, int, int, int, struct sembuf, struct sembuf );
void producer(int, int, int ,int , int, int, int, struct sembuf, struct sembuf);

// standardised function to check for errors, no repeatition
void printerror(const char* printmsg, int argcount, int* argval)
{
    int i;
    for( i = 0; i < argcount; i++)
    {
        if(argval[i]==-1)
        {
            fprintf(stderr, "+--- %s : ",printmsg );
            perror("");
        }
    }
}


int main(int argc,char* argv[])
{
    int m, n, *sum,exitflag = 0;
    bool printdetails = false;
    if(argc > 1)
    {
        if(atoi(argv[1])==1)
        {
            printdetails = true;
        }
    }
    printf("+--- Enter value of m and n : ");
    scanf("%d %d",&m,&n);
    printf("\n"); // for good measure

    struct sembuf pop, vop ;
    // create a shared buffer of 20 ints
    int shmid = shmget(IPC_PRIVATE, 20*sizeof(int), 0777|IPC_CREAT);
    int shmin = shmget(IPC_PRIVATE, 1*sizeof(int), 0777|IPC_CREAT);
    int shmout = shmget(IPC_PRIVATE, 1*sizeof(int), 0777|IPC_CREAT);
    int shmsum = shmget(IPC_PRIVATE, 1*sizeof(int), 0777|IPC_CREAT);
    int shmcount = shmget(IPC_PRIVATE, 1*sizeof(int), 0777|IPC_CREAT);
    printerror("Error in shmget", 4, (int[]){ shmid, shmsum, shmin, shmout, shmcount });

    // create semaphores as indicatives for full, empty and operative buffer
    int full = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
    int empty = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
    int mutex = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
    printerror("Error in semget", 3, (int[]){ full, empty, mutex });

    int c1 = semctl(full, 0, SETVAL, 0);
    int c2 = semctl(empty, 0, SETVAL, 20);
    int c3 = semctl(mutex, 0, SETVAL, 1);
    printerror("Error in semctl(SETVAL)", 3, (int[]){ c1, c2, c3 });

    // initialize sembufs
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1 ;   // value removed from semaphore
    vop.sem_op = 1 ;    // value added to semaphore

    int status, read, id = fork();
    int *count; // Keep a track of numbers read, when = m*50, exit
    if (id==0)     // child process will call the consumer
    {
        int i;
        for(i = 0; i < n; i++)
        {
            int id1 = fork();
            // need to call consumer function via child processes
            if(id1==0)
            {
                sum = (int *) shmat(shmsum, 0, 0);
                printerror("Error in shmat(child)", 1, (int[]){ *sum });

                while(!exitflag)
                {
                    read = consumer(shmid, shmsum, shmout, shmcount, full, empty, mutex, pop, vop);
                    count = (int *) shmat(shmcount, 0, 0);
                    printerror("Error in shmat(child)", 1, (int[]){ *count });

                    if(printdetails) printf("\t\t\tConsumer #%d consumes %d, total consumed %d\n", i+1, read,*count);
                    fflush(0);
                    if(*count==m*50)
                    {
                        exitflag = 1;
                        printf("+--- All numbers read, exitting\n");
                        fflush(0);
                        exit(0);
                    }
                }

                c1 = shmdt(sum);
                c2 = shmdt(count);
                printerror("Error in shmdt(child)", 2, (int[]){ c1, c2 });
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
                printerror("Error in shmat(parent)", 1, (int[]){ *sum });

                for(j = 0; j < 50; j++)
                {
                    producer(j+1, shmid, shmsum, shmin, full, empty, mutex, pop, vop);
                    if(printdetails) printf("Producer #%d produces %d\n", i+1, j+1);
                    fflush(0);
                }

                c1 = shmdt(sum);
                printerror("Error in shmdt(parent)", 1, (int[]){ c1 });
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
    printerror("Error in shmat", 1, (int[]){ *sum });
    printf("+--- Value of sum : %d\n",sum[0]);
    fflush(0);

    c1 = shmdt(sum);
    printerror("Error in shmdt", 1, (int[]){ c1 });

    c1 = semctl(full, 0, IPC_RMID, 0);
    c2 = semctl(empty, 0, IPC_RMID, 0);
    c3 = semctl(mutex, 0, IPC_RMID, 0);
    printerror("Error in semctl", 3, (int[]){ c1, c2, c3 });

    c1 = shmctl(shmid, IPC_RMID, 0);
    c2 = shmctl(shmsum, IPC_RMID, 0);
    c3 = shmctl(shmin, IPC_RMID, 0);
    int c4 = shmctl(shmout, IPC_RMID, 0);
    int c5 = shmctl(shmcount, IPC_RMID, 0);
    printerror("Error in shmctl", 5, (int[]){ c1, c2, c3, c4, c5 });

}


int consumer (int shmid, int shmsum, int shmout, int shmcount, int full, int empty, int mutex, struct sembuf pop, struct sembuf vop )
{
    int *arr, *sum, *out, *count;
    int read = 0;
    arr = (int *) shmat(shmid, 0, 0);
    sum = (int *) shmat(shmsum, 0, 0);
    out = (int *) shmat(shmout, 0, 0);
    count = (int *) shmat(shmcount, 0, 0);
    printerror("Error in shmat(consumer)", 4, (int[]){ *arr, *sum, *out, *count });

    P(full);
    P(mutex);
    sum[0] += arr[out[0]];
    read = arr[out[0]];
    count[0] += 1;       // update total numbers read
    out[0] = (out[0] + 1) % 20;
    V(mutex);
    V(empty);

    int c1 = shmdt(arr);
    int c2 = shmdt(sum);
    int c3 = shmdt(out);
    int c4 = shmdt(count);
    printerror("Error in shmdt(consumer)", 4, (int[]){ c1, c2, c3, c4 });
    return read;
}


void producer (int i,int shmid, int shmsum,int shmin, int full, int empty, int mutex, struct sembuf pop,struct sembuf vop )
{
    int *arr,*sum,*in;
    arr = (int *) shmat(shmid, 0, 0);
    sum = (int *) shmat(shmsum, 0, 0);
    in = (int *) shmat(shmin, 0, 0);
    printerror("Error in shmat(producer)", 3, (int[]){ *arr, *sum, *in });

    P(empty);
    P(mutex);
    arr[in[0]] = i;
    in[0] = (in[0] + 1) % 20;
    V(mutex);
    V(full);

    int c1 = shmdt(arr);
    int c2 = shmdt(sum);
    int c3 = shmdt(in);
    printerror("Error in shmdt(producer)", 3, (int[]){ c1, c2, c3 });
}
