/* Spin locks for communication between threads and signal handlers.
   Copyright (C) 2020-2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2020.  */

#ifndef _ASYNCSAFE_SPIN_H
#define _ASYNCSAFE_SPIN_H

/* Usual spin locks are not allowed for communication between threads and signal
   handlers, because the pthread_spin_* functions are not async-safe; see
   <https://pubs.opengroup.org/onlinepubs/9699919799/functions/V2_chap02.html>
   section 2.4.3 Signal Actions.

   This module provides spin locks with a similar API.  It can be used like this,
   both in regular multithreaded code and in signal handlers:

       sigset_t saved_mask;
       asyncsafe_spin_lock (&lock, &mask, &saved_mask);
       do_something_contentious ();
       asyncsafe_spin_unlock (&lock, &saved_mask);

   The mask you specify here is the set of signals whose handlers might want to
   take the same lock.

   asyncsafe_spin_lock/unlock use pthread_sigmask, to ensure that while a thread
   is executing such code, no signal handler will start such code for the same
   lock *in the same thread* (because if this happened, the signal handler would
   hang!).  */

#include <signal.h>

#if defined _WIN32 && ! defined __CYGWIN__
# include "windows-spin.h"
typedef glwthread_spinlock_t asyncsafe_spinlock_t;
# define ASYNCSAFE_SPIN_INIT GLWTHREAD_SPIN_INIT
#else
typedef unsigned int asyncsafe_spinlock_t;
# define ASYNCSAFE_SPIN_INIT 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern void asyncsafe_spin_init (asyncsafe_spinlock_t *lock);
extern void asyncsafe_spin_lock (asyncsafe_spinlock_t *lock,
                                 const sigset_t *mask, sigset_t *saved_mask);
extern void asyncsafe_spin_unlock (asyncsafe_spinlock_t *lock,
                                   const sigset_t *saved_mask);
extern void asyncsafe_spin_destroy (asyncsafe_spinlock_t *lock);

#ifdef __cplusplus
}
#endif

#endif /* _ASYNCSAFE_SPIN_H */
