/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 * Copyright (C) 2009  VMware, Inc.  All Rights Reserved.
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


/**
 * \file bufferobj.c
 * \brief Functions for the GL_ARB_vertex/pixel_buffer_object extensions.
 * \author Brian Paul, Ian Romanick
 */

#include <stdbool.h>
#include <inttypes.h>  /* for PRId64 macro */
#include "util/u_debug.h"
#include "util/glheader.h"
#include "enums.h"
#include "hash.h"
#include "context.h"
#include "bufferobj.h"
#include "externalobjects.h"
#include "mtypes.h"
#include "teximage.h"
#include "glformats.h"
#include "texstore.h"
#include "transformfeedback.h"
#include "varray.h"
#include "util/u_atomic.h"
#include "util/u_memory.h"
#include "api_exec_decl.h"
#include "util/set.h"

#include "state_tracker/st_debug.h"
#include "state_tracker/st_atom.h"
#include "frontend/api.h"

#include "util/u_inlines.h"
/* Debug flags */
/*#define VBO_DEBUG*/
/*#define BOUNDS_CHECK*/


/**
 * We count the number of buffer modification calls to check for
 * inefficient buffer use.  This is the number of such calls before we
 * issue a warning.
 */
#define BUFFER_WARNING_CALL_COUNT 4


/**
 * Replace data in a subrange of buffer object.  If the data range
 * specified by size + offset extends beyond the end of the buffer or
 * if data is NULL, no copy is performed.
 * Called via glBufferSubDataARB().
 */
void
_mesa_bufferobj_subdata(struct gl_context *ctx,
                        GLintptrARB offset,
                        GLsizeiptrARB size,
                        const void *data, struct gl_buffer_object *obj)
{
   /* we may be called from VBO code, so double-check params here */
   assert(offset >= 0);
   assert(size >= 0);
   assert(offset + size <= obj->Size);

   if (!size)
      return;

   /*
    * According to ARB_vertex_buffer_object specification, if data is null,
    * then the contents of the buffer object's data store is undefined. We just
    * ignore, and leave it unchanged.
    */
   if (!data)
      return;

   if (!obj->buffer) {
      /* we probably ran out of memory during buffer allocation */
      return;
   }

   /* Now that transfers are per-context, we don't have to figure out
    * flushing here.  Usually drivers won't need to flush in this case
    * even if the buffer is currently referenced by hardware - they
    * just queue the upload as dma rather than mapping the underlying
    * buffer directly.
    *
    * If the buffer is mapped, suppress implicit buffer range invalidation
    * by using PIPE_MAP_DIRECTLY.
    */
   struct pipe_context *pipe = ctx->pipe;

   pipe->buffer_subdata(pipe, obj->buffer,
                        _mesa_bufferobj_mapped(obj, MAP_USER) ?
                           PIPE_MAP_DIRECTLY : 0,
                        offset, size, data);
}


/**
 * Called via glGetBufferSubDataARB().
 */
static void
bufferobj_get_subdata(struct gl_context *ctx,
                      GLintptrARB offset,
                      GLsizeiptrARB size,
                      void *data, struct gl_buffer_object *obj)
{
   /* we may be called from VBO code, so double-check params here */
   assert(offset >= 0);
   assert(size >= 0);
   assert(offset + size <= obj->Size);

   if (!size)
      return;

   if (!obj->buffer) {
      /* we probably ran out of memory during buffer allocation */
      return;
   }

   pipe_buffer_read(ctx->pipe, obj->buffer,
                    offset, size, data);
}

void
_mesa_bufferobj_get_subdata(struct gl_context *ctx,
                            GLintptrARB offset,
                            GLsizeiptrARB size,
                            void *data, struct gl_buffer_object *obj)
{
   bufferobj_get_subdata(ctx, offset, size, data, obj);
}

/**
 * Return bitmask of PIPE_BIND_x flags corresponding a GL buffer target.
 */
static unsigned
buffer_target_to_bind_flags(GLenum target)
{
   switch (target) {
   case GL_PIXEL_PACK_BUFFER_ARB:
   case GL_PIXEL_UNPACK_BUFFER_ARB:
      return PIPE_BIND_RENDER_TARGET | PIPE_BIND_SAMPLER_VIEW;
   case GL_ARRAY_BUFFER_ARB:
      return PIPE_BIND_VERTEX_BUFFER;
   case GL_ELEMENT_ARRAY_BUFFER_ARB:
      return PIPE_BIND_INDEX_BUFFER;
   case GL_TEXTURE_BUFFER:
      return PIPE_BIND_SAMPLER_VIEW;
   case GL_TRANSFORM_FEEDBACK_BUFFER:
      return PIPE_BIND_STREAM_OUTPUT;
   case GL_UNIFORM_BUFFER:
      return PIPE_BIND_CONSTANT_BUFFER;
   case GL_DRAW_INDIRECT_BUFFER:
   case GL_PARAMETER_BUFFER_ARB:
      return PIPE_BIND_COMMAND_ARGS_BUFFER;
   case GL_ATOMIC_COUNTER_BUFFER:
   case GL_SHADER_STORAGE_BUFFER:
      return PIPE_BIND_SHADER_BUFFER;
   case GL_QUERY_BUFFER:
      return PIPE_BIND_QUERY_BUFFER;
   default:
      return 0;
   }
}


/**
 * Return bitmask of PIPE_RESOURCE_x flags corresponding to GL_MAP_x flags.
 */
static unsigned
storage_flags_to_buffer_flags(GLbitfield storageFlags)
{
   unsigned flags = 0;
   if (storageFlags & GL_MAP_PERSISTENT_BIT)
      flags |= PIPE_RESOURCE_FLAG_MAP_PERSISTENT;
   if (storageFlags & GL_MAP_COHERENT_BIT)
      flags |= PIPE_RESOURCE_FLAG_MAP_COHERENT;
   if (storageFlags & GL_SPARSE_STORAGE_BIT_ARB)
      flags |= PIPE_RESOURCE_FLAG_SPARSE;
   return flags;
}


/**
 * From a buffer object's target, immutability flag, storage flags and
 * usage hint, return a pipe_resource_usage value (PIPE_USAGE_DYNAMIC,
 * STREAM, etc).
 */
static enum pipe_resource_usage
buffer_usage(GLenum target, GLboolean immutable,
             GLbitfield storageFlags, GLenum usage)
{
   /* "immutable" means that "storageFlags" was set by the user and "usage"
    * was guessed by Mesa. Otherwise, "usage" was set by the user and
    * storageFlags was guessed by Mesa.
    *
    * Therefore, use storageFlags with immutable, else use "usage".
    */
   if (immutable) {
      /* BufferStorage */
      if (storageFlags & GL_MAP_READ_BIT)
         return PIPE_USAGE_STAGING;
      else if (storageFlags & GL_CLIENT_STORAGE_BIT)
         return PIPE_USAGE_STREAM;
      else
         return PIPE_USAGE_DEFAULT;
   }
   else {
      /* These are often read by the CPU, so enable CPU caches. */
      if (target == GL_PIXEL_PACK_BUFFER ||
          target == GL_PIXEL_UNPACK_BUFFER)
         return PIPE_USAGE_STAGING;

      /* BufferData */
      switch (usage) {
      case GL_DYNAMIC_DRAW:
      case GL_DYNAMIC_COPY:
         return PIPE_USAGE_DYNAMIC;
      case GL_STREAM_DRAW:
      case GL_STREAM_COPY:
         return PIPE_USAGE_STREAM;
      case GL_STATIC_READ:
      case GL_DYNAMIC_READ:
      case GL_STREAM_READ:
         return PIPE_USAGE_STAGING;
      case GL_STATIC_DRAW:
      case GL_STATIC_COPY:
      default:
         return PIPE_USAGE_DEFAULT;
      }
   }
}


static ALWAYS_INLINE GLboolean
bufferobj_data(struct gl_context *ctx,
               GLenum target,
               GLsizeiptrARB size,
               const void *data,
               struct gl_memory_object *memObj,
               GLuint64 offset,
               GLenum usage,
               GLbitfield storageFlags,
               struct gl_buffer_object *obj)
{
   struct pipe_context *pipe = ctx->pipe;
   struct pipe_screen *screen = pipe->screen;
   bool is_mapped = _mesa_bufferobj_mapped(obj, MAP_USER);

   if (size > UINT32_MAX || offset > UINT32_MAX) {
      /* pipe_resource.width0 is 32 bits only and increasing it
       * to 64 bits doesn't make much sense since hw support
       * for > 4GB resources is limited.
       */
      obj->Size = 0;
      return GL_FALSE;
   }

   if (target != GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD &&
       size && obj->buffer &&
       obj->Size == size &&
       obj->Usage == usage &&
       obj->StorageFlags == storageFlags) {
      if (data) {
         /* Just discard the old contents and write new data.
          * This should be the same as creating a new buffer, but we avoid
          * a lot of validation in Mesa.
          *
          * If the buffer is mapped, we can't discard it.
          *
          * PIPE_MAP_DIRECTLY supresses implicit buffer range
          * invalidation.
          */
         pipe->buffer_subdata(pipe, obj->buffer,
                              is_mapped ? PIPE_MAP_DIRECTLY :
                                          PIPE_MAP_DISCARD_WHOLE_RESOURCE,
                              0, size, data);
         return GL_TRUE;
      } else if (is_mapped) {
         return GL_TRUE; /* can't reallocate, nothing to do */
      } else if (screen->get_param(screen, PIPE_CAP_INVALIDATE_BUFFER)) {
         pipe->invalidate_resource(pipe, obj->buffer);
         return GL_TRUE;
      }
   }

   obj->Size = size;
   obj->Usage = usage;
   obj->StorageFlags = storageFlags;

   _mesa_bufferobj_release_buffer(obj);

   unsigned bindings = buffer_target_to_bind_flags(target);

   if (storageFlags & MESA_GALLIUM_VERTEX_STATE_STORAGE)
      bindings |= PIPE_BIND_VERTEX_STATE;

   if (ST_DEBUG & DEBUG_BUFFER) {
      debug_printf("Create buffer size %" PRId64 " bind 0x%x\n",
                   (int64_t) size, bindings);
   }

   if (size != 0) {
      struct pipe_resource buffer;

      memset(&buffer, 0, sizeof buffer);
      buffer.target = PIPE_BUFFER;
      buffer.format = PIPE_FORMAT_R8_UNORM; /* want TYPELESS or similar */
      buffer.bind = bindings;
      buffer.usage =
         buffer_usage(target, obj->Immutable, storageFlags, usage);
      buffer.flags = storage_flags_to_buffer_flags(storageFlags);
      buffer.width0 = size;
      buffer.height0 = 1;
      buffer.depth0 = 1;
      buffer.array_size = 1;

      if (memObj) {
         obj->buffer = screen->resource_from_memobj(screen, &buffer,
                                                    memObj->memory,
                                                    offset);
      }
      else if (target == GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD) {
         obj->buffer =
            screen->resource_from_user_memory(screen, &buffer, (void*)data);
      }
      else {
         obj->buffer = screen->resource_create(screen, &buffer);

         if (obj->buffer && data)
            pipe_buffer_write(pipe, obj->buffer, 0, size, data);
      }

      if (!obj->buffer) {
         /* out of memory */
         obj->Size = 0;
         return GL_FALSE;
      }

      obj->private_refcount_ctx = ctx;
   }

   /* The current buffer may be bound, so we have to revalidate all atoms that
    * might be using it.
    */
   if (obj->UsageHistory & USAGE_ARRAY_BUFFER)
      ctx->NewDriverState |= ST_NEW_VERTEX_ARRAYS;
   if (obj->UsageHistory & USAGE_UNIFORM_BUFFER)
      ctx->NewDriverState |= ST_NEW_UNIFORM_BUFFER;
   if (obj->UsageHistory & USAGE_SHADER_STORAGE_BUFFER)
      ctx->NewDriverState |= ST_NEW_STORAGE_BUFFER;
   if (obj->UsageHistory & USAGE_TEXTURE_BUFFER)
      ctx->NewDriverState |= ST_NEW_SAMPLER_VIEWS | ST_NEW_IMAGE_UNITS;
   if (obj->UsageHistory & USAGE_ATOMIC_COUNTER_BUFFER)
      ctx->NewDriverState |= ctx->DriverFlags.NewAtomicBuffer;

   return GL_TRUE;
}

/**
 * Allocate space for and store data in a buffer object.  Any data that was
 * previously stored in the buffer object is lost.  If data is NULL,
 * memory will be allocated, but no copy will occur.
 * Called via ctx->Driver.BufferData().
 * \return GL_TRUE for success, GL_FALSE if out of memory
 */
GLboolean
_mesa_bufferobj_data(struct gl_context *ctx,
                  GLenum target,
                  GLsizeiptrARB size,
                  const void *data,
                  GLenum usage,
                  GLbitfield storageFlags,
                  struct gl_buffer_object *obj)
{
   return bufferobj_data(ctx, target, size, data, NULL, 0, usage, storageFlags, obj);
}

static GLboolean
bufferobj_data_mem(struct gl_context *ctx,
                   GLenum target,
                   GLsizeiptrARB size,
                   struct gl_memory_object *memObj,
                   GLuint64 offset,
                   GLenum usage,
                   struct gl_buffer_object *bufObj)
{
   return bufferobj_data(ctx, target, size, NULL, memObj, offset, usage, GL_DYNAMIC_STORAGE_BIT, bufObj);
}

/**
 * Convert GLbitfield of GL_MAP_x flags to gallium pipe_map_flags flags.
 * \param wholeBuffer  is the whole buffer being mapped?
 */
enum pipe_map_flags
_mesa_access_flags_to_transfer_flags(GLbitfield access, bool wholeBuffer)
{
   enum pipe_map_flags flags = 0;

   if (access & GL_MAP_WRITE_BIT)
      flags |= PIPE_MAP_WRITE;

   if (access & GL_MAP_READ_BIT)
      flags |= PIPE_MAP_READ;

   if (access & GL_MAP_FLUSH_EXPLICIT_BIT)
      flags |= PIPE_MAP_FLUSH_EXPLICIT;

   if (access & GL_MAP_INVALIDATE_BUFFER_BIT) {
      flags |= PIPE_MAP_DISCARD_WHOLE_RESOURCE;
   }
   else if (access & GL_MAP_INVALIDATE_RANGE_BIT) {
      if (wholeBuffer)
         flags |= PIPE_MAP_DISCARD_WHOLE_RESOURCE;
      else
         flags |= PIPE_MAP_DISCARD_RANGE;
   }

   if (access & GL_MAP_UNSYNCHRONIZED_BIT)
      flags |= PIPE_MAP_UNSYNCHRONIZED;

   if (access & GL_MAP_PERSISTENT_BIT)
      flags |= PIPE_MAP_PERSISTENT;

   if (access & GL_MAP_COHERENT_BIT)
      flags |= PIPE_MAP_COHERENT;

   /* ... other flags ...
   */

   if (access & MESA_MAP_NOWAIT_BIT)
      flags |= PIPE_MAP_DONTBLOCK;
   if (access & MESA_MAP_THREAD_SAFE_BIT)
      flags |= PIPE_MAP_THREAD_SAFE;
   if (access & MESA_MAP_ONCE)
      flags |= PIPE_MAP_ONCE;

   return flags;
}


/**
 * Called via glMapBufferRange().
 */
void *
_mesa_bufferobj_map_range(struct gl_context *ctx,
                           GLintptr offset, GLsizeiptr length, GLbitfield access,
                           struct gl_buffer_object *obj,
                           gl_map_buffer_index index)
{
   struct pipe_context *pipe = ctx->pipe;

   assert(offset >= 0);
   assert(length >= 0);
   assert(offset < obj->Size);
   assert(offset + length <= obj->Size);

   enum pipe_map_flags transfer_flags =
      _mesa_access_flags_to_transfer_flags(access,
                                           offset == 0 && length == obj->Size);

   /* Sometimes games do silly things like MapBufferRange(UNSYNC|DISCARD_RANGE)
    * In this case, the the UNSYNC is a bit redundant, but the games rely
    * on the driver rebinding/replacing the backing storage rather than
    * going down the UNSYNC path (ie. honoring DISCARD_x first before UNSYNC).
    */
   if (unlikely(ctx->st_opts->ignore_map_unsynchronized)) {
      if (transfer_flags & (PIPE_MAP_DISCARD_RANGE | PIPE_MAP_DISCARD_WHOLE_RESOURCE))
         transfer_flags &= ~PIPE_MAP_UNSYNCHRONIZED;
   }

   if (ctx->Const.ForceMapBufferSynchronized)
      transfer_flags &= ~PIPE_MAP_UNSYNCHRONIZED;

   obj->Mappings[index].Pointer = pipe_buffer_map_range(pipe,
                                                        obj->buffer,
                                                        offset, length,
                                                        transfer_flags,
                                                        &obj->transfer[index]);
   if (obj->Mappings[index].Pointer) {
      obj->Mappings[index].Offset = offset;
      obj->Mappings[index].Length = length;
      obj->Mappings[index].AccessFlags = access;
   }
   else {
      obj->transfer[index] = NULL;
   }

   return obj->Mappings[index].Pointer;
}


void
_mesa_bufferobj_flush_mapped_range(struct gl_context *ctx,
                                   GLintptr offset, GLsizeiptr length,
                                   struct gl_buffer_object *obj,
                                   gl_map_buffer_index index)
{
   struct pipe_context *pipe = ctx->pipe;

   /* Subrange is relative to mapped range */
   assert(offset >= 0);
   assert(length >= 0);
   assert(offset + length <= obj->Mappings[index].Length);
   assert(obj->Mappings[index].Pointer);

   if (!length)
      return;

   pipe_buffer_flush_mapped_range(pipe, obj->transfer[index],
                                  obj->Mappings[index].Offset + offset,
                                  length);
}


/**
 * Called via glUnmapBufferARB().
 */
GLboolean
_mesa_bufferobj_unmap(struct gl_context *ctx, struct gl_buffer_object *obj,
                      gl_map_buffer_index index)
{
   struct pipe_context *pipe = ctx->pipe;

   if (obj->Mappings[index].Length)
      pipe_buffer_unmap(pipe, obj->transfer[index]);

   obj->transfer[index] = NULL;
   obj->Mappings[index].Pointer = NULL;
   obj->Mappings[index].Offset = 0;
   obj->Mappings[index].Length = 0;
   return GL_TRUE;
}


/**
 * Called via glCopyBufferSubData().
 */
static void
bufferobj_copy_subdata(struct gl_context *ctx,
                       struct gl_buffer_object *src,
                       struct gl_buffer_object *dst,
                       GLintptr readOffset, GLintptr writeOffset,
                       GLsizeiptr size)
{
   struct pipe_context *pipe = ctx->pipe;
   struct pipe_box box;

   dst->MinMaxCacheDirty = true;
   if (!size)
      return;

   /* buffer should not already be mapped */
   assert(!_mesa_check_disallowed_mapping(src));
   /* dst can be mapped, just not the same range as the target range */

   u_box_1d(readOffset, size, &box);

   pipe->resource_copy_region(pipe, dst->buffer, 0, writeOffset, 0, 0,
                              src->buffer, 0, &box);
}

static void
clear_buffer_subdata_sw(struct gl_context *ctx,
                        GLintptr offset, GLsizeiptr size,
                        const GLvoid *clearValue,
                        GLsizeiptr clearValueSize,
                        struct gl_buffer_object *bufObj)
{
   GLsizeiptr i;
   GLubyte *dest;

   dest = _mesa_bufferobj_map_range(ctx, offset, size,
                                    GL_MAP_WRITE_BIT |
                                    GL_MAP_INVALIDATE_RANGE_BIT,
                                    bufObj, MAP_INTERNAL);

   if (!dest) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glClearBuffer[Sub]Data");
      return;
   }

   if (clearValue == NULL) {
      /* Clear with zeros, per the spec */
      memset(dest, 0, size);
      _mesa_bufferobj_unmap(ctx, bufObj, MAP_INTERNAL);
      return;
   }

   for (i = 0; i < size/clearValueSize; ++i) {
      memcpy(dest, clearValue, clearValueSize);
      dest += clearValueSize;
   }

   _mesa_bufferobj_unmap(ctx, bufObj, MAP_INTERNAL);
}

/**
 * Helper to warn of possible performance issues, such as frequently
 * updating a buffer created with GL_STATIC_DRAW.  Called via the macro
 * below.
 */
static void
buffer_usage_warning(struct gl_context *ctx, GLuint *id, const char *fmt, ...)
{
   va_list args;

   va_start(args, fmt);
   _mesa_gl_vdebugf(ctx, id,
                    MESA_DEBUG_SOURCE_API,
                    MESA_DEBUG_TYPE_PERFORMANCE,
                    MESA_DEBUG_SEVERITY_MEDIUM,
                    fmt, args);
   va_end(args);
}

