/**********************************************************
 * Copyright 2014 VMware, Inc.  All rights reserved.
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

#include "util/u_memory.h"
#include "util/u_bitmask.h"

#include "svga_cmd.h"
#include "svga_context.h"
#include "svga_resource_buffer.h"
#include "svga_shader.h"
#include "svga_debug.h"
#include "svga_streamout.h"

struct svga_stream_output_target {
   struct pipe_stream_output_target base;
};

/** cast wrapper */
static inline struct svga_stream_output_target *
svga_stream_output_target(struct pipe_stream_output_target *s)
{
   return (struct svga_stream_output_target *)s;
}


/**
 * A helper function to send different version of the DefineStreamOutput command
 * depending on if device is SM5 capable or not.
 */
static enum pipe_error
svga_define_stream_output(struct svga_context *svga,
       SVGA3dStreamOutputId soid,
       uint32 numOutputStreamEntries,
       uint32 numOutputStreamStrides,
       uint32 streamStrides[SVGA3D_DX_MAX_SOTARGETS],
       const SVGA3dStreamOutputDeclarationEntry decls[SVGA3D_MAX_STREAMOUT_DECLS],
       uint32 rasterizedStream,
       struct svga_stream_output *streamout)
{
   unsigned i;

   SVGA_DBG(DEBUG_STREAMOUT, "%s: id=%d\n", __func__, soid);
   SVGA_DBG(DEBUG_STREAMOUT,
            "numOutputStreamEntires=%d\n", numOutputStreamEntries);

   for (i = 0; i < numOutputStreamEntries; i++) {
      SVGA_DBG(DEBUG_STREAMOUT,
               "  %d: slot=%d regIdx=%d regMask=0x%x stream=%d\n",
               i, decls[i].outputSlot, decls[i].registerIndex,
               decls[i].registerMask, decls[i].stream);
   }

   SVGA_DBG(DEBUG_STREAMOUT,
            "numOutputStreamStrides=%d\n", numOutputStreamStrides);
   for (i = 0; i < numOutputStreamStrides; i++) {
      SVGA_DBG(DEBUG_STREAMOUT, "  %d ", streamStrides[i]);
   }
   SVGA_DBG(DEBUG_STREAMOUT, "\n");

   if (svga_have_sm5(svga) &&
       (numOutputStreamEntries > SVGA3D_MAX_DX10_STREAMOUT_DECLS ||
        numOutputStreamStrides > 1)) {
      unsigned bufSize = sizeof(SVGA3dStreamOutputDeclarationEntry)
         * numOutputStreamEntries;
      struct svga_winsys_buffer *declBuf;
      struct svga_winsys_screen *sws = svga_screen(svga->pipe.screen)->sws;
      void *map;

      declBuf = svga_winsys_buffer_create(svga, 1, SVGA_BUFFER_USAGE_PINNED,
                                          bufSize);
      if (!declBuf)
         return PIPE_ERROR;
      map = sws->buffer_map(sws, declBuf, PIPE_MAP_WRITE);
      if (!map) {
         sws->buffer_destroy(sws, declBuf);
         return PIPE_ERROR;
      }

      /* copy decls to buffer */
      memcpy(map, decls, bufSize);

      /* unmap buffer */
      sws->buffer_unmap(sws, declBuf);
      streamout->declBuf = declBuf;

      SVGA_RETRY(svga, SVGA3D_sm5_DefineAndBindStreamOutput
                 (svga->swc, soid,
                  numOutputStreamEntries,
                  numOutputStreamStrides,
                  streamStrides,
                  streamout->declBuf,
                  rasterizedStream,
                  bufSize));
   } else {
      SVGA_RETRY(svga, SVGA3D_vgpu10_DefineStreamOutput(svga->swc, soid,
                                                        numOutputStreamEntries,
                                                        streamStrides,
                                                        decls));
   }

   return PIPE_OK;
}


/**
 * Creates stream output from the stream output info.
 */
