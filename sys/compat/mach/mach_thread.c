/*-
 * Copyright (c) 2014-2015, Matthew Macy <mmacy@nextbsd.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Neither the name of Matthew Macy nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/eventhandler.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/queue.h>
#include <sys/resource.h>
#include <sys/resourcevar.h>
#include <sys/rwlock.h>
#include <sys/sched.h>
#include <sys/sleepqueue.h>
#include <sys/signal.h>

#include <sys/mach/mach_types.h>
#include <sys/mach/mach_traps.h>

#include <sys/mach/ipc/ipc_kmsg.h>
#include <sys/mach/thread.h>
#include <sys/mach/ipc_tt.h>
#include <sys/mach/thread_switch.h>

#define MT_SETRUNNABLE 0x1

#ifdef notyet
/*
 * Am assuming that Mach lacks the concept of uninterruptible
 * sleep - this may need to be changed back to what is in pci_pass.c
 */
static int
_intr_tdsigwakeup(struct thread *td, int intrval)
{
	struct proc *p = td->td_proc;
	int rc = 0;

	PROC_SLOCK(p);
	thread_lock(td);
	if (TD_ON_SLEEPQ(td)) {
		/*
		 * Give low priority threads a better chance to run.
		 */
		if (td->td_priority > PUSER)
			sched_prio(td, PUSER);

		sleepq_abort(td, intrval);
		rc = 1;
	}
	PROC_SUNLOCK(p);
	thread_unlock(td);
	return (rc);
}
#endif


int
mach_thread_switch(mach_port_name_t thread_name, int option, mach_msg_timeout_t option_time)
{
       int timeout;
       struct mach_emuldata *med;
	   struct thread *td = curthread;

       med = (struct mach_emuldata *)td->td_proc->p_emuldata;
       timeout = option_time * hz / 1000;

       /*
        * The day we will be able to find out the struct proc from
        * the port number, try to use preempt() to call the right thread.
        * [- but preempt() is for _involuntary_ context switches.]
        */
       switch(option) {
       case SWITCH_OPTION_NONE:
               sched_relinquish(curthread);
               break;

       case SWITCH_OPTION_WAIT:
#ifdef notyet		   
               med->med_thpri = 1;
               while (med->med_thpri != 0) {
                       rw_wlock(&med->med_rightlock);
                       (void)msleep(&med->med_thpri, &med->med_rightlock, PZERO|PCATCH,
                                                "thread_switch", timeout);
                       rw_wunlock(&med->med_rightlock);
              }
               break;
#endif
       case SWITCH_OPTION_DEPRESS:
               /* Use a callout to restore the priority after depression? */
               td->td_priority = PRI_MAX_TIMESHARE;
               break;

       default:
              uprintf("sys_mach_syscall_thread_switch(): unknown option %d\n", option);
               break;
       }
       return (0);
}

void
thread_go(thread_t thread)
{
	int needunlock = 0;
	struct mtx *block_lock = thread->ith_block_lock_data;

	MPASS(thread->ith_state != MACH_SEND_IN_PROGRESS &&
		  thread->ith_state != MACH_RCV_IN_PROGRESS &&
		  thread->ith_state != MACH_RCV_IN_PROGRESS_TIMED);

	if (block_lock != NULL && !mtx_owned(block_lock)) {
		needunlock = 1;
		mtx_lock(block_lock);
	}
	wakeup(thread);
	if (needunlock)
		mtx_unlock(block_lock);
}

void
thread_block(void)
{
	thread_t thread = current_thread();
	int rc;

	MPASS(curthread == thread->ith_td);

	rc = msleep(thread, thread->ith_block_lock_data, PCATCH|PSOCK, "thread_block", thread->timeout);
	switch (rc) {
	case EINTR:
	case ERESTART:
		thread->wait_result = THREAD_INTERRUPTED;
		break;
	case EWOULDBLOCK:
		thread->wait_result = THREAD_TIMED_OUT;
		break;
	case 0:
		thread->wait_result = THREAD_AWAKENED;
		break;
	default:
		panic("unexpected return from msleep: %d\n", rc);
	}
#ifdef INVARIANTS
	if (thread->timeout == 0) {
		if(rc == 0)
			MPASS(thread->ith_state == MACH_MSG_SUCCESS);
		else
			MPASS(rc == EINTR || rc == ERESTART);
	}
#endif
}

void
thread_will_wait_with_timeout(thread_t thread, int timeout)
{

	thread->sleep_stamp = ticks;
	thread->timeout = timeout;
}


void
thread_will_wait(thread_t thread)
{

	thread->sleep_stamp = ticks;
	thread->timeout = 0;
}

static void
mach_thread_create(struct thread *td, thread_t thread)
{

	thread->ref_count = 1;
	ipc_thread_init(thread);
}


static uma_zone_t thread_shuttle_zone;

static int
uma_thread_init(void *_thread, int a, int b)
{
	/* allocate thread substructures */
	return (0);
}

static void
uma_thread_fini(void *_thread, int a)
{
	/* deallocate thread substructures */
}

static void
mach_thread_init(void *arg __unused, struct thread *td)
{
	thread_t thread;

	thread = uma_zalloc(thread_shuttle_zone, M_WAITOK|M_ZERO);
	mtx_init(&thread->ith_lock_data, "mach_thread lock", NULL, MTX_DEF);

	MPASS(td->td_machdata == NULL);
	td->td_machdata = thread;
	thread->ith_td = td;
	ipc_thr_act_init(thread);
}

static void
mach_thread_fini(void *arg __unused, struct thread *td)
{
	thread_t thread = td->td_machdata;

	MPASS(thread->ith_kmsg == NULL);
	MPASS(thread->ith_td == td);
	ipc_thr_act_terminate(thread);
	mtx_destroy(&thread->ith_lock_data);
	uma_zfree(thread_shuttle_zone, thread);
}

static void
mach_thread_ctor(void *arg __unused, struct thread *td)
{
	thread_t thread = td->td_machdata;

	MPASS(thread->ith_td == td);
	mach_thread_create(td, thread);
	thread->ith_block_lock_data = NULL;
}

static void
thread_sysinit(void *arg __unused)
{
	thread_shuttle_zone = uma_zcreate("thread_shuttle_zone",
									  sizeof(struct thread_shuttle),
									  NULL, NULL, uma_thread_init,
									  uma_thread_fini, 1, 0);

	EVENTHANDLER_REGISTER(thread_ctor, mach_thread_ctor, NULL, EVENTHANDLER_PRI_ANY);
	EVENTHANDLER_REGISTER(thread_init, mach_thread_init, NULL, EVENTHANDLER_PRI_ANY);
	EVENTHANDLER_REGISTER(thread_fini, mach_thread_fini, NULL, EVENTHANDLER_PRI_ANY);
}

/* before SI_SUB_INTRINSIC and after SI_SUB_EVENTHANDLER */
SYSINIT(mach_thread, SI_SUB_KLD, SI_ORDER_ANY, thread_sysinit, NULL);
