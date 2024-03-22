/*
 * Copyright © 2019-2020 Collabora, Ltd.
 * Author: Antonio Caggiano <antonio.caggiano@collabora.com>
 * Author: Robert Beckett <bob.beckett@collabora.com>
 * Author: Corentin Noël <corentin.noel@collabora.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <cstdlib>

#include "pps_datasource.h"

int main(int argc, const char **argv)
{
   using namespace pps;

   // Connects to the system tracing service
   perfetto::TracingInitArgs args;
   args.backends = perfetto::kSystemBackend;
   perfetto::Tracing::Initialize(args);

   std::string driver_name =
      (argc > 1) ? Driver::find_driver_name(argv[1]) : Driver::default_driver_name();
   GpuDataSource::register_data_source(driver_name);

   const auto &driver = Driver::get_supported_drivers().at(driver_name);
   if (!driver->is_dump_perfcnt_preemptible())
      make_thread_rt();

   while (true) {
      GpuDataSource::wait_started();
      GpuDataSource::Trace(GpuDataSource::trace_callback);
   }

   return EXIT_SUCCESS;
}
