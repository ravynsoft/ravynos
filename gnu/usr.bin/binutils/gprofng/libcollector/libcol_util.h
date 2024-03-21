/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef _LIBCOL_UTIL_H
#define _LIBCOL_UTIL_H

#include <stdarg.h>
#include <pthread.h>
#include <signal.h>

// LIBCOLLECTOR NOT I18N
#define NTXT(x) x
#define STXT(x) x

extern int __collector_tracelevel;

/* Initialization function */
extern	int  __collector_util_init();
extern  void __collector_libkstat_funcs_init();
extern  void __collector_libscf_funcs_init();

/* -------  functions from libcol_util.c ----------------- */
extern void * __collector_memcpy (void *s1, const void *s2, size_t n);
extern int (*__collector_sscanfp)(const char *restrict s, const char *restrict fmt, ...);
extern char * __collector_strcat (char *s1, const char *s2);
extern char * __collector_strchr (const char *s1, int chr);
extern size_t __collector_strlcpy (char *dst, const char *src, size_t dstsize);
extern char* __collector_strrchr (const char *str, int chr);
extern size_t __collector_strlen (const char *s);
extern size_t __collector_strlcat (char *dst, const char *src, size_t dstsize);
extern char* __collector_strchr (const char *str, int chr);
extern int __collector_strcmp (const char *s1, const char *s2);
extern int __collector_strncmp (const char *s1, const char *s2, size_t n);
extern char * __collector_strstr (const char *s1, const char *s2);
extern size_t __collector_strncpy (char *dst, const char *src, size_t dstsize);
extern size_t __collector_strncat (char *dst, const char *src, size_t dstsize);
extern void * __collector_malloc (size_t size);
extern void * __collector_calloc (size_t nelem, size_t elsize);
extern char * __collector_strdup (const char * str);
extern int __collector_strStartWith (const char *s1, const char *s2);
extern int __collector_xml_snprintf (char *s, size_t n, const char *format, ...) __attribute__ ((format (printf, 3, 4)));
extern int __collector_xml_vsnprintf (char *s, size_t n, const char *format, va_list args);

/* -------  collector_thread ----------------- */
extern pid_t __collector_gettid ();
extern void __collector_ext_gettid_tsd_create_key ();
typedef pthread_t collector_thread_t;
#define __collector_lwp_self() ((collector_thread_t) ((unsigned long) __collector_gettid()))
#define __collector_thr_self() ((collector_thread_t) ((unsigned long) __collector_gettid()))

/* -------  collector_mutex ----------------- */
/*
 * mutex_init is defined in libthread. If we don't want to interact
 * with libthread we should use memset to initialize mutexes
 */

typedef volatile int collector_mutex_t;
#define  COLLECTOR_MUTEX_INITIALIZER 0
extern int __collector_mutex_lock (collector_mutex_t *mp);
extern int __collector_mutex_unlock (collector_mutex_t *mp);
extern int __collector_mutex_trylock (collector_mutex_t *mp);

#define __collector_mutex_init(xx) \
  do { collector_mutex_t tmp=COLLECTOR_MUTEX_INITIALIZER; *(xx)=tmp; } while(0)

void __collector_sample (char *name);
void __collector_terminate_expt ();
void __collector_pause ();
void __collector_pause_m ();
void __collector_resume ();

struct DT_lineno;

typedef enum
{
  DFUNC_API = 1, /* dynamic function declared with API */
  DFUNC_JAVA, /* dynamically compiled java method */
  DFUNC_KERNEL /* dynamic code mapped by the kernel (Linux) */
} dfunc_mode_t;

extern void __collector_int_func_load (dfunc_mode_t mode, char *name,
				       char *sourcename, void *vaddr,
				       int size, int lntsize,
				       struct DT_lineno *lntable);
extern void __collector_int_func_unload (dfunc_mode_t mode, void *vaddr);

extern int __collector_sigaction (int sig, const struct sigaction *nact,
				  struct sigaction *oact);
