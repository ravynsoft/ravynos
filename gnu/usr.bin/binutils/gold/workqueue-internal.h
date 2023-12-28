// workqueue-internal.h -- internal work queue header for gold   -*- C++ -*-

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

#ifndef GOLD_WORKQUEUE_INTERNAL_H
#define GOLD_WORKQUEUE_INTERNAL_H

#include <queue>
#include <csignal>

#include "gold-threads.h"
#include "workqueue.h"

// This is an internal header file for different gold workqueue
// implementations.

namespace gold
{

class Workqueue_thread;

// The Workqueue_threader abstract class.  This is the interface used
// by the general workqueue code to manage threads.

class Workqueue_threader
{
 public:
  Workqueue_threader(Workqueue* workqueue)
    : workqueue_(workqueue)
  { }
  virtual ~Workqueue_threader()
  { }

  // Set the number of threads to use.  This is ignored when not using
  // threads.
  virtual void
  set_thread_count(int) = 0;

  // Return whether to cancel the current thread.
  virtual bool
  should_cancel_thread(int thread_number) = 0;

 protected:
  // Get the Workqueue.
  Workqueue*
  get_workqueue()
  { return this->workqueue_; }

 private:
  // The Workqueue.
  Workqueue* workqueue_;
};

// The threaded instantiation of Workqueue_threader.

class Workqueue_threader_threadpool : public Workqueue_threader
{
 public:
  Workqueue_threader_threadpool(Workqueue*);

  ~Workqueue_threader_threadpool();

  // Set the thread count.
  void
  set_thread_count(int);

  // Return whether to cancel a thread.
  bool
  should_cancel_thread(int thread_number);

  // Process all tasks.  This keeps running until told to cancel.
  void
  process(int thread_number)
  { this->get_workqueue()->process(thread_number); }

 private:
  // This is set if we need to check the thread count.
  volatile sig_atomic_t check_thread_count_;

  // Lock for the remaining members.
  Lock lock_;
  // The number of threads we want to create.  This is set to zero
  // when all threads should exit.
  int desired_thread_count_;
  // The number of threads currently running.
  int threads_;
};

} // End namespace gold.

#endif // !defined(GOLD_WORKQUEUE_INTERNAL_H)