struct svga_stream_output *
svga_create_stream_output(struct svga_context *svga,
                          struct svga_shader *shader,
                          const struct pipe_stream_output_info *info)
{
   struct svga_stream_output *streamout;
   SVGA3dStreamOutputDeclarationEntry decls[SVGA3D_MAX_STREAMOUT_DECLS];
   unsigned strides[SVGA3D_DX_MAX_SOTARGETS];
   unsigned dstOffset[SVGA3D_DX_MAX_SOTARGETS];
   unsigned numStreamStrides = 0;
   unsigned numDecls;
   unsigned i;
   enum pipe_error ret;
   unsigned id;
   ASSERTED unsigned maxDecls = 0;

   assert(info->num_outputs <= PIPE_MAX_SO_OUTPUTS);

   /* Gallium utility creates shaders with stream output.
    * For non-DX10, just return NULL.
    */
   if (!svga_have_vgpu10(svga))
      return NULL;

   if (svga_have_sm5(svga))
      maxDecls = SVGA3D_MAX_STREAMOUT_DECLS;
   else if (svga_have_vgpu10(svga))
      maxDecls = SVGA3D_MAX_DX10_STREAMOUT_DECLS;

   assert(info->num_outputs <= maxDecls);

   /* Allocate an integer ID for the stream output */
   id = util_bitmask_add(svga->stream_output_id_bm);
   if (id == UTIL_BITMASK_INVALID_INDEX) {
      return NULL;
   }

   /* Allocate the streamout data structure */
   streamout = CALLOC_STRUCT(svga_stream_output);

   if (!streamout)
      return NULL;

   streamout->info = *info;
   streamout->id = id;
   streamout->pos_out_index = -1;
   streamout->streammask = 0;

   /* Init whole decls and stride arrays to zero to avoid garbage values */
   memset(decls, 0, sizeof(decls));
   memset(strides, 0, sizeof(strides));
   memset(dstOffset, 0, sizeof(dstOffset));

   SVGA_DBG(DEBUG_STREAMOUT, "%s: num_outputs=%d\n",
            __func__, info->num_outputs);

   for (i = 0, numDecls = 0; i < info->num_outputs; i++, numDecls++) {
      unsigned reg_idx = info->output[i].register_index;
      unsigned buf_idx = info->output[i].output_buffer;
      const enum tgsi_semantic sem_name =
         shader->tgsi_info.output_semantic_name[reg_idx];

      assert(buf_idx <= PIPE_MAX_SO_BUFFERS);

      numStreamStrides = MAX2(numStreamStrides, buf_idx);

      SVGA_DBG(DEBUG_STREAMOUT,
               "  %d: register_index=%d output_buffer=%d stream=%d\n",
               i, reg_idx, buf_idx, info->output[i].stream);

      SVGA_DBG(DEBUG_STREAMOUT,
               "     dst_offset=%d start_component=%d num_components=%d\n",
               info->output[i].dst_offset,
               info->output[i].start_component,
               info->output[i].num_components);

      streamout->buffer_stream |= info->output[i].stream << (buf_idx * 4);

      /**
       * Check if the destination offset of the current output
       * is at the expected offset. If it is greater, then that means
       * there is a gap in the stream output. We need to insert
       * extra declaration entries with an invalid register index
       * to specify a gap.
       */
      while (info->output[i].dst_offset > dstOffset[buf_idx]) {

         unsigned numComponents = info->output[i].dst_offset -
                                  dstOffset[buf_idx];;

         assert(svga_have_sm5(svga));

         /* We can only specify at most 4 components to skip in each
          * declaration entry.
          */
         numComponents = numComponents > 4 ? 4 : numComponents;

         decls[numDecls].outputSlot = buf_idx,
         decls[numDecls].stream = info->output[i].stream;
         decls[numDecls].registerIndex = SVGA3D_INVALID_ID;
         decls[numDecls].registerMask = (1 << numComponents) - 1;

         dstOffset[buf_idx] += numComponents;
         numDecls++;
      }

      if (sem_name == TGSI_SEMANTIC_POSITION) {
         /**
          * Check if streaming out POSITION. If so, replace the
          * register index with the index for NON_ADJUSTED POSITION.
          */
         decls[numDecls].registerIndex = shader->tgsi_info.num_outputs;

         /* Save this output index, so we can tell later if this stream output
          * includes an output of a vertex position
          */
         streamout->pos_out_index = numDecls;
      }
      else if (sem_name == TGSI_SEMANTIC_CLIPDIST) {
         /**
          * Use the shadow copy for clip distance because
          * CLIPDIST instruction is only emitted for enabled clip planes.
          * It's valid to write to ClipDistance variable for non-enabled
          * clip planes.
          */
         decls[numDecls].registerIndex =
            shader->tgsi_info.num_outputs + 1 +
            shader->tgsi_info.output_semantic_index[reg_idx];
      }
      else {
         decls[numDecls].registerIndex = reg_idx;
      }

      decls[numDecls].outputSlot = buf_idx;
      decls[numDecls].registerMask =
         ((1 << info->output[i].num_components) - 1)
            << info->output[i].start_component;

      decls[numDecls].stream = info->output[i].stream;
      assert(decls[numDecls].stream == 0 || svga_have_sm5(svga));

      /* Set the bit in streammask for the enabled stream */
      streamout->streammask |= 1 << info->output[i].stream;

      /* Update the expected offset for the next output */
      dstOffset[buf_idx] += info->output[i].num_components;

      strides[buf_idx] = info->stride[buf_idx] * sizeof(float);
   }

   assert(numDecls <= maxDecls);

   /* Send the DefineStreamOutput command.
    * Note, rasterizedStream is always 0.
    */
   ret = svga_define_stream_output(svga, id,
                                   numDecls, numStreamStrides+1,
                                   strides, decls, 0, streamout);

   if (ret != PIPE_OK) {
      util_bitmask_clear(svga->stream_output_id_bm, id);
      FREE(streamout);
      streamout = NULL;
   }
   return streamout;
}


