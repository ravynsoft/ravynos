/*
 * Copyright © 2021 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdarg.h>

#include "common/intel_gem.h"
#include "perf/intel_perf.h"

#include "util/hash_table.h"
#include "util/u_process.h"

#include "intel_driver_ds.h"
#include "intel_pps_priv.h"
#include "intel_tracepoints.h"

#ifdef HAVE_PERFETTO

#include "util/perf/u_perfetto.h"
#include "util/perf/u_perfetto_renderpass.h"

#include "intel_tracepoints_perfetto.h"

/* Just naming stages */
static const struct {
   const char *name;

   /* The perfetto UI requires that there is a parent-child relationship
    * within a row of elements. Which means that all children elements must
    * end within the lifespan of their parent.
    *
    * Some elements like stalls and command buffers follow that relationship,
    * but not all. This tells us in which UI row the elements should live.
    */
   enum intel_ds_queue_stage draw_stage;
} intel_queue_stage_desc[INTEL_DS_QUEUE_STAGE_N_STAGES] = {
   /* Order must match the enum! */
   {
      "queue",
      INTEL_DS_QUEUE_STAGE_QUEUE,
   },
   {
      "frame",
      INTEL_DS_QUEUE_STAGE_FRAME,
   },
   {
      "cmd-buffer",
      INTEL_DS_QUEUE_STAGE_CMD_BUFFER,
   },
   {
      "internal-ops",
      INTEL_DS_QUEUE_STAGE_INTERNAL_OPS,
   },
   {
      "stall",
      INTEL_DS_QUEUE_STAGE_STALL,
   },
   {
      "compute",
      INTEL_DS_QUEUE_STAGE_COMPUTE,
   },
   {
      "as-build",
      INTEL_DS_QUEUE_STAGE_AS,
   },
   {
      "RT",
      INTEL_DS_QUEUE_STAGE_RT,
   },
   {
      "render-pass",
      INTEL_DS_QUEUE_STAGE_RENDER_PASS,
   },
   {
      "blorp",
      INTEL_DS_QUEUE_STAGE_BLORP,
   },
   {
      "draw",
      INTEL_DS_QUEUE_STAGE_DRAW,
   },
   {
      "draw_mesh",
      INTEL_DS_QUEUE_STAGE_DRAW_MESH,
   },
};

struct IntelRenderpassIncrementalState {
   bool was_cleared = true;
};

struct IntelRenderpassTraits : public perfetto::DefaultDataSourceTraits {
   using IncrementalStateType = IntelRenderpassIncrementalState;
};

class IntelRenderpassDataSource : public MesaRenderpassDataSource<IntelRenderpassDataSource,
                                                                  IntelRenderpassTraits> {
public:
   /* Make sure we're not losing traces due to lack of shared memory space */
   constexpr static perfetto::BufferExhaustedPolicy kBufferExhaustedPolicy =
      perfetto::BufferExhaustedPolicy::kDrop;
};

PERFETTO_DECLARE_DATA_SOURCE_STATIC_MEMBERS(IntelRenderpassDataSource);
PERFETTO_DEFINE_DATA_SOURCE_STATIC_MEMBERS(IntelRenderpassDataSource);

using perfetto::protos::pbzero::InternedGpuRenderStageSpecification_RenderStageCategory;

static void
sync_timestamp(IntelRenderpassDataSource::TraceContext &ctx,
               struct intel_ds_device *device)
{
   uint64_t cpu_ts, gpu_ts;

   if (!intel_gem_read_correlate_cpu_gpu_timestamp(device->fd,
                                                   device->info.kmd_type,
                                                   INTEL_ENGINE_CLASS_RENDER, 0,
                                                   CLOCK_BOOTTIME,
                                                   &cpu_ts, &gpu_ts, NULL)) {
      cpu_ts = perfetto::base::GetBootTimeNs().count();
      intel_gem_read_render_timestamp(device->fd, device->info.kmd_type,
                                      &gpu_ts);
   }
   gpu_ts = intel_device_info_timebase_scale(&device->info, gpu_ts);

   if (cpu_ts < device->next_clock_sync_ns)
      return;

   PERFETTO_LOG("sending clocks gpu=0x%08x", device->gpu_clock_id);

   device->sync_gpu_ts = gpu_ts;
   device->next_clock_sync_ns = cpu_ts + 1000000000ull;

   MesaRenderpassDataSource<IntelRenderpassDataSource, IntelRenderpassTraits>::EmitClockSync(ctx,
      cpu_ts, gpu_ts, device->gpu_clock_id);
}

