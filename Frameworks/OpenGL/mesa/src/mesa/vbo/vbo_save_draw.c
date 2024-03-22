/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/* Author:
 *    Keith Whitwell <keithw@vmware.com>
 */

#include <stdbool.h>
#include "main/arrayobj.h"
#include "util/glheader.h"
#include "main/bufferobj.h"
#include "main/context.h"
#include "main/enable.h"
#include "main/mesa_private.h"
#include "main/macros.h"
#include "main/light.h"
#include "main/state.h"
#include "main/varray.h"
#include "util/bitscan.h"
#include "state_tracker/st_draw.h"

#include "vbo_private.h"

static void
copy_vao(struct gl_context *ctx, const struct gl_vertex_array_object *vao,
         GLbitfield mask, GLbitfield state, GLbitfield pop_state,
         int shift, fi_type **data, bool *color0_changed)
{
   struct vbo_context *vbo = vbo_context(ctx);

   mask &= vao->Enabled;
   while (mask) {
      const int i = u_bit_scan(&mask);
      const struct gl_array_attributes *attrib = &vao->VertexAttrib[i];
      unsigned current_index = shift + i;
      struct gl_array_attributes *currval = &vbo->current[current_index];
      const GLubyte size = attrib->Format.User.Size;
      const GLenum16 type = attrib->Format.User.Type;
      fi_type tmp[8];
      int dmul_shift = 0;

      if (type == GL_DOUBLE ||
          type == GL_UNSIGNED_INT64_ARB) {
         dmul_shift = 1;
         memcpy(tmp, *data, size * 2 * sizeof(GLfloat));
      } else {
         COPY_CLEAN_4V_TYPE_AS_UNION(tmp, size, *data, type);
      }

      if (memcmp(currval->Ptr, tmp, 4 * sizeof(GLfloat) << dmul_shift) != 0) {
         memcpy((fi_type*)currval->Ptr, tmp, 4 * sizeof(GLfloat) << dmul_shift);

         if (current_index == VBO_ATTRIB_COLOR0)
            *color0_changed = true;

         /* The fixed-func vertex program uses this. */
         if (current_index == VBO_ATTRIB_MAT_FRONT_SHININESS ||
             current_index == VBO_ATTRIB_MAT_BACK_SHININESS)
            ctx->NewState |= _NEW_FF_VERT_PROGRAM;

         if (current_index == VBO_ATTRIB_EDGEFLAG)
            _mesa_update_edgeflag_state_vao(ctx);

         ctx->NewState |= state;
         ctx->PopAttribState |= pop_state;
      }

      if (type != currval->Format.User.Type ||
          (size >> dmul_shift) != currval->Format.User.Size) {
         vbo_set_vertex_format(&currval->Format, size >> dmul_shift, type);
         /* The format changed. We need to update gallium vertex elements. */
         if (state == _NEW_CURRENT_ATTRIB)
            ctx->NewState |= state;
      }

      *data += size;
   }
}

/**
 * After playback, copy everything but the position from the
 * last vertex to the saved state
 */
static void
playback_copy_to_current(struct gl_context *ctx,
                         const struct vbo_save_vertex_list *node)
{
   if (!node->cold->current_data)
      return;

   fi_type *data = node->cold->current_data;
   bool color0_changed = false;

   /* Copy conventional attribs and generics except pos */
   copy_vao(ctx, node->cold->VAO[VP_MODE_SHADER], ~VERT_BIT_POS,
            _NEW_CURRENT_ATTRIB, GL_CURRENT_BIT, 0, &data, &color0_changed);
   /* Copy materials */
   copy_vao(ctx, node->cold->VAO[VP_MODE_FF], VERT_BIT_MAT_ALL,
            _NEW_MATERIAL, GL_LIGHTING_BIT,
            VBO_MATERIAL_SHIFT, &data, &color0_changed);

   if (color0_changed && ctx->Light.ColorMaterialEnabled) {
      _mesa_update_color_material(ctx, ctx->Current.Attrib[VBO_ATTRIB_COLOR0]);
   }

   /* CurrentExecPrimitive
    */
   if (node->cold->prim_count) {
      const struct _mesa_prim *prim = &node->cold->prims[node->cold->prim_count - 1];
      if (prim->end)
         ctx->Driver.CurrentExecPrimitive = PRIM_OUTSIDE_BEGIN_END;
      else
         ctx->Driver.CurrentExecPrimitive = prim->mode;
   }
}


