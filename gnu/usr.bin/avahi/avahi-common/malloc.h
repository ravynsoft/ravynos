#ifndef foomallochfoo
#define foomallochfoo

/***
  This file is part of avahi.

  avahi is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  avahi is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
  Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with avahi; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

/** \file malloc.h Memory allocation */

#include <sys/types.h>
#include <stdarg.h>
#include <limits.h>
#include <assert.h>

#include <avahi-common/cdecl.h>
#include <avahi-common/gccmacro.h>

AVAHI_C_DECL_BEGIN

/** Allocate some memory, just like the libc malloc() */
void *avahi_malloc(size_t size) AVAHI_GCC_ALLOC_SIZE(1);

/** Similar to avahi_malloc() but set the memory to zero */
void *avahi_malloc0(size_t size) AVAHI_GCC_ALLOC_SIZE(1);

/** Free some memory */
void avahi_free(void *p);

/** Similar to libc's realloc() */
void *avahi_realloc(void *p, size_t size) AVAHI_GCC_ALLOC_SIZE(2);

/** Internal helper for avahi_new() */
static inline void* AVAHI_GCC_ALLOC_SIZE2(1,2) avahi_new_internal(unsigned n, size_t k) {
    assert(n < INT_MAX/k);
    return avahi_malloc(n*k);
}

/** Allocate n new structures of the specified type. */
#define avahi_new(type, n) ((type*) avahi_new_internal((n), sizeof(type)))

/** Internal helper for avahi_new0() */
static inline void* AVAHI_GCC_ALLOC_SIZE2(1,2) avahi_new0_internal(unsigned n, size_t k) {
    assert(n < INT_MAX/k);
    return avahi_malloc0(n*k);
}

/** Same as avahi_new() but set the memory to zero */
#define avahi_new0(type, n) ((type*) avahi_new0_internal((n), sizeof(type)))

/** Just like libc's strdup() */
char *avahi_strdup(const char *s);

/** Just like libc's strndup() */
char *avahi_strndup(const char *s, size_t l);

/** Duplicate the given memory block into a new one allocated with avahi_malloc() */
void *avahi_memdup(const void *s, size_t l) AVAHI_GCC_ALLOC_SIZE(2);

/** Wraps allocator functions */
typedef struct AvahiAllocator {
    void* (*malloc)(size_t size) AVAHI_GCC_ALLOC_SIZE(1);
    void (*free)(void *p);
    void* (*realloc)(void *p, size_t size) AVAHI_GCC_ALLOC_SIZE(2);
    void* (*calloc)(size_t nmemb, size_t size) AVAHI_GCC_ALLOC_SIZE2(1,2);   /**< May be NULL */
} AvahiAllocator;

/** Change the allocator. May be NULL to return to default (libc)
 * allocators. The structure is not copied! */
void avahi_set_allocator(const AvahiAllocator *a);

/** Like sprintf() but store the result in a freshly allocated buffer. Free this with avahi_free() */
char *avahi_strdup_printf(const char *fmt, ... ) AVAHI_GCC_PRINTF_ATTR12;

/** \cond fulldocs */
/** Same as avahi_strdup_printf() but take a va_list instead of varargs */
char *avahi_strdup_vprintf(const char *fmt, va_list ap);
/** \endcond */

AVAHI_C_DECL_END

#endif
