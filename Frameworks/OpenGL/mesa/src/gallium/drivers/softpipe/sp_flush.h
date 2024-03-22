/**************************************************************************
 * 
 * Copyright 2007 VMware, Inc.
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
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **************************************************************************/

#ifndef SP_FLUSH_H
#define SP_FLUSH_H

#include "util/compiler.h"

struct pipe_context;
struct pipe_fence_handle;

#define SP_FLUSH_TEXTURE_CACHE  0x2

void
softpipe_flush(struct pipe_context *pipe,
               unsigned flags,
               struct pipe_fence_handle **fence);

void
softpipe_flush_wrapped(struct pipe_context *pipe,
                       struct pipe_fence_handle **fence,
                       unsigned flags);

bool
softpipe_flush_resource(struct pipe_context *pipe,
                        struct pipe_resource *texture,
                        unsigned level,
                        int layer,
                        unsigned flush_flags,
                        bool read_only,
                        bool cpu_access,
                        bool do_not_block);

void softpipe_texture_barrier(struct pipe_context *pipe, unsigned flags);
void softpipe_memory_barrier(struct pipe_context *pipe, unsigned flags);
#endif
