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

/**
 * @file
 * SVGA buffer manager for DMA buffers.
 * 
 * DMA buffers are used for pixel and vertex data upload/download to/from
 * the virtual SVGA hardware.
 *
 * This file implements a pipebuffer library's buffer manager, so that we can
 * use pipepbuffer's suballocation, fencing, and debugging facilities with
 * DMA buffers.
 * 
 * @author Jose Fonseca <jfonseca@vmware.com>
 */


#include "svga_cmd.h"

#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "pipebuffer/pb_buffer.h"
#include "pipebuffer/pb_bufmgr.h"

#include "svga_winsys.h"

#include "vmw_screen.h"
#include "vmw_buffer.h"

struct vmw_dma_bufmgr;


struct vmw_dma_buffer
{
   struct pb_buffer base;
   
   struct vmw_dma_bufmgr *mgr;
   
   struct vmw_region *region;
   void *map;
   unsigned map_flags;
   unsigned map_count;
};


extern const struct pb_vtbl vmw_dma_buffer_vtbl;


static inline struct vmw_dma_buffer *
vmw_pb_to_dma_buffer(struct pb_buffer *buf)
{
   assert(buf);
   assert(buf->vtbl == &vmw_dma_buffer_vtbl);
   return container_of(buf, struct vmw_dma_buffer, base);
}


struct vmw_dma_bufmgr
{
   struct pb_manager base;
   
   struct vmw_winsys_screen *vws;
};


static inline struct vmw_dma_bufmgr *
vmw_pb_to_dma_bufmgr(struct pb_manager *mgr)
{
   assert(mgr);

   /* Make sure our extra flags don't collide with pipebuffer's flags */
   STATIC_ASSERT((VMW_BUFFER_USAGE_SHARED & PB_USAGE_ALL) == 0);
   STATIC_ASSERT((VMW_BUFFER_USAGE_SYNC & PB_USAGE_ALL) == 0);

   return container_of(mgr, struct vmw_dma_bufmgr, base);
}


static void
vmw_dma_buffer_destroy(void *winsys, struct pb_buffer *_buf)
{
   struct vmw_dma_buffer *buf = vmw_pb_to_dma_buffer(_buf);

   assert(buf->map_count == 0);
   if (buf->map) {
      assert(buf->mgr->vws->cache_maps);
      vmw_ioctl_region_unmap(buf->region);
   }

   vmw_ioctl_region_destroy(buf->region);

   FREE(buf);
}


static void *
vmw_dma_buffer_map(struct pb_buffer *_buf,
                   enum pb_usage_flags flags,
                   void *flush_ctx)
{
   struct vmw_dma_buffer *buf = vmw_pb_to_dma_buffer(_buf);
   int ret;

   if (!buf->map)
      buf->map = vmw_ioctl_region_map(buf->region);

   if (!buf->map)
      return NULL;

   if ((_buf->base.usage & VMW_BUFFER_USAGE_SYNC) &&
       !(flags & PB_USAGE_UNSYNCHRONIZED)) {
      ret = vmw_ioctl_syncforcpu(buf->region,
                                 !!(flags & PB_USAGE_DONTBLOCK),
                                 !(flags & PB_USAGE_CPU_WRITE),
                                 false);
      if (ret)
         return NULL;
   }

   buf->map_count++;
   return buf->map;
}


static void
vmw_dma_buffer_unmap(struct pb_buffer *_buf)
{
   struct vmw_dma_buffer *buf = vmw_pb_to_dma_buffer(_buf);
   enum pb_usage_flags flags = buf->map_flags;

   if ((_buf->base.usage & VMW_BUFFER_USAGE_SYNC) &&
       !(flags & PB_USAGE_UNSYNCHRONIZED)) {
      vmw_ioctl_releasefromcpu(buf->region,
                               !(flags & PB_USAGE_CPU_WRITE),
                               false);
   }

   assert(buf->map_count > 0);
   if (!--buf->map_count && !buf->mgr->vws->cache_maps) {
      vmw_ioctl_region_unmap(buf->region);
      buf->map = NULL;
   }
}


static void
vmw_dma_buffer_get_base_buffer(struct pb_buffer *buf,
                               struct pb_buffer **base_buf,
                               pb_size *offset)
{
   *base_buf = buf;
   *offset = 0;
}


static enum pipe_error
vmw_dma_buffer_validate( struct pb_buffer *_buf,
                         struct pb_validate *vl,
                         enum pb_usage_flags flags )
{
   /* Always pinned */
   return PIPE_OK;
}


static void
vmw_dma_buffer_fence( struct pb_buffer *_buf,
                      struct pipe_fence_handle *fence )
{
   /* We don't need to do anything, as the pipebuffer library
    * will take care of delaying the destruction of fenced buffers */  
}


const struct pb_vtbl vmw_dma_buffer_vtbl = {
   .destroy = vmw_dma_buffer_destroy,
   .map = vmw_dma_buffer_map,
   .unmap = vmw_dma_buffer_unmap,
   .validate = vmw_dma_buffer_validate,
   .fence = vmw_dma_buffer_fence,
   .get_base_buffer = vmw_dma_buffer_get_base_buffer
};


