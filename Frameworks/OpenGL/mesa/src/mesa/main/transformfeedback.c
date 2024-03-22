/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2010  VMware, Inc.  All Rights Reserved.
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


/*
 * Transform feedback support.
 *
 * Authors:
 *   Brian Paul
 */


#include "buffers.h"
#include "context.h"
#include "draw_validate.h"
#include "hash.h"
#include "macros.h"
#include "mtypes.h"
#include "transformfeedback.h"
#include "shaderapi.h"
#include "shaderobj.h"

#include "program/program.h"
#include "program/prog_parameter.h"

#include "util/u_memory.h"
#include "util/u_inlines.h"

#include "api_exec_decl.h"

#include "cso_cache/cso_context.h"
struct using_program_tuple
{
   struct gl_program *prog;
   bool found;
};

static void
active_xfb_object_references_program(void *data, void *user_data)
{
   struct using_program_tuple *callback_data = user_data;
   struct gl_transform_feedback_object *obj = data;
   if (obj->Active && obj->program == callback_data->prog)
      callback_data->found = true;
}

/**
 * Return true if any active transform feedback object is using a program.
 */
bool
_mesa_transform_feedback_is_using_program(struct gl_context *ctx,
                                          struct gl_shader_program *shProg)
{
   if (!shProg->last_vert_prog)
      return false;

   struct using_program_tuple callback_data;
   callback_data.found = false;
   callback_data.prog = shProg->last_vert_prog;

   _mesa_HashWalkLocked(ctx->TransformFeedback.Objects,
                        active_xfb_object_references_program, &callback_data);

   /* Also check DefaultObject, as it's not in the Objects hash table. */
   active_xfb_object_references_program(ctx->TransformFeedback.DefaultObject,
                                        &callback_data);

   return callback_data.found;
}

static struct gl_transform_feedback_object *
new_transform_feedback(struct gl_context *ctx, GLuint name)
{
   struct gl_transform_feedback_object *obj;

   obj = CALLOC_STRUCT(gl_transform_feedback_object);
   if (!obj)
      return NULL;

   obj->Name = name;
   obj->RefCount = 1;
   obj->EverBound = GL_FALSE;

   return obj;
}

static void
delete_transform_feedback(struct gl_context *ctx,
                             struct gl_transform_feedback_object *obj)
{
   unsigned i;

   for (i = 0; i < ARRAY_SIZE(obj->draw_count); i++)
      pipe_so_target_reference(&obj->draw_count[i], NULL);

   /* Unreference targets. */
   for (i = 0; i < obj->num_targets; i++) {
      pipe_so_target_reference(&obj->targets[i], NULL);
   }

   for (unsigned i = 0; i < ARRAY_SIZE(obj->Buffers); i++) {
      _mesa_reference_buffer_object(ctx, &obj->Buffers[i], NULL);
   }

   free(obj->Label);
   FREE(obj);
}

/**
 * Do reference counting of transform feedback buffers.
 */
static void
reference_transform_feedback_object(struct gl_transform_feedback_object **ptr,
                                    struct gl_transform_feedback_object *obj)
{
   if (*ptr == obj)
      return;

   if (*ptr) {
      /* Unreference the old object */
      struct gl_transform_feedback_object *oldObj = *ptr;

      assert(oldObj->RefCount > 0);
      oldObj->RefCount--;

      if (oldObj->RefCount == 0) {
         GET_CURRENT_CONTEXT(ctx);
         if (ctx)
            delete_transform_feedback(ctx, oldObj);
      }

      *ptr = NULL;
   }
   assert(!*ptr);

   if (obj) {
      assert(obj->RefCount > 0);

      /* reference new object */
      obj->RefCount++;
      obj->EverBound = GL_TRUE;
      *ptr = obj;
   }
}


/**
 * Per-context init for transform feedback.
 */
void
_mesa_init_transform_feedback(struct gl_context *ctx)
{
   /* core mesa expects this, even a dummy one, to be available */
   ctx->TransformFeedback.DefaultObject =
      new_transform_feedback(ctx, 0);

   assert(ctx->TransformFeedback.DefaultObject->RefCount == 1);

   reference_transform_feedback_object(&ctx->TransformFeedback.CurrentObject,
                                       ctx->TransformFeedback.DefaultObject);

   assert(ctx->TransformFeedback.DefaultObject->RefCount == 2);

   ctx->TransformFeedback.Objects = _mesa_NewHashTable();

   _mesa_reference_buffer_object(ctx,
                                 &ctx->TransformFeedback.CurrentBuffer, NULL);
}



/**
 * Callback for _mesa_HashDeleteAll().
 */
static void
delete_cb(void *data, void *userData)
{
   struct gl_context *ctx = (struct gl_context *) userData;
   struct gl_transform_feedback_object *obj =
      (struct gl_transform_feedback_object *) data;

   delete_transform_feedback(ctx, obj);
}


/**
 * Per-context free/clean-up for transform feedback.
 */
void
_mesa_free_transform_feedback(struct gl_context *ctx)
{
   /* core mesa expects this, even a dummy one, to be available */
   _mesa_reference_buffer_object(ctx,
                                 &ctx->TransformFeedback.CurrentBuffer,
                                 NULL);

   /* Delete all feedback objects */
   _mesa_HashDeleteAll(ctx->TransformFeedback.Objects, delete_cb, ctx);
   _mesa_DeleteHashTable(ctx->TransformFeedback.Objects);

   /* Delete the default feedback object */
   delete_transform_feedback(ctx,
                             ctx->TransformFeedback.DefaultObject);

   ctx->TransformFeedback.CurrentObject = NULL;
}

