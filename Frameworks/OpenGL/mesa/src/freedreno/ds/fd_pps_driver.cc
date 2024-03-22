/*
 * Copyright © 2021 Google, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "fd_pps_driver.h"

#include <cstring>
#include <iostream>
#include <perfetto.h>

#include "pps/pps.h"
#include "pps/pps_algorithm.h"

namespace pps
{

double
safe_div(uint64_t a, uint64_t b)
{
   if (b == 0)
      return 0;

   return a / static_cast<double>(b);
}

float
percent(uint64_t a, uint64_t b)
{
   /* Sometimes we get bogus values but we want for the timeline
    * to look nice without higher than 100% values.
    */
   if (b == 0 || a > b)
      return 0;

   return 100.f * (a / static_cast<double>(b));
}

bool
FreedrenoDriver::is_dump_perfcnt_preemptible() const
{
   return false;
}

uint64_t
FreedrenoDriver::get_min_sampling_period_ns()
{
   return 100000;
}

/*
TODO this sees like it would be largely the same for a5xx as well
(ie. same countable names)..
 */
void
FreedrenoDriver::setup_a6xx_counters()
{
   /* TODO is there a reason to want more than one group? */
   CounterGroup group = {};
   group.name = "counters";
   groups.clear();
   counters.clear();
   countables.clear();
   enabled_counters.clear();
   groups.emplace_back(std::move(group));

   /*
    * Create the countables that we'll be using.
    */

   auto PERF_CP_ALWAYS_COUNT = countable("PERF_CP_ALWAYS_COUNT");
   auto PERF_CP_BUSY_CYCLES  = countable("PERF_CP_BUSY_CYCLES");
   auto PERF_RB_3D_PIXELS    = countable("PERF_RB_3D_PIXELS");
   auto PERF_TP_L1_CACHELINE_MISSES = countable("PERF_TP_L1_CACHELINE_MISSES");
   auto PERF_TP_L1_CACHELINE_REQUESTS = countable("PERF_TP_L1_CACHELINE_REQUESTS");

   auto PERF_TP_OUTPUT_PIXELS  = countable("PERF_TP_OUTPUT_PIXELS");
   auto PERF_TP_OUTPUT_PIXELS_ANISO  = countable("PERF_TP_OUTPUT_PIXELS_ANISO");
   auto PERF_TP_OUTPUT_PIXELS_BILINEAR = countable("PERF_TP_OUTPUT_PIXELS_BILINEAR");
   auto PERF_TP_OUTPUT_PIXELS_POINT = countable("PERF_TP_OUTPUT_PIXELS_POINT");
   auto PERF_TP_OUTPUT_PIXELS_ZERO_LOD = countable("PERF_TP_OUTPUT_PIXELS_ZERO_LOD");

   auto PERF_TSE_INPUT_PRIM  = countable("PERF_TSE_INPUT_PRIM");
   auto PERF_TSE_CLIPPED_PRIM  = countable("PERF_TSE_CLIPPED_PRIM");
   auto PERF_TSE_TRIVAL_REJ_PRIM  = countable("PERF_TSE_TRIVAL_REJ_PRIM");
   auto PERF_TSE_OUTPUT_VISIBLE_PRIM = countable("PERF_TSE_OUTPUT_VISIBLE_PRIM");

   auto PERF_SP_BUSY_CYCLES  = countable("PERF_SP_BUSY_CYCLES");
   auto PERF_SP_ALU_WORKING_CYCLES = countable("PERF_SP_ALU_WORKING_CYCLES");
   auto PERF_SP_EFU_WORKING_CYCLES = countable("PERF_SP_EFU_WORKING_CYCLES");
   auto PERF_SP_VS_STAGE_EFU_INSTRUCTIONS = countable("PERF_SP_VS_STAGE_EFU_INSTRUCTIONS");
   auto PERF_SP_VS_STAGE_FULL_ALU_INSTRUCTIONS = countable("PERF_SP_VS_STAGE_FULL_ALU_INSTRUCTIONS");
   auto PERF_SP_VS_STAGE_TEX_INSTRUCTIONS = countable("PERF_SP_VS_STAGE_TEX_INSTRUCTIONS");
   auto PERF_SP_FS_STAGE_EFU_INSTRUCTIONS = countable("PERF_SP_FS_STAGE_EFU_INSTRUCTIONS");
   auto PERF_SP_FS_STAGE_FULL_ALU_INSTRUCTIONS = countable("PERF_SP_FS_STAGE_FULL_ALU_INSTRUCTIONS");
   auto PERF_SP_FS_STAGE_HALF_ALU_INSTRUCTIONS = countable("PERF_SP_FS_STAGE_HALF_ALU_INSTRUCTIONS");
   auto PERF_SP_STALL_CYCLES_TP = countable("PERF_SP_STALL_CYCLES_TP");
   auto PERF_SP_ANY_EU_WORKING_FS_STAGE = countable("PERF_SP_ANY_EU_WORKING_FS_STAGE");
   auto PERF_SP_ANY_EU_WORKING_VS_STAGE = countable("PERF_SP_ANY_EU_WORKING_VS_STAGE");
   auto PERF_SP_ANY_EU_WORKING_CS_STAGE = countable("PERF_SP_ANY_EU_WORKING_CS_STAGE");

   auto PERF_UCHE_STALL_CYCLES_ARBITER = countable("PERF_UCHE_STALL_CYCLES_ARBITER");
   auto PERF_UCHE_VBIF_READ_BEATS_TP = countable("PERF_UCHE_VBIF_READ_BEATS_TP");
   auto PERF_UCHE_VBIF_READ_BEATS_VFD = countable("PERF_UCHE_VBIF_READ_BEATS_VFD");
   auto PERF_UCHE_VBIF_READ_BEATS_SP = countable("PERF_UCHE_VBIF_READ_BEATS_SP");
   auto PERF_UCHE_READ_REQUESTS_TP = countable("PERF_UCHE_READ_REQUESTS_TP");

   auto PERF_PC_STALL_CYCLES_VFD = countable("PERF_PC_STALL_CYCLES_VFD");
   auto PERF_PC_VS_INVOCATIONS = countable("PERF_PC_VS_INVOCATIONS");
   auto PERF_PC_VERTEX_HITS = countable("PERF_PC_VERTEX_HITS");

   auto PERF_HLSQ_QUADS = countable("PERF_HLSQ_QUADS"); /* Quads (fragments / 4) produced */

   auto PERF_CP_NUM_PREEMPTIONS = countable("PERF_CP_NUM_PREEMPTIONS");
   auto PERF_CP_PREEMPTION_REACTION_DELAY = countable("PERF_CP_PREEMPTION_REACTION_DELAY");

   /* TODO: resolve() tells there is no PERF_CMPDECMP_VBIF_READ_DATA */
   // auto PERF_CMPDECMP_VBIF_READ_DATA = countable("PERF_CMPDECMP_VBIF_READ_DATA");

   /*
    * And then setup the derived counters that we are exporting to
    * pps based on the captured countable values.
    *
    * We try to expose the same counters as blob:
    * https://gpuinspector.dev/docs/gpu-counters/qualcomm
    */

   counter("GPU Frequency", Counter::Units::Hertz, [=]() {
         return PERF_CP_ALWAYS_COUNT / time;
      }
   );

   counter("GPU % Utilization", Counter::Units::Percent, [=]() {
         return percent(PERF_CP_BUSY_CYCLES / time, max_freq);
      }
   );

   counter("TP L1 Cache Misses", Counter::Units::None, [=]() {
         return PERF_TP_L1_CACHELINE_MISSES / time;
      }
   );

   counter("Shader Core Utilization", Counter::Units::Percent, [=]() {
         return percent(PERF_SP_BUSY_CYCLES / time, max_freq * info->num_sp_cores);
      }
   );

   /* TODO: verify */
   counter("(?) % Texture Fetch Stall", Counter::Units::Percent, [=]() {
         return percent(PERF_SP_STALL_CYCLES_TP / time, max_freq * info->num_sp_cores);
      }
   );

   /* TODO: verify */
   counter("(?) % Vertex Fetch Stall", Counter::Units::Percent, [=]() {
         return percent(PERF_PC_STALL_CYCLES_VFD / time, max_freq * info->num_sp_cores);
      }
   );

   counter("L1 Texture Cache Miss Per Pixel", Counter::Units::None, [=]() {
         return safe_div(PERF_TP_L1_CACHELINE_MISSES, PERF_HLSQ_QUADS * 4);
      }
   );

   counter("% Texture L1 Miss", Counter::Units::Percent, [=]() {
         return percent(PERF_TP_L1_CACHELINE_MISSES, PERF_TP_L1_CACHELINE_REQUESTS);
      }
   );

   counter("% Texture L2 Miss", Counter::Units::Percent, [=]() {
         return percent(PERF_UCHE_VBIF_READ_BEATS_TP / 2, PERF_UCHE_READ_REQUESTS_TP);
      }
   );

   /* TODO: verify */
   counter("(?) % Stalled on System Memory", Counter::Units::Percent, [=]() {
         return percent(PERF_UCHE_STALL_CYCLES_ARBITER / time, max_freq * info->num_sp_cores);
      }
   );

   counter("Pre-clipped Polygons / Second", Counter::Units::None, [=]() {
         return PERF_TSE_INPUT_PRIM * (1.f / time);
      }
   );

   counter("% Prims Trivially Rejected", Counter::Units::Percent, [=]() {
         return percent(PERF_TSE_TRIVAL_REJ_PRIM, PERF_TSE_INPUT_PRIM);
      }
   );

   counter("% Prims Clipped", Counter::Units::Percent, [=]() {
         return percent(PERF_TSE_CLIPPED_PRIM, PERF_TSE_INPUT_PRIM);
      }
   );

   counter("Average Vertices / Polygon", Counter::Units::None, [=]() {
         return PERF_PC_VS_INVOCATIONS / PERF_TSE_INPUT_PRIM;
      }
   );

   counter("Reused Vertices / Second", Counter::Units::None, [=]() {
         return PERF_PC_VERTEX_HITS * (1.f / time);
      }
   );

   counter("Average Polygon Area", Counter::Units::None, [=]() {
         return safe_div(PERF_HLSQ_QUADS * 4, PERF_TSE_OUTPUT_VISIBLE_PRIM);
      }
   );

   /* TODO: find formula */
   // counter("% Shaders Busy", Counter::Units::Percent, [=]() {
   //       return 100.0 * 0;
   //    }
   // );

   counter("Vertices Shaded / Second", Counter::Units::None, [=]() {
         return PERF_PC_VS_INVOCATIONS * (1.f / time);
      }
   );

   counter("Fragments Shaded / Second", Counter::Units::None, [=]() {
         return PERF_HLSQ_QUADS * 4 * (1.f / time);
      }
   );

   counter("Vertex Instructions / Second", Counter::Units::None, [=]() {
         return (PERF_SP_VS_STAGE_FULL_ALU_INSTRUCTIONS +
                 PERF_SP_VS_STAGE_EFU_INSTRUCTIONS) * (1.f / time);
      }
   );

   counter("Fragment Instructions / Second", Counter::Units::None, [=]() {
         return (PERF_SP_FS_STAGE_FULL_ALU_INSTRUCTIONS +
                 PERF_SP_FS_STAGE_HALF_ALU_INSTRUCTIONS / 2 +
                 PERF_SP_FS_STAGE_EFU_INSTRUCTIONS) * (1.f / time);
      }
   );

   counter("Fragment ALU Instructions / Sec (Full)", Counter::Units::None, [=]() {
         return PERF_SP_FS_STAGE_FULL_ALU_INSTRUCTIONS * (1.f / time);
      }
   );

   counter("Fragment ALU Instructions / Sec (Half)", Counter::Units::None, [=]() {
         return PERF_SP_FS_STAGE_HALF_ALU_INSTRUCTIONS * (1.f / time);
      }
   );

   counter("Fragment EFU Instructions / Second", Counter::Units::None, [=]() {
         return PERF_SP_FS_STAGE_EFU_INSTRUCTIONS * (1.f / time);
      }
   );

   counter("Textures / Vertex", Counter::Units::None, [=]() {
         return safe_div(PERF_SP_VS_STAGE_TEX_INSTRUCTIONS, PERF_PC_VS_INVOCATIONS);
      }
   );

   counter("Textures / Fragment", Counter::Units::None, [=]() {
         return safe_div(PERF_TP_OUTPUT_PIXELS, PERF_HLSQ_QUADS * 4);
      }
   );

   counter("ALU / Vertex", Counter::Units::None, [=]() {
         return safe_div(PERF_SP_VS_STAGE_FULL_ALU_INSTRUCTIONS, PERF_PC_VS_INVOCATIONS);
      }
   );

   counter("EFU / Vertex", Counter::Units::None, [=]() {
         return safe_div(PERF_SP_VS_STAGE_EFU_INSTRUCTIONS, PERF_PC_VS_INVOCATIONS);
      }
   );

   counter("ALU / Fragment", Counter::Units::None, [=]() {
         return safe_div(PERF_SP_FS_STAGE_FULL_ALU_INSTRUCTIONS +
                         PERF_SP_FS_STAGE_HALF_ALU_INSTRUCTIONS / 2, PERF_HLSQ_QUADS);
      }
   );

   counter("EFU / Fragment", Counter::Units::None, [=]() {
         return safe_div(PERF_SP_FS_STAGE_EFU_INSTRUCTIONS, PERF_HLSQ_QUADS);
      }
   );

   counter("% Time Shading Vertices", Counter::Units::Percent, [=]() {
         return percent(PERF_SP_ANY_EU_WORKING_VS_STAGE,
                        (PERF_SP_ANY_EU_WORKING_VS_STAGE +
                         PERF_SP_ANY_EU_WORKING_FS_STAGE +
                         PERF_SP_ANY_EU_WORKING_CS_STAGE));
      }
   );

   counter("% Time Shading Fragments", Counter::Units::Percent, [=]() {
         return percent(PERF_SP_ANY_EU_WORKING_FS_STAGE,
                        (PERF_SP_ANY_EU_WORKING_VS_STAGE +
                         PERF_SP_ANY_EU_WORKING_FS_STAGE +
                         PERF_SP_ANY_EU_WORKING_CS_STAGE));
      }
   );

   counter("% Time Compute", Counter::Units::Percent, [=]() {
         return percent(PERF_SP_ANY_EU_WORKING_CS_STAGE,
                        (PERF_SP_ANY_EU_WORKING_VS_STAGE +
                         PERF_SP_ANY_EU_WORKING_FS_STAGE +
                         PERF_SP_ANY_EU_WORKING_CS_STAGE));
      }
   );

   counter("% Shader ALU Capacity Utilized", Counter::Units::Percent, [=]() {
         return percent((PERF_SP_VS_STAGE_FULL_ALU_INSTRUCTIONS +
                         PERF_SP_FS_STAGE_FULL_ALU_INSTRUCTIONS +
                         PERF_SP_FS_STAGE_HALF_ALU_INSTRUCTIONS / 2) / 64,
                        PERF_SP_BUSY_CYCLES);
      }
   );

   counter("% Time ALUs Working", Counter::Units::Percent, [=]() {
         return percent(PERF_SP_ALU_WORKING_CYCLES / 2, PERF_SP_BUSY_CYCLES);
      }
   );

   counter("% Time EFUs Working", Counter::Units::Percent, [=]() {
         return percent(PERF_SP_EFU_WORKING_CYCLES / 2, PERF_SP_BUSY_CYCLES);
      }
   );

   counter("% Anisotropic Filtered", Counter::Units::Percent, [=]() {
         return percent(PERF_TP_OUTPUT_PIXELS_ANISO, PERF_TP_OUTPUT_PIXELS);
      }
   );

   counter("% Linear Filtered", Counter::Units::Percent, [=]() {
         return percent(PERF_TP_OUTPUT_PIXELS_BILINEAR, PERF_TP_OUTPUT_PIXELS);
      }
   );

   counter("% Nearest Filtered", Counter::Units::Percent, [=]() {
         return percent(PERF_TP_OUTPUT_PIXELS_POINT, PERF_TP_OUTPUT_PIXELS);
      }
   );

   counter("% Non-Base Level Textures", Counter::Units::Percent, [=]() {
         return percent(PERF_TP_OUTPUT_PIXELS_ZERO_LOD, PERF_TP_OUTPUT_PIXELS);
      }
   );

   /* Reads from KGSL_PERFCOUNTER_GROUP_VBIF countable=63 */
   // counter("Read Total (Bytes/sec)", Counter::Units::Byte, [=]() {
   //       return  * (1.f / time);
   //    }
   // );

   /* Reads from KGSL_PERFCOUNTER_GROUP_VBIF countable=84 */
   // counter("Write Total (Bytes/sec)", Counter::Units::Byte, [=]() {
   //       return  * (1.f / time);
   //    }
   // );

   /* Cannot get PERF_CMPDECMP_VBIF_READ_DATA countable */
   // counter("Texture Memory Read BW (Bytes/Second)", Counter::Units::Byte, [=]() {
   //       return (PERF_CMPDECMP_VBIF_READ_DATA + PERF_UCHE_VBIF_READ_BEATS_TP) * (1.f / time);
   //    }
   // );

   /* TODO: verify */
   counter("(?) Vertex Memory Read (Bytes/Second)", Counter::Units::Byte, [=]() {
         return PERF_UCHE_VBIF_READ_BEATS_VFD * 32 * (1.f / time);
      }
   );

   /* TODO: verify */
   counter("SP Memory Read (Bytes/Second)", Counter::Units::Byte, [=]() {
         return PERF_UCHE_VBIF_READ_BEATS_SP * 32 * (1.f / time);
      }
   );

   counter("Avg Bytes / Fragment", Counter::Units::Byte, [=]() {
         return safe_div(PERF_UCHE_VBIF_READ_BEATS_TP * 32, PERF_HLSQ_QUADS * 4);
      }
   );

   counter("Avg Bytes / Vertex", Counter::Units::Byte, [=]() {
         return safe_div(PERF_UCHE_VBIF_READ_BEATS_VFD * 32, PERF_PC_VS_INVOCATIONS);
      }
   );

   counter("Preemptions / second", Counter::Units::None, [=]() {
         return PERF_CP_NUM_PREEMPTIONS * (1.f / time);
      }
   );

   counter("Avg Preemption Delay", Counter::Units::None, [=]() {
         return PERF_CP_PREEMPTION_REACTION_DELAY * (1.f / time);
      }
   );
}

