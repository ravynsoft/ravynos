/*
 * Copyright 2018 Collabora Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "zink_batch.h"
#include "zink_context.h"
#include "zink_fence.h"

#include "zink_resource.h"
#include "zink_screen.h"

#include "util/os_file.h"
#include "util/set.h"
#include "util/u_memory.h"

#ifdef _WIN32
#include <windows.h>
#include <vulkan/vulkan_win32.h>
#endif

static void
destroy_fence(struct zink_screen *screen, struct zink_tc_fence *mfence)
{
   if (mfence->fence)
      util_dynarray_delete_unordered(&mfence->fence->mfences, struct zink_tc_fence *, mfence);
   mfence->fence = NULL;
   tc_unflushed_batch_token_reference(&mfence->tc_token, NULL);
   if (mfence->sem)
      VKSCR(DestroySemaphore)(screen->dev, mfence->sem, NULL);
   FREE(mfence);
}

struct zink_tc_fence *
zink_create_tc_fence(void)
{
   struct zink_tc_fence *mfence = CALLOC_STRUCT(zink_tc_fence);
   if (!mfence)
      return NULL;
   pipe_reference_init(&mfence->reference, 1);
   util_queue_fence_init(&mfence->ready);
   return mfence;
}

struct pipe_fence_handle *
zink_create_tc_fence_for_tc(struct pipe_context *pctx, struct tc_unflushed_batch_token *tc_token)
{
   struct zink_tc_fence *mfence = zink_create_tc_fence();
   if (!mfence)
      return NULL;
   util_queue_fence_reset(&mfence->ready);
   tc_unflushed_batch_token_reference(&mfence->tc_token, tc_token);
   return (struct pipe_fence_handle*)mfence;
}

void
zink_fence_reference(struct zink_screen *screen,
                     struct zink_tc_fence **ptr,
                     struct zink_tc_fence *mfence)
{
   if (pipe_reference(&(*ptr)->reference, &mfence->reference))
      destroy_fence(screen, *ptr);

   *ptr = mfence;
}

static void
fence_reference(struct pipe_screen *pscreen,
                struct pipe_fence_handle **pptr,
                struct pipe_fence_handle *pfence)
{
   zink_fence_reference(zink_screen(pscreen), (struct zink_tc_fence **)pptr,
                        zink_tc_fence(pfence));
}

static bool
tc_fence_finish(struct zink_context *ctx, struct zink_tc_fence *mfence, uint64_t *timeout_ns)
{
   if (!util_queue_fence_is_signalled(&mfence->ready)) {
      int64_t abs_timeout = os_time_get_absolute_timeout(*timeout_ns);
      if (mfence->tc_token) {
         /* Ensure that zink_flush will be called for
          * this mfence, but only if we're in the API thread
          * where the context is current.
          *
          * Note that the batch containing the flush may already
          * be in flight in the driver thread, so the mfence
          * may not be ready yet when this call returns.
          */
         threaded_context_flush(&ctx->base, mfence->tc_token, *timeout_ns == 0);
      }

      /* this is a tc mfence, so we're just waiting on the queue mfence to complete
       * after being signaled by the real mfence
       */
      if (*timeout_ns == OS_TIMEOUT_INFINITE) {
         util_queue_fence_wait(&mfence->ready);
      } else {
         if (!util_queue_fence_wait_timeout(&mfence->ready, abs_timeout))
            return false;
      }
      if (*timeout_ns && *timeout_ns != OS_TIMEOUT_INFINITE) {
         int64_t time_ns = os_time_get_nano();
         *timeout_ns = abs_timeout > time_ns ? abs_timeout - time_ns : 0;
      }
   }

   return true;
}

static bool
fence_wait(struct zink_screen *screen, struct zink_fence *fence, uint64_t timeout_ns)
{
   if (screen->device_lost)
      return true;
   if (p_atomic_read(&fence->completed))
      return true;

   assert(fence->batch_id);
   assert(fence->submitted);

   bool success = zink_screen_timeline_wait(screen, fence->batch_id, timeout_ns);

   if (success) {
      p_atomic_set(&fence->completed, true);
      zink_batch_state(fence)->usage.usage = 0;
      zink_screen_update_last_finished(screen, fence->batch_id);
   }
   return success;
}