/**
 * Fill in the correct Size value for each buffer in \c obj.
 *
 * From the GL 4.3 spec, section 6.1.1 ("Binding Buffer Objects to Indexed
 * Targets"):
 *
 *   BindBufferBase binds the entire buffer, even when the size of the buffer
 *   is changed after the binding is established. It is equivalent to calling
 *   BindBufferRange with offset zero, while size is determined by the size of
 *   the bound buffer at the time the binding is used.
 *
 *   Regardless of the size specified with BindBufferRange, or indirectly with
 *   BindBufferBase, the GL will never read or write beyond the end of a bound
 *   buffer. In some cases this constraint may result in visibly different
 *   behavior when a buffer overflow would otherwise result, such as described
 *   for transform feedback operations in section 13.2.2.
 */
static void
compute_transform_feedback_buffer_sizes(
      struct gl_transform_feedback_object *obj)
{
   unsigned i = 0;
   for (i = 0; i < MAX_FEEDBACK_BUFFERS; ++i) {
      GLintptr offset = obj->Offset[i];
      GLsizeiptr buffer_size
         = obj->Buffers[i] == NULL ? 0 : obj->Buffers[i]->Size;
      GLsizeiptr available_space
         = buffer_size <= offset ? 0 : buffer_size - offset;
      GLsizeiptr computed_size;
      if (obj->RequestedSize[i] == 0) {
         /* No size was specified at the time the buffer was bound, so allow
          * writing to all available space in the buffer.
          */
         computed_size = available_space;
      } else {
         /* A size was specified at the time the buffer was bound, however
          * it's possible that the buffer has shrunk since then.  So only
          * allow writing to the minimum of the specified size and the space
          * available.
          */
         computed_size = MIN2(available_space, obj->RequestedSize[i]);
      }

      /* Legal sizes must be multiples of four, so round down if necessary. */
      obj->Size[i] = computed_size & ~0x3;
   }
}


/**
 * Compute the maximum number of vertices that can be written to the currently
 * enabled transform feedback buffers without overflowing any of them.
 */
unsigned
_mesa_compute_max_transform_feedback_vertices(struct gl_context *ctx,
      const struct gl_transform_feedback_object *obj,
      const struct gl_transform_feedback_info *info)
{
   unsigned max_index = 0xffffffff;
   unsigned i;

   for (i = 0; i < ctx->Const.MaxTransformFeedbackBuffers; i++) {
      if ((info->ActiveBuffers >> i) & 1) {
         unsigned stride = info->Buffers[i].Stride;
         unsigned max_for_this_buffer;

         /* Skip any inactive buffers, which have a stride of 0. */
         if (stride == 0)
            continue;

         max_for_this_buffer = obj->Size[i] / (4 * stride);
         max_index = MIN2(max_index, max_for_this_buffer);
      }
   }

   return max_index;
}


/**
 ** Begin API functions
 **/


/**
 * Figure out which stage of the pipeline is the source of transform feedback
 * data given the current context state, and return its gl_program.
 *
 * If no active program can generate transform feedback data (i.e. no vertex
 * shader is active), returns NULL.
 */
static struct gl_program *
get_xfb_source(struct gl_context *ctx)
{
   int i;
   for (i = MESA_SHADER_GEOMETRY; i >= MESA_SHADER_VERTEX; i--) {
      if (ctx->_Shader->CurrentProgram[i] != NULL)
         return ctx->_Shader->CurrentProgram[i];
   }
   return NULL;
}


