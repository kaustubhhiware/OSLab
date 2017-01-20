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
    int file1,file2;
    char line[BUFSIZE], line1[BUFSIZE], line2[BUFSIZE];
    
    if(argc < 3)
    {
        printf("\n+--- Insufficient arguments! Use - ./a.out file1 file2\n");
        exit(0);    
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
    int pipe1,pipe2;
    pipe1 = pipe(fd);
    pipe2 = pipe(fe);
    // parent process creates 2 pipes
    if(pipe1 == -1 || pipe2 == -1)
    {
        perror("\n+--- Pipe error : ");
        exit(EXIT_FAILURE);
    }
    
    int id = fork();
    if(id == 0)
    {
        // child process
        while(1)
        {
            close( fd[1]); // not writing into file1
            close( fe[0]); // not reading to file2, only send status to parent
            
            int r = read( fd[1], line1, 100);
            int w = write( file2, line1, r); // append to current file2 progress
            
            if(w < 0)
            {
                // complain to parent you failed
                int error_back = write( fe[1], "-1", 2); 
                printf("+--- Could not write to file2.");
                exit(-1);
            }
            else if(w < 100) // w >=0 and bytes read is < 100
            {
                printf("+--- File copied successfully! ");
                int success = write( fe[1], "0", 1); // return success execution
                exit(1);
            }
            else    // writing not complete yet
            {
                int writing = write( fe[1], "0", 1);
            }
        }
    }
    else if(id > 0)
    {
        //parent process
        /*
        The parent, on receiving 0, reads the next 100 bytes and repeats the 
        process. It exits either on receiving a -1 from child, or on receiving 
        a 0 after sending < 100 bytes for the last send (you can assume that 
        the file size is not a multiple of 100 bytes, so the last send will have
         < 100 bytes).       
        */
        while(1)
        {
        
            close(fd[0]); // not reading from file2
            close(fe[1]); // don't write to status report pipe
            
            int r = read( file1, line, 100);
            int w = read( file1, line, r);
            
            int status = read( fe[0], line2, 100);
            
            if(status == 2)
            {
                exit(-1);            
            }
            else if(status == 1 && r >=0 && r < 100)
            {
                exit(1);
            }
 
        }
        
    }




}