#define BUFFER_USAGE_WARNING(CTX, FMT, ...) \
   do { \
      static GLuint id = 0; \
      buffer_usage_warning(CTX, &id, FMT, ##__VA_ARGS__); \
   } while (0)


/**
 * Used as a placeholder for buffer objects between glGenBuffers() and
 * glBindBuffer() so that glIsBuffer() can work correctly.
 */
static struct gl_buffer_object DummyBufferObject = {
   .MinMaxCacheMutex = SIMPLE_MTX_INITIALIZER,
   .RefCount = 1000*1000*1000,  /* never delete */
};


/**
 * Return pointer to address of a buffer object target.
 * \param ctx  the GL context
 * \param target  the buffer object target to be retrieved.
 * \return   pointer to pointer to the buffer object bound to \c target in the
 *           specified context or \c NULL if \c target is invalid.
 */
static ALWAYS_INLINE struct gl_buffer_object **
get_buffer_target(struct gl_context *ctx, GLenum target, bool no_error)
{
   /* Other targets are only supported in desktop OpenGL and OpenGL ES 3.0. */
   if (!no_error && !_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx)) {
      switch (target) {
      case GL_ARRAY_BUFFER:
      case GL_ELEMENT_ARRAY_BUFFER:
      case GL_PIXEL_PACK_BUFFER:
      case GL_PIXEL_UNPACK_BUFFER:
         break;
      default:
         return NULL;
      }
   }

   switch (target) {
   case GL_ARRAY_BUFFER_ARB:
      return &ctx->Array.ArrayBufferObj;
   case GL_ELEMENT_ARRAY_BUFFER_ARB:
      return &ctx->Array.VAO->IndexBufferObj;
   case GL_PIXEL_PACK_BUFFER_EXT:
      return &ctx->Pack.BufferObj;
   case GL_PIXEL_UNPACK_BUFFER_EXT:
      return &ctx->Unpack.BufferObj;
   case GL_COPY_READ_BUFFER:
      return &ctx->CopyReadBuffer;
   case GL_COPY_WRITE_BUFFER:
      return &ctx->CopyWriteBuffer;
   case GL_QUERY_BUFFER:
      if (no_error || _mesa_has_ARB_query_buffer_object(ctx))
         return &ctx->QueryBuffer;
      break;
   case GL_DRAW_INDIRECT_BUFFER:
      if (no_error ||
          (_mesa_is_desktop_gl(ctx) && ctx->Extensions.ARB_draw_indirect) ||
           _mesa_is_gles31(ctx)) {
         return &ctx->DrawIndirectBuffer;
      }
      break;
   case GL_PARAMETER_BUFFER_ARB:
      if (no_error || _mesa_has_ARB_indirect_parameters(ctx)) {
         return &ctx->ParameterBuffer;
      }
      break;
   case GL_DISPATCH_INDIRECT_BUFFER:
      if (no_error || _mesa_has_compute_shaders(ctx)) {
         return &ctx->DispatchIndirectBuffer;
      }
      break;
   case GL_TRANSFORM_FEEDBACK_BUFFER:
      if (no_error || ctx->Extensions.EXT_transform_feedback) {
         return &ctx->TransformFeedback.CurrentBuffer;
      }
      break;
   case GL_TEXTURE_BUFFER:
      if (no_error ||
          _mesa_has_ARB_texture_buffer_object(ctx) ||
          _mesa_has_OES_texture_buffer(ctx)) {
         return &ctx->Texture.BufferObject;
      }
      break;
   case GL_UNIFORM_BUFFER:
      if (no_error || ctx->Extensions.ARB_uniform_buffer_object) {
         return &ctx->UniformBuffer;
      }
      break;
   case GL_SHADER_STORAGE_BUFFER:
      if (no_error ||
          ctx->Extensions.ARB_shader_storage_buffer_object ||
          _mesa_is_gles31(ctx)) {
         return &ctx->ShaderStorageBuffer;
      }
      break;
   case GL_ATOMIC_COUNTER_BUFFER:
      if (no_error ||
          ctx->Extensions.ARB_shader_atomic_counters || _mesa_is_gles31(ctx)) {
         return &ctx->AtomicBuffer;
      }
      break;
   case GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD:
      if (no_error || ctx->Extensions.AMD_pinned_memory) {
         return &ctx->ExternalVirtualMemoryBuffer;
      }
      break;
   }
   return NULL;
}


/**
 * Get the buffer object bound to the specified target in a GL context.
 * \param ctx  the GL context
 * \param target  the buffer object target to be retrieved.
 * \param error  the GL error to record if target is illegal.
 * \return   pointer to the buffer object bound to \c target in the
 *           specified context or \c NULL if \c target is invalid.
 */
static inline struct gl_buffer_object *
get_buffer(struct gl_context *ctx, const char *func, GLenum target,
           GLenum error)
{
   struct gl_buffer_object **bufObj = get_buffer_target(ctx, target, false);

   if (!bufObj) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(target)", func);
      return NULL;
   }

   if (!*bufObj) {
      _mesa_error(ctx, error, "%s(no buffer bound)", func);
      return NULL;
   }

   return *bufObj;
}


/**
 * Convert a GLbitfield describing the mapped buffer access flags
 * into one of GL_READ_WRITE, GL_READ_ONLY, or GL_WRITE_ONLY.
 */
static GLenum
simplified_access_mode(struct gl_context *ctx, GLbitfield access)
{
   const GLbitfield rwFlags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
   if ((access & rwFlags) == rwFlags)
      return GL_READ_WRITE;
   if ((access & GL_MAP_READ_BIT) == GL_MAP_READ_BIT)
      return GL_READ_ONLY;
   if ((access & GL_MAP_WRITE_BIT) == GL_MAP_WRITE_BIT)
      return GL_WRITE_ONLY;

   /* Otherwise, AccessFlags is zero (the default state).
    *
    * Table 2.6 on page 31 (page 44 of the PDF) of the OpenGL 1.5 spec says:
    *
    * Name           Type  Initial Value  Legal Values
    * ...            ...   ...            ...
    * BUFFER_ACCESS  enum  READ_WRITE     READ_ONLY, WRITE_ONLY
    *                                     READ_WRITE
    *
    * However, table 6.8 in the GL_OES_mapbuffer extension says:
    *
    * Get Value         Type Get Command          Value          Description
    * ---------         ---- -----------          -----          -----------
    * BUFFER_ACCESS_OES Z1   GetBufferParameteriv WRITE_ONLY_OES buffer map flag
    *
    * The difference is because GL_OES_mapbuffer only supports mapping buffers
    * write-only.
    */
   assert(access == 0);

   return _mesa_is_gles(ctx) ? GL_WRITE_ONLY : GL_READ_WRITE;
}


/**
 * Test if the buffer is mapped, and if so, if the mapped range overlaps the
 * given range.
 * The regions do not overlap if and only if the end of the given
 * region is before the mapped region or the start of the given region
 * is after the mapped region.
 *
 * \param obj     Buffer object target on which to operate.
 * \param offset  Offset of the first byte of the subdata range.
 * \param size    Size, in bytes, of the subdata range.
 * \return   true if ranges overlap, false otherwise
 *
 */
static bool
bufferobj_range_mapped(const struct gl_buffer_object *obj,
                       GLintptr offset, GLsizeiptr size)
{
   if (_mesa_bufferobj_mapped(obj, MAP_USER)) {
      const GLintptr end = offset + size;
      const GLintptr mapEnd = obj->Mappings[MAP_USER].Offset +
                              obj->Mappings[MAP_USER].Length;

      if (!(end <= obj->Mappings[MAP_USER].Offset || offset >= mapEnd)) {
         return true;
      }
   }
   return false;
}


/**
 * Tests the subdata range parameters and sets the GL error code for
 * \c glBufferSubDataARB, \c glGetBufferSubDataARB and
 * \c glClearBufferSubData.
 *
 * \param ctx     GL context.
 * \param bufObj  The buffer object.
 * \param offset  Offset of the first byte of the subdata range.
 * \param size    Size, in bytes, of the subdata range.
 * \param mappedRange  If true, checks if an overlapping range is mapped.
 *                     If false, checks if buffer is mapped.
 * \param caller  Name of calling function for recording errors.
 * \return   false if error, true otherwise
 *
 * \sa glBufferSubDataARB, glGetBufferSubDataARB, glClearBufferSubData
 */
static bool
buffer_object_subdata_range_good(struct gl_context *ctx,
                                 const struct gl_buffer_object *bufObj,
                                 GLintptr offset, GLsizeiptr size,
                                 bool mappedRange, const char *caller)
{
   if (size < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(size < 0)", caller);
      return false;
   }

   if (offset < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(offset < 0)", caller);
      return false;
   }

   if (offset + size > bufObj->Size) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(offset %lu + size %lu > buffer size %lu)", caller,
                  (unsigned long) offset,
                  (unsigned long) size,
                  (unsigned long) bufObj->Size);
      return false;
   }

   if (bufObj->Mappings[MAP_USER].AccessFlags & GL_MAP_PERSISTENT_BIT)
      return true;

   if (mappedRange) {
      if (bufferobj_range_mapped(bufObj, offset, size)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(range is mapped without persistent bit)",
                     caller);
         return false;
      }
   }
   else {
      if (_mesa_bufferobj_mapped(bufObj, MAP_USER)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(buffer is mapped without persistent bit)",
                     caller);
         return false;
      }
   }

   return true;
}


/**
 * Test the format and type parameters and set the GL error code for
 * \c glClearBufferData, \c glClearNamedBufferData, \c glClearBufferSubData
 * and \c glClearNamedBufferSubData.
 *
 * \param ctx             GL context.
 * \param internalformat  Format to which the data is to be converted.
 * \param format          Format of the supplied data.
 * \param type            Type of the supplied data.
 * \param caller          Name of calling function for recording errors.
 * \return   If internalformat, format and type are legal the mesa_format
 *           corresponding to internalformat, otherwise MESA_FORMAT_NONE.
 *
 * \sa glClearBufferData, glClearNamedBufferData, glClearBufferSubData and
 *     glClearNamedBufferSubData.
 */
static mesa_format
validate_clear_buffer_format(struct gl_context *ctx,
                             GLenum internalformat,
                             GLenum format, GLenum type,
                             const char *caller)
{
   mesa_format mesaFormat;
   GLenum errorFormatType;

   mesaFormat = _mesa_validate_texbuffer_format(ctx, internalformat);
   if (mesaFormat == MESA_FORMAT_NONE) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "%s(invalid internalformat)", caller);
      return MESA_FORMAT_NONE;
   }

   /* NOTE: not mentioned in ARB_clear_buffer_object but according to
    * EXT_texture_integer there is no conversion between integer and
    * non-integer formats
   */
   if (_mesa_is_enum_format_signed_int(format) !=
       _mesa_is_format_integer_color(mesaFormat)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(integer vs non-integer)", caller);
      return MESA_FORMAT_NONE;
   }

   if (!_mesa_is_color_format(format)) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(format is not a color format)", caller);
      return MESA_FORMAT_NONE;
   }

   errorFormatType = _mesa_error_check_format_and_type(ctx, format, type);
   if (errorFormatType != GL_NO_ERROR) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(invalid format or type)", caller);
      return MESA_FORMAT_NONE;
   }

   return mesaFormat;
}


/**
 * Convert user-specified clear value to the specified internal format.
 *
 * \param ctx             GL context.
 * \param internalformat  Format to which the data is converted.
 * \param clearValue      Points to the converted clear value.
 * \param format          Format of the supplied data.
 * \param type            Type of the supplied data.
 * \param data            Data which is to be converted to internalformat.
 * \param caller          Name of calling function for recording errors.
 * \return   true if data could be converted, false otherwise.
 *
 * \sa glClearBufferData, glClearBufferSubData
 */
static bool
convert_clear_buffer_data(struct gl_context *ctx,
                          mesa_format internalformat,
                          GLubyte *clearValue, GLenum format, GLenum type,
                          const GLvoid *data, const char *caller)
{
   GLenum internalformatBase = _mesa_get_format_base_format(internalformat);

   if (_mesa_texstore(ctx, 1, internalformatBase, internalformat,
                      0, &clearValue, 1, 1, 1,
                      format, type, data, &ctx->Unpack)) {
      return true;
   }
   else {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s", caller);
      return false;
   }
}

void
_mesa_bufferobj_release_buffer(struct gl_buffer_object *obj)
{
   if (!obj->buffer)
      return;

   /* Subtract the remaining private references before unreferencing
    * the buffer. See the header file for explanation.
    */
   if (obj->private_refcount) {
      assert(obj->private_refcount > 0);
      p_atomic_add(&obj->buffer->reference.count,
                   -obj->private_refcount);
      obj->private_refcount = 0;
   }
   obj->private_refcount_ctx = NULL;

   pipe_resource_reference(&obj->buffer, NULL);
}

/**
 * Delete a buffer object.
 *
 * Default callback for the \c dd_function_table::DeleteBuffer() hook.
 */
void
_mesa_delete_buffer_object(struct gl_context *ctx,
                           struct gl_buffer_object *bufObj)
{
   assert(bufObj->RefCount == 0);
   _mesa_buffer_unmap_all_mappings(ctx, bufObj);
   _mesa_bufferobj_release_buffer(bufObj);

   vbo_delete_minmax_cache(bufObj);

   /* assign strange values here to help w/ debugging */
   bufObj->RefCount = -1000;
   bufObj->Name = ~0;

   simple_mtx_destroy(&bufObj->MinMaxCacheMutex);
   free(bufObj->Label);
   free(bufObj);
}


/**
 * Get the value of MESA_NO_MINMAX_CACHE.
 */
static bool
get_no_minmax_cache()
{
   static bool read = false;
   static bool disable = false;

   if (!read) {
      disable = debug_get_bool_option("MESA_NO_MINMAX_CACHE", false);
      read = true;
   }

   return disable;
}

/**
 * Callback called from _mesa_HashWalk()
 */
static void
count_buffer_size(void *data, void *userData)
{
   const struct gl_buffer_object *bufObj =
      (const struct gl_buffer_object *) data;
   GLuint *total = (GLuint *) userData;

   *total = *total + bufObj->Size;
}


/**
 * Compute total size (in bytes) of all buffer objects for the given context.
 * For debugging purposes.
 */
GLuint
_mesa_total_buffer_object_memory(struct gl_context *ctx)
{
   GLuint total = 0;

   _mesa_HashWalkMaybeLocked(ctx->Shared->BufferObjects, count_buffer_size,
                             &total, ctx->BufferObjectsLocked);

   return total;
}

/**
 * Initialize the state associated with buffer objects
 */
void
_mesa_init_buffer_objects( struct gl_context *ctx )
{
   GLuint i;

   for (i = 0; i < MAX_COMBINED_UNIFORM_BUFFERS; i++) {
      _mesa_reference_buffer_object(ctx,
				    &ctx->UniformBufferBindings[i].BufferObject,
				    NULL);
      ctx->UniformBufferBindings[i].Offset = -1;
      ctx->UniformBufferBindings[i].Size = -1;
   }

   for (i = 0; i < MAX_COMBINED_SHADER_STORAGE_BUFFERS; i++) {
      _mesa_reference_buffer_object(ctx,
                                    &ctx->ShaderStorageBufferBindings[i].BufferObject,
                                    NULL);
      ctx->ShaderStorageBufferBindings[i].Offset = -1;
      ctx->ShaderStorageBufferBindings[i].Size = -1;
   }

   for (i = 0; i < MAX_COMBINED_ATOMIC_BUFFERS; i++) {
      _mesa_reference_buffer_object(ctx,
				    &ctx->AtomicBufferBindings[i].BufferObject,
				    NULL);
      ctx->AtomicBufferBindings[i].Offset = 0;
      ctx->AtomicBufferBindings[i].Size = 0;
   }
}

/**
 * Detach the context from the buffer to re-enable buffer reference counting
 * for this context.
 */
static void
detach_ctx_from_buffer(struct gl_context *ctx, struct gl_buffer_object *buf)
{
   assert(buf->Ctx == ctx);

   /* Move private non-atomic context references to the global ref count. */
   p_atomic_add(&buf->RefCount, buf->CtxRefCount);
   buf->CtxRefCount = 0;
   buf->Ctx = NULL;

   /* Remove the context reference where the context holds one
    * reference for the lifetime of the buffer ID to skip refcount
    * atomics instead of each binding point holding the reference.
    */
   _mesa_reference_buffer_object(ctx, &buf, NULL);
}

/**
 * Zombie buffers are buffers that were created by one context and deleted
 * by another context. The creating context holds a global reference for each
 * buffer it created that can't be unreferenced when another context deletes
 * it. Such a buffer becomes a zombie, which means that it's no longer usable
 * by OpenGL, but the creating context still holds its global reference of
 * the buffer. Only the creating context can remove the reference, which is
 * what this function does.
 *
 * For all zombie buffers, decrement the reference count if the current
 * context owns the buffer.
 */
static void
unreference_zombie_buffers_for_ctx(struct gl_context *ctx)
{
   /* It's assumed that the mutex of Shared->BufferObjects is locked. */
   set_foreach(ctx->Shared->ZombieBufferObjects, entry) {
      struct gl_buffer_object *buf = (struct gl_buffer_object *)entry->key;

      if (buf->Ctx == ctx) {
         _mesa_set_remove(ctx->Shared->ZombieBufferObjects, entry);
         detach_ctx_from_buffer(ctx, buf);
      }
   }
}

/**
 * When a context creates buffers, it holds a global buffer reference count
 * for each buffer and doesn't update their RefCount. When the context is
 * destroyed before the buffers are destroyed, the context must remove
 * its global reference from the buffers, so that the buffers can live
 * on their own.
 *
 * At this point, the buffers shouldn't be bound in any bounding point owned
 * by the context. (it would crash if they did)
 */
static void
detach_unrefcounted_buffer_from_ctx(void *data, void *userData)
{
   struct gl_context *ctx = (struct gl_context *)userData;
   struct gl_buffer_object *buf = (struct gl_buffer_object *)data;

   if (buf->Ctx == ctx) {
      /* Detach the current context from live objects. There should be no
       * bound buffer in the context at this point, therefore we can just
       * unreference the global reference. Other contexts and texture objects
       * might still be using the buffer.
       */
      assert(buf->CtxRefCount == 0);
      buf->Ctx = NULL;
      _mesa_reference_buffer_object(ctx, &buf, NULL);
   }
}

void
_mesa_free_buffer_objects( struct gl_context *ctx )
{
   GLuint i;

   _mesa_reference_buffer_object(ctx, &ctx->Array.ArrayBufferObj, NULL);

   _mesa_reference_buffer_object(ctx, &ctx->CopyReadBuffer, NULL);
   _mesa_reference_buffer_object(ctx, &ctx->CopyWriteBuffer, NULL);

   _mesa_reference_buffer_object(ctx, &ctx->UniformBuffer, NULL);

   _mesa_reference_buffer_object(ctx, &ctx->ShaderStorageBuffer, NULL);

   _mesa_reference_buffer_object(ctx, &ctx->AtomicBuffer, NULL);

   _mesa_reference_buffer_object(ctx, &ctx->DrawIndirectBuffer, NULL);

   _mesa_reference_buffer_object(ctx, &ctx->ParameterBuffer, NULL);

   _mesa_reference_buffer_object(ctx, &ctx->DispatchIndirectBuffer, NULL);

   _mesa_reference_buffer_object(ctx, &ctx->QueryBuffer, NULL);

   for (i = 0; i < MAX_COMBINED_UNIFORM_BUFFERS; i++) {
      _mesa_reference_buffer_object(ctx,
				    &ctx->UniformBufferBindings[i].BufferObject,
				    NULL);
   }

   for (i = 0; i < MAX_COMBINED_SHADER_STORAGE_BUFFERS; i++) {
      _mesa_reference_buffer_object(ctx,
                                    &ctx->ShaderStorageBufferBindings[i].BufferObject,
                                    NULL);
   }

   for (i = 0; i < MAX_COMBINED_ATOMIC_BUFFERS; i++) {
      _mesa_reference_buffer_object(ctx,
				    &ctx->AtomicBufferBindings[i].BufferObject,
				    NULL);
   }

   _mesa_HashLockMutex(ctx->Shared->BufferObjects);
   unreference_zombie_buffers_for_ctx(ctx);
   _mesa_HashWalkLocked(ctx->Shared->BufferObjects,
                        detach_unrefcounted_buffer_from_ctx, ctx);
   _mesa_HashUnlockMutex(ctx->Shared->BufferObjects);
}

struct gl_buffer_object *
_mesa_bufferobj_alloc(struct gl_context *ctx, GLuint id)
{
   struct gl_buffer_object *buf = CALLOC_STRUCT(gl_buffer_object);
   if (!buf)
      return NULL;

   buf->RefCount = 1;
   buf->Name = id;
   buf->Usage = GL_STATIC_DRAW_ARB;

   simple_mtx_init(&buf->MinMaxCacheMutex, mtx_plain);
   if (get_no_minmax_cache())
      buf->UsageHistory |= USAGE_DISABLE_MINMAX_CACHE;
   return buf;
}
/**
 * Create a buffer object that will be backed by an OpenGL buffer ID
 * where the creating context will hold one global buffer reference instead
 * of updating buffer RefCount for every binding point.
 *
 * This shouldn't be used for internal buffers.
 */
static struct gl_buffer_object *
new_gl_buffer_object(struct gl_context *ctx, GLuint id)
{
   struct gl_buffer_object *buf = _mesa_bufferobj_alloc(ctx, id);

   buf->Ctx = ctx;
   buf->RefCount++; /* global buffer reference held by the context */
   return buf;
}

static ALWAYS_INLINE bool
handle_bind_buffer_gen(struct gl_context *ctx,
                       GLuint buffer,
                       struct gl_buffer_object **buf_handle,
                       const char *caller, bool no_error)
{
   struct gl_buffer_object *buf = *buf_handle;

   if (unlikely(!no_error && !buf && _mesa_is_desktop_gl_core(ctx))) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(non-gen name)", caller);
      return false;
   }

   if (unlikely(!buf || buf == &DummyBufferObject)) {
      /* If this is a new buffer object id, or one which was generated but
       * never used before, allocate a buffer object now.
       */
      *buf_handle = new_gl_buffer_object(ctx, buffer);
      if (!no_error && !*buf_handle) {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s", caller);
         return false;
      }
      _mesa_HashLockMaybeLocked(ctx->Shared->BufferObjects,
                                ctx->BufferObjectsLocked);
      _mesa_HashInsertLocked(ctx->Shared->BufferObjects, buffer,
                             *buf_handle, buf != NULL);
      /* If one context only creates buffers and another context only deletes
       * buffers, buffers don't get released because it only produces zombie
       * buffers. Only the context that has created the buffers can release
       * them. Thus, when we create buffers, we prune the list of zombie
       * buffers.
       */
      unreference_zombie_buffers_for_ctx(ctx);
      _mesa_HashUnlockMaybeLocked(ctx->Shared->BufferObjects,
                                  ctx->BufferObjectsLocked);
   }

   return true;
}