enum pipe_error
svga_set_stream_output(struct svga_context *svga,
                       struct svga_stream_output *streamout)
{
   unsigned id = streamout ? streamout->id : SVGA3D_INVALID_ID;

   if (!svga_have_vgpu10(svga)) {
      return PIPE_OK;
   }

   SVGA_DBG(DEBUG_STREAMOUT, "%s streamout=0x%x id=%d\n", __func__,
            streamout, id);

   if (svga->current_so != streamout) {

      /* Before unbinding the current stream output, stop the stream output
       * statistics queries for the active streams.
       */
      if (svga_have_sm5(svga) && svga->current_so) {
         svga->vcount_buffer_stream = svga->current_so->buffer_stream;
         svga_end_stream_output_queries(svga, svga->current_so->streammask);
      }

      enum pipe_error ret = SVGA3D_vgpu10_SetStreamOutput(svga->swc, id);
      if (ret != PIPE_OK) {
         return ret;
      }

      svga->current_so = streamout;

      /* After binding the new stream output, start the stream output
       * statistics queries for the active streams.
       */
      if (svga_have_sm5(svga) && svga->current_so) {
         svga_begin_stream_output_queries(svga, svga->current_so->streammask);
      }
   }

   return PIPE_OK;
}

void
svga_delete_stream_output(struct svga_context *svga,
                          struct svga_stream_output *streamout)
{
   struct svga_winsys_screen *sws = svga_screen(svga->pipe.screen)->sws;

   SVGA_DBG(DEBUG_STREAMOUT, "%s streamout=0x%x\n", __func__, streamout);

   assert(svga_have_vgpu10(svga));
   assert(streamout != NULL);

   SVGA_RETRY(svga, SVGA3D_vgpu10_DestroyStreamOutput(svga->swc,
                                                      streamout->id));

   if (svga_have_sm5(svga) && streamout->declBuf) {
      sws->buffer_destroy(sws, streamout->declBuf);
   }

   /* Before deleting the current streamout, make sure to stop any pending
    * SO queries.
    */
   if (svga->current_so == streamout) {
      if (svga->in_streamout)
         svga_end_stream_output_queries(svga, svga->current_so->streammask);
      svga->current_so = NULL;
   }

   /* Release the ID */
   util_bitmask_clear(svga->stream_output_id_bm, streamout->id);

   /* Free streamout structure */
   FREE(streamout);
}


