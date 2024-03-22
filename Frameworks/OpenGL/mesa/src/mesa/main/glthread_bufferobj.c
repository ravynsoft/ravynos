/*
 * Copyright © 2012 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "main/glthread_marshal.h"
#include "main/dispatch.h"
#include "main/bufferobj.h"

/**
 * Create an upload buffer. This is called from the app thread, so everything
 * has to be thread-safe in the driver.
 */
static struct gl_buffer_object *
new_upload_buffer(struct gl_context *ctx, GLsizeiptr size, uint8_t **ptr)
{
   /* id 0 is used to avoid returning invalid binding values to apps */
   struct gl_buffer_object *obj =
      _mesa_bufferobj_alloc(ctx, 0);
   if (!obj)
      return NULL;

   obj->Immutable = true;
   obj->GLThreadInternal = true;

   if (!_mesa_bufferobj_data(ctx, GL_ARRAY_BUFFER, size, NULL,
                          GL_WRITE_ONLY,
                          GL_CLIENT_STORAGE_BIT | GL_MAP_WRITE_BIT,
                          obj)) {
      _mesa_delete_buffer_object(ctx, obj);
      return NULL;
   }

   *ptr = _mesa_bufferobj_map_range(ctx, 0, size,
                                 GL_MAP_WRITE_BIT |
                                 GL_MAP_UNSYNCHRONIZED_BIT |
                                 MESA_MAP_THREAD_SAFE_BIT,
                                 obj, MAP_GLTHREAD);
   if (!*ptr) {
      _mesa_delete_buffer_object(ctx, obj);
      return NULL;
   }

   return obj;
}

void
_mesa_glthread_release_upload_buffer(struct gl_context *ctx)
{
   struct glthread_state *glthread = &ctx->GLThread;

   if (glthread->upload_buffer_private_refcount > 0) {
      p_atomic_add(&glthread->upload_buffer->RefCount,
                   -glthread->upload_buffer_private_refcount);
      glthread->upload_buffer_private_refcount = 0;
   }
   _mesa_reference_buffer_object(ctx, &glthread->upload_buffer, NULL);
}

void
_mesa_glthread_upload(struct gl_context *ctx, const void *data,
                      GLsizeiptr size, unsigned *out_offset,
                      struct gl_buffer_object **out_buffer,
                      uint8_t **out_ptr,
                      unsigned start_offset)
{
   struct glthread_state *glthread = &ctx->GLThread;
   const unsigned default_size = 1024 * 1024;

   if (unlikely(size > INT_MAX))
      return;

   /* The alignment was chosen arbitrarily. */
   unsigned offset = align(glthread->upload_offset, size <= 4 ? 4 : 8) + start_offset;

   /* Allocate a new buffer if needed. */
   if (unlikely(!glthread->upload_buffer || offset + size > default_size)) {
      /* If the size is greater than the buffer size, allocate a separate buffer
       * just for this upload.
       */
      if (unlikely(start_offset + size > default_size)) {
         uint8_t *ptr;

         assert(*out_buffer == NULL);
         *out_buffer = new_upload_buffer(ctx, size + start_offset, &ptr);
         if (!*out_buffer)
            return;

         ptr += start_offset;
         *out_offset = start_offset;
         if (data)
            memcpy(ptr, data, size);
         else
            *out_ptr = ptr;
         return;
      }

      _mesa_glthread_release_upload_buffer(ctx);

      glthread->upload_buffer =
         new_upload_buffer(ctx, default_size, &glthread->upload_ptr);
      glthread->upload_offset = 0;
      offset = start_offset;

      /* Since atomic operations are very very slow when 2 threads are not
       * sharing one L3 cache (which can happen on AMD Zen), prevent using
       * atomics as follows:
       *
       * This function has to return a buffer reference to the caller.
       * Instead of atomic_inc for every call, it does all possible future
       * increments in advance when the upload buffer is allocated.
       * The maximum number of times the function can be called per upload
       * buffer is default_size, because the minimum allocation size is 1.
       * Therefore the function can only return default_size number of
       * references at most, so we will never need more. This is the number
       * that is added to RefCount at allocation.
       *
       * upload_buffer_private_refcount tracks how many buffer references
       * are left to return to callers. If the buffer is full and there are
       * still references left, they are atomically subtracted from RefCount
       * before the buffer is unreferenced.
       *
       * This can increase performance by 20%.
       */
      glthread->upload_buffer->RefCount += default_size;
      glthread->upload_buffer_private_refcount = default_size;
   }

   /* Upload data. */
   if (data)
      memcpy(glthread->upload_ptr + offset, data, size);
   else
      *out_ptr = glthread->upload_ptr + offset;

   glthread->upload_offset = offset + size;
   *out_offset = offset;

   assert(*out_buffer == NULL);
   assert(glthread->upload_buffer_private_refcount > 0);
   *out_buffer = glthread->upload_buffer;
   glthread->upload_buffer_private_refcount--;
}

