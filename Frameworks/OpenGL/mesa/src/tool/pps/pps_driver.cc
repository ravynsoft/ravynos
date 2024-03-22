/*
 * Copyright © 2019-2020 Collabora, Ltd.
 * Author: Antonio Caggiano <antonio.caggiano@collabora.com>
 * Author: Rohan Garg <rohan.garg@collabora.com>
 * Author: Robert Beckett <bob.beckett@collabora.com>
 * Author: Corentin Noël <corentin.noel@collabora.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "pps_driver.h"

#include <iterator>
#include <sstream>

#ifdef PPS_FREEDRENO
#include "freedreno/ds/fd_pps_driver.h"
#endif // PPS_FREEDRENO

#ifdef PPS_INTEL
#include "intel/ds/intel_pps_driver.h"
#endif // PPS_INTEL

#ifdef PPS_PANFROST
#include "panfrost/ds/pan_pps_driver.h"
#endif // PPS_PANFROST

#include "pps.h"
#include "pps_algorithm.h"

namespace pps
{
std::unordered_map<std::string, std::unique_ptr<Driver>> create_supported_drivers()
{
   std::unordered_map<std::string, std::unique_ptr<Driver>> map;

#ifdef PPS_FREEDRENO
   map.emplace("msm", std::make_unique<FreedrenoDriver>());
#endif // PPS_FREEDRENO

#ifdef PPS_INTEL
   map.emplace("i915", std::make_unique<IntelDriver>());
#endif // PPS_INTEL

#ifdef PPS_PANFROST
   map.emplace("panfrost", std::make_unique<PanfrostDriver>());
#endif // PPS_PANFROST

   return map;
}

const std::unordered_map<std::string, std::unique_ptr<Driver>> &Driver::get_supported_drivers()
{
   static auto map = create_supported_drivers();
   return map;
}

const std::vector<std::string> Driver::supported_device_names()
{
   std::vector<std::string> supported_device_names;

   for (auto &entry : get_supported_drivers()) {
      supported_device_names.emplace_back(entry.first);
   }

   return supported_device_names;
}

Driver *Driver::get_driver(DrmDevice &&drm_device)
{
   auto &supported_drivers = get_supported_drivers();
   auto it = supported_drivers.find(drm_device.name);
   if (it == std::end(supported_drivers)) {
      PERFETTO_FATAL("Failed to find a driver for DRM device %s", drm_device.name.c_str());
   }

   Driver *driver = it->second.get();
   driver->drm_device = std::move(drm_device);
   return driver;
}

std::string Driver::default_driver_name()
{
   auto supported_devices = Driver::supported_device_names();
   auto devices = DrmDevice::create_all();
   for (auto &device : devices) {
      if (CONTAINS(supported_devices, device.name)) {
         PPS_LOG_IMPORTANT("Driver selected: %s", device.name.c_str());
         return device.name;
      }
   }
   PPS_LOG_FATAL("Failed to find any driver");
}

std::string Driver::find_driver_name(const char *requested)
{
   auto supported_devices = Driver::supported_device_names();
   auto devices = DrmDevice::create_all();
   for (auto &device : devices) {
      if (device.name == requested) {
         PPS_LOG_IMPORTANT("Driver selected: %s", device.name.c_str());
         return device.name;
      }
   }

   std::ostringstream drivers_os;
   std::copy(supported_devices.begin(),
      supported_devices.end() - 1,
      std::ostream_iterator<std::string>(drivers_os, ", "));
   drivers_os << supported_devices.back();

   PPS_LOG_ERROR(
      "Device '%s' not found (supported drivers: %s)", requested, drivers_os.str().c_str());

   return default_driver_name();
}

} // namespace pps
