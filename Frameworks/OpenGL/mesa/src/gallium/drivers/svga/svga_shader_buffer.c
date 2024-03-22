/**********************************************************
 * Copyright 2022 VMware, Inc.  All rights reserved.
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

#include "pipe/p_defines.h"
#include "util/u_bitmask.h"
#include "util/format/u_format.h"
#include "util/u_inlines.h"
#include "util/u_math.h"
#include "util/u_memory.h"

#include "svga_context.h"
#include "svga_cmd.h"
#include "svga_debug.h"
#include "svga_resource_buffer.h"
#include "svga_resource_texture.h"
#include "svga_surface.h"
#include "svga_sampler_view.h"
#include "svga_format.h"


/**
 * Create a uav object for the specified shader buffer
 */
SVGA3dUAViewId
svga_create_uav_buffer(struct svga_context *svga,
                       const struct pipe_shader_buffer *buf,
                       SVGA3dSurfaceFormat format,
                       SVGA3dUABufferFlags bufFlag)
{
   SVGA3dUAViewDesc desc;
   unsigned uaViewId;

   assert(buf);

   /* If there is not one defined, create one. */
   memset(&desc, 0, sizeof(desc));
   desc.buffer.firstElement = buf->buffer_offset / sizeof(uint32);
   desc.buffer.numElements = buf->buffer_size / sizeof(uint32);
   desc.buffer.flags = bufFlag;

   uaViewId = svga_create_uav(svga, &desc, format,
                              SVGA3D_RESOURCE_BUFFER,
                              svga_buffer_handle(svga, buf->buffer,
                                                 PIPE_BIND_SHADER_BUFFER));
   if (uaViewId == SVGA3D_INVALID_ID)
      return uaViewId;

   SVGA_DBG(DEBUG_UAV, "%s: resource=0x%x uaViewId=%d\n",
            __func__, buf->buffer, uaViewId);

   /* Mark this buffer as a uav bound buffer */
   struct svga_buffer *sbuf = svga_buffer(buf->buffer);
   sbuf->uav = true;

   return uaViewId;
}


/**
 * Set shader buffers.
 */
static void
svga_set_shader_buffers(struct pipe_context *pipe,
                        enum pipe_shader_type shader,
                        unsigned start, unsigned num,
                        const struct pipe_shader_buffer *buffers,
                        unsigned writeable_bitmask)
{
   struct svga_context *svga = svga_context(pipe);
   const struct pipe_shader_buffer *buf;

   assert(svga_have_gl43(svga));

   assert(start + num <= SVGA_MAX_SHADER_BUFFERS);

#ifdef DEBUG
   const struct pipe_shader_buffer *b = buffers;
   SVGA_DBG(DEBUG_UAV, "%s: shader=%d start=%d num=%d ",
            __func__, shader, start, num);
   if (buffers) {
      for (unsigned i = 0; i < num; i++, b++) {
         SVGA_DBG(DEBUG_UAV, " 0x%x ", b);
      }
   }
   SVGA_DBG(DEBUG_UAV, "\n");
#endif

   buf = buffers;
   if (buffers) {
      int last_buffer = -1;
      for (unsigned i = start, j=0; i < start + num; i++, buf++, j++) {
         struct svga_shader_buffer *cbuf = &svga->curr.shader_buffers[shader][i];

         if (buf && buf->buffer) {
            cbuf->desc = *buf;
            pipe_resource_reference(&cbuf->resource, buf->buffer);

            /* Mark the last bound shader buffer */
            last_buffer = i;
         }
         else {
            cbuf->desc.buffer = NULL;
            pipe_resource_reference(&cbuf->resource, NULL);
         }
         cbuf->uav_index = -1;
	 cbuf->writeAccess = (writeable_bitmask & (1 << j)) != 0;
      }
      svga->curr.num_shader_buffers[shader] =
         MAX2(svga->curr.num_shader_buffers[shader], last_buffer + 1);
   }
   else {
      for (unsigned i = start; i < start + num; i++) {
         struct svga_shader_buffer *cbuf = &svga->curr.shader_buffers[shader][i];
         cbuf->desc.buffer = NULL;
         cbuf->uav_index = -1;
         pipe_resource_reference(&cbuf->resource, NULL);
      }
      if ((start + num) >= svga->curr.num_shader_buffers[shader])
         svga->curr.num_shader_buffers[shader] = start;
   }

#ifdef DEBUG
   SVGA_DBG(DEBUG_UAV,
            "%s: current num_shader_buffers=%d start=%d num=%d buffers=",
            __func__, svga->curr.num_shader_buffers[shader],
            start, num);

   for (unsigned i = start; i < start + num; i++) {
      struct svga_shader_buffer *cbuf = &svga->curr.shader_buffers[shader][i];
      SVGA_DBG(DEBUG_UAV, " 0x%x ", cbuf->desc.buffer);
   }

   SVGA_DBG(DEBUG_UAV, "\n");
#endif

   /* purge any unused uav objects */
   svga_destroy_uav(svga);

   svga->dirty |= SVGA_NEW_SHADER_BUFFER;
}


