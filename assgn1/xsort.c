#include<stdlib.h>
#include<stdio.h>
#include <unistd.h>
#include <string.h>
/*
* Assignment 1 part 2 - xsort.c
* 14CS30011 : Hiware Kaustubh Narendra
*/
/* run sort1 in a seperate xterm when invoked from terminal
*/
/* get sort1 executable first - gcc sort1.c -o sort1
*  create xsort executable then - gcc xsort.c -o xsort
*  Usage - ./xsort <filename> in current dir
*  PATH can be absolute or file must be in same folder as current working dir
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
        exit(0);
    }
    fclose(fp);
    char* path = getcwd(cwd, sizeof(cwd)); // current folder
    char loc_sort[1000]; // location of executable sort1
    strcpy(loc_sort,path);
    strcat(loc_sort,"/sort1");

    // initial method for terminal execution
    //ret = execlp(loc_sort,"/sort1",filename,(char  *) NULL);
    // open xterm and hold until user exits with Ctrl+C
    ret = execl("/usr/bin/xterm", "/usr/bin/xterm", "-hold", "-e",loc_sort,filename, (void*)NULL);

}
