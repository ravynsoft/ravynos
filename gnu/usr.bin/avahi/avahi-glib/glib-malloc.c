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

#include <avahi-common/llist.h>
#include <avahi-common/malloc.h>

#include "glib-malloc.h"

static void* malloc_glue(size_t l) {
    return g_malloc(l);
}

static void* realloc_glue(void *p, size_t l) {
    return g_realloc(p, l);
}

static void* calloc_glue(size_t nmemb, size_t size) {
    return g_malloc0(nmemb * size);
}

const AvahiAllocator *avahi_glib_allocator(void) {

    static AvahiAllocator allocator;
    static int allocator_initialized = 0;

    if (!allocator_initialized) {
        allocator.malloc = malloc_glue;
        allocator.free = g_free;
        allocator.realloc = realloc_glue;
        allocator.calloc = calloc_glue;
        allocator_initialized = 1;
    }

    return &allocator;
}
