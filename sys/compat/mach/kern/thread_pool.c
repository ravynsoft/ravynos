/*
 * Copyright 1991-1998 by Open Software Foundation, Inc. 
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */
/*
 * MkLinux
 */
/*
 * Copyright (c) 1993 The University of Utah and
 * the Computer Systems Laboratory (CSL).  All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * THE UNIVERSITY OF UTAH AND CSL ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS
 * IS" CONDITION.  THE UNIVERSITY OF UTAH AND CSL DISCLAIM ANY LIABILITY OF
 * ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * CSL requests users of this software to return to csl-dist@cs.utah.edu any
 * improvements that they make and grant CSL redistribution rights.
 *
 *      Author: Bryan Ford, University of Utah CSL
 *
 *	File:	thread_pool.c
 *
 *	thread_pool management routines
 *
 */


#include <sys/mach/mach_types.h>
#include <sys/mach/ipc_tt.h>

#include <sys/mach/kern_return.h>
#include <sys/mach/thread_pool.h>
#include <sys/mach/thread.h>



#define THR_ACT_NULL NULL
/* Initialize a new EMPTY thread_pool.  */
kern_return_t
thread_pool_init(thread_pool_t new_thread_pool)
{
	assert(new_thread_pool != THREAD_POOL_NULL);

	/* Start with one reference for the caller */
	new_thread_pool->thr_acts = (struct thread_shuttle *)0;
	return KERN_SUCCESS;
}

void
thread_pool_remove(thread_act_t thread)
{
	thread_pool_t pool = &((struct rpc_common_data *)thread->ith_object)->rcd_thread_pool;
	thread_act_t act = pool->thr_acts;
	int found = 0;

	mtx_assert(thread->ith_block_lock_data, MA_OWNED);
	if (act == thread) {
		pool->thr_acts = pool->thr_acts->ith_pool_next;
		return;
	}
	
	while (act != NULL) {
		if (act->ith_pool_next == thread) {
			act->ith_pool_next = thread->ith_pool_next;
			found = 1;
			break;
		}
		act = act->ith_pool_next;
	}
	assert(found);
}


/*
 * Obtain an activation from a thread pool, blocking if
 * necessary.  Return the activation locked, since it's
 * in an inconsistent state (not in a pool, not attached
 * to a thread).
 *
 * Called with io_lock() held for ith_object.  Returns
 * the same way.
 *
 * If the thread pool port is destroyed while we are blocked,
 * then return a null activation. Callers must check for this
 * error case.
 */
thread_act_t
thread_pool_get_act(ipc_object_t object, int block)
{
	thread_pool_t thread_pool = &((struct rpc_common_data *)object)->rcd_thread_pool;
	thread_act_t thr_act;

#if	MACH_ASSERT
	assert(thread_pool != THREAD_POOL_NULL);
	if (watchacts & WA_ACT_LNK)
		printf("thread_pool_block: %x, waiting=%d\n",
		       thread_pool, thread_pool->waiting);
#endif
	if (block == 0) {
		if ((thr_act = thread_pool->thr_acts) != THR_ACT_NULL) {
			thread_pool->thr_acts = thr_act->ith_pool_next;
			return (thr_act);
		} else
			return (NULL);
	}

	while ((thr_act = thread_pool->thr_acts) == THR_ACT_NULL) {
#if 0
		if (!ip_active(pool_port))
			return THR_ACT_NULL;
#endif
		thread_pool->waiting = 1;
		assert_wait((event_t)thread_pool, FALSE);
		io_unlock(object);
		thread_block();       /* block self */
		io_lock(object);
	}
	thread_pool->thr_acts = thr_act->ith_pool_next;
	thr_act->ith_pool_next = NULL;

#if	MACH_ASSERT
	if (watchacts & WA_ACT_LNK)
		printf("thread_pool_block: return %x, next=%x\n",
		       thr_act, thread_pool->thr_acts);
#endif
	return thr_act;
}

/*
 * 	thread_pool_put_act
 *
 *	Return an activation to its pool. Assumes the activation
 *	and pool (if it exists) are locked.
 */
void
thread_pool_put_act( thread_act_t thr_act )
{
	thread_pool_t   thr_pool;

	mtx_assert(thr_act->ith_block_lock_data, MA_OWNED);
		/*
	 *	Find the thread pool for this activation.
	 */	
        if (thr_act->ith_object)
            thr_pool = &((struct rpc_common_data *)thr_act->ith_object)->rcd_thread_pool;
        else
            thr_pool = THREAD_POOL_NULL;

        /* 
	 *	Return act to the thread_pool's list, if it is still
	 *	alive. Otherwise, remove it from its thread_pool, which
	 *	will deallocate it and destroy it.
	 */
        if (thr_act->ith_active) {
                assert(thr_pool);
                thr_act->ith_pool_next = thr_pool->thr_acts;
                thr_pool->thr_acts = thr_act;
                if (thr_pool->waiting)
					thread_pool_wakeup(thr_pool);
        } else if (thr_pool) {
                assert(thr_act->ith_object);
        }
}
 

/*
 * Called with ip_lock() held for port containing thread_pool.
 * Returns same way.
 */
void
thread_pool_wakeup(thread_pool_t thread_pool)
{
#if	MACH_ASSERT
	assert(thread_pool != THREAD_POOL_NULL);
	if (watchacts & WA_ACT_LNK)
		printf("thread_pool_wakeup: %x, waiting=%d, head=%x\n",
		   thread_pool, thread_pool->waiting, thread_pool->thr_acts);
#endif	/* MACH_ASSERT */

	if (thread_pool->waiting) {
#if 0
		thread_wakeup((event_t)thread_pool);
#endif
		thread_pool->waiting = 0;
	}
}
