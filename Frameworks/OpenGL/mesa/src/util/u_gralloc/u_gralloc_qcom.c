/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2023 Roman Stratiienko (r.stratiienko@gmail.com)
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <string.h>
#include <hardware/gralloc.h>
#include <hardware/gralloc1.h>

#include "drm-uapi/drm_fourcc.h"

#include "util/log.h"
#include "util/u_memory.h"

#include "u_gralloc_internal.h"

/* Using this gralloc is not recommended for new distributions. */

struct qcom_gralloc {
   struct u_gralloc base;
   hw_module_t *gralloc_module;
   gralloc1_device_t *gralloc1_device;
   void *perform_handle;
   int (* perform)(void *dev, int op, ...);
   struct u_gralloc *fallback_gralloc;
};

#define GRALLOC1_FUNCTION_PERFORM 0x00001000 /* QCOM gralloc-specific */

static const char qcom_gralloc_name[] = "Graphics Memory Allocator Module";
static const char qcom_gralloc_author[] = "The Android Open Source Project";

static const char caf_gralloc_name[] = "Graphics Memory Module";
static const char caf_gralloc_author[] = "Code Aurora Forum";

#define QCOM_GRALLOC_PROBE_WIDTH 1024
#define QCOM_GRALLOC_PROBE_FORMAT 1 /* HAL_PIXEL_FORMAT_RGBA_8888 */

#define GRALLOC_MODULE_PERFORM_GET_STRIDE 2
#define GRALLOC_MODULE_PERFORM_GET_YUV_PLANE_INFO 7
#define GRALLOC_MODULE_PERFORM_GET_UBWC_FLAG 9

static int
fallback_gralloc_get_yuv_info(struct u_gralloc *gralloc,
                              struct u_gralloc_buffer_handle *hnd,
                              struct u_gralloc_buffer_basic_info *out)
{
   struct qcom_gralloc *gr = (struct qcom_gralloc *)gralloc;
   struct android_ycbcr ycbcr;
   int ret;

   memset(&ycbcr, 0, sizeof(ycbcr));
   ret = gr->perform(gr->perform_handle,
                     GRALLOC_MODULE_PERFORM_GET_YUV_PLANE_INFO,
                     hnd->handle, &ycbcr);
   if (ret) {
      /* HACK: See native_window_buffer_get_buffer_info() and
       * https://issuetracker.google.com/32077885.*/
      if (hnd->hal_format == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED)
         return -EAGAIN;

      mesa_logw("gralloc->lock_ycbcr failed: %d", ret);
      return -EINVAL;
   }

   ret = bufferinfo_from_ycbcr(&ycbcr, hnd, out);
   if (ret)
      return ret;

   out->fds[1] = out->fds[0] = hnd->handle->data[0];
   if (out->num_planes == 3)
      out->fds[2] = hnd->handle->data[0];

   return 0;
}

static int
get_buffer_info(struct u_gralloc *gralloc,
                struct u_gralloc_buffer_handle *hnd,
                struct u_gralloc_buffer_basic_info *out)
{
   struct qcom_gralloc *gr = (struct qcom_gralloc *)gralloc;

   int drm_fourcc = 0;
   int stride = 0;
   int out_flag = 0;
   int err;

   err = gr->perform(gr->perform_handle, GRALLOC_MODULE_PERFORM_GET_UBWC_FLAG,
                     hnd->handle, &out_flag);
   /* This may fail since some earlier MSM  grallocs do not support this
    * perform call
    */
   if (!err && out_flag)
      out->modifier = DRM_FORMAT_MOD_QCOM_COMPRESSED;
   else
      out->modifier = DRM_FORMAT_MOD_LINEAR;

