/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2023 Roman Stratiienko (r.stratiienko@gmail.com)
 * SPDX-License-Identifier: MIT
 */

#ifndef U_GRALLOC_INTERNAL_H
#define U_GRALLOC_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "u_gralloc.h"

struct u_gralloc_ops {
   int (*get_buffer_basic_info)(struct u_gralloc *gralloc,
                                struct u_gralloc_buffer_handle *hnd,
                                struct u_gralloc_buffer_basic_info *out);
   int (*get_buffer_color_info)(struct u_gralloc *gralloc,
                                struct u_gralloc_buffer_handle *hnd,
                                struct u_gralloc_buffer_color_info *out);
   int (*get_front_rendering_usage)(struct u_gralloc *gralloc,
                                    uint64_t *out_usage);
   int (*destroy)(struct u_gralloc *gralloc);
};

struct u_gralloc {
   struct u_gralloc_ops ops;
   int type;
};

extern struct u_gralloc *u_gralloc_cros_api_create(void);
#ifdef USE_IMAPPER4_METADATA_API
extern struct u_gralloc *u_gralloc_imapper_api_create(void);
#endif
extern struct u_gralloc *u_gralloc_qcom_create(void);
extern struct u_gralloc *u_gralloc_libdrm_create(void);
extern struct u_gralloc *u_gralloc_fallback_create(void);

/* Helpers for legacy grallocs */
struct android_ycbcr;

bool is_hal_format_yuv(int native);
int get_hal_format_bpp(int native);
int get_fourcc_from_hal_format(int native);
int bufferinfo_from_ycbcr(const struct android_ycbcr *ycbcr,
                          struct u_gralloc_buffer_handle *hnd,
                          struct u_gralloc_buffer_basic_info *out);

#ifdef __cplusplus
}
#endif

#endif /* U_GRALLOC_INTERNAL_H */