bool
_mesa_handle_bind_buffer_gen(struct gl_context *ctx,
                             GLuint buffer,
                             struct gl_buffer_object **buf_handle,
                             const char *caller, bool no_error)
{
   return handle_bind_buffer_gen(ctx, buffer, buf_handle, caller, no_error);
}

/**
 * Bind the specified target to buffer for the specified context.
 * Called by glBindBuffer() and other functions.
 */
static void
bind_buffer_object(struct gl_context *ctx,
                   struct gl_buffer_object **bindTarget, GLuint buffer,
                   bool no_error)
{
   struct gl_buffer_object *oldBufObj;
   struct gl_buffer_object *newBufObj;

   assert(bindTarget);

   /* Fast path that unbinds. It's better when NULL is a literal, so that
    * the compiler can simplify this code after inlining.
    */
   if (buffer == 0) {
      _mesa_reference_buffer_object(ctx, bindTarget, NULL);
      return;
   }

   /* Get pointer to old buffer object (to be unbound) */
   oldBufObj = *bindTarget;
   GLuint old_name = oldBufObj && !oldBufObj->DeletePending ? oldBufObj->Name : 0;
   if (unlikely(old_name == buffer))
      return;   /* rebinding the same buffer object- no change */

   newBufObj = _mesa_lookup_bufferobj(ctx, buffer);
   /* Get a new buffer object if it hasn't been created. */
   if (unlikely(!handle_bind_buffer_gen(ctx, buffer, &newBufObj, "glBindBuffer",
                                        no_error)))
      return;

   /* At this point, the compiler should deduce that newBufObj is non-NULL if
    * everything has been inlined, so the compiler should simplify this.
    */
   _mesa_reference_buffer_object(ctx, bindTarget, newBufObj);
}


/**
 * Update the default buffer objects in the given context to reference those
 * specified in the shared state and release those referencing the old
 * shared state.
 */
void
_mesa_update_default_objects_buffer_objects(struct gl_context *ctx)
{
   /* Bind 0 to remove references to those in the shared context hash table. */
   bind_buffer_object(ctx, &ctx->Array.ArrayBufferObj, 0, false);
   bind_buffer_object(ctx, &ctx->Array.VAO->IndexBufferObj, 0, false);
   bind_buffer_object(ctx, &ctx->Pack.BufferObj, 0, false);
   bind_buffer_object(ctx, &ctx->Unpack.BufferObj, 0, false);
}



/**
 * Return the gl_buffer_object for the given ID.
 * Always return NULL for ID 0.
 */
struct gl_buffer_object *
_mesa_lookup_bufferobj(struct gl_context *ctx, GLuint buffer)
{
   if (buffer == 0)
      return NULL;
   else
      return (struct gl_buffer_object *)
         _mesa_HashLookupMaybeLocked(ctx->Shared->BufferObjects, buffer,
                                     ctx->BufferObjectsLocked);
}


struct gl_buffer_object *
_mesa_lookup_bufferobj_locked(struct gl_context *ctx, GLuint buffer)
{
   if (buffer == 0)
      return NULL;
   else
      return (struct gl_buffer_object *)
         _mesa_HashLookupLocked(ctx->Shared->BufferObjects, buffer);
}

/**
 * A convenience function for direct state access functions that throws
 * GL_INVALID_OPERATION if buffer is not the name of an existing
 * buffer object.
 */
struct gl_buffer_object *
_mesa_lookup_bufferobj_err(struct gl_context *ctx, GLuint buffer,
                           const char *caller)
{
   struct gl_buffer_object *bufObj;

   bufObj = _mesa_lookup_bufferobj(ctx, buffer);
   if (!bufObj || bufObj == &DummyBufferObject) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(non-existent buffer object %u)", caller, buffer);
      return NULL;
   }

   return bufObj;
}


/**
 * Look up a buffer object for a multi-bind function.
 *
 * Unlike _mesa_lookup_bufferobj(), this function also takes care
 * of generating an error if the buffer ID is not zero or the name
 * of an existing buffer object.
 *
 * If the buffer ID refers to an existing buffer object, a pointer
 * to the buffer object is returned.  If the ID is zero, NULL is returned.
 * If the ID is not zero and does not refer to a valid buffer object, this
 * function returns NULL.
 *
 * This function assumes that the caller has already locked the
 * hash table mutex by calling
 * _mesa_HashLockMutex(ctx->Shared->BufferObjects).
 */
struct gl_buffer_object *
_mesa_multi_bind_lookup_bufferobj(struct gl_context *ctx,
                                  const GLuint *buffers,
                                  GLuint index, const char *caller,
                                  bool *error)
{
   struct gl_buffer_object *bufObj = NULL;

   *error = false;

   if (buffers[index] != 0) {
      bufObj = _mesa_lookup_bufferobj_locked(ctx, buffers[index]);

      /* The multi-bind functions don't create the buffer objects
         when they don't exist. */
      if (bufObj == &DummyBufferObject)
         bufObj = NULL;

      if (!bufObj) {
         /* The ARB_multi_bind spec says:
          *
          *    "An INVALID_OPERATION error is generated if any value
          *     in <buffers> is not zero or the name of an existing
          *     buffer object (per binding)."
          */
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(buffers[%u]=%u is not zero or the name "
                     "of an existing buffer object)",
                     caller, index, buffers[index]);
         *error = true;
      }
   }

   return bufObj;
}


/**
 * If *ptr points to obj, set ptr = the Null/default buffer object.
 * This is a helper for buffer object deletion.
 * The GL spec says that deleting a buffer object causes it to get
 * unbound from all arrays in the current context.
 */
static void
unbind(struct gl_context *ctx,
       struct gl_vertex_array_object *vao, unsigned index,
       struct gl_buffer_object *obj)
{
   if (vao->BufferBinding[index].BufferObj == obj) {
      _mesa_bind_vertex_buffer(ctx, vao, index, NULL,
                               vao->BufferBinding[index].Offset,
                               vao->BufferBinding[index].Stride, true, false);
   }
}

void
_mesa_buffer_unmap_all_mappings(struct gl_context *ctx,
                                struct gl_buffer_object *bufObj)
{
   for (int i = 0; i < MAP_COUNT; i++) {
      if (_mesa_bufferobj_mapped(bufObj, i)) {
         _mesa_bufferobj_unmap(ctx, bufObj, i);
         assert(bufObj->Mappings[i].Pointer == NULL);
         bufObj->Mappings[i].AccessFlags = 0;
      }
   }
}


/**********************************************************************/
/* API Functions                                                      */
/**********************************************************************/

void GLAPIENTRY
_mesa_BindBuffer_no_error(GLenum target, GLuint buffer)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_buffer_object **bindTarget = get_buffer_target(ctx, target, true);
   bind_buffer_object(ctx, bindTarget, buffer, true);
}


void GLAPIENTRY
_mesa_BindBuffer(GLenum target, GLuint buffer)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API) {
      _mesa_debug(ctx, "glBindBuffer(%s, %u)\n",
                  _mesa_enum_to_string(target), buffer);
   }

   struct gl_buffer_object **bindTarget = get_buffer_target(ctx, target, false);
   if (!bindTarget) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glBindBufferARB(target %s)",
                  _mesa_enum_to_string(target));
      return;
   }

   bind_buffer_object(ctx, bindTarget, buffer, false);
}

/**
 * Binds a buffer object to a binding point.
 *
 * The caller is responsible for validating the offset,
 * flushing the vertices and updating NewDriverState.
 */
static void
set_buffer_binding(struct gl_context *ctx,
                   struct gl_buffer_binding *binding,
                   struct gl_buffer_object *bufObj,
                   GLintptr offset,
                   GLsizeiptr size,
                   bool autoSize, gl_buffer_usage usage)
{
   _mesa_reference_buffer_object(ctx, &binding->BufferObject, bufObj);

   binding->Offset = offset;
   binding->Size = size;
   binding->AutomaticSize = autoSize;

   /* If this is a real buffer object, mark it has having been used
    * at some point as an atomic counter buffer.
    */
   if (size >= 0)
      bufObj->UsageHistory |= usage;
}

static void
set_buffer_multi_binding(struct gl_context *ctx,
                         const GLuint *buffers,
                         int idx,
                         const char *caller,
                         struct gl_buffer_binding *binding,
                         GLintptr offset,
                         GLsizeiptr size,
                         bool range,
                         gl_buffer_usage usage)
{
   struct gl_buffer_object *bufObj;

   if (binding->BufferObject && binding->BufferObject->Name == buffers[idx])
      bufObj = binding->BufferObject;
   else {
      bool error;
      bufObj = _mesa_multi_bind_lookup_bufferobj(ctx, buffers, idx, caller,
                                                 &error);
      if (error)
         return;
   }

   if (!bufObj)
      set_buffer_binding(ctx, binding, bufObj, -1, -1, !range, usage);
   else
      set_buffer_binding(ctx, binding, bufObj, offset, size, !range, usage);
}

static void
bind_buffer(struct gl_context *ctx,
            struct gl_buffer_binding *binding,
            struct gl_buffer_object *bufObj,
            GLintptr offset,
            GLsizeiptr size,
            GLboolean autoSize,
            uint64_t driver_state,
            gl_buffer_usage usage)
{
   if (binding->BufferObject == bufObj &&
       binding->Offset == offset &&
       binding->Size == size &&
       binding->AutomaticSize == autoSize) {
      return;
   }

   FLUSH_VERTICES(ctx, 0, 0);
   ctx->NewDriverState |= driver_state;

   set_buffer_binding(ctx, binding, bufObj, offset, size, autoSize, usage);
}

/**
 * Binds a buffer object to a uniform buffer binding point.
 *
 * Unlike set_buffer_binding(), this function also flushes vertices
 * and updates NewDriverState.  It also checks if the binding
 * has actually changed before updating it.
 */
static void
bind_uniform_buffer(struct gl_context *ctx,
                    GLuint index,
                    struct gl_buffer_object *bufObj,
                    GLintptr offset,
                    GLsizeiptr size,
                    GLboolean autoSize)
{
   bind_buffer(ctx, &ctx->UniformBufferBindings[index],
               bufObj, offset, size, autoSize,
               ST_NEW_UNIFORM_BUFFER,
               USAGE_UNIFORM_BUFFER);
}

/**
 * Binds a buffer object to a shader storage buffer binding point.
 *
 * Unlike set_ssbo_binding(), this function also flushes vertices
 * and updates NewDriverState.  It also checks if the binding
 * has actually changed before updating it.
 */
static void
bind_shader_storage_buffer(struct gl_context *ctx,
                           GLuint index,
                           struct gl_buffer_object *bufObj,
                           GLintptr offset,
                           GLsizeiptr size,
                           GLboolean autoSize)
{
   bind_buffer(ctx, &ctx->ShaderStorageBufferBindings[index],
               bufObj, offset, size, autoSize,
               ST_NEW_STORAGE_BUFFER,
               USAGE_SHADER_STORAGE_BUFFER);
}

/**
 * Binds a buffer object to an atomic buffer binding point.
 *
 * Unlike set_atomic_binding(), this function also flushes vertices
 * and updates NewDriverState.  It also checks if the binding
 * has actually changed before updating it.
 */
static void
bind_atomic_buffer(struct gl_context *ctx, unsigned index,
                   struct gl_buffer_object *bufObj, GLintptr offset,
                   GLsizeiptr size, GLboolean autoSize)
{
   bind_buffer(ctx, &ctx->AtomicBufferBindings[index],
               bufObj, offset, size, autoSize,
               ctx->DriverFlags.NewAtomicBuffer,
               USAGE_ATOMIC_COUNTER_BUFFER);
}

/**
 * Bind a buffer object to a uniform block binding point.
 * As above, but offset = 0.
 */
static void
bind_buffer_base_uniform_buffer(struct gl_context *ctx,
				GLuint index,
				struct gl_buffer_object *bufObj)
{
   if (index >= ctx->Const.MaxUniformBufferBindings) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glBindBufferBase(index=%d)", index);
      return;
   }

   _mesa_reference_buffer_object(ctx, &ctx->UniformBuffer, bufObj);

   if (!bufObj)
      bind_uniform_buffer(ctx, index, bufObj, -1, -1, GL_TRUE);
   else
      bind_uniform_buffer(ctx, index, bufObj, 0, 0, GL_TRUE);
}

/**
 * Bind a buffer object to a shader storage block binding point.
 * As above, but offset = 0.
 */
static void
bind_buffer_base_shader_storage_buffer(struct gl_context *ctx,
                                       GLuint index,
                                       struct gl_buffer_object *bufObj)
{
   if (index >= ctx->Const.MaxShaderStorageBufferBindings) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glBindBufferBase(index=%d)", index);
      return;
   }

   _mesa_reference_buffer_object(ctx, &ctx->ShaderStorageBuffer, bufObj);

   if (!bufObj)
      bind_shader_storage_buffer(ctx, index, bufObj, -1, -1, GL_TRUE);
   else
      bind_shader_storage_buffer(ctx, index, bufObj, 0, 0, GL_TRUE);
}

/**
 * Bind a buffer object to a shader storage block binding point.
 * As above, but offset = 0.
 */
static void
bind_buffer_base_atomic_buffer(struct gl_context *ctx,
                               GLuint index,
                               struct gl_buffer_object *bufObj)
{
   if (index >= ctx->Const.MaxAtomicBufferBindings) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glBindBufferBase(index=%d)", index);
      return;
   }

   _mesa_reference_buffer_object(ctx, &ctx->AtomicBuffer, bufObj);

   if (!bufObj)
      bind_atomic_buffer(ctx, index, bufObj, -1, -1, GL_TRUE);
   else
      bind_atomic_buffer(ctx, index, bufObj, 0, 0, GL_TRUE);
}

/**
 * Delete a set of buffer objects.
 *
 * \param n      Number of buffer objects to delete.
 * \param ids    Array of \c n buffer object IDs.
 */
static void
delete_buffers(struct gl_context *ctx, GLsizei n, const GLuint *ids)
{
   FLUSH_VERTICES(ctx, 0, 0);

   _mesa_HashLockMaybeLocked(ctx->Shared->BufferObjects,
                             ctx->BufferObjectsLocked);
   unreference_zombie_buffers_for_ctx(ctx);

   for (GLsizei i = 0; i < n; i++) {
      struct gl_buffer_object *bufObj =
         _mesa_lookup_bufferobj_locked(ctx, ids[i]);
      if (bufObj) {
         struct gl_vertex_array_object *vao = ctx->Array.VAO;
         GLuint j;

         assert(bufObj->Name == ids[i] || bufObj == &DummyBufferObject);

         _mesa_buffer_unmap_all_mappings(ctx, bufObj);

         /* unbind any vertex pointers bound to this buffer */
         for (j = 0; j < ARRAY_SIZE(vao->BufferBinding); j++) {
            unbind(ctx, vao, j, bufObj);
         }

         if (ctx->Array.ArrayBufferObj == bufObj) {
            bind_buffer_object(ctx, &ctx->Array.ArrayBufferObj, 0, false);
         }
         if (vao->IndexBufferObj == bufObj) {
            bind_buffer_object(ctx, &vao->IndexBufferObj, 0, false);
         }

         /* unbind ARB_draw_indirect binding point */
         if (ctx->DrawIndirectBuffer == bufObj) {
            bind_buffer_object(ctx, &ctx->DrawIndirectBuffer, 0, false);
         }

         /* unbind ARB_indirect_parameters binding point */
         if (ctx->ParameterBuffer == bufObj) {
            bind_buffer_object(ctx, &ctx->ParameterBuffer, 0, false);
         }

         /* unbind ARB_compute_shader binding point */
         if (ctx->DispatchIndirectBuffer == bufObj) {
            bind_buffer_object(ctx, &ctx->DispatchIndirectBuffer, 0, false);
         }

         /* unbind ARB_copy_buffer binding points */
         if (ctx->CopyReadBuffer == bufObj) {
            bind_buffer_object(ctx, &ctx->CopyReadBuffer, 0, false);
         }
         if (ctx->CopyWriteBuffer == bufObj) {
            bind_buffer_object(ctx, &ctx->CopyWriteBuffer, 0, false);
         }

         /* unbind transform feedback binding points */
         if (ctx->TransformFeedback.CurrentBuffer == bufObj) {
            bind_buffer_object(ctx, &ctx->TransformFeedback.CurrentBuffer, 0, false);
         }
         for (j = 0; j < MAX_FEEDBACK_BUFFERS; j++) {
            if (ctx->TransformFeedback.CurrentObject->Buffers[j] == bufObj) {
               _mesa_bind_buffer_base_transform_feedback(ctx,
                                           ctx->TransformFeedback.CurrentObject,
                                           j, NULL, false);
            }
         }

         /* unbind UBO binding points */
         for (j = 0; j < ctx->Const.MaxUniformBufferBindings; j++) {
            if (ctx->UniformBufferBindings[j].BufferObject == bufObj) {
               bind_buffer_base_uniform_buffer(ctx, j, NULL);
            }
         }

         if (ctx->UniformBuffer == bufObj) {
            bind_buffer_object(ctx, &ctx->UniformBuffer, 0, false);
         }

         /* unbind SSBO binding points */
         for (j = 0; j < ctx->Const.MaxShaderStorageBufferBindings; j++) {
            if (ctx->ShaderStorageBufferBindings[j].BufferObject == bufObj) {
               bind_buffer_base_shader_storage_buffer(ctx, j, NULL);
            }
         }

         if (ctx->ShaderStorageBuffer == bufObj) {
            bind_buffer_object(ctx, &ctx->ShaderStorageBuffer, 0, false);
         }

         /* unbind Atomci Buffer binding points */
         for (j = 0; j < ctx->Const.MaxAtomicBufferBindings; j++) {
            if (ctx->AtomicBufferBindings[j].BufferObject == bufObj) {
               bind_buffer_base_atomic_buffer(ctx, j, NULL);
            }
         }

         if (ctx->AtomicBuffer == bufObj) {
            bind_buffer_object(ctx, &ctx->AtomicBuffer, 0, false);
         }

         /* unbind any pixel pack/unpack pointers bound to this buffer */
         if (ctx->Pack.BufferObj == bufObj) {
            bind_buffer_object(ctx, &ctx->Pack.BufferObj, 0, false);
         }
         if (ctx->Unpack.BufferObj == bufObj) {
            bind_buffer_object(ctx, &ctx->Unpack.BufferObj, 0, false);
         }

         if (ctx->Texture.BufferObject == bufObj) {
            bind_buffer_object(ctx, &ctx->Texture.BufferObject, 0, false);
         }

         if (ctx->ExternalVirtualMemoryBuffer == bufObj) {
            bind_buffer_object(ctx, &ctx->ExternalVirtualMemoryBuffer, 0, false);
         }

         /* unbind query buffer binding point */
         if (ctx->QueryBuffer == bufObj) {
            bind_buffer_object(ctx, &ctx->QueryBuffer, 0, false);
         }

         /* The ID is immediately freed for re-use */
         _mesa_HashRemoveLocked(ctx->Shared->BufferObjects, ids[i]);
         /* Make sure we do not run into the classic ABA problem on bind.
          * We don't want to allow re-binding a buffer object that's been
          * "deleted" by glDeleteBuffers().
          *
          * The explicit rebinding to the default object in the current context
          * prevents the above in the current context, but another context
          * sharing the same objects might suffer from this problem.
          * The alternative would be to do the hash lookup in any case on bind
          * which would introduce more runtime overhead than this.
          */
         bufObj->DeletePending = GL_TRUE;

         /* The GLuint ID holds one reference and the context that created
          * the buffer holds the other one.
          */
         assert(p_atomic_read(&bufObj->RefCount) >= (bufObj->Ctx ? 2 : 1));

         if (bufObj->Ctx == ctx) {
            detach_ctx_from_buffer(ctx, bufObj);
         } else if (bufObj->Ctx) {
            /* Only the context holding it can release it. */
            _mesa_set_add(ctx->Shared->ZombieBufferObjects, bufObj);
         }

         _mesa_reference_buffer_object(ctx, &bufObj, NULL);
      }
   }

   _mesa_HashUnlockMaybeLocked(ctx->Shared->BufferObjects,
                               ctx->BufferObjectsLocked);
}


void GLAPIENTRY
_mesa_DeleteBuffers_no_error(GLsizei n, const GLuint *ids)
{
   GET_CURRENT_CONTEXT(ctx);
   delete_buffers(ctx, n, ids);
}


void GLAPIENTRY
_mesa_DeleteBuffers(GLsizei n, const GLuint *ids)
{
   GET_CURRENT_CONTEXT(ctx);

   if (n < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glDeleteBuffersARB(n)");
      return;
   }

   delete_buffers(ctx, n, ids);
}


/**
 * This is the implementation for glGenBuffers and glCreateBuffers. It is not
 * exposed to the rest of Mesa to encourage the use of nameless buffers in
 * driver internals.
 */
