/*
 * Copyright © 2009 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file syncobj.c
 * Sync object management.
 *
 * Unlike textures and other objects that are shared between contexts, sync
 * objects are not bound to the context.  As a result, the reference counting
 * and delete behavior of sync objects is slightly different.  References to
 * sync objects are added:
 *
 *    - By \c glFencSynce.  This sets the initial reference count to 1.
 *    - At the start of \c glClientWaitSync.  The reference is held for the
 *      duration of the wait call.
 *
 * References are removed:
 *
 *    - By \c glDeleteSync.
 *    - At the end of \c glClientWaitSync.
 *
 * Additionally, drivers may call \c _mesa_ref_sync_object and
 * \c _mesa_unref_sync_object as needed to implement \c ServerWaitSync.
 *
 * As with shader objects, sync object names become invalid as soon as
 * \c glDeleteSync is called.  For this reason \c glDeleteSync sets the
 * \c DeletePending flag.  All functions validate object handles by testing
 * this flag.
 *
 * \note
 * Only \c GL_ARB_sync objects are shared between contexts.  If support is ever
 * added for either \c GL_NV_fence or \c GL_APPLE_fence different semantics
 * will need to be implemented.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include <inttypes.h>
#include "util/glheader.h"

#include "context.h"
#include "macros.h"
#include "get.h"
#include "mtypes.h"
#include "util/hash_table.h"
#include "util/set.h"
#include "util/u_memory.h"
#include "util/perf/cpu_trace.h"

#include "syncobj.h"

#include "api_exec_decl.h"

#include "pipe/p_context.h"
#include "pipe/p_screen.h"

/**
 * Allocate/init the context state related to sync objects.
 */
void
_mesa_init_sync(struct gl_context *ctx)
{
   (void) ctx;
}


/**
 * Free the context state related to sync objects.
 */
void
_mesa_free_sync_data(struct gl_context *ctx)
{
   (void) ctx;
}

static struct gl_sync_object *
new_sync_object(struct gl_context *ctx)
{
   struct gl_sync_object *so = CALLOC_STRUCT(gl_sync_object);

   simple_mtx_init(&so->mutex, mtx_plain);
   return so;
}

static void
delete_sync_object(struct gl_context *ctx,
                      struct gl_sync_object *obj)
{
   struct pipe_screen *screen = ctx->pipe->screen;

   screen->fence_reference(screen, &obj->fence, NULL);
   simple_mtx_destroy(&obj->mutex);
   free(obj->Label);
   FREE(obj);
}

static void
__client_wait_sync(struct gl_context *ctx,
                   struct gl_sync_object *obj,
                   GLbitfield flags, GLuint64 timeout)
{
   struct pipe_context *pipe = ctx->pipe;
   struct pipe_screen *screen = pipe->screen;
   struct pipe_fence_handle *fence = NULL;

   MESA_TRACE_FUNC();

   /* If the fence doesn't exist, assume it's signalled. */
   simple_mtx_lock(&obj->mutex);
   if (!obj->fence) {
      simple_mtx_unlock(&obj->mutex);
      obj->StatusFlag = GL_TRUE;
      return;
   }

   /* We need a local copy of the fence pointer, so that we can call
    * fence_finish unlocked.
    */
   screen->fence_reference(screen, &fence, obj->fence);
   simple_mtx_unlock(&obj->mutex);

   /* Section 4.1.2 of OpenGL 4.5 (Compatibility Profile) says:
    *    [...] if ClientWaitSync is called and all of the following are true:
    *    - the SYNC_FLUSH_COMMANDS_BIT bit is set in flags,
    *    - sync is unsignaled when ClientWaitSync is called,
    *    - and the calls to ClientWaitSync and FenceSync were issued from
    *      the same context,
    *    then the GL will behave as if the equivalent of Flush were inserted
    *    immediately after the creation of sync.
    *
    * Assume GL_SYNC_FLUSH_COMMANDS_BIT is always set, because applications
    * forget to set it.
    */
   if (screen->fence_finish(screen, pipe, fence, timeout)) {
      simple_mtx_lock(&obj->mutex);
      screen->fence_reference(screen, &obj->fence, NULL);
      simple_mtx_unlock(&obj->mutex);
      obj->StatusFlag = GL_TRUE;
   }
   screen->fence_reference(screen, &fence, NULL);
}

