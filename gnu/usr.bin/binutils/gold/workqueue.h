// workqueue.h -- the work queue for gold   -*- C++ -*-

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

// After processing the command line, everything the linker does is
// driven from a work queue.  This permits us to parallelize the
// linker where possible.

#ifndef GOLD_WORKQUEUE_H
#define GOLD_WORKQUEUE_H

#include <string>

#include "gold-threads.h"
#include "token.h"

namespace gold
{

class General_options;
class Workqueue;

// The superclass for tasks to be placed on the workqueue.  Each
// specific task class will inherit from this one.

class Task
{
 public:
  Task()
    : list_next_(NULL), name_(), should_run_soon_(false)
  { }
  virtual ~Task()
  { }

  // Check whether the Task can be run now.  This method is only
  // called with the workqueue lock held.  If the Task can run, this
  // returns NULL.  Otherwise it returns a pointer to a token which
  // must be released before the Task can run.
  virtual Task_token*
  is_runnable() = 0;

  // Lock all the resources required by the Task, and store the locks
  // in a Task_locker.  This method does not need to do anything if no
  // locks are required.  This method is only called with the
  // workqueue lock held.
  virtual void
  locks(Task_locker*) = 0;

  // Run the task.
  virtual void
  run(Workqueue*) = 0;

  // Return whether this task should run soon.
  bool
  should_run_soon() const
  { return this->should_run_soon_; }

  // Note that this task should run soon.
  void
  set_should_run_soon()
  { this->should_run_soon_ = true; }

  // Get the next Task on the list of Tasks.  Called by Task_list.
  Task*
  list_next() const
  { return this->list_next_; }

  // Set the next Task on the list of Tasks.  Called by Task_list.
  void
  set_list_next(Task* t)
  {
    gold_assert(this->list_next_ == NULL);
    this->list_next_ = t;
  }

  // Clear the next Task on the list of Tasks.  Called by Task_list.
  void
  clear_list_next()
  { this->list_next_ = NULL; }

  // Return the name of the Task.  This is only used for debugging
  // purposes.
  const std::string&
  name()
  {
    if (this->name_.empty())
      this->name_ = this->get_name();
    return this->name_;
  }

 protected:
  // Get the name of the task.  This must be implemented by the child
  // class.
  virtual std::string
  get_name() const = 0;

 private:
  // Tasks may not be copied.
  Task(const Task&);
  Task& operator=(const Task&);

  // If this Task is on a list, this is a pointer to the next Task on
  // the list.  We use this simple list structure rather than building
  // a container, in order to avoid memory allocation while holding
  // the Workqueue lock.
  Task* list_next_;
  // Task name, for debugging purposes.
  std::string name_;
  // Whether this Task should be executed soon.  This is used for
  // Tasks which can be run after some data is read.
  bool should_run_soon_;
};

// An interface for Task_function.  This is a convenience class to run
// a single function.

class Task_function_runner
{
 public:
  virtual ~Task_function_runner()
  { }

  virtual void
  run(Workqueue*, const Task*) = 0;
};

// A simple task which waits for a blocker and then runs a function.

class Task_function : public Task
{
 public:
  // RUNNER and BLOCKER should be allocated using new, and will be
  // deleted after the task runs.
  Task_function(Task_function_runner* runner, Task_token* blocker,
		const char* name)
    : runner_(runner), blocker_(blocker), name_(name)
  { gold_assert(blocker != NULL); }

  ~Task_function()
  {
    delete this->runner_;
    delete this->blocker_;
  }

  // The standard task methods.

  // Wait until the task is unblocked.
  Task_token*
  is_runnable()
  { return this->blocker_->is_blocked() ? this->blocker_ : NULL; }

  // This type of task does not normally hold any locks.
  virtual void
  locks(Task_locker*)
  { }

  // Run the action.
  void
  run(Workqueue* workqueue)
  { this->runner_->run(workqueue, this); }

  // The debugging name.
  std::string
  get_name() const
  { return this->name_; }

 private:
  Task_function(const Task_function&);
  Task_function& operator=(const Task_function&);

  Task_function_runner* runner_;
  Task_token* blocker_;
  const char* name_;
};

// The workqueue itself.

class Workqueue_threader;

class Workqueue
{
 public:
  Workqueue(const General_options&);
  ~Workqueue();

  // Add a new task to the work queue.
  void
  queue(Task*);

  // Add a new task to the work queue which should run soon.  If the
  // task is ready, it will be run before any tasks added using
  // queue().
  void
  queue_soon(Task*);

  // Add a new task to the work queue which should run next if it is
  // ready.
  void
  queue_next(Task*);

  // Process all the tasks on the work queue.  This function runs
  // until all tasks have completed.  The argument is the thread
  // number, used only for debugging.
  void
  process(int);

  // Set the desired thread count--the number of threads we want to
  // have running.
  void
  set_thread_count(int);

  // Add a new blocker to an existing Task_token. This must be done
  // with the workqueue lock held.  This should not be done routinely,
  // only in special circumstances.
  void
  add_blocker(Task_token*);

 private:
  // This class can not be copied.
  Workqueue(const Workqueue&);
  Workqueue& operator=(const Workqueue&);

  // Add a task to a queue.
  void
  add_to_queue(Task_list* queue, Task* t, bool front);

  // Find a runnable task, or wait for one.
  Task*
  find_runnable_or_wait(int thread_number);

  // Find a runnable task.
  Task*
  find_runnable();

  // Find a runnable task in a list.
  Task*
  find_runnable_in_list(Task_list*);

  // Find an run a task.
  bool
  find_and_run_task(int);

  // Release the locks for a Task.  Return the next Task to run.
  Task*
  release_locks(Task*, Task_locker*);

  // Store T into *PRET, or queue it as appropriate.
  bool
  return_or_queue(Task* t, bool is_blocker, Task** pret);

  // Return whether to cancel this thread.
  bool
  should_cancel_thread(int thread_number);

  // Master Workqueue lock.  This controls access to the following
  // member variables.
  Lock lock_;
  // List of tasks to execute soon.
  Task_list first_tasks_;
  // List of tasks to execute after the ones in first_tasks_.
  Task_list tasks_;
  // Number of tasks currently running.
  int running_;
  // Number of tasks waiting for a lock to release.
  int waiting_;
  // Condition variable associated with lock_.  This is signalled when
  // there may be a new Task to execute.
  Condvar condvar_;

  // The threading implementation.  This is set at construction time
  // and not changed thereafter.
  Workqueue_threader* threader_;
};

} // End namespace gold.

#endif // !defined(GOLD_WORKQUEUE_H)