static void
create_buffers(struct gl_context *ctx, GLsizei n, GLuint *buffers, bool dsa)
{
   struct gl_buffer_object *buf;

   if (!buffers)
      return;

   /*
    * This must be atomic (generation and allocation of buffer object IDs)
    */
   _mesa_HashLockMaybeLocked(ctx->Shared->BufferObjects,
                             ctx->BufferObjectsLocked);
   /* If one context only creates buffers and another context only deletes
    * buffers, buffers don't get released because it only produces zombie
    * buffers. Only the context that has created the buffers can release
    * them. Thus, when we create buffers, we prune the list of zombie
    * buffers.
    */
   unreference_zombie_buffers_for_ctx(ctx);

   _mesa_HashFindFreeKeys(ctx->Shared->BufferObjects, buffers, n);

   /* Insert the ID and pointer into the hash table. If non-DSA, insert a
    * DummyBufferObject.  Otherwise, create a new buffer object and insert
    * it.
    */
   for (int i = 0; i < n; i++) {
      if (dsa) {
         buf = new_gl_buffer_object(ctx, buffers[i]);
         if (!buf) {
            _mesa_error(ctx, GL_OUT_OF_MEMORY, "glCreateBuffers");
            _mesa_HashUnlockMaybeLocked(ctx->Shared->BufferObjects,
                                        ctx->BufferObjectsLocked);
            return;
         }
      }
      else
         buf = &DummyBufferObject;

      _mesa_HashInsertLocked(ctx->Shared->BufferObjects, buffers[i], buf, true);
   }

   _mesa_HashUnlockMaybeLocked(ctx->Shared->BufferObjects,
                               ctx->BufferObjectsLocked);
}


static void
create_buffers_err(struct gl_context *ctx, GLsizei n, GLuint *buffers, bool dsa)
{
   const char *func = dsa ? "glCreateBuffers" : "glGenBuffers";

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "%s(%d)\n", func, n);

   if (n < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(n %d < 0)", func, n);
      return;
   }

   create_buffers(ctx, n, buffers, dsa);
}

/**
 * Generate a set of unique buffer object IDs and store them in \c buffers.
 *
 * \param n        Number of IDs to generate.
 * \param buffers  Array of \c n locations to store the IDs.
 */
void GLAPIENTRY
_mesa_GenBuffers_no_error(GLsizei n, GLuint *buffers)
{
   GET_CURRENT_CONTEXT(ctx);
   create_buffers(ctx, n, buffers, false);
}


void GLAPIENTRY
_mesa_GenBuffers(GLsizei n, GLuint *buffers)
{
   GET_CURRENT_CONTEXT(ctx);
   create_buffers_err(ctx, n, buffers, false);
}

/**
 * Create a set of buffer objects and store their unique IDs in \c buffers.
 *
 * \param n        Number of IDs to generate.
 * \param buffers  Array of \c n locations to store the IDs.
 */
void GLAPIENTRY
_mesa_CreateBuffers_no_error(GLsizei n, GLuint *buffers)
{
   GET_CURRENT_CONTEXT(ctx);
   create_buffers(ctx, n, buffers, true);
}


void GLAPIENTRY
_mesa_CreateBuffers(GLsizei n, GLuint *buffers)
{
   GET_CURRENT_CONTEXT(ctx);
   create_buffers_err(ctx, n, buffers, true);
}


/**
 * Determine if ID is the name of a buffer object.
 *
 * \param id  ID of the potential buffer object.
 * \return  \c GL_TRUE if \c id is the name of a buffer object,
 *          \c GL_FALSE otherwise.
 */
GLboolean GLAPIENTRY
_mesa_IsBuffer(GLuint id)
{
   struct gl_buffer_object *bufObj;
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_WITH_RETVAL(ctx, GL_FALSE);

   bufObj = _mesa_lookup_bufferobj(ctx, id);

   return bufObj && bufObj != &DummyBufferObject;
}


static bool
validate_buffer_storage(struct gl_context *ctx,
                        struct gl_buffer_object *bufObj, GLsizeiptr size,
                        GLbitfield flags, const char *func)
{
   if (size <= 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(size <= 0)", func);
      return false;
   }

   GLbitfield valid_flags = GL_MAP_READ_BIT |
                            GL_MAP_WRITE_BIT |
                            GL_MAP_PERSISTENT_BIT |
                            GL_MAP_COHERENT_BIT |
                            GL_DYNAMIC_STORAGE_BIT |
                            GL_CLIENT_STORAGE_BIT;

   if (ctx->Extensions.ARB_sparse_buffer)
      valid_flags |= GL_SPARSE_STORAGE_BIT_ARB;

   if (flags & ~valid_flags) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(invalid flag bits set)", func);
      return false;
   }

   /* The Errors section of the GL_ARB_sparse_buffer spec says:
    *
    *    "INVALID_VALUE is generated by BufferStorage if <flags> contains
    *     SPARSE_STORAGE_BIT_ARB and <flags> also contains any combination of
    *     MAP_READ_BIT or MAP_WRITE_BIT."
    */
   if (flags & GL_SPARSE_STORAGE_BIT_ARB &&
       flags & (GL_MAP_READ_BIT | GL_MAP_WRITE_BIT)) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(SPARSE_STORAGE and READ/WRITE)", func);
      return false;
   }

   if (flags & GL_MAP_PERSISTENT_BIT &&
       !(flags & (GL_MAP_READ_BIT | GL_MAP_WRITE_BIT))) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(PERSISTENT and flags!=READ/WRITE)", func);
      return false;
   }

   if (flags & GL_MAP_COHERENT_BIT && !(flags & GL_MAP_PERSISTENT_BIT)) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(COHERENT and flags!=PERSISTENT)", func);
      return false;
   }

   if (bufObj->Immutable || bufObj->HandleAllocated) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(immutable)", func);
      return false;
   }

   return true;
}


static void
buffer_storage(struct gl_context *ctx, struct gl_buffer_object *bufObj,
               struct gl_memory_object *memObj, GLenum target,
               GLsizeiptr size, const GLvoid *data, GLbitfield flags,
               GLuint64 offset, const char *func)
{
   GLboolean res;

   /* Unmap the existing buffer.  We'll replace it now.  Not an error. */
   _mesa_buffer_unmap_all_mappings(ctx, bufObj);

   FLUSH_VERTICES(ctx, 0, 0);

   bufObj->Immutable = GL_TRUE;
   bufObj->MinMaxCacheDirty = true;

   if (memObj) {
      res = bufferobj_data_mem(ctx, target, size, memObj, offset,
                               GL_DYNAMIC_DRAW, bufObj);
   }
   else {
      res = _mesa_bufferobj_data(ctx, target, size, data, GL_DYNAMIC_DRAW,
                                 flags, bufObj);
   }

   if (!res) {
      if (target == GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD) {
         /* Even though the interaction between AMD_pinned_memory and
          * glBufferStorage is not described in the spec, Graham Sellers
          * said that it should behave the same as glBufferData.
          */
         _mesa_error(ctx, GL_INVALID_OPERATION, "%s", func);
      }
      else {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s", func);
      }
   }
}


static ALWAYS_INLINE void
inlined_buffer_storage(GLenum target, GLuint buffer, GLsizeiptr size,
                       const GLvoid *data, GLbitfield flags,
                       GLuint memory, GLuint64 offset,
                       bool dsa, bool mem, bool no_error, const char *func)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;
   struct gl_memory_object *memObj = NULL;

   if (mem) {
      if (!no_error) {
         if (!ctx->Extensions.EXT_memory_object) {
            _mesa_error(ctx, GL_INVALID_OPERATION, "%s(unsupported)", func);
            return;
         }

         /* From the EXT_external_objects spec:
          *
          *   "An INVALID_VALUE error is generated by BufferStorageMemEXT and
          *   NamedBufferStorageMemEXT if <memory> is 0, or ..."
          */
         if (memory == 0) {
            _mesa_error(ctx, GL_INVALID_VALUE, "%s(memory == 0)", func);
         }
      }

      memObj = _mesa_lookup_memory_object(ctx, memory);
      if (!memObj)
         return;

      /* From the EXT_external_objects spec:
       *
       *   "An INVALID_OPERATION error is generated if <memory> names a
       *   valid memory object which has no associated memory."
       */
      if (!no_error && !memObj->Immutable) {
         _mesa_error(ctx, GL_INVALID_OPERATION, "%s(no associated memory)",
                     func);
         return;
      }
   }

   if (dsa) {
      if (no_error) {
         bufObj = _mesa_lookup_bufferobj(ctx, buffer);
      } else {
         bufObj = _mesa_lookup_bufferobj_err(ctx, buffer, func);
         if (!bufObj)
            return;
      }
   } else {
      if (no_error) {
         struct gl_buffer_object **bufObjPtr =
            get_buffer_target(ctx, target, true);
         bufObj = *bufObjPtr;
      } else {
         bufObj = get_buffer(ctx, func, target, GL_INVALID_OPERATION);
         if (!bufObj)
            return;
      }
   }

   if (no_error || validate_buffer_storage(ctx, bufObj, size, flags, func))
      buffer_storage(ctx, bufObj, memObj, target, size, data, flags, offset, func);
}


void GLAPIENTRY
_mesa_BufferStorage_no_error(GLenum target, GLsizeiptr size,
                             const GLvoid *data, GLbitfield flags)
{
   inlined_buffer_storage(target, 0, size, data, flags, GL_NONE, 0,
                          false, false, true, "glBufferStorage");
}


void GLAPIENTRY
_mesa_BufferStorage(GLenum target, GLsizeiptr size, const GLvoid *data,
                    GLbitfield flags)
{
   inlined_buffer_storage(target, 0, size, data, flags, GL_NONE, 0,
                          false, false, false, "glBufferStorage");
}

void GLAPIENTRY
_mesa_NamedBufferStorageEXT(GLuint buffer, GLsizeiptr size,
                            const GLvoid *data, GLbitfield flags)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_buffer_object *bufObj = _mesa_lookup_bufferobj(ctx, buffer);
   if (!handle_bind_buffer_gen(ctx, buffer,
                               &bufObj, "glNamedBufferStorageEXT", false))
      return;

   inlined_buffer_storage(GL_NONE, buffer, size, data, flags, GL_NONE, 0,
                          true, false, false, "glNamedBufferStorageEXT");
}


void GLAPIENTRY
_mesa_BufferStorageMemEXT(GLenum target, GLsizeiptr size,
                          GLuint memory, GLuint64 offset)
{
   inlined_buffer_storage(target, 0, size, NULL, 0, memory, offset,
                          false, true, false, "glBufferStorageMemEXT");
}


void GLAPIENTRY
_mesa_BufferStorageMemEXT_no_error(GLenum target, GLsizeiptr size,
                                   GLuint memory, GLuint64 offset)
{
   inlined_buffer_storage(target, 0, size, NULL, 0, memory, offset,
                          false, true, true, "glBufferStorageMemEXT");
}


void GLAPIENTRY
_mesa_NamedBufferStorage_no_error(GLuint buffer, GLsizeiptr size,
                                  const GLvoid *data, GLbitfield flags)
{
   /* In direct state access, buffer objects have an unspecified target
    * since they are not required to be bound.
    */
   inlined_buffer_storage(GL_NONE, buffer, size, data, flags, GL_NONE, 0,
                          true, false, true, "glNamedBufferStorage");
}


void GLAPIENTRY
_mesa_NamedBufferStorage(GLuint buffer, GLsizeiptr size, const GLvoid *data,
                         GLbitfield flags)
{
   /* In direct state access, buffer objects have an unspecified target
    * since they are not required to be bound.
    */
   inlined_buffer_storage(GL_NONE, buffer, size, data, flags, GL_NONE, 0,
                          true, false, false, "glNamedBufferStorage");
}

void GLAPIENTRY
_mesa_NamedBufferStorageMemEXT(GLuint buffer, GLsizeiptr size,
                               GLuint memory, GLuint64 offset)
{
   inlined_buffer_storage(GL_NONE, buffer, size, NULL, 0, memory, offset,
                          true, true, false, "glNamedBufferStorageMemEXT");
}


void GLAPIENTRY
_mesa_NamedBufferStorageMemEXT_no_error(GLuint buffer, GLsizeiptr size,
                                        GLuint memory, GLuint64 offset)
{
   inlined_buffer_storage(GL_NONE, buffer, size, NULL, 0, memory, offset,
                          true, true, true, "glNamedBufferStorageMemEXT");
}


static ALWAYS_INLINE void
buffer_data(struct gl_context *ctx, struct gl_buffer_object *bufObj,
            GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage,
            const char *func, bool no_error)
{
   bool valid_usage;

   if (MESA_VERBOSE & VERBOSE_API) {
      _mesa_debug(ctx, "%s(%s, %ld, %p, %s)\n",
                  func,
                  _mesa_enum_to_string(target),
                  (long int) size, data,
                  _mesa_enum_to_string(usage));
   }

   if (!no_error) {
      if (size < 0) {
         _mesa_error(ctx, GL_INVALID_VALUE, "%s(size < 0)", func);
         return;
      }

      switch (usage) {
      case GL_STREAM_DRAW_ARB:
         valid_usage = (ctx->API != API_OPENGLES);
         break;
      case GL_STATIC_DRAW_ARB:
      case GL_DYNAMIC_DRAW_ARB:
         valid_usage = true;
         break;
      case GL_STREAM_READ_ARB:
      case GL_STREAM_COPY_ARB:
      case GL_STATIC_READ_ARB:
      case GL_STATIC_COPY_ARB:
      case GL_DYNAMIC_READ_ARB:
      case GL_DYNAMIC_COPY_ARB:
         valid_usage = _mesa_is_desktop_gl(ctx) || _mesa_is_gles3(ctx);
         break;
      default:
         valid_usage = false;
         break;
      }

      if (!valid_usage) {
         _mesa_error(ctx, GL_INVALID_ENUM, "%s(invalid usage: %s)", func,
                     _mesa_enum_to_string(usage));
         return;
      }

      if (bufObj->Immutable || bufObj->HandleAllocated) {
         _mesa_error(ctx, GL_INVALID_OPERATION, "%s(immutable)", func);
         return;
      }
   }

   /* Unmap the existing buffer.  We'll replace it now.  Not an error. */
   _mesa_buffer_unmap_all_mappings(ctx, bufObj);

   FLUSH_VERTICES(ctx, 0, 0);

   bufObj->MinMaxCacheDirty = true;

#ifdef VBO_DEBUG
   printf("glBufferDataARB(%u, sz %ld, from %p, usage 0x%x)\n",
                bufObj->Name, size, data, usage);
#endif

#ifdef BOUNDS_CHECK
   size += 100;
#endif

   if (!_mesa_bufferobj_data(ctx, target, size, data, usage,
                             GL_MAP_READ_BIT |
                             GL_MAP_WRITE_BIT |
                             GL_DYNAMIC_STORAGE_BIT,
                             bufObj)) {
      if (target == GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD) {
         if (!no_error) {
            /* From GL_AMD_pinned_memory:
             *
             *   INVALID_OPERATION is generated by BufferData if <target> is
             *   EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD, and the store cannot be
             *   mapped to the GPU address space.
             */
            _mesa_error(ctx, GL_INVALID_OPERATION, "%s", func);
         }
      } else {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s", func);
      }
   }
}

static void
buffer_data_error(struct gl_context *ctx, struct gl_buffer_object *bufObj,
                  GLenum target, GLsizeiptr size, const GLvoid *data,
                  GLenum usage, const char *func)
{
   buffer_data(ctx, bufObj, target, size, data, usage, func, false);
}

static void
buffer_data_no_error(struct gl_context *ctx, struct gl_buffer_object *bufObj,
                     GLenum target, GLsizeiptr size, const GLvoid *data,
                     GLenum usage, const char *func)
{
   buffer_data(ctx, bufObj, target, size, data, usage, func, true);
}

void
_mesa_buffer_data(struct gl_context *ctx, struct gl_buffer_object *bufObj,
                  GLenum target, GLsizeiptr size, const GLvoid *data,
                  GLenum usage, const char *func)
{
   buffer_data_error(ctx, bufObj, target, size, data, usage, func);
}

void GLAPIENTRY
_mesa_BufferData_no_error(GLenum target, GLsizeiptr size, const GLvoid *data,
                          GLenum usage)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_buffer_object **bufObj = get_buffer_target(ctx, target, true);
   buffer_data_no_error(ctx, *bufObj, target, size, data, usage,
                        "glBufferData");
}

void GLAPIENTRY
_mesa_BufferData(GLenum target, GLsizeiptr size,
                 const GLvoid *data, GLenum usage)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;

   bufObj = get_buffer(ctx, "glBufferData", target, GL_INVALID_OPERATION);
   if (!bufObj)
      return;

   _mesa_buffer_data(ctx, bufObj, target, size, data, usage,
                     "glBufferData");
}

void GLAPIENTRY
_mesa_NamedBufferData_no_error(GLuint buffer, GLsizeiptr size,
                               const GLvoid *data, GLenum usage)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_buffer_object *bufObj = _mesa_lookup_bufferobj(ctx, buffer);
   buffer_data_no_error(ctx, bufObj, GL_NONE, size, data, usage,
                        "glNamedBufferData");
}

void GLAPIENTRY
_mesa_NamedBufferData(GLuint buffer, GLsizeiptr size, const GLvoid *data,
                      GLenum usage)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;

   bufObj = _mesa_lookup_bufferobj_err(ctx, buffer, "glNamedBufferData");
   if (!bufObj)
      return;

   /* In direct state access, buffer objects have an unspecified target since
    * they are not required to be bound.
    */
   _mesa_buffer_data(ctx, bufObj, GL_NONE, size, data, usage,
                     "glNamedBufferData");
}

void GLAPIENTRY
_mesa_NamedBufferDataEXT(GLuint buffer, GLsizeiptr size, const GLvoid *data,
                         GLenum usage)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;

   if (!buffer) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glNamedBufferDataEXT(buffer=0)");
      return;
   }

   bufObj = _mesa_lookup_bufferobj(ctx, buffer);
   if (!handle_bind_buffer_gen(ctx, buffer,
                               &bufObj, "glNamedBufferDataEXT", false))
      return;

   _mesa_buffer_data(ctx, bufObj, GL_NONE, size, data, usage,
                     "glNamedBufferDataEXT");
}

static bool
validate_buffer_sub_data(struct gl_context *ctx,
                         struct gl_buffer_object *bufObj,
                         GLintptr offset, GLsizeiptr size,
                         const char *func)
{
   if (!buffer_object_subdata_range_good(ctx, bufObj, offset, size,
                                         true, func)) {
      /* error already recorded */
      return false;
   }

   if (bufObj->Immutable &&
       !(bufObj->StorageFlags & GL_DYNAMIC_STORAGE_BIT)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s", func);
      return false;
   }

   if ((bufObj->Usage == GL_STATIC_DRAW ||
        bufObj->Usage == GL_STATIC_COPY) &&
       bufObj->NumSubDataCalls >= BUFFER_WARNING_CALL_COUNT - 1) {
      /* If the application declared the buffer as static draw/copy or stream
       * draw, it should not be frequently modified with glBufferSubData.
       */
      BUFFER_USAGE_WARNING(ctx,
                           "using %s(buffer %u, offset %u, size %u) to "
                           "update a %s buffer",
                           func, bufObj->Name, offset, size,
                           _mesa_enum_to_string(bufObj->Usage));
   }

   return true;
}


/**
 * Implementation for glBufferSubData and glNamedBufferSubData.
 *
 * \param ctx     GL context.
 * \param bufObj  The buffer object.
 * \param offset  Offset of the first byte of the subdata range.
 * \param size    Size, in bytes, of the subdata range.
 * \param data    The data store.
 * \param func  Name of calling function for recording errors.
 *
 */
void
_mesa_buffer_sub_data(struct gl_context *ctx, struct gl_buffer_object *bufObj,
                      GLintptr offset, GLsizeiptr size, const GLvoid *data)
{
   if (size == 0)
      return;

   bufObj->NumSubDataCalls++;
   bufObj->MinMaxCacheDirty = true;

   _mesa_bufferobj_subdata(ctx, offset, size, data, bufObj);
}


static ALWAYS_INLINE void
buffer_sub_data(GLenum target, GLuint buffer, GLintptr offset,
                GLsizeiptr size, const GLvoid *data,
                bool dsa, bool no_error, const char *func)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;

   if (dsa) {
      if (no_error) {
         bufObj = _mesa_lookup_bufferobj(ctx, buffer);
      } else {
         bufObj = _mesa_lookup_bufferobj_err(ctx, buffer, func);
         if (!bufObj)
            return;
      }
   } else {
      if (no_error) {
         struct gl_buffer_object **bufObjPtr = get_buffer_target(ctx, target, true);
         bufObj = *bufObjPtr;
      } else {
         bufObj = get_buffer(ctx, func, target, GL_INVALID_OPERATION);
         if (!bufObj)
            return;
      }
   }

   if (no_error || validate_buffer_sub_data(ctx, bufObj, offset, size, func))
      _mesa_buffer_sub_data(ctx, bufObj, offset, size, data);
}


void GLAPIENTRY
_mesa_BufferSubData_no_error(GLenum target, GLintptr offset,
                             GLsizeiptr size, const GLvoid *data)
{
   buffer_sub_data(target, 0, offset, size, data, false, true,
                   "glBufferSubData");
}


void GLAPIENTRY
_mesa_BufferSubData(GLenum target, GLintptr offset,
                    GLsizeiptr size, const GLvoid *data)
{
   buffer_sub_data(target, 0, offset, size, data, false, false,
                   "glBufferSubData");
}

void GLAPIENTRY
_mesa_NamedBufferSubData_no_error(GLuint buffer, GLintptr offset,
                                  GLsizeiptr size, const GLvoid *data)
{
   buffer_sub_data(0, buffer, offset, size, data, true, true,
                   "glNamedBufferSubData");
}

void GLAPIENTRY
_mesa_NamedBufferSubData(GLuint buffer, GLintptr offset,
                         GLsizeiptr size, const GLvoid *data)
{
   buffer_sub_data(0, buffer, offset, size, data, true, false,
                   "glNamedBufferSubData");
}