/**
 * Check if the given sync object is:
 *  - non-null
 *  - not in sync objects hash table
 *  - not marked as deleted
 *
 * Returns the internal gl_sync_object pointer if the sync object is valid
 * or NULL if it isn't.
 *
 * If "incRefCount" is true, the reference count is incremented, which is
 * normally what you want; otherwise, a glDeleteSync from another thread
 * could delete the sync object while you are still working on it.
 */
struct gl_sync_object *
_mesa_get_and_ref_sync(struct gl_context *ctx, GLsync sync, bool incRefCount)
{
   struct gl_sync_object *syncObj = (struct gl_sync_object *) sync;
   simple_mtx_lock(&ctx->Shared->Mutex);
   if (syncObj != NULL
      && _mesa_set_search(ctx->Shared->SyncObjects, syncObj) != NULL
      && !syncObj->DeletePending) {
     if (incRefCount) {
       syncObj->RefCount++;
     }
   } else {
     syncObj = NULL;
   }
   simple_mtx_unlock(&ctx->Shared->Mutex);
   return syncObj;
}


void
_mesa_unref_sync_object(struct gl_context *ctx, struct gl_sync_object *syncObj,
                        int amount)
{
   struct set_entry *entry;

   simple_mtx_lock(&ctx->Shared->Mutex);
   syncObj->RefCount -= amount;
   if (syncObj->RefCount == 0) {
      entry = _mesa_set_search(ctx->Shared->SyncObjects, syncObj);
      assert (entry != NULL);
      _mesa_set_remove(ctx->Shared->SyncObjects, entry);
      simple_mtx_unlock(&ctx->Shared->Mutex);

      delete_sync_object(ctx, syncObj);
   } else {
      simple_mtx_unlock(&ctx->Shared->Mutex);
   }
}


GLboolean GLAPIENTRY
_mesa_IsSync(GLsync sync)
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_WITH_RETVAL(ctx, GL_FALSE);

   return _mesa_get_and_ref_sync(ctx, sync, false) ? GL_TRUE : GL_FALSE;
}


static ALWAYS_INLINE void
delete_sync(struct gl_context *ctx, GLsync sync, bool no_error)
{
   struct gl_sync_object *syncObj;

   /* From the GL_ARB_sync spec:
    *
    *    DeleteSync will silently ignore a <sync> value of zero. An
    *    INVALID_VALUE error is generated if <sync> is neither zero nor the
    *    name of a sync object.
    */
   if (sync == 0) {
      return;
   }

   syncObj = _mesa_get_and_ref_sync(ctx, sync, true);
   if (!no_error && !syncObj) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glDeleteSync (not a valid sync object)");
      return;
   }

   /* If there are no client-waits or server-waits pending on this sync, delete
    * the underlying object. Note that we double-unref the object, as
    * _mesa_get_and_ref_sync above took an extra refcount to make sure the
    * pointer is valid for us to manipulate.
    */
   syncObj->DeletePending = GL_TRUE;
   _mesa_unref_sync_object(ctx, syncObj, 2);
}


void GLAPIENTRY
_mesa_DeleteSync_no_error(GLsync sync)
{
   GET_CURRENT_CONTEXT(ctx);
   delete_sync(ctx, sync, true);
}


void GLAPIENTRY
_mesa_DeleteSync(GLsync sync)
{
   GET_CURRENT_CONTEXT(ctx);
   delete_sync(ctx, sync, false);
}


