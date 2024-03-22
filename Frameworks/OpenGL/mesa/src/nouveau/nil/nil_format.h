/*
 * Copyright © 2022 Collabora Ltd.
 * SPDX-License-Identifier: MIT
 */
#ifndef NIL_FORMAT_H
#define NIL_FORMAT_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "util/format/u_format.h"

struct nv_device_info;

/* We don't have our own format enum; we use PIPE_FORMAT for everything */

bool nil_format_supports_texturing(struct nv_device_info *dev,
                                   enum pipe_format format);

bool nil_format_supports_filtering(struct nv_device_info *dev,
                                   enum pipe_format format);

bool nil_format_supports_buffer(struct nv_device_info *dev,
                                enum pipe_format format);

bool nil_format_supports_storage(struct nv_device_info *dev,
                                 enum pipe_format format);

bool nil_format_supports_color_targets(struct nv_device_info *dev,
                                       enum pipe_format format);

bool nil_format_supports_blending(struct nv_device_info *dev,
                                  enum pipe_format format);

bool nil_format_supports_depth_stencil(struct nv_device_info *dev,
                                       enum pipe_format format);

uint8_t nil_format_to_color_target(enum pipe_format format);

uint8_t nil_format_to_depth_stencil(enum pipe_format format);

struct nil_tic_format {
   unsigned comp_sizes:8;
   unsigned type_r:3;
   unsigned type_g:3;
   unsigned type_b:3;
   unsigned type_a:3;
   unsigned src_x:3;
   unsigned src_y:3;
   unsigned src_z:3;
   unsigned src_w:3;
};

const struct nil_tic_format *
nil_tic_format_for_pipe(enum pipe_format format);

#endif /* NIL_FORMAT_H */
