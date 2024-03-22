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

/*
 * This file implements the st_draw_vbo() function which is called from
 * Mesa's VBO module.  All point/line/triangle rendering is done through
 * this function whether the user called glBegin/End, glDrawArrays,
 * glDrawElements, glEvalMesh, or glCalList, etc.
 *
 * Authors:
 *   Keith Whitwell <keithw@vmware.com>
 */

#include "main/context.h"
#include "main/errors.h"

#include "main/image.h"
#include "main/bufferobj.h"
#include "main/macros.h"
#include "main/varray.h"

#include "compiler/glsl/ir_uniform.h"

#include "vbo/vbo.h"

#include "st_context.h"
#include "st_atom.h"
#include "st_cb_bitmap.h"
#include "st_debug.h"
#include "st_draw.h"
#include "st_program.h"
#include "st_util.h"

#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "util/u_cpu_detect.h"
#include "util/u_inlines.h"
#include "util/format/u_format.h"
#include "util/u_prim.h"
#include "util/u_draw.h"
#include "util/u_upload_mgr.h"
#include "util/u_threaded_context.h"
#include "draw/draw_context.h"
#include "cso_cache/cso_context.h"

/* GL prims should match Gallium prims, spot-check a few */
static_assert(GL_POINTS == MESA_PRIM_POINTS, "enum mismatch");
static_assert(GL_QUADS == MESA_PRIM_QUADS, "enum mismatch");
static_assert(GL_TRIANGLE_STRIP_ADJACENCY == MESA_PRIM_TRIANGLE_STRIP_ADJACENCY, "enum mismatch");
static_assert(GL_PATCHES == MESA_PRIM_PATCHES, "enum mismatch");

void
st_prepare_draw(struct gl_context *ctx, uint64_t state_mask)
{
   struct st_context *st = ctx->st;

   /* Mesa core state should have been validated already */
   assert(ctx->NewState == 0x0);

   if (unlikely(!st->bitmap.cache.empty))
      st_flush_bitmap_cache(st);

   st_invalidate_readpix_cache(st);

   /* Validate state. */
   st_validate_state(st, state_mask);

   /* Pin threads regularly to the same Zen CCX that the main thread is
    * running on. The main thread can move between CCXs.
    */
   if (unlikely(st->pin_thread_counter != ST_L3_PINNING_DISABLED &&
                /* do it occasionally */
                ++st->pin_thread_counter % 512 == 0)) {
      st->pin_thread_counter = 0;

      int cpu = util_get_current_cpu();
      if (cpu >= 0) {
         struct pipe_context *pipe = st->pipe;
         uint16_t L3_cache = util_get_cpu_caps()->cpu_to_L3[cpu];

         if (L3_cache != U_CPU_INVALID_L3) {
            pipe->set_context_param(pipe,
                                    PIPE_CONTEXT_PARAM_PIN_THREADS_TO_L3_CACHE,
                                    L3_cache);
         }
      }
   }
}

void
st_draw_gallium(struct gl_context *ctx,
                struct pipe_draw_info *info,
                unsigned drawid_offset,
                const struct pipe_draw_indirect_info *indirect,
                const struct pipe_draw_start_count_bias *draws,
                unsigned num_draws)
{
   struct st_context *st = st_context(ctx);

   cso_draw_vbo(st->cso_context, info, drawid_offset, indirect, draws, num_draws);
}

static void
st_draw_gallium_multimode(struct gl_context *ctx,
                          struct pipe_draw_info *info,
                          const struct pipe_draw_start_count_bias *draws,
                          const unsigned char *mode,
                          unsigned num_draws)
{
   struct st_context *st = st_context(ctx);

   unsigned i, first;
   struct cso_context *cso = st->cso_context;

   /* Find consecutive draws where mode doesn't vary. */
   for (i = 0, first = 0; i <= num_draws; i++) {
      if (i == num_draws || mode[i] != mode[first]) {
         info->mode = mode[first];
         cso_draw_vbo(cso, info, 0, NULL, &draws[first], i - first);
         first = i;

         /* We can pass the reference only once. st_buffer_object keeps
          * the reference alive for later draws.
          */
         info->take_index_buffer_ownership = false;
      }
   }
}