void GLAPIENTRY
_mesa_NamedBufferSubDataEXT(GLuint buffer, GLintptr offset,
                            GLsizeiptr size, const GLvoid *data)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;

   if (!buffer) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glNamedBufferSubDataEXT(buffer=0)");
      return;
   }

   bufObj = _mesa_lookup_bufferobj(ctx, buffer);
   if (!handle_bind_buffer_gen(ctx, buffer,
                               &bufObj, "glNamedBufferSubDataEXT", false))
      return;

   if (validate_buffer_sub_data(ctx, bufObj, offset, size,
                                "glNamedBufferSubDataEXT")) {
      _mesa_buffer_sub_data(ctx, bufObj, offset, size, data);
   }
}


void GLAPIENTRY
_mesa_GetBufferSubData(GLenum target, GLintptr offset,
                       GLsizeiptr size, GLvoid *data)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;

   bufObj = get_buffer(ctx, "glGetBufferSubData", target,
                       GL_INVALID_OPERATION);
   if (!bufObj)
      return;

   if (!buffer_object_subdata_range_good(ctx, bufObj, offset, size, false,
                                         "glGetBufferSubData")) {
      return;
   }

   bufferobj_get_subdata(ctx, offset, size, data, bufObj);
}

void GLAPIENTRY
_mesa_GetNamedBufferSubData(GLuint buffer, GLintptr offset,
                            GLsizeiptr size, GLvoid *data)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;

   bufObj = _mesa_lookup_bufferobj_err(ctx, buffer,
                                       "glGetNamedBufferSubData");
   if (!bufObj)
      return;

   if (!buffer_object_subdata_range_good(ctx, bufObj, offset, size, false,
                                         "glGetNamedBufferSubData")) {
      return;
   }

   bufferobj_get_subdata(ctx, offset, size, data, bufObj);
}


void GLAPIENTRY
_mesa_GetNamedBufferSubDataEXT(GLuint buffer, GLintptr offset,
                               GLsizeiptr size, GLvoid *data)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;

   if (!buffer) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glGetNamedBufferSubDataEXT(buffer=0)");
      return;
   }

   bufObj = _mesa_lookup_bufferobj(ctx, buffer);
   if (!handle_bind_buffer_gen(ctx, buffer,
                               &bufObj, "glGetNamedBufferSubDataEXT", false))
      return;

   if (!buffer_object_subdata_range_good(ctx, bufObj, offset, size, false,
                                         "glGetNamedBufferSubDataEXT")) {
      return;
   }

   bufferobj_get_subdata(ctx, offset, size, data, bufObj);
}

/**
 * \param subdata   true if caller is *SubData, false if *Data
 */
static ALWAYS_INLINE void
clear_buffer_sub_data(struct gl_context *ctx, struct gl_buffer_object *bufObj,
                      GLenum internalformat, GLintptr offset, GLsizeiptr size,
                      GLenum format, GLenum type, const GLvoid *data,
                      const char *func, bool subdata, bool no_error)
{
   mesa_format mesaFormat;
   GLubyte clearValue[MAX_PIXEL_BYTES];
   GLsizeiptr clearValueSize;

   /* This checks for disallowed mappings. */
   if (!no_error && !buffer_object_subdata_range_good(ctx, bufObj, offset, size,
                                                      subdata, func)) {
      return;
   }

   if (no_error) {
      mesaFormat = _mesa_get_texbuffer_format(ctx, internalformat);
   } else {
      mesaFormat = validate_clear_buffer_format(ctx, internalformat,
                                                format, type, func);
   }

   if (mesaFormat == MESA_FORMAT_NONE)
      return;

   clearValueSize = _mesa_get_format_bytes(mesaFormat);
   if (!no_error &&
       (offset % clearValueSize != 0 || size % clearValueSize != 0)) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(offset or size is not a multiple of "
                  "internalformat size)", func);
      return;
   }

   /* Bail early. Negative size has already been checked. */
   if (size == 0)
      return;

   bufObj->MinMaxCacheDirty = true;

   if (!ctx->pipe->clear_buffer) {
      clear_buffer_subdata_sw(ctx, offset, size,
                              data, clearValueSize, bufObj);
      return;
   }

   if (!data)
      memset(clearValue, 0, MAX_PIXEL_BYTES);
   else if (!convert_clear_buffer_data(ctx, mesaFormat, clearValue,
                                       format, type, data, func)) {
      return;
   }

   ctx->pipe->clear_buffer(ctx->pipe, bufObj->buffer, offset, size,
                           clearValue, clearValueSize);
}

static void
clear_buffer_sub_data_error(struct gl_context *ctx,
                            struct gl_buffer_object *bufObj,
                            GLenum internalformat, GLintptr offset,
                            GLsizeiptr size, GLenum format, GLenum type,
                            const GLvoid *data, const char *func, bool subdata)
{
   clear_buffer_sub_data(ctx, bufObj, internalformat, offset, size, format,
                         type, data, func, subdata, false);
}


static void
clear_buffer_sub_data_no_error(struct gl_context *ctx,
                               struct gl_buffer_object *bufObj,
                               GLenum internalformat, GLintptr offset,
                               GLsizeiptr size, GLenum format, GLenum type,
                               const GLvoid *data, const char *func,
                               bool subdata)
{
   clear_buffer_sub_data(ctx, bufObj, internalformat, offset, size, format,
                         type, data, func, subdata, true);
}


void GLAPIENTRY
_mesa_ClearBufferData_no_error(GLenum target, GLenum internalformat,
                               GLenum format, GLenum type, const GLvoid *data)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_buffer_object **bufObj = get_buffer_target(ctx, target, true);
   clear_buffer_sub_data_no_error(ctx, *bufObj, internalformat, 0,
                                  (*bufObj)->Size, format, type, data,
                                  "glClearBufferData", false);
}


void GLAPIENTRY
_mesa_ClearBufferData(GLenum target, GLenum internalformat, GLenum format,
                      GLenum type, const GLvoid *data)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;

   bufObj = get_buffer(ctx, "glClearBufferData", target, GL_INVALID_VALUE);
   if (!bufObj)
      return;

   clear_buffer_sub_data_error(ctx, bufObj, internalformat, 0, bufObj->Size,
                               format, type, data, "glClearBufferData", false);
}


void GLAPIENTRY
_mesa_ClearNamedBufferData_no_error(GLuint buffer, GLenum internalformat,
                                    GLenum format, GLenum type,
                                    const GLvoid *data)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_buffer_object *bufObj = _mesa_lookup_bufferobj(ctx, buffer);
   clear_buffer_sub_data_no_error(ctx, bufObj, internalformat, 0, bufObj->Size,
                                  format, type, data, "glClearNamedBufferData",
                                  false);
}


void GLAPIENTRY
_mesa_ClearNamedBufferData(GLuint buffer, GLenum internalformat,
                           GLenum format, GLenum type, const GLvoid *data)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;

   bufObj = _mesa_lookup_bufferobj_err(ctx, buffer, "glClearNamedBufferData");
   if (!bufObj)
      return;

   clear_buffer_sub_data_error(ctx, bufObj, internalformat, 0, bufObj->Size,
                               format, type, data, "glClearNamedBufferData",
                               false);
}


void GLAPIENTRY
_mesa_ClearNamedBufferDataEXT(GLuint buffer, GLenum internalformat,
                              GLenum format, GLenum type, const GLvoid *data)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj = _mesa_lookup_bufferobj(ctx, buffer);
   if (!handle_bind_buffer_gen(ctx, buffer,
                               &bufObj, "glClearNamedBufferDataEXT", false))
      return;

   clear_buffer_sub_data_error(ctx, bufObj, internalformat, 0, bufObj->Size,
                               format, type, data, "glClearNamedBufferDataEXT",
                               false);
}


void GLAPIENTRY
_mesa_ClearBufferSubData_no_error(GLenum target, GLenum internalformat,
                                  GLintptr offset, GLsizeiptr size,
                                  GLenum format, GLenum type,
                                  const GLvoid *data)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_buffer_object **bufObj = get_buffer_target(ctx, target, true);
   clear_buffer_sub_data_no_error(ctx, *bufObj, internalformat, offset, size,
                                  format, type, data, "glClearBufferSubData",
                                  true);
}


void GLAPIENTRY
_mesa_ClearBufferSubData(GLenum target, GLenum internalformat,
                         GLintptr offset, GLsizeiptr size,
                         GLenum format, GLenum type,
                         const GLvoid *data)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;

   bufObj = get_buffer(ctx, "glClearBufferSubData", target, GL_INVALID_VALUE);
   if (!bufObj)
      return;

   clear_buffer_sub_data_error(ctx, bufObj, internalformat, offset, size,
                               format, type, data, "glClearBufferSubData",
                               true);
}


void GLAPIENTRY
_mesa_ClearNamedBufferSubData_no_error(GLuint buffer, GLenum internalformat,
                                       GLintptr offset, GLsizeiptr size,
                                       GLenum format, GLenum type,
                                       const GLvoid *data)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_buffer_object *bufObj = _mesa_lookup_bufferobj(ctx, buffer);
   clear_buffer_sub_data_no_error(ctx, bufObj, internalformat, offset, size,
                                  format, type, data,
                                  "glClearNamedBufferSubData", true);
}


void GLAPIENTRY
_mesa_ClearNamedBufferSubData(GLuint buffer, GLenum internalformat,
                              GLintptr offset, GLsizeiptr size,
                              GLenum format, GLenum type,
                              const GLvoid *data)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;

   bufObj = _mesa_lookup_bufferobj_err(ctx, buffer,
                                       "glClearNamedBufferSubData");
   if (!bufObj)
      return;

   clear_buffer_sub_data_error(ctx, bufObj, internalformat, offset, size,
                               format, type, data, "glClearNamedBufferSubData",
                               true);
}

void GLAPIENTRY
_mesa_ClearNamedBufferSubDataEXT(GLuint buffer, GLenum internalformat,
                                 GLintptr offset, GLsizeiptr size,
                                 GLenum format, GLenum type,
                                 const GLvoid *data)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj = _mesa_lookup_bufferobj(ctx, buffer);
   if (!handle_bind_buffer_gen(ctx, buffer,
                               &bufObj, "glClearNamedBufferSubDataEXT", false))
      return;

   clear_buffer_sub_data_error(ctx, bufObj, internalformat, offset, size,
                               format, type, data, "glClearNamedBufferSubDataEXT",
                               true);
}

static GLboolean
unmap_buffer(struct gl_context *ctx, struct gl_buffer_object *bufObj)
{
   GLboolean status = _mesa_bufferobj_unmap(ctx, bufObj, MAP_USER);
   bufObj->Mappings[MAP_USER].AccessFlags = 0;
   assert(bufObj->Mappings[MAP_USER].Pointer == NULL);
   assert(bufObj->Mappings[MAP_USER].Offset == 0);
   assert(bufObj->Mappings[MAP_USER].Length == 0);

   return status;
}

static GLboolean
validate_and_unmap_buffer(struct gl_context *ctx,
                          struct gl_buffer_object *bufObj,
                          const char *func)
{
   ASSERT_OUTSIDE_BEGIN_END_WITH_RETVAL(ctx, GL_FALSE);

   if (!_mesa_bufferobj_mapped(bufObj, MAP_USER)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(buffer is not mapped)", func);
      return GL_FALSE;
   }

#ifdef BOUNDS_CHECK
   if (bufObj->Mappings[MAP_USER].AccessFlags != GL_READ_ONLY_ARB) {
      GLubyte *buf = (GLubyte *) bufObj->Mappings[MAP_USER].Pointer;
      GLuint i;
      /* check that last 100 bytes are still = magic value */
      for (i = 0; i < 100; i++) {
         GLuint pos = bufObj->Size - i - 1;
         if (buf[pos] != 123) {
            _mesa_warning(ctx, "Out of bounds buffer object write detected"
                          " at position %d (value = %u)\n",
                          pos, buf[pos]);
         }
      }
   }
#endif

#ifdef VBO_DEBUG
   if (bufObj->Mappings[MAP_USER].AccessFlags & GL_MAP_WRITE_BIT) {
      GLuint i, unchanged = 0;
      GLubyte *b = (GLubyte *) bufObj->Mappings[MAP_USER].Pointer;
      GLint pos = -1;
      /* check which bytes changed */
      for (i = 0; i < bufObj->Size - 1; i++) {
         if (b[i] == (i & 0xff) && b[i+1] == ((i+1) & 0xff)) {
            unchanged++;
            if (pos == -1)
               pos = i;
         }
      }
      if (unchanged) {
         printf("glUnmapBufferARB(%u): %u of %ld unchanged, starting at %d\n",
                      bufObj->Name, unchanged, bufObj->Size, pos);
      }
   }
#endif

   return unmap_buffer(ctx, bufObj);
}

GLboolean GLAPIENTRY
_mesa_UnmapBuffer_no_error(GLenum target)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object **bufObjPtr = get_buffer_target(ctx, target, true);
   struct gl_buffer_object *bufObj = *bufObjPtr;

   return unmap_buffer(ctx, bufObj);
}

GLboolean GLAPIENTRY
_mesa_UnmapBuffer(GLenum target)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;

   bufObj = get_buffer(ctx, "glUnmapBuffer", target, GL_INVALID_OPERATION);
   if (!bufObj)
      return GL_FALSE;

   return validate_and_unmap_buffer(ctx, bufObj, "glUnmapBuffer");
}

GLboolean GLAPIENTRY
_mesa_UnmapNamedBufferEXT_no_error(GLuint buffer)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj = _mesa_lookup_bufferobj(ctx, buffer);

   return unmap_buffer(ctx, bufObj);
}

GLboolean GLAPIENTRY
_mesa_UnmapNamedBufferEXT(GLuint buffer)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;

   if (!buffer) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glUnmapNamedBufferEXT(buffer=0)");
      return GL_FALSE;
   }

   bufObj = _mesa_lookup_bufferobj_err(ctx, buffer, "glUnmapNamedBuffer");
   if (!bufObj)
      return GL_FALSE;

   return validate_and_unmap_buffer(ctx, bufObj, "glUnmapNamedBuffer");
}


static bool
get_buffer_parameter(struct gl_context *ctx,
                     struct gl_buffer_object *bufObj, GLenum pname,
                     GLint64 *params, const char *func)
{
   switch (pname) {
   case GL_BUFFER_SIZE_ARB:
      *params = bufObj->Size;
      break;
   case GL_BUFFER_USAGE_ARB:
      *params = bufObj->Usage;
      break;
   case GL_BUFFER_ACCESS_ARB:
      *params = simplified_access_mode(ctx,
                            bufObj->Mappings[MAP_USER].AccessFlags);
      break;
   case GL_BUFFER_MAPPED_ARB:
      *params = _mesa_bufferobj_mapped(bufObj, MAP_USER);
      break;
   case GL_BUFFER_ACCESS_FLAGS:
      if (!ctx->Extensions.ARB_map_buffer_range)
         goto invalid_pname;
      *params = bufObj->Mappings[MAP_USER].AccessFlags;
      break;
   case GL_BUFFER_MAP_OFFSET:
      if (!ctx->Extensions.ARB_map_buffer_range)
         goto invalid_pname;
      *params = bufObj->Mappings[MAP_USER].Offset;
      break;
   case GL_BUFFER_MAP_LENGTH:
      if (!ctx->Extensions.ARB_map_buffer_range)
         goto invalid_pname;
      *params = bufObj->Mappings[MAP_USER].Length;
      break;
   case GL_BUFFER_IMMUTABLE_STORAGE:
      if (!ctx->Extensions.ARB_buffer_storage)
         goto invalid_pname;
      *params = bufObj->Immutable;
      break;
   case GL_BUFFER_STORAGE_FLAGS:
      if (!ctx->Extensions.ARB_buffer_storage)
         goto invalid_pname;
      *params = bufObj->StorageFlags;
      break;
   default:
      goto invalid_pname;
   }

   return true;

invalid_pname:
   _mesa_error(ctx, GL_INVALID_ENUM, "%s(invalid pname: %s)", func,
               _mesa_enum_to_string(pname));
   return false;
}

void GLAPIENTRY
_mesa_GetBufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;
   GLint64 parameter;

   bufObj = get_buffer(ctx, "glGetBufferParameteriv", target,
                       GL_INVALID_OPERATION);
   if (!bufObj)
      return;

   if (!get_buffer_parameter(ctx, bufObj, pname, &parameter,
                             "glGetBufferParameteriv"))
      return; /* Error already recorded. */

   *params = (GLint) parameter;
}

void GLAPIENTRY
_mesa_GetBufferParameteri64v(GLenum target, GLenum pname, GLint64 *params)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;
   GLint64 parameter;

   bufObj = get_buffer(ctx, "glGetBufferParameteri64v", target,
                       GL_INVALID_OPERATION);
   if (!bufObj)
      return;

   if (!get_buffer_parameter(ctx, bufObj, pname, &parameter,
                             "glGetBufferParameteri64v"))
      return; /* Error already recorded. */

   *params = parameter;
}

void GLAPIENTRY
_mesa_GetNamedBufferParameteriv(GLuint buffer, GLenum pname, GLint *params)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;
   GLint64 parameter;

   bufObj = _mesa_lookup_bufferobj_err(ctx, buffer,
                                       "glGetNamedBufferParameteriv");
   if (!bufObj)
      return;

   if (!get_buffer_parameter(ctx, bufObj, pname, &parameter,
                             "glGetNamedBufferParameteriv"))
      return; /* Error already recorded. */

   *params = (GLint) parameter;
}

void GLAPIENTRY
_mesa_GetNamedBufferParameterivEXT(GLuint buffer, GLenum pname, GLint *params)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;
   GLint64 parameter;

   if (!buffer) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glGetNamedBufferParameterivEXT: buffer=0");
      return;
   }

   bufObj = _mesa_lookup_bufferobj(ctx, buffer);
   if (!handle_bind_buffer_gen(ctx, buffer,
                               &bufObj, "glGetNamedBufferParameterivEXT", false))
      return;

   if (!get_buffer_parameter(ctx, bufObj, pname, &parameter,
                             "glGetNamedBufferParameterivEXT"))
      return; /* Error already recorded. */

   *params = (GLint) parameter;
}

void GLAPIENTRY
_mesa_GetNamedBufferParameteri64v(GLuint buffer, GLenum pname,
                                  GLint64 *params)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;
   GLint64 parameter;

   bufObj = _mesa_lookup_bufferobj_err(ctx, buffer,
                                       "glGetNamedBufferParameteri64v");
   if (!bufObj)
      return;

   if (!get_buffer_parameter(ctx, bufObj, pname, &parameter,
                             "glGetNamedBufferParameteri64v"))
      return; /* Error already recorded. */

   *params = parameter;
}


void GLAPIENTRY
_mesa_GetBufferPointerv(GLenum target, GLenum pname, GLvoid **params)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;

   if (pname != GL_BUFFER_MAP_POINTER) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glGetBufferPointerv(pname != "
                  "GL_BUFFER_MAP_POINTER)");
      return;
   }

   bufObj = get_buffer(ctx, "glGetBufferPointerv", target,
                       GL_INVALID_OPERATION);
   if (!bufObj)
      return;

   *params = bufObj->Mappings[MAP_USER].Pointer;
}

void GLAPIENTRY
_mesa_GetNamedBufferPointerv(GLuint buffer, GLenum pname, GLvoid **params)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;

   if (pname != GL_BUFFER_MAP_POINTER) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glGetNamedBufferPointerv(pname != "
                  "GL_BUFFER_MAP_POINTER)");
      return;
   }

   bufObj = _mesa_lookup_bufferobj_err(ctx, buffer,
                                       "glGetNamedBufferPointerv");
   if (!bufObj)
      return;

   *params = bufObj->Mappings[MAP_USER].Pointer;
}

void GLAPIENTRY
_mesa_GetNamedBufferPointervEXT(GLuint buffer, GLenum pname, GLvoid **params)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;

   if (!buffer) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glGetNamedBufferPointervEXT(buffer=0)");
      return;
   }
   if (pname != GL_BUFFER_MAP_POINTER) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glGetNamedBufferPointervEXT(pname != "
                  "GL_BUFFER_MAP_POINTER)");
      return;
   }

   bufObj = _mesa_lookup_bufferobj(ctx, buffer);
   if (!handle_bind_buffer_gen(ctx, buffer,
                               &bufObj, "glGetNamedBufferPointervEXT", false))
      return;

   *params = bufObj->Mappings[MAP_USER].Pointer;
}

static void
copy_buffer_sub_data(struct gl_context *ctx, struct gl_buffer_object *src,
                     struct gl_buffer_object *dst, GLintptr readOffset,
                     GLintptr writeOffset, GLsizeiptr size, const char *func)
{
   if (_mesa_check_disallowed_mapping(src)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(readBuffer is mapped)", func);
      return;
   }

   if (_mesa_check_disallowed_mapping(dst)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(writeBuffer is mapped)", func);
      return;
   }

   if (readOffset < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(readOffset %d < 0)", func, (int) readOffset);
      return;
   }

   if (writeOffset < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(writeOffset %d < 0)", func, (int) writeOffset);
      return;
   }

   if (size < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(size %d < 0)", func, (int) size);
      return;
   }

   if (size > src->Size || readOffset > src->Size - size) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(readOffset %d + size %d > src_buffer_size %d)", func,
                  (int) readOffset, (int) size, (int) src->Size);
      return;
   }

   if (size > dst->Size || writeOffset > dst->Size - size) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(writeOffset %d + size %d > dst_buffer_size %d)", func,
                  (int) writeOffset, (int) size, (int) dst->Size);
      return;
   }

   if (src == dst) {
      if (readOffset + size <= writeOffset) {
         /* OK */
      }
      else if (writeOffset + size <= readOffset) {
         /* OK */
      }
      else {
         /* overlapping src/dst is illegal */
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "%s(overlapping src/dst)", func);
         return;
      }
   }

   bufferobj_copy_subdata(ctx, src, dst, readOffset, writeOffset, size);
}

