/*
 * Mesa 3-D graphics library
 *
 * Copyright © 2021, Google Inc.
 * Copyright (C) 2023 Roman Stratiienko (r.stratiienko@gmail.com)
 * SPDX-License-Identifier: MIT
 */

#include "u_gralloc_internal.h"

#include <hardware/gralloc.h>
#include <errno.h>

#include "drm-uapi/drm_fourcc.h"
#include "util/log.h"

enum chroma_order {
   YCbCr,
   YCrCb,
};

struct droid_yuv_format {
   /* Lookup keys */
   int native;                     /* HAL_PIXEL_FORMAT_ */
   enum chroma_order chroma_order; /* chroma order is {Cb, Cr} or {Cr, Cb} */
   int chroma_step; /* Distance in bytes between subsequent chroma pixels. */

   /* Result */
   int fourcc; /* DRM_FORMAT_ */
};

/* The following table is used to look up a DRI image FourCC based
 * on native format and information contained in android_ycbcr struct. */
static const struct droid_yuv_format droid_yuv_formats[] = {
   /* Native format, YCrCb, Chroma step, DRI image FourCC */
   {HAL_PIXEL_FORMAT_YCbCr_420_888, YCbCr, 2, DRM_FORMAT_NV12},
   {HAL_PIXEL_FORMAT_YCbCr_420_888, YCbCr, 1, DRM_FORMAT_YUV420},
   {HAL_PIXEL_FORMAT_YCbCr_420_888, YCrCb, 1, DRM_FORMAT_YVU420},
   {HAL_PIXEL_FORMAT_YV12, YCrCb, 1, DRM_FORMAT_YVU420},
   /* HACK: See droid_create_image_from_prime_fds() and
    * https://issuetracker.google.com/32077885. */
   {HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED, YCbCr, 2, DRM_FORMAT_NV12},
   {HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED, YCbCr, 1, DRM_FORMAT_YUV420},
   {HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED, YCrCb, 1, DRM_FORMAT_YVU420},
   {HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED, YCrCb, 1, DRM_FORMAT_AYUV},
   {HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED, YCrCb, 1, DRM_FORMAT_XYUV8888},
};

static int
get_fourcc_yuv(int native, enum chroma_order chroma_order, int chroma_step)
{
   for (int i = 0; i < ARRAY_SIZE(droid_yuv_formats); ++i)
      if (droid_yuv_formats[i].native == native &&
          droid_yuv_formats[i].chroma_order == chroma_order &&
          droid_yuv_formats[i].chroma_step == chroma_step)
         return droid_yuv_formats[i].fourcc;

   return -1;
}

bool
is_hal_format_yuv(int native)
{
   for (int i = 0; i < ARRAY_SIZE(droid_yuv_formats); ++i)
      if (droid_yuv_formats[i].native == native)
         return true;

   return false;
}

int
get_hal_format_bpp(int native)
{
   int bpp;

   switch (native) {
   case HAL_PIXEL_FORMAT_RGBA_FP16:
      bpp = 8;
      break;
   case HAL_PIXEL_FORMAT_RGBA_8888:
   case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
      /*
       * HACK: Hardcode this to RGBX_8888 as per cros_gralloc hack.
       * TODO: Remove this once https://issuetracker.google.com/32077885 is
       * fixed.
       */
   case HAL_PIXEL_FORMAT_RGBX_8888:
   case HAL_PIXEL_FORMAT_BGRA_8888:
   case HAL_PIXEL_FORMAT_RGBA_1010102:
      bpp = 4;
      break;
   case HAL_PIXEL_FORMAT_RGB_565:
      bpp = 2;
      break;
   default:
      bpp = 0;
      break;
   }

   return bpp;
}

int
get_fourcc_from_hal_format(int native)
{
   switch (native) {
   case HAL_PIXEL_FORMAT_RGB_565:
      return DRM_FORMAT_RGB565;
   case HAL_PIXEL_FORMAT_BGRA_8888:
      return DRM_FORMAT_ARGB8888;
   case HAL_PIXEL_FORMAT_RGBA_8888:
      return DRM_FORMAT_ABGR8888;
   case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
      /*
       * HACK: Hardcode this to RGBX_8888 as per cros_gralloc hack.
       * TODO: Remove this once https://issuetracker.google.com/32077885 is
       * fixed.
       */
   case HAL_PIXEL_FORMAT_RGBX_8888:
      return DRM_FORMAT_XBGR8888;
   case HAL_PIXEL_FORMAT_RGBA_FP16:
      return DRM_FORMAT_ABGR16161616F;
   case HAL_PIXEL_FORMAT_RGBA_1010102:
      return DRM_FORMAT_ABGR2101010;
   default:
      mesa_logw("unsupported native buffer format 0x%x", native);
   }
   return -1;
}

/* Fill the bufferinfo structure using the information from the
 * android_ycbcr struct. fds[] are not filled by this function.
 */
int
bufferinfo_from_ycbcr(const struct android_ycbcr *ycbcr,
                      struct u_gralloc_buffer_handle *hnd,
                      struct u_gralloc_buffer_basic_info *out)
{
   enum chroma_order chroma_order =
      ((size_t)ycbcr->cr < (size_t)ycbcr->cb) ? YCrCb : YCbCr;

   /* .chroma_step is the byte distance between the same chroma channel
    * values of subsequent pixels, assumed to be the same for Cb and Cr. */
   int drm_fourcc =
      get_fourcc_yuv(hnd->hal_format, chroma_order, ycbcr->chroma_step);

   if (drm_fourcc == -1) {
      mesa_logw("unsupported YUV format, native = %x, chroma_order = %s, "
                "chroma_step = %zu",
                hnd->hal_format, chroma_order == YCbCr ? "YCbCr" : "YCrCb",
                ycbcr->chroma_step);
      return -EINVAL;
   }

   out->drm_fourcc = drm_fourcc;

   out->num_planes = ycbcr->chroma_step == 2 ? 2 : 3;
   /* When lock_ycbcr's usage argument contains no SW_READ/WRITE flags
    * it will return the .y/.cb/.cr pointers based on a NULL pointer,
    * so they can be interpreted as offsets. */
   out->offsets[0] = (size_t)ycbcr->y;
   /* We assume here that all the planes are located in one DMA-buf. */
   if (chroma_order == YCrCb) {
      out->offsets[1] = (size_t)ycbcr->cr;
      out->offsets[2] = (size_t)ycbcr->cb;
   } else {
      out->offsets[1] = (size_t)ycbcr->cb;
      out->offsets[2] = (size_t)ycbcr->cr;
   }

   /* .ystride is the line length (in bytes) of the Y plane,
    * .cstride is the line length (in bytes) of any of the remaining
    * Cb/Cr/CbCr planes, assumed to be the same for Cb and Cr for fully
    * planar formats. */
   out->strides[0] = ycbcr->ystride;
   out->strides[1] = out->strides[2] = ycbcr->cstride;

   return 0;
}
