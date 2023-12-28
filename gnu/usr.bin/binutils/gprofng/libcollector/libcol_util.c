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

#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <dlfcn.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "gp-defs.h"
#include "collector.h"
#include "libcol_util.h"
#include "gp-experiment.h"
#include "Emsgnum.h"
#include "memmgr.h"  // __collector_allocCSize, __collector_freeCSize
#include "tsd.h"

/*
 *    This file is intended for collector's own implementation of
 *    various routines to avoid interaction with libc and other
 *    libraries.
 */

/* -------  libc interface ----------------- */
CollectorUtilFuncs __collector_util_funcs = {NULL};
int __collector_dlsym_guard = 0;
int(*__collector_sscanfp)(const char *restrict s, const char *restrict fmt, ...);

/*
 * We have calls on Solaris to get the thread ID.
 * On Linux, there is a gettid() system call.
 * From user space, we have to use syscall(__NR_gettid).
 * The call is probably fast (with the tid in vdso), but dbx intercepts the syscall.
 *     7182047 syscall() has large overhead under dbx on linux
 * One option is to use an assembly call to get the tid.
 * We know how to do this on x86, but not on SPARC.
 * So another option is to do the syscall once and cache the result in thread-local storage.
 * This solves the SPARC case.
 * On x86 we could use one or both strategies.  So there are opportunities here to simplify the code.
 */
static unsigned gettid_key = COLLECTOR_TSD_INVALID_KEY;

void
__collector_ext_gettid_tsd_create_key ()
{
  gettid_key = __collector_tsd_create_key (sizeof (pid_t), NULL, NULL);
}

pid_t
__collector_gettid ()
{
  pid_t *tid_ptr = (pid_t *) __collector_tsd_get_by_key (gettid_key);
  // check if we have a thread-specific tid and if it's been initialized
  // (it's 0 before initialization and cannot be 0 after since pid 0 is the boot process)
  if (tid_ptr && *tid_ptr > 0)
    return *tid_ptr;
  pid_t r;

#if ARCH(Intel)
#if WSIZE(32)
#define syscall_instr          "int $0x80"
#define syscall_clobber        "memory"
#else //WSIZE(64)
#define syscall_instr          "syscall"
#define syscall_clobber        "rcx", "r11", "memory"
#endif
  __asm__ __volatile__(syscall_instr
		       : "=a" (r) : "0" (__NR_gettid)
		       : syscall_clobber);
#else
  r = syscall (__NR_gettid);
#endif
  if (tid_ptr)
    *tid_ptr = r;
  return r;
}

static inline int
atomic_swap (volatile int * p, int v)
{
#if ARCH(Intel)
  int r;
  __asm__ __volatile__("xchg %1, %2" : "=r" (r) : "m" (*p), "0" (v));
  return r;
#else
  /* Since the inline templates perfan/libcollector/src/inline.*.il all
   * have implementations for __collector_cas_32(), how about we just
   * use that interface for Intel as well and drop the "#if ARCH()" stuff here?
   *
   * As it is, we're using an atomic swap on Intel and
   * compare-and-swap on SPARC.  The semantics are different
   * (cas requires an expected "compare" value and swaps ONLY
   * if we match that value).  Nevertheless, the results of the
   * two operations
   *     Intel:  atomic_swap(&lock,  1)
   *     SPARC:          cas(&lock,0,1)
   * happen to be the same for the two cases we're interested in:
   *     if lock==0  lock=1 return 0
   *     if lock==1  lock=1 return 1
   * You CANNOT always simply substitute cas for swap.
   */
  return __collector_cas_32 ((volatile uint32_t *)p, 0, v);
#endif
}

int
__collector_mutex_lock (collector_mutex_t *lock_var)
{
  volatile unsigned int i = 0; /* xxxx volatile may not be honored on amd64 -x04 */

  if (!(*lock_var) && !atomic_swap (lock_var, 1))
    return 0;

  do
    {
      while ((collector_mutex_t) (*lock_var) == 1)
	i++;
    }
  while (atomic_swap (lock_var, 1));
  return 0;
}

int
__collector_mutex_trylock (collector_mutex_t *lock_var)
{
  if (!(*lock_var) && !atomic_swap (lock_var, 1))
    return 0;
  return EBUSY;
}

int
__collector_mutex_unlock (collector_mutex_t *lock_var)
{
  (*lock_var) = 0;
  return 0;
}

#if ARCH(SPARC)
void
__collector_inc_32 (volatile uint32_t *mem)
{
  uint32_t t1, t2;
  __asm__ __volatile__("	ld	%2,%0 \n"
		       "1:	add	%0,1,%1 \n"
		       "	cas	%2,%0,%1 \n"
		       "	cmp	%0,%1 \n"
		       "	bne,a	1b \n"
		       "	mov	%1,%0 \n"
		       : "=&r" (t1), "=&r" (t2)
		       : "m" (*mem)
		       : "cc"
		       );
}

void
__collector_dec_32 (volatile uint32_t *mem)
{
  uint32_t t1, t2;
  __asm__ __volatile__("	ld	%2,%0 \n"
		       "1:	sub	%0,1,%1 \n"
		       "	cas	%2,%0,%1 \n"
		       "	cmp	%0,%1 \n"
		       "	bne,a	1b \n"
		       "	mov	%1,%0 \n"
		       : "=&r" (t1), "=&r" (t2)
		       : "m" (*mem)
		       : "cc"
		       );
}