static struct pipe_stream_output_target *
svga_create_stream_output_target(struct pipe_context *pipe,
                                 struct pipe_resource *buffer,
                                 unsigned buffer_offset,
                                 unsigned buffer_size)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_stream_output_target *sot;

   SVGA_DBG(DEBUG_STREAMOUT, "%s offset=%d size=%d\n", __func__,
            buffer_offset, buffer_size);

   assert(svga_have_vgpu10(svga));
   (void) svga;

   sot = CALLOC_STRUCT(svga_stream_output_target);
   if (!sot)
      return NULL;

   pipe_reference_init(&sot->base.reference, 1);
   pipe_resource_reference(&sot->base.buffer, buffer);
   sot->base.context = pipe;
   sot->base.buffer = buffer;
   sot->base.buffer_offset = buffer_offset;
   sot->base.buffer_size = buffer_size;

   return &sot->base;
}

static void
svga_destroy_stream_output_target(struct pipe_context *pipe,
                                  struct pipe_stream_output_target *target)
{
   struct svga_stream_output_target *sot = svga_stream_output_target(target);

   SVGA_DBG(DEBUG_STREAMOUT, "%s\n", __func__);

   pipe_resource_reference(&sot->base.buffer, NULL);
   FREE(sot);
}

static void
svga_set_stream_output_targets(struct pipe_context *pipe,
                               unsigned num_targets,
                               struct pipe_stream_output_target **targets,
                               const unsigned *offsets)
{
   struct svga_context *svga = svga_context(pipe);
   struct SVGA3dSoTarget soBindings[SVGA3D_DX_MAX_SOTARGETS];
   unsigned i;
   unsigned num_so_targets;
   bool begin_so_queries = num_targets > 0;

   SVGA_DBG(DEBUG_STREAMOUT, "%s num_targets=%d\n", __func__,
            num_targets);

   assert(svga_have_vgpu10(svga));

   /* Mark the streamout buffers as dirty so that we'll issue readbacks
    * before mapping.
    */
   for (i = 0; i < svga->num_so_targets; i++) {
      struct svga_buffer *sbuf = svga_buffer(svga->so_targets[i]->buffer);
      sbuf->dirty = true;
   }

   /* Before the currently bound streamout targets are unbound,
    * save them in case they need to be referenced to retrieve the
    * number of vertices being streamed out.
    */
   for (i = 0; i < ARRAY_SIZE(svga->so_targets); i++) {
      svga->vcount_so_targets[i] = svga->so_targets[i];
   }

   assert(num_targets <= SVGA3D_DX_MAX_SOTARGETS);

   for (i = 0; i < num_targets; i++) {
      struct svga_stream_output_target *sot
         = svga_stream_output_target(targets[i]);
      struct svga_buffer *sbuf = svga_buffer(sot->base.buffer);
      unsigned size;

      svga->so_surfaces[i] = svga_buffer_handle(svga, sot->base.buffer,
                                                PIPE_BIND_STREAM_OUTPUT);

      assert(svga_buffer(sot->base.buffer)->key.flags
             & SVGA3D_SURFACE_BIND_STREAM_OUTPUT);

      /* Mark the buffer surface as RENDERED */
      assert(sbuf->bufsurf);
      sbuf->bufsurf->surface_state = SVGA_SURFACE_STATE_RENDERED;

      svga->so_targets[i] = &sot->base;
      if (offsets[i] == -1) {
         soBindings[i].offset = -1;

         /* The streamout is being resumed. There is no need to restart streamout statistics
          * queries for the draw-auto fallback since those queries are still active.
          */
         begin_so_queries = false;
      }
      else
         soBindings[i].offset = sot->base.buffer_offset + offsets[i];

      /* The size cannot extend beyond the end of the buffer.  Clamp it. */
      size = MIN2(sot->base.buffer_size,
                  sot->base.buffer->width0 - sot->base.buffer_offset);

      soBindings[i].sizeInBytes = size;
   }

   /* unbind any previously bound stream output buffers */
   for (; i < svga->num_so_targets; i++) {
      svga->so_surfaces[i] = NULL;
      svga->so_targets[i] = NULL;
   }

   num_so_targets = MAX2(svga->num_so_targets, num_targets);
   SVGA_RETRY(svga, SVGA3D_vgpu10_SetSOTargets(svga->swc, num_so_targets,
                                               soBindings, svga->so_surfaces));
   svga->num_so_targets = num_targets;

   if (svga_have_sm5(svga) && svga->current_so && begin_so_queries) {

      /* If there are already active queries and we need to start a new streamout,
       * we need to stop the current active queries first.
       */
      if (svga->in_streamout) {
         svga_end_stream_output_queries(svga, svga->current_so->streammask);
      }

      /* Start stream out statistics queries for the new streamout */
      svga_begin_stream_output_queries(svga, svga->current_so->streammask);
   }
}

