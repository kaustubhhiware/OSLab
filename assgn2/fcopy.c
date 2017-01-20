#include<stdlib.h>
#include<stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFSIZE 100
/*
* Assignment 2 part 1 - fcopy.c
* 14CS30011 : Hiware Kaustubh Narendra
*/
/* copy from one file to the other using pipes
*/
/* 
*  Usage - ./a.out <file1> <file2> in current dir
*  PATH can be absolute or file must be in same folder as current working dir
*/
int main(int argc,char* argv[])
{
    int file1,file2,details = 0;
    char line1[2*BUFSIZE], line2[2*BUFSIZE], stat[2*BUFSIZE];
    
    if(argc < 3)
    {
        printf("\n+--- Insufficient arguments! Use - ./a.out file1 file2\n");
        exit(0);    
    }
    if(argc > 3)
    {
        sscanf(argv[3],"%d",&details);
    }
    
    file1 = open( argv[1], O_RDONLY);
    file2 = open( argv[2], O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    // Create new file if absent. File2 created must be accessible to user. 

    if(file1 < 0)
    {
        // file1 is missing
        perror("\n+--- Error in reading file1 : ");
        exit(0);    
    }
    else if(file2 < 0)
    {
        // file2 not present and could not be created
        perror("\n+--- Error in handling file2 : ");
        exit(0);    
    }
    
    int fd[2],fe[2]; // file descriptors for 2 pipes

    // parent process creates 2 pipes
    if(pipe(fd) == -1 || pipe(fe) == -1)
    {
        perror("\n+--- Pipe error : ");
        exit(1);
    }
    
    int id = fork();
    if(id == 0)
    {
        // child process
        while(1)
        {
            close( fd[1]); // not writing into file1
            close( fe[0]); // not reading to file2, only send status to parent
            
            int rc = read( fd[0], line2, BUFSIZE);
            int wc = write( file2, line2, rc); // append to current file2 progress
            
            if(details == 1)
            {
                printf("\nline2 : %s\n",line2);
            }
            
            if(wc < 0)
            {
                // complain to parent you failed
                int error_back = write( fe[1], "-1", 2); 
                printf("+--- Could not write to file2");
                exit(-1);
            }
            else if(wc < BUFSIZE) // w >=0 and bytes read is < 100
            {
                printf("+--- File copied successfully! \n");
                int success = write( fe[1], "0", 1); // return success execution
                exit(1);
            }
            else    // writing not complete yet
            {
                int writing = write( fe[1], "0", 1);
            }
        }
    }
    else
    {
        //parent process
        while(1)
        {
            close(fd[0]); // not reading from file2
            close(fe[1]); // don't write to status report pipe
            
            int rp = read( file1, line1, BUFSIZE);
            int wp = write( fd[1], line1, rp);
            
            if(details == 1)
            {
                printf("\nline1 : %s\n",line1);
            }
            
            int status = read( fe[0], stat, BUFSIZE);
            
            if(status == 2)
            {
                printf("+--- Parent exitting with error status\n");
                exit(-1);            
            }
            else if(status == 1 && rp >=0 && rp < BUFSIZE)
            {
                printf("+--- Parent exitting with success status\n");
                exit(1);
            }
 
        }
        
    }
}
