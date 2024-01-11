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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include "malloc.h"

#ifndef va_copy
#ifdef __va_copy
#define va_copy(DEST,SRC) __va_copy((DEST),(SRC))
#else
#define va_copy(DEST,SRC) memcpy(&(DEST), &(SRC), sizeof(va_list))
#endif
#endif

static const AvahiAllocator *allocator = NULL;

static void oom(void) AVAHI_GCC_NORETURN;

static void oom(void) {

    static const char msg[] = "Out of memory, aborting ...\n";
    const char *n = msg;

    while (strlen(n) > 0) {
        ssize_t r;

        if ((r = write(2, n, strlen(n))) < 0)
            break;

        n += r;
    }

    abort();
}

/* Default implementation for avahi_malloc() */
static void* xmalloc(size_t size) {
    void *p;

    if (size == 0)
        return NULL;

    if (!(p = malloc(size)))
        oom();

    return p;
}

/* Default implementation for avahi_realloc() */
static void *xrealloc(void *p, size_t size) {

    if (size == 0) {
        free(p);
        return NULL;
    }

    if (!(p = realloc(p, size)))
        oom();

    return p;
}

/* Default implementation for avahi_calloc() */
static void *xcalloc(size_t nmemb, size_t size) {
    void *p;

    if (size == 0 || nmemb == 0)
        return NULL;

    if (!(p = calloc(nmemb, size)))
        oom();

    return p;
}

void *avahi_malloc(size_t size) {

    if (size <= 0)
        return NULL;

    if (!allocator)
        return xmalloc(size);

    assert(allocator->malloc);
    return allocator->malloc(size);
}

void *avahi_malloc0(size_t size) {
    void *p;

    if (size <= 0)
        return NULL;

    if (!allocator)
        return xcalloc(1, size);

    if (allocator->calloc)
        return allocator->calloc(1, size);

    assert(allocator->malloc);
    if ((p = allocator->malloc(size)))
        memset(p, 0, size);

    return p;
}

void avahi_free(void *p) {

    if (!p)
        return;

    if (!allocator) {
        free(p);
        return;
    }

    assert(allocator->free);
    allocator->free(p);
}

void *avahi_realloc(void *p, size_t size) {

    if (size == 0) {
        avahi_free(p);
        return NULL;
    }

    if (!allocator)
        return xrealloc(p, size);

    assert(allocator->realloc);
    return allocator->realloc(p, size);
}

char *avahi_strdup(const char *s) {
    char *r;
    size_t size;

    if (!s)
        return NULL;

    size = strlen(s);
    if (!(r = avahi_malloc(size+1)))
        return NULL;

    memcpy(r, s, size+1);
    return r;
}

char *avahi_strndup(const char *s, size_t max) {
    char *r;
    size_t size;
    const char *p;

    if (!s)
        return NULL;

    for (p = s, size = 0;
         size < max && *p;
         p++, size++);

    if (!(r = avahi_new(char, size+1)))
        return NULL;

    memcpy(r, s, size);
    r[size] = 0;
    return r;
}

/* Change the allocator */
void avahi_set_allocator(const AvahiAllocator *a) {
    allocator = a;
}

char *avahi_strdup_vprintf(const char *fmt, va_list ap) {
    size_t len = 80;
    char *buf;

    assert(fmt);

    if (!(buf = avahi_malloc(len)))
        return NULL;

    for (;;) {
        int n;
        char *nbuf;
        va_list ap2;

        va_copy (ap2, ap);
        n = vsnprintf(buf, len, fmt, ap2);
        va_end (ap2);

        if (n >= 0 && n < (int) len)
            return buf;

        if (n >= 0)
            len = n+1;
        else
            len *= 2;

        if (!(nbuf = avahi_realloc(buf, len))) {
            avahi_free(buf);
            return NULL;
        }

        buf = nbuf;
    }
}

char *avahi_strdup_printf(const char *fmt, ... ) {
    char *s;
    va_list ap;

    assert(fmt);

    va_start(ap, fmt);
    s = avahi_strdup_vprintf(fmt, ap);
    va_end(ap);

    return s;
}

void *avahi_memdup(const void *s, size_t l) {
    void *p;
    assert(s);

    if (!(p = avahi_malloc(l)))
        return NULL;

    memcpy(p, s, l);
    return p;
}
