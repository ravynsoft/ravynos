// tls_test.cc -- test TLS variables for gold, main function

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

// This is the main function for the TLS test.  See tls_test.cc for
// more information.

#include <cassert>
#include <cstdio>
#include <pthread.h>
#include <semaphore.h>

#include "tls_test.h"

// We make these macros so the assert() will give useful line-numbers.
#define safe_lock(semptr)			\
  do						\
    {						\
      int err = sem_wait(semptr);		\
      assert(err == 0);				\
    }						\
  while (0)

#define safe_unlock(semptr)			\
  do						\
    {						\
      int err = sem_post(semptr);		\
      assert(err == 0);				\
    }						\
  while (0)

struct Sem_set
{
  sem_t sem1;
  sem_t sem2;
  sem_t sem3;
};

Sem_set sems1;
Sem_set sems2;

bool failed = false;

void
check(const char* name, bool val)
{
  if (!val)
    {
      fprintf(stderr, "Test %s failed\n", name);
      failed = true;
    }
}

// The body of the thread function.  This acquires the first
// semaphore, runs the tests, and then releases the second semaphore.
// Then it acquires the third semaphore, and the runs the verification
// test again.

void*
thread_routine(void* arg)
{
  Sem_set* pms = static_cast<Sem_set*>(arg);

  // Acquire the first semaphore.
  if (pms)
    safe_lock(&pms->sem1);

  // Run the tests.
  check("t1", t1());
  check("t2", t2());
  check("t3", t3());
  check("t4", t4());
  f5b(f5a());
  check("t5", t5());
  f6b(f6a());
  check("t6", t6());
  check("t8", t8());
  check("t9", t9());
  f10b(f10a());
  check("t10", t10());
  check("t11", t11() != 0);
  check("t12", t12());
  check("t_last", t_last());

  // Release the second semaphore.
  if (pms)
    safe_unlock(&pms->sem2);

  // Acquire the third semaphore.
  if (pms)
    safe_lock(&pms->sem3);

  check("t_last", t_last());

  return 0;
}

// The main function.

int
main()
{
  // First, as a sanity check, run through the tests in the "main" thread.
  thread_routine(0);

  // Set up the semaphores.  We want the first thread to start right
  // away, tell us when it is done with the first part, and wait for
  // us to release it.  We want the second thread to wait to start,
  // tell us when it is done with the first part, and wait for us to
  // release it.
  sem_init(&sems1.sem1, 0, 1);
  sem_init(&sems1.sem2, 0, 0);
  sem_init(&sems1.sem3, 0, 0);

  sem_init(&sems2.sem1, 0, 0);
  sem_init(&sems2.sem2, 0, 0);
  sem_init(&sems2.sem3, 0, 0);

  pthread_t thread1;
  int err = pthread_create(&thread1, NULL, thread_routine, &sems1);
  assert(err == 0);

  pthread_t thread2;
  err = pthread_create(&thread2, NULL, thread_routine, &sems2);
  assert(err == 0);

  // Wait for the first thread to complete the first part.
  safe_lock(&sems1.sem2);

  // Tell the second thread to start.
  safe_unlock(&sems2.sem1);

  // Wait for the second thread to complete the first part.
  safe_lock(&sems2.sem2);

  // Tell the first thread to continue and finish.
  safe_unlock(&sems1.sem3);

  // Wait for the first thread to finish.
  void* thread_val;
  err = pthread_join(thread1, &thread_val);
  assert(err == 0);
  assert(thread_val == 0);

  // Tell the second thread to continue and finish.
  safe_unlock(&sems2.sem3);

  // Wait for the second thread to finish.
  err = pthread_join(thread2, &thread_val);
  assert(err == 0);
  assert(thread_val == 0);

  // All done.
  return failed ? 1 : 0;
}