uint32_t
__collector_cas_32 (volatile uint32_t *mem, uint32_t old, uint32_t new)
{
  __asm__ __volatile__("cas [%1],%2,%0"
		       : "+r" (new)
		       : "r" (mem), "r" (old));
  return new;
}

uint32_t
__collector_subget_32 (volatile uint32_t *mem, uint32_t val)
{
  uint32_t t1, t2;
  __asm__ __volatile__("	ld	%2,%0 \n"
		       "1:	sub	%0,%3,%1 \n"
		       "	cas	%2,%0,%1 \n"
		       "	cmp	%0,%1 \n"
		       "	bne,a	1b \n"
		       "	mov	%1,%0 \n"
		       "	sub	%0,%3,%1 \n"
		       : "=&r" (t1), "=&r" (t2)
		       : "m" (*mem), "r" (val)
		       : "cc"
		       );
  return t2;
}

#if WSIZE(32)

void *
__collector_cas_ptr (volatile void *mem, void *old, void *new)
{
  __asm__ __volatile__("cas [%1],%2,%0"
		       : "+r" (new)
		       : "r" (mem), "r" (old));
  return new;
}

uint64_t
__collector_cas_64p (volatile uint64_t *mem, uint64_t *old, uint64_t *new)
{
  uint64_t t;
  __asm__ __volatile__("	ldx	[%2],%2 \n"
		       "	ldx	[%3],%3 \n"
		       "	casx	[%1],%2,%3 \n"
		       "	stx	%3,%0 \n"
		       : "=m" (t)
		       : "r" (mem), "r" (old), "r" (new)
		       );
  return t;
}

#elif WSIZE(64)

void *
__collector_cas_ptr (volatile void *mem, void *old, void *new)
{
  __asm__ __volatile__("casx [%1],%2,%0"
		       : "+r" (new)
		       : "r" (mem), "r" (old));
  return new;
}

uint64_t
__collector_cas_64p (volatile uint64_t *mem, uint64_t *old, uint64_t *new)
{
  uint64_t t;
  __asm__ __volatile__("	ldx	[%2],%2 \n"
		       "	ldx	[%3],%3 \n"
		       "	casx	[%1],%2,%3 \n"
		       "	mov	%3,%0 \n"
		       : "=&r" (t)
		       : "r" (mem), "r" (old), "r" (new)
		       );
  return t;
}

#endif /* WSIZE() */
#endif /* ARCH() */

void *
__collector_memcpy (void *s1, const void *s2, size_t n)
{
  char *cp1 = (char*) s1;
  char *cp2 = (char*) s2;
  while (n--)
    *cp1++ = *cp2++;
  return s1;
}

static void *
collector_memset (void *s, int c, size_t n)
{
  unsigned char *s1 = s;
  while (n--)
    *s1++ = (unsigned char) c;
  return s;
}

int
__collector_strcmp (const char *s1, const char *s2)
{
  for (;;)
    {
      if (*s1 != *s2)
	return *s1 - *s2;
      if (*s1 == 0)
	return 0;
      s1++;
      s2++;
    }
}

int
__collector_strncmp (const char *s1, const char *s2, size_t n)
{
  while (n > 0)
    {
      if (*s1 != *s2)
	return *s1 - *s2;
      if (*s1 == 0)
	return 0;
      s1++;
      s2++;
      n--;
    }
  return 0;
}

char *
__collector_strstr (const char *s1, const char *s2)
{
  if (s2 == NULL || *s2 == 0)
    return NULL;
  size_t len = __collector_strlen (s2);
  for (char c = *s2; *s1; s1++)
    if (c == *s1 && __collector_strncmp (s1, s2, len) == 0)
      return (char *) s1;
  return NULL;
}

char *
__collector_strchr (const char *str, int chr)
{
  if (chr == '\0')
    return (char *) (str + __collector_strlen (str));
  for (; *str; str++)
    if (chr == (int) *str)
      return (char *) str;
  return NULL;
}

char *
__collector_strrchr (const char *str, int chr)
{
  const char *p = str + __collector_strlen (str);
  for (; p - str >= 0; p--)
    if (chr == *p)
      return (char *) p;
  return NULL;
}

int
__collector_strStartWith (const char *s1, const char *s2)
{
  size_t slen = __collector_strlen (s2);
  return __collector_strncmp (s1, s2, slen);
}

size_t
__collector_strlen (const char *s)
{
  int len = -1;
  while (s[++len] != '\0')
    ;
  return len;
}

size_t
__collector_strlcpy (char *dst, const char *src, size_t dstsize)
{
  size_t srcsize = 0;
  size_t n = dstsize - 1;
  char c;
  while ((c = *src++) != 0)
    if (srcsize++ < n)
      *dst++ = c;
  if (dstsize > 0)
    *dst = '\0';
  return srcsize;
}

size_t
__collector_strncpy (char *dst, const char *src, size_t dstsize)
{
  size_t i;
  for (i = 0; i < dstsize; i++)
    {
      dst[i] = src[i];
      if (src[i] == '\0')
	break;
    }
  return i;
}

char *
__collector_strcat (char *dst, const char *src)
{
  size_t sz = __collector_strlen (dst);
  for (size_t i = 0;; i++)
    {
      dst[sz + i] = src[i];
      if (src[i] == '\0')
	break;
    }
  return dst;
}

