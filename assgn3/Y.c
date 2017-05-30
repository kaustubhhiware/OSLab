#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
/*
* Assignment 3 part 2
* written by @kaustubhhiware as a part of OS Lab
*/

/* Querying process to do this :
* * search by roll and display
* * update CG
*
* Multiple copies supported
* Y must wait for X to start, only then
*/

key_t keyx = 0; // agrees with key of X
key_t keys = 1;
key_t keyd = 2;
key_t keyn = 3;
key_t keyu = 4;
key_t keyw = 10;
#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)

typedef struct records_
{
    char first[21];
    char last[21];
    int roll;
    float cgpa;
} records;

void printerror(const char* printmsg, int argcount, int* argval)
{
    int i;
    for( i = 0; i < argcount; i++)
    {
        if(argval[i]==-1)
        {
            fprintf(stderr, "+--- %s : ",printmsg );
            perror("");
            exit(0);
        }
    }
}

void letxstart();
void printLine();
int shmx, shmstud, shmdelta, shmnum, shmusers, wrt;


int main(int argc, char* argv[])
{
    letxstart();
    printf("X started! Can support operations now\n");

    int *num;
    shmnum = shmget(keyn, sizeof(int), 0);
    printerror("Error in shmget num", 1, (int[]){ shmnum });
    num = (int *) shmat(shmnum, 0, 0);
    printerror("Error in shmat num", 1, (int[]){ *num });

    shmstud = shmget(keys, num[0]*sizeof(int), 0);
    shmdelta = shmget(keyd, sizeof(int), 0);
    shmusers = shmget(keyu, sizeof(int), 0);
    printerror("Error in shmget", 3, (int[]){ shmstud, shmdelta, shmnum });

    int *delta, *users;
    delta = (int *) shmat(shmdelta, 0, 0);
    users = (int *) shmat(shmusers, 0, 0);
    printerror("Error in shmat", 2, (int[]){ *delta, *users });
    records* students = (records *) shmat(shmstud, 0, 0);

    wrt = semget(keyw, 1, 0777|IPC_CREAT);
    printerror("Error in semget", 1, (int[]){ wrt });

    struct sembuf pop, vop;
    // initialize sembufs
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1 ;   // value removed from semaphore
    vop.sem_op = 1 ;    // value added to semaphore

    int opt = 0, r, found = 0, i;
    float cg;
    while(1)
    {
        letxstart();
        printf("\n");
        printf("+--------------------------------------------------+\n");
        printf("| Following operations are supported. What to do ? |\n");
        printf("| 0 |   Query memory by roll numbers               |\n");
        printf("| 1 |   Update CGPA of a student                   |\n");
        printf("| 2 |   Exit                                       |\n");
        printf("+--------------------------------------------------+\n");
        printf("+--- Enter choice : ");
        char *in;
        //scanf("%s", in);
        //sscanf(in, "%d", &opt);
        scanf("%d",&opt);
        if(opt==0)
        {
            printf("+--- You chose to query. Enter roll number : ");
            //scanf("%s", in);
            //sscanf(in, "%d", &r);
            scanf("%d",&r);

            users[0] += 1;
            if(users[0]==1)
            {
                P(wrt);
            }

            found = 0;
            for(i = 0; i < num[0]; i++)
            {
                if(students[i].roll==r)
                {
                    printf("+--- Student found!\n");
                    printLine();
                    printf("|  first name         |  last name          | Roll| CGPA |\n");
                    printLine();
                    printf("| %-20s| %-20s| %-4d| %-2.2f |\n",students[i].first, students[i].last, students[i].roll, students[i].cgpa );
                    printLine();

                    found = 1;
                    break;
                }
            }
            if(!found)
            {
                printf("+--- Student not found \n");
            }

            users[0] -= 1;
            if(users[0]==0)
            {
                V(wrt);
            }

        }
        else if(opt==1)
        {
            printf("+--- You chose to update CG. Enter roll number : ");
            //scanf("%s", in);
            //sscanf(in, "%d", &r);
            scanf("%d",&r);
            printf("+--- Enter updated CGPA : ");
            scanf("%f", &cg);

            P(wrt);

            found = 0;
            for(i = 0; i < num[0]; i++)
            {
                if(students[i].roll==r)
                {
                    students[i].cgpa = cg;

                    printf("+--- CGPA updated for roll# %-4d\n",r );
                    delta[0] = 1;
                    found = 1;
                    break;
                }
            }
            if(!found)
            {
                printf("+--- Student not found \n");
            }

            V(wrt);

        }
        else
        {
            printf("Exitting...\n");
            int e1 = shmdt(num);
            int e2 = shmdt(delta);
            int e3 = shmdt(users);
            int e4 = shmdt(students);
            printerror("Error in shmdt", 4, (int[]){ e1, e2, e3, e4 });
            exit(0);
        }
    }

}


void letxstart()
{
    int didxstart = shmget(keys, sizeof(int), 0);
    while(didxstart==-1)
    {
        printf("Wait for X to start\n");
        sleep(5);
        didxstart = shmget(keys, sizeof(int), 0);
    }
}


void printLine()
{
    printf("+---------------------+---------------------+-----+------+\n");
}