static void
send_descriptors(IntelRenderpassDataSource::TraceContext &ctx,
                 struct intel_ds_device *device)
{
   PERFETTO_LOG("Sending renderstage descriptors");

   device->event_id = 0;
   list_for_each_entry_safe(struct intel_ds_queue, queue, &device->queues, link) {
      for (uint32_t s = 0; s < ARRAY_SIZE(queue->stages); s++) {
         queue->stages[s].start_ns[0] = 0;
      }
   }

   {
      auto packet = ctx.NewTracePacket();

      packet->set_timestamp(perfetto::base::GetBootTimeNs().count());
      packet->set_timestamp_clock_id(perfetto::protos::pbzero::BUILTIN_CLOCK_BOOTTIME);
      packet->set_sequence_flags(perfetto::protos::pbzero::TracePacket::SEQ_INCREMENTAL_STATE_CLEARED);

      auto interned_data = packet->set_interned_data();

      {
         auto desc = interned_data->add_graphics_contexts();
         desc->set_iid(device->iid);
         desc->set_pid(getpid());
         switch (device->api) {
         case INTEL_DS_API_OPENGL:
            desc->set_api(perfetto::protos::pbzero::InternedGraphicsContext_Api::OPEN_GL);
            break;
         case INTEL_DS_API_VULKAN:
            desc->set_api(perfetto::protos::pbzero::InternedGraphicsContext_Api::VULKAN);
            break;
         default:
            break;
         }
      }

      /* Emit all the IID picked at device/queue creation. */
      list_for_each_entry_safe(struct intel_ds_queue, queue, &device->queues, link) {
         for (unsigned s = 0; s < INTEL_DS_QUEUE_STAGE_N_STAGES; s++) {
            {
               /* We put the stage number in there so that all rows are order
                * by intel_ds_queue_stage.
                */
               char name[100];
               snprintf(name, sizeof(name), "%.10s-%s-%u-%s",
                        util_get_process_name(),
                        queue->name, s, intel_queue_stage_desc[s].name);

               auto desc = interned_data->add_gpu_specifications();
               desc->set_iid(queue->stages[s].queue_iid);
               desc->set_name(name);
            }
         }
      }

      for (unsigned i = 0; i < ARRAY_SIZE(intel_tracepoint_names); i++) {
         /* Skip the begin tracepoint, the label represent the couple of
          * begin/end tracepoints.
          */
         if (strstr(intel_tracepoint_names[i], "intel_begin_") != NULL)
            continue;

         auto desc = interned_data->add_gpu_specifications();
         desc->set_iid(device->tracepoint_iids[i]);
         desc->set_name(intel_tracepoint_names[i] + strlen("intel_end_"));
      }
   }

   device->next_clock_sync_ns = 0;
   sync_timestamp(ctx, device);
}

typedef void (*trace_payload_as_extra_func)(perfetto::protos::pbzero::GpuRenderStageEvent *, const void*);

static void
begin_event(struct intel_ds_queue *queue, uint64_t ts_ns,
            enum intel_ds_queue_stage stage_id)
{
   uint32_t level = queue->stages[stage_id].level;
   /* If we haven't managed to calibrate the alignment between GPU and CPU
    * timestamps yet, then skip this trace, otherwise perfetto won't know
    * what to do with it.
    */
   if (!queue->device->sync_gpu_ts) {
      queue->stages[stage_id].start_ns[level] = 0;
      return;
   }

   if (level >= (ARRAY_SIZE(queue->stages[stage_id].start_ns) - 1))
      return;

   queue->stages[stage_id].start_ns[level] = ts_ns;
   queue->stages[stage_id].level++;
}

