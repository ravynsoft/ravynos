/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2021 GlobalLogic Ukraine
 * Copyright (C) 2021-2022 Roman Stratiienko (r.stratiienko@gmail.com)
 * SPDX-License-Identifier: MIT
 */

#include <aidl/android/hardware/graphics/common/BufferUsage.h>
#include <aidl/android/hardware/graphics/common/ChromaSiting.h>
#include <aidl/android/hardware/graphics/common/Dataspace.h>
#include <aidl/android/hardware/graphics/common/ExtendableType.h>
#include <aidl/android/hardware/graphics/common/PlaneLayoutComponent.h>
#include <aidl/android/hardware/graphics/common/PlaneLayoutComponentType.h>
#include <gralloctypes/Gralloc4.h>
#include <system/window.h>

#include "util/log.h"
#include "u_gralloc_internal.h"

using aidl::android::hardware::graphics::common::BufferUsage;
using aidl::android::hardware::graphics::common::ChromaSiting;
using aidl::android::hardware::graphics::common::Dataspace;
using aidl::android::hardware::graphics::common::ExtendableType;
using aidl::android::hardware::graphics::common::PlaneLayout;
using aidl::android::hardware::graphics::common::PlaneLayoutComponent;
using aidl::android::hardware::graphics::common::PlaneLayoutComponentType;
using android::hardware::hidl_handle;
using android::hardware::hidl_vec;
using android::hardware::graphics::mapper::V4_0::Error;
using android::hardware::graphics::mapper::V4_0::IMapper;

using MetadataType =
   android::hardware::graphics::mapper::V4_0::IMapper::MetadataType;

Error
GetMetadata(android::sp<IMapper> mapper, const native_handle_t *buffer,
            MetadataType type, hidl_vec<uint8_t> *metadata)
{
   Error error = Error::NONE;

   auto native_handle = const_cast<native_handle_t *>(buffer);

   auto ret =
      mapper->get(native_handle, type,
                  [&](const auto &get_error, const auto &get_metadata) {
                     error = get_error;
                     *metadata = get_metadata;
                  });

   if (!ret.isOk())
      error = Error::NO_RESOURCES;

   return error;
}

std::optional<std::vector<PlaneLayout>>
GetPlaneLayouts(android::sp<IMapper> mapper, const native_handle_t *buffer)
{
   hidl_vec<uint8_t> encoded_layouts;

   Error error = GetMetadata(mapper, buffer,
                             android::gralloc4::MetadataType_PlaneLayouts,
                             &encoded_layouts);

   if (error != Error::NONE)
      return std::nullopt;

   std::vector<PlaneLayout> plane_layouts;

   auto status =
      android::gralloc4::decodePlaneLayouts(encoded_layouts, &plane_layouts);

   if (status != android::OK)
      return std::nullopt;

   return plane_layouts;
}

struct gralloc4 {
   struct u_gralloc base;
   android::sp<IMapper> mapper;
};