size_t
__collector_strlcat (char *dst, const char *src, size_t dstsize)
{
  size_t sz = __collector_strlen (dst);
  return sz + __collector_strlcpy (dst + sz, src, dstsize - sz);
}

void *
__collector_malloc (size_t size)
{
  void * ptr = __collector_allocCSize (__collector_heap, size, 0);
  return ptr;
}

void *
__collector_calloc (size_t nelem, size_t elsize)
{
  size_t n = nelem * elsize;
  void * ptr = __collector_malloc (n);
  if (NULL == ptr)
    return NULL;
  collector_memset (ptr, 0, n);
  return ptr;
}

char *
__collector_strdup (const char * str)
{
  if (NULL == str)
    return NULL;
  size_t size = __collector_strlen (str);
  char * dst = (char *) __collector_malloc (size + 1);
  if (NULL == dst)
    return NULL;
  __collector_strncpy (dst, str, size + 1);
  return dst;
}

#define C_FMT 1
#define C_STR 2
static char
Printable[256] = {//characters should be escaped by xml: "'<>&
  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,   /* ................ */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* ................ */
  3, 3, 1, 3, 3, 3, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3,   /*  !"#$%&'()*+,-./ */
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 3, 1, 3,   /* 0123456789:;<=>? */
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,   /* @ABCDEFGHIJKLMNO */
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,   /* PQRSTUVWXYZ[\]^_ */
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,   /* `abcdefghijklmno */
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0,   /* pqrstuvwxyz{|}~. */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* ................ */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* ................ */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* ................ */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* ................ */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* ................ */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* ................ */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* ................ */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0    /* ................ */
};
static char hex[17] = "0123456789abcdef";
static char HEX[17] = "0123456789ABCDEF";

int
__collector_xml_snprintf (char *s, size_t n, const char *format, ...)
{
  va_list args;
  va_start (args, format);
  int res = __collector_xml_vsnprintf (s, n, format, args);
  va_end (args);
  return res;
}

