// workqueue-threads.cc -- the threaded workqueue for gold

// Copyright (C) 2007-2023 Free Software Foundation, Inc.
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

// This file holds the workqueue implementation which may be used when
// using threads.

#include "gold.h"

#ifdef ENABLE_THREADS

#include <cstring>
#include <pthread.h>

#include "debug.h"
#include "gold-threads.h"
#include "workqueue.h"
#include "workqueue-internal.h"

namespace gold
{

// Class Workqueue_thread represents a single thread.  Creating an
// instance of this spawns a new thread.

class Workqueue_thread
{
 public:
  Workqueue_thread(Workqueue_threader_threadpool*, int thread_number);

  ~Workqueue_thread();

 private:
  // This class can not be copied.
  Workqueue_thread(const Workqueue_thread&);
  Workqueue_thread& operator=(const Workqueue_thread&);

  // Check for error from a pthread function.
  void
  check(const char* function, int err) const;

  // A function to pass to pthread_create.  This is called with a
  // pointer to an instance of this object.
  static void*
  thread_body(void*);

  // A pointer to the threadpool that this thread is part of.
  Workqueue_threader_threadpool* threadpool_;
  // The thread number.
  int thread_number_;
  // The thread ID.
  pthread_t tid_;
};

// Create the thread in the constructor.

Workqueue_thread::Workqueue_thread(Workqueue_threader_threadpool* threadpool,
				   int thread_number)
  : threadpool_(threadpool), thread_number_(thread_number)
{
  pthread_attr_t attr;
  int err = pthread_attr_init(&attr);
  this->check("pthread_attr_init", err);

  err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  this->check("pthread_attr_setdetachstate", err);

  err = pthread_create(&this->tid_, &attr, &Workqueue_thread::thread_body,
		       reinterpret_cast<void*>(this));
  this->check("pthread_create", err);

  err = pthread_attr_destroy(&attr);
  this->check("pthread_attr_destroy", err);
}

// The destructor will be called when the thread is exiting.

Workqueue_thread::~Workqueue_thread()
{
}

// Check for an error.

void
Workqueue_thread::check(const char* function, int err) const
{
  if (err != 0)
    gold_fatal(_("%s failed: %s"), function, strerror(err));
}

// Passed to pthread_create.

extern "C"
void*
Workqueue_thread::thread_body(void* arg)
{
  Workqueue_thread* pwt = reinterpret_cast<Workqueue_thread*>(arg);

  pwt->threadpool_->process(pwt->thread_number_);

  // Delete the thread object as we exit.
  delete pwt;

  return NULL;
}

// Class Workqueue_threader_threadpool.

// Constructor.

Workqueue_threader_threadpool::Workqueue_threader_threadpool(
    Workqueue* workqueue)
  : Workqueue_threader(workqueue),
    check_thread_count_(0),
    lock_(),
    desired_thread_count_(1),
    threads_(1)
{
}

// Destructor.

Workqueue_threader_threadpool::~Workqueue_threader_threadpool()
{
  // Tell the threads to exit.
  this->get_workqueue()->set_thread_count(0);
}

// Set the thread count.

void
Workqueue_threader_threadpool::set_thread_count(int thread_count)
{
  int create;
  {
    Hold_lock hl(this->lock_);

    this->desired_thread_count_ = thread_count;
    create = this->desired_thread_count_ - this->threads_;
    if (create < 0)
      this->check_thread_count_ = 1;
  }

  if (create > 0)
    {
      for (int i = 0; i < create; ++i)
	{
	  // Note that threads delete themselves when they exit, so we
	  // don't keep pointers to them.
	  new Workqueue_thread(this, this->threads_);
	  ++this->threads_;
	}
    }
}

// Return whether the current thread should be cancelled.

bool
Workqueue_threader_threadpool::should_cancel_thread(int thread_number)
{
  // Fast exit without taking a lock.
  if (!this->check_thread_count_)
    return false;

  {
    Hold_lock hl(this->lock_);
    if (thread_number > this->desired_thread_count_)
      {
	--this->threads_;
	if (this->threads_ <= this->desired_thread_count_)
	  this->check_thread_count_ = 0;
	return true;
      }
  }

  return false;
}

} // End namespace gold.

#endif // defined(ENABLE_THREADS)
