#include<stdlib.h>
#include<stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#define BUFSIZE 1000
/*
* Assignment 2 part 2 - shell.c
* 14CS30011 : Hiware Kaustubh Narendra
*/
/* implement basic shell functions
*/
/* Instructions to support :
*  cd <dir>
*  pwd                                      - Done pwd
*  mkdir <dir>
*  rmdir <dir>
*  ls (support ls -l)
*  cp <file1> <file2>
*  exit                                     - Done exit
*  execute any other function like ./a.out
* support background execution - &
* redirect input output >, <
*  a.out | b.out - must support a| b| c
*/

int exitflag = 0;
char cwd[BUFSIZE];
void screenfetch();
void function_pwd(char*,int);
int function_exit();

int main()
{
    screenfetch();
    function_pwd(cwd,0);
    char input[BUFSIZE];
    while(exitflag==0)
    {
        printf("%s ~> ",cwd ); //print user prompt
        scanf("%s",input);

        if(strcmp(input,"exit")==0)
        {
            function_exit();
        }
        else if(strcmp(input,"pwd")==0)
        {
            function_pwd(cwd,1);
        }
    }
}

/*Implement basic exit*/
int function_exit()
{
    exitflag = 1;
    return 0; // return 0 to parent process in run.c
}

/* Implement pwd function in shell */
void function_pwd(char* cwdstr,int command)
{
    char temp[BUFSIZE];
    char* path=getcwd(temp, sizeof(temp));
    if(path != NULL)
    {
        strcpy(cwdstr,temp);
        if(command==1)
        {
            printf("%s\n",cwdstr);
        }
    }
    else perror("+--- Error in getcwd() : ");

}

/* mimic screenfetch like logo functionality from ubuntu*/
void screenfetch()
{
/*    char* welcomestr = "\n                           ./+o+-\n                  yyyyy- -yyyyyy+\n
* ://+//////-yyyyyyo\n           .++ .:/++++++/-.+sss/`\n         .:++o:  /++++++++/:--:/-\n
* o:+o+:++.`..```.-/oo+++++/\n       .:+o:+o/.          `+sssoo+/\n  .++/+:+oo+o:`             /sssooo.\n
* /+++//+:`oo+o               /::--:.\n \\+/+o+++`o++o               ++////.\n  .++.o+++oo+:`
* /dddhhh.\n       .+.o+oo:.          `oddhhhh+\n        \\+.++o+o``-````.:ohdhhhhh+\n         `:o+++
* `ohhhhhhhhyo++os:\n           .o:`.syhhhhhhh/.oo++o`\n               /osyyyyyyo++ooo+++/\n
* ````` +oo+++o\\:\n                          `oo++.\n\n";
*/// Source - http://ascii.co.uk/art/seashell
    char* welcomestr = "           _.-''|''-._\n        .-'     |     `-.\n      .'\\       |       /`.\n    .'   \\      |      /   `.\n    \\     \\     |     /     /\n     `\\    \\    |    /    /'\n       `\\   \\   |   /   /'\n         `\\  \\  |  /  /'\n        _.-`\\ \\ | / /'-._         Cshell\n       {_____`\\\\|//'_____}        Made by Kaustubh Hiware\n               `-'\n\n";
    printf("%s",welcomestr);
}