int
__collector_xml_vsnprintf (char *s, size_t n, const char *format, va_list args)
{
  const char *src = format;
  char *dst = s;
  int cnt = 0;
  unsigned char c;
  while ((c = *src) != 0)
    {
      if (c == '%')
	{
	  char numbuf[32];
	  int done = 0;
	  int jflag = 0;
	  int lflag = 0;
	  int zflag = 0;
	  int width = 0;
	  src++;
	  while (!done)
	    {
	      c = *src;
	      switch (c)
		{
		case '%':
		  {
		    if (cnt++ < n - 1)
		      *dst++ = '%';
		    if (cnt++ < n - 1)
		      *dst++ = hex[c / 16];
		    if (cnt++ < n - 1)
		      *dst++ = hex[c % 16];
		    if (cnt++ < n - 1)
		      *dst++ = '%';
		    src++;
		    done = 1;
		    break;
		  }
		case '-':
		  {
		    if (jflag != 0)
		      done = 1;
		    else
		      {
			jflag = 1;
			src++;
		      }
		    break;
		  }
		case 'l':
		  {
		    if (lflag != 0)
		      done = 1;
		    else
		      {
			lflag = 1;
			c = *++src;
			if (c == 'l')
			  {
			    lflag++;
			    src++;
			  }
		      }
		    break;
		  }
		case 'c':
		  {
		    unsigned char c1 = (unsigned char) va_arg (args, int);
		    if ((Printable[(int) c1] & C_STR) == 0)
		      {
			if (c1 == '"')
			  {//&quot;
			    if (cnt++ < n - 1)
			      *dst++ = '&';
			    if (cnt++ < n - 1)
			      *dst++ = 'q';
			    if (cnt++ < n - 1)
			      *dst++ = 'u';
			    if (cnt++ < n - 1)
			      *dst++ = 'o';
			    if (cnt++ < n - 1)
			      *dst++ = 't';
			    if (cnt++ < n - 1)
			      *dst++ = ';';
			  }
			else if (c1 == '\'')
			  {//&apos;
			    if (cnt++ < n - 1)
			      *dst++ = '&';
			    if (cnt++ < n - 1)
			      *dst++ = 'a';
			    if (cnt++ < n - 1)
			      *dst++ = 'p';
			    if (cnt++ < n - 1)
			      *dst++ = 'o';
			    if (cnt++ < n - 1)
			      *dst++ = 's';
			    if (cnt++ < n - 1)
			      *dst++ = ';';
			  }
			else if (c1 == '&')
			  {//&amp;
			    if (cnt++ < n - 1)
			      *dst++ = '&';
			    if (cnt++ < n - 1)
			      *dst++ = 'a';
			    if (cnt++ < n - 1)
			      *dst++ = 'm';
			    if (cnt++ < n - 1)
			      *dst++ = 'p';
			    if (cnt++ < n - 1)
			      *dst++ = ';';
			  }
			else if (c1 == '<')
			  {//&lt;
			    if (cnt++ < n - 1)
			      *dst++ = '&';
			    if (cnt++ < n - 1)
			      *dst++ = 'l';
			    if (cnt++ < n - 1)
			      *dst++ = 't';
			    if (cnt++ < n - 1)
			      *dst++ = ';';
			  }
			else if (c1 == '>')
			  {//&gt;
			    if (cnt++ < n - 1)
			      *dst++ = '&';
			    if (cnt++ < n - 1)
			      *dst++ = 'g';
			    if (cnt++ < n - 1)
			      *dst++ = 't';
			    if (cnt++ < n - 1)
			      *dst++ = ';';
			  }
			else
			  {
			    if (cnt++ < n - 1)
			      *dst++ = '%';
			    if (cnt++ < n - 1)
			      *dst++ = hex[c1 / 16];
			    if (cnt++ < n - 1)
			      *dst++ = hex[c1 % 16];
			    if (cnt++ < n - 1)
			      *dst++ = '%';
			  }
		      }
		    else if (cnt++ < n - 1)
		      *dst++ = c1;
		    src++;
		    done = 1;
		    break;
		  }
		case 's':
		  {
		    /* Strings are always left justified */
		    char *str = va_arg (args, char*);
		    if (!str)
		      str = "<NULL>";
		    unsigned char c1;
		    while ((c1 = *str++) != 0)
		      {
			if ((Printable[(int) c1] & C_STR) == 0)
			  {
			    if (c1 == '"')
			      {//&quot;
				if (cnt++ < n - 1)
				  *dst++ = '&';
				if (cnt++ < n - 1)
				  *dst++ = 'q';
				if (cnt++ < n - 1)
				  *dst++ = 'u';
				if (cnt++ < n - 1)
				  *dst++ = 'o';
				if (cnt++ < n - 1)
				  *dst++ = 't';
				if (cnt++ < n - 1)
				  *dst++ = ';';
			      }
			    else if (c1 == '\'')
			      {//&apos;
				if (cnt++ < n - 1)
				  *dst++ = '&';
				if (cnt++ < n - 1)
				  *dst++ = 'a';
				if (cnt++ < n - 1)
				  *dst++ = 'p';
				if (cnt++ < n - 1)
				  *dst++ = 'o';
				if (cnt++ < n - 1)
				  *dst++ = 's';
				if (cnt++ < n - 1)
				  *dst++ = ';';
			      }
			    else if (c1 == '&')
			      {//&amp;
				if (cnt++ < n - 1)
				  *dst++ = '&';
				if (cnt++ < n - 1)
				  *dst++ = 'a';
				if (cnt++ < n - 1)
				  *dst++ = 'm';
				if (cnt++ < n - 1)
				  *dst++ = 'p';
				if (cnt++ < n - 1)
				  *dst++ = ';';
			      }
			    else if (c1 == '<')
			      {//&lt;
				if (cnt++ < n - 1)
				  *dst++ = '&';
				if (cnt++ < n - 1)
				  *dst++ = 'l';
				if (cnt++ < n - 1)
				  *dst++ = 't';
				if (cnt++ < n - 1)
				  *dst++ = ';';
			      }
			    else if (c1 == '>')
			      {//&gt;
				if (cnt++ < n - 1)
				  *dst++ = '&';
				if (cnt++ < n - 1)
				  *dst++ = 'g';
				if (cnt++ < n - 1)
				  *dst++ = 't';
				if (cnt++ < n - 1)
				  *dst++ = ';';
			      }
			    else
			      {
				if (cnt++ < n - 1)
				  *dst++ = '%';
				if (cnt++ < n - 1)
				  *dst++ = hex[c1 / 16];
				if (cnt++ < n - 1)
				  *dst++ = hex[c1 % 16];
				if (cnt++ < n - 1)
				  *dst++ = '%';
			      }
			  }
			else if (cnt++ < n - 1)
			  *dst++ = c1;
			width--;
		      }
		    while (width > 0)
		      {
			if (cnt++ < n - 1)
			  *dst++ = ' ';
			width--;
		      }
		    src++;
		    done = 1;
		    break;
		  }
		case 'i':
		case 'd':
		case 'o':
		case 'p':
		case 'u':
		case 'x':
		case 'X':
		  {
		    int base = 10;
		    int uflag = 0;
		    int sflag = 0;
		    if (c == 'o')
		      {
			uflag = 1;
			base = 8;
		      }
		    else if (c == 'u')
		      uflag = 1;
		    else if (c == 'p')
		      {
			lflag = 1;
			uflag = 1;
			base = 16;
		      }
		    else if (c == 'x' || c == 'X')
		      {
			uflag = 1;
			base = 16;
		      }
		    long long argll = 0LL;
		    if (lflag == 0)
		      {
			if (uflag)
			  argll = va_arg (args, unsigned int);
			else
			  argll = va_arg (args, int);
		      }
		    else if (lflag == 1)
		      {
			if (uflag)
			  argll = va_arg (args, unsigned long);
			else
			  argll = va_arg (args, long);
		      }
		    else if (lflag == 2)
		      argll = va_arg (args, long long);
		    unsigned long long argllu = 0ULL;
		    if (uflag || argll >= 0)
		      argllu = argll;
		    else
		      {
			sflag = 1;
			argllu = -argll;
		      }
		    int idx = sizeof (numbuf);
		    do
		      {
			numbuf[--idx] = (c == 'X' ? HEX[argllu % base] : hex[argllu % base]);
			argllu = argllu / base;
		      }
		    while (argllu != 0)
		      ;
		    if (sflag)
		      {
			if (jflag || zflag)
			  {
			    if (cnt++ < n - 1)
			      *dst++ = '-';
			  }
			else
			  numbuf[--idx] = '-';
		      }

		    if (jflag)
		      {
			while (idx < sizeof (numbuf) && width > 0)
			  {
			    if (cnt++ < n - 1)
			      *dst++ = numbuf[idx];
			    idx++;
			    width--;
			  }
			zflag = 0;
		      }

		    while (width > sizeof (numbuf) - idx)
		      {
			if (cnt++ < n - 1)
			  *dst++ = zflag ? '0' : ' ';
			width--;
		      }
		    while (idx != sizeof (numbuf))
		      {
			if (cnt++ < n - 1)
			  *dst++ = numbuf[idx];
			idx++;
		      }
		    src++;
		    done = 1;
		    break;
		  }
		case '0':
		  zflag = 1;
		case '1': case '2': case '3': case '4': case '5':
		case '6': case '7': case '8': case '9':
		  {
		    while (c >= '0' && c <= '9')
		      {
			width = width * 10 + (c - '0');
			c = *++src;
		      }
		    break;
		  }
		default:
		  done = 1;
		  break;
		}
	    }
	}
      else if ((Printable[(int) c] & C_FMT) == 0)
	{
	  if (cnt++ < n - 1)
	    *dst++ = '%';
	  if (cnt++ < n - 1)
	    *dst++ = hex[c / 16];
	  if (cnt++ < n - 1)
	    *dst++ = hex[c % 16];
	  if (cnt++ < n - 1)
	    *dst++ = '%';
	  src++;
	}
      else
	{
	  if (cnt++ < n - 1)
	    *dst++ = c;
	  src++;
	}
    }

  if (cnt < n - 1)
    s[cnt] = '\0';
  else
    s[n - 1] = '\0';

  return cnt;
}

