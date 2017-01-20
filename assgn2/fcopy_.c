#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include  <fcntl.h>

int main(int argc,char* argv[]){

	//if the format for command line input is incorect, program terminates giving an error
	if(argc!=3) {
		printf("Incorrect format\nProvide the two filenames seperated by spaces\nExample ./a.out file1 file2\n");
		return 0;
	}

	//defining file1 and file2, file1 is the source and file2 is the destination
	int file1, file2;
	file1=open(argv[1], O_RDONLY );//read only
	file2=open(argv[2], O_CREAT | O_WRONLY ,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);//write only but creates file if non existant

	//if file open returns error, perror prints the specific error message and program terminates
	if(file1==-1||file2==-1) {
		perror("ERROR : ");
		return 0;
	}


	int fd1[2],fd2[2];

	//creating two pipes where index 0 is the source and index 1 is the destination
	if (pipe(fd1)==-1) exit(1);
	if (pipe(fd2)==-1) exit(1);

	//defining char buffers to be used later
	char buff[200],buff1[200],buff2[200];

	switch(fork()) {//making a child process
		//error in fork()
		case -1 : 
			printf("fork() error\n");
			exit(1);

		//child process
		case 0 : 
			while(1) {//continue taking inputs from pipe1 and storing in file2 till exit condition is met (EOF for file1 is reached)
				//closing unnecessary pipe openings in child process
				close(fd1[1]);
				close(fd2[0]);

				//reading 100 bytes from pipe1 
				int rd1=read(fd1[0],buff1,100);
				//writing the read number of bytes to file2			
				int wr1=write(file2,buff1,rd1);
				
				//if error encountered in writing to file2
				if(wr1==-1) {
					int wr3=write(fd2[1],"-1",2);
					perror("Error in writing to file2 : ");
					exit(1);
				}
				//EOF for file1 is reached
				else if (wr1<100 && wr1>=0) {
					printf("file copied successfully\n");
					int wr4=write(fd2[1],"0",1);//sending appropriate strings through pipe2
					exit(1);
				}
				//writing successful but EOF for file1 is not reached
				else {
					int wr5=write(fd2[1],"0",1);//sending appropriate strings through pipe2
				}

			}
			break;

		//parent process
		default :
			while(1) {  //continue providing inputs to pipe1 till exit condition is not met [(pipe2 returns "0" and 
						//less than 100 bytes sent through pipe1) or (error in writing to file2)]
				
				//closing unnecessary pipe openings in child process
				close(fd1[0]);
				close(fd2[1]);

				//reading 100 bytes from file1
				int rd2=read(file1,buff,100);
				//writing read number of bytes to pipe1
				int wr2=write(fd1[1],buff,rd2);

				//reading the string sent by child process through pipe2
				int rd3=read(fd2[0],buff2,100);

				//exit conditions
				if(rd3==2 || (rd3==1&& (rd2>=0&&rd2<100))) exit(1); 
			}
			break;
		}
		
	return 0;
}