static ALWAYS_INLINE void
begin_transform_feedback(struct gl_context *ctx, GLenum mode, bool no_error)
{
   struct gl_transform_feedback_object *obj;
   struct gl_transform_feedback_info *info = NULL;
   struct gl_program *source;
   GLuint i;
   unsigned vertices_per_prim;

   obj = ctx->TransformFeedback.CurrentObject;

   /* Figure out what pipeline stage is the source of data for transform
    * feedback.
    */
   source = get_xfb_source(ctx);
   if (!no_error && source == NULL) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glBeginTransformFeedback(no program active)");
      return;
   }

   info = source->sh.LinkedTransformFeedback;

   if (!no_error && info->NumOutputs == 0) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glBeginTransformFeedback(no varyings to record)");
      return;
   }

   switch (mode) {
   case GL_POINTS:
      vertices_per_prim = 1;
      break;
   case GL_LINES:
      vertices_per_prim = 2;
      break;
   case GL_TRIANGLES:
      vertices_per_prim = 3;
      break;
   default:
      if (!no_error) {
         _mesa_error(ctx, GL_INVALID_ENUM, "glBeginTransformFeedback(mode)");
         return;
      } else {
         /* Stop compiler warnings */
         unreachable("Error in API use when using KHR_no_error");
      }
   }

   if (!no_error) {
      if (obj->Active) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glBeginTransformFeedback(already active)");
         return;
      }

      for (i = 0; i < ctx->Const.MaxTransformFeedbackBuffers; i++) {
         if ((info->ActiveBuffers >> i) & 1) {
            if (obj->BufferNames[i] == 0) {
               _mesa_error(ctx, GL_INVALID_OPERATION,
                           "glBeginTransformFeedback(binding point %d does not "
                           "have a buffer object bound)", i);
               return;
            }
         }
      }
   }

   FLUSH_VERTICES(ctx, 0, 0);

   obj->Active = GL_TRUE;
   ctx->TransformFeedback.Mode = mode;

   compute_transform_feedback_buffer_sizes(obj);

   if (_mesa_is_gles3(ctx)) {
      /* In GLES3, we are required to track the usage of the transform
       * feedback buffer and report INVALID_OPERATION if a draw call tries to
       * exceed it.  So compute the maximum number of vertices that we can
       * write without overflowing any of the buffers currently being used for
       * feedback.
       */
      unsigned max_vertices
         = _mesa_compute_max_transform_feedback_vertices(ctx, obj, info);
      obj->GlesRemainingPrims = max_vertices / vertices_per_prim;
   }

   if (obj->program != source) {
      _mesa_reference_program_(ctx, &obj->program, source);
      obj->program = source;
   }

   struct pipe_context *pipe = ctx->pipe;
   unsigned max_num_targets;
   unsigned offsets[PIPE_MAX_SO_BUFFERS] = {0};

   max_num_targets = MIN2(ARRAY_SIZE(obj->Buffers),
                          ARRAY_SIZE(obj->targets));

   /* Convert the transform feedback state into the gallium representation. */
   for (i = 0; i < max_num_targets; i++) {
      struct gl_buffer_object *bo = obj->Buffers[i];

      if (bo && bo->buffer) {
         unsigned stream = obj->program->sh.LinkedTransformFeedback->
            Buffers[i].Stream;

         /* Check whether we need to recreate the target. */
         if (!obj->targets[i] ||
             obj->targets[i] == obj->draw_count[stream] ||
             obj->targets[i]->buffer != bo->buffer ||
             obj->targets[i]->buffer_offset != obj->Offset[i] ||
             obj->targets[i]->buffer_size != obj->Size[i]) {
            /* Create a new target. */
            struct pipe_stream_output_target *so_target =
                  pipe->create_stream_output_target(pipe, bo->buffer,
                                                    obj->Offset[i],
                                                    obj->Size[i]);

            pipe_so_target_reference(&obj->targets[i], NULL);
            obj->targets[i] = so_target;
         }

         obj->num_targets = i+1;
      } else {
         pipe_so_target_reference(&obj->targets[i], NULL);
      }
   }

   /* Start writing at the beginning of each target. */
   cso_set_stream_outputs(ctx->cso_context, obj->num_targets,
                          obj->targets, offsets);
   _mesa_update_valid_to_render_state(ctx);
}


void GLAPIENTRY
_mesa_BeginTransformFeedback_no_error(GLenum mode)
{
   GET_CURRENT_CONTEXT(ctx);
   begin_transform_feedback(ctx, mode, true);
}


void GLAPIENTRY
_mesa_BeginTransformFeedback(GLenum mode)
{
   GET_CURRENT_CONTEXT(ctx);
   begin_transform_feedback(ctx, mode, false);
}


static void
end_transform_feedback(struct gl_context *ctx,
                       struct gl_transform_feedback_object *obj)
{
   unsigned i;
   FLUSH_VERTICES(ctx, 0, 0);

   cso_set_stream_outputs(ctx->cso_context, 0, NULL, NULL);

   /* The next call to glDrawTransformFeedbackStream should use the vertex
    * count from the last call to glEndTransformFeedback.
    * Therefore, save the targets for each stream.
    *
    * NULL means the vertex counter is 0 (initial state).
    */
   for (i = 0; i < ARRAY_SIZE(obj->draw_count); i++)
      pipe_so_target_reference(&obj->draw_count[i], NULL);

   for (i = 0; i < ARRAY_SIZE(obj->targets); i++) {
      unsigned stream = obj->program->sh.LinkedTransformFeedback->
         Buffers[i].Stream;

      /* Is it not bound or already set for this stream? */
      if (!obj->targets[i] || obj->draw_count[stream])
         continue;

      pipe_so_target_reference(&obj->draw_count[stream], obj->targets[i]);
   }

   _mesa_reference_program_(ctx, &obj->program, NULL);
   ctx->TransformFeedback.CurrentObject->Active = GL_FALSE;
   ctx->TransformFeedback.CurrentObject->Paused = GL_FALSE;
   ctx->TransformFeedback.CurrentObject->EndedAnytime = GL_TRUE;
   _mesa_update_valid_to_render_state(ctx);
}


void GLAPIENTRY
_mesa_EndTransformFeedback_no_error(void)
{
   GET_CURRENT_CONTEXT(ctx);
   end_transform_feedback(ctx, ctx->TransformFeedback.CurrentObject);
}


void GLAPIENTRY
_mesa_EndTransformFeedback(void)
{
   struct gl_transform_feedback_object *obj;
   GET_CURRENT_CONTEXT(ctx);

   obj = ctx->TransformFeedback.CurrentObject;

   if (!obj->Active) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glEndTransformFeedback(not active)");
      return;
   }

   end_transform_feedback(ctx, obj);
}


/**
 * Helper used by BindBufferRange() and BindBufferBase().
 */
