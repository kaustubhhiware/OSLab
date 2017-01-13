#include<stdlib.h>
#include<stdio.h>
#define MAX 1000 // maximum number of elements in the array
/*
* Assignment 1 part 2 - sort1.c
* 14CS30011 : Hiware Kaustubh Narendra
*/
/* take filename as a command line parameter and read array of integers from it
* print the sorted array using mergesort
*/

//executable -  gcc sort1.c -o sort1
// ./sort1 filename
// cc -o sort1 sort1.c

void merge(int a[], int l, int m, int r);
void mergeSort(int a[], int l, int r);

int main(int argc,char* argv[])
{
    char filename[100];
    int a[MAX],n,i = 0;
    FILE* fp;
    if(argc>1)
    {
        sscanf(argv[1],"%s",filename);
    }
    else
    {
        printf("Command line argument expected! Reading from terminal ..\n");
        printf("+--- Enter file name to read numbers from : ");
        scanf("%s",filename);
    }

    fp = fopen(filename,"r");
    if(fp == NULL)
    {
        printf("File %s does not exist!Exitting...",filename);
        exit(0);
    }
    // array of integers - stop at first float taking its integer value
    while (!feof(fp) && fscanf(fp, "%d",&a[i]) == 1)
    {
        i++;
    }
    fclose(fp);
    n = i;
    printf("+--- Array from file :" );
    for(i = 0; i < n ; i++)
    {
        printf("%d ",a[i]);
    }
    printf("\n");

    mergeSort(a, 0, n - 1);
    printf("\n+--- Sorted array is :");
    for( i = 0;i < n; i++)
    {
        printf("%d ",a[i]);
    }
}

void merge(int a[], int l, int m, int r)
{
    int i, j, k;
    int n1 = m-l+1;
    int n2 =  r-m;
    int L[n1], R[n2];

    for(i = 0; i < n1; i++)
    {
        L[i] = a[l + i];
    }
    for(j = 0; j < n2; j++)
    {
        R[j] = a[m + 1+ j];
    }
    i = 0;j = 0; k = l;
    while (i < n1 && j < n2)
    {
        if (L[i] <= R[j])
        {
            a[k] = L[i];
            i++;
        }
        else
        {
            a[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1)
    {
        a[k] = L[i];
        i++;
        k++;
    }

    while (j < n2)
    {
        a[k] = R[j];
        j++;
        k++;
    }
}

void mergeSort(int a[], int l, int r)
{
    if (l < r)
    {
        int m = l+(r-l)/2;
        mergeSort(a, l, m);
        mergeSort(a, m+1, r);
        merge(a, l, m, r);
    }
}