static void
rewrite_partial_stride_indirect(struct st_context *st,
                            const struct pipe_draw_info *info,
                            const struct pipe_draw_indirect_info *indirect,
                            const struct pipe_draw_start_count_bias draw)
{
   unsigned draw_count = 0;
   struct u_indirect_params *new_draws = util_draw_indirect_read(st->pipe, info, indirect, &draw_count);
   if (!new_draws)
      return;
   for (unsigned i = 0; i < draw_count; i++)
      st->ctx->Driver.DrawGallium(st->ctx, &new_draws[i].info, i, NULL, &new_draws[i].draw, 1);
   free(new_draws);
}

void
st_indirect_draw_vbo(struct gl_context *ctx,
                     GLenum mode, GLenum index_type,
                     GLintptr indirect_offset,
                     GLintptr indirect_draw_count_offset,
                     GLsizei draw_count, GLsizei stride)
{
   struct gl_buffer_object *indirect_data = ctx->DrawIndirectBuffer;
   struct gl_buffer_object *indirect_draw_count = ctx->ParameterBuffer;
   struct st_context *st = st_context(ctx);
   struct pipe_draw_info info;
   struct pipe_draw_indirect_info indirect;
   struct pipe_draw_start_count_bias draw = {0};

   /* If indirect_draw_count is set, drawcount is the maximum draw count.*/
   if (!draw_count)
      return;

   assert(stride);
   st_prepare_draw(ctx, ST_PIPELINE_RENDER_STATE_MASK);

   memset(&indirect, 0, sizeof(indirect));
   util_draw_init_info(&info);
   info.max_index = ~0u; /* so that u_vbuf can tell that it's unknown */

   switch (index_type) {
   case GL_UNSIGNED_BYTE:
      info.index_size = 1;
      break;
   case GL_UNSIGNED_SHORT:
      info.index_size = 2;
      break;
   case GL_UNSIGNED_INT:
      info.index_size = 4;
      break;
   }

   assert(st->has_multi_draw_indirect || !indirect_draw_count);

   if (info.index_size) {
      struct gl_buffer_object *bufobj = ctx->Array.VAO->IndexBufferObj;

      /* indices are always in a real VBO */
      assert(bufobj);

      if (st->pipe->draw_vbo == tc_draw_vbo &&
          (draw_count == 1 || st->has_multi_draw_indirect)) {
         /* Fast path for u_threaded_context to eliminate atomics. */
         info.index.resource = _mesa_get_bufferobj_reference(ctx, bufobj);
         info.take_index_buffer_ownership = true;
      } else {
         info.index.resource = bufobj->buffer;
      }

      /* No index buffer storage allocated - nothing to do. */
      if (!info.index.resource)
         return;

      draw.start = 0;

      unsigned index_size_shift = util_logbase2(info.index_size);
      info.restart_index = ctx->Array._RestartIndex[index_size_shift];
      info.primitive_restart = ctx->Array._PrimitiveRestart[index_size_shift];
   }

   info.mode = mode;
   indirect.buffer = indirect_data->buffer;
   indirect.offset = indirect_offset;

   /* Viewperf2020/Maya draws with a buffer that has no storage. */
   if (!indirect.buffer)
      return;

   if (!st->has_multi_draw_indirect) {
      int i;

      indirect.draw_count = 1;
      for (i = 0; i < draw_count; i++) {
         ctx->Driver.DrawGallium(ctx, &info, i, &indirect, &draw, 1);
         indirect.offset += stride;
      }
   } else {
      indirect.draw_count = draw_count;
      indirect.stride = stride;
      if (!st->has_indirect_partial_stride && stride &&
          (draw_count > 1 || indirect_draw_count)) {
         /* DrawElementsIndirectCommand or DrawArraysIndirectCommand */
         const size_t struct_size = info.index_size ? sizeof(uint32_t) * 5 : sizeof(uint32_t) * 4;
         if (indirect.stride && indirect.stride < struct_size) {
            rewrite_partial_stride_indirect(st, &info, &indirect, draw);
            return;
         }
      }
      if (indirect_draw_count) {
         indirect.indirect_draw_count =
            indirect_draw_count->buffer;
         indirect.indirect_draw_count_offset = indirect_draw_count_offset;
      }
      ctx->Driver.DrawGallium(ctx, &info, 0, &indirect, &draw, 1);
   }

   if (MESA_DEBUG_FLAGS & DEBUG_ALWAYS_FLUSH)
      _mesa_flush(ctx);
}