static void
loopback_vertex_list(struct gl_context *ctx,
                     const struct vbo_save_vertex_list *list)
{
   struct gl_buffer_object *bo = list->cold->VAO[0]->BufferBinding[0].BufferObj;
   void *buffer = NULL;

   /* Reuse BO mapping when possible to avoid costly mapping on every glCallList(). */
   if (_mesa_bufferobj_mapped(bo, MAP_INTERNAL)) {
      if (list->cold->bo_bytes_used <= bo->Mappings[MAP_INTERNAL].Length)
         buffer = bo->Mappings[MAP_INTERNAL].Pointer;
      else
         _mesa_bufferobj_unmap(ctx, bo, MAP_INTERNAL);
   }

   if (!buffer && list->cold->bo_bytes_used)
      buffer = _mesa_bufferobj_map_range(ctx, 0, list->cold->bo_bytes_used, GL_MAP_READ_BIT,
                                         bo, MAP_INTERNAL);

   /* TODO: in this case, we shouldn't create a bo at all and instead keep
    * the in-RAM buffer. */
   _vbo_loopback_vertex_list(ctx, list, buffer);

   if (!ctx->Const.AllowMappedBuffersDuringExecution && buffer)
      _mesa_bufferobj_unmap(ctx, bo, MAP_INTERNAL);
}


void
vbo_save_playback_vertex_list_loopback(struct gl_context *ctx, void *data)
{
   const struct vbo_save_vertex_list *node =
      (const struct vbo_save_vertex_list *) data;

   FLUSH_FOR_DRAW(ctx);

   if (_mesa_inside_begin_end(ctx) && node->draw_begins) {
      /* Error: we're about to begin a new primitive but we're already
       * inside a glBegin/End pair.
       */
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "draw operation inside glBegin/End");
      return;
   }
   /* Various degenerate cases: translate into immediate mode
    * calls rather than trying to execute in place.
    */
   loopback_vertex_list(ctx, node);
}

enum vbo_save_status {
   DONE,
   USE_SLOW_PATH,
};

static enum vbo_save_status
vbo_save_playback_vertex_list_gallium(struct gl_context *ctx,
                                      const struct vbo_save_vertex_list *node,
                                      bool copy_to_current)
{
   /* Don't use this if selection or feedback mode is enabled. st/mesa can't
    * handle it.
    */
   if (!ctx->Driver.DrawGalliumVertexState || ctx->RenderMode != GL_RENDER)
      return USE_SLOW_PATH;

   const gl_vertex_processing_mode mode = ctx->VertexProgram._VPMode;

   /* This sets which vertex arrays are enabled, which determines
    * which attribs have stride = 0 and whether edge flags are enabled.
    */
   const GLbitfield enabled = node->enabled_attribs[mode];
   _mesa_set_varying_vp_inputs(ctx, enabled);

   if (ctx->NewState)
      _mesa_update_state(ctx);

   /* Return precomputed GL errors such as invalid shaders. */
   if (!ctx->ValidPrimMask) {
      _mesa_error(ctx, ctx->DrawGLError, "glCallList");
      return DONE;
   }

   /* Use the slow path when there are vertex inputs without vertex
    * elements. This happens with zero-stride attribs and non-fixed-func
    * shaders.
    *
    * Dual-slot inputs are also unsupported because the higher slot is
    * always missing in vertex elements.
    *
    * TODO: Add support for zero-stride attribs.
    */
   struct gl_program *vp = ctx->VertexProgram._Current;

   if (vp->info.inputs_read & ~enabled || vp->DualSlotInputs)
      return USE_SLOW_PATH;

   struct pipe_vertex_state *state = node->state[mode];
   struct pipe_draw_vertex_state_info info;

   info.mode = node->mode;
   info.take_vertex_state_ownership = false;

   if (node->ctx == ctx) {
      /* This mechanism allows passing references to the driver without
       * using atomics to increase the reference count.
       *
       * This private refcount can be decremented without atomics but only
       * one context (ctx above) can use this counter (so that it's only
       * used by 1 thread).
       *
       * This number is atomically added to reference.count at
       * initialization. If it's never used, the same number is atomically
       * subtracted from reference.count before destruction. If this number
       * is decremented, we can pass one reference to the driver without
       * touching reference.count with atomics. At destruction we only
       * subtract the number of references we have not returned. This can
       * possibly turn a million atomic increments into 1 add and 1 subtract
       * atomic op over the whole lifetime of an app.
       */
      int16_t * const private_refcount = (int16_t*)&node->private_refcount[mode];
      assert(*private_refcount >= 0);

      if (unlikely(*private_refcount == 0)) {
         /* pipe_vertex_state can be reused through util_vertex_state_cache,
          * and there can be many display lists over-incrementing this number,
          * causing it to overflow.
          *
          * Guess that the same state can never be used by N=500000 display
          * lists, so one display list can only increment it by
          * INT_MAX / N.
          */
         const int16_t add_refs = INT_MAX / 500000;
         p_atomic_add(&state->reference.count, add_refs);
         *private_refcount = add_refs;
      }

      (*private_refcount)--;
      info.take_vertex_state_ownership = true;
   }

   /* Set edge flags. */
   _mesa_update_edgeflag_state_explicit(ctx, enabled & VERT_BIT_EDGEFLAG);

   /* Fast path using a pre-built gallium vertex buffer state. */
   if (node->modes || node->num_draws > 1) {
      ctx->Driver.DrawGalliumVertexState(ctx, state, info,
                                         node->start_counts,
                                         node->modes,
                                         node->num_draws);
   } else if (node->num_draws) {
      ctx->Driver.DrawGalliumVertexState(ctx, state, info,
                                         &node->start_count,
                                         NULL, 1);
   }

   /* Restore edge flag state and ctx->VertexProgram._VaryingInputs. */
   _mesa_update_edgeflag_state_vao(ctx);

   if (copy_to_current)
      playback_copy_to_current(ctx, node);
   return DONE;
}