void GLAPIENTRY
_mesa_CopyBufferSubData_no_error(GLenum readTarget, GLenum writeTarget,
                                 GLintptr readOffset, GLintptr writeOffset,
                                 GLsizeiptr size)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_buffer_object **src_ptr = get_buffer_target(ctx, readTarget, true);
   struct gl_buffer_object *src = *src_ptr;

   struct gl_buffer_object **dst_ptr = get_buffer_target(ctx, writeTarget, true);
   struct gl_buffer_object *dst = *dst_ptr;

   bufferobj_copy_subdata(ctx, src, dst, readOffset, writeOffset,
                       size);
}

void GLAPIENTRY
_mesa_CopyBufferSubData(GLenum readTarget, GLenum writeTarget,
                        GLintptr readOffset, GLintptr writeOffset,
                        GLsizeiptr size)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *src, *dst;

   src = get_buffer(ctx, "glCopyBufferSubData", readTarget,
                    GL_INVALID_OPERATION);
   if (!src)
      return;

   dst = get_buffer(ctx, "glCopyBufferSubData", writeTarget,
                    GL_INVALID_OPERATION);
   if (!dst)
      return;

   copy_buffer_sub_data(ctx, src, dst, readOffset, writeOffset, size,
                        "glCopyBufferSubData");
}

void GLAPIENTRY
_mesa_NamedCopyBufferSubDataEXT(GLuint readBuffer, GLuint writeBuffer,
                                GLintptr readOffset, GLintptr writeOffset,
                                GLsizeiptr size)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *src, *dst;

   src = _mesa_lookup_bufferobj(ctx, readBuffer);
   if (!handle_bind_buffer_gen(ctx, readBuffer,
                               &src,
                                     "glNamedCopyBufferSubDataEXT", false))
      return;

   dst = _mesa_lookup_bufferobj(ctx, writeBuffer);
   if (!handle_bind_buffer_gen(ctx, writeBuffer,
                               &dst, "glNamedCopyBufferSubDataEXT", false))
      return;

   copy_buffer_sub_data(ctx, src, dst, readOffset, writeOffset, size,
                        "glNamedCopyBufferSubDataEXT");
}

void GLAPIENTRY
_mesa_CopyNamedBufferSubData_no_error(GLuint readBuffer, GLuint writeBuffer,
                                      GLintptr readOffset,
                                      GLintptr writeOffset, GLsizeiptr size)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_buffer_object *src = _mesa_lookup_bufferobj(ctx, readBuffer);
   struct gl_buffer_object *dst = _mesa_lookup_bufferobj(ctx, writeBuffer);

   bufferobj_copy_subdata(ctx, src, dst, readOffset, writeOffset,
                       size);
}

void GLAPIENTRY
_mesa_CopyNamedBufferSubData(GLuint readBuffer, GLuint writeBuffer,
                             GLintptr readOffset, GLintptr writeOffset,
                             GLsizeiptr size)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *src, *dst;

   src = _mesa_lookup_bufferobj_err(ctx, readBuffer,
                                    "glCopyNamedBufferSubData");
   if (!src)
      return;

   dst = _mesa_lookup_bufferobj_err(ctx, writeBuffer,
                                    "glCopyNamedBufferSubData");
   if (!dst)
      return;

   copy_buffer_sub_data(ctx, src, dst, readOffset, writeOffset, size,
                        "glCopyNamedBufferSubData");
}

void GLAPIENTRY
_mesa_InternalBufferSubDataCopyMESA(GLintptr srcBuffer, GLuint srcOffset,
                                    GLuint dstTargetOrName, GLintptr dstOffset,
                                    GLsizeiptr size, GLboolean named,
                                    GLboolean ext_dsa)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *src = (struct gl_buffer_object *)srcBuffer;
   struct gl_buffer_object *dst;
   const char *func;

   /* Handle behavior for all 3 variants. */
   if (named && ext_dsa) {
      func = "glNamedBufferSubDataEXT";
      dst = _mesa_lookup_bufferobj(ctx, dstTargetOrName);
      if (!handle_bind_buffer_gen(ctx, dstTargetOrName, &dst, func, false))
         goto done;
   } else if (named) {
      func = "glNamedBufferSubData";
      dst = _mesa_lookup_bufferobj_err(ctx, dstTargetOrName, func);
      if (!dst)
         goto done;
   } else {
      assert(!ext_dsa);
      func = "glBufferSubData";
      dst = get_buffer(ctx, func, dstTargetOrName, GL_INVALID_OPERATION);
      if (!dst)
         goto done;
   }

   if (!validate_buffer_sub_data(ctx, dst, dstOffset, size, func))
      goto done; /* the error is already set */

   bufferobj_copy_subdata(ctx, src, dst, srcOffset, dstOffset, size);

done:
   /* The caller passes the reference to this function, so unreference it. */
   _mesa_reference_buffer_object(ctx, &src, NULL);
}

static bool
validate_map_buffer_range(struct gl_context *ctx,
                          struct gl_buffer_object *bufObj, GLintptr offset,
                          GLsizeiptr length, GLbitfield access,
                          const char *func)
{
   GLbitfield allowed_access;

   ASSERT_OUTSIDE_BEGIN_END_WITH_RETVAL(ctx, false);

   if (offset < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(offset %ld < 0)", func, (long) offset);
      return false;
   }

   if (length < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(length %ld < 0)", func, (long) length);
      return false;
   }

   /* Page 38 of the PDF of the OpenGL ES 3.0 spec says:
    *
    *     "An INVALID_OPERATION error is generated for any of the following
    *     conditions:
    *
    *     * <length> is zero."
    *
    * Additionally, page 94 of the PDF of the OpenGL 4.5 core spec
    * (30.10.2014) also says this, so it's no longer allowed for desktop GL,
    * either.
    */
   if (length == 0) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(length = 0)", func);
      return false;
   }

   allowed_access = GL_MAP_READ_BIT |
                    GL_MAP_WRITE_BIT |
                    GL_MAP_INVALIDATE_RANGE_BIT |
                    GL_MAP_INVALIDATE_BUFFER_BIT |
                    GL_MAP_FLUSH_EXPLICIT_BIT |
                    GL_MAP_UNSYNCHRONIZED_BIT;

   if (ctx->Extensions.ARB_buffer_storage) {
         allowed_access |= GL_MAP_PERSISTENT_BIT |
                           GL_MAP_COHERENT_BIT;
   }

   if (access & ~allowed_access) {
      /* generate an error if any bits other than those allowed are set */
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(access has undefined bits set)", func);
      return false;
   }

   if ((access & (GL_MAP_READ_BIT | GL_MAP_WRITE_BIT)) == 0) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(access indicates neither read or write)", func);
      return false;
   }

   if ((access & GL_MAP_READ_BIT) &&
       (access & (GL_MAP_INVALIDATE_RANGE_BIT |
                  GL_MAP_INVALIDATE_BUFFER_BIT |
                  GL_MAP_UNSYNCHRONIZED_BIT))) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(read access with disallowed bits)", func);
      return false;
   }

   if ((access & GL_MAP_FLUSH_EXPLICIT_BIT) &&
       ((access & GL_MAP_WRITE_BIT) == 0)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(access has flush explicit without write)", func);
      return false;
   }

   if (access & GL_MAP_READ_BIT &&
       !(bufObj->StorageFlags & GL_MAP_READ_BIT)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(buffer does not allow read access)", func);
      return false;
   }

   if (access & GL_MAP_WRITE_BIT &&
       !(bufObj->StorageFlags & GL_MAP_WRITE_BIT)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(buffer does not allow write access)", func);
      return false;
   }

   if (access & GL_MAP_COHERENT_BIT &&
       !(bufObj->StorageFlags & GL_MAP_COHERENT_BIT)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(buffer does not allow coherent access)", func);
      return false;
   }

   if (access & GL_MAP_PERSISTENT_BIT &&
       !(bufObj->StorageFlags & GL_MAP_PERSISTENT_BIT)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(buffer does not allow persistent access)", func);
      return false;
   }

   if (offset + length > bufObj->Size) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(offset %lu + length %lu > buffer_size %lu)", func,
                  (unsigned long) offset, (unsigned long) length,
                  (unsigned long) bufObj->Size);
      return false;
   }

   if (_mesa_bufferobj_mapped(bufObj, MAP_USER)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(buffer already mapped)", func);
      return false;
   }

   if (access & GL_MAP_WRITE_BIT) {
      bufObj->NumMapBufferWriteCalls++;
      if ((bufObj->Usage == GL_STATIC_DRAW ||
           bufObj->Usage == GL_STATIC_COPY) &&
          bufObj->NumMapBufferWriteCalls >= BUFFER_WARNING_CALL_COUNT) {
         BUFFER_USAGE_WARNING(ctx,
                              "using %s(buffer %u, offset %u, length %u) to "
                              "update a %s buffer",
                              func, bufObj->Name, offset, length,
                              _mesa_enum_to_string(bufObj->Usage));
      }
   }

   return true;
}

static void *
map_buffer_range(struct gl_context *ctx, struct gl_buffer_object *bufObj,
                 GLintptr offset, GLsizeiptr length, GLbitfield access,
                 const char *func)
{
   if (!bufObj->Size) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s(buffer size = 0)", func);
      return NULL;
   }

   void *map = _mesa_bufferobj_map_range(ctx, offset, length, access, bufObj,
                                         MAP_USER);
   if (!map) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s(map failed)", func);
   }
   else {
      /* The driver callback should have set all these fields.
       * This is important because other modules (like VBO) might call
       * the driver function directly.
       */
      assert(bufObj->Mappings[MAP_USER].Pointer == map);
      assert(bufObj->Mappings[MAP_USER].Length == length);
      assert(bufObj->Mappings[MAP_USER].Offset == offset);
      assert(bufObj->Mappings[MAP_USER].AccessFlags == access);
   }

   if (access & GL_MAP_WRITE_BIT) {
      bufObj->MinMaxCacheDirty = true;
   }

#ifdef VBO_DEBUG
   if (strstr(func, "Range") == NULL) { /* If not MapRange */
      printf("glMapBuffer(%u, sz %ld, access 0x%x)\n",
            bufObj->Name, bufObj->Size, access);
      /* Access must be write only */
      if ((access & GL_MAP_WRITE_BIT) && (!(access & ~GL_MAP_WRITE_BIT))) {
         GLuint i;
         GLubyte *b = (GLubyte *) bufObj->Mappings[MAP_USER].Pointer;
         for (i = 0; i < bufObj->Size; i++)
            b[i] = i & 0xff;
      }
   }
#endif

#ifdef BOUNDS_CHECK
   if (strstr(func, "Range") == NULL) { /* If not MapRange */
      GLubyte *buf = (GLubyte *) bufObj->Mappings[MAP_USER].Pointer;
      GLuint i;
      /* buffer is 100 bytes larger than requested, fill with magic value */
      for (i = 0; i < 100; i++) {
         buf[bufObj->Size - i - 1] = 123;
      }
   }
#endif

   return map;
}

void * GLAPIENTRY
_mesa_MapBufferRange_no_error(GLenum target, GLintptr offset,
                              GLsizeiptr length, GLbitfield access)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_buffer_object **bufObjPtr = get_buffer_target(ctx, target, true);
   struct gl_buffer_object *bufObj = *bufObjPtr;

   return map_buffer_range(ctx, bufObj, offset, length, access,
                           "glMapBufferRange");
}

void * GLAPIENTRY
_mesa_MapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length,
                     GLbitfield access)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;

   if (!ctx->Extensions.ARB_map_buffer_range) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glMapBufferRange(ARB_map_buffer_range not supported)");
      return NULL;
   }

   bufObj = get_buffer(ctx, "glMapBufferRange", target, GL_INVALID_OPERATION);
   if (!bufObj)
      return NULL;

   if (!validate_map_buffer_range(ctx, bufObj, offset, length, access,
                                  "glMapBufferRange"))
      return NULL;

   return map_buffer_range(ctx, bufObj, offset, length, access,
                           "glMapBufferRange");
}

void * GLAPIENTRY
_mesa_MapNamedBufferRange_no_error(GLuint buffer, GLintptr offset,
                                   GLsizeiptr length, GLbitfield access)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj = _mesa_lookup_bufferobj(ctx, buffer);

   return map_buffer_range(ctx, bufObj, offset, length, access,
                           "glMapNamedBufferRange");
}

static void *
map_named_buffer_range(GLuint buffer, GLintptr offset, GLsizeiptr length,
                       GLbitfield access, bool dsa_ext, const char *func)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj = NULL;

   if (!ctx->Extensions.ARB_map_buffer_range) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(ARB_map_buffer_range not supported)", func);
      return NULL;
   }

   if (dsa_ext) {
      bufObj = _mesa_lookup_bufferobj(ctx, buffer);
      if (!handle_bind_buffer_gen(ctx, buffer, &bufObj, func, false))
         return NULL;
   } else {
      bufObj = _mesa_lookup_bufferobj_err(ctx, buffer, func);
      if (!bufObj)
         return NULL;
   }

   if (!validate_map_buffer_range(ctx, bufObj, offset, length, access, func))
      return NULL;

   return map_buffer_range(ctx, bufObj, offset, length, access, func);
}

void * GLAPIENTRY
_mesa_MapNamedBufferRangeEXT(GLuint buffer, GLintptr offset, GLsizeiptr length,
                             GLbitfield access)
{
   GET_CURRENT_CONTEXT(ctx);
   if (!buffer) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glMapNamedBufferRangeEXT(buffer=0)");
      return NULL;
   }
   return map_named_buffer_range(buffer, offset, length, access, true,
                                 "glMapNamedBufferRangeEXT");
}

void * GLAPIENTRY
_mesa_MapNamedBufferRange(GLuint buffer, GLintptr offset, GLsizeiptr length,
                          GLbitfield access)
{
   return map_named_buffer_range(buffer, offset, length, access, false,
                                 "glMapNamedBufferRange");
}

/**
 * Converts GLenum access from MapBuffer and MapNamedBuffer into
 * flags for input to map_buffer_range.
 *
 * \return true if the type of requested access is permissible.
 */
static bool
get_map_buffer_access_flags(struct gl_context *ctx, GLenum access,
                            GLbitfield *flags)
{
   switch (access) {
   case GL_READ_ONLY_ARB:
      *flags = GL_MAP_READ_BIT;
      return _mesa_is_desktop_gl(ctx);
   case GL_WRITE_ONLY_ARB:
      *flags = GL_MAP_WRITE_BIT;
      return true;
   case GL_READ_WRITE_ARB:
      *flags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
      return _mesa_is_desktop_gl(ctx);
   default:
      *flags = 0;
      return false;
   }
}

void * GLAPIENTRY
_mesa_MapBuffer_no_error(GLenum target, GLenum access)
{
   GET_CURRENT_CONTEXT(ctx);

   GLbitfield accessFlags;
   get_map_buffer_access_flags(ctx, access, &accessFlags);

   struct gl_buffer_object **bufObjPtr = get_buffer_target(ctx, target, true);
   struct gl_buffer_object *bufObj = *bufObjPtr;

   return map_buffer_range(ctx, bufObj, 0, bufObj->Size, accessFlags,
                           "glMapBuffer");
}

void * GLAPIENTRY
_mesa_MapBuffer(GLenum target, GLenum access)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;
   GLbitfield accessFlags;

   if (!get_map_buffer_access_flags(ctx, access, &accessFlags)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glMapBuffer(invalid access)");
      return NULL;
   }

   bufObj = get_buffer(ctx, "glMapBuffer", target, GL_INVALID_OPERATION);
   if (!bufObj)
      return NULL;

   if (!validate_map_buffer_range(ctx, bufObj, 0, bufObj->Size, accessFlags,
                                  "glMapBuffer"))
      return NULL;

   return map_buffer_range(ctx, bufObj, 0, bufObj->Size, accessFlags,
                           "glMapBuffer");
}

void * GLAPIENTRY
_mesa_MapNamedBuffer_no_error(GLuint buffer, GLenum access)
{
   GET_CURRENT_CONTEXT(ctx);

   GLbitfield accessFlags;
   get_map_buffer_access_flags(ctx, access, &accessFlags);

   struct gl_buffer_object *bufObj = _mesa_lookup_bufferobj(ctx, buffer);

   return map_buffer_range(ctx, bufObj, 0, bufObj->Size, accessFlags,
                           "glMapNamedBuffer");
}

void * GLAPIENTRY
_mesa_MapNamedBuffer(GLuint buffer, GLenum access)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;
   GLbitfield accessFlags;

   if (!get_map_buffer_access_flags(ctx, access, &accessFlags)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glMapNamedBuffer(invalid access)");
      return NULL;
   }

   bufObj = _mesa_lookup_bufferobj_err(ctx, buffer, "glMapNamedBuffer");
   if (!bufObj)
      return NULL;

   if (!validate_map_buffer_range(ctx, bufObj, 0, bufObj->Size, accessFlags,
                                  "glMapNamedBuffer"))
      return NULL;

   return map_buffer_range(ctx, bufObj, 0, bufObj->Size, accessFlags,
                           "glMapNamedBuffer");
}

void * GLAPIENTRY
_mesa_MapNamedBufferEXT(GLuint buffer, GLenum access)
{
   GET_CURRENT_CONTEXT(ctx);

   GLbitfield accessFlags;
   if (!buffer) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glMapNamedBufferEXT(buffer=0)");
      return NULL;
   }
   if (!get_map_buffer_access_flags(ctx, access, &accessFlags)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glMapNamedBufferEXT(invalid access)");
      return NULL;
   }

   struct gl_buffer_object *bufObj = _mesa_lookup_bufferobj(ctx, buffer);
   if (!handle_bind_buffer_gen(ctx, buffer,
                               &bufObj, "glMapNamedBufferEXT", false))
      return NULL;

   if (!validate_map_buffer_range(ctx, bufObj, 0, bufObj->Size, accessFlags,
                                  "glMapNamedBufferEXT"))
      return NULL;

   return map_buffer_range(ctx, bufObj, 0, bufObj->Size, accessFlags,
                           "glMapNamedBufferEXT");
}

static void
flush_mapped_buffer_range(struct gl_context *ctx,
                          struct gl_buffer_object *bufObj,
                          GLintptr offset, GLsizeiptr length,
                          const char *func)
{
   if (!ctx->Extensions.ARB_map_buffer_range) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(ARB_map_buffer_range not supported)", func);
      return;
   }

   if (offset < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(offset %ld < 0)", func, (long) offset);
      return;
   }

   if (length < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(length %ld < 0)", func, (long) length);
      return;
   }

   if (!_mesa_bufferobj_mapped(bufObj, MAP_USER)) {
      /* buffer is not mapped */
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(buffer is not mapped)", func);
      return;
   }

   if ((bufObj->Mappings[MAP_USER].AccessFlags &
        GL_MAP_FLUSH_EXPLICIT_BIT) == 0) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(GL_MAP_FLUSH_EXPLICIT_BIT not set)", func);
      return;
   }

   if (offset + length > bufObj->Mappings[MAP_USER].Length) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(offset %ld + length %ld > mapped length %ld)", func,
                  (long) offset, (long) length,
                  (long) bufObj->Mappings[MAP_USER].Length);
      return;
   }

   assert(bufObj->Mappings[MAP_USER].AccessFlags & GL_MAP_WRITE_BIT);

   _mesa_bufferobj_flush_mapped_range(ctx, offset, length, bufObj,
                                      MAP_USER);
}

void GLAPIENTRY
_mesa_FlushMappedBufferRange_no_error(GLenum target, GLintptr offset,
                                      GLsizeiptr length)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object **bufObjPtr = get_buffer_target(ctx, target, true);
   struct gl_buffer_object *bufObj = *bufObjPtr;

   _mesa_bufferobj_flush_mapped_range(ctx, offset, length, bufObj,
                                      MAP_USER);
}

void GLAPIENTRY
_mesa_FlushMappedBufferRange(GLenum target, GLintptr offset,
                             GLsizeiptr length)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;

   bufObj = get_buffer(ctx, "glFlushMappedBufferRange", target,
                       GL_INVALID_OPERATION);
   if (!bufObj)
      return;

   flush_mapped_buffer_range(ctx, bufObj, offset, length,
                             "glFlushMappedBufferRange");
}

void GLAPIENTRY
_mesa_FlushMappedNamedBufferRange_no_error(GLuint buffer, GLintptr offset,
                                           GLsizeiptr length)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj = _mesa_lookup_bufferobj(ctx, buffer);

   _mesa_bufferobj_flush_mapped_range(ctx, offset, length, bufObj,
                                      MAP_USER);
}

void GLAPIENTRY
_mesa_FlushMappedNamedBufferRange(GLuint buffer, GLintptr offset,
                                  GLsizeiptr length)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;

   bufObj = _mesa_lookup_bufferobj_err(ctx, buffer,
                                       "glFlushMappedNamedBufferRange");
   if (!bufObj)
      return;

   flush_mapped_buffer_range(ctx, bufObj, offset, length,
                             "glFlushMappedNamedBufferRange");
}

void GLAPIENTRY
_mesa_FlushMappedNamedBufferRangeEXT(GLuint buffer, GLintptr offset,
                                     GLsizeiptr length)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;

   if (!buffer) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glFlushMappedNamedBufferRangeEXT(buffer=0)");
      return;
   }

   bufObj = _mesa_lookup_bufferobj(ctx, buffer);
   if (!handle_bind_buffer_gen(ctx, buffer,
                               &bufObj, "glFlushMappedNamedBufferRangeEXT", false))
      return;

   flush_mapped_buffer_range(ctx, bufObj, offset, length,
                             "glFlushMappedNamedBufferRangeEXT");
}