static void
st_draw_gallium_vertex_state(struct gl_context *ctx,
                             struct pipe_vertex_state *state,
                             struct pipe_draw_vertex_state_info info,
                             const struct pipe_draw_start_count_bias *draws,
                             const uint8_t *mode,
                             unsigned num_draws)
{
   struct st_context *st = st_context(ctx);

   st_prepare_draw(ctx, ST_PIPELINE_RENDER_STATE_MASK_NO_VARRAYS);

   struct pipe_context *pipe = st->pipe;
   uint32_t velem_mask = ctx->VertexProgram._Current->info.inputs_read;

   if (!mode) {
      pipe->draw_vertex_state(pipe, state, velem_mask, info, draws, num_draws);
   } else {
      /* Find consecutive draws where mode doesn't vary. */
      for (unsigned i = 0, first = 0; i <= num_draws; i++) {
         if (i == num_draws || mode[i] != mode[first]) {
            unsigned current_num_draws = i - first;

            /* Increase refcount to be able to use take_vertex_state_ownership
             * with all draws.
             */
            if (i != num_draws && info.take_vertex_state_ownership)
               p_atomic_inc(&state->reference.count);

            info.mode = mode[first];
            pipe->draw_vertex_state(pipe, state, velem_mask, info, &draws[first],
                                    current_num_draws);
            first = i;
         }
      }
   }
}

void
st_init_draw_functions(struct pipe_screen *screen,
                       struct dd_function_table *functions)
{
   functions->DrawGallium = st_draw_gallium;
   functions->DrawGalliumMultiMode = st_draw_gallium_multimode;

   if (screen->get_param(screen, PIPE_CAP_DRAW_VERTEX_STATE)) {
      functions->DrawGalliumVertexState = st_draw_gallium_vertex_state;
      functions->CreateGalliumVertexState = st_create_gallium_vertex_state;
   }
}


void
st_destroy_draw(struct st_context *st)
{
   draw_destroy(st->draw);
}

/**
 * Getter for the draw_context, so that initialization of it can happen only
 * when needed (the TGSI exec machines take up quite a bit of memory).
 */
struct draw_context *
st_get_draw_context(struct st_context *st)
{
   if (!st->draw) {
      st->draw = draw_create(st->pipe);
      if (!st->draw) {
         _mesa_error(st->ctx, GL_OUT_OF_MEMORY, "feedback fallback allocation");
         return NULL;
      }
   }

   /* Disable draw options that might convert points/lines to tris, etc.
    * as that would foul-up feedback/selection mode.
    */
   draw_wide_line_threshold(st->draw, 1000.0f);
   draw_wide_point_threshold(st->draw, 1000.0f);
   draw_enable_line_stipple(st->draw, false);
   draw_enable_point_sprites(st->draw, false);

   return st->draw;
}

/**
 * Draw a quad with given position, texcoords and color.
 */