static void
end_event(struct intel_ds_queue *queue, uint64_t ts_ns,
          enum intel_ds_queue_stage stage_id,
          uint32_t submission_id,
          uint16_t tracepoint_idx,
          const char *app_event,
          const void* payload = nullptr,
          trace_payload_as_extra_func payload_as_extra = nullptr)
{
   struct intel_ds_device *device = queue->device;

   /* If we haven't managed to calibrate the alignment between GPU and CPU
    * timestamps yet, then skip this trace, otherwise perfetto won't know
    * what to do with it.
    */
   if (!device->sync_gpu_ts)
      return;

   if (queue->stages[stage_id].level == 0)
      return;

   uint32_t level = --queue->stages[stage_id].level;
   struct intel_ds_stage *stage = &queue->stages[stage_id];
   uint64_t start_ns = stage->start_ns[level];

   if (!start_ns)
      return;


   IntelRenderpassDataSource::Trace([=](IntelRenderpassDataSource::TraceContext tctx) {
      if (auto state = tctx.GetIncrementalState(); state->was_cleared) {
         send_descriptors(tctx, queue->device);
         state->was_cleared = false;
      }

      sync_timestamp(tctx, queue->device);

      uint64_t evt_id = device->event_id++;

      /* If this is an application event, we might need to generate a new
       * stage_iid if not already seen. Otherwise, it's a driver event and we
       * have use the internal stage_iid.
       */
      uint64_t stage_iid = app_event ?
         tctx.GetDataSourceLocked()->debug_marker_stage(tctx, app_event) :
         device->tracepoint_iids[tracepoint_idx];

      auto packet = tctx.NewTracePacket();

      packet->set_timestamp(start_ns);
      packet->set_timestamp_clock_id(queue->device->gpu_clock_id);

      assert(ts_ns >= start_ns);

      auto event = packet->set_gpu_render_stage_event();
      event->set_gpu_id(queue->device->gpu_id);

      event->set_hw_queue_iid(stage->queue_iid);
      event->set_stage_iid(stage_iid);
      event->set_context(queue->device->iid);
      event->set_event_id(evt_id);
      event->set_duration(ts_ns - start_ns);
      event->set_submission_id(submission_id);

      if (payload && payload_as_extra) {
         payload_as_extra(event, payload);
      }
   });

   stage->start_ns[level] = 0;
}

static void
custom_trace_payload_as_extra_end_stall(perfetto::protos::pbzero::GpuRenderStageEvent *event,
                                        const struct trace_intel_end_stall *payload)
{
   char buf[256];

   {
      auto data = event->add_extra_data();
      data->set_name("stall_reason");

      snprintf(buf, sizeof(buf), "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s : %s",
              (payload->flags & INTEL_DS_DEPTH_CACHE_FLUSH_BIT) ? "+depth_flush" : "",
              (payload->flags & INTEL_DS_DATA_CACHE_FLUSH_BIT) ? "+dc_flush" : "",
              (payload->flags & INTEL_DS_HDC_PIPELINE_FLUSH_BIT) ? "+hdc_flush" : "",
              (payload->flags & INTEL_DS_RENDER_TARGET_CACHE_FLUSH_BIT) ? "+rt_flush" : "",
              (payload->flags & INTEL_DS_TILE_CACHE_FLUSH_BIT) ? "+tile_flush" : "",
              (payload->flags & INTEL_DS_STATE_CACHE_INVALIDATE_BIT) ? "+state_inv" : "",
              (payload->flags & INTEL_DS_CONST_CACHE_INVALIDATE_BIT) ? "+const_inv" : "",
              (payload->flags & INTEL_DS_VF_CACHE_INVALIDATE_BIT) ? "+vf_inv" : "",
              (payload->flags & INTEL_DS_TEXTURE_CACHE_INVALIDATE_BIT) ? "+tex_inv" : "",
              (payload->flags & INTEL_DS_INST_CACHE_INVALIDATE_BIT) ? "+inst_inv" : "",
              (payload->flags & INTEL_DS_STALL_AT_SCOREBOARD_BIT) ? "+pb_stall" : "",
              (payload->flags & INTEL_DS_DEPTH_STALL_BIT) ? "+depth_stall" : "",
              (payload->flags & INTEL_DS_HDC_PIPELINE_FLUSH_BIT) ? "+hdc_flush" : "",
              (payload->flags & INTEL_DS_CS_STALL_BIT) ? "+cs_stall" : "",
              (payload->flags & INTEL_DS_UNTYPED_DATAPORT_CACHE_FLUSH_BIT) ? "+udp_flush" : "",
              (payload->flags & INTEL_DS_END_OF_PIPE_BIT) ? "+eop" : "",
              (payload->flags & INTEL_DS_CCS_CACHE_FLUSH_BIT) ? "+ccs_flush" : "",
              payload->reason ? payload->reason : "unknown");

      assert(strlen(buf) > 0);

      data->set_value(buf);
   }
}

#endif /* HAVE_PERFETTO */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_PERFETTO

/*
 * Trace callbacks, called from u_trace once the timestamps from GPU have been
 * collected.
 */