static void
bind_buffer_range(struct gl_context *ctx,
                  struct gl_transform_feedback_object *obj,
                  GLuint index,
                  struct gl_buffer_object *bufObj,
                  GLintptr offset, GLsizeiptr size,
                  bool dsa)
{
   /* Note: no need to FLUSH_VERTICES because
    * transform feedback buffers can't be changed while transform feedback is
    * active.
    */

   if (!dsa) {
      /* The general binding point */
      _mesa_reference_buffer_object(ctx,
                                    &ctx->TransformFeedback.CurrentBuffer,
                                    bufObj);
   }

   /* The per-attribute binding point */
   _mesa_set_transform_feedback_binding(ctx, obj, index, bufObj, offset, size);
}


/**
 * Validate the buffer object to receive transform feedback results.  Plus,
 * validate the starting offset to place the results, and max size.
 * Called from the glBindBufferRange() and glTransformFeedbackBufferRange
 * functions.
 */
bool
_mesa_validate_buffer_range_xfb(struct gl_context *ctx,
                                struct gl_transform_feedback_object *obj,
                                GLuint index, struct gl_buffer_object *bufObj,
                                GLintptr offset, GLsizeiptr size, bool dsa)
{
   const char *gl_methd_name;
   if (dsa)
      gl_methd_name = "glTransformFeedbackBufferRange";
   else
      gl_methd_name = "glBindBufferRange";


   if (obj->Active) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(transform feedback active)",
                  gl_methd_name);
      return false;
   }

   if (index >= ctx->Const.MaxTransformFeedbackBuffers) {
      /* OpenGL 4.5 core profile, 6.1, pdf page 82: "An INVALID_VALUE error is
       * generated if index is greater than or equal to the number of binding
       * points for transform feedback, as described in section 6.7.1."
       */
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(index=%d out of bounds)",
                  gl_methd_name, index);
      return false;
   }

   if (size & 0x3) {
      /* OpenGL 4.5 core profile, 6.7, pdf page 103: multiple of 4 */
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(size=%d must be a multiple of "
                  "four)", gl_methd_name, (int) size);
      return false;
   }

   if (offset & 0x3) {
      /* OpenGL 4.5 core profile, 6.7, pdf page 103: multiple of 4 */
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(offset=%d must be a multiple "
                  "of four)", gl_methd_name, (int) offset);
      return false;
   }

   if (offset < 0) {
      /* OpenGL 4.5 core profile, 6.1, pdf page 82: "An INVALID_VALUE error is
       * generated by BindBufferRange if offset is negative."
       *
       * OpenGL 4.5 core profile, 13.2, pdf page 445: "An INVALID_VALUE error
       * is generated by TransformFeedbackBufferRange if offset is negative."
       */
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(offset=%d must be >= 0)",
                  gl_methd_name,
                  (int) offset);
      return false;
   }

   if (size <= 0 && (dsa || bufObj)) {
      /* OpenGL 4.5 core profile, 6.1, pdf page 82: "An INVALID_VALUE error is
       * generated by BindBufferRange if buffer is non-zero and size is less
       * than or equal to zero."
       *
       * OpenGL 4.5 core profile, 13.2, pdf page 445: "An INVALID_VALUE error
       * is generated by TransformFeedbackBufferRange if size is less than or
       * equal to zero."
       */
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(size=%d must be > 0)",
                  gl_methd_name, (int) size);
      return false;
   }

   return true;
}


/**
 * Specify a buffer object to receive transform feedback results.
 * As above, but start at offset = 0.
 * Called from the glBindBufferBase() and glTransformFeedbackBufferBase()
 * functions.
 */
void
_mesa_bind_buffer_base_transform_feedback(struct gl_context *ctx,
                                          struct gl_transform_feedback_object *obj,
                                          GLuint index,
                                          struct gl_buffer_object *bufObj,
                                          bool dsa)
{
   if (obj->Active) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(transform feedback active)",
                  dsa ? "glTransformFeedbackBufferBase" : "glBindBufferBase");
      return;
   }

   if (index >= ctx->Const.MaxTransformFeedbackBuffers) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(index=%d out of bounds)",
                  dsa ? "glTransformFeedbackBufferBase" : "glBindBufferBase",
                  index);
      return;
   }

   bind_buffer_range(ctx, obj, index, bufObj, 0, 0, dsa);
}

/**
 * Wrapper around lookup_transform_feedback_object that throws
 * GL_INVALID_OPERATION if id is not in the hash table. After calling
 * _mesa_error, it returns NULL.
 */
static struct gl_transform_feedback_object *
lookup_transform_feedback_object_err(struct gl_context *ctx,
                                     GLuint xfb, const char* func)
{
   struct gl_transform_feedback_object *obj;

   obj = _mesa_lookup_transform_feedback_object(ctx, xfb);
   if (!obj) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(xfb=%u: non-generated object name)", func, xfb);
   }

   return obj;
}

/**
 * Wrapper around _mesa_lookup_bufferobj that throws GL_INVALID_VALUE if id
 * is not in the hash table. Specialised version for the
 * transform-feedback-related functions. After calling _mesa_error, it
 * returns NULL.
 */
static struct gl_buffer_object *
lookup_transform_feedback_bufferobj_err(struct gl_context *ctx,
                                        GLuint buffer, const char* func,
                                        bool *error)
{
   struct gl_buffer_object *bufObj = NULL;

   *error = false;

   /* OpenGL 4.5 core profile, 13.2, pdf page 444: buffer must be zero or the
    * name of an existing buffer object.
    */
   if (buffer) {
      bufObj = _mesa_lookup_bufferobj(ctx, buffer);
      if (!bufObj) {
         _mesa_error(ctx, GL_INVALID_VALUE, "%s(invalid buffer=%u)", func,
                     buffer);
         *error = true;
      }
   }

   return bufObj;
}

