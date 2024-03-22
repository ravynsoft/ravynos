/*
 * Copyright © 2023 Intel Corporation
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
#include "i915/iris_batch.h"

#include "iris/iris_batch.h"
#include "iris/iris_context.h"

#include "common/intel_defines.h"
#include "common/intel_gem.h"
#include "util/u_debug.h"

#define FILE_DEBUG_FLAG DEBUG_BATCH

static int
iris_context_priority_to_i915_priority(enum iris_context_priority priority)
{
   switch (priority) {
   case IRIS_CONTEXT_HIGH_PRIORITY:
      return INTEL_CONTEXT_HIGH_PRIORITY;
   case IRIS_CONTEXT_LOW_PRIORITY:
      return INTEL_CONTEXT_LOW_PRIORITY;
   case IRIS_CONTEXT_MEDIUM_PRIORITY:
      FALLTHROUGH;
   default:
      return INTEL_CONTEXT_MEDIUM_PRIORITY;
   }
}

static int
context_set_priority(struct iris_bufmgr *bufmgr, uint32_t ctx_id,
                     enum iris_context_priority priority)
{
   int err = 0;
   int i915_priority = iris_context_priority_to_i915_priority(priority);
   if (!intel_gem_set_context_param(iris_bufmgr_get_fd(bufmgr), ctx_id,
                                    I915_CONTEXT_PARAM_PRIORITY, i915_priority))
      err = -errno;

   return err;
}

static void
iris_hw_context_set_unrecoverable(struct iris_bufmgr *bufmgr,
                                  uint32_t ctx_id)
{
   /* Upon declaring a GPU hang, the kernel will zap the guilty context
    * back to the default logical HW state and attempt to continue on to
    * our next submitted batchbuffer.  However, our render batches assume
    * the previous GPU state is preserved, and only emit commands needed
    * to incrementally change that state.  In particular, we inherit the
    * STATE_BASE_ADDRESS and PIPELINE_SELECT settings, which are critical.
    * With default base addresses, our next batches will almost certainly
    * cause more GPU hangs, leading to repeated hangs until we're banned
    * or the machine is dead.
    *
    * Here we tell the kernel not to attempt to recover our context but
    * immediately (on the next batchbuffer submission) report that the
    * context is lost, and we will do the recovery ourselves.  Ideally,
    * we'll have two lost batches instead of a continual stream of hangs.
    */
   intel_gem_set_context_param(iris_bufmgr_get_fd(bufmgr), ctx_id,
                               I915_CONTEXT_PARAM_RECOVERABLE, false);
}

static void
iris_hw_context_set_vm_id(struct iris_bufmgr *bufmgr, uint32_t ctx_id)
{
   if (!iris_bufmgr_use_global_vm_id(bufmgr))
      return;

   if (!intel_gem_set_context_param(iris_bufmgr_get_fd(bufmgr), ctx_id,
                                    I915_CONTEXT_PARAM_VM,
                                    iris_bufmgr_use_global_vm_id(bufmgr)))
      DBG("DRM_IOCTL_I915_GEM_CONTEXT_SETPARAM failed: %s\n",
          strerror(errno));
}

static uint32_t
iris_create_hw_context(struct iris_bufmgr *bufmgr, bool protected)
{
   uint32_t ctx_id;

   if (protected) {
      /* User explicitly requested for PXP so wait for the kernel + firmware
       * dependencies to complete to avoid a premature PXP context-create failure.
       */
      if (!intel_gem_wait_on_get_param(iris_bufmgr_get_fd(bufmgr),
                                      I915_PARAM_PXP_STATUS, 1,
                                      8000))
         DBG("unable to wait for pxp-readiness\n");

      if (!intel_gem_create_context_ext(iris_bufmgr_get_fd(bufmgr),
                                        INTEL_GEM_CREATE_CONTEXT_EXT_PROTECTED_FLAG,
                                        &ctx_id)) {
         DBG("DRM_IOCTL_I915_GEM_CONTEXT_CREATE_EXT failed: %s\n", strerror(errno));
         return 0;
      }
   } else {
      if (!intel_gem_create_context(iris_bufmgr_get_fd(bufmgr), &ctx_id)) {
         DBG("intel_gem_create_context failed: %s\n", strerror(errno));
         return 0;
      }
      iris_hw_context_set_unrecoverable(bufmgr, ctx_id);
   }

   iris_hw_context_set_vm_id(bufmgr, ctx_id);

   return ctx_id;
}

static void
iris_init_non_engine_contexts(struct iris_context *ice)
{
   struct iris_screen *screen = (void *) ice->ctx.screen;

   iris_foreach_batch(ice, batch) {
      batch->i915.ctx_id = iris_create_hw_context(screen->bufmgr, ice->protected);
      batch->i915.exec_flags = I915_EXEC_RENDER;
      assert(batch->i915.ctx_id);
      context_set_priority(screen->bufmgr, batch->i915.ctx_id, ice->priority);
   }

   ice->batches[IRIS_BATCH_BLITTER].i915.exec_flags = I915_EXEC_BLT;
   ice->has_engines_context = false;
}