#define CREATE_DUAL_EVENT_CALLBACK(event_name, stage)                   \
   void                                                                 \
   intel_ds_begin_##event_name(struct intel_ds_device *device,          \
                               uint64_t ts_ns,                          \
                               uint16_t tp_idx,                         \
                               const void *flush_data,                  \
                               const struct trace_intel_begin_##event_name *payload) \
   {                                                                    \
      const struct intel_ds_flush_data *flush =                         \
         (const struct intel_ds_flush_data *) flush_data;               \
      begin_event(flush->queue, ts_ns, stage);                          \
   }                                                                    \
                                                                        \
   void                                                                 \
   intel_ds_end_##event_name(struct intel_ds_device *device,            \
                             uint64_t ts_ns,                            \
                             uint16_t tp_idx,                           \
                             const void *flush_data,                    \
                             const struct trace_intel_end_##event_name *payload) \
   {                                                                    \
      const struct intel_ds_flush_data *flush =                         \
         (const struct intel_ds_flush_data *) flush_data;               \
      end_event(flush->queue, ts_ns, stage, flush->submission_id,       \
                tp_idx, NULL, payload,                                  \
                (trace_payload_as_extra_func)                           \
                &trace_payload_as_extra_intel_end_##event_name);        \
   }                                                                    \

CREATE_DUAL_EVENT_CALLBACK(frame, INTEL_DS_QUEUE_STAGE_FRAME)
CREATE_DUAL_EVENT_CALLBACK(batch, INTEL_DS_QUEUE_STAGE_CMD_BUFFER)
CREATE_DUAL_EVENT_CALLBACK(cmd_buffer, INTEL_DS_QUEUE_STAGE_CMD_BUFFER)
CREATE_DUAL_EVENT_CALLBACK(render_pass, INTEL_DS_QUEUE_STAGE_RENDER_PASS)
CREATE_DUAL_EVENT_CALLBACK(blorp, INTEL_DS_QUEUE_STAGE_BLORP)
CREATE_DUAL_EVENT_CALLBACK(draw, INTEL_DS_QUEUE_STAGE_DRAW)
CREATE_DUAL_EVENT_CALLBACK(draw_indexed, INTEL_DS_QUEUE_STAGE_DRAW)
CREATE_DUAL_EVENT_CALLBACK(draw_indexed_multi, INTEL_DS_QUEUE_STAGE_DRAW)
CREATE_DUAL_EVENT_CALLBACK(draw_indexed_indirect, INTEL_DS_QUEUE_STAGE_DRAW)
CREATE_DUAL_EVENT_CALLBACK(draw_multi, INTEL_DS_QUEUE_STAGE_DRAW)
CREATE_DUAL_EVENT_CALLBACK(draw_indirect, INTEL_DS_QUEUE_STAGE_DRAW)
CREATE_DUAL_EVENT_CALLBACK(draw_indirect_count, INTEL_DS_QUEUE_STAGE_DRAW)
CREATE_DUAL_EVENT_CALLBACK(draw_indirect_byte_count, INTEL_DS_QUEUE_STAGE_DRAW)
CREATE_DUAL_EVENT_CALLBACK(draw_indexed_indirect_count, INTEL_DS_QUEUE_STAGE_DRAW)
CREATE_DUAL_EVENT_CALLBACK(draw_mesh, INTEL_DS_QUEUE_STAGE_DRAW_MESH)
CREATE_DUAL_EVENT_CALLBACK(draw_mesh_indirect, INTEL_DS_QUEUE_STAGE_DRAW_MESH)
CREATE_DUAL_EVENT_CALLBACK(draw_mesh_indirect_count, INTEL_DS_QUEUE_STAGE_DRAW_MESH)
CREATE_DUAL_EVENT_CALLBACK(xfb, INTEL_DS_QUEUE_STAGE_CMD_BUFFER)
CREATE_DUAL_EVENT_CALLBACK(compute, INTEL_DS_QUEUE_STAGE_COMPUTE)
CREATE_DUAL_EVENT_CALLBACK(generate_draws, INTEL_DS_QUEUE_STAGE_INTERNAL_OPS)
CREATE_DUAL_EVENT_CALLBACK(trace_copy, INTEL_DS_QUEUE_STAGE_INTERNAL_OPS)
CREATE_DUAL_EVENT_CALLBACK(trace_copy_cb, INTEL_DS_QUEUE_STAGE_INTERNAL_OPS)
CREATE_DUAL_EVENT_CALLBACK(query_clear_blorp, INTEL_DS_QUEUE_STAGE_INTERNAL_OPS)
CREATE_DUAL_EVENT_CALLBACK(query_clear_cs, INTEL_DS_QUEUE_STAGE_INTERNAL_OPS)
CREATE_DUAL_EVENT_CALLBACK(query_copy_cs, INTEL_DS_QUEUE_STAGE_INTERNAL_OPS)
CREATE_DUAL_EVENT_CALLBACK(query_copy_shader, INTEL_DS_QUEUE_STAGE_INTERNAL_OPS)
CREATE_DUAL_EVENT_CALLBACK(write_buffer_marker, INTEL_DS_QUEUE_STAGE_CMD_BUFFER)
CREATE_DUAL_EVENT_CALLBACK(rays, INTEL_DS_QUEUE_STAGE_RT)
CREATE_DUAL_EVENT_CALLBACK(as_build, INTEL_DS_QUEUE_STAGE_AS)

void
intel_ds_begin_cmd_buffer_annotation(struct intel_ds_device *device,
                                     uint64_t ts_ns,
                                     uint16_t tp_idx,
                                     const void *flush_data,
                                     const struct trace_intel_begin_cmd_buffer_annotation *payload)
{
   const struct intel_ds_flush_data *flush =
      (const struct intel_ds_flush_data *) flush_data;
   begin_event(flush->queue, ts_ns, INTEL_DS_QUEUE_STAGE_CMD_BUFFER);
}

void
intel_ds_end_cmd_buffer_annotation(struct intel_ds_device *device,
                                   uint64_t ts_ns,
                                   uint16_t tp_idx,
                                   const void *flush_data,
                                   const struct trace_intel_end_cmd_buffer_annotation *payload)
{
   const struct intel_ds_flush_data *flush =
      (const struct intel_ds_flush_data *) flush_data;
   end_event(flush->queue, ts_ns, INTEL_DS_QUEUE_STAGE_CMD_BUFFER,
             flush->submission_id, tp_idx, payload->str, NULL, NULL);
}

void
intel_ds_begin_queue_annotation(struct intel_ds_device *device,
                                uint64_t ts_ns,
                                uint16_t tp_idx,
                                const void *flush_data,
                                const struct trace_intel_begin_queue_annotation *payload)
{
   const struct intel_ds_flush_data *flush =
      (const struct intel_ds_flush_data *) flush_data;
   begin_event(flush->queue, ts_ns, INTEL_DS_QUEUE_STAGE_QUEUE);
}

void
intel_ds_end_queue_annotation(struct intel_ds_device *device,
                              uint64_t ts_ns,
                              uint16_t tp_idx,
                              const void *flush_data,
                              const struct trace_intel_end_queue_annotation *payload)
{
   const struct intel_ds_flush_data *flush =
      (const struct intel_ds_flush_data *) flush_data;
   end_event(flush->queue, ts_ns, INTEL_DS_QUEUE_STAGE_QUEUE,
             flush->submission_id, tp_idx, payload->str, NULL, NULL);
}

void
intel_ds_begin_stall(struct intel_ds_device *device,
                     uint64_t ts_ns,
                     uint16_t tp_idx,
                     const void *flush_data,
                     const struct trace_intel_begin_stall *payload)
{
   const struct intel_ds_flush_data *flush =
      (const struct intel_ds_flush_data *) flush_data;
   begin_event(flush->queue, ts_ns, INTEL_DS_QUEUE_STAGE_STALL);
}

void
intel_ds_end_stall(struct intel_ds_device *device,
                   uint64_t ts_ns,
                   uint16_t tp_idx,
                   const void *flush_data,
                   const struct trace_intel_end_stall *payload)
{
   const struct intel_ds_flush_data *flush =
      (const struct intel_ds_flush_data *) flush_data;
   end_event(flush->queue, ts_ns, INTEL_DS_QUEUE_STAGE_STALL,
             flush->submission_id, tp_idx, NULL, payload,
             (trace_payload_as_extra_func)custom_trace_payload_as_extra_end_stall);
}

uint64_t
intel_ds_begin_submit(struct intel_ds_queue *queue)
{
   return perfetto::base::GetBootTimeNs().count();
}

void
intel_ds_end_submit(struct intel_ds_queue *queue,
                    uint64_t start_ts)
{
   if (!u_trace_should_process(&queue->device->trace_context)) {
      queue->device->sync_gpu_ts = 0;
      queue->device->next_clock_sync_ns = 0;
      return;
   }

   uint64_t end_ts = perfetto::base::GetBootTimeNs().count();
   uint32_t submission_id = queue->submission_id++;

   IntelRenderpassDataSource::Trace([=](IntelRenderpassDataSource::TraceContext tctx) {
      if (auto state = tctx.GetIncrementalState(); state->was_cleared) {
         send_descriptors(tctx, queue->device);
         state->was_cleared = false;
      }

      sync_timestamp(tctx, queue->device);

      auto packet = tctx.NewTracePacket();

      packet->set_timestamp(start_ts);

      auto event = packet->set_vulkan_api_event();
      auto submit = event->set_vk_queue_submit();

      // submit->set_pid(os_get_pid());
      // submit->set_tid(os_get_tid());
      submit->set_duration_ns(end_ts - start_ts);
      submit->set_vk_queue((uintptr_t) queue);
      submit->set_submission_id(submission_id);
   });
}

#endif /* HAVE_PERFETTO */

static void
intel_driver_ds_init_once(void)
{
#ifdef HAVE_PERFETTO
   util_perfetto_init();
   perfetto::DataSourceDescriptor dsd;
   dsd.set_name("gpu.renderstages.intel");
   IntelRenderpassDataSource::Register(dsd);
#endif
}

static once_flag intel_driver_ds_once_flag = ONCE_FLAG_INIT;
static uint64_t iid = 1;

static uint64_t get_iid()
{
   return iid++;
}

void
intel_driver_ds_init(void)
{
   call_once(&intel_driver_ds_once_flag,
             intel_driver_ds_init_once);
   intel_gpu_tracepoint_config_variable();
}

void
intel_ds_device_init(struct intel_ds_device *device,
                     const struct intel_device_info *devinfo,
                     int drm_fd,
                     uint32_t gpu_id,
                     enum intel_ds_api api)
{
   memset(device, 0, sizeof(*device));

   device->gpu_id = gpu_id;
   device->gpu_clock_id = intel_pps_clock_id(gpu_id);
   device->fd = drm_fd;
   device->info = *devinfo;
   device->iid = get_iid();
   device->api = api;

#ifdef HAVE_PERFETTO
   assert(ARRAY_SIZE(intel_tracepoint_names) < ARRAY_SIZE(device->tracepoint_iids));
   for (unsigned i = 0; i < ARRAY_SIZE(intel_tracepoint_names); i++)
      device->tracepoint_iids[i] = get_iid();
#endif

   list_inithead(&device->queues);
   simple_mtx_init(&device->trace_context_mutex, mtx_plain);
}

void
intel_ds_device_fini(struct intel_ds_device *device)
{
   u_trace_context_fini(&device->trace_context);
   simple_mtx_destroy(&device->trace_context_mutex);
}

struct intel_ds_queue *
intel_ds_device_init_queue(struct intel_ds_device *device,
                           struct intel_ds_queue *queue,
                           const char *fmt_name,
                           ...)
{
   va_list ap;

   memset(queue, 0, sizeof(*queue));

   queue->device = device;

   va_start(ap, fmt_name);
   vsnprintf(queue->name, sizeof(queue->name), fmt_name, ap);
   va_end(ap);

   for (unsigned s = 0; s < INTEL_DS_QUEUE_STAGE_N_STAGES; s++) {
      queue->stages[s].queue_iid = get_iid();
   }

   list_add(&queue->link, &device->queues);

   return queue;
}

void intel_ds_flush_data_init(struct intel_ds_flush_data *data,
                              struct intel_ds_queue *queue,
                              uint64_t submission_id)
{
   memset(data, 0, sizeof(*data));

   data->queue = queue;
   data->submission_id = submission_id;

   u_trace_init(&data->trace, &queue->device->trace_context);
}

void intel_ds_flush_data_fini(struct intel_ds_flush_data *data)
{
   u_trace_fini(&data->trace);
}

void intel_ds_queue_flush_data(struct intel_ds_queue *queue,
                               struct u_trace *ut,
                               struct intel_ds_flush_data *data,
                               bool free_data)
{
   simple_mtx_lock(&queue->device->trace_context_mutex);
   u_trace_flush(ut, data, free_data);
   simple_mtx_unlock(&queue->device->trace_context_mutex);
}

void intel_ds_device_process(struct intel_ds_device *device,
                             bool eof)
{
   simple_mtx_lock(&device->trace_context_mutex);
   u_trace_context_process(&device->trace_context, eof);
   simple_mtx_unlock(&device->trace_context_mutex);
}

#ifdef __cplusplus
}
#endif
