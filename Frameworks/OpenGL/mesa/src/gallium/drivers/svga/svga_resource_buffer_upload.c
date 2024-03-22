/**********************************************************
 * Copyright 2008-2022 VMware, Inc.  All rights reserved.
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


#include "util/u_thread.h"
#include "pipe/p_state.h"
#include "pipe/p_defines.h"
#include "util/u_inlines.h"
#include "util/u_math.h"
#include "util/u_memory.h"

#include "svga_cmd.h"
#include "svga_context.h"
#include "svga_debug.h"
#include "svga_resource_buffer.h"
#include "svga_resource_buffer_upload.h"
#include "svga_screen.h"
#include "svga_winsys.h"

/**
 * Describes a complete SVGA_3D_CMD_UPDATE_GB_IMAGE command
 *
 */
struct svga_3d_update_gb_image {
   SVGA3dCmdHeader header;
   SVGA3dCmdUpdateGBImage body;
};

struct svga_3d_invalidate_gb_image {
   SVGA3dCmdHeader header;
   SVGA3dCmdInvalidateGBImage body;
};


static void
svga_buffer_upload_ranges(struct svga_context *, struct svga_buffer *);


/**
 * Allocate a winsys_buffer (ie. DMA, aka GMR memory).
 *
 * It will flush and retry in case the first attempt to create a DMA buffer
 * fails, so it should not be called from any function involved in flushing
 * to avoid recursion.
 */
struct svga_winsys_buffer *
svga_winsys_buffer_create( struct svga_context *svga,
                           unsigned alignment,
                           unsigned usage,
                           unsigned size )
{
   struct svga_screen *svgascreen = svga_screen(svga->pipe.screen);
   struct svga_winsys_screen *sws = svgascreen->sws;
   struct svga_winsys_buffer *buf;

   /* Just try */
   buf = SVGA_TRY_PTR(sws->buffer_create(sws, alignment, usage, size));
   if (!buf) {
      SVGA_DBG(DEBUG_DMA|DEBUG_PERF, "flushing context to find %d bytes GMR\n",
               size);

      /* Try flushing all pending DMAs */
      svga_retry_enter(svga);
      svga_context_flush(svga, NULL);
      buf = sws->buffer_create(sws, alignment, usage, size);
      svga_retry_exit(svga);
   }

   return buf;
}


/**
 * Destroy HW storage if separate from the host surface.
 * In the GB case, the HW storage is associated with the host surface
 * and is therefore a No-op.
 */
void
svga_buffer_destroy_hw_storage(struct svga_screen *ss, struct svga_buffer *sbuf)
{
   struct svga_winsys_screen *sws = ss->sws;

   assert(sbuf->map.count == 0);
   assert(sbuf->hwbuf);
   if (sbuf->hwbuf) {
      sws->buffer_destroy(sws, sbuf->hwbuf);
      sbuf->hwbuf = NULL;
   }
}



/**
 * Allocate DMA'ble or Updatable storage for the buffer.
 *
 * Called before mapping a buffer.
 */
enum pipe_error
svga_buffer_create_hw_storage(struct svga_screen *ss,
                              struct svga_buffer *sbuf,
                              unsigned bind_flags)
{
   assert(!sbuf->user);

   if (ss->sws->have_gb_objects) {
      assert(sbuf->handle || !sbuf->dma.pending);
      return svga_buffer_create_host_surface(ss, sbuf, bind_flags);
   }
   if (!sbuf->hwbuf) {
      struct svga_winsys_screen *sws = ss->sws;
      unsigned alignment = 16;
      unsigned usage = 0;
      unsigned size = sbuf->b.width0;

      sbuf->hwbuf = sws->buffer_create(sws, alignment, usage, size);
      if (!sbuf->hwbuf)
         return PIPE_ERROR_OUT_OF_MEMORY;

      assert(!sbuf->dma.pending);
   }

   return PIPE_OK;
}


/**
 * Allocate graphics memory for vertex/index/constant/texture buffer.
 */
