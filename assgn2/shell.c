#include<stdlib.h>
#include<stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <dirent.h> // ls
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#define BUFSIZE 1000
#define ARGMAX 10
#define GREEN  "\x1b[92m"
#define BLUE  "\x1b[94m"
#define DEF  "\x1B[0m"
// colour guide - https://github.com/shiena/ansicolor/blob/master/README.md
/*
* Assignment 2 part 2 - shell.c
* 14CS30011 : Hiware Kaustubh Narendra
* Usage - make clean ; clear ; make ; ./Cshell
*/
/* implement basic shell functions
*/
/* Instructions to support :
*  cd <dir>                                 - Done cd
*  pwd                                      - Done pwd
*  mkdir <dir>                              - Done mkdir
*  rmdir <dir>                              - Done rmdir
*  ls (support ls -l)                       - Done ls , remains ls -l
*  cp <file1> <file2>                       - TODo
*  exit                                     - Done exit
*  execute any other function like ./a.out  - TODo
* support background execution - &          - TODo
* redirect input output >, <                - TODo
*  a.out | b.out - must support a| b| c     - TODo
* Additional - clear
* Additional - screenfetch
*/
extern int alphasort();
int exitflag = 0;
char cwd[BUFSIZE];
char* argval[ARGMAX]; // our local argc, argv
int argcount = 0;
void screenfetch();
void getInput();
int function_exit();
void function_pwd(char*,int);
void function_cd(char*);
void function_mkdir(char*);
void function_rmdir(char*);
void function_clear();
void function_ls();
void function_lsl();



int main()
{
    int i;
    screenfetch();
    function_pwd(cwd,0);

    while(exitflag==0)
    {
        printf("%s%s ~> ",DEF,cwd ); //print user prompt
        // scanf("%s",input); - fails due to spaces, tabs
        getInput();

        if(strcmp(argval[0],"exit")==0)
        {
            function_exit();
        }
        else if(strcmp(argval[0],"screenfetch")==0)
        {
           screenfetch();
        }
        else if(strcmp(argval[0],"pwd")==0)
        {
            function_pwd(cwd,1);
        }
        else if(strcmp(argval[0],"cd")==0)
        {
            char* path = argval[1];
            function_cd(path);
        }
        else if(strcmp(argval[0],"mkdir")==0)
        {
            char* foldername = argval[1];
            function_mkdir(foldername);
        }
        else if(strcmp(argval[0],"rmdir")==0)
        {
            char* foldername = argval[1];
            function_rmdir(foldername);
        }
        else if(strcmp(argval[0],"clear")==0)
        {
            function_clear();
        }
        else if(strcmp(argval[0],"ls")==0)
        {
            char* optional = argval[1];
            if(strcmp(optional,"-l")==0)
            {
                function_lsl();
            }
            else function_ls();
        }


    }

}


/*ls -l  lists date permissions etc*/
void function_lsl()
{
    printf("Hello sweetie\n");
}

/* list cwd contents*/
void function_ls()
{
    DIR *dir = opendir(cwd);
    int i=0;
    struct dirent **listr;
    int listn = scandir(".", &listr, 0, alphasort);
    if (listn >= 0)
    {
        printf("+--- Total %d objects in this directory\n",listn-2);
        for(i = 0; i < listn; i++ )
        {
            if(strcmp(listr[i]->d_name,".")==0 || strcmp(listr[i]->d_name,"..")==0)
            {
                continue;
            }
            else if(listr[i]-> d_type != DT_DIR)
            {
                printf("%s%s    ",BLUE, listr[i]->d_name);
            }
            else if(listr[i] -> d_type == DT_DIR)
            {
                printf("%s%s    ",GREEN, listr[i]->d_name);
            }
            if(i%10==0) printf("\n");
        }
        printf("\n");
        closedir (dir);
    }
    else
    {
        perror ("+--- Error in ls ");
    }

}


/* clear the screen*/
void function_clear()
{
    const char* blank = "\e[1;1H\e[2J";
    write(STDOUT_FILENO,blank,12);
}

/* remove folder */
void function_rmdir(char* name)
{
    int statrm = rmdir(name);
    if(statrm==-1)
    {
        perror("+--- Error in rmdir ");
    }
}


/* Make folder */
void function_mkdir(char* name)
{
    int stat = mkdir(name, 0777);// all appropriate permissions
    if(stat==-1)
    {
        perror("+--- Error in mkdir ");
    }
}


/*change directory functionality*/
void function_cd(char* path)
{
    int ret = chdir(path);
    if(ret==0) // path could be changed if cd successful
    {
        function_pwd(cwd,0);
    }
    else perror("+--- Error in cd ");
}


/*get input containing spaces and tabs and store it in argval*/
void getInput()
{
    char* input = NULL;
    ssize_t buf = 0;
    getline(&input,&buf,stdin);

    argcount = 0;
    while((argval[argcount] = strsep(&input, " \t\n")) != NULL && argcount < ARGMAX-1)
    {
        argcount++;
    }
    free(input);
}


/*Implement basic exit*/
int function_exit()
{
    exitflag = 1;
    return 0; // return 0 to parent process in run.c
}

/* Implement pwd function in shell - 1 prints, 0 stores*/
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
