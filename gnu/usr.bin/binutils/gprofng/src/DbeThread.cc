/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "DbeThread.h"
#include "util.h"
#include "vec.h"

static void
cleanup_free_mutex (void* arg) {
  //  pthread_mutex_t *p_mutex = (pthread_mutex_t *) arg;
  //  if (p_mutex)
  //    pthread_mutex_unlock (p_mutex);
}

static void*
thread_pool_loop (void* arg)
{
  DbeThreadPool *thrp = (DbeThreadPool*) arg;
  Dprintf (DEBUG_THREADS, "thread_pool_loop:%d starting thread=%llu\n",
	   __LINE__, (unsigned long long) pthread_self ());

  /* set my cancel state to 'enabled', and cancel type to 'defered'. */
  pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype (PTHREAD_CANCEL_DEFERRED, NULL);

  /* set thread cleanup handler */
  pthread_cleanup_push (cleanup_free_mutex, (void*) & (thrp->p_mutex));
  for (;;)
    {
      DbeQueue *q = thrp->get_queue ();
      if (q)
	{ /* a request is pending */
	  Dprintf (DEBUG_THREADS,
		   "thread_pool_loop:%d thread=%llu queue=%d start\n",
		   __LINE__, (unsigned long long) pthread_self (), q->id);
	  q->func (q->arg);
	  Dprintf (DEBUG_THREADS,
		   "thread_pool_loop:%d thread=%llu queue=%d done\n",
		   __LINE__, (unsigned long long) pthread_self (), q->id);
	  delete q;
	  continue;
	}
      if (thrp->no_new_queues)
	{
	  Dprintf (DEBUG_THREADS, "thread_pool_loop:%d exit thread=%llu\n",
		   __LINE__, (unsigned long long) pthread_self ());
	  pthread_exit (NULL);
	}
      Dprintf (DEBUG_THREADS,
	       "thread_pool_loop:%d before pthread_cond_wait thread=%llu\n",
	       __LINE__, (unsigned long long) pthread_self ());
      pthread_mutex_lock (&thrp->p_mutex);
      pthread_cond_wait (&thrp->p_cond_var, &thrp->p_mutex);
      Dprintf (DEBUG_THREADS,
	       "thread_pool_loop:%d after pthread_cond_wait thread=%llu\n",
	       __LINE__, (unsigned long long) pthread_self ());
      pthread_mutex_unlock (&thrp->p_mutex);
    }

  // never reached, but we must use it here. See `man pthread_cleanup_push`
  pthread_cleanup_pop (0);
}

DbeThreadPool::DbeThreadPool (int _max_threads)
{
  static const int DBE_NTHREADS_DEFAULT = 4;
  char *s = getenv ("GPROFNG_DBE_NTHREADS");
  if (s)
    {
      max_threads = atoi (s);
      if (max_threads < 0)
	max_threads = 0;
      if (_max_threads > 0 && max_threads < _max_threads)
	max_threads = _max_threads;
    }
  else
    {
      max_threads = _max_threads;
      if (max_threads < 0)
	max_threads = DBE_NTHREADS_DEFAULT;
    }
  Dprintf (DEBUG_THREADS, "DbeThreadPool:%d  max_threads %d ---> %d\n",
	   __LINE__, _max_threads, max_threads);
  pthread_mutex_init (&p_mutex, NULL);
  pthread_cond_init (&p_cond_var, NULL);
  threads = new Vector <pthread_t>(max_threads);
  queue = NULL;
  last_queue = NULL;
  no_new_queues = false;
  queues_cnt = 0;
  total_queues = 0;
}

DbeThreadPool::~DbeThreadPool ()
{
  delete threads;
}

DbeQueue *
DbeThreadPool::get_queue ()
{
  pthread_mutex_lock (&p_mutex);
  DbeQueue *q = queue;
  Dprintf (DEBUG_THREADS,
   "get_queue:%d thr: %lld id=%d queues_cnt=%d threads_cnt=%d max_threads=%d\n",
	   __LINE__, (unsigned long long) pthread_self (),
	   q ? q->id : -1, queues_cnt, (int) threads->size (), max_threads);
  if (q)
    {
      queue = q->next;
      queues_cnt--;
    }
  pthread_mutex_unlock (&p_mutex);
  return q;
}

void
DbeThreadPool::put_queue (DbeQueue *q)
{
  if (max_threads == 0)
    {
      // nothing runs in parallel
      q->id = ++total_queues;
      Dprintf (DEBUG_THREADS, NTXT ("put_queue:%d thr=%lld max_threads=%d queue (%d) runs on the worked thread\n"),
	       __LINE__, (unsigned long long) pthread_self (), max_threads, q->id);
      q->func (q->arg);
      delete q;
      return;
    }

  pthread_mutex_lock (&p_mutex);
  // nothing runs in parallel
  q->id = ++total_queues;
  Dprintf (DEBUG_THREADS, "put_queue:%d thr=%lld max_threads=%d queue (%d)\n",
	   __LINE__, (unsigned long long) pthread_self (), max_threads, q->id);
  if (queue)
    {
      last_queue->next = q;
      last_queue = q;
    }
  else
    {
      queue = q;
      last_queue = q;
    }
  queues_cnt++;
  Dprintf (DEBUG_THREADS,
	   "put_queue:%d id=%d queues_cnt=%d threads_cnt=%d max_threads=%d\n",
	   __LINE__, q->id, queues_cnt, (int) threads->size (), max_threads);
  if (queues_cnt > threads->size () && threads->size () < max_threads)
    {
      pthread_t thr;
      int r = pthread_create (&thr, NULL, thread_pool_loop, (void *) this);
      Dprintf (DEBUG_THREADS,
	       "put_queue:%d pthread_create returns %d thr=%llu\n",
	       __LINE__, r, (unsigned long long) thr);
      if (r)
	fprintf (stderr, GTXT ("pthread_create failed. errnum=%d (%s)\n"), r,
		 STR (strerror (r)));
      else
	threads->append (thr);
    }
  pthread_cond_signal (&p_cond_var);
  pthread_mutex_unlock (&p_mutex);
}

void
DbeThreadPool::wait_queues ()
{
  pthread_mutex_lock (&p_mutex);
  no_new_queues = true;
  pthread_mutex_unlock (&p_mutex);
  pthread_cond_broadcast (&p_cond_var);
  for (;;) // Run requests on the worked thread too
    {
      DbeQueue *q = get_queue ();
      if (q == NULL)
	break;
      Dprintf (DEBUG_THREADS, "wait_queues:%d thread=%llu queue=%d start\n",
	       __LINE__, (unsigned long long) pthread_self (), q->id);
      q->func (q->arg);
      Dprintf (DEBUG_THREADS, "wait_queues:%d thread=%llu queue=%d done\n",
	       __LINE__, (unsigned long long) pthread_self (), q->id);
      delete q;
    }
  for (int i = 0, sz = threads->size (); i < sz; i++)
    {
      void *retval;
      pthread_join (threads->get (i), &retval);
    }
}

DbeQueue::DbeQueue (int (*_func) (void *arg), void *_arg)
{
  func = _func;
  arg = _arg;
  next = NULL;
}

DbeQueue::~DbeQueue () { }
