/**********************************************************
 * Copyright 2009-2015 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/


#ifndef VMW_FENCE_H_
#define VMW_FENCE_H_


#include "util/compiler.h"
#include "pipebuffer/pb_buffer_fenced.h"

struct pipe_fence_handle;
struct pb_fence_ops;
struct vmw_winsys_screen;


struct pipe_fence_handle *
vmw_fence_create(struct pb_fence_ops *fence_ops,
		 uint32_t handle, uint32_t seqno, uint32_t mask, int32_t fd);

int
vmw_fence_finish(struct vmw_winsys_screen *vws,
		 struct pipe_fence_handle *fence,
		 uint64_t timeout,
		 unsigned flag);

int
vmw_fence_get_fd(struct pipe_fence_handle *fence);

int
vmw_fence_signalled(struct vmw_winsys_screen *vws,
		    struct pipe_fence_handle *fence,
		    unsigned flag);
void
vmw_fence_reference(struct vmw_winsys_screen *vws,
		    struct pipe_fence_handle **ptr,
		    struct pipe_fence_handle *fence);

struct pb_fence_ops *
vmw_fence_ops_create(struct vmw_winsys_screen *vws); 



#endif /* VMW_FENCE_H_ */