GLsync
_mesa_fence_sync(struct gl_context *ctx, GLenum condition, GLbitfield flags)
{
   struct gl_sync_object *syncObj;

   syncObj = new_sync_object(ctx);
   if (syncObj != NULL) {
      /* The name is not currently used, and it is never visible to
       * applications.  If sync support is extended to provide support for
       * NV_fence, this field will be used.  We'll also need to add an object
       * ID hashtable.
       */
      syncObj->Name = 1;
      syncObj->RefCount = 1;
      syncObj->DeletePending = GL_FALSE;
      syncObj->SyncCondition = condition;
      syncObj->Flags = flags;
      syncObj->StatusFlag = 0;

      assert(condition == GL_SYNC_GPU_COMMANDS_COMPLETE && flags == 0);
      assert(syncObj->fence == NULL);

      /* Deferred flush are only allowed when there's a single context. See issue 1430 */
      ctx->pipe->flush(ctx->pipe, &syncObj->fence, ctx->Shared->RefCount == 1 ? PIPE_FLUSH_DEFERRED : 0);

      simple_mtx_lock(&ctx->Shared->Mutex);
      _mesa_set_add(ctx->Shared->SyncObjects, syncObj);
      simple_mtx_unlock(&ctx->Shared->Mutex);

      return (GLsync)syncObj;
   }

   return NULL;
}


GLsync GLAPIENTRY
_mesa_FenceSync_no_error(GLenum condition, GLbitfield flags)
{
   GET_CURRENT_CONTEXT(ctx);
   return _mesa_fence_sync(ctx, condition, flags);
}


GLsync GLAPIENTRY
_mesa_FenceSync(GLenum condition, GLbitfield flags)
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_WITH_RETVAL(ctx, 0);

   if (condition != GL_SYNC_GPU_COMMANDS_COMPLETE) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glFenceSync(condition=0x%x)",
                  condition);
      return 0;
   }

   if (flags != 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glFenceSync(flags=0x%x)", condition);
      return 0;
   }

   return _mesa_fence_sync(ctx, condition, flags);
}


static GLenum
client_wait_sync(struct gl_context *ctx, struct gl_sync_object *syncObj,
                 GLbitfield flags, GLuint64 timeout)
{
   GLenum ret;

   /* From the GL_ARB_sync spec:
    *
    *    ClientWaitSync returns one of four status values. A return value of
    *    ALREADY_SIGNALED indicates that <sync> was signaled at the time
    *    ClientWaitSync was called. ALREADY_SIGNALED will always be returned
    *    if <sync> was signaled, even if the value of <timeout> is zero.
    */
   __client_wait_sync(ctx, syncObj, 0, 0);
   if (syncObj->StatusFlag) {
      ret = GL_ALREADY_SIGNALED;
   } else {
      if (timeout == 0) {
         ret = GL_TIMEOUT_EXPIRED;
      } else {
         __client_wait_sync(ctx, syncObj, flags, timeout);

         ret = syncObj->StatusFlag
            ? GL_CONDITION_SATISFIED : GL_TIMEOUT_EXPIRED;
      }
   }

   _mesa_unref_sync_object(ctx, syncObj, 1);
   return ret;
}


GLenum GLAPIENTRY
_mesa_ClientWaitSync_no_error(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_sync_object *syncObj = _mesa_get_and_ref_sync(ctx, sync, true);
   return client_wait_sync(ctx, syncObj, flags, timeout);
}


GLenum GLAPIENTRY
_mesa_ClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_sync_object *syncObj;

   ASSERT_OUTSIDE_BEGIN_END_WITH_RETVAL(ctx, GL_WAIT_FAILED);

   if ((flags & ~GL_SYNC_FLUSH_COMMANDS_BIT) != 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glClientWaitSync(flags=0x%x)", flags);
      return GL_WAIT_FAILED;
   }

   syncObj = _mesa_get_and_ref_sync(ctx, sync, true);
   if (!syncObj) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glClientWaitSync (not a valid sync object)");
      return GL_WAIT_FAILED;
   }

   return client_wait_sync(ctx, syncObj, flags, timeout);
}