extern void __collector_SIGDFL_handler (int sig);
extern int __collector_ext_itimer_set (int period);

#if ARCH(Intel)
/* Atomic functions on x86/x64 */

/**
 * This function enables the inrementing (by one) of the value stored in target
 * to occur in an atomic manner.
 */
static __attribute__ ((always_inline)) inline void
__collector_inc_32 (uint32_t *ptr)
{
  __asm__ __volatile__("lock; incl %0"
		       : // "=m" (*ptr)    // output
		       : "m" (*ptr)); // input
}

/**
 * This function enables the decrementing (by one) of the value stored in target
 * to occur in an atomic manner.
 */
static __attribute__ ((always_inline)) inline void
__collector_dec_32 (volatile uint32_t *ptr)
{
  __asm__ __volatile__("lock; decl %0"
		       : // "=m" (*ptr)    // output
		       : "m" (*ptr)); // input
}

/**
 * This function subtrackts the value "off" of the value stored in target
 * to occur in an atomic manner, and returns new value stored in target.
 */
static __attribute__ ((always_inline)) inline uint32_t
__collector_subget_32 (uint32_t *ptr, uint32_t off)
{
  uint32_t r;
  uint32_t offset = off;
  __asm__ __volatile__("movl %2, %0; negl %0; lock; xaddl %0, %1"
		       : "=r" (r), "=m" (*ptr) /* output */
		       : "a" (off), "r" (*ptr) /* input */
		       );
  return (r - offset);
}

/**
 * This function returns the value of the stack pointer register
 */