/** Tracks the current bindings for the vertex array and index array buffers.
 *
 * This is part of what we need to enable glthread on compat-GL contexts that
 * happen to use VBOs, without also supporting the full tracking of VBO vs
 * user vertex array bindings per attribute on each vertex array for
 * determining what to upload at draw call time.
 *
 * Note that GL core makes it so that a buffer binding with an invalid handle
 * in the "buffer" parameter will throw an error, and then a
 * glVertexAttribPointer() that followsmight not end up pointing at a VBO.
 * However, in GL core the draw call would throw an error as well, so we don't
 * really care if our tracking is wrong for this case -- we never need to
 * marshal user data for draw calls, and the unmarshal will just generate an
 * error or not as appropriate.
 *
 * For compatibility GL, we do need to accurately know whether the draw call
 * on the unmarshal side will dereference a user pointer or load data from a
 * VBO per vertex.  That would make it seem like we need to track whether a
 * "buffer" is valid, so that we can know when an error will be generated
 * instead of updating the binding.  However, compat GL has the ridiculous
 * feature that if you pass a bad name, it just gens a buffer object for you,
 * so we escape without having to know if things are valid or not.
 */
static void
_mesa_glthread_BindBuffer(struct gl_context *ctx, GLenum target, GLuint buffer)
{
   struct glthread_state *glthread = &ctx->GLThread;

   switch (target) {
   case GL_ARRAY_BUFFER:
      glthread->CurrentArrayBufferName = buffer;
      break;
   case GL_ELEMENT_ARRAY_BUFFER:
      /* The current element array buffer binding is actually tracked in the
       * vertex array object instead of the context, so this would need to
       * change on vertex array object updates.
       */
      glthread->CurrentVAO->CurrentElementBufferName = buffer;
      break;
   case GL_DRAW_INDIRECT_BUFFER:
      glthread->CurrentDrawIndirectBufferName = buffer;
      break;
   case GL_PIXEL_PACK_BUFFER:
      glthread->CurrentPixelPackBufferName = buffer;
      break;
   case GL_PIXEL_UNPACK_BUFFER:
      glthread->CurrentPixelUnpackBufferName = buffer;
      break;
   case GL_QUERY_BUFFER:
      glthread->CurrentQueryBufferName = buffer;
      break;
   }
}

/* This can hold up to 2 BindBuffer calls. This is used to eliminate
 * duplicated BindBuffer calls, which are plentiful in viewperf2020/catia.
 * In this example, the first 2 calls are eliminated by glthread by keeping
 * track of the last 2 BindBuffer calls and overwriting them if the target
 * matches.
 *
 *   glBindBuffer(GL_ARRAY_BUFFER, 0);
 *   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
 *   glBindBuffer(GL_ARRAY_BUFFER, 6);
 *   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 7);
 */
struct marshal_cmd_BindBuffer
{
   struct marshal_cmd_base cmd_base;
   GLenum16 target[2];
   GLuint buffer[2];
};

uint32_t
_mesa_unmarshal_BindBuffer(struct gl_context *ctx,
                           const struct marshal_cmd_BindBuffer *restrict cmd)
{
   CALL_BindBuffer(ctx->Dispatch.Current, (cmd->target[0], cmd->buffer[0]));

   if (cmd->target[1])
      CALL_BindBuffer(ctx->Dispatch.Current, (cmd->target[1], cmd->buffer[1]));

   const unsigned cmd_size = (align(sizeof(struct marshal_cmd_BindBuffer), 8) / 8);
   assert (cmd_size == cmd->cmd_base.cmd_size);
   return cmd_size;
}