void GLAPIENTRY
_mesa_TransformFeedbackBufferBase(GLuint xfb, GLuint index, GLuint buffer)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_transform_feedback_object *obj;
   struct gl_buffer_object *bufObj;

   obj = lookup_transform_feedback_object_err(ctx, xfb,
                                              "glTransformFeedbackBufferBase");
   if (!obj) {
      return;
   }

   bool error;
   bufObj = lookup_transform_feedback_bufferobj_err(ctx, buffer,
                                              "glTransformFeedbackBufferBase",
                                                    &error);
   if (error) {
      return;
   }

   _mesa_bind_buffer_base_transform_feedback(ctx, obj, index, bufObj, true);
}

void GLAPIENTRY
_mesa_TransformFeedbackBufferRange(GLuint xfb, GLuint index, GLuint buffer,
                                   GLintptr offset, GLsizeiptr size)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_transform_feedback_object *obj;
   struct gl_buffer_object *bufObj;

   obj = lookup_transform_feedback_object_err(ctx, xfb,
                                              "glTransformFeedbackBufferRange");
   if (!obj) {
      return;
   }

   bool error;
   bufObj = lookup_transform_feedback_bufferobj_err(ctx, buffer,
                                              "glTransformFeedbackBufferRange",
                                                    &error);
   if (error) {
      return;
   }

   if (!_mesa_validate_buffer_range_xfb(ctx, obj, index, bufObj, offset,
                                        size, true))
      return;

   /* The per-attribute binding point */
   _mesa_set_transform_feedback_binding(ctx, obj, index, bufObj, offset,
                                        size);
}

/**
 * Specify a buffer object to receive transform feedback results, plus the
 * offset in the buffer to start placing results.
 * This function is part of GL_EXT_transform_feedback, but not GL3.
 */
static ALWAYS_INLINE void
bind_buffer_offset(struct gl_context *ctx,
                   struct gl_transform_feedback_object *obj, GLuint index,
                   GLuint buffer, GLintptr offset, bool no_error)
{
   struct gl_buffer_object *bufObj;

   if (buffer == 0) {
      bufObj = NULL;
   } else {
      bufObj = _mesa_lookup_bufferobj(ctx, buffer);
      if (!no_error && !bufObj) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glBindBufferOffsetEXT(invalid buffer=%u)", buffer);
         return;
      }
   }

   _mesa_bind_buffer_range_xfb(ctx, obj, index, bufObj, offset, 0);
}


void GLAPIENTRY
_mesa_BindBufferOffsetEXT_no_error(GLenum target, GLuint index, GLuint buffer,
                                   GLintptr offset)
{
   GET_CURRENT_CONTEXT(ctx);
   bind_buffer_offset(ctx, ctx->TransformFeedback.CurrentObject, index, buffer,
                      offset, true);
}


void GLAPIENTRY
_mesa_BindBufferOffsetEXT(GLenum target, GLuint index, GLuint buffer,
                          GLintptr offset)
{
   struct gl_transform_feedback_object *obj;
   GET_CURRENT_CONTEXT(ctx);

   if (target != GL_TRANSFORM_FEEDBACK_BUFFER) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glBindBufferOffsetEXT(target)");
      return;
   }

   obj = ctx->TransformFeedback.CurrentObject;

   if (obj->Active) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glBindBufferOffsetEXT(transform feedback active)");
      return;
   }

   if (index >= ctx->Const.MaxTransformFeedbackBuffers) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glBindBufferOffsetEXT(index=%d)", index);
      return;
   }

   if (offset & 0x3) {
      /* must be multiple of four */
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glBindBufferOffsetEXT(offset=%d)", (int) offset);
      return;
   }

   bind_buffer_offset(ctx, obj, index, buffer, offset, false);
}


/**
 * This function specifies the transform feedback outputs to be written
 * to the feedback buffer(s), and in what order.
 */
static ALWAYS_INLINE void
transform_feedback_varyings(struct gl_context *ctx,
                            struct gl_shader_program *shProg, GLsizei count,
                            const GLchar *const *varyings, GLenum bufferMode)
{
   GLint i;

   /* free existing varyings, if any */
   for (i = 0; i < (GLint) shProg->TransformFeedback.NumVarying; i++) {
      free(shProg->TransformFeedback.VaryingNames[i]);
   }
   free(shProg->TransformFeedback.VaryingNames);

   /* allocate new memory for varying names */
   shProg->TransformFeedback.VaryingNames =
      malloc(count * sizeof(GLchar *));

   if (!shProg->TransformFeedback.VaryingNames) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glTransformFeedbackVaryings()");
      return;
   }

   /* Save the new names and the count */
   for (i = 0; i < count; i++) {
      shProg->TransformFeedback.VaryingNames[i] = strdup(varyings[i]);
   }
   shProg->TransformFeedback.NumVarying = count;

   shProg->TransformFeedback.BufferMode = bufferMode;

   /* No need to invoke FLUSH_VERTICES since
    * the varyings won't be used until shader link time.
    */
}


void GLAPIENTRY
_mesa_TransformFeedbackVaryings_no_error(GLuint program, GLsizei count,
                                         const GLchar *const *varyings,
                                         GLenum bufferMode)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_shader_program *shProg = _mesa_lookup_shader_program(ctx, program);
   transform_feedback_varyings(ctx, shProg, count, varyings, bufferMode);
}

