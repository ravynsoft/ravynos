/**************************************************************************
 *
 * Copyright 2011 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 **************************************************************************/

#ifndef RADEON_UVD_H
#define RADEON_UVD_H

#include "winsys/radeon_winsys.h"
#include "vl/vl_video_buffer.h"

#include "ac_uvd_dec.h"

/* driver dependent callback */
typedef struct pb_buffer_lean *(*ruvd_set_dtb)(struct ruvd_msg *msg, struct vl_video_buffer *vb);

/* create an UVD decode */
struct pipe_video_codec *si_common_uvd_create_decoder(struct pipe_context *context,
                                                      const struct pipe_video_codec *templat,
                                                      ruvd_set_dtb set_dtb);

/* fill decoding target field from the luma and chroma surfaces */
void si_uvd_set_dt_surfaces(struct ruvd_msg *msg, struct radeon_surf *luma,
                            struct radeon_surf *chroma, enum ruvd_surface_type type);
#endif