void GLAPIENTRY
_mesa_marshal_BindBuffer(GLenum target, GLuint buffer)
{
   GET_CURRENT_CONTEXT(ctx);
   struct glthread_state *glthread = &ctx->GLThread;
   struct marshal_cmd_BindBuffer *last = glthread->LastBindBuffer;

   _mesa_glthread_BindBuffer(ctx, target, buffer);

   /* If the last call is BindBuffer... */
   if (_mesa_glthread_call_is_last(glthread, &last->cmd_base)) {
      /* If the target is in the last call and unbinding the buffer, overwrite
       * the buffer ID there.
       *
       * We can't optimize out binding non-zero buffers because binding also
       * creates the GL objects (like glCreateBuffers), which can't be skipped.
       */
      if (target == last->target[0] && !last->buffer[0]) {
         last->buffer[0] = buffer;
         return;
      }
      if (target == last->target[1] && !last->buffer[1]) {
         last->buffer[1] = buffer;
         return;
      }

      /* If the last call has an unused buffer field, add this call to it. */
      if (last->target[1] == 0) {
         last->target[1] = MIN2(target, 0xffff); /* clamped to 0xffff (invalid enum) */
         last->buffer[1] = buffer;
         return;
      }
   }

   int cmd_size = sizeof(struct marshal_cmd_BindBuffer);
   struct marshal_cmd_BindBuffer *cmd =
      _mesa_glthread_allocate_command(ctx, DISPATCH_CMD_BindBuffer, cmd_size);

   cmd->target[0] = MIN2(target, 0xffff); /* clamped to 0xffff (invalid enum) */
   cmd->target[1] = 0;
   cmd->buffer[0] = buffer;

   glthread->LastBindBuffer = cmd;
}

void
_mesa_glthread_DeleteBuffers(struct gl_context *ctx, GLsizei n,
                             const GLuint *buffers)
{
   struct glthread_state *glthread = &ctx->GLThread;

   if (!buffers || n < 0)
      return;

   for (unsigned i = 0; i < n; i++) {
      GLuint id = buffers[i];

      if (id == glthread->CurrentArrayBufferName)
         _mesa_glthread_BindBuffer(ctx, GL_ARRAY_BUFFER, 0);
      if (id == glthread->CurrentVAO->CurrentElementBufferName)
         _mesa_glthread_BindBuffer(ctx, GL_ELEMENT_ARRAY_BUFFER, 0);
      if (id == glthread->CurrentDrawIndirectBufferName)
         _mesa_glthread_BindBuffer(ctx, GL_DRAW_INDIRECT_BUFFER, 0);
      if (id == glthread->CurrentPixelPackBufferName)
         _mesa_glthread_BindBuffer(ctx, GL_PIXEL_PACK_BUFFER, 0);
      if (id == glthread->CurrentPixelUnpackBufferName)
         _mesa_glthread_BindBuffer(ctx, GL_PIXEL_UNPACK_BUFFER, 0);
   }
}

/* BufferData: marshalled asynchronously */
struct marshal_cmd_BufferData
{
   struct marshal_cmd_base cmd_base;
   GLuint target_or_name;
   GLsizeiptr size;
   GLenum usage;
   const GLvoid *data_external_mem;
   bool data_null; /* If set, no data follows for "data" */
   bool named;
   bool ext_dsa;
   /* Next size bytes are GLubyte data[size] */
};

uint32_t
_mesa_unmarshal_BufferData(struct gl_context *ctx,
                           const struct marshal_cmd_BufferData *restrict cmd)
{
   const GLuint target_or_name = cmd->target_or_name;
   const GLsizei size = cmd->size;
   const GLenum usage = cmd->usage;
   const void *data;

   if (cmd->data_null)
      data = NULL;
   else if (!cmd->named && target_or_name == GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD)
      data = cmd->data_external_mem;
   else
      data = (const void *) (cmd + 1);

   if (cmd->ext_dsa) {
      CALL_NamedBufferDataEXT(ctx->Dispatch.Current,
                              (target_or_name, size, data, usage));
   } else if (cmd->named) {
      CALL_NamedBufferData(ctx->Dispatch.Current,
                           (target_or_name, size, data, usage));
   } else {
      CALL_BufferData(ctx->Dispatch.Current,
                      (target_or_name, size, data, usage));
   }
   return cmd->cmd_base.cmd_size;
}

