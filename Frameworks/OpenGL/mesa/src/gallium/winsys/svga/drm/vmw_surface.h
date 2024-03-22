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

/**
 * @file
 * Surfaces for VMware SVGA winsys.
 * 
 * @author Jose Fonseca <jfonseca@vmware.com>
 */


#ifndef VMW_SURFACE_H_
#define VMW_SURFACE_H_


#include "util/compiler.h"
#include "util/u_atomic.h"
#include "util/u_inlines.h"
#include "util/u_thread.h"
#include "pipebuffer/pb_buffer.h"

#define VMW_MAX_PRESENTS 3



struct vmw_svga_winsys_surface
{
   int32_t validated; /* atomic */
   struct pipe_reference refcnt;

   struct vmw_winsys_screen *screen;
   uint32_t sid;

   /* FIXME: make this thread safe */
   unsigned next_present_no;
   uint32_t present_fences[VMW_MAX_PRESENTS];

   mtx_t mutex;
   struct svga_winsys_buffer *buf; /* Current backing guest buffer */
   uint32_t mapcount; /* Number of mappers */
   uint32_t map_mode; /* PIPE_MAP_[READ|WRITE] */
   void *data; /* Pointer to data if mapcount != 0*/
   bool shared; /* Shared surface. Never discard */
   uint32_t size; /* Size of backing buffer */
   bool rebind; /* Surface needs a rebind after next unmap */
};


static inline struct svga_winsys_surface *
svga_winsys_surface(struct vmw_svga_winsys_surface *surf)
{
   assert(!surf || surf->sid != SVGA3D_INVALID_ID);
   return (struct svga_winsys_surface *)surf;
}


static inline struct vmw_svga_winsys_surface *
vmw_svga_winsys_surface(struct svga_winsys_surface *surf)
{
   return (struct vmw_svga_winsys_surface *)surf;
}


void
vmw_svga_winsys_surface_reference(struct vmw_svga_winsys_surface **pdst,
                                  struct vmw_svga_winsys_surface *src);
void *
vmw_svga_winsys_surface_map(struct svga_winsys_context *swc,
                            struct svga_winsys_surface *srf,
                            unsigned flags, bool *retry,
                            bool *rebind);
void
vmw_svga_winsys_surface_unmap(struct svga_winsys_context *swc,
                              struct svga_winsys_surface *srf,
                              bool *rebind);

void
vmw_svga_winsys_surface_init(struct svga_winsys_screen *sws,
                             struct svga_winsys_surface *surface,
                             unsigned surf_size, SVGA3dSurfaceAllFlags flags);
 
#endif /* VMW_SURFACE_H_ */
