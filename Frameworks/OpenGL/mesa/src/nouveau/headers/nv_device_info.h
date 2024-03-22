#ifndef NV_DEVINFO_H
#define NV_DEVINFO_H

#include "util/macros.h"

#define NVIDIA_VENDOR_ID 0x10de

enum ENUM_PACKED nv_device_type {
   NV_DEVICE_TYPE_IGP,
   NV_DEVICE_TYPE_DIS,
   NV_DEVICE_TYPE_SOC,
};

struct nv_device_info {
   enum nv_device_type type;

   uint16_t device_id;
   uint16_t chipset;

   char device_name[64];
   char chipset_name[16];

   /* Populated if type == NV_DEVICE_TYPE_DIS */
   struct {
      uint16_t domain;
      uint8_t bus;
      uint8_t dev;
      uint8_t func;
      uint8_t revision_id;
   } pci;

   uint8_t sm; /**< Shader model */

   uint8_t gpc_count;
   uint16_t tpc_count;
   uint8_t mp_per_tpc;
   uint8_t max_warps_per_mp;

   uint16_t cls_copy;
   uint16_t cls_eng2d;
   uint16_t cls_eng3d;
   uint16_t cls_m2mf;
   uint16_t cls_compute;

   uint64_t vram_size_B;
};

#endif /* NV_DEVINFO_H */
