#include<stdlib.h>
#include<stdio.h>
#include <unistd.h>
#include <string.h>
/*
* Assignment 1 part 2 - xsort.c
* 14CS30011 : Hiware Kaustubh Narendra
*/

/* run sort1.c
* Usage - xsort <filename>
*/
int main(int argc,char* argv[])
{
    int ret;
    char filename[100],cwd[1000];
    if (argc<2)
    {
        printf("Need to enter filename!\n");
        exit(0);
    }
    sscanf(argv[1],"%s",filename);
    FILE* fp;
    fp = fopen(filename,"r");
    if(fp == NULL)
    {
        printf("File %s does not exist!\n",filename);
    }
    fclose(fp);
    char* path = getcwd(cwd, sizeof(cwd)); // current folder
    char loc_sort[1000]; // location of executable sort1
    strcpy(loc_sort,path);
    strcat(loc_sort,"/sort1");

    //ret = execl(getcwd(cwd, sizeof(cwd)), "./sort1",filename,(char *)0);
    ret = execlp(loc_sort,"./sort1",filename,(char  *) NULL);

}
