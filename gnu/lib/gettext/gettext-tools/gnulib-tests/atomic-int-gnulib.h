/* Atomic integers.  Useful for testing multithreaded locking primitives.
   Copyright (C) 2005, 2008-2023 Free Software Foundation, Inc.

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


/* Whether to use 'volatile' on some variables that communicate information
   between threads.  If set to 0, a semaphore or a lock is used to protect
   these variables.  If set to 1, 'volatile' is used; this is theoretically
   equivalent but can lead to much slower execution (e.g. 30x slower total
   run time on a 40-core machine), because 'volatile' does not imply any
   synchronization/communication between different CPUs.  */
#define USE_VOLATILE 0

#if USE_POSIX_THREADS && HAVE_SEMAPHORE_H
/* Whether to use a semaphore to communicate information between threads.
   If set to 0, a lock is used. If set to 1, a semaphore is used.
   Uncomment this to reduce the dependencies of this test.  */
# define USE_SEMAPHORE 1
/* Mac OS X provides only named semaphores (sem_open); its facility for
   unnamed semaphores (sem_init) does not work.  */
# if defined __APPLE__ && defined __MACH__
#  define USE_NAMED_SEMAPHORE 1
# else
#  define USE_UNNAMED_SEMAPHORE 1
# endif
#endif


#if USE_SEMAPHORE
# include <errno.h>
# include <fcntl.h>
# include <semaphore.h>
# include <unistd.h>
#endif


#if USE_VOLATILE
struct atomic_int {
  volatile int value;
};
static void
init_atomic_int (struct atomic_int *ai)
{
}
static int
get_atomic_int_value (struct atomic_int *ai)
{
  return ai->value;
}
static void
set_atomic_int_value (struct atomic_int *ai, int new_value)
{
  ai->value = new_value;
}
#elif USE_SEMAPHORE
/* This atomic_int implementation can only support the values 0 and 1.
   It is initially 0 and can be set to 1 only once.  */
# if USE_UNNAMED_SEMAPHORE
struct atomic_int {
  sem_t semaphore;
};
#define atomic_int_semaphore(ai) (&(ai)->semaphore)
static void
init_atomic_int (struct atomic_int *ai)
{
  sem_init (&ai->semaphore, 0, 0);
}
# endif
# if USE_NAMED_SEMAPHORE
struct atomic_int {
  sem_t *semaphore;
};
#define atomic_int_semaphore(ai) ((ai)->semaphore)
static void
init_atomic_int (struct atomic_int *ai)
{
  sem_t *s;
  unsigned int count;
  for (count = 0; ; count++)
    {
      char name[80];
      /* Use getpid() in the name, so that different processes running at the
         same time will not interfere.  Use ai in the name, so that different
         atomic_int in the same process will not interfere.  Use a count in
         the name, so that even in the (unlikely) case that a semaphore with
         the specified name already exists, we can try a different name.  */
      sprintf (name, "test-lock-%lu-%p-%u",
               (unsigned long) getpid (), ai, count);
      s = sem_open (name, O_CREAT | O_EXCL, 0600, 0);
      if (s == SEM_FAILED)
        {
          if (errno == EEXIST)
            /* Retry with a different name.  */
            continue;
          else
            {
              perror ("sem_open failed");
              fflush (stderr);
              abort ();
            }
        }
      else
        {
          /* Try not to leave a semaphore hanging around on the file system
             eternally, if we can avoid it.  */
          sem_unlink (name);
          break;
        }
    }
  ai->semaphore = s;
}
# endif
static int
get_atomic_int_value (struct atomic_int *ai)
{
  if (sem_trywait (atomic_int_semaphore (ai)) == 0)
    {
      if (sem_post (atomic_int_semaphore (ai)))
        abort ();
      return 1;
    }
  else if (errno == EAGAIN)
    return 0;
  else
    abort ();
}
static void
set_atomic_int_value (struct atomic_int *ai, int new_value)
{
  if (new_value == 0)
    /* It's already initialized with 0.  */
    return;
  /* To set the value 1: */
  if (sem_post (atomic_int_semaphore (ai)))
    abort ();
}
#else
struct atomic_int {
  gl_lock_define (, lock)
  int value;
};
static void
init_atomic_int (struct atomic_int *ai)
{
  gl_lock_init (ai->lock);
}
static int
get_atomic_int_value (struct atomic_int *ai)
{
  gl_lock_lock (ai->lock);
  int ret = ai->value;
  gl_lock_unlock (ai->lock);
  return ret;
}
static void
set_atomic_int_value (struct atomic_int *ai, int new_value)
{
  gl_lock_lock (ai->lock);
  ai->value = new_value;
  gl_lock_unlock (ai->lock);
}
#endif
