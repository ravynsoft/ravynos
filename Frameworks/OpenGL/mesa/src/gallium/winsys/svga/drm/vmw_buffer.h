/**********************************************************
 * Copyright 2009-2023 VMware, Inc.  All rights reserved.
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


#ifndef VMW_BUFFER_H_
#define VMW_BUFFER_H_

#include <assert.h>
#include "util/compiler.h"
#include "pipebuffer/pb_bufmgr.h"
#include "util/u_debug_flush.h"


/* These extra flags are used wherever the pb_usage_flags enum type is used */
#define VMW_BUFFER_USAGE_SHARED    (1 << 14)
#define VMW_BUFFER_USAGE_SYNC      (1 << 15)

struct SVGAGuestPtr;
struct pb_buffer;
struct pb_manager;
struct svga_winsys_buffer;
struct svga_winsys_surface;
struct vmw_winsys_screen;

struct vmw_buffer_desc {
   struct pb_desc pb_desc;
   struct vmw_region *region;
};


#ifdef DEBUG

struct pb_buffer *
vmw_pb_buffer(struct svga_winsys_buffer *buffer);
struct svga_winsys_buffer *
vmw_svga_winsys_buffer_wrap(struct pb_buffer *buffer);
struct debug_flush_buf *
vmw_debug_flush_buf(struct svga_winsys_buffer *buffer);

#else
static inline struct pb_buffer *
vmw_pb_buffer(struct svga_winsys_buffer *buffer)
{
   assert(buffer);
   return (struct pb_buffer *)buffer;
}


static inline struct svga_winsys_buffer *
vmw_svga_winsys_buffer_wrap(struct pb_buffer *buffer)
{
   return (struct svga_winsys_buffer *)buffer;
}
#endif

void
vmw_svga_winsys_buffer_destroy(struct svga_winsys_screen *sws,
                               struct svga_winsys_buffer *buf);
void *
vmw_svga_winsys_buffer_map(struct svga_winsys_screen *sws,
                           struct svga_winsys_buffer *buf,
                           enum pipe_map_flags flags);

void
vmw_svga_winsys_buffer_unmap(struct svga_winsys_screen *sws,
                             struct svga_winsys_buffer *buf);

struct pb_manager *
vmw_dma_bufmgr_create(struct vmw_winsys_screen *vws);

bool
vmw_dma_bufmgr_region_ptr(struct pb_buffer *buf,
                          struct SVGAGuestPtr *ptr);


#endif /* VMW_BUFFER_H_ */