enum pipe_error
svga_buffer_create_host_surface(struct svga_screen *ss,
                                struct svga_buffer *sbuf,
                                unsigned bind_flags)
{
   enum pipe_error ret = PIPE_OK;

   assert(!sbuf->user);

   if (!sbuf->handle) {
      bool invalidated;

      sbuf->key.flags = 0;

      sbuf->key.format = SVGA3D_BUFFER;
      if (bind_flags & PIPE_BIND_VERTEX_BUFFER) {
         sbuf->key.flags |= SVGA3D_SURFACE_HINT_VERTEXBUFFER;
         sbuf->key.flags |= SVGA3D_SURFACE_BIND_VERTEX_BUFFER;
      }
      if (bind_flags & PIPE_BIND_INDEX_BUFFER) {
         sbuf->key.flags |= SVGA3D_SURFACE_HINT_INDEXBUFFER;
         sbuf->key.flags |= SVGA3D_SURFACE_BIND_INDEX_BUFFER;
      }
      if (bind_flags & PIPE_BIND_CONSTANT_BUFFER)
         sbuf->key.flags |= SVGA3D_SURFACE_BIND_CONSTANT_BUFFER;

      if (bind_flags & PIPE_BIND_STREAM_OUTPUT)
         sbuf->key.flags |= SVGA3D_SURFACE_BIND_STREAM_OUTPUT;

      if (bind_flags & PIPE_BIND_SAMPLER_VIEW)
         sbuf->key.flags |= SVGA3D_SURFACE_BIND_SHADER_RESOURCE;

      if (bind_flags & PIPE_BIND_COMMAND_ARGS_BUFFER) {
         assert(ss->sws->have_sm5);
         sbuf->key.flags |= SVGA3D_SURFACE_DRAWINDIRECT_ARGS;
      }

      if (!bind_flags && sbuf->b.usage == PIPE_USAGE_STAGING) {
         /* This surface is to be used with the
          * SVGA3D_CMD_DX_TRANSFER_FROM_BUFFER command, and no other
          * bind flags are allowed to be set for this surface.
          */
         sbuf->key.flags = SVGA3D_SURFACE_TRANSFER_FROM_BUFFER;
      }

      if (ss->sws->have_gl43 &&
          (bind_flags & (PIPE_BIND_SHADER_BUFFER | PIPE_BIND_SHADER_IMAGE)) &&
          (!(bind_flags & (PIPE_BIND_STREAM_OUTPUT)))) {
         /* This surface can be bound to a uav. */
         assert((bind_flags & PIPE_BIND_CONSTANT_BUFFER) == 0);
         sbuf->key.flags |= SVGA3D_SURFACE_BIND_UAVIEW |
                            SVGA3D_SURFACE_BIND_RAW_VIEWS;
      }

      if (sbuf->b.flags & PIPE_RESOURCE_FLAG_MAP_PERSISTENT) {
         /* This surface can be mapped persistently. We use
          * coherent memory if available to avoid implementing memory barriers
          * for persistent non-coherent memory for now.
          */
         sbuf->key.coherent = ss->sws->have_coherent;

         if (ss->sws->have_gl43) {
            /* Set the persistent bit so if the buffer is to be bound
             * as constant buffer, we'll access it as raw buffer
             * instead of copying the content back and forth between the
             * mapped buffer surface and the constant buffer surface.
             */
            sbuf->key.persistent = 1;

            /* Set the raw views bind flag only if the mapped buffer surface
             * is not already bound as constant buffer since constant buffer
             * surface cannot have other bind flags.
             */
            if ((bind_flags & PIPE_BIND_CONSTANT_BUFFER) == 0) {
	       sbuf->key.flags |= SVGA3D_SURFACE_BIND_UAVIEW |
                                  SVGA3D_SURFACE_BIND_RAW_VIEWS;
               bind_flags = bind_flags | PIPE_BIND_SHADER_BUFFER;
               //sbuf->key.flags |= SVGA3D_SURFACE_BIND_RAW_VIEWS;
            }
         }
      }

      sbuf->key.size.width = sbuf->b.width0;
      sbuf->key.size.height = 1;
      sbuf->key.size.depth = 1;

      sbuf->key.numFaces = 1;
      sbuf->key.numMipLevels = 1;
      sbuf->key.cachable = 1;
      sbuf->key.arraySize = 1;
      sbuf->key.sampleCount = 0;

      SVGA_DBG(DEBUG_DMA, "surface_create for buffer sz %d\n",
               sbuf->b.width0);

      sbuf->handle = svga_screen_surface_create(ss, bind_flags,
                                                sbuf->b.usage,
                                                &invalidated, &sbuf->key);
      if (!sbuf->handle)
         return PIPE_ERROR_OUT_OF_MEMORY;

      /* Set the discard flag on the first time the buffer is written
       * as svga_screen_surface_create might have passed a recycled host
       * buffer. This is only needed for host-backed mode. As in guest-backed
       * mode, the recycled buffer would have been invalidated.
       */
      if (!ss->sws->have_gb_objects)
         sbuf->dma.flags.discard = true;

      SVGA_DBG(DEBUG_DMA, "   --> got sid %p sz %d (buffer)\n",
               sbuf->handle, sbuf->b.width0);

      /* Add the new surface to the buffer surface list */
      sbuf->bufsurf = svga_buffer_add_host_surface(sbuf, sbuf->handle,
		                                   &sbuf->key,
                                                   bind_flags);
      if (sbuf->bufsurf == NULL)
         return PIPE_ERROR_OUT_OF_MEMORY;

      sbuf->bufsurf->surface_state =
	 invalidated ? SVGA_SURFACE_STATE_INVALIDATED :
	               SVGA_SURFACE_STATE_CREATED;

      if (ss->sws->have_gb_objects) {
         /* Initialize the surface with zero */
         ss->sws->surface_init(ss->sws, sbuf->handle, svga_surface_size(&sbuf->key),
                               sbuf->key.flags);
      }
   }

   return ret;
}


