/*
 * Copyright © 2023 Google LLC
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

#include "perfetto.h"

#include "util/hash_table.h"
#include "util/perf/u_trace.h"
#include "util/ralloc.h"

using perfetto::DataSource;
template <typename DataSourceType, typename DataSourceTraits>
class MesaRenderpassDataSource
    : public perfetto::DataSource<DataSourceType, DataSourceTraits> {

 public:
   typedef typename perfetto::DataSource<DataSourceType,
                                         DataSourceTraits>::TraceContext
      TraceContext;

   void OnSetup(const perfetto::DataSourceBase::SetupArgs &) override
   {
      // Use this callback to apply any custom configuration to your data
      // source based on the TraceConfig in SetupArgs.
      debug_markers = NULL;
   }

   void OnStart(const perfetto::DataSourceBase::StartArgs &) override
   {
      debug_markers = _mesa_hash_table_create(NULL, _mesa_hash_string,
                                              _mesa_key_string_equal);
      // This notification can be used to initialize the GPU driver, enable
      // counters, etc. StartArgs will contains the DataSourceDescriptor,
      // which can be extended.
      u_trace_perfetto_start();
      PERFETTO_LOG("Tracing started");
   }

   void OnStop(const perfetto::DataSourceBase::StopArgs &) override
   {
      PERFETTO_LOG("Tracing stopped");

      // Undo any initialization done in OnStart.
      u_trace_perfetto_stop();
      // TODO we should perhaps block until queued traces are flushed?

      static_cast<DataSourceType *>(this)->Trace([](auto ctx) {
         auto packet = ctx.NewTracePacket();
         packet->Finalize();
         ctx.Flush();
      });

      ralloc_free(debug_markers);
   }

   /* Emits a clock sync trace event.  Perfetto uses periodic clock events
    * like this to sync up our GPU render stages with the CPU on the same
    * timeline, since clocks always drift over time.  Note that perfetto
    * relies on gpu_ts being monotonic, and will perform badly if it goes
    * backwards -- see tu_perfetto.cc for an example implemntation of handling
    * going backwards.
    */
   static void EmitClockSync(TraceContext &ctx,
                             uint64_t cpu_ts,
                             uint64_t gpu_ts,
                             uint32_t gpu_clock_id)
   {
      auto packet = ctx.NewTracePacket();

      packet->set_timestamp_clock_id(
         perfetto::protos::pbzero::BUILTIN_CLOCK_BOOTTIME);
      packet->set_timestamp(cpu_ts);

      auto event = packet->set_clock_snapshot();

      {
         auto clock = event->add_clocks();

         clock->set_clock_id(
            perfetto::protos::pbzero::BUILTIN_CLOCK_BOOTTIME);
         clock->set_timestamp(cpu_ts);
      }

      {
         auto clock = event->add_clocks();

         clock->set_clock_id(gpu_clock_id);
         clock->set_timestamp(gpu_ts);
      }
   }

   /* Returns a stage iid to use for a command stream or queue annotation.
    *
    * Using a new stage lets the annotation string show up right on the track
    * event in the UI, rather than needing to click into the event to find the
    * name in the metadata.  Intended for use with
    * vkCmdBeginDebugUtilsLabelEXT() and glPushDebugGroup().
    *
    * Note that SEQ_INCREMENTAL_STATE_CLEARED must have been set in the
    * sequence before this is called.
    */
   uint64_t debug_marker_stage(TraceContext &ctx, const char *name)
   {
      struct hash_entry *entry = _mesa_hash_table_search(debug_markers, name);
      const uint64_t dynamic_iid_base = 1ull << 32;

      if (entry) {
         return dynamic_iid_base + (uint32_t) (uintptr_t) entry->data;
      } else {
         uint64_t iid = dynamic_iid_base + debug_markers->entries;

         auto packet = ctx.NewTracePacket();
         auto interned_data = packet->set_interned_data();

         auto desc = interned_data->add_gpu_specifications();
         desc->set_iid(iid);
         desc->set_name(name);

         /* We only track the entry count in entry->data, because the
          * dynamic_iid_base would get lost on 32-bit builds.
          */
         _mesa_hash_table_insert(debug_markers,
                                 ralloc_strdup(debug_markers, name),
                                 (void *) (uintptr_t) debug_markers->entries);

         return iid;
      }
   }

 private:
   /* Hash table of application generated events (string -> iid) (use
    * tctx.GetDataSourceLocked()->debug_marker_stage() to get a stage iid)
    */
   struct hash_table *debug_markers;
};

/* Begin the C API section. */