static bool
zink_fence_finish(struct zink_screen *screen, struct pipe_context *pctx, struct zink_tc_fence *mfence,
                  uint64_t timeout_ns)
{
   pctx = threaded_context_unwrap_sync(pctx);
   struct zink_context *ctx = zink_context(pctx);

   if (screen->device_lost)
      return true;

   if (pctx && mfence->deferred_ctx == pctx) {
      if (mfence->fence == ctx->deferred_fence) {
         zink_context(pctx)->batch.has_work = true;
         /* this must be the current batch */
         pctx->flush(pctx, NULL, !timeout_ns ? PIPE_FLUSH_ASYNC : 0);
         if (!timeout_ns)
            return false;
      }
   }

   /* need to ensure the tc mfence has been flushed before we wait */
   bool tc_finish = tc_fence_finish(ctx, mfence, &timeout_ns);
   /* the submit thread hasn't finished yet */
   if (!tc_finish)
      return false;
   /* this was an invalid flush, just return completed */
   if (!mfence->fence)
      return true;

   struct zink_fence *fence = mfence->fence;

   unsigned submit_diff = zink_batch_state(mfence->fence)->usage.submit_count - mfence->submit_count;
   /* this batch is known to have finished because it has been submitted more than 1 time
    * since the tc fence last saw it
    */
   if (submit_diff > 1)
      return true;

   /* - if fence is submitted, batch_id is nonzero and can be checked
    * - if fence is not submitted here, it must be reset; batch_id will be 0 and submitted is false
    * in either case, the fence has finished
    */
   if ((fence->submitted && zink_screen_check_last_finished(screen, fence->batch_id)) ||
       (!fence->submitted && submit_diff))
      return true;

   return fence_wait(screen, fence, timeout_ns);
}

static bool
fence_finish(struct pipe_screen *pscreen, struct pipe_context *pctx,
                  struct pipe_fence_handle *pfence, uint64_t timeout_ns)
{
   return zink_fence_finish(zink_screen(pscreen), pctx, zink_tc_fence(pfence),
                            timeout_ns);
}

static int
fence_get_fd(struct pipe_screen *pscreen, struct pipe_fence_handle *pfence)
{
   struct zink_screen *screen = zink_screen(pscreen);
   if (screen->device_lost)
      return -1;

   struct zink_tc_fence *mfence = (struct zink_tc_fence *)pfence;
   if (!mfence->sem)
      return -1;

   const VkSemaphoreGetFdInfoKHR sgfi = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR,
      .semaphore = mfence->sem,
      .handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT,
   };
   int fd = -1;
   VkResult result = VKSCR(GetSemaphoreFdKHR)(screen->dev, &sgfi, &fd);
   if (!zink_screen_handle_vkresult(screen, result)) {
      mesa_loge("ZINK: vkGetSemaphoreFdKHR failed (%s)", vk_Result_to_str(result));
      return -1;
   }

   return fd;
}

void
zink_fence_server_signal(struct pipe_context *pctx, struct pipe_fence_handle *pfence)
{
   struct zink_context *ctx = zink_context(pctx);
   struct zink_tc_fence *mfence = (struct zink_tc_fence *)pfence;

   assert(!ctx->batch.state->signal_semaphore);
   ctx->batch.state->signal_semaphore = mfence->sem;
   ctx->batch.has_work = true;
   struct zink_batch_state *bs = ctx->batch.state;
   /* this must produce a synchronous flush that completes before the function returns */
   pctx->flush(pctx, NULL, 0);
   if (zink_screen(ctx->base.screen)->threaded_submit)
      util_queue_fence_wait(&bs->flush_completed);
}

void
zink_fence_server_sync(struct pipe_context *pctx, struct pipe_fence_handle *pfence)
{
   struct zink_context *ctx = zink_context(pctx);
   struct zink_tc_fence *mfence = (struct zink_tc_fence *)pfence;

   if (mfence->deferred_ctx == pctx || !mfence->sem)
      return;

   mfence->deferred_ctx = pctx;
   /* this will be applied on the next submit */
   VkPipelineStageFlags flag = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
   util_dynarray_append(&ctx->batch.state->wait_semaphores, VkSemaphore, mfence->sem);
   util_dynarray_append(&ctx->batch.state->wait_semaphore_stages, VkPipelineStageFlags, flag);
   pipe_reference(NULL, &mfence->reference);
   util_dynarray_append(&ctx->batch.state->fences, struct zink_tc_fence*, mfence);

   /* transfer the external wait sempahore ownership to the next submit */
   mfence->sem = VK_NULL_HANDLE;
}