/*
 *    Functions to be called directly from libc.so
 */
#if ARCH(Intel)    /* intel-Linux */
/*
 * The CPUIDinfo/__collector_cpuid() code is old,
 * incorrect, and complicated.  It returns the apicid
 * rather than the processor number.
 *
 * Unfortunately, the higher-level sched_getcpu() function,
 * which we use on SPARC-Linux, is not available on Oracle
 * Linux 5.  So we have to test for its existence.
 */

/* a pointer to sched_getcpu(), in case we find it */
typedef int (*sched_getcpu_ptr_t)(void);
sched_getcpu_ptr_t sched_getcpu_ptr;
static int need_warning = 0;

/* the old, low-level code */
static int useLeafB = 0;

/* access to the CPUID instruction on Intel/AMD */
typedef struct
{
  uint32_t eax, ebx, ecx, edx;
} CPUIDinfo;

/**
 * This function returns the result of the "cpuid" instruction
 */
static __attribute__ ((always_inline)) inline void
__collector_cpuid (CPUIDinfo* info)
{
  uint32_t ebx = info->ebx, ecx = info->ecx, edx = info->edx, eax = info->eax;
  __asm__ ("cpuid" : "=b" (ebx), "=c" (ecx), "=d" (edx), "=a" (eax) : "a" (eax));
  info->eax = eax;
  info->ebx = ebx;
  info->ecx = ecx;
  info->edx = edx;
}

static void
getcpuid_init ()
{
  CPUIDinfo info;
  info.eax = 0; /* max input value for CPUID */
  __collector_cpuid (&info);

  if (info.eax >= 0xb)
    {
      info.eax = 0xb;
      info.ecx = 0;
      __collector_cpuid (&info);
      useLeafB = info.ebx != 0;
    }

  /* indicate that we need a warning */
  /* (need to wait until log mechanism has been initialized) */
  need_warning = 1;
}

static uint32_t
getcpuid ()
{
  /* if we found sched_getcpu(), use it */
  if (sched_getcpu_ptr)
    return (*sched_getcpu_ptr)();

  /* otherwise, check if we need warning */
  if (need_warning)
    {
      if (useLeafB)
	(void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">x2APIC</event>\n",
				      SP_JCMD_CWARN, COL_WARN_LINUX_X86_APICID);
      else
	(void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">APIC</event>\n",
				      SP_JCMD_CWARN, COL_WARN_LINUX_X86_APICID);
      need_warning = 0;
    }

  /* and use the old, low-level code */
  CPUIDinfo info;
  if (useLeafB)
    {
      info.eax = 0xb;
      info.ecx = 0;
      __collector_cpuid (&info);
      return info.edx; /*  x2APIC ID */
    }
  else
    {
      info.eax = 0x1;
      info.ecx = 0;
      __collector_cpuid (&info);
      return info.ebx >> 24; /* APIC ID */
    }
}

#else /* sparc-Linux */

/*
 * EUGENE
 * How should sched_getcpu() be prototyped?  Like this?
 *     #include <sched.h>
 * Or like this?
 *     #define _GNU_SOURCE
 *     #include <utmpx.h>
 * Or just prototype this function explicitly without bothering with include files.
 */
int sched_getcpu ();

static int
getcpuid ()
{
  return sched_getcpu ();
}
#endif

/* if ever retries time-out, we will stop allowing them */
static int exhausted_retries = 0;