/**
 * Generate an submit the cmdstream to configure the counter/countable
 * muxing
 */
void
FreedrenoDriver::configure_counters(bool reset, bool wait)
{
   struct fd_submit *submit = fd_submit_new(pipe);
   enum fd_ringbuffer_flags flags =
      (enum fd_ringbuffer_flags)(FD_RINGBUFFER_PRIMARY | FD_RINGBUFFER_GROWABLE);
   struct fd_ringbuffer *ring = fd_submit_new_ringbuffer(submit, 0x1000, flags);

   for (const auto &countable : countables)
      countable.configure(ring, reset);

   struct fd_fence *fence = fd_submit_flush(submit, -1, false);

   fd_fence_flush(fence);
   fd_fence_del(fence);

   fd_ringbuffer_del(ring);
   fd_submit_del(submit);

   if (wait)
      fd_pipe_wait(pipe, fence);
}

/**
 * Read the current counter values and record the time.
 */
void
FreedrenoDriver::collect_countables()
{
   last_dump_ts = perfetto::base::GetBootTimeNs().count();

   for (const auto &countable : countables)
      countable.collect();
}

bool
FreedrenoDriver::init_perfcnt()
{
   uint64_t val;

   dev = fd_device_new(drm_device.fd);
   pipe = fd_pipe_new2(dev, FD_PIPE_3D, 0);
   dev_id = fd_pipe_dev_id(pipe);

   if (fd_pipe_get_param(pipe, FD_MAX_FREQ, &val)) {
      PERFETTO_FATAL("Could not get MAX_FREQ");
      return false;
   }
   max_freq = val;

   if (fd_pipe_get_param(pipe, FD_SUSPEND_COUNT, &val)) {
      PERFETTO_ILOG("Could not get SUSPEND_COUNT");
   } else {
      suspend_count = val;
      has_suspend_count = true;
   }

   fd_pipe_set_param(pipe, FD_SYSPROF, 1);

   perfcntrs = fd_perfcntrs(fd_pipe_dev_id(pipe), &num_perfcntrs);
   if (num_perfcntrs == 0) {
      PERFETTO_FATAL("No hw counters available");
      return false;
   }

   assigned_counters.resize(num_perfcntrs);
   assigned_counters.assign(assigned_counters.size(), 0);

   switch (fd_dev_gen(dev_id)) {
   case 6:
      setup_a6xx_counters();
      break;
   default:
      PERFETTO_FATAL("Unsupported GPU: a%03u", fd_dev_gpu_id(dev_id));
      return false;
   }

   state.resize(next_countable_id);

   for (const auto &countable : countables)
      countable.resolve();

   info = fd_dev_info_raw(dev_id);

   io = fd_dt_find_io();
   if (!io) {
      PERFETTO_FATAL("Could not map GPU I/O space");
      return false;
   }

   configure_counters(true, true);
   collect_countables();

   return true;
}