void
zink_create_fence_fd(struct pipe_context *pctx, struct pipe_fence_handle **pfence, int fd, enum pipe_fd_type type)
{
   struct zink_screen *screen = zink_screen(pctx->screen);
   VkResult result;

   assert(fd >= 0);

   struct zink_tc_fence *mfence = zink_create_tc_fence();
   if (!mfence)
      goto fail_tc_fence_create;

   const VkSemaphoreCreateInfo sci = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
   };
   result = VKSCR(CreateSemaphore)(screen->dev, &sci, NULL, &mfence->sem);
   if (result != VK_SUCCESS) {
      mesa_loge("ZINK: vkCreateSemaphore failed (%s)", vk_Result_to_str(result));
      goto fail_sem_create;
   }

   int dup_fd = os_dupfd_cloexec(fd);
   if (dup_fd < 0)
      goto fail_fd_dup;

   static const VkExternalSemaphoreHandleTypeFlagBits flags[] = {
      [PIPE_FD_TYPE_NATIVE_SYNC] = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT,
      [PIPE_FD_TYPE_SYNCOBJ] = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT,
   };
   assert(type < ARRAY_SIZE(flags));

   const VkImportSemaphoreFdInfoKHR sdi = {
      .sType = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR,
      .semaphore = mfence->sem,
      .flags = VK_SEMAPHORE_IMPORT_TEMPORARY_BIT,
      .handleType = flags[type],
      .fd = dup_fd,
   };
   result = VKSCR(ImportSemaphoreFdKHR)(screen->dev, &sdi);
   if (!zink_screen_handle_vkresult(screen, result)) {
      mesa_loge("ZINK: vkImportSemaphoreFdKHR failed (%s)", vk_Result_to_str(result));
      goto fail_sem_import;
   }

   *pfence = (struct pipe_fence_handle *)mfence;
   return;

fail_sem_import:
   close(dup_fd);
fail_fd_dup:
   VKSCR(DestroySemaphore)(screen->dev, mfence->sem, NULL);
fail_sem_create:
   FREE(mfence);
fail_tc_fence_create:
   *pfence = NULL;
}

#ifdef _WIN32
void
zink_create_fence_win32(struct pipe_screen *pscreen, struct pipe_fence_handle **pfence, void *handle, const void *name, enum pipe_fd_type type)
{
   struct zink_screen *screen = zink_screen(pscreen);
   VkResult ret = VK_ERROR_UNKNOWN;
   VkSemaphoreCreateInfo sci = {
      VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      NULL,
      0
   };
   struct zink_tc_fence *mfence = zink_create_tc_fence();
   VkExternalSemaphoreHandleTypeFlagBits flags[] = {
      [PIPE_FD_TYPE_NATIVE_SYNC] = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT,
      [PIPE_FD_TYPE_SYNCOBJ] = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT,
   };
   VkImportSemaphoreWin32HandleInfoKHR sdi = {0};
   assert(type < ARRAY_SIZE(flags));

   *pfence = NULL;

   if (VKSCR(CreateSemaphore)(screen->dev, &sci, NULL, &mfence->sem) != VK_SUCCESS) {
      FREE(mfence);
      return;
   }

   sdi.sType = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR;
   sdi.semaphore = mfence->sem;
   sdi.handleType = flags[type];
   sdi.handle = handle;
   sdi.name = (LPCWSTR)name;
   ret = VKSCR(ImportSemaphoreWin32HandleKHR)(screen->dev, &sdi);

   if (!zink_screen_handle_vkresult(screen, ret))
      goto fail;
   *pfence = (struct pipe_fence_handle *)mfence;
   return;

fail:
   VKSCR(DestroySemaphore)(screen->dev, mfence->sem, NULL);
   FREE(mfence);
}
#endif

void
zink_screen_fence_init(struct pipe_screen *pscreen)
{
   pscreen->fence_reference = fence_reference;
   pscreen->fence_finish = fence_finish;
   pscreen->fence_get_fd = fence_get_fd;
}