/**
 * Execute the buffer and save copied verts.
 * This is called from the display list code when executing
 * a drawing command.
 */
void
vbo_save_playback_vertex_list(struct gl_context *ctx, void *data, bool copy_to_current)
{
   const struct vbo_save_vertex_list *node =
      (const struct vbo_save_vertex_list *) data;

   FLUSH_FOR_DRAW(ctx);

   if (_mesa_inside_begin_end(ctx) && node->draw_begins) {
      /* Error: we're about to begin a new primitive but we're already
       * inside a glBegin/End pair.
       */
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "draw operation inside glBegin/End");
      return;
   }

   if (vbo_save_playback_vertex_list_gallium(ctx, node, copy_to_current) == DONE)
      return;

   /* Save the Draw VAO before we override it. */
   const gl_vertex_processing_mode mode = ctx->VertexProgram._VPMode;
   GLbitfield vao_filter = _vbo_get_vao_filter(mode);
   struct gl_vertex_array_object *old_vao;
   GLbitfield old_vp_input_filter;

   _mesa_save_and_set_draw_vao(ctx, node->cold->VAO[mode], vao_filter,
                               &old_vao, &old_vp_input_filter);
   _mesa_set_varying_vp_inputs(ctx, vao_filter &
                               ctx->Array._DrawVAO->_EnabledWithMapMode);

   /* Need that at least one time. */
   if (ctx->NewState)
      _mesa_update_state(ctx);

   /* Return precomputed GL errors such as invalid shaders. */
   if (!ctx->ValidPrimMask) {
      _mesa_restore_draw_vao(ctx, old_vao, old_vp_input_filter);
      _mesa_error(ctx, ctx->DrawGLError, "glCallList");
      return;
   }

   assert(ctx->NewState == 0);

   struct pipe_draw_info *info = (struct pipe_draw_info *) &node->cold->info;

   st_prepare_draw(ctx, ST_PIPELINE_RENDER_STATE_MASK);

   if (node->modes) {
      ctx->Driver.DrawGalliumMultiMode(ctx, info,
                                       node->start_counts,
                                       node->modes,
                                       node->num_draws);
   } else if (node->num_draws == 1) {
      ctx->Driver.DrawGallium(ctx, info, 0, NULL, &node->start_count, 1);
   } else if (node->num_draws) {
      ctx->Driver.DrawGallium(ctx, info, 0, NULL, node->start_counts,
                              node->num_draws);
   }

   _mesa_restore_draw_vao(ctx, old_vao, old_vp_input_filter);

   if (copy_to_current)
      playback_copy_to_current(ctx, node);
}
