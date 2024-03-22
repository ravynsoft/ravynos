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
#include "xe/iris_batch.h"

#include "iris_batch.h"
#include "iris_context.h"
#include "iris_screen.h"

#include "common/intel_gem.h"
#include "common/intel_engine.h"
#include "common/xe/intel_device_query.h"
#include "common/xe/intel_engine.h"

#include "drm-uapi/xe_drm.h"
#include "drm-uapi/gpu_scheduler.h"

static enum drm_sched_priority
iris_context_priority_to_drm_sched_priority(enum iris_context_priority priority)
{
   switch (priority) {
   case IRIS_CONTEXT_HIGH_PRIORITY:
      return DRM_SCHED_PRIORITY_HIGH;
   case IRIS_CONTEXT_LOW_PRIORITY:
      return DRM_SCHED_PRIORITY_MIN;
   case IRIS_CONTEXT_MEDIUM_PRIORITY:
      FALLTHROUGH;
   default:
      return DRM_SCHED_PRIORITY_NORMAL;
   }
}

static bool
iris_xe_init_batch(struct iris_bufmgr *bufmgr,
                   struct intel_query_engine_info *engines_info,
                   enum intel_engine_class engine_class,
                   enum iris_context_priority priority, uint32_t *exec_queue_id)

{
   struct drm_xe_engine_class_instance *instances;

   instances = malloc(sizeof(*instances) *
                      intel_engines_count(engines_info, engine_class));
   if (!instances)
      return false;

   enum drm_sched_priority requested_priority = iris_context_priority_to_drm_sched_priority(priority);
   enum drm_sched_priority allowed_priority = DRM_SCHED_PRIORITY_MIN;
   if (requested_priority > DRM_SCHED_PRIORITY_MIN) {
      struct drm_xe_query_config *config;

      config = xe_device_query_alloc_fetch(iris_bufmgr_get_fd(bufmgr),
                                           DRM_XE_DEVICE_QUERY_CONFIG, NULL);
      if (config)
         allowed_priority = config->info[DRM_XE_QUERY_CONFIG_MAX_EXEC_QUEUE_PRIORITY];
      free(config);
   }
   if (requested_priority < allowed_priority)
      allowed_priority = requested_priority;

   uint32_t count = 0;
   for (uint32_t i = 0; i < engines_info->num_engines; i++) {
      const struct intel_engine_class_instance engine = engines_info->engines[i];
      if (engine.engine_class != engine_class)
         continue;

      instances[count].engine_class = intel_engine_class_to_xe(engine.engine_class);
      instances[count].engine_instance = engine.engine_instance;
      instances[count++].gt_id = engine.gt_id;
   }
   struct drm_xe_ext_set_property ext = {
      .base.name = DRM_XE_EXEC_QUEUE_EXTENSION_SET_PROPERTY,
      .property = DRM_XE_EXEC_QUEUE_SET_PROPERTY_PRIORITY,
      .value = allowed_priority,
   };
   struct drm_xe_exec_queue_create create = {
         .instances = (uintptr_t)instances,
         .vm_id = iris_bufmgr_get_global_vm_id(bufmgr),
         .width = 1,
         .num_placements = count,
         .extensions = (uintptr_t)&ext,
   };
   int ret = intel_ioctl(iris_bufmgr_get_fd(bufmgr),
                         DRM_IOCTL_XE_EXEC_QUEUE_CREATE, &create);
   free(instances);
   if (ret)
      goto error_create_exec_queue;

   /* TODO: handle "protected" context/exec_queue */
   *exec_queue_id = create.exec_queue_id;
error_create_exec_queue:
   return ret == 0;
}

static void
iris_xe_map_intel_engine_class(const struct intel_query_engine_info *engines_info,
                               enum intel_engine_class *engine_classes)
{
   engine_classes[IRIS_BATCH_RENDER] = INTEL_ENGINE_CLASS_RENDER;
   engine_classes[IRIS_BATCH_COMPUTE] = INTEL_ENGINE_CLASS_RENDER;
   engine_classes[IRIS_BATCH_BLITTER] = INTEL_ENGINE_CLASS_COPY;
   STATIC_ASSERT(IRIS_BATCH_COUNT == 3);

   if (debug_get_bool_option("INTEL_COMPUTE_CLASS", false) &&
       intel_engines_count(engines_info, INTEL_ENGINE_CLASS_COMPUTE) > 0)
      engine_classes[IRIS_BATCH_COMPUTE] = INTEL_ENGINE_CLASS_COMPUTE;
}

void iris_xe_init_batches(struct iris_context *ice)
{
   struct iris_screen *screen = (struct iris_screen *)ice->ctx.screen;
   struct iris_bufmgr *bufmgr = screen->bufmgr;
   const int fd = iris_bufmgr_get_fd(screen->bufmgr);
   enum intel_engine_class engine_classes[IRIS_BATCH_COUNT];
   struct intel_query_engine_info *engines_info;

   engines_info = intel_engine_get_info(fd, INTEL_KMD_TYPE_XE);
   assert(engines_info);
   if (!engines_info)
      return;
   iris_xe_map_intel_engine_class(engines_info, engine_classes);

   iris_foreach_batch(ice, batch) {
      const enum iris_batch_name name = batch - &ice->batches[0];
      ASSERTED bool ret;

      ret = iris_xe_init_batch(bufmgr, engines_info, engine_classes[name],
                               ice->priority, &batch->xe.exec_queue_id);
      assert(ret);
   }

   free(engines_info);
}

void iris_xe_destroy_batch(struct iris_batch *batch)
{
   struct iris_screen *screen = batch->screen;
   struct iris_bufmgr *bufmgr = screen->bufmgr;
   struct drm_xe_exec_queue_destroy destroy = {
      .exec_queue_id = batch->xe.exec_queue_id,
   };
   ASSERTED int ret;

   ret = intel_ioctl(iris_bufmgr_get_fd(bufmgr), DRM_IOCTL_XE_EXEC_QUEUE_DESTROY,
                     &destroy);
   assert(ret == 0);
}

bool iris_xe_replace_batch(struct iris_batch *batch)
{
   enum intel_engine_class engine_classes[IRIS_BATCH_COUNT];
   struct iris_screen *screen = batch->screen;
   struct iris_bufmgr *bufmgr = screen->bufmgr;
   struct iris_context *ice = batch->ice;
   struct intel_query_engine_info *engines_info;
   uint32_t new_exec_queue_id;
   bool ret;

   engines_info = intel_engine_get_info(iris_bufmgr_get_fd(bufmgr),
                                        INTEL_KMD_TYPE_XE);
   if (!engines_info)
      return false;
   iris_xe_map_intel_engine_class(engines_info, engine_classes);

   ret = iris_xe_init_batch(bufmgr, engines_info, engine_classes[batch->name],
                            ice->priority, &new_exec_queue_id);
   if (ret) {
      iris_xe_destroy_batch(batch);
      batch->xe.exec_queue_id = new_exec_queue_id;
      iris_lost_context_state(batch);
   }

   free(engines_info);
   return ret;
}
