#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#define MAX 1000 // maximum number of elements in the array
/*
* Assignment 1 part 1
* 14CS30011 : Hiware Kaustubh Narendra
*/
/* scan array of integers from file and find if a number exists in it
*  by creating 2 child processes which search linearly in subarrays of size 10
*/
// Usage - gcc search.c
// ./a.out 1 - for child search details
// else ./a.out
// ./a.out 1 filename and ./a.out filename are allowed

/* Split and search array*/
void searchArray(int a[],int l,int r,int k,pid_t filepid,bool child_details)
{
    int flag=0;
    if( r - l < 11) // linear search
    {
        int i;
        for(i = l; i < r + 1; i++ )
        {
            if(a[i] == k)
            {    // if the process is a child created, return exit status 1
                if(getpid() != filepid)
                {
                    if(child_details)
                    {
                       printf("Found!\n");
                    }
                    exit(1);
                }
                else
                {
                    printf("%d found at index %d\n",k,i );
                    return; //return to query another k in case of small array
                }
            }
        }
        // if the process is a child created, return exit status 0 for not found
        if(getpid() != filepid)
        {
            if(child_details)
            {
                printf("Not found\n");
            }
                exit(0);
        }
        else
        {
            //return to query another k in case of small array
            printf("not found\n");
        }
    }
    else //split into two child processes
    {
        pid_t child1, child2, status;
        int m = l+(r-l)/2;
        child1 = fork();
        if(child1 == 0)
        {
            if(child_details)
            {
                printf("\tChild 1 searching from index %d to %d ...",l,m);
            }
            searchArray(a,l,m,k,filepid,child_details);
        }
        else
        {
            child2 = fork();
            if(child2 == 0)
            {
                if(child_details)
                {
                    printf("\tChild 2 searching from index %d to %d ...",m+1,r);
                }
                searchArray(a,m+1,r,k,filepid,child_details);
            }
            else
            {   //parent process of child1 and child2
                // must wait for both child processes to finish as per question
                wait(&child1);
                wait(&child2);
                if(getpid() != filepid)
                {
                    // not the first parent, this itself is a child
                    if(child1+child2 >= 1)
                    {
                        exit(1);
                    }
                    else exit(0);
                }
                else
                {
                    // this is the first process
                    // separated to ensure only one time printing
                    if(child1+child2 >= 1)
                    {
                        printf("\n%d found\n",k);
                    }
                    else printf("\n%d not found\n",k);
                }
            }
        }
    }
}

int main(int argc,char* argv[])
{
    bool child_details = false;
    char filename[100];
    int file_set=0;
    if(argc > 1)
    {
        if(atoi(argv[1]) == 1)
        {
            child_details = true;
        }
        else
        {
            sscanf(argv[1],"%s",filename);
            file_set=1;
        }
    }
    FILE *fp;
    int n;// size of array
    int a[MAX];
    pid_t filepid = getpid();

    //filename either from command line or input
    if(argc > 2 && file_set == 0)
    {
        sscanf(argv[2],"%s",filename);
    }
    else if(file_set == 0)
    {
        printf("+--- Enter file name to read numbers from : ");
        scanf("%s",filename);
    }
    fp = fopen(filename,"r");
    if(fp == NULL)
    {
        printf("File %s does not exist!Exitting...",filename);
        exit(0);
    }
    int i = 0;
    // array of integers - stop at first float taking its integer value
    while (!feof(fp) && fscanf(fp, "%d",&a[i]) == 1)
    {
        i++;
    }
    fclose(fp);
    n = i;
    printf("+--- Array from file: " );
    for(i = 0; i < n ; i++)
    {
        printf("%d ",a[i]);
    }
    printf("\n");

    int k=1;
    char kstr[100];
    printf("+--- Enter key to be found(<1 to exit) : ");

    // assumed that input is int
    // make sure to take only int part, even if float is entered
    // for input 21.6, it will search for 21
    scanf("%s",kstr);
    sscanf(kstr,"%d",&k);
    while(k>0)
    {
        searchArray(a,0,n-1,k,filepid,child_details);
        printf("+--- Enter key to be found(<1 to exit) : ");
        scanf("%s",kstr);
        sscanf(kstr,"%d",&k);
    }
    printf("\nSearch terminated\n");
}
