/*-
 * Copyright (c) 2014 Matthew Macy <mmacy@netxbsd.org>
 * Copyright (c) 2002-2003 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Emmanuel Dreyfus
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
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
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/signal.h>
#include <sys/proc.h>
#include <sys/time.h>

#include <sys/mach/mach_types.h>
#include <sys/mach/message.h>
#include <sys/mach/mach.h>
#include <sys/mach/ipc/ipc_mqueue.h>
#include <sys/mach/thread.h>

#include <sys/mach/clock_types.h>
#include <sys/mach/clock_server.h>

#define timespecsub_netbsd(tsp, usp, vsp)                               \
        do {                                                            \
                (vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;          \
                (vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;       \
                if ((vsp)->tv_nsec < 0) {                               \
                        (vsp)->tv_sec--;                                \
                        (vsp)->tv_nsec += 1000000000L;                  \
                }                                                       \
        } while (/* CONSTCOND */ 0)



kern_return_t
clock_sleep(mach_port_name_t clock_name, mach_sleep_type_t type, int sleep_sec, int sleep_nsec, mach_timespec_t *wakeup_time)
{
	struct timespec mts, cts, tts;
	mach_timespec_t mcts;
	int error;
	int ticks;
	thread_t thread;

	mts.tv_sec = sleep_sec;
	mts.tv_nsec = sleep_nsec;

	if (type == TIME_ABSOLUTE) {
		nanotime(&cts);
		timespecsub_netbsd(&mts, &cts, &tts);
	} else {
		tts.tv_sec = mts.tv_sec;
		tts.tv_nsec = mts.tv_nsec;
	}

	ticks = tts.tv_sec * hz;
	ticks += (tts.tv_nsec * hz) / 100000000L;

	if (ticks <= 0)
		return (EINVAL);

	thread = current_thread();
	thread->ith_block_lock_data = &curproc->p_mtx;
	thread->timeout = ticks;
	PROC_LOCK(curproc);
	thread_block();
	PROC_UNLOCK(curproc);

	if (wakeup_time != NULL) {
		nanotime(&cts);
		mcts.tv_sec = cts.tv_sec;
		mcts.tv_nsec = cts.tv_nsec;
		error = copyout(&mcts, wakeup_time, sizeof(mcts));
		if (error != 0)
			return (error);
	}

	return (0);
}

int
mach_timebase_info(mach_timebase_info_t infop)
{
	/* {
		syscallarg(mach_timebase_info_t) info;
	} */
	int error;
	struct mach_timebase_info info;

	/* XXX This is probably bus speed, fill it accurately */
	info.numer = 4000000000UL;
	info.denom = 75189611UL;

	if ((error = copyout(&info, (void *)infop,
	    sizeof(info))) != 0)
		return (error);

	return (0);
}

int
clock_get_time(clock_serv_t clock_serv, mach_timespec_t *cur_time)
{
	struct timespec ts;

	nanotime(&ts);

	return (copyout(&ts, cur_time, sizeof(ts)));
}

int
clock_get_attributes(
	clock_serv_t clock_serv,
	clock_flavor_t flavor,
	clock_attr_t clock_attr,
	mach_msg_type_number_t *clock_attrCnt
)
UNSUPPORTED;

int
clock_alarm(
	clock_serv_t clock_serv,
	alarm_type_t alarm_type,
	mach_timespec_t alarm_time,
	clock_reply_t alarm_port,
	mach_msg_type_name_t alarm_portPoly
)
UNSUPPORTED;	