void
FreedrenoDriver::enable_counter(const uint32_t counter_id)
{
   enabled_counters.push_back(counters[counter_id]);
}

void
FreedrenoDriver::enable_all_counters()
{
   enabled_counters.reserve(counters.size());
   for (auto &counter : counters) {
      enabled_counters.push_back(counter);
   }
}

void
FreedrenoDriver::enable_perfcnt(const uint64_t /* sampling_period_ns */)
{
}

bool
FreedrenoDriver::dump_perfcnt()
{
   if (has_suspend_count) {
      uint64_t val;

      fd_pipe_get_param(pipe, FD_SUSPEND_COUNT, &val);

      if (suspend_count != val) {
         PERFETTO_ILOG("Device had suspended!");

         suspend_count = val;

         configure_counters(true, true);
         collect_countables();

         /* We aren't going to have anything sensible by comparing
          * current values to values from prior to the suspend, so
          * just skip this sampling period.
          */
         return false;
      }
   }

   auto last_ts = last_dump_ts;

   /* Capture the timestamp from the *start* of the sampling period: */
   last_capture_ts = last_dump_ts;

   collect_countables();

   auto elapsed_time_ns = last_dump_ts - last_ts;

   time = (float)elapsed_time_ns / 1000000000.0;

   /* On older kernels that dont' support querying the suspend-
    * count, just send configuration cmdstream regularly to keep
    * the GPU alive and correctly configured for the countables
    * we want
    */
   if (!has_suspend_count) {
      configure_counters(false, false);
   }

   return true;
}

