/*
 * Copyright © 2020-2021 Collabora, Ltd.
 * Author: Antonio Caggiano <antonio.caggiano@collabora.com>
 * Author: Rohan Garg <rohan.garg@collabora.com>
 * Author: Robert Beckett <bob.beckett@collabora.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <pps/pps_driver.h>

#include "pan_pps_perf.h"

namespace pps {
/// @brief Panfrost implementation of PPS driver.
/// This driver queries the GPU through `drm/panfrost_drm.h`, using performance
/// counters ioctls, which can be enabled by setting a kernel parameter:
/// `modprobe panfrost unstable_ioctls=1`. The ioctl needs a buffer to copy data
/// from kernel to user space.
class PanfrostDriver : public Driver {
 public:
   static inline PanfrostDriver &into(Driver &dri);
   static inline const PanfrostDriver &into(const Driver &dri);

   /// @param A list of mali counter names
   /// @return A pair with two lists: counter groups and available counters
   static std::pair<std::vector<CounterGroup>, std::vector<Counter>>
   create_available_counters(const PanfrostPerf &perf);

   PanfrostDriver();
   ~PanfrostDriver();

   uint64_t get_min_sampling_period_ns() override;
   bool init_perfcnt() override;
   void enable_counter(uint32_t counter_id) override;
   void enable_all_counters() override;
   void enable_perfcnt(uint64_t sampling_period_ns) override;
   void disable_perfcnt() override;
   bool dump_perfcnt() override;
   uint64_t next() override;
   uint32_t gpu_clock_id() const override;
   uint64_t gpu_timestamp() const override;
   bool cpu_gpu_timestamp(uint64_t &cpu_timestamp,
                          uint64_t &gpu_timestamp) const override;

   uint64_t last_dump_ts = 0;

   std::unique_ptr<PanfrostDevice> dev = nullptr;
   std::unique_ptr<PanfrostPerf> perf = nullptr;
};

PanfrostDriver &
PanfrostDriver::into(Driver &dri)
{
   return reinterpret_cast<PanfrostDriver &>(dri);
}

const PanfrostDriver &
PanfrostDriver::into(const Driver &dri)
{
   return reinterpret_cast<const PanfrostDriver &>(dri);
}

} // namespace pps