static void
bind_buffer_range_uniform_buffer(struct gl_context *ctx, GLuint index,
                                 struct gl_buffer_object *bufObj,
                                 GLintptr offset, GLsizeiptr size)
{
   if (!bufObj) {
      offset = -1;
      size = -1;
   }

   _mesa_reference_buffer_object(ctx, &ctx->UniformBuffer, bufObj);
   bind_uniform_buffer(ctx, index, bufObj, offset, size, GL_FALSE);
}

/**
 * Bind a region of a buffer object to a uniform block binding point.
 * \param index  the uniform buffer binding point index
 * \param bufObj  the buffer object
 * \param offset  offset to the start of buffer object region
 * \param size  size of the buffer object region
 */
static void
bind_buffer_range_uniform_buffer_err(struct gl_context *ctx, GLuint index,
                                     struct gl_buffer_object *bufObj,
                                     GLintptr offset, GLsizeiptr size)
{
   if (index >= ctx->Const.MaxUniformBufferBindings) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glBindBufferRange(index=%d)", index);
      return;
   }

   if (offset & (ctx->Const.UniformBufferOffsetAlignment - 1)) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glBindBufferRange(offset misaligned %d/%d)", (int) offset,
		  ctx->Const.UniformBufferOffsetAlignment);
      return;
   }

   bind_buffer_range_uniform_buffer(ctx, index, bufObj, offset, size);
}

static void
bind_buffer_range_shader_storage_buffer(struct gl_context *ctx,
                                        GLuint index,
                                        struct gl_buffer_object *bufObj,
                                        GLintptr offset,
                                        GLsizeiptr size)
{
   if (!bufObj) {
      offset = -1;
      size = -1;
   }

   _mesa_reference_buffer_object(ctx, &ctx->ShaderStorageBuffer, bufObj);
   bind_shader_storage_buffer(ctx, index, bufObj, offset, size, GL_FALSE);
}

/**
 * Bind a region of a buffer object to a shader storage block binding point.
 * \param index  the shader storage buffer binding point index
 * \param bufObj  the buffer object
 * \param offset  offset to the start of buffer object region
 * \param size  size of the buffer object region
 */
static void
bind_buffer_range_shader_storage_buffer_err(struct gl_context *ctx,
                                            GLuint index,
                                            struct gl_buffer_object *bufObj,
                                            GLintptr offset, GLsizeiptr size)
{
   if (index >= ctx->Const.MaxShaderStorageBufferBindings) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glBindBufferRange(index=%d)", index);
      return;
   }

   if (offset & (ctx->Const.ShaderStorageBufferOffsetAlignment - 1)) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glBindBufferRange(offset misaligned %d/%d)", (int) offset,
                  ctx->Const.ShaderStorageBufferOffsetAlignment);
      return;
   }

   bind_buffer_range_shader_storage_buffer(ctx, index, bufObj, offset, size);
}

static void
bind_buffer_range_atomic_buffer(struct gl_context *ctx, GLuint index,
                                 struct gl_buffer_object *bufObj,
                                 GLintptr offset, GLsizeiptr size)
{
   if (!bufObj) {
      offset = -1;
      size = -1;
   }

   _mesa_reference_buffer_object(ctx, &ctx->AtomicBuffer, bufObj);
   bind_atomic_buffer(ctx, index, bufObj, offset, size, GL_FALSE);
}

/**
 * Bind a region of a buffer object to an atomic storage block binding point.
 * \param index  the shader storage buffer binding point index
 * \param bufObj  the buffer object
 * \param offset  offset to the start of buffer object region
 * \param size  size of the buffer object region
 */
static void
bind_buffer_range_atomic_buffer_err(struct gl_context *ctx,
                                    GLuint index,
                                    struct gl_buffer_object *bufObj,
                                    GLintptr offset, GLsizeiptr size)
{
   if (index >= ctx->Const.MaxAtomicBufferBindings) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glBindBufferRange(index=%d)", index);
      return;
   }

   if (offset & (ATOMIC_COUNTER_SIZE - 1)) {
      _mesa_error(ctx, GL_INVALID_VALUE,
		  "glBindBufferRange(offset misaligned %d/%d)", (int) offset,
		  ATOMIC_COUNTER_SIZE);
      return;
   }

   bind_buffer_range_atomic_buffer(ctx, index, bufObj, offset, size);
}

static inline bool
bind_buffers_check_offset_and_size(struct gl_context *ctx,
                                   GLuint index,
                                   const GLintptr *offsets,
                                   const GLsizeiptr *sizes)
{
   if (offsets[index] < 0) {
      /* The ARB_multi_bind spec says:
       *
       *    "An INVALID_VALUE error is generated by BindBuffersRange if any
       *     value in <offsets> is less than zero (per binding)."
       */
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glBindBuffersRange(offsets[%u]=%" PRId64 " < 0)",
                  index, (int64_t) offsets[index]);
      return false;
   }

   if (sizes[index] <= 0) {
      /* The ARB_multi_bind spec says:
       *
       *     "An INVALID_VALUE error is generated by BindBuffersRange if any
       *      value in <sizes> is less than or equal to zero (per binding)."
       */
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glBindBuffersRange(sizes[%u]=%" PRId64 " <= 0)",
                  index, (int64_t) sizes[index]);
      return false;
   }

   return true;
}

static bool
error_check_bind_uniform_buffers(struct gl_context *ctx,
                                 GLuint first, GLsizei count,
                                 const char *caller)
{
   if (!ctx->Extensions.ARB_uniform_buffer_object) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "%s(target=GL_UNIFORM_BUFFER)", caller);
      return false;
   }

   /* The ARB_multi_bind_spec says:
    *
    *     "An INVALID_OPERATION error is generated if <first> + <count> is
    *      greater than the number of target-specific indexed binding points,
    *      as described in section 6.7.1."
    */
   if (first + count > ctx->Const.MaxUniformBufferBindings) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(first=%u + count=%d > the value of "
                  "GL_MAX_UNIFORM_BUFFER_BINDINGS=%u)",
                  caller, first, count,
                  ctx->Const.MaxUniformBufferBindings);
      return false;
   }

   return true;
}

static bool
error_check_bind_shader_storage_buffers(struct gl_context *ctx,
                                        GLuint first, GLsizei count,
                                        const char *caller)
{
   if (!ctx->Extensions.ARB_shader_storage_buffer_object) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "%s(target=GL_SHADER_STORAGE_BUFFER)", caller);
      return false;
   }

   /* The ARB_multi_bind_spec says:
    *
    *     "An INVALID_OPERATION error is generated if <first> + <count> is
    *      greater than the number of target-specific indexed binding points,
    *      as described in section 6.7.1."
    */
   if (first + count > ctx->Const.MaxShaderStorageBufferBindings) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(first=%u + count=%d > the value of "
                  "GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS=%u)",
                  caller, first, count,
                  ctx->Const.MaxShaderStorageBufferBindings);
      return false;
   }

   return true;
}

/**
 * Unbind all uniform buffers in the range
 * <first> through <first>+<count>-1
 */
static void
unbind_uniform_buffers(struct gl_context *ctx, GLuint first, GLsizei count)
{
   for (int i = 0; i < count; i++)
      set_buffer_binding(ctx, &ctx->UniformBufferBindings[first + i],
                         NULL, -1, -1, GL_TRUE, 0);
}

/**
 * Unbind all shader storage buffers in the range
 * <first> through <first>+<count>-1
 */
static void
unbind_shader_storage_buffers(struct gl_context *ctx, GLuint first,
                              GLsizei count)
{
   for (int i = 0; i < count; i++)
      set_buffer_binding(ctx, &ctx->ShaderStorageBufferBindings[first + i],
                         NULL, -1, -1, GL_TRUE, 0);
}

static void
bind_uniform_buffers(struct gl_context *ctx, GLuint first, GLsizei count,
                     const GLuint *buffers,
                     bool range,
                     const GLintptr *offsets, const GLsizeiptr *sizes,
                     const char *caller)
{
   if (!error_check_bind_uniform_buffers(ctx, first, count, caller))
      return;

   /* Assume that at least one binding will be changed */
   FLUSH_VERTICES(ctx, 0, 0);
   ctx->NewDriverState |= ST_NEW_UNIFORM_BUFFER;

   if (!buffers) {
      /* The ARB_multi_bind spec says:
       *
       *    "If <buffers> is NULL, all bindings from <first> through
       *     <first>+<count>-1 are reset to their unbound (zero) state.
       *     In this case, the offsets and sizes associated with the
       *     binding points are set to default values, ignoring
       *     <offsets> and <sizes>."
       */
      unbind_uniform_buffers(ctx, first, count);
      return;
   }

   /* Note that the error semantics for multi-bind commands differ from
    * those of other GL commands.
    *
    * The Issues section in the ARB_multi_bind spec says:
    *
    *    "(11) Typically, OpenGL specifies that if an error is generated by a
    *          command, that command has no effect.  This is somewhat
    *          unfortunate for multi-bind commands, because it would require a
    *          first pass to scan the entire list of bound objects for errors
    *          and then a second pass to actually perform the bindings.
    *          Should we have different error semantics?
    *
    *       RESOLVED:  Yes.  In this specification, when the parameters for
    *       one of the <count> binding points are invalid, that binding point
    *       is not updated and an error will be generated.  However, other
    *       binding points in the same command will be updated if their
    *       parameters are valid and no other error occurs."
    */

   _mesa_HashLockMaybeLocked(ctx->Shared->BufferObjects,
                             ctx->BufferObjectsLocked);

   for (int i = 0; i < count; i++) {
      struct gl_buffer_binding *binding =
         &ctx->UniformBufferBindings[first + i];
      GLintptr offset = 0;
      GLsizeiptr size = 0;

      if (range) {
         if (!bind_buffers_check_offset_and_size(ctx, i, offsets, sizes))
            continue;

         /* The ARB_multi_bind spec says:
          *
          *     "An INVALID_VALUE error is generated by BindBuffersRange if any
          *      pair of values in <offsets> and <sizes> does not respectively
          *      satisfy the constraints described for those parameters for the
          *      specified target, as described in section 6.7.1 (per binding)."
          *
          * Section 6.7.1 refers to table 6.5, which says:
          *
          *     "┌───────────────────────────────────────────────────────────────┐
          *      │ Uniform buffer array bindings (see sec. 7.6)                  │
          *      ├─────────────────────┬─────────────────────────────────────────┤
          *      │  ...                │  ...                                    │
          *      │  offset restriction │  multiple of value of UNIFORM_BUFFER_-  │
          *      │                     │  OFFSET_ALIGNMENT                       │
          *      │  ...                │  ...                                    │
          *      │  size restriction   │  none                                   │
          *      └─────────────────────┴─────────────────────────────────────────┘"
          */
         if (offsets[i] & (ctx->Const.UniformBufferOffsetAlignment - 1)) {
            _mesa_error(ctx, GL_INVALID_VALUE,
                        "glBindBuffersRange(offsets[%u]=%" PRId64
                        " is misaligned; it must be a multiple of the value of "
                        "GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT=%u when "
                        "target=GL_UNIFORM_BUFFER)",
                        i, (int64_t) offsets[i],
                        ctx->Const.UniformBufferOffsetAlignment);
            continue;
         }

         offset = offsets[i];
         size = sizes[i];
      }

      set_buffer_multi_binding(ctx, buffers, i, caller,
                               binding, offset, size, range,
                               USAGE_UNIFORM_BUFFER);
   }

   _mesa_HashUnlockMaybeLocked(ctx->Shared->BufferObjects,
                               ctx->BufferObjectsLocked);
}

static void
bind_shader_storage_buffers(struct gl_context *ctx, GLuint first,
                            GLsizei count, const GLuint *buffers,
                            bool range,
                            const GLintptr *offsets,
                            const GLsizeiptr *sizes,
                            const char *caller)
{
   if (!error_check_bind_shader_storage_buffers(ctx, first, count, caller))
      return;

   /* Assume that at least one binding will be changed */
   FLUSH_VERTICES(ctx, 0, 0);
   ctx->NewDriverState |= ST_NEW_STORAGE_BUFFER;

   if (!buffers) {
      /* The ARB_multi_bind spec says:
       *
       *    "If <buffers> is NULL, all bindings from <first> through
       *     <first>+<count>-1 are reset to their unbound (zero) state.
       *     In this case, the offsets and sizes associated with the
       *     binding points are set to default values, ignoring
       *     <offsets> and <sizes>."
       */
      unbind_shader_storage_buffers(ctx, first, count);
      return;
   }

   /* Note that the error semantics for multi-bind commands differ from
    * those of other GL commands.
    *
    * The Issues section in the ARB_multi_bind spec says:
    *
    *    "(11) Typically, OpenGL specifies that if an error is generated by a
    *          command, that command has no effect.  This is somewhat
    *          unfortunate for multi-bind commands, because it would require a
    *          first pass to scan the entire list of bound objects for errors
    *          and then a second pass to actually perform the bindings.
    *          Should we have different error semantics?
    *
    *       RESOLVED:  Yes.  In this specification, when the parameters for
    *       one of the <count> binding points are invalid, that binding point
    *       is not updated and an error will be generated.  However, other
    *       binding points in the same command will be updated if their
    *       parameters are valid and no other error occurs."
    */

   _mesa_HashLockMaybeLocked(ctx->Shared->BufferObjects,
                             ctx->BufferObjectsLocked);

   for (int i = 0; i < count; i++) {
      struct gl_buffer_binding *binding =
         &ctx->ShaderStorageBufferBindings[first + i];
      GLintptr offset = 0;
      GLsizeiptr size = 0;

      if (range) {
         if (!bind_buffers_check_offset_and_size(ctx, i, offsets, sizes))
            continue;

         /* The ARB_multi_bind spec says:
         *
         *     "An INVALID_VALUE error is generated by BindBuffersRange if any
         *      pair of values in <offsets> and <sizes> does not respectively
         *      satisfy the constraints described for those parameters for the
         *      specified target, as described in section 6.7.1 (per binding)."
         *
         * Section 6.7.1 refers to table 6.5, which says:
         *
         *     "┌───────────────────────────────────────────────────────────────┐
         *      │ Shader storage buffer array bindings (see sec. 7.8)           │
         *      ├─────────────────────┬─────────────────────────────────────────┤
         *      │  ...                │  ...                                    │
         *      │  offset restriction │  multiple of value of SHADER_STORAGE_-  │
         *      │                     │  BUFFER_OFFSET_ALIGNMENT                │
         *      │  ...                │  ...                                    │
         *      │  size restriction   │  none                                   │
         *      └─────────────────────┴─────────────────────────────────────────┘"
         */
         if (offsets[i] & (ctx->Const.ShaderStorageBufferOffsetAlignment - 1)) {
            _mesa_error(ctx, GL_INVALID_VALUE,
                        "glBindBuffersRange(offsets[%u]=%" PRId64
                        " is misaligned; it must be a multiple of the value of "
                        "GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT=%u when "
                        "target=GL_SHADER_STORAGE_BUFFER)",
                        i, (int64_t) offsets[i],
                        ctx->Const.ShaderStorageBufferOffsetAlignment);
            continue;
         }

         offset = offsets[i];
         size = sizes[i];
      }

      set_buffer_multi_binding(ctx, buffers, i, caller,
                               binding, offset, size, range,
                               USAGE_SHADER_STORAGE_BUFFER);
   }

   _mesa_HashUnlockMaybeLocked(ctx->Shared->BufferObjects,
                               ctx->BufferObjectsLocked);
}

static bool
error_check_bind_xfb_buffers(struct gl_context *ctx,
                             struct gl_transform_feedback_object *tfObj,
                             GLuint first, GLsizei count, const char *caller)
{
   if (!ctx->Extensions.EXT_transform_feedback) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "%s(target=GL_TRANSFORM_FEEDBACK_BUFFER)", caller);
      return false;
   }

   /* Page 398 of the PDF of the OpenGL 4.4 (Core Profile) spec says:
    *
    *     "An INVALID_OPERATION error is generated :
    *
    *     ...
    *     • by BindBufferRange or BindBufferBase if target is TRANSFORM_-
    *       FEEDBACK_BUFFER and transform feedback is currently active."
    *
    * We assume that this is also meant to apply to BindBuffersRange
    * and BindBuffersBase.
    */
   if (tfObj->Active) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(Changing transform feedback buffers while "
                  "transform feedback is active)", caller);
      return false;
   }

   /* The ARB_multi_bind_spec says:
    *
    *     "An INVALID_OPERATION error is generated if <first> + <count> is
    *      greater than the number of target-specific indexed binding points,
    *      as described in section 6.7.1."
    */
   if (first + count > ctx->Const.MaxTransformFeedbackBuffers) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(first=%u + count=%d > the value of "
                  "GL_MAX_TRANSFORM_FEEDBACK_BUFFERS=%u)",
                  caller, first, count,
                  ctx->Const.MaxTransformFeedbackBuffers);
      return false;
   }

   return true;
}

/**
 * Unbind all transform feedback buffers in the range
 * <first> through <first>+<count>-1
 */
static void
unbind_xfb_buffers(struct gl_context *ctx,
                   struct gl_transform_feedback_object *tfObj,
                   GLuint first, GLsizei count)
{
   for (int i = 0; i < count; i++)
      _mesa_set_transform_feedback_binding(ctx, tfObj, first + i,
                                           NULL, 0, 0);
}

static void
bind_xfb_buffers(struct gl_context *ctx,
                 GLuint first, GLsizei count,
                 const GLuint *buffers,
                 bool range,
                 const GLintptr *offsets,
                 const GLsizeiptr *sizes,
                 const char *caller)
{
   struct gl_transform_feedback_object *tfObj =
       ctx->TransformFeedback.CurrentObject;

   if (!error_check_bind_xfb_buffers(ctx, tfObj, first, count, caller))
      return;

   /* Assume that at least one binding will be changed */
   FLUSH_VERTICES(ctx, 0, 0);

   if (!buffers) {
      /* The ARB_multi_bind spec says:
       *
       *    "If <buffers> is NULL, all bindings from <first> through
       *     <first>+<count>-1 are reset to their unbound (zero) state.
       *     In this case, the offsets and sizes associated with the
       *     binding points are set to default values, ignoring
       *     <offsets> and <sizes>."
       */
      unbind_xfb_buffers(ctx, tfObj, first, count);
      return;
   }

   /* Note that the error semantics for multi-bind commands differ from
    * those of other GL commands.
    *
    * The Issues section in the ARB_multi_bind spec says:
    *
    *    "(11) Typically, OpenGL specifies that if an error is generated by a
    *          command, that command has no effect.  This is somewhat
    *          unfortunate for multi-bind commands, because it would require a
    *          first pass to scan the entire list of bound objects for errors
    *          and then a second pass to actually perform the bindings.
    *          Should we have different error semantics?
    *
    *       RESOLVED:  Yes.  In this specification, when the parameters for
    *       one of the <count> binding points are invalid, that binding point
    *       is not updated and an error will be generated.  However, other
    *       binding points in the same command will be updated if their
    *       parameters are valid and no other error occurs."
    */

   _mesa_HashLockMaybeLocked(ctx->Shared->BufferObjects,
                             ctx->BufferObjectsLocked);

   for (int i = 0; i < count; i++) {
      const GLuint index = first + i;
      struct gl_buffer_object * const boundBufObj = tfObj->Buffers[index];
      struct gl_buffer_object *bufObj;
      GLintptr offset = 0;
      GLsizeiptr size = 0;

      if (range) {
         if (!bind_buffers_check_offset_and_size(ctx, i, offsets, sizes))
            continue;

         /* The ARB_multi_bind spec says:
          *
          *     "An INVALID_VALUE error is generated by BindBuffersRange if any
          *      pair of values in <offsets> and <sizes> does not respectively
          *      satisfy the constraints described for those parameters for the
          *      specified target, as described in section 6.7.1 (per binding)."
          *
          * Section 6.7.1 refers to table 6.5, which says:
          *
          *     "┌───────────────────────────────────────────────────────────────┐
          *      │ Transform feedback array bindings (see sec. 13.2.2)           │
          *      ├───────────────────────┬───────────────────────────────────────┤
          *      │    ...                │    ...                                │
          *      │    offset restriction │    multiple of 4                      │
          *      │    ...                │    ...                                │
          *      │    size restriction   │    multiple of 4                      │
          *      └───────────────────────┴───────────────────────────────────────┘"
          */
         if (offsets[i] & 0x3) {
            _mesa_error(ctx, GL_INVALID_VALUE,
                        "glBindBuffersRange(offsets[%u]=%" PRId64
                        " is misaligned; it must be a multiple of 4 when "
                        "target=GL_TRANSFORM_FEEDBACK_BUFFER)",
                        i, (int64_t) offsets[i]);
            continue;
         }

         if (sizes[i] & 0x3) {
            _mesa_error(ctx, GL_INVALID_VALUE,
                        "glBindBuffersRange(sizes[%u]=%" PRId64
                        " is misaligned; it must be a multiple of 4 when "
                        "target=GL_TRANSFORM_FEEDBACK_BUFFER)",
                        i, (int64_t) sizes[i]);
            continue;
         }

         offset = offsets[i];
         size = sizes[i];
      }

      if (boundBufObj && boundBufObj->Name == buffers[i])
         bufObj = boundBufObj;
      else {
         bool error;
         bufObj = _mesa_multi_bind_lookup_bufferobj(ctx, buffers, i, caller,
                                                    &error);
         if (error)
            continue;
      }

      _mesa_set_transform_feedback_binding(ctx, tfObj, index, bufObj,
                                           offset, size);
   }

   _mesa_HashUnlockMaybeLocked(ctx->Shared->BufferObjects,
                               ctx->BufferObjectsLocked);
}

