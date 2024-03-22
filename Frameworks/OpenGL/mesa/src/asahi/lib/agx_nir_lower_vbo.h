/*
 * Copyright 2022 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#ifndef __AGX_NIR_LOWER_VBO_H
#define __AGX_NIR_LOWER_VBO_H

#include <stdbool.h>
#include <stdint.h>
#include "util/format/u_formats.h"
#include "nir.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AGX_MAX_ATTRIBS (16)
#define AGX_MAX_VBUFS   (16)

/* See pipe_vertex_element for justification on the sizes. This structure should
 * be small so it can be embedded into a shader key.
 */
struct agx_attribute {
   uint32_t divisor;
   uint32_t stride;
   uint16_t src_offset;
   uint8_t buf;

   /* pipe_format, all vertex formats should be <= 255 */
   uint8_t format;
};

bool agx_nir_lower_vbo(nir_shader *shader, struct agx_attribute *attribs);
bool agx_vbo_supports_format(enum pipe_format format);

#ifdef __cplusplus
} /* extern C */
#endif

#endif
