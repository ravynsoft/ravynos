/**************************************************************************
 *
 * Copyright 2013 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 **************************************************************************/

#include "radeon_video.h"

#include "radeon_vce.h"
#include "radeonsi/si_pipe.h"
#include "util/u_memory.h"
#include "util/u_video.h"
#include "vl/vl_defines.h"
#include "vl/vl_video_buffer.h"

#include <unistd.h>

/* generate an stream handle */
unsigned si_vid_alloc_stream_handle()
{
   static unsigned counter = 0;
   unsigned stream_handle = 0;
   unsigned pid = getpid();
   int i;

   for (i = 0; i < 32; ++i)
      stream_handle |= ((pid >> i) & 1) << (31 - i);

   stream_handle ^= ++counter;
   return stream_handle;
}

/* create a buffer in the winsys */
bool si_vid_create_buffer(struct pipe_screen *screen, struct rvid_buffer *buffer, unsigned size,
                          unsigned usage)
{
   memset(buffer, 0, sizeof(*buffer));
   buffer->usage = usage;

   /* Hardware buffer placement restrictions require the kernel to be
    * able to move buffers around individually, so request a
    * non-sub-allocated buffer.
    */
   buffer->res = si_resource(pipe_buffer_create(screen, PIPE_BIND_CUSTOM, usage, size));

   return buffer->res != NULL;
}

/* create a tmz buffer in the winsys */
bool si_vid_create_tmz_buffer(struct pipe_screen *screen, struct rvid_buffer *buffer, unsigned size,
                              unsigned usage)
{
   memset(buffer, 0, sizeof(*buffer));
   buffer->usage = usage;
   buffer->res = si_resource(pipe_buffer_create(screen, PIPE_BIND_CUSTOM | PIPE_BIND_PROTECTED,
                                                usage, size));
   return buffer->res != NULL;
}


/* destroy a buffer */
void si_vid_destroy_buffer(struct rvid_buffer *buffer)
{
   si_resource_reference(&buffer->res, NULL);
}

/* reallocate a buffer, preserving its content */
bool si_vid_resize_buffer(struct pipe_screen *screen, struct radeon_cmdbuf *cs,
                          struct rvid_buffer *new_buf, unsigned new_size,
                          struct rvid_buf_offset_info *buf_ofst_info)
{
   struct si_screen *sscreen = (struct si_screen *)screen;
   struct radeon_winsys *ws = sscreen->ws;
   unsigned bytes = MIN2(new_buf->res->buf->size, new_size);
   struct rvid_buffer old_buf = *new_buf;
   void *src = NULL, *dst = NULL;

   if (!si_vid_create_buffer(screen, new_buf, new_size, new_buf->usage))
      goto error;

   src = ws->buffer_map(ws, old_buf.res->buf, cs, PIPE_MAP_READ | RADEON_MAP_TEMPORARY);
   if (!src)
      goto error;

   dst = ws->buffer_map(ws, new_buf->res->buf, cs, PIPE_MAP_WRITE | RADEON_MAP_TEMPORARY);
   if (!dst)
      goto error;

   if (buf_ofst_info) {
      memset(dst, 0, new_size);
      for(int i =0; i < buf_ofst_info->num_units; i++) {
          memcpy(dst, src, buf_ofst_info->old_offset);
          dst += buf_ofst_info->new_offset;
          src += buf_ofst_info->old_offset;
      }
   } else {
      memcpy(dst, src, bytes);
      if (new_size > bytes) {
         new_size -= bytes;
         dst += bytes;
         memset(dst, 0, new_size);
      }
   }
   ws->buffer_unmap(ws, new_buf->res->buf);
   ws->buffer_unmap(ws, old_buf.res->buf);
   si_vid_destroy_buffer(&old_buf);
   return true;

error:
   if (src)
      ws->buffer_unmap(ws, old_buf.res->buf);
   si_vid_destroy_buffer(new_buf);
   *new_buf = old_buf;
   return false;
}

/* clear the buffer with zeros */
void si_vid_clear_buffer(struct pipe_context *context, struct rvid_buffer *buffer)
{
   struct si_context *sctx = (struct si_context *)context;
   uint32_t zero = 0;

   sctx->b.clear_buffer(&sctx->b, &buffer->res->b.b, 0, buffer->res->b.b.width0, &zero, 4);
   context->flush(context, NULL, 0);
}
