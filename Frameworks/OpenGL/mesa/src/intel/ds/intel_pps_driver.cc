/*
 * Copyright © 2020-2021 Collabora, Ltd.
 * Author: Antonio Caggiano <antonio.caggiano@collabora.com>
 * Author: Corentin Noël <corentin.noel@collabora.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "intel_pps_driver.h"

#include <dirent.h>
#include <fcntl.h>
#include <math.h>
#include <poll.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "drm-uapi/i915_drm.h"

#include "common/intel_gem.h"
#include "dev/intel_device_info.h"
#include "perf/intel_perf.h"
#include "perf/intel_perf_query.h"

#include <pps/pps.h>
#include <pps/pps_algorithm.h>

#include "intel_pps_perf.h"
#include "intel_pps_priv.h"

namespace pps
{

// The HW sampling period is programmed using period_exponent following this
// formula:
//    sample_period = timestamp_period * 2^(period_exponent + 1)
// So our minimum sampling period is twice the timestamp period

uint64_t IntelDriver::get_min_sampling_period_ns()
{
   return (2.f * perf->devinfo.timestamp_frequency) / 1000000000ull;
}

IntelDriver::IntelDriver()
{
}

IntelDriver::~IntelDriver()
{
}

void IntelDriver::enable_counter(uint32_t counter_id)
{
   auto &counter = counters[counter_id];

   enabled_counters.emplace_back(counter);
}

void IntelDriver::enable_all_counters()
{
   // We should only have one group
   assert(groups.size() == 1);
   for (uint32_t counter_id : groups[0].counters) {
      auto &counter = counters[counter_id];
      enabled_counters.emplace_back(counter);
   }
}

bool IntelDriver::init_perfcnt()
{
   /* Note: clock_id's below 128 are reserved.. for custom clock sources,
    * using the hash of a namespaced string is the recommended approach.
    * See: https://perfetto.dev/docs/concepts/clock-sync
    */
   this->clock_id = intel_pps_clock_id(drm_device.gpu_num);

   assert(!perf && "Intel perf should not be initialized at this point");

   perf = std::make_unique<IntelPerf>(drm_device.fd);

   const char *metric_set_name = getenv("INTEL_PERFETTO_METRIC_SET");

   struct intel_perf_query_info *default_query = nullptr;
   selected_query = nullptr;
   for (auto &query : perf->get_queries()) {
      if (!strcmp(query->symbol_name, "RenderBasic"))
         default_query = query;
      if (metric_set_name && !strcmp(query->symbol_name, metric_set_name))
         selected_query = query;
   }

   assert(default_query);

   if (!selected_query) {
      if (metric_set_name) {
         PPS_LOG_ERROR("Available metric sets:");
         for (auto &query : perf->get_queries())
            PPS_LOG_ERROR("   %s", query->symbol_name);
         PPS_LOG_FATAL("Metric set '%s' not available.", metric_set_name);
      }
      selected_query = default_query;
   }

   PPS_LOG("Using metric set '%s': %s",
           selected_query->symbol_name, selected_query->name);

   // Create group
   CounterGroup group = {};
   group.id = groups.size();
   group.name = selected_query->symbol_name;

   for (int i = 0; i < selected_query->n_counters; ++i) {
      intel_perf_query_counter &counter = selected_query->counters[i];

      // Create counter
      Counter counter_desc = {};
      counter_desc.id = counters.size();
      counter_desc.name = counter.symbol_name;
      counter_desc.group = group.id;
      counter_desc.getter = [counter, this](
         const Counter &c, const Driver &dri) -> Counter::Value {
         switch (counter.data_type) {
         case INTEL_PERF_COUNTER_DATA_TYPE_UINT64:
         case INTEL_PERF_COUNTER_DATA_TYPE_UINT32:
         case INTEL_PERF_COUNTER_DATA_TYPE_BOOL32:
            return (int64_t)counter.oa_counter_read_uint64(perf->cfg,
                                                           selected_query,
                                                           &perf->result);
            break;
         case INTEL_PERF_COUNTER_DATA_TYPE_DOUBLE:
         case INTEL_PERF_COUNTER_DATA_TYPE_FLOAT:
            return counter.oa_counter_read_float(perf->cfg,
                                                 selected_query,
                                                 &perf->result);
            break;
         }

         return {};
      };

      // Add counter id to the group
      group.counters.emplace_back(counter_desc.id);

      // Store counter
      counters.emplace_back(std::move(counter_desc));
   }

   // Store group
   groups.emplace_back(std::move(group));

   assert(counters.size() && "Failed to query counters");

   // Clear accumulations
   intel_perf_query_result_clear(&perf->result);

   return true;
}

void IntelDriver::enable_perfcnt(uint64_t sampling_period_ns)
{
   this->sampling_period_ns = sampling_period_ns;

   intel_gem_read_render_timestamp(drm_device.fd, perf->devinfo.kmd_type,
                                   &gpu_timestamp_udw);
   gpu_timestamp_udw &= ~perf->cfg->oa_timestamp_mask;
   if (!perf->open(sampling_period_ns, selected_query)) {
      PPS_LOG_FATAL("Failed to open intel perf");
   }
}

void IntelDriver::disable_perfcnt()
{
   gpu_timestamp_udw = 0;
   perf = nullptr;
   groups.clear();
   counters.clear();
   enabled_counters.clear();
}

/// @brief Some perf record durations can be really short
/// @return True if the duration is at least close to the sampling period
static bool close_enough(uint64_t duration, uint64_t sampling_period)
{
   return duration > sampling_period - 100000;
}