void GLAPIENTRY
_mesa_TransformFeedbackVaryings(GLuint program, GLsizei count,
                                const GLchar * const *varyings,
                                GLenum bufferMode)
{
   struct gl_shader_program *shProg;
   GLint i;
   GET_CURRENT_CONTEXT(ctx);

   /* From the ARB_transform_feedback2 specification:
    * "The error INVALID_OPERATION is generated by TransformFeedbackVaryings
    *  if the current transform feedback object is active, even if paused."
    */
   if (ctx->TransformFeedback.CurrentObject->Active) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
               "glTransformFeedbackVaryings(current object is active)");
      return;
   }

   switch (bufferMode) {
   case GL_INTERLEAVED_ATTRIBS:
      break;
   case GL_SEPARATE_ATTRIBS:
      break;
   default:
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glTransformFeedbackVaryings(bufferMode)");
      return;
   }

   if (count < 0 ||
       (bufferMode == GL_SEPARATE_ATTRIBS &&
        (GLuint) count > ctx->Const.MaxTransformFeedbackBuffers)) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glTransformFeedbackVaryings(count=%d)", count);
      return;
   }

   shProg = _mesa_lookup_shader_program_err(ctx, program,
                                            "glTransformFeedbackVaryings");
   if (!shProg)
      return;

   if (ctx->Extensions.ARB_transform_feedback3) {
      if (bufferMode == GL_INTERLEAVED_ATTRIBS) {
         unsigned buffers = 1;

         for (i = 0; i < count; i++) {
            if (strcmp(varyings[i], "gl_NextBuffer") == 0)
               buffers++;
         }

         if (buffers > ctx->Const.MaxTransformFeedbackBuffers) {
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "glTransformFeedbackVaryings(too many gl_NextBuffer "
                        "occurrences)");
            return;
         }
      } else {
         for (i = 0; i < count; i++) {
            if (strcmp(varyings[i], "gl_NextBuffer") == 0 ||
                strcmp(varyings[i], "gl_SkipComponents1") == 0 ||
                strcmp(varyings[i], "gl_SkipComponents2") == 0 ||
                strcmp(varyings[i], "gl_SkipComponents3") == 0 ||
                strcmp(varyings[i], "gl_SkipComponents4") == 0) {
               _mesa_error(ctx, GL_INVALID_OPERATION,
                           "glTransformFeedbackVaryings(SEPARATE_ATTRIBS,"
                           "varying=%s)",
                           varyings[i]);
               return;
            }
         }
      }
   }

   transform_feedback_varyings(ctx, shProg, count, varyings, bufferMode);
}


/**
 * Get info about the transform feedback outputs which are to be written
 * to the feedback buffer(s).
 */
void GLAPIENTRY
_mesa_GetTransformFeedbackVarying(GLuint program, GLuint index,
                                  GLsizei bufSize, GLsizei *length,
                                  GLsizei *size, GLenum *type, GLchar *name)
{
   const struct gl_shader_program *shProg;
   struct gl_program_resource *res;
   GET_CURRENT_CONTEXT(ctx);

   shProg = _mesa_lookup_shader_program_err(ctx, program,
                                            "glGetTransformFeedbackVarying");
   if (!shProg)
      return;

   res = _mesa_program_resource_find_index((struct gl_shader_program *) shProg,
                                           GL_TRANSFORM_FEEDBACK_VARYING,
                                           index);
   if (!res) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glGetTransformFeedbackVarying(index=%u)", index);
      return;
   }

   /* return the varying's name and length */
   _mesa_copy_string(name, bufSize, length, _mesa_program_resource_name(res));

   /* return the datatype and value's size (in datatype units) */
   if (type)
      _mesa_program_resource_prop((struct gl_shader_program *) shProg,
                                  res, index, GL_TYPE, (GLint*) type,
                                  false, "glGetTransformFeedbackVarying");
   if (size)
      _mesa_program_resource_prop((struct gl_shader_program *) shProg,
                                  res, index, GL_ARRAY_SIZE, (GLint*) size,
                                  false, "glGetTransformFeedbackVarying");
}



struct gl_transform_feedback_object *
_mesa_lookup_transform_feedback_object(struct gl_context *ctx, GLuint name)
{
   /* OpenGL 4.5 core profile, 13.2 pdf page 444: "xfb must be zero, indicating
    * the default transform feedback object, or the name of an existing
    * transform feedback object."
    */
   if (name == 0) {
      return ctx->TransformFeedback.DefaultObject;
   }
   else
      return (struct gl_transform_feedback_object *)
         _mesa_HashLookupLocked(ctx->TransformFeedback.Objects, name);
}

static void
create_transform_feedbacks(struct gl_context *ctx, GLsizei n, GLuint *ids,
                           bool dsa)
{
   const char* func;

   if (dsa)
      func = "glCreateTransformFeedbacks";
   else
      func = "glGenTransformFeedbacks";

   if (n < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(n < 0)", func);
      return;
   }

   if (!ids)
      return;

   if (_mesa_HashFindFreeKeys(ctx->TransformFeedback.Objects, ids, n)) {
      GLsizei i;
      for (i = 0; i < n; i++) {
         struct gl_transform_feedback_object *obj
            = new_transform_feedback(ctx, ids[i]);
         if (!obj) {
            _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s", func);
            return;
         }
         _mesa_HashInsertLocked(ctx->TransformFeedback.Objects, ids[i],
                                obj, true);
         if (dsa) {
            /* this is normally done at bind time in the non-dsa case */
            obj->EverBound = GL_TRUE;
         }
      }
   }
   else {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s", func);
   }
}

