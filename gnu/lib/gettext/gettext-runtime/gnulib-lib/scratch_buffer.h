/* Variable-sized buffer with on-stack default allocation.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.

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

/* Written by Paul Eggert, 2017.  */

#ifndef _GL_SCRATCH_BUFFER_H
#define _GL_SCRATCH_BUFFER_H

/* Scratch buffers with a default stack allocation and fallback to
   heap allocation.  It is expected that this function is used in this
   way:

     struct scratch_buffer tmpbuf;
     scratch_buffer_init (&tmpbuf);

     while (!function_that_uses_buffer (tmpbuf.data, tmpbuf.length))
       if (!scratch_buffer_grow (&tmpbuf))
         return -1;

     scratch_buffer_free (&tmpbuf);
     return 0;

   The allocation functions (scratch_buffer_grow,
   scratch_buffer_grow_preserve, scratch_buffer_set_array_size) make
   sure that the heap allocation, if any, is freed, so that the code
   above does not have a memory leak.  The buffer still remains in a
   state that can be deallocated using scratch_buffer_free, so a loop
   like this is valid as well:

     struct scratch_buffer tmpbuf;
     scratch_buffer_init (&tmpbuf);

     while (!function_that_uses_buffer (tmpbuf.data, tmpbuf.length))
       if (!scratch_buffer_grow (&tmpbuf))
         break;

     scratch_buffer_free (&tmpbuf);

   scratch_buffer_grow and scratch_buffer_grow_preserve are guaranteed
   to grow the buffer by at least 512 bytes.  This means that when
   using the scratch buffer as a backing store for a non-character
   array whose element size, in bytes, is 512 or smaller, the scratch
   buffer only has to grow once to make room for at least one more
   element.
*/

/* Scratch buffer.  Must be initialized with scratch_buffer_init
   before its use.  */
struct scratch_buffer;

/* Initializes *BUFFER so that BUFFER->data points to BUFFER->__space
   and BUFFER->length reflects the available space.  */
#if 0
extern void scratch_buffer_init (struct scratch_buffer *buffer);
#endif

/* Deallocates *BUFFER (if it was heap-allocated).  */
#if 0
extern void scratch_buffer_free (struct scratch_buffer *buffer);
#endif

/* Grow *BUFFER by some arbitrary amount.  The buffer contents is NOT
   preserved.  Return true on success, false on allocation failure (in
   which case the old buffer is freed).  On success, the new buffer is
   larger than the previous size.  On failure, *BUFFER is deallocated,
   but remains in a free-able state, and errno is set.  */
#if 0
extern bool scratch_buffer_grow (struct scratch_buffer *buffer);
#endif

/* Like scratch_buffer_grow, but preserve the old buffer
   contents on success, as a prefix of the new buffer.  */
#if 0
extern bool scratch_buffer_grow_preserve (struct scratch_buffer *buffer);
#endif

/* Grow *BUFFER so that it can store at least NELEM elements of SIZE
   bytes.  The buffer contents are NOT preserved.  Both NELEM and SIZE
   can be zero.  Return true on success, false on allocation failure
   (in which case the old buffer is freed, but *BUFFER remains in a
   free-able state, and errno is set).  It is unspecified whether this
   function can reduce the array size.  */
#if 0
extern bool scratch_buffer_set_array_size (struct scratch_buffer *buffer,
                                           size_t nelem, size_t size);
#endif


/* The implementation is imported from glibc.  */

/* Avoid possible conflicts with symbols exported by the GNU libc.  */
#define __libc_scratch_buffer_grow gl_scratch_buffer_grow
#define __libc_scratch_buffer_grow_preserve gl_scratch_buffer_grow_preserve
#define __libc_scratch_buffer_set_array_size gl_scratch_buffer_set_array_size

#ifndef _GL_LIKELY
/* Rely on __builtin_expect, as provided by the module 'builtin-expect'.  */
# define _GL_LIKELY(cond) __builtin_expect ((cond), 1)
# define _GL_UNLIKELY(cond) __builtin_expect ((cond), 0)
#endif

#include <malloc/scratch_buffer.gl.h>

#endif /* _GL_SCRATCH_BUFFER_H */
