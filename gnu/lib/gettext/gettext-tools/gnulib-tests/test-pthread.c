/* Test of <pthread.h> substitute.
   Copyright (C) 2019-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2019.  */

#include <config.h>

#include <pthread.h>

/* Check that the types are all defined.  */

pthread_t t1;
pthread_attr_t t2;

pthread_once_t t3 = PTHREAD_ONCE_INIT;

pthread_mutex_t t4 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutexattr_t t5;

pthread_rwlock_t t6 = PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlockattr_t t7;

pthread_cond_t t8 = PTHREAD_COND_INITIALIZER;
pthread_condattr_t t9;

pthread_key_t t10;

pthread_spinlock_t t11;

#ifdef TODO /* Not implemented in gnulib yet */
pthread_barrier_t t12;
pthread_barrierattr_t t13;
#endif

/* Check that the various macros are defined.  */

/* Constants for pthread_attr_setdetachstate().  */
int ds[] = { PTHREAD_CREATE_JOINABLE, PTHREAD_CREATE_DETACHED };

/* Constants for pthread_exit().  */
void *canceled = PTHREAD_CANCELED;

/* Constants for pthread_mutexattr_settype().  */
int mt[] = {
  PTHREAD_MUTEX_DEFAULT,
  PTHREAD_MUTEX_NORMAL,
  PTHREAD_MUTEX_RECURSIVE,
  PTHREAD_MUTEX_ERRORCHECK
};

#ifdef TODO /* Not implemented in gnulib yet */

/* Constants for pthread_mutexattr_setrobust().  */
int mr[] = { PTHREAD_MUTEX_ROBUST, PTHREAD_MUTEX_STALLED };

/* Constants for pthread_barrierattr_setpshared().  */
int bp[] = { PTHREAD_PROCESS_SHARED, PTHREAD_PROCESS_PRIVATE };

/* Constants for pthread_barrier_wait().  */
int bw[] = { PTHREAD_BARRIER_SERIAL_THREAD };

/* Constants for pthread_setcancelstate().  */
int cs[] = { PTHREAD_CANCEL_ENABLE, PTHREAD_CANCEL_DISABLE };

/* Constants for pthread_setcanceltype().  */
int ct[] = { PTHREAD_CANCEL_DEFERRED, PTHREAD_CANCEL_ASYNCHRONOUS };

#endif


int
main (void)
{
  return 0;
}