/**
 * Recreates a host surface with the new bind flags.
 */
enum pipe_error
svga_buffer_recreate_host_surface(struct svga_context *svga,
                                  struct svga_buffer *sbuf,
                                  unsigned bind_flags)
{
   enum pipe_error ret = PIPE_OK;
   struct svga_winsys_surface *old_handle = sbuf->handle;

   assert(sbuf->bind_flags != bind_flags);
   assert(old_handle);

   sbuf->handle = NULL;

   /* Create a new resource with the requested bind_flags */
   ret = svga_buffer_create_host_surface(svga_screen(svga->pipe.screen),
                                         sbuf, bind_flags);
   if (ret == PIPE_OK) {
      /* Copy the surface data */
      assert(sbuf->handle);
      assert(sbuf->bufsurf);
      SVGA_RETRY(svga, SVGA3D_vgpu10_BufferCopy(svga->swc, old_handle,
                                                sbuf->handle,
                                                0, 0, sbuf->b.width0));

      /* Mark this surface as RENDERED */
      sbuf->bufsurf->surface_state = SVGA_SURFACE_STATE_RENDERED;
   }

   /* Set the new bind flags for this buffer resource */
   sbuf->bind_flags = bind_flags;

   /* Set the dirty bit to signal a read back is needed before the data copied
    * to this new surface can be referenced.
    */
   sbuf->dirty = true;

   return ret;
}


/**
 * Returns TRUE if the surface bind flags is compatible with the new bind flags.
 */
static bool
compatible_bind_flags(unsigned bind_flags,
                      unsigned tobind_flags)
{
   if ((bind_flags & tobind_flags) == tobind_flags)
      return true;
   else if ((bind_flags|tobind_flags) & PIPE_BIND_CONSTANT_BUFFER)
      return false;
   else if ((bind_flags & PIPE_BIND_STREAM_OUTPUT) &&
            (tobind_flags & (PIPE_BIND_SHADER_IMAGE | PIPE_BIND_SHADER_BUFFER)))
      /* Stream out cannot be mixed with UAV */
      return false;
   else
      return true;
}


/**
 * Returns a buffer surface from the surface list
 * that has the requested bind flags or its existing bind flags
 * can be promoted to include the new bind flags.
 */
static struct svga_buffer_surface *
svga_buffer_get_host_surface(struct svga_buffer *sbuf,
                             unsigned bind_flags)
{
   struct svga_buffer_surface *bufsurf;

   LIST_FOR_EACH_ENTRY(bufsurf, &sbuf->surfaces, list) {
      if (compatible_bind_flags(bufsurf->bind_flags, bind_flags))
         return bufsurf;
   }
   return NULL;
}


/**
 * Adds the host surface to the buffer surface list.
 */
struct svga_buffer_surface *
svga_buffer_add_host_surface(struct svga_buffer *sbuf,
                             struct svga_winsys_surface *handle,
                             struct svga_host_surface_cache_key *key,
                             unsigned bind_flags)
{
   struct svga_buffer_surface *bufsurf;

   bufsurf = CALLOC_STRUCT(svga_buffer_surface);
   if (!bufsurf)
      return NULL;

   bufsurf->bind_flags = bind_flags;
   bufsurf->handle = handle;
   bufsurf->key = *key;

   /* add the surface to the surface list */
   list_add(&bufsurf->list, &sbuf->surfaces);

   /* Set the new bind flags for this buffer resource */
   sbuf->bind_flags = bind_flags;

   return bufsurf;
}


/**
 * Start using the specified surface for this buffer resource.
 */
void
svga_buffer_bind_host_surface(struct svga_context *svga,
                              struct svga_buffer *sbuf,
                              struct svga_buffer_surface *bufsurf)
{
   /* Update the to-bind surface */
   assert(bufsurf->handle);
   assert(sbuf->handle);

   /* If we are switching from stream output to other buffer,
    * make sure to copy the buffer content.
    */
   if (sbuf->bind_flags & PIPE_BIND_STREAM_OUTPUT) {
      SVGA_RETRY(svga, SVGA3D_vgpu10_BufferCopy(svga->swc, sbuf->handle,
                                                bufsurf->handle,
                                                0, 0, sbuf->b.width0));
      bufsurf->surface_state = SVGA_SURFACE_STATE_RENDERED;
   }

   /* Set this surface as the current one */
   sbuf->handle = bufsurf->handle;
   sbuf->key = bufsurf->key;
   sbuf->bind_flags = bufsurf->bind_flags;
   sbuf->bufsurf = bufsurf;
}


/**
 * Prepare a host surface that can be used as indicated in the
 * tobind_flags. If the existing host surface is not created
 * with the necessary binding flags and if the new bind flags can be
 * combined with the existing bind flags, then we will recreate a
 * new surface with the combined bind flags. Otherwise, we will create
 * a surface for that incompatible bind flags.
 * For example, if a stream output buffer is reused as a constant buffer,
 * since constant buffer surface cannot be bound as a stream output surface,
 * two surfaces will be created, one for stream output,
 * and another one for constant buffer.
 */