uint32_t
_mesa_unmarshal_NamedBufferData(struct gl_context *ctx,
                                const struct marshal_cmd_NamedBufferData *restrict cmd)
{
   unreachable("never used - all BufferData variants use DISPATCH_CMD_BufferData");
   return 0;
}

uint32_t
_mesa_unmarshal_NamedBufferDataEXT(struct gl_context *ctx,
                                   const struct marshal_cmd_NamedBufferDataEXT *restrict cmd)
{
   unreachable("never used - all BufferData variants use DISPATCH_CMD_BufferData");
   return 0;
}

static void
_mesa_marshal_BufferData_merged(GLuint target_or_name, GLsizeiptr size,
                                const GLvoid *data, GLenum usage, bool named,
                                bool ext_dsa, const char *func)
{
   GET_CURRENT_CONTEXT(ctx);
   bool external_mem = !named &&
                       target_or_name == GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD;
   bool copy_data = data && !external_mem;
   size_t cmd_size = sizeof(struct marshal_cmd_BufferData) + (copy_data ? size : 0);

   if (unlikely(size < 0 || size > INT_MAX || cmd_size > MARSHAL_MAX_CMD_SIZE ||
                (named && target_or_name == 0))) {
      _mesa_glthread_finish_before(ctx, func);
      if (named) {
         CALL_NamedBufferData(ctx->Dispatch.Current,
                              (target_or_name, size, data, usage));
      } else {
         CALL_BufferData(ctx->Dispatch.Current,
                         (target_or_name, size, data, usage));
      }
      return;
   }

   struct marshal_cmd_BufferData *cmd =
      _mesa_glthread_allocate_command(ctx, DISPATCH_CMD_BufferData,
                                      cmd_size);

   cmd->target_or_name = target_or_name;
   cmd->size = size;
   cmd->usage = usage;
   cmd->data_null = !data;
   cmd->named = named;
   cmd->ext_dsa = ext_dsa;
   cmd->data_external_mem = data;

   if (copy_data) {
      char *variable_data = (char *) (cmd + 1);
      memcpy(variable_data, data, size);
   }
}

void GLAPIENTRY
_mesa_marshal_BufferData(GLenum target, GLsizeiptr size, const GLvoid * data,
                         GLenum usage)
{
   _mesa_marshal_BufferData_merged(target, size, data, usage, false, false,
                                   "BufferData");
}

void GLAPIENTRY
_mesa_marshal_NamedBufferData(GLuint buffer, GLsizeiptr size,
                              const GLvoid * data, GLenum usage)
{
   _mesa_marshal_BufferData_merged(buffer, size, data, usage, true, false,
                                   "NamedBufferData");
}

void GLAPIENTRY
_mesa_marshal_NamedBufferDataEXT(GLuint buffer, GLsizeiptr size,
                                 const GLvoid *data, GLenum usage)
{
   _mesa_marshal_BufferData_merged(buffer, size, data, usage, true, true,
                                   "NamedBufferDataEXT");
}


/* BufferSubData: marshalled asynchronously */
struct marshal_cmd_BufferSubData
{
   struct marshal_cmd_base cmd_base;
   GLenum target_or_name;
   GLintptr offset;
   GLsizeiptr size;
   bool named;
   bool ext_dsa;
   /* Next size bytes are GLubyte data[size] */
};

uint32_t
_mesa_unmarshal_BufferSubData(struct gl_context *ctx,
                              const struct marshal_cmd_BufferSubData *restrict cmd)
{
   const GLenum target_or_name = cmd->target_or_name;
   const GLintptr offset = cmd->offset;
   const GLsizeiptr size = cmd->size;
   const void *data = (const void *) (cmd + 1);

   if (cmd->ext_dsa) {
      CALL_NamedBufferSubDataEXT(ctx->Dispatch.Current,
                                 (target_or_name, offset, size, data));
   } else if (cmd->named) {
      CALL_NamedBufferSubData(ctx->Dispatch.Current,
                              (target_or_name, offset, size, data));
   } else {
      CALL_BufferSubData(ctx->Dispatch.Current,
                         (target_or_name, offset, size, data));
   }
   return cmd->cmd_base.cmd_size;
}

