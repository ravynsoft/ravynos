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
 *
 * Authors:
 *    Keith Whitwell <keithw@vmware.com>
 */

#include <stdbool.h>
#include <stdio.h>
#include "main/arrayobj.h"
#include "util/glheader.h"
#include "main/bufferobj.h"
#include "main/context.h"
#include "main/enums.h"
#include "main/state.h"
#include "main/varray.h"
#include "state_tracker/st_draw.h"

#include "vbo_private.h"

static void
vbo_exec_debug_verts(struct vbo_exec_context *exec)
{
   GLuint count = exec->vtx.vert_count;
   GLuint i;

   printf("%s: %u vertices %d primitives, %d vertsize\n",
          __func__,
          count,
          exec->vtx.prim_count,
          exec->vtx.vertex_size);

   for (i = 0 ; i < exec->vtx.prim_count ; i++) {
      printf("   prim %d: %s %d..%d %s %s\n",
             i,
             _mesa_lookup_prim_by_nr(exec->vtx.mode[i]),
             exec->vtx.draw[i].start,
             exec->vtx.draw[i].start + exec->vtx.draw[i].count,
             exec->vtx.markers[i].begin ? "BEGIN" : "(wrap)",
             exec->vtx.markers[i].end ? "END" : "(wrap)");
   }
}


static GLuint
vbo_exec_copy_vertices(struct vbo_exec_context *exec)
{
   struct gl_context *ctx = gl_context_from_vbo_exec(exec);
   const GLuint sz = exec->vtx.vertex_size;
   fi_type *dst = exec->vtx.copied.buffer;
   unsigned last = exec->vtx.prim_count - 1;
   unsigned start = exec->vtx.draw[last].start;
   const fi_type *src = exec->vtx.buffer_map + start * sz;

   return vbo_copy_vertices(ctx, ctx->Driver.CurrentExecPrimitive,
                            start,
                            &exec->vtx.draw[last].count,
                            exec->vtx.markers[last].begin,
                            sz, false, dst, src);
}



/* TODO: populate these as the vertex is defined:
 */
static void
vbo_exec_bind_arrays(struct gl_context *ctx,
                     struct gl_vertex_array_object **old_vao,
                     GLbitfield *old_vp_input_filter)
{
   struct vbo_context *vbo = vbo_context(ctx);
   struct gl_vertex_array_object *vao = vbo->VAO;
   struct vbo_exec_context *exec = &vbo->exec;

   GLintptr buffer_offset;
   if (exec->vtx.bufferobj) {
      assert(exec->vtx.bufferobj->Mappings[MAP_INTERNAL].Pointer);
      buffer_offset = exec->vtx.bufferobj->Mappings[MAP_INTERNAL].Offset +
                      exec->vtx.buffer_offset;
   } else {
      /* Ptr into ordinary app memory */
      buffer_offset = (GLbyte *)exec->vtx.buffer_map - (GLbyte *)NULL;
   }

   const gl_vertex_processing_mode mode = ctx->VertexProgram._VPMode;

   GLbitfield vao_enabled, vao_filter;
   if (_mesa_hw_select_enabled(ctx)) {
      /* HW GL_SELECT has fixed input */
      vao_enabled = vao_filter = VERT_BIT_POS | VERT_BIT_SELECT_RESULT_OFFSET;
   } else {
      vao_enabled = _vbo_get_vao_enabled_from_vbo(mode, exec->vtx.enabled);
      vao_filter = _vbo_get_vao_filter(mode);
   }

   /* At first disable arrays no longer needed */
   _mesa_disable_vertex_array_attribs(ctx, vao, ~vao_enabled);
   assert((~vao_enabled & vao->Enabled) == 0);

   /* Bind the buffer object */
   const GLuint stride = exec->vtx.vertex_size*sizeof(GLfloat);
   _mesa_bind_vertex_buffer(ctx, vao, 0, exec->vtx.bufferobj, buffer_offset,
                            stride, false, false);

   /* Retrieve the mapping from VBO_ATTRIB to VERT_ATTRIB space
    * Note that the position/generic0 aliasing is done in the VAO.
    */
   const GLubyte *const vao_to_vbo_map = _vbo_attribute_alias_map[mode];
   /* Now set the enabled arrays */
   GLbitfield mask = vao_enabled;
   while (mask) {
      const int vao_attr = u_bit_scan(&mask);
      const GLubyte vbo_attr = vao_to_vbo_map[vao_attr];

      const GLubyte size = exec->vtx.attr[vbo_attr].size;
      const GLenum16 type = exec->vtx.attr[vbo_attr].type;
      const GLuint offset = (GLuint)((GLbyte *)exec->vtx.attrptr[vbo_attr] -
                                     (GLbyte *)exec->vtx.vertex);
      assert(offset <= ctx->Const.MaxVertexAttribRelativeOffset);

      /* Set and enable */
      _vbo_set_attrib_format(ctx, vao, vao_attr, buffer_offset,
                             size, type, offset);

      /* The vao is initially created with all bindings set to 0. */
      assert(vao->VertexAttrib[vao_attr].BufferBindingIndex == 0);
   }
   _mesa_enable_vertex_array_attribs(ctx, vao, vao_enabled);
   assert(vao_enabled == vao->Enabled);
   assert(!exec->vtx.bufferobj ||
          (vao_enabled & ~vao->VertexAttribBufferMask) == 0);

   _mesa_save_and_set_draw_vao(ctx, vao, vao_filter,
                               old_vao, old_vp_input_filter);
   _mesa_set_varying_vp_inputs(ctx, vao_filter &
                               ctx->Array._DrawVAO->_EnabledWithMapMode);
}