uint64_t FreedrenoDriver::next()
{
   auto ret = last_capture_ts;
   last_capture_ts = 0;
   return ret;
}

void FreedrenoDriver::disable_perfcnt()
{
   /* There isn't really any disable, only reconfiguring which countables
    * get muxed to which counters
    */
}

/*
 * Countable
 */

FreedrenoDriver::Countable
FreedrenoDriver::countable(std::string name)
{
   auto countable = Countable(this, name);
   countables.emplace_back(countable);
   return countable;
}

FreedrenoDriver::Countable::Countable(FreedrenoDriver *d, std::string name)
   : id {d->next_countable_id++}, d {d}, name {name}
{
}

/* Emit register writes on ring to configure counter/countable muxing: */
void
FreedrenoDriver::Countable::configure(struct fd_ringbuffer *ring, bool reset) const
{
   const struct fd_perfcntr_countable *countable = d->state[id].countable;
   const struct fd_perfcntr_counter   *counter   = d->state[id].counter;

   OUT_PKT7(ring, CP_WAIT_FOR_IDLE, 0);

   if (counter->enable && reset) {
      OUT_PKT4(ring, counter->enable, 1);
      OUT_RING(ring, 0);
   }

   if (counter->clear && reset) {
      OUT_PKT4(ring, counter->clear, 1);
      OUT_RING(ring, 1);

      OUT_PKT4(ring, counter->clear, 1);
      OUT_RING(ring, 0);
   }

   OUT_PKT4(ring, counter->select_reg, 1);
   OUT_RING(ring, countable->selector);

   if (counter->enable && reset) {
      OUT_PKT4(ring, counter->enable, 1);
      OUT_RING(ring, 1);
   }
}

