// gold-threads.h -- thread support for gold  -*- C++ -*-

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

// gold can be configured to support threads.  If threads are
// supported, the user can specify at runtime whether or not to
// support them.  This provides an interface to manage locking
// accordingly.

// Lock
//   A simple lock class.

#ifndef GOLD_THREADS_H
#define GOLD_THREADS_H

namespace gold
{

class Condvar;
class Once_initialize;
class Initialize_lock_once;

// The interface for the implementation of a Lock.

class Lock_impl
{
 public:
  Lock_impl()
  { }

  virtual
  ~Lock_impl()
  { }

  virtual void
  acquire() = 0;

  virtual void
  release() = 0;
};

// A simple lock class.

class Lock
{
 public:
  Lock();

  ~Lock();

  // Acquire the lock.
  void
  acquire()
  { this->lock_->acquire(); }

  // Release the lock.
  void
  release()
  { this->lock_->release(); }

 private:
  // This class can not be copied.
  Lock(const Lock&);
  Lock& operator=(const Lock&);

  friend class Condvar;
  Lock_impl*
  get_impl() const
  { return this->lock_; }

  Lock_impl* lock_;
};

// RAII for Lock.

class Hold_lock
{
 public:
  Hold_lock(Lock& lock)
    : lock_(lock)
  { this->lock_.acquire(); }

  ~Hold_lock()
  { this->lock_.release(); }

 private:
  // This class can not be copied.
  Hold_lock(const Hold_lock&);
  Hold_lock& operator=(const Hold_lock&);

  Lock& lock_;
};

class Hold_optional_lock
{
 public:
  Hold_optional_lock(Lock* lock)
    : lock_(lock)
  {
    if (this->lock_ != NULL)
      this->lock_->acquire();
  }

  ~Hold_optional_lock()
  {
    if (this->lock_ != NULL)
      this->lock_->release();
  }

 private:
  Hold_optional_lock(const Hold_optional_lock&);
  Hold_optional_lock& operator=(const Hold_optional_lock&);

  Lock* lock_;
};

// The interface for the implementation of a condition variable.

class Condvar_impl
{
 public:
  Condvar_impl()
  { }

  virtual
  ~Condvar_impl()
  { }

  virtual void
  wait(Lock_impl*) = 0;

  virtual void
  signal() = 0;

  virtual void
  broadcast() = 0;
};

// A simple condition variable class.  It is always associated with a
// specific lock.

class Condvar
{
 public:
  Condvar(Lock& lock);
  ~Condvar();

  // Wait for the condition variable to be signalled.  This should
  // only be called when the lock is held.
  void
  wait()
  { this->condvar_->wait(this->lock_.get_impl()); }

  // Signal the condition variable--wake up at least one thread
  // waiting on the condition variable.  This should only be called
  // when the lock is held.
  void
  signal()
  { this->condvar_->signal(); }

  // Broadcast the condition variable--wake up all threads waiting on
  // the condition variable.  This should only be called when the lock
  // is held.
  void
  broadcast()
  { this->condvar_->broadcast(); }

 private:
  // This class can not be copied.
  Condvar(const Condvar&);
  Condvar& operator=(const Condvar&);

  Lock& lock_;
  Condvar_impl* condvar_;
};

// A class used to do something once.  This is an abstract parent
// class; any actual use will involve a child of this.

class Once
{
 public:
  Once();

  virtual
  ~Once()
  { }

  // Call this function to do whatever it is.  We pass an argument
  // even though you have to use a child class because in some uses
  // setting the argument would itself require a Once class.
  void
  run_once(void* arg);

  // This is an internal function, which must be public because it is
  // run by an extern "C" function called via pthread_once.
  void
  internal_run(void* arg);

 protected:
  // This must be implemented by the child class.
  virtual void
  do_run_once(void* arg) = 0;

 private:
  // True if we have already run the function.
  bool was_run_;
#if defined(ENABLE_THREADS) && defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4)
  // Internal compare-and-swap lock on was_run_;
  uint32_t was_run_lock_;
#endif
  // The lock to run the function only once.
  Once_initialize* once_;
};

// A class used to initialize a lock exactly once, after the options
// have been read.  This is needed because the implementation of locks
// depends on whether we've seen the --threads option.  Before the
// options have been read, we know we are single-threaded, so we can
// get by without using a lock.  This class should be an instance
// variable of the class which has a lock which needs to be
// initialized.

class Initialize_lock : public Once
{
 public:
  // The class which uses this will have a pointer to a lock.  This
  // must be constructed with a pointer to that pointer.
  Initialize_lock(Lock** pplock)
    : pplock_(pplock)
  { }

  // Initialize the lock.  Return true if the lock is now initialized,
  // false if it is not (because the options have not yet been read).
  bool
  initialize();

 protected:
  void
  do_run_once(void*);

 private:
  // A pointer to the lock pointer which must be initialized.
  Lock** const pplock_;
};

} // End namespace gold.

#endif // !defined(GOLD_THREADS_H)
