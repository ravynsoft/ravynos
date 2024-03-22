/*
 * Copyright © 2020 Intel Corporation
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

#include "anv_measure.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "common/intel_measure.h"
#include "util/u_debug.h"

struct anv_measure_batch {
   struct anv_bo *bo;
   struct intel_measure_batch base;
};

void
anv_measure_device_init(struct anv_physical_device *device)
{
   switch (device->info.verx10) {
   case 80:
      device->cmd_emit_timestamp = &gfx8_cmd_emit_timestamp;
      break;
   case 75:
      device->cmd_emit_timestamp = &gfx75_cmd_emit_timestamp;
      break;
   case 70:
      device->cmd_emit_timestamp = &gfx7_cmd_emit_timestamp;
      break;
   default:
      assert(false);
   }

   /* initialise list of measure structures that await rendering */
   struct intel_measure_device *measure_device = &device->measure_device;
   intel_measure_init(measure_device);
   struct intel_measure_config *config = measure_device->config;
   if (config == NULL)
      return;

   /* the final member of intel_measure_ringbuffer is a zero-length array of
    * intel_measure_buffered_result objects.  Allocate additional space for
    * the buffered objects based on the run-time configurable buffer_size
    */
   const size_t rb_bytes = sizeof(struct intel_measure_ringbuffer) +
      config->buffer_size * sizeof(struct intel_measure_buffered_result);
   struct intel_measure_ringbuffer * rb =
      vk_zalloc(&device->instance->vk.alloc,
                rb_bytes, 8,
                VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   measure_device->ringbuffer = rb;
}

static struct intel_measure_config*
config_from_command_buffer(struct anv_cmd_buffer *cmd_buffer)
{
   return cmd_buffer->device->physical->measure_device.config;
}