enum pipe_error
svga_buffer_validate_host_surface(struct svga_context *svga,
                                  struct svga_buffer *sbuf,
                                  unsigned tobind_flags)
{
   struct svga_buffer_surface *bufsurf;
   enum pipe_error ret = PIPE_OK;

   /* upload any dirty ranges */
   svga_buffer_upload_ranges(svga, sbuf);

   /* Flush any pending upload first */
   svga_buffer_upload_flush(svga, sbuf);

   /* First check from the cached buffer surface list to see if there is
    * already a buffer surface that has the requested bind flags, or
    * surface with compatible bind flags that can be promoted.
    */
   bufsurf = svga_buffer_get_host_surface(sbuf, tobind_flags);

   if (bufsurf) {
      if ((bufsurf->bind_flags & tobind_flags) == tobind_flags) {
         /* there is a surface with the requested bind flags */
         svga_buffer_bind_host_surface(svga, sbuf, bufsurf);
      } else {

         /* Recreate a host surface with the combined bind flags */
         ret = svga_buffer_recreate_host_surface(svga, sbuf,
                                                 bufsurf->bind_flags |
                                                 tobind_flags);

         /* Destroy the old surface */
         svga_screen_surface_destroy(svga_screen(sbuf->b.screen),
                                     &bufsurf->key,
                                     svga_was_buffer_rendered_to(bufsurf),
                                     &bufsurf->handle);

         list_del(&bufsurf->list);
         FREE(bufsurf);
      }
   } else {
      /* Need to create a new surface if the bind flags are incompatible,
       * such as constant buffer surface & stream output surface.
       */
      ret = svga_buffer_recreate_host_surface(svga, sbuf,
                                              tobind_flags);
   }
   return ret;
}


void
svga_buffer_destroy_host_surface(struct svga_screen *ss,
                                 struct svga_buffer *sbuf)
{
   struct svga_buffer_surface *bufsurf, *next;

   LIST_FOR_EACH_ENTRY_SAFE(bufsurf, next, &sbuf->surfaces, list) {
      SVGA_DBG(DEBUG_DMA, " ungrab sid %p sz %d\n",
               bufsurf->handle, sbuf->b.width0);
      svga_screen_surface_destroy(ss, &bufsurf->key,
                                  svga_was_buffer_rendered_to(bufsurf),
                                  &bufsurf->handle);
      FREE(bufsurf);
   }
}


/**
 * Insert a number of preliminary UPDATE_GB_IMAGE commands in the
 * command buffer, equal to the current number of mapped ranges.
 * The UPDATE_GB_IMAGE commands will be patched with the
 * actual ranges just before flush.
 */
static enum pipe_error
svga_buffer_upload_gb_command(struct svga_context *svga,
                              struct svga_buffer *sbuf)
{
   struct svga_winsys_context *swc = svga->swc;
   SVGA3dCmdUpdateGBImage *update_cmd;
   struct svga_3d_update_gb_image *whole_update_cmd = NULL;
   const uint32 numBoxes = sbuf->map.num_ranges;
   struct pipe_resource *dummy;
   unsigned i;

   if (swc->force_coherent || sbuf->key.coherent)
      return PIPE_OK;

   assert(svga_have_gb_objects(svga));
   assert(numBoxes);
   assert(sbuf->dma.updates == NULL);

   /* Allocate FIFO space for 'numBoxes' UPDATE_GB_IMAGE commands */
   const unsigned total_commands_size =
      sizeof(*update_cmd) + (numBoxes - 1) * sizeof(*whole_update_cmd);