static __attribute__ ((always_inline)) inline void *
__collector_getsp ()
{
  void *r;
#if WSIZE(32) || defined(__ILP32__)
  __asm__ __volatile__("movl %%esp, %0"
#else
  __asm__ __volatile__("movq %%rsp, %0"
#endif 
	  : "=r" (r)); // output
  return r;
}

/**
 * This function returns the value of the frame pointer register
 */
static __attribute__ ((always_inline)) inline void *
__collector_getfp ()
{
  void *r;
#if WSIZE(32) || defined(__ILP32__)
  __asm__ __volatile__("movl %%ebp, %0"
#else
  __asm__ __volatile__("movq %%rbp, %0"
#endif
	  : "=r" (r)); // output
  return r;
}

/**
 * This function returns the value of the processor counter register
 */
static __attribute__ ((always_inline)) inline void *
__collector_getpc ()
{
  void *r;
#if defined(__x86_64)
  __asm__ __volatile__("lea (%%rip), %0" : "=r" (r));
#else
  __asm__ __volatile__("call  1f \n"
		       "1: popl  %0" : "=r" (r));
#endif
  return r;
}

/**
 * This function enables a compare and swap operation to occur atomically.
 * The 32-bit value stored in target is compared with "old". If these values
 * are equal, the value stored in target is replaced with "new". The old
 * 32-bit value stored in target is returned by the function whether or not
 * the replacement occurred.
 */
static __attribute__ ((always_inline)) inline uint32_t
__collector_cas_32 (volatile uint32_t *pdata, uint32_t old, uint32_t new)
{
  uint32_t r;
  __asm__ __volatile__("lock; cmpxchgl %2, %1"
		       : "=a" (r), "=m" (*pdata) : "r" (new),
		       "a" (old), "m" (*pdata));
  return r;
}
/**
 * This function enables a compare and swap operation to occur atomically.
 * The 64-bit value stored in target is compared with "old". If these values
 * are equal, the value stored in target is replaced with "new". The old
 * 64-bit value stored in target is returned by the function whether or not
 * the replacement occurred.
 */
static __attribute__ ((always_inline)) inline uint64_t
__collector_cas_64p (volatile uint64_t *mem, uint64_t *old, uint64_t * new)
{
  uint64_t r;
#if WSIZE(32)
  uint32_t old1 = (uint32_t) (*old & 0xFFFFFFFFL);
  uint32_t old2 = (uint32_t) ((*old >> 32) & 0xFFFFFFFFL);
  uint32_t new1 = (uint32_t) (*new & 0xFFFFFFFFL);
  uint32_t new2 = (uint32_t) ((*new >> 32) & 0xFFFFFFFFL);
  uint32_t res1 = 0;
  uint32_t res2 = 0;
  __asm__ __volatile__(
      "movl %3, %%esi; lock; cmpxchg8b (%%esi); movl %%edx, %2; movl %%eax, %1"
      : "=m" (r), "=m" (res1), "=m" (res2) /* output */
      : "m" (mem), "a" (old1), "d" (old2), "b" (new1), "c" (new2) /* input */
      : "memory", "cc", "esi" //, "edx", "ecx", "ebx", "eax" /* clobbered register */
		       );
  r = (((uint64_t) res2) << 32) | ((uint64_t) res1);
#else
  __asm__ __volatile__( "lock; cmpxchgq %2, %1"
		       : "=a" (r), "=m" (*mem) /* output */
		       : "r" (*new), "a" (*old), "m" (*mem) /* input */
		       : "%rcx", "rdx" /* clobbered register */
		       );
#endif
  return r;
}
/**
 * This function enables a compare and swap operation to occur atomically.
 * The 32-/64-bit value stored in target is compared with "cmp". If these values
 * are equal, the value stored in target is replaced with "new".
 * The old value stored in target is returned by the function whether or not
 * the replacement occurred.
 */
static __attribute__ ((always_inline)) inline void *
__collector_cas_ptr (void *mem, void *cmp, void *new)
{
  void *r;
#if WSIZE(32) || defined(__ILP32__)
  r = (void *) __collector_cas_32 ((volatile uint32_t *)mem, (uint32_t) cmp, (uint32_t)new);
#else
  __asm__ __volatile__("lock; cmpxchgq %2, (%1)"
		       : "=a" (r), "=b" (mem) /* output */
		       : "r" (new), "a" (cmp), "b" (mem) /* input */
		       );
#endif
  return r;
}

#elif ARCH(Aarch64)
static __attribute__ ((always_inline)) inline uint32_t
__collector_inc_32 (volatile uint32_t *ptr)
{
  return __sync_add_and_fetch (ptr, 1);
}

static __attribute__ ((always_inline)) inline uint32_t
__collector_dec_32 (volatile uint32_t *ptr)
{
  return __sync_sub_and_fetch (ptr, 1);
}

static __attribute__ ((always_inline)) inline uint32_t
__collector_subget_32 (volatile uint32_t *ptr, uint32_t off)
{
  return __sync_sub_and_fetch (ptr, off);
}

static __attribute__ ((always_inline)) inline uint32_t
__collector_cas_32 (volatile uint32_t *ptr, uint32_t old, uint32_t new)
{
  return __sync_val_compare_and_swap (ptr, old, new);
}

static __attribute__ ((always_inline)) inline uint64_t
__collector_cas_64p (volatile uint64_t *ptr, uint64_t *old, uint64_t * new)
{
  return __sync_val_compare_and_swap (ptr, *old, *new);
}

static __attribute__ ((always_inline)) inline void *
__collector_cas_ptr (void *ptr, void *old, void *new)
{
  return (void *) __sync_val_compare_and_swap ((unsigned long *) ptr, (unsigned long) old, (unsigned long) new);
}

#else
extern void __collector_flushw (); /* defined for SPARC only */
extern void* __collector_getpc ();
extern void* __collector_getsp ();
extern void* __collector_getfp ();
extern void __collector_inc_32 (volatile uint32_t *);
extern void __collector_dec_32 (volatile uint32_t *);
extern void* __collector_cas_ptr (volatile void *, void *, void *);
extern uint32_t __collector_cas_32 (volatile uint32_t *, uint32_t, uint32_t);
extern uint32_t __collector_subget_32 (volatile uint32_t *, uint32_t);
extern uint64_t __collector_cas_64p (volatile uint64_t *, uint64_t *, uint64_t *);
#endif /* ARCH() */
#endif /* _LIBCOL_UTIL_H */
