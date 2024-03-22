/*
 * Copyright © 2020 Collabora, Ltd.
 * Author: Antonio Caggiano <antonio.caggiano@collabora.com>
 * Author: Robert Beckett <bob.beckett@collabora.com>
 * Author: Corentin Noël <corentin.noel@collabora.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "pps_counter.h"
#include "pps_device.h"

namespace pps
{
/// @brief Abstract Driver class
class Driver
{
   public:
   /// @return A map of supported DRM device names and their relative pps driver
   static const std::unordered_map<std::string, std::unique_ptr<Driver>> &get_supported_drivers();

   /// @return A list of supported DRM device names
   static const std::vector<std::string> supported_device_names();

   /// @return A driver supporting a specific DRM device
   static Driver *get_driver(DrmDevice &&drm_device);

   /// @return The name of a default selected PPS driver
   static std::string default_driver_name();

   /// @return The name of a driver based on the request, otherwise the default driver name
   static std::string find_driver_name(const char *requested_name);

   Driver() = default;
   virtual ~Driver() = default;

   // Forbid copy
   Driver(const Driver &) = delete;
   Driver &operator=(const Driver &) = delete;

   /// @return Whether dump_perfcnt is preemptible
   virtual bool is_dump_perfcnt_preemptible() const { return true; }

   /// @return The minimum sampling period for the current device
   virtual uint64_t get_min_sampling_period_ns() = 0;

   /// @brief Enable a counter by its ID
   virtual void enable_counter(uint32_t counter_id) = 0;

   virtual void enable_all_counters() = 0;

   /// @brief Initialize performance counters data such as groups and counters
   /// @return Whether it was successful or not
   virtual bool init_perfcnt() = 0;

   /// @brief Enables performance counters, meaning that from now on they can be sampled
   virtual void enable_perfcnt(uint64_t sampling_period_ns) = 0;

   /// @brief Disables performance counters on the device
   virtual void disable_perfcnt() = 0;

   /// @brief Asking the GPU to dump performance counters could have different meanings
   /// depending on the concrete driver. Some could just ask the GPU to dump counters to a
   /// user space buffer, while some others will need to read data from a stream which was
   /// written asynchronously.
   /// @return Whether it was able to dump, false otherwise
   virtual bool dump_perfcnt() = 0;

   /// @brief After dumping performance counters, with this function you can iterate
   /// through the samples collected.
   /// @return The GPU timestamp associated to current sample, or 0 if there are no more samples
   virtual uint64_t next() = 0;

   /// Clock ID in which the values returned by gpu_timestamp() belong
   virtual uint32_t gpu_clock_id() const = 0;

   /// Sample a timestamp from the GPU
   virtual uint64_t gpu_timestamp() const = 0;

   /// Sample a timestamp from both the CPU & the GPU
   ///
   /// This is useful when the driver can do a better timestamp correlation
   /// than sampling separately CPU & GPU timestamps.
   virtual bool cpu_gpu_timestamp(uint64_t &cpu_timestamp, uint64_t &gpu_timestamp) const = 0;

   DrmDevice drm_device;

   /// List of counter groups
   std::vector<CounterGroup> groups;

   /// List of counters exposed by the GPU
   std::vector<Counter> counters;

   /// List of counters that are actually enabled
   std::vector<Counter> enabled_counters;

   protected:
   // Prevent object slicing by allowing move only from subclasses
   Driver(Driver &&) = default;
   Driver &operator=(Driver &&) = default;
};

} // namespace pps
