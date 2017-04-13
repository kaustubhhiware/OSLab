#include <debug.h>
#include <list.h>
#include <stdint.h>

struct thread;

enum sigval {
    SIG_CHLD = 0,
    SIG_USER = 1,
    SIG_CPU = 2,
    SIG_UBLOCK = 3,
    SIG_KILL = 4,
    SIG_COUNT = 5
};

#define SIG_BLOCK  0
#define SIG_UNBLOCK  1
#define SIG_SETMASK  2
#define NUM_SIGNAL  4


enum sighandler_t {
    SIG_DFL,
    SIG_IGN
};
struct signal_t {
	int type;
	int sent_by;
	struct list_elem threadelem;
};
typedef unsigned short sigset_t;


enum sighandler_t Signal(int signum, enum sighandler_t handler);
int kill(int pid, int sig);

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);

int sigemptyset(sigset_t *set);
int sigfillset(sigset_t *set);
int sigaddset(sigset_t *set, int signum);
int sigdelset(sigset_t *set, int signum);

void SIG_KILL_DFL(int by);
void SIG_USER_DFL(int by);
void SIG_CPU_DFL(int by);
void SIG_CHLD_DFL(int by);