bool
st_draw_quad(struct st_context *st,
             float x0, float y0, float x1, float y1, float z,
             float s0, float t0, float s1, float t1,
             const float *color,
             unsigned num_instances)
{
   struct pipe_vertex_buffer vb = {0};
   struct st_util_vertex *verts;

   u_upload_alloc(st->pipe->stream_uploader, 0,
                  4 * sizeof(struct st_util_vertex), 4,
                  &vb.buffer_offset, &vb.buffer.resource, (void **) &verts);
   if (!vb.buffer.resource) {
      return false;
   }

   /* lower-left */
   verts[0].x = x0;
   verts[0].y = y1;
   verts[0].z = z;
   verts[0].r = color[0];
   verts[0].g = color[1];
   verts[0].b = color[2];
   verts[0].a = color[3];
   verts[0].s = s0;
   verts[0].t = t0;

   /* lower-right */
   verts[1].x = x1;
   verts[1].y = y1;
   verts[1].z = z;
   verts[1].r = color[0];
   verts[1].g = color[1];
   verts[1].b = color[2];
   verts[1].a = color[3];
   verts[1].s = s1;
   verts[1].t = t0;

   /* upper-right */
   verts[2].x = x1;
   verts[2].y = y0;
   verts[2].z = z;
   verts[2].r = color[0];
   verts[2].g = color[1];
   verts[2].b = color[2];
   verts[2].a = color[3];
   verts[2].s = s1;
   verts[2].t = t1;

   /* upper-left */
   verts[3].x = x0;
   verts[3].y = y0;
   verts[3].z = z;
   verts[3].r = color[0];
   verts[3].g = color[1];
   verts[3].b = color[2];
   verts[3].a = color[3];
   verts[3].s = s0;
   verts[3].t = t1;

   u_upload_unmap(st->pipe->stream_uploader);

   cso_set_vertex_buffers(st->cso_context, 1, 0, false, &vb);
   st->last_num_vbuffers = MAX2(st->last_num_vbuffers, 1);

   if (num_instances > 1) {
      cso_draw_arrays_instanced(st->cso_context, MESA_PRIM_TRIANGLE_FAN, 0, 4,
                                0, num_instances);
   } else {
      cso_draw_arrays(st->cso_context, MESA_PRIM_TRIANGLE_FAN, 0, 4);
   }

   pipe_resource_reference(&vb.buffer.resource, NULL);

   return true;
}

static void
st_hw_select_draw_gallium(struct gl_context *ctx,
                          struct pipe_draw_info *info,
                          unsigned drawid_offset,
                          const struct pipe_draw_indirect_info *indirect,
                          const struct pipe_draw_start_count_bias *draws,
                          unsigned num_draws)
{
   struct st_context *st = st_context(ctx);
   enum mesa_prim old_mode = info->mode;

   if (st_draw_hw_select_prepare_common(ctx) &&
       st_draw_hw_select_prepare_mode(ctx, info)) {
      cso_draw_vbo(st->cso_context, info, drawid_offset, indirect, draws,
                   num_draws);
   }

   info->mode = old_mode;
}

static void
st_hw_select_draw_gallium_multimode(struct gl_context *ctx,
                                    struct pipe_draw_info *info,
                                    const struct pipe_draw_start_count_bias *draws,
                                    const unsigned char *mode,
                                    unsigned num_draws)
{
   struct st_context *st = st_context(ctx);

   if (!st_draw_hw_select_prepare_common(ctx))
      return;

   unsigned i, first;
   struct cso_context *cso = st->cso_context;

   /* Find consecutive draws where mode doesn't vary. */
   for (i = 0, first = 0; i <= num_draws; i++) {
      if (i == num_draws || mode[i] != mode[first]) {
         info->mode = mode[first];

         if (st_draw_hw_select_prepare_mode(ctx, info))
            cso_draw_vbo(cso, info, 0, NULL, &draws[first], i - first);

         first = i;

         /* We can pass the reference only once. st_buffer_object keeps
          * the reference alive for later draws.
          */
         info->take_index_buffer_ownership = false;
      }
   }
}

void
st_init_hw_select_draw_functions(struct pipe_screen *screen,
                                 struct dd_function_table *functions)
{
   functions->DrawGallium = st_hw_select_draw_gallium;
   functions->DrawGalliumMultiMode = st_hw_select_draw_gallium_multimode;
}