static void
wait_sync(struct gl_context *ctx, struct gl_sync_object *syncObj,
          GLbitfield flags, GLuint64 timeout)
{
   struct pipe_context *pipe = ctx->pipe;
   struct pipe_screen *screen = pipe->screen;
   struct pipe_fence_handle *fence = NULL;

   /* Nothing needs to be done here if the driver does not support async
    * flushes. */
   if (!pipe->fence_server_sync) {
      _mesa_unref_sync_object(ctx, syncObj, 1);
      return;
   }

   /* If the fence doesn't exist, assume it's signalled. */
   simple_mtx_lock(&syncObj->mutex);
   if (!syncObj->fence) {
      simple_mtx_unlock(&syncObj->mutex);
      syncObj->StatusFlag = GL_TRUE;
      _mesa_unref_sync_object(ctx, syncObj, 1);
      return;
   }

   /* We need a local copy of the fence pointer. */
   screen->fence_reference(screen, &fence, syncObj->fence);
   simple_mtx_unlock(&syncObj->mutex);

   pipe->fence_server_sync(pipe, fence);
   screen->fence_reference(screen, &fence, NULL);
   _mesa_unref_sync_object(ctx, syncObj, 1);
}


void GLAPIENTRY
_mesa_WaitSync_no_error(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_sync_object *syncObj = _mesa_get_and_ref_sync(ctx, sync, true);
   wait_sync(ctx, syncObj, flags, timeout);
}


void GLAPIENTRY
_mesa_WaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_sync_object *syncObj;

   if (flags != 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glWaitSync(flags=0x%x)", flags);
      return;
   }

   if (timeout != GL_TIMEOUT_IGNORED) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glWaitSync(timeout=0x%" PRIx64 ")",
                  (uint64_t) timeout);
      return;
   }

   syncObj = _mesa_get_and_ref_sync(ctx, sync, true);
   if (!syncObj) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glWaitSync (not a valid sync object)");
      return;
   }

   wait_sync(ctx, syncObj, flags, timeout);
}


void GLAPIENTRY
_mesa_GetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length,
                GLint *values)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_sync_object *syncObj;
   GLsizei size = 0;
   GLint v[1];

   syncObj = _mesa_get_and_ref_sync(ctx, sync, true);
   if (!syncObj) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glGetSynciv (not a valid sync object)");
      return;
   }

   switch (pname) {
   case GL_OBJECT_TYPE:
      v[0] = GL_SYNC_FENCE;
      size = 1;
      break;

   case GL_SYNC_CONDITION:
      v[0] = syncObj->SyncCondition;
      size = 1;
      break;

   case GL_SYNC_STATUS:
      /* Update the state of the sync by dipping into the driver.  Note that
       * this call won't block.  It just updates state in the common object
       * data from the current driver state.
       */
      __client_wait_sync(ctx, syncObj, 0, 0);

      v[0] = (syncObj->StatusFlag) ? GL_SIGNALED : GL_UNSIGNALED;
      size = 1;
      break;

   case GL_SYNC_FLAGS:
      v[0] = syncObj->Flags;
      size = 1;
      break;

   default:
      _mesa_error(ctx, GL_INVALID_ENUM, "glGetSynciv(pname=0x%x)\n", pname);
      _mesa_unref_sync_object(ctx, syncObj, 1);
      return;
   }

   /* Section 4.1.3 (Sync Object Queries) of the OpenGL ES 3.10 spec says:
    *
    *    "An INVALID_VALUE error is generated if bufSize is negative."
    */
   if (bufSize < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glGetSynciv(pname=0x%x)\n", pname);
   }

   if (size > 0 && bufSize > 0) {
      const GLsizei copy_count = MIN2(size, bufSize);

      memcpy(values, v, sizeof(GLint) * copy_count);
   }

   if (length != NULL) {
      *length = size;
   }

   _mesa_unref_sync_object(ctx, syncObj, 1);
}