int
__collector_open (const char *path, int oflag, ...)
{
  int fd;
  mode_t mode = 0;

  hrtime_t t_timeout = __collector_gethrtime () + 5 * ((hrtime_t) NANOSEC);
  int nretries = 0;
  long long delay = 100; /* start at some small, arbitrary value */

  /* get optional mode argument if it's expected/required */
  if (oflag | O_CREAT)
    {
      va_list ap;
      va_start (ap, oflag);
      mode = (mode_t) va_arg (ap, mode_t);
      va_end (ap);
    }

  /* retry upon failure */
  while ((fd = CALL_UTIL (open_bare)(path, oflag, mode)) < 0)
    {
      if (exhausted_retries)
	break;

      /* The particular condition we're willing to retry is if
       * too many file descriptors were in use.  The errno should
       * be EMFILE, but apparently and mysteriously it can also be
       * and often is ENOENT.
       */
      if ((errno != EMFILE) && (errno != ENOENT))
	break;
      if (__collector_gethrtime () > t_timeout)
	{
	  exhausted_retries = 1;
	  break;
	}

      /* Oddly, if I replace this spin wait with
       *   -  a usleep() call or
       *   -  a loop on gethrtime() calls
       * for roughly the same length of time, retries aren't very effective. */
      int ispin;
      double xdummy = 0.5;
      for (ispin = 0; ispin < delay; ispin++)
	xdummy = 0.5 * (xdummy + 1.);
      if (xdummy < 0.1)
	/* should never happen, but we check so the loop won't be optimized away */
	break;
      delay *= 2;
      if (delay > 100000000)
	delay = 100000000; /* cap at some large, arbitrary value */
      nretries++;
    }
  return fd;
}

