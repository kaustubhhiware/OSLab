#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
# define MAX 100
/*
* Assignment 4 part 1
* 14CS30011 : Hiware Kaustubh Narendra
*/
// run with -
//  gcc -pthread shuffle.c
//
int n, rowshift=1;// roswhift maintains if rowshift or colshift
int a[MAX][MAX], last[MAX], last2[MAX];
pthread_t thread_id[MAX];
pthread_mutex_t mutex1;

typedef struct thread_data_ {
    int n;
    int x;
    int index; // which thread is this
}thread_data;

// just a function to fancy print your n x n array
void printArr()
{
    int i,j,k;
    for (k = 0; k < n; k++)
    {
        printf("+----");
    }
    printf("+\n");
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            printf("|%4d",a[i][j] );
        }
        printf("|\n");
    }
    for (k = 0; k < n; k++)
    {
        printf("+----");
    }
    printf("+\n");
}


// Function executed by all threads created
void *start_routine(void *param)
{
    // Take out the parameter
    int i, j;
    thread_data *t_param = (thread_data *) param;
    int tid = (*t_param).index;
    int tx = (*t_param).x;
    int tn = (*t_param).n;

    // inside critical section
    pthread_mutex_lock(&mutex1);
    if(rowshift==1) // rowshift
    {
        for ( i = 0; i < tn; i++)
        {
            for(j = (tn/tx)*tid; j < (tn/tx)*(tid + 1) && j < n; j++)
            {
                a[i][(tn+j-1)%tn] = a[i][j];
            }
        }
        if(tid==tx-1)
        {
            for(i = 0; i < n; i++)
            {
                a[i][n-2] = last[i];
            }
        }
    }
    else // this is columnshift
    {
        for ( j = 0; j < tn; j++)
        {
            for(i = tn - (tn/tx)*tid ; i > tn - (tn/tx)*(tid + 1) && i > 0; i--)
            {
                a[i][j] = a[(tn+i-1)%tn][j];
            }
        }
        if(tid==tx-1)
        {
            for(j = 0; j < n; j++)
            {
                a[0][j] = last2[j];
            }
        }
    }
    pthread_mutex_unlock(&mutex1);
    // Print success and exit. Actually should not print
    // thread id with %ld, but ok here as the types match in Linux
    printf("Thread  %ld finished updating\n", thread_id[tid]);
    pthread_exit(NULL);
}


int main(int argc, char* argv[])
{
    int i, j, k, x;
    printf("+--- Enter n : ");
    scanf("%d",&n );
    printf("+--- Enter %d*%d array\n",n,n);
    for(i = 0; i < n; i++)
    {
        for ( j = 0; j < n; j++)
        {
            scanf("%d",&a[i][j] );
        }
    }
    printArr();

    printf("+--- Enter k and x : ");
    scanf("%d%d",&k,&x);

    rowshift = 0; // first complete rowshift
    // initialize mutex
    pthread_mutex_init(&mutex1, NULL);
    thread_data param[x];


    // creating the threads - one rotation
    for(i = 0; i < n; i++)
    {
        last[i] = a[i][n-1];
        last2[i] = a[n-1][i];
    }

    for (i = 0; i < x; i++)
    {
        param[i].n = n;
        param[i].x = x;
        param[i].index = i;
        pthread_create(&thread_id[i], NULL, start_routine, (void *) &param[i]);
    }

    // Wait for all threads to terminate
    for(i = 0; i < x; i++)
    {
        pthread_join(thread_id[i], NULL);
    }

    pthread_mutex_lock(&mutex1);
    printf("+--- After one rowshift\n");
    printArr();
    pthread_mutex_unlock(&mutex1);

    // Clean up the mutex
    pthread_mutex_destroy(&mutex1);

}