extern "C" {

static int
mapper4_get_buffer_basic_info(struct u_gralloc *gralloc,
                              struct u_gralloc_buffer_handle *hnd,
                              struct u_gralloc_buffer_basic_info *out)
{
   gralloc4 *gr4 = (gralloc4 *)gralloc;

   if (gr4->mapper == nullptr)
      return -EINVAL;

   if (!hnd->handle)
      return -EINVAL;

   hidl_vec<uint8_t> encoded_format;
   auto err = GetMetadata(gr4->mapper, hnd->handle,
                          android::gralloc4::MetadataType_PixelFormatFourCC,
                          &encoded_format);
   if (err != Error::NONE)
      return -EINVAL;

   uint32_t drm_fourcc;

   auto status =
      android::gralloc4::decodePixelFormatFourCC(encoded_format, &drm_fourcc);
   if (status != android::OK)
      return -EINVAL;

   uint64_t drm_modifier;

   hidl_vec<uint8_t> encoded_modifier;
   err = GetMetadata(gr4->mapper, hnd->handle,
                     android::gralloc4::MetadataType_PixelFormatModifier,
                     &encoded_modifier);
   if (err != Error::NONE)
      return -EINVAL;

   status = android::gralloc4::decodePixelFormatModifier(encoded_modifier,
                                                         &drm_modifier);
   if (status != android::OK)
      return -EINVAL;

   out->drm_fourcc = drm_fourcc;
   out->modifier = drm_modifier;

   auto layouts_opt = GetPlaneLayouts(gr4->mapper, hnd->handle);

   if (!layouts_opt)
      return -EINVAL;

   std::vector<PlaneLayout> &layouts = *layouts_opt;

   out->num_planes = layouts.size();

   int fd_index = 0;

   for (uint32_t i = 0; i < layouts.size(); i++) {
      out->strides[i] = layouts[i].strideInBytes;
      out->offsets[i] = layouts[i].offsetInBytes;

      /* offset == 0 means layer is located in different dma-buf */
      if (out->offsets[i] == 0 && i > 0)
         fd_index++;

      if (fd_index >= hnd->handle->numFds)
         return -EINVAL;

      out->fds[i] = hnd->handle->data[fd_index];
   }

   return 0;
}

static int
mapper4_get_buffer_color_info(struct u_gralloc *gralloc,
                              struct u_gralloc_buffer_handle *hnd,
                              struct u_gralloc_buffer_color_info *out)
{
   gralloc4 *gr4 = (gralloc4 *)gralloc;

   if (gr4->mapper == nullptr)
      return -EINVAL;

   if (!hnd->handle)
      return -EINVAL;

   /* optional attributes */
   hidl_vec<uint8_t> encoded_chroma_siting;
   std::optional<ChromaSiting> chroma_siting;
   auto err = GetMetadata(gr4->mapper, hnd->handle,
                          android::gralloc4::MetadataType_ChromaSiting,
                          &encoded_chroma_siting);
   if (err == Error::NONE) {
      ExtendableType chroma_siting_ext;
      auto status = android::gralloc4::decodeChromaSiting(
         encoded_chroma_siting, &chroma_siting_ext);
      if (status != android::OK)
         return -EINVAL;

      chroma_siting =
         android::gralloc4::getStandardChromaSitingValue(chroma_siting_ext);
   }

   hidl_vec<uint8_t> encoded_dataspace;
   err = GetMetadata(gr4->mapper, hnd->handle,
                     android::gralloc4::MetadataType_Dataspace,
                     &encoded_dataspace);
   if (err == Error::NONE) {
      Dataspace dataspace;
      auto status =
         android::gralloc4::decodeDataspace(encoded_dataspace, &dataspace);
      if (status != android::OK)
         return -EINVAL;

      Dataspace standard =
         (Dataspace)((int)dataspace & (uint32_t)Dataspace::STANDARD_MASK);
      switch (standard) {
      case Dataspace::STANDARD_BT709:
         out->yuv_color_space = __DRI_YUV_COLOR_SPACE_ITU_REC709;
         break;
      case Dataspace::STANDARD_BT601_625:
      case Dataspace::STANDARD_BT601_625_UNADJUSTED:
      case Dataspace::STANDARD_BT601_525:
      case Dataspace::STANDARD_BT601_525_UNADJUSTED:
         out->yuv_color_space = __DRI_YUV_COLOR_SPACE_ITU_REC601;
         break;
      case Dataspace::STANDARD_BT2020:
      case Dataspace::STANDARD_BT2020_CONSTANT_LUMINANCE:
         out->yuv_color_space = __DRI_YUV_COLOR_SPACE_ITU_REC2020;
         break;
      default:
         break;
      }

      Dataspace range =
         (Dataspace)((int)dataspace & (uint32_t)Dataspace::RANGE_MASK);
      switch (range) {
      case Dataspace::RANGE_FULL:
         out->sample_range = __DRI_YUV_FULL_RANGE;
         break;
      case Dataspace::RANGE_LIMITED:
         out->sample_range = __DRI_YUV_NARROW_RANGE;
         break;
      default:
         break;
      }
   }

   if (chroma_siting) {
      switch (*chroma_siting) {
      case ChromaSiting::SITED_INTERSTITIAL:
         out->horizontal_siting = __DRI_YUV_CHROMA_SITING_0_5;
         out->vertical_siting = __DRI_YUV_CHROMA_SITING_0_5;
         break;
      case ChromaSiting::COSITED_HORIZONTAL:
         out->horizontal_siting = __DRI_YUV_CHROMA_SITING_0;
         out->vertical_siting = __DRI_YUV_CHROMA_SITING_0_5;
         break;
      default:
         break;
      }
   }

   return 0;
}

static int
mapper4_get_front_rendering_usage(struct u_gralloc *gralloc,
                                  uint64_t *out_usage)
{
   assert(out_usage);
#if ANDROID_API_LEVEL >= 33
   *out_usage = static_cast<uint64_t>(BufferUsage::FRONT_BUFFER);

   return 0;
#else
   return -ENOTSUP;
#endif
}

static int
destroy(struct u_gralloc *gralloc)
{
   gralloc4 *gr = (struct gralloc4 *)gralloc;
   delete gr;

   return 0;
}

struct u_gralloc *
u_gralloc_imapper_api_create()
{
   auto mapper = IMapper::getService();
   if (!mapper)
      return NULL;

   auto gr = new gralloc4;
   gr->mapper = mapper;
   gr->base.ops.get_buffer_basic_info = mapper4_get_buffer_basic_info;
   gr->base.ops.get_buffer_color_info = mapper4_get_buffer_color_info;
   gr->base.ops.get_front_rendering_usage = mapper4_get_front_rendering_usage;
   gr->base.ops.destroy = destroy;

   mesa_logi("Using IMapper v4 API");

   return &gr->base;
}

} // extern "C"
