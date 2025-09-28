// workqueue.cc -- the workqueue for gold

// Copyright (C) 2006-2023 Free Software Foundation, Inc.
// Written by Ian Lance Taylor <iant@google.com>.

// This file is part of gold.

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
// MA 02110-1301, USA.

#include "gold.h"

#include "debug.h"
#include "options.h"
#include "timer.h"
#include "workqueue.h"
#include "workqueue-internal.h"

namespace gold
{

// Class Task_list.

// Add T to the end of the list.

inline void
Task_list::push_back(Task* t)
{
  gold_assert(t->list_next() == NULL);
  if (this->head_ == NULL)
    {
      this->head_ = t;
      this->tail_ = t;
    }
  else
    {
      this->tail_->set_list_next(t);
      this->tail_ = t;
    }
}

// Add T to the front of the list.

inline void
Task_list::push_front(Task* t)
{
  gold_assert(t->list_next() == NULL);
  if (this->head_ == NULL)
    {
      this->head_ = t;
      this->tail_ = t;
    }
  else
    {
      t->set_list_next(this->head_);
      this->head_ = t;
    }
}

// Remove and return the first Task waiting for this lock to be
// released.

inline Task*
Task_list::pop_front()
{
  Task* ret = this->head_;
  if (ret != NULL)
    {
      if (ret == this->tail_)
	{
	  gold_assert(ret->list_next() == NULL);
	  this->head_ = NULL;
	  this->tail_ = NULL;
	}
      else
	{
	  this->head_ = ret->list_next();
	  gold_assert(this->head_ != NULL);
	  ret->clear_list_next();
	}
    }
  return ret;
}

// The simple single-threaded implementation of Workqueue_threader.

class Workqueue_threader_single : public Workqueue_threader
{
 public:
  Workqueue_threader_single(Workqueue* workqueue)
    : Workqueue_threader(workqueue)
  { }
  ~Workqueue_threader_single()
  { }

  void
  set_thread_count(int thread_count)
  { gold_assert(thread_count > 0); }