/**
 * Set HW atomic buffers.
 */
static void
svga_set_hw_atomic_buffers(struct pipe_context *pipe,
                           unsigned start, unsigned num,
                           const struct pipe_shader_buffer *buffers)
{
   struct svga_context *svga = svga_context(pipe);
   const struct pipe_shader_buffer *buf = buffers;

   assert(svga_have_gl43(svga));

   assert(start + num <= SVGA_MAX_ATOMIC_BUFFERS);

#ifdef DEBUG
   SVGA_DBG(DEBUG_UAV, "%s: start=%d num=%d \n", __func__, start, num);
#endif

   buf = buffers;
   if (buffers) {
      int last_buffer = -1;
      for (unsigned i = start; i < start + num; i++, buf++) {
         struct svga_shader_buffer *cbuf = &svga->curr.atomic_buffers[i];

         if (buf && buf->buffer) {
            cbuf->desc = *buf;
            pipe_resource_reference(&cbuf->resource, buf->buffer);

            last_buffer = i;

            /* Mark the buffer as uav buffer so that a readback will
             * be done at each read transfer. We can't rely on the
             * dirty bit because it is reset after each read, but
             * the uav buffer can be updated at each draw.
             */
            struct svga_buffer *sbuf = svga_buffer(cbuf->desc.buffer);
            sbuf->uav = true;
         }
         else {
            cbuf->desc.buffer = NULL;
            pipe_resource_reference(&cbuf->resource, NULL);
         }
         cbuf->uav_index = -1;
      }
      svga->curr.num_atomic_buffers = MAX2(svga->curr.num_atomic_buffers,
                                        last_buffer + 1);
   }
   else {
      for (unsigned i = start; i < start + num; i++) {
         struct svga_shader_buffer *cbuf = &svga->curr.atomic_buffers[i];
         cbuf->desc.buffer = NULL;
         cbuf->uav_index = -1;
         pipe_resource_reference(&cbuf->resource, NULL);
      }
      if ((start + num) >= svga->curr.num_atomic_buffers)
         svga->curr.num_atomic_buffers = start;
   }

#ifdef DEBUG
   SVGA_DBG(DEBUG_UAV, "%s: current num_atomic_buffers=%d start=%d num=%d ",
            __func__, svga->curr.num_atomic_buffers,
            start, num);

   for (unsigned i = start; i < start + num; i++) {
      struct svga_shader_buffer *cbuf = &svga->curr.atomic_buffers[i];
      SVGA_DBG(DEBUG_UAV, " 0x%x ", cbuf->desc.buffer);
   }

   SVGA_DBG(DEBUG_UAV, "\n");
#endif

   /* purge any unused uav objects */
   svga_destroy_uav(svga);

   svga->dirty |= SVGA_NEW_SHADER_BUFFER;
}


/**
 *  Initialize shader images gallium interface
 */
void
svga_init_shader_buffer_functions(struct svga_context *svga)
{
   if (!svga_have_gl43(svga))
      return;

   svga->pipe.set_shader_buffers = svga_set_shader_buffers;
   svga->pipe.set_hw_atomic_buffers = svga_set_hw_atomic_buffers;

   /* Initialize shader buffers */
   for (unsigned shader = 0; shader < PIPE_SHADER_TYPES; ++shader) {
      struct svga_shader_buffer *hw_buf =
         &svga->state.hw_draw.shader_buffers[shader][0];
      struct svga_shader_buffer *cur_buf =
         &svga->curr.shader_buffers[shader][0];

      /* Initialize uaViewId to SVGA3D_INVALID_ID for current shader buffers
       * and shader buffers in hw state to avoid unintentional unbinding of
       * shader buffers with uaViewId 0.
       */
      for (unsigned i = 0; i < ARRAY_SIZE(svga->curr.shader_buffers[shader]);
           i++, hw_buf++, cur_buf++) {
         hw_buf->resource = NULL;
         hw_buf->uav_index = -1;
         cur_buf->desc.buffer = NULL;
         cur_buf->resource = NULL;
         cur_buf->uav_index = -1;
      }
   }
   memset(svga->state.hw_draw.num_shader_buffers, 0,
          sizeof(svga->state.hw_draw.num_shader_buffers));

   /* Initialize atomic buffers */

   /* Initialize uaViewId to SVGA3D_INVALID_ID for current atomic buffers
    * and atomic buffers in hw state to avoid unintentional unbinding of
    * shader buffer with uaViewId 0.
    */
   for (unsigned i = 0; i < ARRAY_SIZE(svga->state.hw_draw.atomic_buffers); i++) {
      svga->curr.atomic_buffers[i].resource = NULL;
      svga->curr.atomic_buffers[i].uav_index = -1;
   }
   svga->state.hw_draw.num_atomic_buffers = 0;
}


