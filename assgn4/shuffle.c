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
int rounds = 0, threads = 0;
int n, k, x, rowshift=1;// roswhift maintains if rowshift or colshift
int a[MAX][MAX], last[MAX], last2[MAX];
pthread_t thread_id[MAX];
pthread_mutex_t mutex1;
pthread_cond_t condition;

typedef struct thread_data_ {
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

    // while(rounds < 2*k)
    // {
        // inside critical section
        pthread_mutex_lock(&mutex1);
        if(rounds%2==0) // rowshift
        {
            for ( i = 0; i < n; i++)
            {
                if(tid==0) last[i] = a[i][n-1];
                for(j = (n/x)*tid ; j < (n/x)*(tid + 1) && j < n; j++)
                {
                    a[i][(n+j-1)%n] = a[i][j];
                }
            }
            if(tid==x-1)
            {
                for(i = 0; i < n; i++)
                {
                    a[i][n-2] = last[i];
                }
            }
        }
        else // this is columnshift
        {
            for ( j = 0; j < n; j++)
            {
                if(tid==0) last2[j] = a[n-1][j];
                for(i = n - (n/x)*tid ; i > n - (n/x)*(tid + 1) && i > 0; i--)
                {
                    a[i][j] = a[(n+i-1)%n][j];
                }
            }
            if(tid==x-1)
            {
                for(j = 0; j < n; j++)
                {
                    a[0][j] = last2[j];
                }
            }
        }

        threads++;
        pthread_mutex_unlock(&mutex1);
    // }

    printf("\nThread  %ld finished updating\n", thread_id[tid]);
    pthread_exit(NULL);
}


int main(int argc, char* argv[])
{
    int i, j;
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

    printf("+--- Enter k and x : "); // Assuming x < n is given
    scanf("%d%d",&k,&x);
    if(x>n || x<0 || k < 0)
    {
        printf("+--- Incorrect parameters! Exitting\n");
        exit(0);
    }

    // initialize mutex and condition
    pthread_mutex_init(&mutex1, NULL);
    pthread_cond_init(&condition, NULL);
    thread_data param[x];

    // creating the threads - one rotation
    for (i = 0; i < x; i++)
    {
        param[i].index = i;
        pthread_create(&thread_id[i], NULL, start_routine, (void *) &param[i]);
    }

    // Wait for all threads to terminate
    for(i = 0; i < x; i++)
    {
        pthread_join(thread_id[i], NULL);
    }

    pthread_mutex_lock(&mutex1);
    printf("+--- After %d shifts\n",k);
    printArr();
    pthread_mutex_unlock(&mutex1);

    // Clean up the mutex
    pthread_mutex_destroy(&mutex1);
    pthread_cond_destroy(&condition);
}