   update_cmd = SVGA3D_FIFOReserve(swc,
                                   SVGA_3D_CMD_UPDATE_GB_IMAGE,
                                   total_commands_size, numBoxes);
   if (!update_cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   /* The whole_update_command is a SVGA3dCmdHeader plus the
    * SVGA3dCmdUpdateGBImage command.
    */
   whole_update_cmd = container_of(update_cmd, struct svga_3d_update_gb_image, body);

   /* Init the first UPDATE_GB_IMAGE command */
   whole_update_cmd->header.size = sizeof(*update_cmd);
   swc->surface_relocation(swc, &update_cmd->image.sid, NULL, sbuf->handle,
                           SVGA_RELOC_WRITE | SVGA_RELOC_INTERNAL);
   update_cmd->image.face = 0;
   update_cmd->image.mipmap = 0;

   /* Save pointer to the first UPDATE_GB_IMAGE command so that we can
    * fill in the box info below.
    */
   sbuf->dma.updates = whole_update_cmd;

   /*
    * Copy the face, mipmap, etc. info to all subsequent commands.
    * Also do the surface relocation for each subsequent command.
    */
   for (i = 1; i < numBoxes; ++i) {
      whole_update_cmd++;
      memcpy(whole_update_cmd, sbuf->dma.updates, sizeof(*whole_update_cmd));

      swc->surface_relocation(swc, &whole_update_cmd->body.image.sid, NULL,
                              sbuf->handle,
                              SVGA_RELOC_WRITE | SVGA_RELOC_INTERNAL);
   }

   /* Increment reference count */
   sbuf->dma.svga = svga;
   dummy = NULL;
   pipe_resource_reference(&dummy, &sbuf->b);
   SVGA_FIFOCommitAll(swc);

   swc->hints |= SVGA_HINT_FLAG_CAN_PRE_FLUSH;
   sbuf->dma.flags.discard = false;

   svga->hud.num_resource_updates++;

   return PIPE_OK;
}


/**
 * Issue DMA commands to transfer guest memory to the host.
 * Note that the memory segments (offset, size) will be patched in
 * later in the svga_buffer_upload_flush() function.
 */
static enum pipe_error
svga_buffer_upload_hb_command(struct svga_context *svga,
                              struct svga_buffer *sbuf)
{
   struct svga_winsys_context *swc = svga->swc;
   struct svga_winsys_buffer *guest = sbuf->hwbuf;
   struct svga_winsys_surface *host = sbuf->handle;
   const SVGA3dTransferType transfer = SVGA3D_WRITE_HOST_VRAM;
   SVGA3dCmdSurfaceDMA *cmd;
   const uint32 numBoxes = sbuf->map.num_ranges;
   SVGA3dCopyBox *boxes;
   SVGA3dCmdSurfaceDMASuffix *pSuffix;
   unsigned region_flags;
   unsigned surface_flags;
   struct pipe_resource *dummy;

   assert(!svga_have_gb_objects(svga));

   if (transfer == SVGA3D_WRITE_HOST_VRAM) {
      region_flags = SVGA_RELOC_READ;
      surface_flags = SVGA_RELOC_WRITE;
   }
   else if (transfer == SVGA3D_READ_HOST_VRAM) {
      region_flags = SVGA_RELOC_WRITE;
      surface_flags = SVGA_RELOC_READ;
   }
   else {
      assert(0);
      return PIPE_ERROR_BAD_INPUT;
   }

   assert(numBoxes);

   cmd = SVGA3D_FIFOReserve(swc,
                            SVGA_3D_CMD_SURFACE_DMA,
                            sizeof *cmd + numBoxes * sizeof *boxes + sizeof *pSuffix,
                            2);
   if (!cmd)
      return PIPE_ERROR_OUT_OF_MEMORY;

   swc->region_relocation(swc, &cmd->guest.ptr, guest, 0, region_flags);
   cmd->guest.pitch = 0;

   swc->surface_relocation(swc, &cmd->host.sid, NULL, host, surface_flags);
   cmd->host.face = 0;
   cmd->host.mipmap = 0;

   cmd->transfer = transfer;

   sbuf->dma.boxes = (SVGA3dCopyBox *)&cmd[1];
   sbuf->dma.svga = svga;

   /* Increment reference count */
   dummy = NULL;
   pipe_resource_reference(&dummy, &sbuf->b);

   pSuffix = (SVGA3dCmdSurfaceDMASuffix *)((uint8_t*)cmd + sizeof *cmd + numBoxes * sizeof *boxes);
   pSuffix->suffixSize = sizeof *pSuffix;
   pSuffix->maximumOffset = sbuf->b.width0;
   pSuffix->flags = sbuf->dma.flags;

   SVGA_FIFOCommitAll(swc);

   swc->hints |= SVGA_HINT_FLAG_CAN_PRE_FLUSH;
   sbuf->dma.flags.discard = false;

   svga->hud.num_buffer_uploads++;