void
anv_measure_init(struct anv_cmd_buffer *cmd_buffer)
{
   struct intel_measure_config *config = config_from_command_buffer(cmd_buffer);
   struct anv_device *device = cmd_buffer->device;

   if (!config || !config->enabled) {
      cmd_buffer->measure = NULL;
      return;
   }

   /* the final member of anv_measure is a zero-length array of
    * intel_measure_snapshot objects.  Create additional space for the
    * snapshot objects based on the run-time configurable batch_size
    */
   const size_t batch_bytes = sizeof(struct anv_measure_batch) +
      config->batch_size * sizeof(struct intel_measure_snapshot);
   struct anv_measure_batch * measure =
      vk_alloc(&cmd_buffer->vk.pool->alloc,
               batch_bytes, 8,
               VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

   memset(measure, 0, batch_bytes);
   ASSERTED VkResult result =
      anv_device_alloc_bo(device, "measure data",
                          config->batch_size * sizeof(uint64_t),
                          ANV_BO_ALLOC_MAPPED,
                          0,
                          (struct anv_bo**)&measure->bo);
   measure->base.timestamps = measure->bo->map;
   assert(result == VK_SUCCESS);

   cmd_buffer->measure = measure;
}

static void
anv_measure_start_snapshot(struct anv_cmd_buffer *cmd_buffer,
                           enum intel_measure_snapshot_type type,
                           const char *event_name,
                           uint32_t count)
{
   struct anv_batch *batch = &cmd_buffer->batch;
   struct anv_measure_batch *measure = cmd_buffer->measure;
   struct anv_physical_device *device = cmd_buffer->device->physical;
   struct intel_measure_device *measure_device = &device->measure_device;

   const unsigned device_frame = measure_device->frame;

   /* if the command buffer is not associated with a frame, associate it with
    * the most recent acquired frame
    */
   if (measure->base.frame == 0)
      measure->base.frame = device_frame;

//   uintptr_t framebuffer = (uintptr_t)cmd_buffer->state.framebuffer;
//
//   if (!measure->base.framebuffer &&
//       cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY)
//      /* secondary command buffer inherited the framebuffer from the primary */
//      measure->base.framebuffer = framebuffer;
//
//   /* verify framebuffer has been properly tracked */
//   assert(type == INTEL_SNAPSHOT_END ||
//          framebuffer == measure->base.framebuffer ||
//          framebuffer == 0 ); /* compute has no framebuffer */

   unsigned index = measure->base.index++;

   (*device->cmd_emit_timestamp)(batch, cmd_buffer->device,
                                 (struct anv_address) {
                                    .bo = measure->bo,
                                    .offset = index * sizeof(uint64_t) },
                                 ANV_TIMESTAMP_CAPTURE_AT_CS_STALL);

   if (event_name == NULL)
      event_name = intel_measure_snapshot_string(type);

   struct intel_measure_snapshot *snapshot = &(measure->base.snapshots[index]);
   memset(snapshot, 0, sizeof(*snapshot));
   snapshot->type = type;
   snapshot->count = (unsigned) count;
   snapshot->event_count = measure->base.event_count;
   snapshot->event_name = event_name;
//   snapshot->framebuffer = framebuffer;

   if (type == INTEL_SNAPSHOT_COMPUTE && cmd_buffer->state.compute.pipeline) {
      snapshot->cs = (uintptr_t) cmd_buffer->state.compute.pipeline->cs;
   } else if (cmd_buffer->state.gfx.pipeline) {
      const struct anv_graphics_pipeline *pipeline =
         cmd_buffer->state.gfx.pipeline;
      snapshot->vs = (uintptr_t) pipeline->shaders[MESA_SHADER_VERTEX];
      snapshot->tcs = (uintptr_t) pipeline->shaders[MESA_SHADER_TESS_CTRL];
      snapshot->tes = (uintptr_t) pipeline->shaders[MESA_SHADER_TESS_EVAL];
      snapshot->gs = (uintptr_t) pipeline->shaders[MESA_SHADER_GEOMETRY];
      snapshot->fs = (uintptr_t) pipeline->shaders[MESA_SHADER_FRAGMENT];
   }
}

static void
anv_measure_end_snapshot(struct anv_cmd_buffer *cmd_buffer,
                         uint32_t event_count)
{
   struct anv_batch *batch = &cmd_buffer->batch;
   struct anv_measure_batch *measure = cmd_buffer->measure;
   struct anv_physical_device *device = cmd_buffer->device->physical;

   unsigned index = measure->base.index++;
   assert(index % 2 == 1);

   (*device->cmd_emit_timestamp)(batch, cmd_buffer->device,
                                 (struct anv_address) {
                                    .bo = measure->bo,
                                    .offset = index * sizeof(uint64_t) },
                                 ANV_TIMESTAMP_CAPTURE_AT_CS_STALL);

   struct intel_measure_snapshot *snapshot = &(measure->base.snapshots[index]);
   memset(snapshot, 0, sizeof(*snapshot));
   snapshot->type = INTEL_SNAPSHOT_END;
   snapshot->event_count = event_count;
}

static bool
state_changed(struct anv_cmd_buffer *cmd_buffer,
              enum intel_measure_snapshot_type type)
{
   uintptr_t vs=0, tcs=0, tes=0, gs=0, fs=0, cs=0;

   if (cmd_buffer->usage_flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT)
      /* can't record timestamps in this mode */
      return false;

   if (type == INTEL_SNAPSHOT_COMPUTE) {
      const struct anv_compute_pipeline *cs_pipe =
         cmd_buffer->state.compute.pipeline;
      assert(cs_pipe);
      cs = (uintptr_t)cs_pipe->cs;
   } else if (type == INTEL_SNAPSHOT_DRAW) {
      const struct anv_graphics_pipeline *gfx = cmd_buffer->state.gfx.pipeline;
      assert(gfx);
      vs = (uintptr_t) gfx->shaders[MESA_SHADER_VERTEX];
      tcs = (uintptr_t) gfx->shaders[MESA_SHADER_TESS_CTRL];
      tes = (uintptr_t) gfx->shaders[MESA_SHADER_TESS_EVAL];
      gs = (uintptr_t) gfx->shaders[MESA_SHADER_GEOMETRY];
      fs = (uintptr_t) gfx->shaders[MESA_SHADER_FRAGMENT];
   }
   /* else blorp, all programs NULL */

   return intel_measure_state_changed(&cmd_buffer->measure->base,
                                      vs, tcs, tes, gs, fs, cs, 0, 0);
}

void
_anv_measure_snapshot(struct anv_cmd_buffer *cmd_buffer,
                     enum intel_measure_snapshot_type type,
                     const char *event_name,
                     uint32_t count)
{
   struct intel_measure_config *config = config_from_command_buffer(cmd_buffer);
   struct anv_measure_batch *measure = cmd_buffer->measure;

   assert(config);
   if (measure == NULL)
      return;

   assert(type != INTEL_SNAPSHOT_END);
   if (!state_changed(cmd_buffer, type)) {
      /* filter out this event */
      return;
   }

   /* increment event count */
   ++measure->base.event_count;
   if (measure->base.event_count == 1 ||
       measure->base.event_count == config->event_interval + 1) {
      /* the first event of an interval */

      if (measure->base.index % 2) {
         /* end the previous event */
         anv_measure_end_snapshot(cmd_buffer, measure->base.event_count - 1);
      }
      measure->base.event_count = 1;

      if (measure->base.index == config->batch_size) {
         /* Snapshot buffer is full.  The batch must be flushed before
          * additional snapshots can be taken.
          */
         static bool warned = false;
         if (unlikely(!warned)) {
            fprintf(config->file,
                    "WARNING: batch size exceeds INTEL_MEASURE limit: %d. "
                    "Data has been dropped. "
                    "Increase setting with INTEL_MEASURE=batch_size={count}\n",
                    config->batch_size);
         }

         warned = true;
         return;
      }

      anv_measure_start_snapshot(cmd_buffer, type, event_name, count);
   }
}

/**
 * Called when a command buffer is reset.  Re-initializes existing anv_measure
 * data structures.
 */
void
anv_measure_reset(struct anv_cmd_buffer *cmd_buffer)
{
   struct intel_measure_config *config = config_from_command_buffer(cmd_buffer);
   struct anv_device *device = cmd_buffer->device;
   struct anv_measure_batch *measure = cmd_buffer->measure;

   if (!config)
      return;

   if (!config->enabled) {
      cmd_buffer->measure = NULL;
      return;
   }

   if (!measure) {
      /* Capture has recently been enabled. Instead of resetting, a new data
       * structure must be allocated and initialized.
       */
      return anv_measure_init(cmd_buffer);
   }

   /* it is possible that the command buffer contains snapshots that have not
    * yet been processed
    */
   intel_measure_gather(&device->physical->measure_device,
                        device->info);

   assert(cmd_buffer->device != NULL);

   measure->base.index = 0;
//   measure->base.framebuffer = 0;
   measure->base.frame = 0;
   measure->base.event_count = 0;
   list_inithead(&measure->base.link);
}

void
anv_measure_destroy(struct anv_cmd_buffer *cmd_buffer)
{
   struct intel_measure_config *config = config_from_command_buffer(cmd_buffer);
   struct anv_measure_batch *measure = cmd_buffer->measure;
   struct anv_device *device = cmd_buffer->device;
   struct anv_physical_device *physical = device->physical;

   if (!config)
      return;
   if (measure == NULL)
      return;

   /* it is possible that the command buffer contains snapshots that have not
    * yet been processed
    */
   intel_measure_gather(&physical->measure_device, &physical->info);

   anv_device_release_bo(device, measure->bo);
   vk_free(&cmd_buffer->vk.pool->alloc, measure);
   cmd_buffer->measure = NULL;
}

static struct intel_measure_config*
config_from_device(struct anv_device *device)
{
   return device->physical->measure_device.config;
}

void
anv_measure_device_destroy(struct anv_physical_device *device)
{
   struct intel_measure_device *measure_device = &device->measure_device;
   struct intel_measure_config *config = measure_device->config;

   if (!config)
      return;

   if (measure_device->ringbuffer != NULL) {
      vk_free(&device->instance->vk.alloc, measure_device->ringbuffer);
      measure_device->ringbuffer = NULL;
   }
}

/**
 *  Hook for command buffer submission.
 */
void
_anv_measure_submit(struct anv_cmd_buffer *cmd_buffer)
{
   struct intel_measure_config *config = config_from_command_buffer(cmd_buffer);
   struct anv_measure_batch *measure = cmd_buffer->measure;
   struct intel_measure_device *measure_device = &cmd_buffer->device->physical->measure_device;

   if (!config)
      return;
   if (measure == NULL)
      return;

   struct intel_measure_batch *base = &measure->base;
   if (base->index == 0)
      /* no snapshots were started */
      return;

   /* finalize snapshots and enqueue them */
   static unsigned cmd_buffer_count = 0;
   base->batch_count = p_atomic_inc_return(&cmd_buffer_count);

   if (base->index %2 == 1) {
      anv_measure_end_snapshot(cmd_buffer, base->event_count);
      base->event_count = 0;
   }

   /* Mark the final timestamp as 'not completed'.  This marker will be used
    * to verify that rendering is complete.
    */
   base->timestamps[base->index - 1] = 0;

   /* add to the list of submitted snapshots */
   pthread_mutex_lock(&measure_device->mutex);
   list_addtail(&measure->base.link, &measure_device->queued_snapshots);
   pthread_mutex_unlock(&measure_device->mutex);
}

/**
 *  Hook for the start of a frame.
 */
void
_anv_measure_acquire(struct anv_device *device)
{
   struct intel_measure_config *config = config_from_device(device);
   struct intel_measure_device *measure_device = &device->physical->measure_device;

   if (!config)
      return;
   if (measure_device == NULL)
      return;

   intel_measure_frame_transition(p_atomic_inc_return(&measure_device->frame));

   /* iterate the queued snapshots and publish those that finished */
   intel_measure_gather(measure_device, &device->physical->info);
}

void
_anv_measure_endcommandbuffer(struct anv_cmd_buffer *cmd_buffer)
{
   struct intel_measure_config *config = config_from_command_buffer(cmd_buffer);
   struct anv_measure_batch *measure = cmd_buffer->measure;

   if (!config)
      return;
   if (measure == NULL)
      return;
   if (measure->base.index % 2 == 0)
      return;

   anv_measure_end_snapshot(cmd_buffer, measure->base.event_count);
   measure->base.event_count = 0;
}

void
_anv_measure_beginrenderpass(struct anv_cmd_buffer *cmd_buffer)
{
   struct intel_measure_config *config = config_from_command_buffer(cmd_buffer);
   struct anv_measure_batch *measure = cmd_buffer->measure;

   if (!config)
      return;
   if (measure == NULL)
      return;

//   if (measure->base.framebuffer == (uintptr_t) cmd_buffer->state.framebuffer)
//      /* no change */
//      return;

   bool filtering = (config->flags & (INTEL_MEASURE_RENDERPASS |
                                      INTEL_MEASURE_SHADER));
   if (filtering && measure->base.index % 2 == 1) {
      /* snapshot for previous renderpass was not ended */
      anv_measure_end_snapshot(cmd_buffer,
                               measure->base.event_count);
      measure->base.event_count = 0;
   }

//   measure->base.framebuffer = (uintptr_t) cmd_buffer->state.framebuffer;
}

void
_anv_measure_add_secondary(struct anv_cmd_buffer *primary,
                           struct anv_cmd_buffer *secondary)
{
   struct intel_measure_config *config = config_from_command_buffer(primary);
   struct anv_measure_batch *measure = primary->measure;
   if (!config)
      return;
   if (measure == NULL)
      return;
   if (config->flags & (INTEL_MEASURE_BATCH | INTEL_MEASURE_FRAME))
      /* secondary timing will be contained within the primary */
      return;
   if (secondary->usage_flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT) {
         static bool warned = false;
         if (unlikely(!warned)) {
            fprintf(config->file,
                    "WARNING: INTEL_MEASURE cannot capture timings of commands "
                    "in secondary command buffers with "
                    "VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT set.\n");
         }
      return;
   }

   if (measure->base.index % 2 == 1)
      anv_measure_end_snapshot(primary, measure->base.event_count);

   struct intel_measure_snapshot *snapshot = &(measure->base.snapshots[measure->base.index]);
   _anv_measure_snapshot(primary, INTEL_SNAPSHOT_SECONDARY_BATCH, NULL, 0);

   snapshot->secondary = &secondary->measure->base;
}
