/*
 * Copyright © 2021 Collabora, Ltd.
 * Author: Antonio Caggiano <antonio.caggiano@collabora.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "dev/intel_device_info.h"
#include "perf/intel_perf.h"
#include "perf/intel_perf_query.h"

namespace pps
{
class IntelPerf
{
   public:
   IntelPerf(int drm_fd);
   ~IntelPerf();

   std::vector<struct intel_perf_query_info*> get_queries() const;

   bool open(uint64_t sampling_period_ns, struct intel_perf_query_info *query);
   void close();

   bool oa_stream_ready() const;
   ssize_t read_oa_stream(void *buf, size_t bytes) const;

   int drm_fd = -1;

   void *ralloc_ctx = nullptr;
   void *ralloc_cfg = nullptr;

   struct intel_perf_context *ctx = nullptr;
   struct intel_perf_config *cfg = nullptr;

   // Accumulations are stored here
   struct intel_perf_query_result result = {};

   struct intel_device_info devinfo = {};
};

} // namespace pps
