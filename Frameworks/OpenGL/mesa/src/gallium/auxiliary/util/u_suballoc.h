/**************************************************************************
 *
 * Copyright 2009 VMware, Inc.
 * Copyright 2012 Marek Olšák <maraeo@gmail.com>
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/* A simple allocator that suballocates memory from a large buffer. */

#ifndef U_SUBALLOC
#define U_SUBALLOC

#ifdef __cplusplus
extern "C" {
#endif

struct pipe_context;

struct u_suballocator {
   struct pipe_context *pipe;

   unsigned size;          /* Size of the whole buffer, in bytes. */
   unsigned bind;          /* Bitmask of PIPE_BIND_* flags. */
   enum pipe_resource_usage usage;
   unsigned flags;         /* bitmask of PIPE_RESOURCE_FLAG_x */
   bool zero_buffer_memory; /* If the buffer contents should be zeroed. */

   struct pipe_resource *buffer;   /* The buffer we suballocate from. */
   unsigned offset; /* Aligned offset pointing at the first unused byte. */
};

void
u_suballocator_init(struct u_suballocator *allocator,
                    struct pipe_context *pipe,
                    unsigned size, unsigned bind,
                    enum pipe_resource_usage usage, unsigned flags,
                    bool zero_buffer_memory);

void
u_suballocator_destroy(struct u_suballocator *allocator);

void
u_suballocator_alloc(struct u_suballocator *allocator, unsigned size,
                     unsigned alignment, unsigned *out_offset,
                     struct pipe_resource **outbuf);

#ifdef __cplusplus
}
#endif

#endif
