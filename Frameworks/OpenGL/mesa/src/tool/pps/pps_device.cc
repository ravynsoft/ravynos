/*
 * Copyright © 2020 Collabora, Ltd.
 * Author: Antonio Caggiano <antonio.caggiano@collabora.com>
 * Author: Rohan Garg <rohan.garg@collabora.com>
 * Author: Robert Beckett <bob.beckett@collabora.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "pps_device.h"

#include <cassert>
#include <fcntl.h>
#include <memory>
#include <unistd.h>
#include <xf86drm.h>

namespace pps
{
#define MAX_DRM_DEVICES 64

uint32_t DrmDevice::device_count()
{
   drmDevicePtr devices[MAX_DRM_DEVICES] = {};
   int num_devices = drmGetDevices2(0, devices, MAX_DRM_DEVICES);
   drmFreeDevices(devices, num_devices);
   return static_cast<uint32_t>(num_devices);
}

/// @return The name of a DRM device, empty string in case of error
std::string query_drm_name(const int fd)
{
   assert(fd && "Failed to query DrmDevice: invalid fd");

   std::string name = "";

   if (drmVersionPtr version = drmGetVersion(fd)) {
      name = std::string(version->name, version->name_len);
      drmFreeVersion(version);
   }

   return name;
}

/// @return A DRM device, nullopt in case of error
std::optional<DrmDevice> create_drm_device(int fd, int32_t gpu_num)
{
   if (fd < 0 || gpu_num < 0) {
      return std::nullopt;
   }

   // Try getting the name
   std::string name = query_drm_name(fd);
   if (name.empty()) {
      return std::nullopt;
   }

   const char *dri_prime = getenv("DRI_PRIME");
   if (dri_prime != NULL) {
      drmDevicePtr drm_device;
      uint16_t vendor_id, device_id;
      bool prime_is_vid_did =
         sscanf(dri_prime, "%hx:%hx", &vendor_id, &device_id) == 2;

      if (prime_is_vid_did && drmGetDevice2(fd, 0, &drm_device) == 0) {
         if (drm_device->bustype == DRM_BUS_PCI &&
             (drm_device->deviceinfo.pci->vendor_id != vendor_id ||
              drm_device->deviceinfo.pci->device_id != device_id))
            return std::nullopt;
      }
   }

   auto ret = DrmDevice();
   ret.fd = fd;
   ret.gpu_num = gpu_num;
   ret.name = name;
   return ret;
}

std::vector<DrmDevice> DrmDevice::create_all()
{
   std::vector<DrmDevice> ret = {};

   drmDevicePtr devices[MAX_DRM_DEVICES] = {};
   int num_devices = drmGetDevices2(0, devices, MAX_DRM_DEVICES);
   if (num_devices <= 0) {
      return ret;
   }

   for (int32_t gpu_num = 0; gpu_num < num_devices; gpu_num++) {
      drmDevicePtr device = devices[gpu_num];
      if ((device->available_nodes & (1 << DRM_NODE_RENDER))) {
         int fd = open(device->nodes[DRM_NODE_RENDER], O_RDWR);

         // If it can create a device, push it into the vector
         if (auto drm_device = create_drm_device(fd, gpu_num)) {
            ret.emplace_back(std::move(drm_device.value()));
         }
      }
   }

   drmFreeDevices(devices, num_devices);
   return ret;
}

std::optional<DrmDevice> DrmDevice::create(int32_t gpu_num)
{
   std::optional<DrmDevice> ret = std::nullopt;

   if (gpu_num < 0) {
      return ret;
   }

   drmDevicePtr devices[MAX_DRM_DEVICES] = {};
   int num_devices = drmGetDevices2(0, devices, MAX_DRM_DEVICES);

   if (num_devices > 0 && gpu_num < num_devices) {
      drmDevicePtr device = devices[gpu_num];
      int fd = open(device->nodes[DRM_NODE_RENDER], O_RDWR);
      ret = create_drm_device(fd, gpu_num);
   }

   drmFreeDevices(devices, num_devices);
   return ret;
}

DrmDevice::DrmDevice(DrmDevice &&other)
   : fd {other.fd}
   , gpu_num {other.gpu_num}
   , name {std::move(other.name)}
{
   other.fd = -1;
   other.gpu_num = -1;
}

DrmDevice &DrmDevice::operator=(DrmDevice &&other)
{
   std::swap(fd, other.fd);
   std::swap(gpu_num, other.gpu_num);
   std::swap(name, other.name);
   return *this;
}

DrmDevice::~DrmDevice()
{
   if (fd >= 0) {
      close(fd);
   }
}

DrmDevice::operator bool() const
{
   return !name.empty();
}

} // namespace pps