   if (is_hal_format_yuv(hnd->hal_format)) {
      int ret = fallback_gralloc_get_yuv_info(gralloc, hnd, out);
      /*
       * HACK: https://issuetracker.google.com/32077885
       * There is no API available to properly query the
       * IMPLEMENTATION_DEFINED format. As a workaround we rely here on
       * gralloc allocating either an arbitrary YCbCr 4:2:0 or RGBX_8888, with
       * the latter being recognized by lock_ycbcr failing.
       */
      if (ret != -EAGAIN)
         return ret;
   }

   drm_fourcc = get_fourcc_from_hal_format(hnd->hal_format);
   if (drm_fourcc == -1) {
      mesa_loge("Failed to get drm_fourcc");
      return -EINVAL;
   }

   stride = hnd->pixel_stride * get_hal_format_bpp(hnd->hal_format);
   if (stride == 0) {
      mesa_loge("Failed to calcuulate stride");
      return -EINVAL;
   }

   out->drm_fourcc = drm_fourcc;
   out->num_planes = 1;
   out->fds[0] = hnd->handle->data[0];
   out->strides[0] = stride;

   return 0;
}

static int
destroy(struct u_gralloc *gralloc)
{
   struct qcom_gralloc *gr = (struct qcom_gralloc *)gralloc;
   if (gr->gralloc1_device)
      gralloc1_close(gr->gralloc1_device);

   if (gr->gralloc_module)
      dlclose(gr->gralloc_module->dso);

   if (gr->fallback_gralloc)
      gr->fallback_gralloc->ops.destroy(gr->fallback_gralloc);

   FREE(gr);

   return 0;
}

struct u_gralloc *
u_gralloc_qcom_create()
{
   struct qcom_gralloc *gr = CALLOC_STRUCT(qcom_gralloc);
   int err = 0;

   err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID,
                       (const hw_module_t **)&gr->gralloc_module);

   if (err)
      goto fail;

   bool match = false;

   if (strcmp(gr->gralloc_module->name, qcom_gralloc_name) == 0 &&
       strcmp(gr->gralloc_module->author, qcom_gralloc_author) == 0) {
      match = true;
   }

   if (strcmp(gr->gralloc_module->name, caf_gralloc_name) == 0 &&
       strcmp(gr->gralloc_module->author, caf_gralloc_author) == 0) {
      match = true;
   }

   if (!match)
      goto fail;

   if (gr->gralloc_module->module_api_version <= GRALLOC_MODULE_API_VERSION_0_3) {
      gralloc_module_t *gralloc_module =
         (gralloc_module_t *)gr->gralloc_module;
      gr->perform = (int (*)(void *, int, ...))gralloc_module->perform;
      gr->perform_handle = gr->gralloc_module;
   } else {
      err = gralloc1_open(gr->gralloc_module, &gr->gralloc1_device);
      if (err)
         goto fail;

      gr->perform = (int (*)(void *, int, ...))gr->gralloc1_device->
         getFunction(gr->gralloc1_device, GRALLOC1_FUNCTION_PERFORM);

      gr->perform_handle = gr->gralloc1_device;
   }

   if (!gr->perform)
      goto fail;

   /* Check if the gralloc module supports the required perform call */
   int out_stride = 0;
   err = gr->perform(gr->perform_handle,
                     GRALLOC_MODULE_PERFORM_GET_STRIDE,
                     QCOM_GRALLOC_PROBE_WIDTH,
                     QCOM_GRALLOC_PROBE_FORMAT,
                     &out_stride);
   if (err)
      goto fail;

   if (out_stride == 0)
      goto fail;

   gr->base.ops.get_buffer_basic_info = get_buffer_info;
   gr->base.ops.destroy = destroy;

   mesa_logi("Using QCOM gralloc (aosp/hardware/qcom/display/*/libgralloc). ");
   mesa_logw("QCOM Gralloc API is old. Consider using Gralloc4 API instead.");

   gr->fallback_gralloc = u_gralloc_fallback_create();

   return &gr->base;

fail:
   destroy(&gr->base);

   return NULL;
}