static bool
error_check_bind_atomic_buffers(struct gl_context *ctx,
                                GLuint first, GLsizei count,
                                const char *caller)
{
   if (!ctx->Extensions.ARB_shader_atomic_counters) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "%s(target=GL_ATOMIC_COUNTER_BUFFER)", caller);
      return false;
   }

   /* The ARB_multi_bind_spec says:
    *
    *     "An INVALID_OPERATION error is generated if <first> + <count> is
    *      greater than the number of target-specific indexed binding points,
    *      as described in section 6.7.1."
    */
   if (first + count > ctx->Const.MaxAtomicBufferBindings) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(first=%u + count=%d > the value of "
                  "GL_MAX_ATOMIC_BUFFER_BINDINGS=%u)",
                  caller, first, count, ctx->Const.MaxAtomicBufferBindings);
      return false;
   }

   return true;
}

/**
 * Unbind all atomic counter buffers in the range
 * <first> through <first>+<count>-1
 */
static void
unbind_atomic_buffers(struct gl_context *ctx, GLuint first, GLsizei count)
{
   for (int i = 0; i < count; i++)
      set_buffer_binding(ctx, &ctx->AtomicBufferBindings[first + i],
                         NULL, -1, -1, GL_TRUE, 0);
}

static void
bind_atomic_buffers(struct gl_context *ctx,
                    GLuint first,
                    GLsizei count,
                    const GLuint *buffers,
                    bool range,
                    const GLintptr *offsets,
                    const GLsizeiptr *sizes,
                    const char *caller)
{
   if (!error_check_bind_atomic_buffers(ctx, first, count, caller))
     return;

   /* Assume that at least one binding will be changed */
   FLUSH_VERTICES(ctx, 0, 0);
   ctx->NewDriverState |= ctx->DriverFlags.NewAtomicBuffer;

   if (!buffers) {
      /* The ARB_multi_bind spec says:
       *
       *    "If <buffers> is NULL, all bindings from <first> through
       *     <first>+<count>-1 are reset to their unbound (zero) state.
       *     In this case, the offsets and sizes associated with the
       *     binding points are set to default values, ignoring
       *     <offsets> and <sizes>."
       */
      unbind_atomic_buffers(ctx, first, count);
      return;
   }

   /* Note that the error semantics for multi-bind commands differ from
    * those of other GL commands.
    *
    * The Issues section in the ARB_multi_bind spec says:
    *
    *    "(11) Typically, OpenGL specifies that if an error is generated by a
    *          command, that command has no effect.  This is somewhat
    *          unfortunate for multi-bind commands, because it would require a
    *          first pass to scan the entire list of bound objects for errors
    *          and then a second pass to actually perform the bindings.
    *          Should we have different error semantics?
    *
    *       RESOLVED:  Yes.  In this specification, when the parameters for
    *       one of the <count> binding points are invalid, that binding point
    *       is not updated and an error will be generated.  However, other
    *       binding points in the same command will be updated if their
    *       parameters are valid and no other error occurs."
    */

   _mesa_HashLockMaybeLocked(ctx->Shared->BufferObjects,
                             ctx->BufferObjectsLocked);

   for (int i = 0; i < count; i++) {
      struct gl_buffer_binding *binding =
         &ctx->AtomicBufferBindings[first + i];
      GLintptr offset = 0;
      GLsizeiptr size = 0;

      if (range) {
         if (!bind_buffers_check_offset_and_size(ctx, i, offsets, sizes))
            continue;

         /* The ARB_multi_bind spec says:
          *
          *     "An INVALID_VALUE error is generated by BindBuffersRange if any
          *      pair of values in <offsets> and <sizes> does not respectively
          *      satisfy the constraints described for those parameters for the
          *      specified target, as described in section 6.7.1 (per binding)."
          *
          * Section 6.7.1 refers to table 6.5, which says:
          *
          *     "┌───────────────────────────────────────────────────────────────┐
          *      │ Atomic counter array bindings (see sec. 7.7.2)                │
          *      ├───────────────────────┬───────────────────────────────────────┤
          *      │    ...                │    ...                                │
          *      │    offset restriction │    multiple of 4                      │
          *      │    ...                │    ...                                │
          *      │    size restriction   │    none                               │
          *      └───────────────────────┴───────────────────────────────────────┘"
          */
         if (offsets[i] & (ATOMIC_COUNTER_SIZE - 1)) {
            _mesa_error(ctx, GL_INVALID_VALUE,
                        "glBindBuffersRange(offsets[%u]=%" PRId64
                        " is misaligned; it must be a multiple of %d when "
                        "target=GL_ATOMIC_COUNTER_BUFFER)",
                        i, (int64_t) offsets[i], ATOMIC_COUNTER_SIZE);
            continue;
         }

         offset = offsets[i];
         size = sizes[i];
      }

      set_buffer_multi_binding(ctx, buffers, i, caller,
                               binding, offset, size, range,
                               USAGE_ATOMIC_COUNTER_BUFFER);
   }

   _mesa_HashUnlockMaybeLocked(ctx->Shared->BufferObjects,
                               ctx->BufferObjectsLocked);
}

static ALWAYS_INLINE void
bind_buffer_range(GLenum target, GLuint index, GLuint buffer, GLintptr offset,
                  GLsizeiptr size, bool no_error)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;

   if (MESA_VERBOSE & VERBOSE_API) {
      _mesa_debug(ctx, "glBindBufferRange(%s, %u, %u, %lu, %lu)\n",
                  _mesa_enum_to_string(target), index, buffer,
                  (unsigned long) offset, (unsigned long) size);
   }

   if (buffer == 0) {
      bufObj = NULL;
   } else {
      bufObj = _mesa_lookup_bufferobj(ctx, buffer);
      if (!handle_bind_buffer_gen(ctx, buffer,
                                  &bufObj, "glBindBufferRange", no_error))
         return;
   }

   if (no_error) {
      switch (target) {
      case GL_TRANSFORM_FEEDBACK_BUFFER:
         _mesa_bind_buffer_range_xfb(ctx, ctx->TransformFeedback.CurrentObject,
                                     index, bufObj, offset, size);
         return;
      case GL_UNIFORM_BUFFER:
         bind_buffer_range_uniform_buffer(ctx, index, bufObj, offset, size);
         return;
      case GL_SHADER_STORAGE_BUFFER:
         bind_buffer_range_shader_storage_buffer(ctx, index, bufObj, offset,
                                                 size);
         return;
      case GL_ATOMIC_COUNTER_BUFFER:
         bind_buffer_range_atomic_buffer(ctx, index, bufObj, offset, size);
         return;
      default:
         unreachable("invalid BindBufferRange target with KHR_no_error");
      }
   } else {
      if (buffer != 0) {
         if (size <= 0) {
            _mesa_error(ctx, GL_INVALID_VALUE, "glBindBufferRange(size=%d)",
                        (int) size);
            return;
         }
      }

      switch (target) {
      case GL_TRANSFORM_FEEDBACK_BUFFER:
         if (!_mesa_validate_buffer_range_xfb(ctx,
                                              ctx->TransformFeedback.CurrentObject,
                                              index, bufObj, offset, size,
                                              false))
            return;

         _mesa_bind_buffer_range_xfb(ctx, ctx->TransformFeedback.CurrentObject,
                                     index, bufObj, offset, size);
         return;
      case GL_UNIFORM_BUFFER:
         bind_buffer_range_uniform_buffer_err(ctx, index, bufObj, offset,
                                              size);
         return;
      case GL_SHADER_STORAGE_BUFFER:
         bind_buffer_range_shader_storage_buffer_err(ctx, index, bufObj,
                                                     offset, size);
         return;
      case GL_ATOMIC_COUNTER_BUFFER:
         bind_buffer_range_atomic_buffer_err(ctx, index, bufObj,
                                             offset, size);
         return;
      default:
         _mesa_error(ctx, GL_INVALID_ENUM, "glBindBufferRange(target)");
         return;
      }
   }
}

void GLAPIENTRY
_mesa_BindBufferRange_no_error(GLenum target, GLuint index, GLuint buffer,
                               GLintptr offset, GLsizeiptr size)
{
   bind_buffer_range(target, index, buffer, offset, size, true);
}

void GLAPIENTRY
_mesa_BindBufferRange(GLenum target, GLuint index,
                      GLuint buffer, GLintptr offset, GLsizeiptr size)
{
   bind_buffer_range(target, index, buffer, offset, size, false);
}

void GLAPIENTRY
_mesa_BindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;

   if (MESA_VERBOSE & VERBOSE_API) {
      _mesa_debug(ctx, "glBindBufferBase(%s, %u, %u)\n",
                  _mesa_enum_to_string(target), index, buffer);
   }

   if (buffer == 0) {
      bufObj = NULL;
   } else {
      bufObj = _mesa_lookup_bufferobj(ctx, buffer);
      if (!handle_bind_buffer_gen(ctx, buffer,
                                  &bufObj, "glBindBufferBase", false))
         return;
   }

   /* Note that there's some oddness in the GL 3.1-GL 3.3 specifications with
    * regards to BindBufferBase.  It says (GL 3.1 core spec, page 63):
    *
    *     "BindBufferBase is equivalent to calling BindBufferRange with offset
    *      zero and size equal to the size of buffer."
    *
    * but it says for glGetIntegeri_v (GL 3.1 core spec, page 230):
    *
    *     "If the parameter (starting offset or size) was not specified when the
    *      buffer object was bound, zero is returned."
    *
    * What happens if the size of the buffer changes?  Does the size of the
    * buffer at the moment glBindBufferBase was called still play a role, like
    * the first quote would imply, or is the size meaningless in the
    * glBindBufferBase case like the second quote would suggest?  The GL 4.1
    * core spec page 45 says:
    *
    *     "It is equivalent to calling BindBufferRange with offset zero, while
    *      size is determined by the size of the bound buffer at the time the
    *      binding is used."
    *
    * My interpretation is that the GL 4.1 spec was a clarification of the
    * behavior, not a change.  In particular, this choice will only make
    * rendering work in cases where it would have had undefined results.
    */

   switch (target) {
   case GL_TRANSFORM_FEEDBACK_BUFFER:
      _mesa_bind_buffer_base_transform_feedback(ctx,
                                                ctx->TransformFeedback.CurrentObject,
                                                index, bufObj, false);
      return;
   case GL_UNIFORM_BUFFER:
      bind_buffer_base_uniform_buffer(ctx, index, bufObj);
      return;
   case GL_SHADER_STORAGE_BUFFER:
      bind_buffer_base_shader_storage_buffer(ctx, index, bufObj);
      return;
   case GL_ATOMIC_COUNTER_BUFFER:
      bind_buffer_base_atomic_buffer(ctx, index, bufObj);
      return;
   default:
      _mesa_error(ctx, GL_INVALID_ENUM, "glBindBufferBase(target)");
      return;
   }
}

void GLAPIENTRY
_mesa_BindBuffersRange(GLenum target, GLuint first, GLsizei count,
                       const GLuint *buffers,
                       const GLintptr *offsets, const GLsizeiptr *sizes)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API) {
      _mesa_debug(ctx, "glBindBuffersRange(%s, %u, %d, %p, %p, %p)\n",
                  _mesa_enum_to_string(target), first, count,
                  buffers, offsets, sizes);
   }

   switch (target) {
   case GL_TRANSFORM_FEEDBACK_BUFFER:
      bind_xfb_buffers(ctx, first, count, buffers, true, offsets, sizes,
                       "glBindBuffersRange");
      return;
   case GL_UNIFORM_BUFFER:
      bind_uniform_buffers(ctx, first, count, buffers, true, offsets, sizes,
                           "glBindBuffersRange");
      return;
   case GL_SHADER_STORAGE_BUFFER:
      bind_shader_storage_buffers(ctx, first, count, buffers, true, offsets, sizes,
                                  "glBindBuffersRange");
      return;
   case GL_ATOMIC_COUNTER_BUFFER:
      bind_atomic_buffers(ctx, first, count, buffers, true, offsets, sizes,
                          "glBindBuffersRange");
      return;
   default:
      _mesa_error(ctx, GL_INVALID_ENUM, "glBindBuffersRange(target=%s)",
                  _mesa_enum_to_string(target));
      break;
   }
}

void GLAPIENTRY
_mesa_BindBuffersBase(GLenum target, GLuint first, GLsizei count,
                      const GLuint *buffers)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API) {
      _mesa_debug(ctx, "glBindBuffersBase(%s, %u, %d, %p)\n",
                  _mesa_enum_to_string(target), first, count, buffers);
   }

   switch (target) {
   case GL_TRANSFORM_FEEDBACK_BUFFER:
      bind_xfb_buffers(ctx, first, count, buffers, false, NULL, NULL,
                       "glBindBuffersBase");
      return;
   case GL_UNIFORM_BUFFER:
      bind_uniform_buffers(ctx, first, count, buffers, false, NULL, NULL,
                           "glBindBuffersBase");
      return;
   case GL_SHADER_STORAGE_BUFFER:
      bind_shader_storage_buffers(ctx, first, count, buffers, false, NULL, NULL,
                                  "glBindBuffersBase");
      return;
   case GL_ATOMIC_COUNTER_BUFFER:
      bind_atomic_buffers(ctx, first, count, buffers, false, NULL, NULL,
                          "glBindBuffersBase");
      return;
   default:
      _mesa_error(ctx, GL_INVALID_ENUM, "glBindBuffersBase(target=%s)",
                  _mesa_enum_to_string(target));
      break;
   }
}

/**
 * Called via glInvalidateBuffer(Sub)Data.
 */
static void
bufferobj_invalidate(struct gl_context *ctx,
                     struct gl_buffer_object *obj,
                     GLintptr offset,
                     GLsizeiptr size)
{
   struct pipe_context *pipe = ctx->pipe;

   /* We ignore partial invalidates. */
   if (offset != 0 || size != obj->Size)
      return;

   /* If the buffer is mapped, we can't invalidate it. */
   if (!obj->buffer || _mesa_bufferobj_mapped(obj, MAP_USER))
      return;

   pipe->invalidate_resource(pipe, obj->buffer);
}

static ALWAYS_INLINE void
invalidate_buffer_subdata(struct gl_context *ctx,
                          struct gl_buffer_object *bufObj, GLintptr offset,
                          GLsizeiptr length)
{
   if (ctx->has_invalidate_buffer)
      bufferobj_invalidate(ctx, bufObj, offset, length);
}

void GLAPIENTRY
_mesa_InvalidateBufferSubData_no_error(GLuint buffer, GLintptr offset,
                                       GLsizeiptr length)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_buffer_object *bufObj = _mesa_lookup_bufferobj(ctx, buffer);
   invalidate_buffer_subdata(ctx, bufObj, offset, length);
}

void GLAPIENTRY
_mesa_InvalidateBufferSubData(GLuint buffer, GLintptr offset,
                              GLsizeiptr length)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;
   const GLintptr end = offset + length;

   /* Section 6.5 (Invalidating Buffer Data) of the OpenGL 4.5 (Compatibility
    * Profile) spec says:
    *
    *     "An INVALID_VALUE error is generated if buffer is zero or is not the
    *     name of an existing buffer object."
    */
   bufObj = _mesa_lookup_bufferobj(ctx, buffer);
   if (!bufObj || bufObj == &DummyBufferObject) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glInvalidateBufferSubData(name = %u) invalid object",
                  buffer);
      return;
   }

   /* The GL_ARB_invalidate_subdata spec says:
    *
    *     "An INVALID_VALUE error is generated if <offset> or <length> is
    *     negative, or if <offset> + <length> is greater than the value of
    *     BUFFER_SIZE."
    */
   if (offset < 0 || length < 0 || end > bufObj->Size) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glInvalidateBufferSubData(invalid offset or length)");
      return;
   }

   /* The OpenGL 4.4 (Core Profile) spec says:
    *
    *     "An INVALID_OPERATION error is generated if buffer is currently
    *     mapped by MapBuffer or if the invalidate range intersects the range
    *     currently mapped by MapBufferRange, unless it was mapped
    *     with MAP_PERSISTENT_BIT set in the MapBufferRange access flags."
    */
   if (!(bufObj->Mappings[MAP_USER].AccessFlags & GL_MAP_PERSISTENT_BIT) &&
       bufferobj_range_mapped(bufObj, offset, length)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glInvalidateBufferSubData(intersection with mapped "
                  "range)");
      return;
   }

   invalidate_buffer_subdata(ctx, bufObj, offset, length);
}

void GLAPIENTRY
_mesa_InvalidateBufferData_no_error(GLuint buffer)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_buffer_object *bufObj =_mesa_lookup_bufferobj(ctx, buffer);
   invalidate_buffer_subdata(ctx, bufObj, 0, bufObj->Size);
}

void GLAPIENTRY
_mesa_InvalidateBufferData(GLuint buffer)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufObj;

   /* Section 6.5 (Invalidating Buffer Data) of the OpenGL 4.5 (Compatibility
    * Profile) spec says:
    *
    *     "An INVALID_VALUE error is generated if buffer is zero or is not the
    *     name of an existing buffer object."
    */
   bufObj = _mesa_lookup_bufferobj(ctx, buffer);
   if (!bufObj || bufObj == &DummyBufferObject) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glInvalidateBufferData(name = %u) invalid object",
                  buffer);
      return;
   }

   /* The OpenGL 4.4 (Core Profile) spec says:
    *
    *     "An INVALID_OPERATION error is generated if buffer is currently
    *     mapped by MapBuffer or if the invalidate range intersects the range
    *     currently mapped by MapBufferRange, unless it was mapped
    *     with MAP_PERSISTENT_BIT set in the MapBufferRange access flags."
    */
   if (_mesa_check_disallowed_mapping(bufObj)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glInvalidateBufferData(intersection with mapped "
                  "range)");
      return;
   }

   invalidate_buffer_subdata(ctx, bufObj, 0, bufObj->Size);
}

static void
buffer_page_commitment(struct gl_context *ctx,
                       struct gl_buffer_object *bufferObj,
                       GLintptr offset, GLsizeiptr size,
                       GLboolean commit, const char *func)
{
   if (!(bufferObj->StorageFlags & GL_SPARSE_STORAGE_BIT_ARB)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(not a sparse buffer object)",
                  func);
      return;
   }

   if (size < 0 || size > bufferObj->Size ||
       offset < 0 || offset > bufferObj->Size - size) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(out of bounds)",
                  func);
      return;
   }

   /* The GL_ARB_sparse_buffer extension specification says:
    *
    *     "INVALID_VALUE is generated by BufferPageCommitmentARB if <offset> is
    *     not an integer multiple of SPARSE_BUFFER_PAGE_SIZE_ARB, or if <size>
    *     is not an integer multiple of SPARSE_BUFFER_PAGE_SIZE_ARB and does
    *     not extend to the end of the buffer's data store."
    */
   if (offset % ctx->Const.SparseBufferPageSize != 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(offset not aligned to page size)",
                  func);
      return;
   }

   if (size % ctx->Const.SparseBufferPageSize != 0 &&
       offset + size != bufferObj->Size) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(size not aligned to page size)",
                  func);
      return;
   }

   struct pipe_context *pipe = ctx->pipe;
   struct pipe_box box;

   u_box_1d(offset, size, &box);

   if (!pipe->resource_commit(pipe, bufferObj->buffer, 0, &box, commit)) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glBufferPageCommitmentARB(out of memory)");
   }
}

void GLAPIENTRY
_mesa_BufferPageCommitmentARB(GLenum target, GLintptr offset, GLsizeiptr size,
                              GLboolean commit)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufferObj;

   bufferObj = get_buffer(ctx, "glBufferPageCommitmentARB", target,
                          GL_INVALID_ENUM);
   if (!bufferObj)
      return;

   buffer_page_commitment(ctx, bufferObj, offset, size, commit,
                          "glBufferPageCommitmentARB");
}

void GLAPIENTRY
_mesa_NamedBufferPageCommitmentARB(GLuint buffer, GLintptr offset,
                                   GLsizeiptr size, GLboolean commit)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufferObj;

   bufferObj = _mesa_lookup_bufferobj(ctx, buffer);
   if (!bufferObj || bufferObj == &DummyBufferObject) {
      /* Note: the extension spec is not clear about the excpected error value. */
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glNamedBufferPageCommitmentARB(name = %u) invalid object",
                  buffer);
      return;
   }

   buffer_page_commitment(ctx, bufferObj, offset, size, commit,
                          "glNamedBufferPageCommitmentARB");
}

void GLAPIENTRY
_mesa_NamedBufferPageCommitmentEXT(GLuint buffer, GLintptr offset,
                                   GLsizeiptr size, GLboolean commit)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_buffer_object *bufferObj;

   /* Use NamedBuffer* functions logic from EXT_direct_state_access */
   if (buffer != 0) {
      bufferObj = _mesa_lookup_bufferobj(ctx, buffer);
      if (!handle_bind_buffer_gen(ctx, buffer, &bufferObj,
                                  "glNamedBufferPageCommitmentEXT", false))
         return;
   } else {
      /* GL_EXT_direct_state_access says about NamedBuffer* functions:
       *
       *   There is no buffer corresponding to the name zero, these commands
       *   generate the INVALID_OPERATION error if the buffer parameter is
       *   zero.
       */
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glNamedBufferPageCommitmentEXT(buffer = 0)");
      return;
   }
   buffer_page_commitment(ctx, bufferObj, offset, size, commit,
                          "glNamedBufferPageCommitmentEXT");
}
