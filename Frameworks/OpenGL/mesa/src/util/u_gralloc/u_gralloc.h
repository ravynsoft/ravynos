/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2022 Roman Stratiienko (r.stratiienko@gmail.com)
 * SPDX-License-Identifier: MIT
 */

#ifndef U_GRALLOC_H
#define U_GRALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cutils/native_handle.h>

#include <stdbool.h>

#include "util/macros.h"
#include "GL/internal/dri_interface.h"

struct u_gralloc;

/* Both Vulkan and EGL API exposes HAL format / pixel stride which is required
 * by the fallback implementation.
 */
struct u_gralloc_buffer_handle {
   const native_handle_t *handle;
   int hal_format;
   int pixel_stride;
};

struct u_gralloc_buffer_basic_info {
   uint32_t drm_fourcc;
   uint64_t modifier;

   int num_planes;
   int fds[4];
   int offsets[4];
   int strides[4];
};

struct u_gralloc_buffer_color_info {
   enum __DRIYUVColorSpace yuv_color_space;
   enum __DRISampleRange sample_range;
   enum __DRIChromaSiting horizontal_siting;
   enum __DRIChromaSiting vertical_siting;
};

enum u_gralloc_type {
   U_GRALLOC_TYPE_AUTO,
   U_GRALLOC_TYPE_GRALLOC4,
   U_GRALLOC_TYPE_CROS,
   U_GRALLOC_TYPE_LIBDRM,
   U_GRALLOC_TYPE_QCOM,
   U_GRALLOC_TYPE_FALLBACK,
   U_GRALLOC_TYPE_COUNT,
};

struct u_gralloc *u_gralloc_create(enum u_gralloc_type type);

void u_gralloc_destroy(struct u_gralloc **gralloc);

int u_gralloc_get_buffer_basic_info(
   struct u_gralloc *gralloc,
   struct u_gralloc_buffer_handle *hnd,
   struct u_gralloc_buffer_basic_info *out);

int u_gralloc_get_buffer_color_info(
   struct u_gralloc *gralloc,
   struct u_gralloc_buffer_handle *hnd,
   struct u_gralloc_buffer_color_info *out);

int u_gralloc_get_front_rendering_usage(struct u_gralloc *gralloc,
                                        uint64_t *out_usage);

int u_gralloc_get_type(struct u_gralloc *gralloc);

#ifdef __cplusplus
}
#endif

#endif /* U_GRALLOC_H */