uint32_t
_mesa_unmarshal_NamedBufferSubData(struct gl_context *ctx,
                                   const struct marshal_cmd_NamedBufferSubData *restrict cmd)
{
   unreachable("never used - all BufferSubData variants use DISPATCH_CMD_BufferSubData");
   return 0;
}

uint32_t
_mesa_unmarshal_NamedBufferSubDataEXT(struct gl_context *ctx,
                                      const struct marshal_cmd_NamedBufferSubDataEXT *restrict cmd)
{
   unreachable("never used - all BufferSubData variants use DISPATCH_CMD_BufferSubData");
   return 0;
}

static void
_mesa_marshal_BufferSubData_merged(GLuint target_or_name, GLintptr offset,
                                   GLsizeiptr size, const GLvoid *data,
                                   bool named, bool ext_dsa, const char *func)
{
   GET_CURRENT_CONTEXT(ctx);
   size_t cmd_size = sizeof(struct marshal_cmd_BufferSubData) + size;

   /* Fast path: Copy the data to an upload buffer, and use the GPU
    * to copy the uploaded data to the destination buffer.
    */
   /* TODO: Handle offset == 0 && size < buffer_size.
    *       If offset == 0 and size == buffer_size, it's better to discard
    *       the buffer storage, but we don't know the buffer size in glthread.
    */
   if (ctx->Const.AllowGLThreadBufferSubDataOpt &&
       ctx->Dispatch.Current != ctx->Dispatch.ContextLost &&
       data && offset > 0 && size > 0) {
      struct gl_buffer_object *upload_buffer = NULL;
      unsigned upload_offset = 0;

      _mesa_glthread_upload(ctx, data, size, &upload_offset, &upload_buffer,
                            NULL, 0);

      if (upload_buffer) {
         _mesa_marshal_InternalBufferSubDataCopyMESA((GLintptr)upload_buffer,
                                                     upload_offset,
                                                     target_or_name,
                                                     offset, size, named,
                                                     ext_dsa);
         return;
      }
   }

   if (unlikely(size < 0 || size > INT_MAX || cmd_size < 0 ||
                cmd_size > MARSHAL_MAX_CMD_SIZE || !data ||
                (named && target_or_name == 0))) {
      _mesa_glthread_finish_before(ctx, func);
      if (named) {
         CALL_NamedBufferSubData(ctx->Dispatch.Current,
                                 (target_or_name, offset, size, data));
      } else {
         CALL_BufferSubData(ctx->Dispatch.Current,
                            (target_or_name, offset, size, data));
      }
      return;
   }

   struct marshal_cmd_BufferSubData *cmd =
      _mesa_glthread_allocate_command(ctx, DISPATCH_CMD_BufferSubData,
                                      cmd_size);
   cmd->target_or_name = target_or_name;
   cmd->offset = offset;
   cmd->size = size;
   cmd->named = named;
   cmd->ext_dsa = ext_dsa;

   char *variable_data = (char *) (cmd + 1);
   memcpy(variable_data, data, size);
}

void GLAPIENTRY
_mesa_marshal_BufferSubData(GLenum target, GLintptr offset, GLsizeiptr size,
                            const GLvoid * data)
{
   _mesa_marshal_BufferSubData_merged(target, offset, size, data, false,
                                      false, "BufferSubData");
}

void GLAPIENTRY
_mesa_marshal_NamedBufferSubData(GLuint buffer, GLintptr offset,
                                 GLsizeiptr size, const GLvoid * data)
{
   _mesa_marshal_BufferSubData_merged(buffer, offset, size, data, true,
                                      false, "NamedBufferSubData");
}

void GLAPIENTRY
_mesa_marshal_NamedBufferSubDataEXT(GLuint buffer, GLintptr offset,
                                    GLsizeiptr size, const GLvoid * data)
{
   _mesa_marshal_BufferSubData_merged(buffer, offset, size, data, true,
                                      true, "NamedBufferSubDataEXT");
}