/**
 * Create new transform feedback objects.   Transform feedback objects
 * encapsulate the state related to transform feedback to allow quickly
 * switching state (and drawing the results, below).
 * Part of GL_ARB_transform_feedback2.
 */
void GLAPIENTRY
_mesa_GenTransformFeedbacks(GLsizei n, GLuint *names)
{
   GET_CURRENT_CONTEXT(ctx);

   /* GenTransformFeedbacks should just reserve the object names that a
    * subsequent call to BindTransformFeedback should actively create. For
    * the sake of simplicity, we reserve the names and create the objects
    * straight away.
    */

   create_transform_feedbacks(ctx, n, names, false);
}

/**
 * Create new transform feedback objects.   Transform feedback objects
 * encapsulate the state related to transform feedback to allow quickly
 * switching state (and drawing the results, below).
 * Part of GL_ARB_direct_state_access.
 */
void GLAPIENTRY
_mesa_CreateTransformFeedbacks(GLsizei n, GLuint *names)
{
   GET_CURRENT_CONTEXT(ctx);

   create_transform_feedbacks(ctx, n, names, true);
}


/**
 * Is the given ID a transform feedback object?
 * Part of GL_ARB_transform_feedback2.
 */
GLboolean GLAPIENTRY
_mesa_IsTransformFeedback(GLuint name)
{
   struct gl_transform_feedback_object *obj;
   GET_CURRENT_CONTEXT(ctx);

   ASSERT_OUTSIDE_BEGIN_END_WITH_RETVAL(ctx, GL_FALSE);

   if (name == 0)
      return GL_FALSE;

   obj = _mesa_lookup_transform_feedback_object(ctx, name);
   if (obj == NULL)
      return GL_FALSE;

   return obj->EverBound;
}


/**
 * Bind the given transform feedback object.
 * Part of GL_ARB_transform_feedback2.
 */
static ALWAYS_INLINE void
bind_transform_feedback(struct gl_context *ctx, GLuint name, bool no_error)
{
   struct gl_transform_feedback_object *obj;

   obj = _mesa_lookup_transform_feedback_object(ctx, name);
   if (!no_error && !obj) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glBindTransformFeedback(name=%u)", name);
      return;
   }

   reference_transform_feedback_object(&ctx->TransformFeedback.CurrentObject,
                                       obj);
}


void GLAPIENTRY
_mesa_BindTransformFeedback_no_error(GLenum target, GLuint name)
{
   GET_CURRENT_CONTEXT(ctx);
   bind_transform_feedback(ctx, name, true);
}


void GLAPIENTRY
_mesa_BindTransformFeedback(GLenum target, GLuint name)
{
   GET_CURRENT_CONTEXT(ctx);

   if (target != GL_TRANSFORM_FEEDBACK) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glBindTransformFeedback(target)");
      return;
   }

   if (_mesa_is_xfb_active_and_unpaused(ctx)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
              "glBindTransformFeedback(transform is active, or not paused)");
      return;
   }

   bind_transform_feedback(ctx, name, false);
}


/**
 * Delete the given transform feedback objects.
 * Part of GL_ARB_transform_feedback2.
 */
void GLAPIENTRY
_mesa_DeleteTransformFeedbacks(GLsizei n, const GLuint *names)
{
   GLint i;
   GET_CURRENT_CONTEXT(ctx);

   if (n < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glDeleteTransformFeedbacks(n < 0)");
      return;
   }

   if (!names)
      return;

   for (i = 0; i < n; i++) {
      if (names[i] > 0) {
         struct gl_transform_feedback_object *obj
            = _mesa_lookup_transform_feedback_object(ctx, names[i]);
         if (obj) {
            if (obj->Active) {
               _mesa_error(ctx, GL_INVALID_OPERATION,
                           "glDeleteTransformFeedbacks(object %u is active)",
                           names[i]);
               return;
            }
            _mesa_HashRemoveLocked(ctx->TransformFeedback.Objects, names[i]);
            /* unref, but object may not be deleted until later */
            if (obj == ctx->TransformFeedback.CurrentObject) {
               reference_transform_feedback_object(
                     &ctx->TransformFeedback.CurrentObject,
                     ctx->TransformFeedback.DefaultObject);
            }
            reference_transform_feedback_object(&obj, NULL);
         }
      }
   }
}


/**
 * Pause transform feedback.
 * Part of GL_ARB_transform_feedback2.
 */
static void
pause_transform_feedback(struct gl_context *ctx,
                         struct gl_transform_feedback_object *obj)
{
   FLUSH_VERTICES(ctx, 0, 0);

   cso_set_stream_outputs(ctx->cso_context, 0, NULL, NULL);

   obj->Paused = GL_TRUE;
   _mesa_update_valid_to_render_state(ctx);
}


void GLAPIENTRY
_mesa_PauseTransformFeedback_no_error(void)
{
   GET_CURRENT_CONTEXT(ctx);
   pause_transform_feedback(ctx, ctx->TransformFeedback.CurrentObject);
}