static int
iris_create_engines_context(struct iris_context *ice)
{
   struct iris_screen *screen = (void *) ice->ctx.screen;
   const struct intel_device_info *devinfo = screen->devinfo;
   int fd = iris_bufmgr_get_fd(screen->bufmgr);

   struct intel_query_engine_info *engines_info;
   engines_info = intel_engine_get_info(fd, screen->devinfo->kmd_type);

   if (!engines_info)
      return -1;

   if (intel_engines_count(engines_info, INTEL_ENGINE_CLASS_RENDER) < 1) {
      free(engines_info);
      return -1;
   }

   STATIC_ASSERT(IRIS_BATCH_COUNT == 3);
   enum intel_engine_class engine_classes[IRIS_BATCH_COUNT] = {
      [IRIS_BATCH_RENDER] = INTEL_ENGINE_CLASS_RENDER,
      [IRIS_BATCH_COMPUTE] = INTEL_ENGINE_CLASS_RENDER,
      [IRIS_BATCH_BLITTER] = INTEL_ENGINE_CLASS_COPY,
   };

   /* Blitter is only supported on Gfx12+ */
   unsigned num_batches = IRIS_BATCH_COUNT - (devinfo->ver >= 12 ? 0 : 1);

   if (debug_get_bool_option("INTEL_COMPUTE_CLASS", false) &&
       intel_engines_count(engines_info, INTEL_ENGINE_CLASS_COMPUTE) > 0)
      engine_classes[IRIS_BATCH_COMPUTE] = INTEL_ENGINE_CLASS_COMPUTE;

   enum intel_gem_create_context_flags flags = 0;
   if (ice->protected) {
      flags |= INTEL_GEM_CREATE_CONTEXT_EXT_PROTECTED_FLAG;

      /* User explicitly requested for PXP so wait for the kernel + firmware
       * dependencies to complete to avoid a premature PXP context-create failure.
       */
      if (!intel_gem_wait_on_get_param(fd,
                                      I915_PARAM_PXP_STATUS, 1,
                                      8000))
         DBG("unable to wait for pxp-readiness\n");
   }

   uint32_t engines_ctx;
   if (!intel_gem_create_context_engines(fd, flags, engines_info, num_batches,
                                         engine_classes, 0, &engines_ctx)) {
      free(engines_info);
      return -1;
   }

   iris_hw_context_set_unrecoverable(screen->bufmgr, engines_ctx);
   iris_hw_context_set_vm_id(screen->bufmgr, engines_ctx);
   context_set_priority(screen->bufmgr, engines_ctx, ice->priority);

   free(engines_info);
   return engines_ctx;
}

static bool
iris_init_engines_context(struct iris_context *ice)
{
   int engines_ctx = iris_create_engines_context(ice);
   if (engines_ctx < 0)
      return false;

   iris_foreach_batch(ice, batch) {
      unsigned i = batch - &ice->batches[0];
      batch->i915.ctx_id = engines_ctx;
      batch->i915.exec_flags = i;
   }

   ice->has_engines_context = true;
   return true;
}

static bool
iris_hw_context_get_protected(struct iris_bufmgr *bufmgr, uint32_t ctx_id)
{
   uint64_t protected_content = 0;
   intel_gem_get_context_param(iris_bufmgr_get_fd(bufmgr), ctx_id,
                               I915_CONTEXT_PARAM_PROTECTED_CONTENT,
                               &protected_content);
   return protected_content;
}

static uint32_t
clone_hw_context(struct iris_batch *batch)
{
   struct iris_screen *screen = batch->screen;
   struct iris_bufmgr *bufmgr = screen->bufmgr;
   struct iris_context *ice = batch->ice;
   bool protected = iris_hw_context_get_protected(bufmgr, batch->i915.ctx_id);
   uint32_t new_ctx = iris_create_hw_context(bufmgr, protected);

   if (new_ctx)
      context_set_priority(bufmgr, new_ctx, ice->priority);

   return new_ctx;
}

static void
iris_destroy_kernel_context(struct iris_bufmgr *bufmgr, uint32_t ctx_id)
{
   if (ctx_id != 0 &&
       !intel_gem_destroy_context(iris_bufmgr_get_fd(bufmgr), ctx_id)) {
      fprintf(stderr, "DRM_IOCTL_I915_GEM_CONTEXT_DESTROY failed: %s\n",
              strerror(errno));
   }
}

bool
iris_i915_replace_batch(struct iris_batch *batch)
{
   struct iris_screen *screen = batch->screen;
   struct iris_bufmgr *bufmgr = screen->bufmgr;
   struct iris_context *ice = batch->ice;

   if (ice->has_engines_context) {
      uint32_t old_ctx = batch->i915.ctx_id;
      int new_ctx = iris_create_engines_context(ice);
      if (new_ctx < 0)
         return false;
      iris_foreach_batch(ice, bat) {
         bat->i915.ctx_id = new_ctx;
         /* Notify the context that state must be re-initialized. */
         iris_lost_context_state(bat);
      }
      iris_destroy_kernel_context(bufmgr, old_ctx);
   } else {
      uint32_t new_ctx = clone_hw_context(batch);
      if (!new_ctx)
         return false;

      iris_destroy_kernel_context(bufmgr, batch->i915.ctx_id);
      batch->i915.ctx_id = new_ctx;

      /* Notify the context that state must be re-initialized. */
      iris_lost_context_state(batch);
   }

   return true;
}

void iris_i915_destroy_batch(struct iris_batch *batch)
{
   struct iris_screen *screen = batch->screen;
   struct iris_bufmgr *bufmgr = screen->bufmgr;

   /* destroy the engines context on the first batch or destroy each batch
    * context
    */
   if (batch->ice->has_engines_context && batch != &batch->ice->batches[0])
      return;

   iris_destroy_kernel_context(bufmgr, batch->i915.ctx_id);
}

void iris_i915_init_batches(struct iris_context *ice)
{
   if (!iris_init_engines_context(ice))
      iris_init_non_engine_contexts(ice);
}