/* Collect current counter value and calculate delta since last sample: */
void
FreedrenoDriver::Countable::collect() const
{
   const struct fd_perfcntr_counter *counter = d->state[id].counter;

   d->state[id].last_value = d->state[id].value;

   /* this is true on a5xx and later */
   assert(counter->counter_reg_lo + 1 == counter->counter_reg_hi);
   uint64_t *reg = (uint64_t *)((uint32_t *)d->io + counter->counter_reg_lo);

   d->state[id].value = *reg;
}

/* Resolve the countable and assign next counter from it's group: */
void
FreedrenoDriver::Countable::resolve() const
{
   for (unsigned i = 0; i < d->num_perfcntrs; i++) {
      const struct fd_perfcntr_group *g = &d->perfcntrs[i];
      for (unsigned j = 0; j < g->num_countables; j++) {
         const struct fd_perfcntr_countable *c = &g->countables[j];
         if (name == c->name) {
            d->state[id].countable = c;

            /* Assign a counter from the same group: */
            assert(d->assigned_counters[i] < g->num_counters);
            d->state[id].counter = &g->counters[d->assigned_counters[i]++];

            std::cout << "Countable: " << name << ", group=" << g->name <<
                  ", counter=" << d->assigned_counters[i] - 1 << "\n";

            return;
         }
      }
   }
   unreachable("no such countable!");
}