   return PIPE_OK;
}


/**
 * Issue commands to transfer guest memory to the host.
 */
static enum pipe_error
svga_buffer_upload_command(struct svga_context *svga, struct svga_buffer *sbuf)
{
   if (svga_have_gb_objects(svga)) {
      return svga_buffer_upload_gb_command(svga, sbuf);
   } else {
      return svga_buffer_upload_hb_command(svga, sbuf);
   }
}


/**
 * Patch up the upload DMA command reserved by svga_buffer_upload_command
 * with the final ranges.
 */
void
svga_buffer_upload_flush(struct svga_context *svga, struct svga_buffer *sbuf)
{
   unsigned i;
   struct pipe_resource *dummy;

   if (!sbuf->dma.pending || svga->swc->force_coherent ||
       sbuf->key.coherent) {
      //debug_printf("no dma pending on buffer\n");
      return;
   }

   assert(sbuf->handle);
   assert(sbuf->map.num_ranges);
   assert(sbuf->dma.svga == svga);

   /*
    * Patch the DMA/update command with the final copy box.
    */
   if (svga_have_gb_objects(svga)) {
      struct svga_3d_update_gb_image *update = sbuf->dma.updates;

      assert(update);

      for (i = 0; i < sbuf->map.num_ranges; ++i, ++update) {
         SVGA3dBox *box = &update->body.box;

         SVGA_DBG(DEBUG_DMA, "  bytes %u - %u\n",
                  sbuf->map.ranges[i].start, sbuf->map.ranges[i].end);

         box->x = sbuf->map.ranges[i].start;
         box->y = 0;
         box->z = 0;
         box->w = sbuf->map.ranges[i].end - sbuf->map.ranges[i].start;
         box->h = 1;
         box->d = 1;

         assert(box->x <= sbuf->b.width0);
         assert(box->x + box->w <= sbuf->b.width0);

         svga->hud.num_bytes_uploaded += box->w;
         svga->hud.num_buffer_uploads++;
      }
   }
   else {
      assert(sbuf->hwbuf);
      assert(sbuf->dma.boxes);
      SVGA_DBG(DEBUG_DMA, "dma to sid %p\n", sbuf->handle);

      for (i = 0; i < sbuf->map.num_ranges; ++i) {
         SVGA3dCopyBox *box = sbuf->dma.boxes + i;

         SVGA_DBG(DEBUG_DMA, "  bytes %u - %u\n",
               sbuf->map.ranges[i].start, sbuf->map.ranges[i].end);

         box->x = sbuf->map.ranges[i].start;
         box->y = 0;
         box->z = 0;
         box->w = sbuf->map.ranges[i].end - sbuf->map.ranges[i].start;
         box->h = 1;
         box->d = 1;
         box->srcx = sbuf->map.ranges[i].start;
         box->srcy = 0;
         box->srcz = 0;

         assert(box->x <= sbuf->b.width0);
         assert(box->x + box->w <= sbuf->b.width0);

         svga->hud.num_bytes_uploaded += box->w;
         svga->hud.num_buffer_uploads++;
      }
   }

   /* Reset sbuf for next use/upload */

   sbuf->map.num_ranges = 0;

   assert(sbuf->head.prev && sbuf->head.next);
   list_del(&sbuf->head);  /* remove from svga->dirty_buffers list */
   sbuf->dma.pending = false;
   sbuf->dma.flags.discard = false;
   sbuf->dma.flags.unsynchronized = false;

   sbuf->dma.svga = NULL;
   sbuf->dma.boxes = NULL;
   sbuf->dma.updates = NULL;

   /* Decrement reference count (and potentially destroy) */
   dummy = &sbuf->b;
   pipe_resource_reference(&dummy, NULL);
}


/**
 * Note a dirty range.
 *
 * This function only notes the range down. It doesn't actually emit a DMA
 * upload command. That only happens when a context tries to refer to this
 * buffer, and the DMA upload command is added to that context's command
 * buffer.
 *
 * We try to lump as many contiguous DMA transfers together as possible.
 */
void
svga_buffer_add_range(struct svga_buffer *sbuf, unsigned start, unsigned end)
{
   unsigned i;
   unsigned nearest_range;
   unsigned nearest_dist;

   assert(end > start);

   if (sbuf->map.num_ranges < SVGA_BUFFER_MAX_RANGES) {
      nearest_range = sbuf->map.num_ranges;
      nearest_dist = ~0;
   } else {
      nearest_range = SVGA_BUFFER_MAX_RANGES - 1;
      nearest_dist = 0;
   }

   /*
    * Try to grow one of the ranges.
    */
   for (i = 0; i < sbuf->map.num_ranges; ++i) {
      const int left_dist = start - sbuf->map.ranges[i].end;
      const int right_dist = sbuf->map.ranges[i].start - end;
      const int dist = MAX2(left_dist, right_dist);

      if (dist <= 0) {
         /*
          * Ranges are contiguous or overlapping -- extend this one and return.
          *
          * Note that it is not this function's task to prevent overlapping
          * ranges, as the GMR was already given so it is too late to do
          * anything.  If the ranges overlap here it must surely be because
          * PIPE_MAP_UNSYNCHRONIZED was set.
          */
         sbuf->map.ranges[i].start = MIN2(sbuf->map.ranges[i].start, start);
         sbuf->map.ranges[i].end   = MAX2(sbuf->map.ranges[i].end,   end);
         return;
      }
      else {
         /*
          * Discontiguous ranges -- keep track of the nearest range.
          */
         if (dist < nearest_dist) {
            nearest_range = i;
            nearest_dist = dist;
         }
      }
   }

   /*
    * We cannot add a new range to an existing DMA command, so patch-up the
    * pending DMA upload and start clean.
    */

   svga_buffer_upload_flush(sbuf->dma.svga, sbuf);

   assert(!sbuf->dma.pending);
   assert(!sbuf->dma.svga);
   assert(!sbuf->dma.boxes);

   if (sbuf->map.num_ranges < SVGA_BUFFER_MAX_RANGES) {
      /*
       * Add a new range.
       */

      sbuf->map.ranges[sbuf->map.num_ranges].start = start;
      sbuf->map.ranges[sbuf->map.num_ranges].end = end;
      ++sbuf->map.num_ranges;
   } else {
      /*
       * Everything else failed, so just extend the nearest range.
       *
       * It is OK to do this because we always keep a local copy of the
       * host buffer data, for SW TNL, and the host never modifies the buffer.
       */

      assert(nearest_range < SVGA_BUFFER_MAX_RANGES);
      assert(nearest_range < sbuf->map.num_ranges);
      sbuf->map.ranges[nearest_range].start =
         MIN2(sbuf->map.ranges[nearest_range].start, start);
      sbuf->map.ranges[nearest_range].end =
         MAX2(sbuf->map.ranges[nearest_range].end, end);
   }
}


/**
 * Copy the contents of the malloc buffer to a hardware buffer.
 */
static enum pipe_error
svga_buffer_update_hw(struct svga_context *svga, struct svga_buffer *sbuf,
                      unsigned bind_flags)
{
   assert(!sbuf->user);
   if (!svga_buffer_has_hw_storage(sbuf)) {
      struct svga_screen *ss = svga_screen(sbuf->b.screen);
      enum pipe_error ret;
      bool retry;
      void *map;
      unsigned i;

      assert(sbuf->swbuf);
      if (!sbuf->swbuf)
         return PIPE_ERROR;

      ret = svga_buffer_create_hw_storage(svga_screen(sbuf->b.screen), sbuf,
                                          bind_flags);
      if (ret != PIPE_OK)
         return ret;

      mtx_lock(&ss->swc_mutex);
      map = svga_buffer_hw_storage_map(svga, sbuf, PIPE_MAP_WRITE, &retry);
      assert(map);
      assert(!retry);
      if (!map) {
         mtx_unlock(&ss->swc_mutex);
         svga_buffer_destroy_hw_storage(ss, sbuf);
         return PIPE_ERROR;
      }

      /* Copy data from malloc'd swbuf to the new hardware buffer */
      for (i = 0; i < sbuf->map.num_ranges; i++) {
         unsigned start = sbuf->map.ranges[i].start;
         unsigned len = sbuf->map.ranges[i].end - start;
         memcpy((uint8_t *) map + start, (uint8_t *) sbuf->swbuf + start, len);
      }

      if (svga->swc->force_coherent || sbuf->key.coherent)
         sbuf->map.num_ranges = 0;

      svga_buffer_hw_storage_unmap(svga, sbuf);

      /* This user/malloc buffer is now indistinguishable from a gpu buffer */
      assert(sbuf->map.count == 0);
      if (sbuf->map.count == 0) {
         if (sbuf->user)
            sbuf->user = false;
         else
            align_free(sbuf->swbuf);
         sbuf->swbuf = NULL;
      }

      mtx_unlock(&ss->swc_mutex);
   }

   return PIPE_OK;
}


/**
 * Upload the buffer to the host in a piecewise fashion.
 *
 * Used when the buffer is too big to fit in the GMR aperture.
 * This function should never get called in the guest-backed case
 * since we always have a full-sized hardware storage backing the
 * host surface.
 */
static enum pipe_error
svga_buffer_upload_piecewise(struct svga_screen *ss,
                             struct svga_context *svga,
                             struct svga_buffer *sbuf)
{
   struct svga_winsys_screen *sws = ss->sws;
   const unsigned alignment = sizeof(void *);
   const unsigned usage = 0;
   unsigned i;