/**
 * Unmap the VBO.  This is called before drawing.
 */
static void
vbo_exec_vtx_unmap(struct vbo_exec_context *exec)
{
   if (exec->vtx.bufferobj) {
      struct gl_context *ctx = gl_context_from_vbo_exec(exec);

      if (!ctx->Extensions.ARB_buffer_storage) {
         GLintptr offset = exec->vtx.buffer_used -
                           exec->vtx.bufferobj->Mappings[MAP_INTERNAL].Offset;
         GLsizeiptr length = (exec->vtx.buffer_ptr - exec->vtx.buffer_map) *
                             sizeof(float);

         if (length)
            _mesa_bufferobj_flush_mapped_range(ctx, offset, length,
                                               exec->vtx.bufferobj,
                                               MAP_INTERNAL);
      }

      exec->vtx.buffer_used += (exec->vtx.buffer_ptr -
                                exec->vtx.buffer_map) * sizeof(float);

      assert(exec->vtx.buffer_used <= ctx->Const.glBeginEndBufferSize);
      assert(exec->vtx.buffer_ptr != NULL);

      _mesa_bufferobj_unmap(ctx, exec->vtx.bufferobj, MAP_INTERNAL);
      exec->vtx.buffer_map = NULL;
      exec->vtx.buffer_ptr = NULL;
      exec->vtx.max_vert = 0;
   }
}

static bool
vbo_exec_buffer_has_space(struct vbo_exec_context *exec)
{
   struct gl_context *ctx = gl_context_from_vbo_exec(exec);

   return ctx->Const.glBeginEndBufferSize > exec->vtx.buffer_used + 1024;
}


/**
 * Map the vertex buffer to begin storing glVertex, glColor, etc data.
 */
void
vbo_exec_vtx_map(struct vbo_exec_context *exec)
{
   struct gl_context *ctx = gl_context_from_vbo_exec(exec);
   const GLenum usage = GL_STREAM_DRAW_ARB;
   GLenum accessRange = GL_MAP_WRITE_BIT |  /* for MapBufferRange */
                        GL_MAP_UNSYNCHRONIZED_BIT;

   if (ctx->Extensions.ARB_buffer_storage) {
      /* We sometimes read from the buffer, so map it for read too.
       * Only the persistent mapping can do that, because the non-persistent
       * mapping uses flags that are incompatible with GL_MAP_READ_BIT.
       */
      accessRange |= GL_MAP_PERSISTENT_BIT |
                     GL_MAP_COHERENT_BIT |
                     GL_MAP_READ_BIT;
   } else {
      accessRange |= GL_MAP_INVALIDATE_RANGE_BIT |
                     GL_MAP_FLUSH_EXPLICIT_BIT |
                     MESA_MAP_NOWAIT_BIT;
   }

   if (!exec->vtx.bufferobj)
      return;

   assert(!exec->vtx.buffer_map);
   assert(!exec->vtx.buffer_ptr);

   if (vbo_exec_buffer_has_space(exec)) {
      /* The VBO exists and there's room for more */
      if (exec->vtx.bufferobj->Size > 0) {
         exec->vtx.buffer_map = (fi_type *)
            _mesa_bufferobj_map_range(ctx,
                                      exec->vtx.buffer_used,
                                      ctx->Const.glBeginEndBufferSize
                                      - exec->vtx.buffer_used,
                                      accessRange,
                                      exec->vtx.bufferobj,
                                      MAP_INTERNAL);
         exec->vtx.buffer_ptr = exec->vtx.buffer_map;
      }
      else {
         exec->vtx.buffer_ptr = exec->vtx.buffer_map = NULL;
      }
   }

   if (!exec->vtx.buffer_map) {
      /* Need to allocate a new VBO */
      exec->vtx.buffer_used = 0;

      if (_mesa_bufferobj_data(ctx, GL_ARRAY_BUFFER_ARB,
                               ctx->Const.glBeginEndBufferSize,
                               NULL, usage,
                               GL_MAP_WRITE_BIT |
                               (ctx->Extensions.ARB_buffer_storage ?
                                GL_MAP_PERSISTENT_BIT |
                                GL_MAP_COHERENT_BIT |
                                GL_MAP_READ_BIT : 0) |
                               GL_DYNAMIC_STORAGE_BIT |
                               GL_CLIENT_STORAGE_BIT,
                               exec->vtx.bufferobj)) {
         /* buffer allocation worked, now map the buffer */
         exec->vtx.buffer_map =
            (fi_type *)_mesa_bufferobj_map_range(ctx,
                                                 0, ctx->Const.glBeginEndBufferSize,
                                                 accessRange,
                                                 exec->vtx.bufferobj,
                                                 MAP_INTERNAL);
      }
      else {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "VBO allocation");
         exec->vtx.buffer_map = NULL;
      }
   }

   exec->vtx.buffer_ptr = exec->vtx.buffer_map;
   exec->vtx.buffer_offset = 0;

   if (!exec->vtx.buffer_map) {
      /* out of memory */
      vbo_install_exec_vtxfmt_noop(ctx);
   }
   else {
      if (_mesa_using_noop_vtxfmt(ctx->Dispatch.Exec)) {
         /* The no-op functions are installed so switch back to regular
          * functions.  We do this test just to avoid frequent and needless
          * calls to vbo_install_exec_vtxfmt().
          */
         vbo_init_dispatch_begin_end(ctx);
      }
   }

   if (0)
      printf("map %d..\n", exec->vtx.buffer_used);
}