void GLAPIENTRY
_mesa_PauseTransformFeedback(void)
{
   struct gl_transform_feedback_object *obj;
   GET_CURRENT_CONTEXT(ctx);

   obj = ctx->TransformFeedback.CurrentObject;

   if (!_mesa_is_xfb_active_and_unpaused(ctx)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
           "glPauseTransformFeedback(feedback not active or already paused)");
      return;
   }

   pause_transform_feedback(ctx, obj);
}


/**
 * Resume transform feedback.
 * Part of GL_ARB_transform_feedback2.
 */
static void
resume_transform_feedback(struct gl_context *ctx,
                          struct gl_transform_feedback_object *obj)
{
   FLUSH_VERTICES(ctx, 0, 0);

   obj->Paused = GL_FALSE;

   unsigned offsets[PIPE_MAX_SO_BUFFERS];
   unsigned i;

   for (i = 0; i < PIPE_MAX_SO_BUFFERS; i++)
      offsets[i] = (unsigned)-1;

   cso_set_stream_outputs(ctx->cso_context, obj->num_targets,
                          obj->targets, offsets);
   _mesa_update_valid_to_render_state(ctx);
}


void GLAPIENTRY
_mesa_ResumeTransformFeedback_no_error(void)
{
   GET_CURRENT_CONTEXT(ctx);
   resume_transform_feedback(ctx, ctx->TransformFeedback.CurrentObject);
}


void GLAPIENTRY
_mesa_ResumeTransformFeedback(void)
{
   struct gl_transform_feedback_object *obj;
   GET_CURRENT_CONTEXT(ctx);

   obj = ctx->TransformFeedback.CurrentObject;

   if (!obj->Active || !obj->Paused) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
               "glResumeTransformFeedback(feedback not active or not paused)");
      return;
   }

   /* From the ARB_transform_feedback2 specification:
    * "The error INVALID_OPERATION is generated by ResumeTransformFeedback if
    *  the program object being used by the current transform feedback object
    *  is not active."
    */
   if (obj->program != get_xfb_source(ctx)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glResumeTransformFeedback(wrong program bound)");
      return;
   }

   resume_transform_feedback(ctx, obj);
}

extern void GLAPIENTRY
_mesa_GetTransformFeedbackiv(GLuint xfb, GLenum pname, GLint *param)
{
    struct gl_transform_feedback_object *obj;
    GET_CURRENT_CONTEXT(ctx);

    obj = lookup_transform_feedback_object_err(ctx, xfb,
                                               "glGetTransformFeedbackiv");
    if (!obj) {
       return;
    }

    switch(pname) {
    case GL_TRANSFORM_FEEDBACK_PAUSED:
       *param = obj->Paused;
       break;
    case GL_TRANSFORM_FEEDBACK_ACTIVE:
       *param = obj->Active;
       break;
    default:
       _mesa_error(ctx, GL_INVALID_ENUM,
                   "glGetTransformFeedbackiv(pname=%i)", pname);
    }
}

extern void GLAPIENTRY
_mesa_GetTransformFeedbacki_v(GLuint xfb, GLenum pname, GLuint index,
                              GLint *param)
{
   struct gl_transform_feedback_object *obj;
   GET_CURRENT_CONTEXT(ctx);

   obj = lookup_transform_feedback_object_err(ctx, xfb,
                                              "glGetTransformFeedbacki_v");
   if (!obj) {
      return;
   }

   if (index >= ctx->Const.MaxTransformFeedbackBuffers) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glGetTransformFeedbacki_v(index=%i)", index);
      return;
   }

   switch(pname) {
   case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
      *param = obj->BufferNames[index];
      break;
   default:
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glGetTransformFeedbacki_v(pname=%i)", pname);
   }
}

extern void GLAPIENTRY
_mesa_GetTransformFeedbacki64_v(GLuint xfb, GLenum pname, GLuint index,
                                GLint64 *param)
{
   struct gl_transform_feedback_object *obj;
   GET_CURRENT_CONTEXT(ctx);

   obj = lookup_transform_feedback_object_err(ctx, xfb,
                                              "glGetTransformFeedbacki64_v");
   if (!obj) {
      return;
   }

   if (index >= ctx->Const.MaxTransformFeedbackBuffers) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glGetTransformFeedbacki64_v(index=%i)", index);
      return;
   }

   /**
    * This follows the same general rules used for BindBufferBase:
    *
    *   "To query the starting offset or size of the range of a buffer
    *    object binding in an indexed array, call GetInteger64i_v with
    *    target set to respectively the starting offset or binding size
    *    name from table 6.5 for that array. Index must be in the range
    *    zero to the number of bind points supported minus one. If the
    *    starting offset or size was not specified when the buffer object
    *    was bound (e.g. if it was bound with BindBufferBase), or if no
    *    buffer object is bound to the target array at index, zero is
    *    returned."
    */
   if (obj->RequestedSize[index] == 0 &&
       (pname == GL_TRANSFORM_FEEDBACK_BUFFER_START ||
        pname == GL_TRANSFORM_FEEDBACK_BUFFER_SIZE)) {
      *param = 0;
      return;
   }

   compute_transform_feedback_buffer_sizes(obj);
   switch(pname) {
   case GL_TRANSFORM_FEEDBACK_BUFFER_START:
      assert(obj->RequestedSize[index] > 0);
      *param = obj->Offset[index];
      break;
   case GL_TRANSFORM_FEEDBACK_BUFFER_SIZE:
      assert(obj->RequestedSize[index] > 0);
      *param = obj->Size[index];
      break;
   default:
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glGetTransformFeedbacki64_v(pname=%i)", pname);
   }
}