/// @brief Transforms the raw data received in from the driver into records
std::vector<PerfRecord> IntelDriver::parse_perf_records(const std::vector<uint8_t> &data,
   const size_t byte_count)
{
   std::vector<PerfRecord> records;
   records.reserve(128);

   PerfRecord record;
   record.data.reserve(512);

   const uint8_t *iter = data.data();
   const uint8_t *end = iter + byte_count;

   uint64_t prev_gpu_timestamp = last_gpu_timestamp;

   while (iter < end) {
      // Iterate a record at a time
      auto header = reinterpret_cast<const drm_i915_perf_record_header *>(iter);

      if (header->type == DRM_I915_PERF_RECORD_SAMPLE) {
         // Report is next to the header
         const uint32_t *report = reinterpret_cast<const uint32_t *>(header + 1);
         uint64_t gpu_timestamp_ldw =
            intel_perf_report_timestamp(selected_query, report);

         /* Our HW only provides us with the lower 32 bits of the 36bits
          * timestamp counter value. If we haven't captured the top bits yet,
          * do it now. If we see a roll over the lower 32bits capture it
          * again.
          */
         if (gpu_timestamp_udw == 0 ||
             (gpu_timestamp_udw | gpu_timestamp_ldw) < last_gpu_timestamp) {
            intel_gem_read_render_timestamp(drm_device.fd,
                                            perf->devinfo.kmd_type,
                                            &gpu_timestamp_udw);
            gpu_timestamp_udw &= ~perf->cfg->oa_timestamp_mask;
         }

         uint64_t gpu_timestamp = gpu_timestamp_udw | gpu_timestamp_ldw;

         auto duration = intel_device_info_timebase_scale(&perf->devinfo,
                                                          gpu_timestamp - prev_gpu_timestamp);

         // Skip perf-records that are too short by checking
         // the distance between last report and this one
         if (close_enough(duration, sampling_period_ns)) {
            prev_gpu_timestamp = gpu_timestamp;

            // Add the new record to the list
            record.timestamp = gpu_timestamp;
            record.data.resize(header->size); // Possibly 264?
            memcpy(record.data.data(), iter, header->size);
            records.emplace_back(record);
         }
      }

      // Go to the next record
      iter += header->size;
   }

   return records;
}

/// @brief Read all the available data from the metric set currently in use
void IntelDriver::read_data_from_metric_set()
{
   assert(metric_buffer.size() >= 1024 && "Metric buffer should have space for reading");

   ssize_t bytes_read = 0;
   while ((bytes_read = perf->read_oa_stream(metric_buffer.data() + total_bytes_read,
              metric_buffer.size() - total_bytes_read)) > 0 ||
      errno == EINTR) {
      total_bytes_read += std::max(ssize_t(0), bytes_read);

      // Increase size of the buffer for the next read
      if (metric_buffer.size() / 2 < total_bytes_read) {
         metric_buffer.resize(metric_buffer.size() * 2);
      }
   }

   assert(total_bytes_read < metric_buffer.size() && "Buffer not big enough");
}

bool IntelDriver::dump_perfcnt()
{
   if (!perf->oa_stream_ready()) {
      return false;
   }

   read_data_from_metric_set();

   auto new_records = parse_perf_records(metric_buffer, total_bytes_read);
   if (new_records.empty()) {
      // No new records from the GPU yet
      return false;
   } else {
      // Records are parsed correctly, so we can reset the
      // number of bytes read so far from the metric set
      total_bytes_read = 0;
   }

   APPEND(records, new_records);

   if (records.size() < 2) {
      // Not enough records to accumulate
      return false;
   }

   return true;
}

uint64_t IntelDriver::gpu_next()
{
   if (records.size() < 2) {
      // Not enough records to accumulate
      return 0;
   }

   // Get first and second
   auto record_a = reinterpret_cast<const drm_i915_perf_record_header *>(records[0].data.data());
   auto record_b = reinterpret_cast<const drm_i915_perf_record_header *>(records[1].data.data());

   intel_perf_query_result_accumulate_fields(&perf->result,
                                             selected_query,
                                             record_a + 1,
                                             record_b + 1,
                                             false /* no_oa_accumulate */);

   // Get last timestamp
   auto gpu_timestamp = records[1].timestamp;

   // Consume first record
   records.erase(std::begin(records), std::begin(records) + 1);

   return intel_device_info_timebase_scale(&perf->devinfo, gpu_timestamp);
}

uint64_t IntelDriver::next()
{
   // Reset accumulation
   intel_perf_query_result_clear(&perf->result);
   return gpu_next();
}

uint32_t IntelDriver::gpu_clock_id() const
{
   return this->clock_id;
}

uint64_t IntelDriver::gpu_timestamp() const
{
   uint64_t timestamp;
   intel_gem_read_render_timestamp(drm_device.fd, perf->devinfo.kmd_type,
                                   &timestamp);
   return intel_device_info_timebase_scale(&perf->devinfo, timestamp);
}

bool IntelDriver::cpu_gpu_timestamp(uint64_t &cpu_timestamp,
                                    uint64_t &gpu_timestamp) const
{
   if (!intel_gem_read_correlate_cpu_gpu_timestamp(drm_device.fd,
                                                   perf->devinfo.kmd_type,
                                                   INTEL_ENGINE_CLASS_RENDER, 0,
                                                   CLOCK_BOOTTIME,
                                                   &cpu_timestamp,
                                                   &gpu_timestamp,
                                                   NULL))
      return false;

   gpu_timestamp =
      intel_device_info_timebase_scale(&perf->devinfo, gpu_timestamp);
   return true;
}

} // namespace pps