uint64_t
FreedrenoDriver::Countable::get_value() const
{
   return d->state[id].value - d->state[id].last_value;
}

/*
 * DerivedCounter
 */

FreedrenoDriver::DerivedCounter::DerivedCounter(FreedrenoDriver *d, std::string name,
                                                Counter::Units units,
                                                std::function<int64_t()> derive)
   : Counter(d->next_counter_id++, name, 0)
{
   std::cout << "DerivedCounter: " << name << ", id=" << id << "\n";
   this->units = units;
   set_getter([=](const Counter &c, const Driver &d) {
         return derive();
      }
   );
}

FreedrenoDriver::DerivedCounter
FreedrenoDriver::counter(std::string name, Counter::Units units,
                         std::function<int64_t()> derive)
{
   auto counter = DerivedCounter(this, name, units, derive);
   counters.emplace_back(counter);
   return counter;
}

uint32_t
FreedrenoDriver::gpu_clock_id() const
{
   return perfetto::protos::pbzero::BUILTIN_CLOCK_BOOTTIME;
}

uint64_t
FreedrenoDriver::gpu_timestamp() const
{
   return perfetto::base::GetBootTimeNs().count();
}

bool
FreedrenoDriver::cpu_gpu_timestamp(uint64_t &, uint64_t &) const
{
   /* Not supported */
   return false;
}

} // namespace pps
