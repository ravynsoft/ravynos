/*
 * Copyright © 2019-2021 Collabora, Ltd.
 * Author: Antonio Caggiano <antonio.caggiano@collabora.com>
 * Author: Rohan Garg <rohan.garg@collabora.com>
 * Author: Robert Beckett <bob.beckett@collabora.com>
 * Author: Corentin Noël <corentin.noel@collabora.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "pps_datasource.h"
#include "pps_driver.h"

#include <condition_variable>
#include <thread>
#include <variant>
#include <inttypes.h>

// Minimum supported sampling period in nanoseconds
#define MIN_SAMPLING_PERIOD_NS 50000

#define CORRELATION_TIMESTAMP_PERIOD (1000000000ull)

namespace pps
{
static std::string driver_name;

/// Synchronize access to started_cv and started
static std::mutex started_m;
static std::condition_variable started_cv;
static bool started = false;

float ms(const std::chrono::nanoseconds &t)
{
   return t.count() / 1000000.0f;
}

void GpuDataSource::OnSetup(const SetupArgs &args)
{
   // Create drivers for all supported devices
   auto drm_devices = DrmDevice::create_all();
   for (auto &drm_device : drm_devices) {
      if (drm_device.name != driver_name)
         continue;

      if (auto driver = Driver::get_driver(std::move(drm_device))) {
         if (!driver->init_perfcnt()) {
            // Skip failing driver
            PPS_LOG_ERROR("Failed to initialize %s driver", driver->drm_device.name.c_str());
            continue;
         }

         this->driver = driver;
      }
   }
   if (driver == nullptr) {
      PPS_LOG_FATAL("No DRM devices supported");
   }

   // Parse perfetto config
   const std::string &config_raw = args.config->gpu_counter_config_raw();
   perfetto::protos::pbzero::GpuCounterConfig::Decoder config(config_raw);

   if (config.has_counter_ids()) {
      // Get enabled counters
      PPS_LOG_IMPORTANT("Selecting counters");
      for (auto it = config.counter_ids(); it; ++it) {
         uint32_t counter_id = it->as_uint32();
         driver->enable_counter(counter_id);
      }
   } else {
      // Enable all counters
      driver->enable_all_counters();
   }

   // Get sampling period
   auto min_sampling_period = std::chrono::nanoseconds(MIN_SAMPLING_PERIOD_NS);

   auto dev_supported = std::chrono::nanoseconds(driver->get_min_sampling_period_ns());
   if (dev_supported > min_sampling_period) {
      min_sampling_period = dev_supported;
   }

   time_to_sleep = std::max(time_to_sleep, min_sampling_period);

   if (config.has_counter_period_ns()) {
      auto requested_sampling_period = std::chrono::nanoseconds(config.counter_period_ns());
      if (requested_sampling_period < min_sampling_period) {
         PPS_LOG_ERROR("Sampling period should be greater than %" PRIu64 " ns (%.2f ms)",
            uint64_t(min_sampling_period.count()),
            ms(min_sampling_period));
      } else {
         time_to_sleep = requested_sampling_period;
      }
   }
   PPS_LOG("Sampling period set to %" PRIu64 " ns", uint64_t(time_to_sleep.count()));
}

void GpuDataSource::OnStart(const StartArgs &args)
{
   driver->enable_perfcnt(time_to_sleep.count());

   state = State::Start;
   got_first_counters = false;

   {
      std::lock_guard<std::mutex> lock(started_m);
      started = true;
   }
   started_cv.notify_all();
}

void close_callback(GpuDataSource::TraceContext ctx)
{
   auto packet = ctx.NewTracePacket();
   packet->set_timestamp_clock_id(perfetto::protos::pbzero::BUILTIN_CLOCK_BOOTTIME);
   packet->set_timestamp(perfetto::base::GetBootTimeNs().count());
   packet->Finalize();
   ctx.Flush();
   PPS_LOG("Context flushed");
}

void GpuDataSource::OnStop(const StopArgs &args)
{
   state = State::Stop;
   auto stop_closure = args.HandleStopAsynchronously();
   Trace(close_callback);
   stop_closure();

   driver->disable_perfcnt();
   driver = nullptr;

   std::lock_guard<std::mutex> lock(started_m);
   started = false;
}

void GpuDataSource::wait_started()
{
   std::unique_lock<std::mutex> lock(started_m);
   if (!started) {
      PPS_LOG("Waiting for start");
      started_cv.wait(lock, [] { return started; });
   }
}

void GpuDataSource::register_data_source(const std::string &_driver_name)
{
   driver_name = _driver_name;
   static perfetto::DataSourceDescriptor dsd;
   dsd.set_name("gpu.counters." + driver_name);
   Register(dsd);
}

void add_group(perfetto::protos::pbzero::GpuCounterDescriptor *desc,
   const CounterGroup &group,
   const std::string &prefix,
   int32_t gpu_num)
{
   if (!group.counters.empty()) {
      // Define a block for each group containing counters
      auto block_desc = desc->add_blocks();
      block_desc->set_name(prefix + "." + group.name);
      block_desc->set_block_id(group.id);

      // Associate counters to blocks
      for (auto id : group.counters) {
         block_desc->add_counter_ids(id);
      }
   }

   for (auto const &sub : group.subgroups) {
      // Perfetto doesnt currently support nested groups.
      // Flatten group hierarchy, using dot separator
      add_group(desc, sub, prefix + "." + group.name, gpu_num);
   }
}

void add_descriptors(perfetto::protos::pbzero::GpuCounterEvent *event,
   std::vector<CounterGroup> const &groups,
   std::vector<Counter> const &counters,
   Driver &driver)
{
   // Start a counter descriptor
   auto desc = event->set_counter_descriptor();

   // Add the groups
   for (auto const &group : groups) {
      add_group(desc, group, driver.drm_device.name, driver.drm_device.gpu_num);
   }

   // Add the counters
   for (auto const &counter : counters) {
      auto spec = desc->add_specs();
      spec->set_counter_id(counter.id);
      spec->set_name(counter.name);

      auto units = perfetto::protos::pbzero::GpuCounterDescriptor::NONE;
      switch (counter.units) {
      case Counter::Units::Percent:
         units = perfetto::protos::pbzero::GpuCounterDescriptor::PERCENT;
         break;
      case Counter::Units::Byte:
         units = perfetto::protos::pbzero::GpuCounterDescriptor::BYTE;
         break;
      case Counter::Units::Hertz:
         units = perfetto::protos::pbzero::GpuCounterDescriptor::HERTZ;
         break;
      case Counter::Units::None:
         units = perfetto::protos::pbzero::GpuCounterDescriptor::NONE;
         break;
      default:
         assert(false && "Missing counter units type!");
         break;
      }
      spec->add_numerator_units(units);
   }
}

void add_samples(perfetto::protos::pbzero::GpuCounterEvent &event, const Driver &driver)
{
   if (driver.enabled_counters.size() == 0) {
      PPS_LOG_FATAL("There are no counters enabled");
   }

   for (const auto &counter : driver.enabled_counters) {
      auto counter_event = event.add_counters();

      counter_event->set_counter_id(counter.id);

      auto value = counter.get_value(driver);
      if (auto d_value = std::get_if<double>(&value)) {
         counter_event->set_double_value(*d_value);
      } else if (auto i_value = std::get_if<int64_t>(&value)) {
         counter_event->set_int_value(*i_value);
      } else {
         PPS_LOG_ERROR("Failed to get value for counter %s", counter.name.c_str());
      }
   }
}

void add_timestamp(perfetto::protos::pbzero::ClockSnapshot *event, const Driver *driver)
{
   uint32_t gpu_clock_id = driver->gpu_clock_id();
   if (perfetto::protos::pbzero::BUILTIN_CLOCK_BOOTTIME == gpu_clock_id)
      return;

   // Send a correlation event between GPU & CPU timestamps
   uint64_t cpu_ts, gpu_ts;

   // Try to use the optimized driver correlation if available, otherwise do a
   // separate CPU & GPU sample
   if (!driver->cpu_gpu_timestamp(cpu_ts, gpu_ts)) {
      cpu_ts = perfetto::base::GetBootTimeNs().count();
      gpu_ts = driver->gpu_timestamp();
   }

   {
      auto clock = event->add_clocks();

      clock->set_clock_id(perfetto::protos::pbzero::BUILTIN_CLOCK_BOOTTIME);
      clock->set_timestamp(cpu_ts);
   }

   {
      auto clock = event->add_clocks();

      clock->set_clock_id(gpu_clock_id);
      clock->set_timestamp(gpu_ts);
   }
}

void GpuDataSource::trace(TraceContext &ctx)
{
   using namespace perfetto::protos::pbzero;

   if (auto state = ctx.GetIncrementalState(); state->was_cleared) {
      descriptor_timestamp = perfetto::base::GetBootTimeNs().count();

      {
         // Mark any incremental state before this point invalid
         auto packet = ctx.NewTracePacket();
         packet->set_timestamp_clock_id(perfetto::protos::pbzero::BUILTIN_CLOCK_BOOTTIME);
         packet->set_timestamp(descriptor_timestamp);
         packet->set_sequence_flags(TracePacket::SEQ_INCREMENTAL_STATE_CLEARED);
      }

      descriptor_timestamp = perfetto::base::GetBootTimeNs().count();
      {
         // Counter descriptions
         auto packet = ctx.NewTracePacket();
         packet->set_timestamp_clock_id(perfetto::protos::pbzero::BUILTIN_CLOCK_BOOTTIME);
         packet->set_timestamp(descriptor_timestamp);
         auto event = packet->set_gpu_counter_event();
         event->set_gpu_id(driver->drm_device.gpu_num);

         auto &groups = driver->groups;
         auto &counters = driver->enabled_counters;
         add_descriptors(event, groups, counters, *driver);
      }

      {
         // Initial timestamp correlation event
         auto packet = ctx.NewTracePacket();
         packet->set_timestamp_clock_id(perfetto::protos::pbzero::BUILTIN_CLOCK_BOOTTIME);
         packet->set_timestamp(descriptor_timestamp);
         last_correlation_timestamp = perfetto::base::GetBootTimeNs().count();
         auto event = packet->set_clock_snapshot();
         add_timestamp(event, driver);
      }

      // Capture GPU timestamp of the first packet. Anything prior to this can
      // be discarded.
      descriptor_gpu_timestamp = driver->gpu_timestamp();
      state->was_cleared = false;
   }

   if (driver->dump_perfcnt()) {
      while (auto gpu_timestamp = driver->next()) {
         if (gpu_timestamp <= descriptor_gpu_timestamp) {
            // Do not send counter values before counter descriptors
            PPS_LOG_ERROR("Skipping counter values coming before descriptors");
            continue;
         }

         if (!got_first_counters) {
            PPS_LOG("Got first counters at gpu_ts=0x%016" PRIx64, gpu_timestamp);
            got_first_counters = true;
         }

         auto packet = ctx.NewTracePacket();
         packet->set_timestamp_clock_id(driver->gpu_clock_id());
         packet->set_timestamp(gpu_timestamp);

         auto event = packet->set_gpu_counter_event();
         event->set_gpu_id(driver->drm_device.gpu_num);

         add_samples(*event, *driver);
      }
   }

   uint64_t cpu_ts = perfetto::base::GetBootTimeNs().count();
   if ((cpu_ts - last_correlation_timestamp) > CORRELATION_TIMESTAMP_PERIOD) {
      auto packet = ctx.NewTracePacket();
      packet->set_timestamp_clock_id(perfetto::protos::pbzero::BUILTIN_CLOCK_BOOTTIME);
      packet->set_timestamp(cpu_ts);
      auto event = packet->set_clock_snapshot();
      add_timestamp(event, driver);
      last_correlation_timestamp = cpu_ts;
   }
}

void GpuDataSource::trace_callback(TraceContext ctx)
{
   using namespace std::chrono;

   nanoseconds sleep_time = nanoseconds(0);

   if (auto data_source = ctx.GetDataSourceLocked()) {
      if (data_source->time_to_sleep > data_source->time_to_trace) {
         sleep_time = data_source->time_to_sleep - data_source->time_to_trace;
      }
   }

   // Wait sampling period before tracing
   std::this_thread::sleep_for(sleep_time);

   auto time_zero = perfetto::base::GetBootTimeNs();
   if (auto data_source = ctx.GetDataSourceLocked()) {
      // Check data source is still running
      if (data_source->state == pps::State::Start) {
         data_source->trace(ctx);
         data_source->time_to_trace = perfetto::base::GetBootTimeNs() - time_zero;
      }
   } else {
      PPS_LOG("Tracing finished");
   }
}

} // namespace pps