/**
 * Cleanup shader image state
 */
void
svga_cleanup_shader_buffer_state(struct svga_context *svga)
{
   if (!svga_have_gl43(svga))
      return;

   svga_destroy_uav(svga);
}


/**
 * Validate shader buffer resources to ensure any pending changes to the
 * buffers are emitted before they are referenced.
 * The helper function also rebinds the buffer resources if the rebind flag
 * is specified.
 */
enum pipe_error
svga_validate_shader_buffer_resources(struct svga_context *svga,
                                      unsigned count,
                                      struct svga_shader_buffer *bufs,
                                      bool rebind)
{
   assert(svga_have_gl43(svga));

   struct svga_winsys_surface *surf;
   enum pipe_error ret;
   unsigned i;

   for (i = 0; i < count; i++) {
      if (bufs[i].resource) {
         assert(bufs[i].resource == bufs[i].desc.buffer);

         struct svga_buffer *sbuf = svga_buffer(bufs[i].resource);
         surf = svga_buffer_handle(svga, bufs[i].desc.buffer,
                                   PIPE_BIND_SHADER_BUFFER);
         assert(surf);
         if (rebind) {
            ret = svga->swc->resource_rebind(svga->swc, surf, NULL,
                                             SVGA_RELOC_READ|SVGA_RELOC_WRITE);
            if (ret != PIPE_OK)
               return ret;
         }

         /* Mark buffer as RENDERED */
         svga_set_buffer_rendered_to(sbuf->bufsurf);
      }
   }

   return PIPE_OK;
}


/**
 * Returns TRUE if the shader buffer can be bound to SRV as raw buffer.
 * It is TRUE if the shader buffer is readonly and the surface already
 * has the RAW_BUFFER_VIEW bind flag set.
 */
bool
svga_shader_buffer_can_use_srv(struct svga_context *svga,
			       enum pipe_shader_type shader,
                               unsigned index,
                               struct svga_shader_buffer *buf)
{
   if (buf->resource) {
      struct svga_buffer *sbuf = svga_buffer(buf->resource);
      if (sbuf && !buf->writeAccess && svga_has_raw_buffer_view(sbuf)) {
         return true;
      }
   }
   return false;
}


#define SVGA_SSBO_SRV_START  SVGA_MAX_CONST_BUFS

/**
 * Bind the shader buffer as SRV raw buffer.
 */
enum pipe_error
svga_shader_buffer_bind_srv(struct svga_context *svga,
                            enum pipe_shader_type shader,
                            unsigned index,
                            struct svga_shader_buffer *buf)
{
   enum pipe_error ret;
   unsigned slot = index + SVGA_SSBO_SRV_START;

   svga->state.raw_shaderbufs[shader] |= (1 << index);
   ret = svga_emit_rawbuf(svga, slot, shader, buf->desc.buffer_offset,
                          buf->desc.buffer_size, buf->resource);
   if (ret == PIPE_OK)
      svga->state.hw_draw.enabled_raw_shaderbufs[shader] |= (1 << index);

   return ret;
}


/**
 * Unbind the shader buffer SRV.
 */
enum pipe_error
svga_shader_buffer_unbind_srv(struct svga_context *svga,
                              enum pipe_shader_type shader,
                              unsigned index,
                              struct svga_shader_buffer *buf)
{
   enum pipe_error ret = PIPE_OK;
   unsigned slot = index + SVGA_SSBO_SRV_START;

   if ((svga->state.hw_draw.enabled_raw_shaderbufs[shader] & (1 << index))
          != 0) {
      ret = svga_emit_rawbuf(svga, slot, shader, 0, 0, NULL);
      if (ret == PIPE_OK)
         svga->state.hw_draw.enabled_raw_shaderbufs[shader] &= ~(1 << index);
   }
   svga->state.raw_shaderbufs[shader] &= ~(1 << index);
   return ret;
}
