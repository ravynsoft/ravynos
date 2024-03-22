/*
 * Copyright © 2020 Collabora, Ltd.
 * Author: Antonio Caggiano <antonio.caggiano@collabora.com>
 * Author: Rohan Garg <rohan.garg@collabora.com>
 * Author: Robert Beckett <bob.beckett@collabora.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace pps
{
/// @brief Helper class for a DRM device
class DrmDevice
{
   public:
   /// @return The number of DRM devices available in the system
   static uint32_t device_count();

   /// @return All DRM devices available in the system
   static std::vector<DrmDevice> create_all();

   /// @return A DRM device selected by its number in the system, nullopt otherwise
   static std::optional<DrmDevice> create(int32_t gpu_num);

   /// @brief Prefer calling create instead of default constructor
   DrmDevice() = default;

   // Allow move
   DrmDevice(DrmDevice &&);
   DrmDevice &operator=(DrmDevice &&);

   // Forbid copy
   DrmDevice(const DrmDevice &) = delete;
   DrmDevice &operator=(const DrmDevice &) = delete;

   ~DrmDevice();

   /// @return Whether a device has a valid name
   operator bool() const;

   /// File descriptor of the device opened in read/write mode
   int fd = -1;
   int32_t gpu_num = -1;
   std::string name = "";
};

} // namespace pps