   assert(sbuf->map.num_ranges);
   assert(!sbuf->dma.pending);
   assert(!svga_have_gb_objects(svga));

   SVGA_DBG(DEBUG_DMA, "dma to sid %p\n", sbuf->handle);

   for (i = 0; i < sbuf->map.num_ranges; ++i) {
      const struct svga_buffer_range *range = &sbuf->map.ranges[i];
      unsigned offset = range->start;
      unsigned size = range->end - range->start;

      while (offset < range->end) {
         struct svga_winsys_buffer *hwbuf;
         uint8_t *map;

         if (offset + size > range->end)
            size = range->end - offset;

         hwbuf = sws->buffer_create(sws, alignment, usage, size);
         while (!hwbuf) {
            size /= 2;
            if (!size)
               return PIPE_ERROR_OUT_OF_MEMORY;
            hwbuf = sws->buffer_create(sws, alignment, usage, size);
         }

         SVGA_DBG(DEBUG_DMA, "  bytes %u - %u\n",
                  offset, offset + size);

         map = sws->buffer_map(sws, hwbuf,
                               PIPE_MAP_WRITE |
                               PIPE_MAP_DISCARD_RANGE);
         assert(map);
         if (map) {
            memcpy(map, (const char *) sbuf->swbuf + offset, size);
            sws->buffer_unmap(sws, hwbuf);
         }

         SVGA_RETRY(svga, SVGA3D_BufferDMA(svga->swc,
                                           hwbuf, sbuf->handle,
                                           SVGA3D_WRITE_HOST_VRAM,
                                           size, 0, offset, sbuf->dma.flags));
         sbuf->dma.flags.discard = false;

         sws->buffer_destroy(sws, hwbuf);

         offset += size;
      }
   }

