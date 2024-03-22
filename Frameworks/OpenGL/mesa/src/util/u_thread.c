/*
 * Copyright 1999-2006 Brian Paul
 * Copyright 2008 VMware, Inc.
 * Copyright 2022 Yonggang Luo
 * SPDX-License-Identifier: MIT
 */

#include "util/u_thread.h"

#include "macros.h"

#ifdef HAVE_PTHREAD
#include <signal.h>
#ifdef HAVE_PTHREAD_NP_H
#include <pthread_np.h>
#endif
#endif

#ifdef __HAIKU__
#include <OS.h>
#endif

#if DETECT_OS_LINUX && !defined(ANDROID)
#include <sched.h>
#elif defined(_WIN32) && !defined(HAVE_PTHREAD)
#include <windows.h>
#endif

#ifdef __FreeBSD__
/* pthread_np.h -> sys/param.h -> machine/param.h
 * - defines ALIGN which clashes with our ALIGN
 */
#undef ALIGN
#define cpu_set_t cpuset_t
#endif

int
util_get_current_cpu(void)
{
#if DETECT_OS_LINUX && !defined(ANDROID)
   return sched_getcpu();

#elif defined(_WIN32) && !defined(HAVE_PTHREAD)
   return GetCurrentProcessorNumber();

#else
   return -1;
#endif
}

int u_thread_create(thrd_t *thrd, int (*routine)(void *), void *param)
{
   int ret = thrd_error;
#ifdef HAVE_PTHREAD
   sigset_t saved_set, new_set;

   sigfillset(&new_set);
   sigdelset(&new_set, SIGSYS);

   /* SIGSEGV is commonly used by Vulkan API tracing layers in order to track
    * accesses in device memory mapped to user space. Blocking the signal hinders
    * that tracking mechanism.
    */
   sigdelset(&new_set, SIGSEGV);
   pthread_sigmask(SIG_BLOCK, &new_set, &saved_set);
   ret = thrd_create(thrd, routine, param);
   pthread_sigmask(SIG_SETMASK, &saved_set, NULL);
#else
   ret = thrd_create(thrd, routine, param);
#endif

   return ret;
}

void u_thread_setname( const char *name )
{
#if defined(HAVE_PTHREAD)
#if DETECT_OS_LINUX || DETECT_OS_CYGWIN || DETECT_OS_SOLARIS || defined(__GLIBC__) || DETECT_OS_MANAGARM
   int ret = pthread_setname_np(pthread_self(), name);
   if (ret == ERANGE) {
      char buf[16];
      const size_t len = MIN2(strlen(name), ARRAY_SIZE(buf) - 1);
      memcpy(buf, name, len);
      buf[len] = '\0';
      pthread_setname_np(pthread_self(), buf);
   }
#elif DETECT_OS_FREEBSD || DETECT_OS_OPENBSD
   pthread_set_name_np(pthread_self(), name);
#elif DETECT_OS_NETBSD
   pthread_setname_np(pthread_self(), "%s", (void *)name);
#elif DETECT_OS_APPLE
   pthread_setname_np(name);
#elif DETECT_OS_HAIKU
   rename_thread(find_thread(NULL), name);
#else
#warning Not sure how to call pthread_setname_np
#endif
#endif
   (void)name;
}

bool
util_set_thread_affinity(thrd_t thread,
                         const uint32_t *mask,
                         uint32_t *old_mask,
                         unsigned num_mask_bits)
{
#if defined(HAVE_PTHREAD_SETAFFINITY)
   cpu_set_t cpuset;

   if (old_mask) {
      if (pthread_getaffinity_np(thread, sizeof(cpuset), &cpuset) != 0)
         return false;

      memset(old_mask, 0, num_mask_bits / 8);
      for (unsigned i = 0; i < num_mask_bits && i < CPU_SETSIZE; i++) {
         if (CPU_ISSET(i, &cpuset))
            old_mask[i / 32] |= 1u << (i % 32);
      }
   }

   CPU_ZERO(&cpuset);
   for (unsigned i = 0; i < num_mask_bits && i < CPU_SETSIZE; i++) {
      if (mask[i / 32] & (1u << (i % 32)))
         CPU_SET(i, &cpuset);
   }
   return pthread_setaffinity_np(thread, sizeof(cpuset), &cpuset) == 0;

#elif defined(_WIN32) && !defined(HAVE_PTHREAD)
   DWORD_PTR m = mask[0];

   if (sizeof(m) > 4 && num_mask_bits > 32)
      m |= (uint64_t)mask[1] << 32;

   m = SetThreadAffinityMask(thread.handle, m);
   if (!m)
      return false;

   if (old_mask) {
      memset(old_mask, 0, num_mask_bits / 8);

      old_mask[0] = m;
#ifdef _WIN64
      old_mask[1] = m >> 32;
#endif
   }

   return true;
#else
   return false;
#endif
}

int64_t
util_thread_get_time_nano(thrd_t thread)
{
#if defined(HAVE_PTHREAD) && !defined(__APPLE__) && !defined(__HAIKU__) && !defined(__managarm__)
   struct timespec ts;
   clockid_t cid;

   pthread_getcpuclockid(thread, &cid);
   clock_gettime(cid, &ts);
   return (int64_t)ts.tv_sec * 1000000000 + ts.tv_nsec;
#elif defined(_WIN32)
   union {
      FILETIME time;
      ULONGLONG value;
   } kernel_time, user_time;
   GetThreadTimes((HANDLE)thread.handle, NULL, NULL, &kernel_time.time, &user_time.time);
   return (kernel_time.value + user_time.value) * 100;
#else
   (void)thread;
   return 0;
#endif
}

#if defined(HAVE_PTHREAD) && !defined(__APPLE__) && !defined(__HAIKU__)

void util_barrier_init(util_barrier *barrier, unsigned count)
{
   pthread_barrier_init(barrier, NULL, count);
}

void util_barrier_destroy(util_barrier *barrier)
{
   pthread_barrier_destroy(barrier);
}

bool util_barrier_wait(util_barrier *barrier)
{
   return pthread_barrier_wait(barrier) == PTHREAD_BARRIER_SERIAL_THREAD;
}

#else /* If the OS doesn't have its own, implement barriers using a mutex and a condvar */

void util_barrier_init(util_barrier *barrier, unsigned count)
{
   barrier->count = count;
   barrier->waiters = 0;
   barrier->sequence = 0;
   (void) mtx_init(&barrier->mutex, mtx_plain);
   cnd_init(&barrier->condvar);
}

void util_barrier_destroy(util_barrier *barrier)
{
   assert(barrier->waiters == 0);
   mtx_destroy(&barrier->mutex);
   cnd_destroy(&barrier->condvar);
}

bool util_barrier_wait(util_barrier *barrier)
{
   mtx_lock(&barrier->mutex);

   assert(barrier->waiters < barrier->count);
   barrier->waiters++;

   if (barrier->waiters < barrier->count) {
      uint64_t sequence = barrier->sequence;

      do {
         cnd_wait(&barrier->condvar, &barrier->mutex);
      } while (sequence == barrier->sequence);
   } else {
      barrier->waiters = 0;
      barrier->sequence++;
      cnd_broadcast(&barrier->condvar);
   }

   mtx_unlock(&barrier->mutex);

   return true;
}

#endif