/**
 * Execute the buffer and save copied verts.
 */
void
vbo_exec_vtx_flush(struct vbo_exec_context *exec)
{
   struct gl_context *ctx = gl_context_from_vbo_exec(exec);

   /* Only unmap if persistent mappings are unsupported. */
   bool persistent_mapping = ctx->Extensions.ARB_buffer_storage &&
                             exec->vtx.bufferobj &&
                             exec->vtx.buffer_map;

   if (0)
      vbo_exec_debug_verts(exec);

   if (exec->vtx.prim_count &&
       exec->vtx.vert_count) {

      exec->vtx.copied.nr = vbo_exec_copy_vertices(exec);

      if (exec->vtx.copied.nr != exec->vtx.vert_count) {
         struct gl_vertex_array_object *old_vao;
         GLbitfield old_vp_input_filter;

         /* Prepare and set the Begin/End internal VAO for drawing. */
         vbo_exec_bind_arrays(ctx, &old_vao, &old_vp_input_filter);

         if (ctx->NewState)
            _mesa_update_state(ctx);

         if (!persistent_mapping)
            vbo_exec_vtx_unmap(exec);

         assert(ctx->NewState == 0);

         if (0)
            printf("%s %d %d\n", __func__, exec->vtx.prim_count,
                   exec->vtx.vert_count);

         st_prepare_draw(ctx, ST_PIPELINE_RENDER_STATE_MASK);

         ctx->Driver.DrawGalliumMultiMode(ctx, &exec->vtx.info,
                                          exec->vtx.draw,
                                          exec->vtx.mode,
                                          exec->vtx.prim_count);

         /* Get new storage -- unless asked not to. */
         if (!persistent_mapping)
            vbo_exec_vtx_map(exec);

         _mesa_restore_draw_vao(ctx, old_vao, old_vp_input_filter);
      }
   }

   if (persistent_mapping) {
      exec->vtx.buffer_used += (exec->vtx.buffer_ptr - exec->vtx.buffer_map) *
                               sizeof(float);
      exec->vtx.buffer_map = exec->vtx.buffer_ptr;

      /* Set the buffer offset for the next draw. */
      exec->vtx.buffer_offset = exec->vtx.buffer_used;

      if (!vbo_exec_buffer_has_space(exec)) {
         /* This will allocate a new buffer. */
         vbo_exec_vtx_unmap(exec);
         vbo_exec_vtx_map(exec);
      }
   }

   if (exec->vtx.vertex_size == 0)
      exec->vtx.max_vert = 0;
   else
      exec->vtx.max_vert = vbo_compute_max_verts(exec);

   exec->vtx.buffer_ptr = exec->vtx.buffer_map;
   exec->vtx.prim_count = 0;
   exec->vtx.vert_count = 0;
}
