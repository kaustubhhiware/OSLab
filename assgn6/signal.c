#include "threads/thread.h"
#include <debug.h>
#include <stddef.h>
#include <random.h>
#include <stdio.h>
#include <string.h>
#include "threads/flags.h"
#include "threads/interrupt.h"
#include "threads/intr-stubs.h"
#include "threads/palloc.h"
#include "threads/switch.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#ifdef USERPROG
#include "userprog/process.h"
#endif

enum sighandler_t Signal(int signum, enum sighandler_t handler)
{
	if (signum == SIG_KILL) return 0;
	enum intr_level old_level;
	old_level = intr_disable ();

	struct thread * cur = thread_current();
	enum sighandler_t prev_handler;

    if((cur->mask >> signum) & 1)
        prev_handler = SIG_IGN;
    else
        prev_handler = SIG_DFL;

	if (prev_handler != handler)
    {
        // flip only the signum bit
		cur->mask ^= (1 << signum);
	}

	intr_set_level (old_level);
	return prev_handler;
}


int kill(int tid, int sig)
{
	if (sig == SIG_CHLD || sig == SIG_CPU)
        return -1;
    if( tid <= 2)
        return -1;
	enum intr_level old_level;
	old_level = intr_disable ();

	struct thread * x = get_thread(tid);

	if (x == NULL) // if no valid thread found
    {
        intr_set_level (old_level);
        return -1;
    }

    int sig_mask = (x->mask >> sig) & 1;
	if (sig != SIG_KILL && sig_mask)
    {
        intr_set_level (old_level);
        return 0;
    }
    else if (sig == SIG_UBLOCK)
    {
		if (x->status == THREAD_BLOCKED)
        {
			list_push_back(&wait_unblock_list, &x->elem1);
		}
		intr_set_level (old_level);
		return 0;
	}
    else if (sig == SIG_KILL)
    {
		if (x->parent_tid != running_thread()->tid)
        {
            intr_set_level(old_level);
            return -1;
        }
	}

	if (x->signals[sig].type != -1)
    {
		x->signals[sig].sent_by = running_thread()->tid;
		intr_set_level (old_level);
		return 0;
	}
	x->signals[sig].type = sig;
	x->signals[sig].sent_by = running_thread()->tid;
	list_push_back(&x->signal_list, &x->signals[sig].threadelem);
	intr_set_level (old_level);
	return 0;
}

// 0 - SIGBLOCK 1 - SIG_UNBLOCK 2 - SIG_SETMASK
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
	if (*set >= (1 << NUM_SIGNAL))
        return -1;
	enum intr_level old_level;
	old_level = intr_disable ();

	struct thread * cur = running_thread();
	if (oldset) *oldset = cur->mask;
	if (set == NULL)
    {
		intr_set_level (old_level);
		return 0;
	}

    switch (how)
    {
        case SIG_BLOCK:    cur->mask |= (*set); break;
        case SIG_UNBLOCK:  cur->mask &= ( ( (((sigset_t)1) << NUM_SIGNAL) - 1) ^ (*set) );
                           break;
        case SIG_SETMASK:  cur->mask = (*set); break;
        default:           intr_set_level (old_level); return -1;
	}

	intr_set_level (old_level);
	return 0;
}

int sigemptyset(sigset_t *set)
{
	if (!set)
    {
		return -1;
	}
	enum intr_level old_level;
	old_level = intr_disable ();

	*set = 0;
	intr_set_level (old_level);
	return 0;
}

int sigfillset(sigset_t *set)
{
	if (!set)
    {
		return -1;
	}
	enum intr_level old_level;
	old_level = intr_disable ();

	*set = ( ((sigset_t)1) << NUM_SIGNAL)-1;
	intr_set_level (old_level);
	return 0;
}

int sigaddset(sigset_t *set, int signum)
{
	if (signum >= NUM_SIGNAL || !set)
    {
		return -1;
	}
	enum intr_level old_level;
	old_level = intr_disable ();

	*set |= ( ((sigset_t)1) << signum);
	intr_set_level (old_level);
	return 0;
}

int sigdelset(sigset_t *set, int signum)
{
	if(signum >= NUM_SIGNAL || !set)
    {
		return -1;
	}
	enum intr_level old_level;
	old_level = intr_disable ();

	*set &= ~(((sigset_t)1) << signum);
	intr_set_level (old_level);
	return 0;
}

void SIG_KILL_DFL(int by)
{
	printf("Thread #%d killed by Thread #%d\n", running_thread()->tid, by);
	thread_exit();
}

void SIG_USER_DFL(int by)
{
	printf("Thread #%d sent SIG_USER to Thread #%d\n", by, running_thread()->tid);
}

void SIG_CPU_DFL(int by UNUSED)
{
	printf("Lifetime of Thread #%d = %lld\n", running_thread()->tid, running_thread()->lifetime);
	thread_exit();
}

void SIG_CHLD_DFL(int by UNUSED)
{
	running_thread()->children_alive--;
	printf("Thread #%d: %d children created, %d alive\n", running_thread()->tid, running_thread()->children_created, running_thread()->children_alive);
}