static struct pb_buffer *
vmw_dma_bufmgr_create_buffer(struct pb_manager *_mgr,
                             pb_size size,
                             const struct pb_desc *pb_desc)
{
   struct vmw_dma_bufmgr *mgr = vmw_pb_to_dma_bufmgr(_mgr);
   struct vmw_winsys_screen *vws = mgr->vws;
   struct vmw_dma_buffer *buf;
   const struct vmw_buffer_desc *desc =
      (const struct vmw_buffer_desc *) pb_desc;
   
   buf = CALLOC_STRUCT(vmw_dma_buffer);
   if(!buf)
      goto error1;

   pipe_reference_init(&buf->base.base.reference, 1);
   buf->base.base.alignment_log2 = util_logbase2(pb_desc->alignment);
   buf->base.base.usage = pb_desc->usage & ~VMW_BUFFER_USAGE_SHARED;
   buf->base.vtbl = &vmw_dma_buffer_vtbl;
   buf->mgr = mgr;
   buf->base.base.size = size;
   if ((pb_desc->usage & VMW_BUFFER_USAGE_SHARED) && desc->region) {
      buf->region = desc->region;
   } else {
      buf->region = vmw_ioctl_region_create(vws, size);
      if(!buf->region)
	 goto error2;
   }
	 
   return &buf->base;
error2:
   FREE(buf);
error1:
   return NULL;
}


static void
vmw_dma_bufmgr_flush(struct pb_manager *mgr)
{
   /* No-op */
}


static void
vmw_dma_bufmgr_destroy(struct pb_manager *_mgr)
{
   struct vmw_dma_bufmgr *mgr = vmw_pb_to_dma_bufmgr(_mgr);
   FREE(mgr);
}


struct pb_manager *
vmw_dma_bufmgr_create(struct vmw_winsys_screen *vws)
{
   struct vmw_dma_bufmgr *mgr;
   
   mgr = CALLOC_STRUCT(vmw_dma_bufmgr);
   if(!mgr)
      return NULL;

   mgr->base.destroy = vmw_dma_bufmgr_destroy;
   mgr->base.create_buffer = vmw_dma_bufmgr_create_buffer;
   mgr->base.flush = vmw_dma_bufmgr_flush;
   
   mgr->vws = vws;
   
   return &mgr->base;
}


bool
vmw_dma_bufmgr_region_ptr(struct pb_buffer *buf,
                          struct SVGAGuestPtr *ptr)
{
   struct pb_buffer *base_buf;
   pb_size offset = 0;
   struct vmw_dma_buffer *dma_buf;
   
   pb_get_base_buffer( buf, &base_buf, &offset );
   
   dma_buf = vmw_pb_to_dma_buffer(base_buf);
   if(!dma_buf)
      return false;
   
   *ptr = vmw_ioctl_region_ptr(dma_buf->region);
   
   ptr->offset += offset;
   
   return true;
}

#ifdef DEBUG
struct svga_winsys_buffer {
   struct pb_buffer *pb_buf;
   struct debug_flush_buf *fbuf;
};

struct pb_buffer *
vmw_pb_buffer(struct svga_winsys_buffer *buffer)
{
   assert(buffer);
   return buffer->pb_buf;
}

struct svga_winsys_buffer *
vmw_svga_winsys_buffer_wrap(struct pb_buffer *buffer)
{
   struct svga_winsys_buffer *buf;

   if (!buffer)
      return NULL;

   buf = CALLOC_STRUCT(svga_winsys_buffer);
   if (!buf) {
      pb_reference(&buffer, NULL);
      return NULL;
   }

   buf->pb_buf = buffer;
   buf->fbuf = debug_flush_buf_create(false, VMW_DEBUG_FLUSH_STACK);
   return buf;
}

struct debug_flush_buf *
vmw_debug_flush_buf(struct svga_winsys_buffer *buffer)
{
   return buffer->fbuf;
}

#endif

void
vmw_svga_winsys_buffer_destroy(struct svga_winsys_screen *sws,
                               struct svga_winsys_buffer *buf)
{
   struct pb_buffer *pbuf = vmw_pb_buffer(buf);
   (void)sws;
   pb_reference(&pbuf, NULL);
#ifdef DEBUG
   debug_flush_buf_reference(&buf->fbuf, NULL);
   FREE(buf);
#endif
}

void *
vmw_svga_winsys_buffer_map(struct svga_winsys_screen *sws,
                           struct svga_winsys_buffer *buf,
                           enum pipe_map_flags flags)
{
   void *map;
   enum pb_usage_flags pb_flags = 0;

   (void)sws;
   if (flags & PIPE_MAP_UNSYNCHRONIZED)
      flags &= ~PIPE_MAP_DONTBLOCK;

   if (flags & PIPE_MAP_READ)
      pb_flags |= PB_USAGE_CPU_READ;
   if (flags & PIPE_MAP_WRITE)
      pb_flags |= PB_USAGE_CPU_WRITE;
   if (flags & PIPE_MAP_DIRECTLY)
      pb_flags |= PB_USAGE_GPU_READ;
   if (flags & PIPE_MAP_DONTBLOCK)
      pb_flags |= PB_USAGE_DONTBLOCK;
   if (flags & PIPE_MAP_UNSYNCHRONIZED)
      pb_flags |= PB_USAGE_UNSYNCHRONIZED;
   if (flags & PIPE_MAP_PERSISTENT)
      pb_flags |= PB_USAGE_PERSISTENT;

   map = pb_map(vmw_pb_buffer(buf), pb_flags, NULL);

#ifdef DEBUG
   if (map != NULL)
      debug_flush_map(buf->fbuf, pb_flags);
#endif

   return map;
}


void
vmw_svga_winsys_buffer_unmap(struct svga_winsys_screen *sws,
                             struct svga_winsys_buffer *buf)
{
   (void)sws;

#ifdef DEBUG
   debug_flush_unmap(buf->fbuf);
#endif

   pb_unmap(vmw_pb_buffer(buf));
}
