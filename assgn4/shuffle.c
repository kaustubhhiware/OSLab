#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#define MAX 100
/*
* Assignment 4 part 1
* 14CS30011 : Hiware Kaustubh Narendra
*/
/*
* perform rowshift and column shift on n*n matrix
*/
// run with -
//  gcc -pthread shuffle.c
// ./a.out 1 prints intermediate matrices as well
//
int rowrounds = 0, colrounds = 0, threads = 0;
int n, k, x, details = 0;
int a[MAX][MAX], last[MAX], last2[MAX];
pthread_t thread_id[MAX];
pthread_mutex_t mutex1;
pthread_cond_t row_cond, col_cond;

typedef struct thread_data_ {
    int index; // which thread is this
}thread_data;

// just a function to fancy print your n x n array
void printArr(const char* delim)
{
    int i,j,k;
    printf("%s",delim);
    for (k = 0; k < n; k++)
    {
        printf("+----");
    }
    printf("+\n");
    for (i = 0; i < n; i++)
    {
        printf("%s",delim);
        for (j = 0; j < n; j++)
        {
            printf("|%4d",a[i][j] );
        }
        printf("|\n");
    }
    printf("%s",delim);
    for (k = 0; k < n; k++)
    {
        printf("+----");
    }
    printf("+\n");
}


// Function executed by all threads created
void *start_routine(void *param)
{
    int i, j;
    thread_data *t_param = (thread_data *) param;
    int tid = (*t_param).index;

    while(rowrounds < k || colrounds < k)
    {
        // inside critical section
        pthread_mutex_lock(&mutex1);
        if(rowrounds==colrounds) // rowshift
        {
            for ( i = (n*tid)/x ; i < (n*(tid + 1))/x && i < n; i++)
            {
                last[i] = a[i][0];
                for(j = 0; j < n-1; j++)
                {
                    a[i][j] = a[i][j+1];
                }
            }
            if(tid==x-1) // in the last thread, copy the last column
            {
                for(i = 0; i < n; i++)
                {
                    a[i][n-1] = last[i];
                }
            }
        }
        else // this is columnshift
        {
            for ( j = (n*tid)/x; j < (n*(tid + 1))/x && j < n; j++)
            {
                last2[j] = a[n-1][j];
                for(i = n-1; i > 0; i--)
                {
                    a[i][j] = a[(n+i-1)%n][j];
                }
            }
            if(tid==0)
            {
                for(j = 0; j < n; j++)
                {
                    a[0][j] = last2[j];
                }
            }
        }

        threads++;
        // printf("Currently %d rowrounds, %d colrounds & %d threads \n", rowrounds, colrounds, threads );
        if(threads==x)
        {
            if(rowrounds==colrounds)
            {
                pthread_cond_broadcast(&row_cond);
                rowrounds++;
            }
            else
            {
                pthread_cond_broadcast(&col_cond);
                colrounds++;
            }
            threads = 0;
            printf("Completed %d rowrounds and %d colrounds\n", rowrounds, colrounds );
            if(details) printArr("\t");
        }
        if(threads!=0) // yet to complete this round
        {
            if(rowrounds==colrounds)
            {
                // wait for ith rowshift to start ith colshift
                pthread_cond_wait(&row_cond, &mutex1);
            }
            else
            {
                // wait for ith colshift to start i+1 th rowshift
                pthread_cond_wait(&col_cond, &mutex1);
            }
        }
        pthread_mutex_unlock(&mutex1);
    }

    printf("\nThread  %ld finished updating\n", thread_id[tid]);
    pthread_exit(NULL);
}


int main(int argc, char* argv[])
{
    int i, j;
    if(argc > 1) details = 1;

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
    printArr("");

    printf("+--- Enter k and x : "); // Assuming x < n is given
    scanf("%d%d",&k,&x);
    if(k>n) printf("Reducing %d to %d since after %d shifts, matrix regains content\n", k, k%n, n);
    k = k%n;
    if(x>n || x<0 || k < 0 || n > MAX)
    {
        printf("+--- Incorrect parameters! Exitting\n");
        exit(0);
    }

    // initialize mutex and condition
    pthread_mutex_init(&mutex1, NULL);
    pthread_cond_init(&row_cond, NULL);
    pthread_cond_init(&col_cond, NULL);
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
    printArr("");
    pthread_mutex_unlock(&mutex1);

    // Clean up the thread vars
    pthread_mutex_destroy(&mutex1);
    pthread_cond_destroy(&row_cond);
    pthread_cond_destroy(&col_cond);
}