  bool
  should_cancel_thread(int)
  { return false; }
};

// Workqueue methods.

Workqueue::Workqueue(const General_options& options)
  : lock_(),
    first_tasks_(),
    tasks_(),
    running_(0),
    waiting_(0),
    condvar_(this->lock_),
    threader_(NULL)
{
  bool threads = options.threads();
#ifndef ENABLE_THREADS
  threads = false;
#endif
  if (!threads)
    this->threader_ = new Workqueue_threader_single(this);
  else
    {
#ifdef ENABLE_THREADS
      this->threader_ = new Workqueue_threader_threadpool(this);
#else
      gold_unreachable();
#endif
    }
}

Workqueue::~Workqueue()
{
}

// Add a task to the end of a specific queue, or put it on the list
// waiting for a Token.

void
Workqueue::add_to_queue(Task_list* queue, Task* t, bool front)
{
  Hold_lock hl(this->lock_);

  Task_token* token = t->is_runnable();
  if (token != NULL)
    {
      if (front)
	token->add_waiting_front(t);
      else
	token->add_waiting(t);
      ++this->waiting_;
    }
  else
    {
      if (front)
	queue->push_front(t);
      else
	queue->push_back(t);
      // Tell any waiting thread that there is work to do.
      this->condvar_.signal();
    }
}

// Add a task to the queue.

void
Workqueue::queue(Task* t)
{
  this->add_to_queue(&this->tasks_, t, false);
}

// Queue a task which should run soon.

void
Workqueue::queue_soon(Task* t)
{
  t->set_should_run_soon();
  this->add_to_queue(&this->first_tasks_, t, false);
}

// Queue a task which should run next.

void
Workqueue::queue_next(Task* t)
{
  t->set_should_run_soon();
  this->add_to_queue(&this->first_tasks_, t, true);
}

// Return whether to cancel the current thread.

inline bool
Workqueue::should_cancel_thread(int thread_number)
{
  return this->threader_->should_cancel_thread(thread_number);
}

// Find a runnable task in TASKS.  Return NULL if none could be found.
// If we find a Task waiting for a Token, add it to the list for that
// Token.  The workqueue lock must be held when this is called.

Task*
Workqueue::find_runnable_in_list(Task_list* tasks)
{
  Task* t;
  while ((t = tasks->pop_front()) != NULL)
    {
      Task_token* token = t->is_runnable();

      if (token == NULL)
	return t;

      token->add_waiting(t);
      ++this->waiting_;
    }

  // We couldn't find any runnable task.
  return NULL;
}

// Find a runnable task.  Return NULL if none could be found.  The
// workqueue lock must be held when this is called.

Task*
Workqueue::find_runnable()
{
  Task* t = this->find_runnable_in_list(&this->first_tasks_);
  if (t == NULL)
    t = this->find_runnable_in_list(&this->tasks_);
  return t;
}

// Find a runnable a task, and wait until we find one.  Return NULL if
// we should exit.  The workqueue lock must be held when this is
// called.

Task*
Workqueue::find_runnable_or_wait(int thread_number)
{
  Task* t = this->find_runnable();

  while (t == NULL)
    {
      if (this->running_ == 0
	  && this->first_tasks_.empty()
	  && this->tasks_.empty())
	{
	  // Kick all the threads to make them exit.
	  this->condvar_.broadcast();

	  gold_assert(this->waiting_ == 0);
	  return NULL;
	}

      if (this->should_cancel_thread(thread_number))
	return NULL;

      gold_debug(DEBUG_TASK, "%3d sleeping", thread_number);

      this->condvar_.wait();

      gold_debug(DEBUG_TASK, "%3d awake", thread_number);

      t = this->find_runnable();
    }

  return t;
}

// Find and run tasks.  If we can't find a runnable task, wait for one
// to become available.  If we run a task, and it frees up another
// runnable task, then run that one too.  This returns true if we
// should look for another task, false if we are cancelling this
// thread.

bool
Workqueue::find_and_run_task(int thread_number)
{
  Task* t;
  Task_locker tl;

  {
    Hold_lock hl(this->lock_);

    // Find a runnable task.
    t = this->find_runnable_or_wait(thread_number);

    if (t == NULL)
      return false;

    // Get the locks for the task.  This must be called while we are
    // still holding the Workqueue lock.
    t->locks(&tl);

    ++this->running_;
  }

  while (t != NULL)
    {
      gold_debug(DEBUG_TASK, "%3d running   task %s", thread_number,
		 t->name().c_str());

      Timer timer;
      if (is_debugging_enabled(DEBUG_TASK))
        timer.start();

      t->run(this);

      if (is_debugging_enabled(DEBUG_TASK))
        {
          Timer::TimeStats elapsed = timer.get_elapsed_time();

          gold_debug(DEBUG_TASK,
                     "%3d completed task %s "
                     "(user: %ld.%06ld sys: %ld.%06ld wall: %ld.%06ld)",
                     thread_number,  t->name().c_str(),
                     elapsed.user / 1000, (elapsed.user % 1000) * 1000,
                     elapsed.sys / 1000, (elapsed.sys % 1000) * 1000,
                     elapsed.wall / 1000, (elapsed.wall % 1000) * 1000);
        }

      Task* next;
      {
	Hold_lock hl(this->lock_);

	--this->running_;

	// Release the locks for the task.  This must be done with the
	// workqueue lock held.  Get the next Task to run if any.
	next = this->release_locks(t, &tl);

	if (next == NULL)
	  next = this->find_runnable();

	// If we have another Task to run, get the Locks.  This must
	// be called while we are still holding the Workqueue lock.
	if (next != NULL)
	  {
	    tl.clear();
	    next->locks(&tl);

	    ++this->running_;
	  }
      }

      // We are done with this task.
      delete t;

      t = next;
    }

  return true;
}

// Handle the return value of release_locks, and get tasks ready to
// run.

// 1) If T is not runnable, queue it on the appropriate token.

// 2) Otherwise, T is runnable.  If *PRET is not NULL, then we have
// already decided which Task to run next.  Add T to the list of
// runnable tasks, and signal another thread.

// 3) Otherwise, *PRET is NULL.  If IS_BLOCKER is false, then T was
// waiting on a write lock.  We can grab that lock now, so we run T
// now.

// 4) Otherwise, IS_BLOCKER is true.  If we should run T soon, then
// run it now.

// 5) Otherwise, check whether there are other tasks to run.  If there
// are, then we generally get a better ordering if we run those tasks
// now, before T.  A typical example is tasks waiting on the Dirsearch
// blocker.  We don't want to run those tasks right away just because
// the Dirsearch was unblocked.

// 6) Otherwise, there are no other tasks to run, so we might as well
// run this one now.

// This function must be called with the Workqueue lock held.

// Return true if we set *PRET to T, false otherwise.

bool
Workqueue::return_or_queue(Task* t, bool is_blocker, Task** pret)
{
  Task_token* token = t->is_runnable();

  if (token != NULL)
    {
      token->add_waiting(t);
      ++this->waiting_;
      return false;
    }

  bool should_queue = false;
  bool should_return = false;

  if (*pret != NULL)
    should_queue = true;
  else if (!is_blocker)
    should_return = true;
  else if (t->should_run_soon())
    should_return = true;
  else if (!this->first_tasks_.empty() || !this->tasks_.empty())
    should_queue = true;
  else
    should_return = true;

  if (should_return)
    {
      gold_assert(*pret == NULL);
      *pret = t;
      return true;
    }
  else if (should_queue)
    {
      if (t->should_run_soon())
	this->first_tasks_.push_back(t);
      else
	this->tasks_.push_back(t);
      this->condvar_.signal();
      return false;
    }

  gold_unreachable();
}

// Release the locks associated with a Task.  Return the first
// runnable Task that we find.  If we find more runnable tasks, add
// them to the run queue and signal any other threads.  This must be
// called with the Workqueue lock held.

Task*
Workqueue::release_locks(Task* t, Task_locker* tl)
{
  Task* ret = NULL;
  for (Task_locker::iterator p = tl->begin(); p != tl->end(); ++p)
    {
      Task_token* token = *p;
      if (token->is_blocker())
	{
	  if (token->remove_blocker())
	    {
	      // The token has been unblocked.  Every waiting Task may
	      // now be runnable.
	      Task* t;
	      while ((t = token->remove_first_waiting()) != NULL)
		{
		  --this->waiting_;
		  this->return_or_queue(t, true, &ret);
		}
	    }
	}
      else
	{
	  token->remove_writer(t);

	  // One more waiting Task may now be runnable.  If we are
	  // going to run it next, we can stop.  Otherwise we need to
	  // move all the Tasks to the runnable queue, to avoid a
	  // potential deadlock if the locking status changes before
	  // we run the next thread.
	  Task* t;
	  while ((t = token->remove_first_waiting()) != NULL)
	    {
	      --this->waiting_;
	      if (this->return_or_queue(t, false, &ret))
		break;
	    }
	}
    }
  return ret;
}

// Process all the tasks on the workqueue.  Keep going until the
// workqueue is empty, or until we have been told to exit.  This
// function is called by all threads.

void
Workqueue::process(int thread_number)
{
  while (this->find_and_run_task(thread_number))
    ;
}

// Set the number of threads to use for the workqueue, if we are using
// threads.

void
Workqueue::set_thread_count(int threads)
{
  Hold_lock hl(this->lock_);

  this->threader_->set_thread_count(threads);
  // Wake up all the threads, since something has changed.
  this->condvar_.broadcast();
}

// Add a new blocker to an existing Task_token.

void
Workqueue::add_blocker(Task_token* token)
{
  Hold_lock hl(this->lock_);
  token->add_blocker();
}

} // End namespace gold.