   sbuf->map.num_ranges = 0;

   return PIPE_OK;
}


/**
 * A helper function to add an update command for the dirty ranges if there
 * isn't already one.
 */
static void
svga_buffer_upload_ranges(struct svga_context *svga,
                          struct svga_buffer *sbuf)
{
   struct pipe_screen *screen = svga->pipe.screen;
   struct svga_screen *ss = svga_screen(screen);
   enum pipe_error ret = PIPE_OK;

   if (sbuf->map.num_ranges) {
      if (!sbuf->dma.pending) {
         /* No pending DMA/update commands yet. */

         /* Migrate the data from swbuf -> hwbuf if necessary */
         ret = svga_buffer_update_hw(svga, sbuf, sbuf->bind_flags);
         if (ret == PIPE_OK) {
            /* Emit DMA or UpdateGBImage commands */
            SVGA_RETRY_OOM(svga, ret, svga_buffer_upload_command(svga, sbuf));
            if (ret == PIPE_OK) {
               sbuf->dma.pending = true;
               assert(!sbuf->head.prev && !sbuf->head.next);
               list_addtail(&sbuf->head, &svga->dirty_buffers);
            }
         }
         else if (ret == PIPE_ERROR_OUT_OF_MEMORY) {
            /*
             * The buffer is too big to fit in the GMR aperture, so break it in
             * smaller pieces.
             */
            ret = svga_buffer_upload_piecewise(ss, svga, sbuf);
         }

         if (ret != PIPE_OK) {
            /*
             * Something unexpected happened above. There is very little that
             * we can do other than proceeding while ignoring the dirty ranges.
             */
            assert(0);
            sbuf->map.num_ranges = 0;
         }
      }
      else {
         /*
          * There a pending dma already. Make sure it is from this context.
          */
         assert(sbuf->dma.svga == svga);
      }
   }
   return;
}


/**
 * Get (or create/upload) the winsys surface handle so that we can
 * refer to this buffer in fifo commands.
 * This function will create the host surface, and in the GB case also the
 * hardware storage. In the non-GB case, the hardware storage will be created
 * if there are mapped ranges and the data is currently in a malloc'ed buffer.
 */
struct svga_winsys_surface *
svga_buffer_handle(struct svga_context *svga, struct pipe_resource *buf,
                   unsigned tobind_flags)
{
   struct pipe_screen *screen = svga->pipe.screen;
   struct svga_screen *ss = svga_screen(screen);
   struct svga_buffer *sbuf;
   enum pipe_error ret;

   if (!buf)
      return NULL;

   sbuf = svga_buffer(buf);

   assert(!sbuf->user);

   if (sbuf->handle) {
      if ((sbuf->bind_flags & tobind_flags) != tobind_flags) {
         /* If the allocated resource's bind flags do not include the
          * requested bind flags, validate the host surface.
          */
         ret = svga_buffer_validate_host_surface(svga, sbuf, tobind_flags);
         if (ret != PIPE_OK)
            return NULL;
      }
   } else {
      /* If there is no resource handle yet, then combine the buffer bind
       * flags and the tobind_flags if they are compatible.
       * If not, just use the tobind_flags for creating the resource handle.
       */
      if (compatible_bind_flags(sbuf->bind_flags, tobind_flags))
         sbuf->bind_flags = sbuf->bind_flags | tobind_flags;
      else
         sbuf->bind_flags = tobind_flags;

      assert((sbuf->bind_flags & tobind_flags) == tobind_flags);

      /* This call will set sbuf->handle */
      if (svga_have_gb_objects(svga)) {
         ret = svga_buffer_update_hw(svga, sbuf, sbuf->bind_flags);
      } else {
         ret = svga_buffer_create_host_surface(ss, sbuf, sbuf->bind_flags);
      }
      if (ret != PIPE_OK)
         return NULL;
   }

   assert(sbuf->handle);
   assert(sbuf->bufsurf);
   if (svga->swc->force_coherent || sbuf->key.coherent)
      return sbuf->handle;

   /* upload any dirty ranges */
   svga_buffer_upload_ranges(svga, sbuf);

   assert(sbuf->map.num_ranges == 0 || sbuf->dma.pending);

   return sbuf->handle;
}


void
svga_context_flush_buffers(struct svga_context *svga)
{
   struct list_head *curr, *next;

   SVGA_STATS_TIME_PUSH(svga_sws(svga), SVGA_STATS_TIME_BUFFERSFLUSH);

   curr = svga->dirty_buffers.next;
   next = curr->next;
   while (curr != &svga->dirty_buffers) {
      struct svga_buffer *sbuf = list_entry(curr, struct svga_buffer, head);

      assert(p_atomic_read(&sbuf->b.reference.count) != 0);
      assert(sbuf->dma.pending);

      svga_buffer_upload_flush(svga, sbuf);

      curr = next;
      next = curr->next;
   }

   SVGA_STATS_TIME_POP(svga_sws(svga));
}