/**
 * Rebind stream output target surfaces
 */
enum pipe_error
svga_rebind_stream_output_targets(struct svga_context *svga)
{
   struct svga_winsys_context *swc = svga->swc;
   enum pipe_error ret;
   unsigned i;

   for (i = 0; i < svga->num_so_targets; i++) {
      ret = swc->resource_rebind(swc, svga->so_surfaces[i], NULL, SVGA_RELOC_WRITE);
      if (ret != PIPE_OK)
         return ret;
   }

   return PIPE_OK;
}


void
svga_init_stream_output_functions(struct svga_context *svga)
{
   svga->pipe.create_stream_output_target = svga_create_stream_output_target;
   svga->pipe.stream_output_target_destroy = svga_destroy_stream_output_target;
   svga->pipe.set_stream_output_targets = svga_set_stream_output_targets;
}


/**
 * A helper function to create stream output statistics queries for each stream.
 * These queries are created as a workaround for DrawTransformFeedbackInstanced or
 * DrawTransformFeedbackStreamInstanced when auto draw doesn't support
 * instancing or non-0 stream. In this case, the vertex count will
 * be retrieved from the stream output statistics query.
 */
void
svga_create_stream_output_queries(struct svga_context *svga)
{
   unsigned i;

   if (!svga_have_sm5(svga))
      return;

   for (i = 0; i < ARRAY_SIZE(svga->so_queries); i++) {
      svga->so_queries[i] = svga->pipe.create_query(&svga->pipe,
                               PIPE_QUERY_SO_STATISTICS, i);
      assert(svga->so_queries[i] != NULL);
   }
}


/**
 * Destroy the stream output statistics queries for the draw-auto workaround.
 */
void
svga_destroy_stream_output_queries(struct svga_context *svga)
{
   unsigned i;

   if (!svga_have_sm5(svga))
      return;

   for (i = 0; i < ARRAY_SIZE(svga->so_queries); i++) {
      svga->pipe.destroy_query(&svga->pipe, svga->so_queries[i]);
   }
}


/**
 * Start stream output statistics queries for the active streams.
 */
void
svga_begin_stream_output_queries(struct svga_context *svga,
                                 unsigned streammask)
{
   assert(svga_have_sm5(svga));
   assert(!svga->in_streamout);

   for (unsigned i = 0; i < ARRAY_SIZE(svga->so_queries); i++) {
      bool ret;
      if (streammask & (1 << i)) {
         ret = svga->pipe.begin_query(&svga->pipe, svga->so_queries[i]);
      }
      (void) ret;
   }   
   svga->in_streamout = true;

   return;
}


/**
 * Stop stream output statistics queries for the active streams.
 */
void
svga_end_stream_output_queries(struct svga_context *svga,
                               unsigned streammask)
{
   assert(svga_have_sm5(svga));

   if (!svga->in_streamout)
      return;

   for (unsigned i = 0; i < ARRAY_SIZE(svga->so_queries); i++) {
      bool ret;
      if (streammask & (1 << i)) {
         ret = svga->pipe.end_query(&svga->pipe, svga->so_queries[i]);
      }
      (void) ret;
   }   
   svga->in_streamout = false;

   return;
}


/**
 * Return the primitive count returned from the stream output statistics query
 * for the specified stream.
 */
unsigned
svga_get_primcount_from_stream_output(struct svga_context *svga,
                                      unsigned stream)
{
   unsigned primcount = 0;
   union pipe_query_result result;
   bool ret;

   if (svga->current_so) {
      svga_end_stream_output_queries(svga, svga->current_so->streammask);
   }

   ret = svga->pipe.get_query_result(&svga->pipe,
                                     svga->so_queries[stream],
                                     true, &result);
   if (ret)
      primcount = result.so_statistics.num_primitives_written;

   return primcount;
}