int
__collector_util_init ()
{
  int oldos = 0;

  /* Linux requires RTLD_LAZY, Solaris can do just RTLD_NOLOAD */
  void *libc = dlopen (SYS_LIBC_NAME, RTLD_LAZY | RTLD_NOLOAD);
  if (libc == NULL)
    libc = dlopen (SYS_LIBC_NAME, RTLD_NOW | RTLD_LOCAL);
  if (libc == NULL)
    {
      /* libcollector will subsequently abort, as all the pointers in the vector are NULL */
#if 0
      /* SP_COLLECTOR_TRACELEVEL is not yet set, so no Tprintf */
      fprintf (stderr, "__collector_util_init: dlopen(%s) failed: %s\n", SYS_LIBC_NAME, dlerror ());
      return COL_ERROR_UTIL_INIT;
#endif
      abort ();
    }

  void *ptr = dlsym (libc, "fprintf");
  if (ptr)
    __collector_util_funcs.fprintf = (int(*)(FILE *, const char *, ...))ptr;
  else
    {
      // We can't write any error messages without a libc reference
#if 0
      fprintf (stderr, "__collector_util_init: COLERROR_UTIL_INIT fprintf: %s\n", dlerror ());
      return COL_ERROR_UTIL_INIT;
#endif
      abort ();
    }
  int err = 0;

  ptr = dlsym (libc, "mmap");
  if (ptr)
    __collector_util_funcs.mmap = (void*(*)(void *, size_t, int, int, int, off_t))ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT mmap: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  /* mmap64 is only in 32-bits; this call goes to mmap in 64-bits */
  /*    internal calls for mapping in libcollector call mmap64 */
  ptr = dlsym (libc, "mmap64");
  if (ptr)
    __collector_util_funcs.mmap64_ = (void*(*)(void *, size_t, int, int, int, off_t))ptr;
  else
    __collector_util_funcs.mmap64_ = __collector_util_funcs.mmap;

  ptr = dlsym (libc, "munmap");
  if (ptr)
    __collector_util_funcs.munmap = (int(*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT munmap: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "close");
  if (ptr)
    __collector_util_funcs.close = (int(*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT close: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "open");
  if (ptr)
    __collector_util_funcs.open = (int(*)(const char *path, int oflag, ...))ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT open: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

#if ARCH(Intel) && WSIZE(32)
  ptr = dlvsym (libc, "open64", "GLIBC_2.2"); // it is in /lib/libpthread.so.0
  if (ptr)
    __collector_util_funcs.open_bare = (int(*)(const char *path, int oflag, ...))ptr;
  else
    {
      Tprintf (DBG_LT0, "libcol_util: WARNING: dlvsym for %s@%s failed. Using dlsym() instead.", "open64", "GLIBC_2.2");
#endif /* ARCH(Intel) && WSIZE(32) */
      ptr = dlsym (libc, "open64");
      if (ptr)
	__collector_util_funcs.open_bare = (int(*)(const char *path, int oflag, ...))ptr;
      else
	__collector_util_funcs.open_bare = __collector_util_funcs.open;
#if ARCH(Intel) && WSIZE(32)
    }
#endif /* ARCH(Intel) && WSIZE(32) */

  ptr = dlsym (libc, "close");
  if (ptr)
    __collector_util_funcs.close = (int(*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT close: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "read");
  if (ptr)
    __collector_util_funcs.read = (ssize_t (*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT read: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "write");
  if (ptr)
    __collector_util_funcs.write = (ssize_t (*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT write: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

#if ARCH(Intel) && WSIZE(32)
  ptr = dlvsym (libc, "pwrite", "GLIBC_2.2"); // it is in /lib/libpthread.so.0
  if (ptr)
    __collector_util_funcs.pwrite = (ssize_t (*)())ptr;
  else
    {
      Tprintf (DBG_LT0, "libcol_util: WARNING: dlvsym for %s@%s failed. Using dlsym() instead.", "pwrite", "GLIBC_2.2");
#endif /* ARCH(Intel) && WSIZE(32) */
      ptr = dlsym (libc, "pwrite");
      if (ptr)
	__collector_util_funcs.pwrite = (ssize_t (*)())ptr;
      else
	{
	  CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT pwrite: %s\n", dlerror ());
	  err = COL_ERROR_UTIL_INIT;
	}
#if ARCH(Intel) && WSIZE(32)
    }
#endif

#if ARCH(Intel) && WSIZE(32)
  ptr = dlvsym (libc, "pwrite64", "GLIBC_2.2"); // it is in /lib/libpthread.so.0
  if (ptr)
    __collector_util_funcs.pwrite64_ = (ssize_t (*)())ptr;
  else
    {
      Tprintf (DBG_LT0, "libcol_util: WARNING: dlvsym for %s@%s failed. Using dlsym() instead.", "pwrite64", "GLIBC_2.2");
#endif /* ARCH(Intel) && WSIZE(32) */
      ptr = dlsym (libc, "pwrite64");
      if (ptr)
	__collector_util_funcs.pwrite64_ = (ssize_t (*)())ptr;
      else
	__collector_util_funcs.pwrite64_ = __collector_util_funcs.pwrite;
#if ARCH(Intel) && WSIZE(32)
    }
#endif /* ARCH(Intel) && WSIZE(32) */

  ptr = dlsym (libc, "lseek");
  if (ptr)
    __collector_util_funcs.lseek = (off_t (*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT lseek: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "access");
  if (ptr)
    __collector_util_funcs.access = (int(*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT access: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "mkdir");
  if (ptr)
    __collector_util_funcs.mkdir = (int(*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT mkdir: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "opendir");
  if (ptr)
    __collector_util_funcs.opendir = (DIR * (*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT opendir: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "closedir");
  if (ptr)
    __collector_util_funcs.closedir = (int(*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT closedir: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "execv");
  if (ptr)
    __collector_util_funcs.execv = (int(*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT execv: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "exit");
  if (ptr)
    __collector_util_funcs.exit = (void(*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT exit: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "vfork");
  if (ptr)
    __collector_util_funcs.vfork = (pid_t (*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT vfork: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "waitpid");
  if (ptr)
    __collector_util_funcs.waitpid = (pid_t (*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT waitpid: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  int (*__collector_getcpuid)() = (int(*)()) & getcpuid;
#if ARCH(Intel)
  /* if sched_getcpu() not found, init our getcpuid() */
  sched_getcpu_ptr = (sched_getcpu_ptr_t) dlsym (libc, "sched_getcpu");
  if (sched_getcpu_ptr == NULL)
    getcpuid_init ();
#endif
  __collector_util_funcs.getcpuid = __collector_getcpuid;
  __collector_util_funcs.memset = collector_memset;

  ptr = dlsym (libc, "getcontext");
  if (ptr)
    __collector_util_funcs.getcontext = (int(*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT getcontext: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "malloc");
  if (ptr)
    __collector_util_funcs.malloc = (void *(*)(size_t))ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT malloc: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "putenv");
  if (ptr)
    __collector_util_funcs.putenv = (int(*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT putenv: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "getenv");
  if (ptr)
    __collector_util_funcs.getenv = (char*(*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT getenv: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "time");
  if (ptr)
    __collector_util_funcs.time = (time_t (*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT time: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "mktime");
  if (ptr)
    __collector_util_funcs.mktime = (time_t (*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT mktime: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  __collector_util_funcs.strcmp = __collector_strcmp;
  __collector_util_funcs.strncmp = __collector_strncmp;
  __collector_util_funcs.strncpy = __collector_strncpy;
  __collector_util_funcs.strstr = __collector_strstr;

  ptr = dlsym (libc, "gmtime_r");
  if (ptr)
    __collector_util_funcs.gmtime_r = (struct tm * (*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT gmtime_r: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "strtol");
  if (ptr)
    __collector_util_funcs.strtol = (long (*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT strtol: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "strtoll");
  if (ptr)
    __collector_util_funcs.strtoll = (long long (*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT strtoll: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  __collector_util_funcs.strchr = __collector_strchr;
  __collector_util_funcs.strrchr = __collector_strrchr;

  ptr = dlsym (libc, "setenv");
  if (ptr)
    __collector_util_funcs.setenv = (int(*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT setenv: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "unsetenv");
  if (ptr)
    __collector_util_funcs.unsetenv = (int(*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT unsetenv: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "atof");
  if (ptr)
    __collector_util_funcs.atof = (double (*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT atof: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "sysinfo");
  if (ptr)
    __collector_util_funcs.sysinfo = (long (*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT sysinfo: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "clearenv");
  if (ptr)
    __collector_util_funcs.clearenv = (int(*)())ptr;
  else
    {
      /* suppress warning on S10 or earlier Solaris */
      if (oldos == 0)
	CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT clearenv: %s\n", dlerror ());
      /* err = COL_ERROR_UTIL_INIT; */
      /* don't treat this as fatal, so that S10 could work */
    }

  if ((ptr = dlvsym (libc, "fopen", "GLIBC_2.17")) != NULL)
    __collector_util_funcs.fopen = ptr;
  else if ((ptr = dlvsym (libc, "fopen", "GLIBC_2.2.5")) != NULL)
    __collector_util_funcs.fopen = ptr;
  else if ((ptr = dlvsym (libc, "fopen", "GLIBC_2.1")) != NULL)
    __collector_util_funcs.fopen = ptr;
  else if ((ptr = dlvsym (libc, "fopen", "GLIBC_2.0")) != NULL)
    __collector_util_funcs.fopen = ptr;
  else
    ptr = dlsym (libc, "fopen");
  if (__collector_util_funcs.fopen == NULL)
    {
      CALL_UTIL (fprintf)(stderr, "COL_ERROR_UTIL_INIT fopen: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  if ((ptr = dlvsym (libc, "popen", "GLIBC_2.17")) != NULL)
    __collector_util_funcs.popen = ptr;
  else if ((ptr = dlvsym (libc, "popen", "GLIBC_2.2.5")) != NULL)
    __collector_util_funcs.popen = ptr;
  else if ((ptr = dlvsym (libc, "popen", "GLIBC_2.1")) != NULL)
    __collector_util_funcs.popen = ptr;
  else if ((ptr = dlvsym (libc, "popen", "GLIBC_2.0")) != NULL)
    __collector_util_funcs.popen = ptr;
  else
    ptr = dlsym (libc, "popen");
  if (__collector_util_funcs.popen == NULL)
    {
      CALL_UTIL (fprintf)(stderr, "COL_ERROR_UTIL_INIT popen: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  if ((ptr = dlvsym (libc, "fclose", "GLIBC_2.17")) != NULL)
    __collector_util_funcs.fclose = ptr;
  else if ((ptr = dlvsym (libc, "fclose", "GLIBC_2.2.5")) != NULL)
    __collector_util_funcs.fclose = ptr;
  else if ((ptr = dlvsym (libc, "fclose", "GLIBC_2.1")) != NULL)
    __collector_util_funcs.fclose = ptr;
  else if ((ptr = dlvsym (libc, "fclose", "GLIBC_2.0")) != NULL)
    __collector_util_funcs.fclose = ptr;
  else
    ptr = dlsym (libc, "fclose");
  if (__collector_util_funcs.fclose == NULL)
    {
      CALL_UTIL (fprintf)(stderr, "COL_ERROR_UTIL_INIT fclose: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "pclose");
  if (ptr)
    __collector_util_funcs.pclose = (int(*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT pclose: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "fgets");
  if (ptr)
    __collector_util_funcs.fgets = (char*(*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT fgets: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "sscanf");
  if (ptr)
    __collector_sscanfp = (int(*)(const char *restrict s, const char *restrict fmt, ...))ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT sscanf: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "snprintf");
  if (ptr)
    __collector_util_funcs.snprintf = (int(*)(char *, size_t, const char *, ...))ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT snprintf: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "vsnprintf");
  if (ptr)
    __collector_util_funcs.vsnprintf = (int(*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT vsnprintf: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "atoi");
  if (ptr)
    __collector_util_funcs.atoi = (int(*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT atoi: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "calloc");
  if (ptr)
    __collector_util_funcs.calloc = (void*(*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT calloc: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "free");
  if (ptr)
    {
      __collector_util_funcs.free = (void(*)())ptr;
    }
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT free: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "strdup");
  if (ptr)
    __collector_util_funcs.libc_strdup = (char*(*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT strdup: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  __collector_util_funcs.strlen = __collector_strlen;
  __collector_util_funcs.strlcat = __collector_strlcat;
  __collector_util_funcs.strlcpy = __collector_strlcpy;

  ptr = dlsym (libc, "strerror");
  if (ptr)
    __collector_util_funcs.strerror = (char*(*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT strerror: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }
  ptr = dlsym (libc, "strerror_r");
  if (ptr)
    __collector_util_funcs.strerror_r = (int(*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT strerror_r: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }
  ptr = dlsym (libc, "strspn");
  if (ptr)
    __collector_util_funcs.strspn = (size_t (*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT strspn: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "strtoul");
  if (ptr)
    __collector_util_funcs.strtoul = (unsigned long int(*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT strtoul: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "strtoull");
  if (ptr)
    __collector_util_funcs.strtoull = (unsigned long long int(*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT strtoull: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "fcntl");
  if (ptr)
    __collector_util_funcs.fcntl = (int(*)(int, int, ...))ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT fcntl: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "ioctl");
  if (ptr)
    __collector_util_funcs.ioctl = (int(*)(int, int, ...))ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT ioctl: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "symlink");
  if (ptr)
    __collector_util_funcs.symlink = (int(*)(const char*, const char*))ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT symlink: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "syscall");
  if (ptr)
    __collector_util_funcs.syscall = (int(*)(int, ...))ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT syscall: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "sysconf");
  if (ptr)
    __collector_util_funcs.sysconf = (long(*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT sysconf: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "sigfillset");
  if (ptr)
    __collector_util_funcs.sigfillset = (int(*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT sigfillset: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  ptr = dlsym (libc, "sigprocmask");
  if (ptr)
    __collector_util_funcs.sigprocmask = (int(*)())ptr;
  else
    {
      CALL_UTIL (fprintf)(stderr, "collector_util_init COL_ERROR_UTIL_INIT sigprocmask: %s\n", dlerror ());
      err = COL_ERROR_UTIL_INIT;
    }

  return err;
}
