/*
 * Copyright 2022 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#ifndef AGX_INTERNAL_FORMATS_H
#define AGX_INTERNAL_FORMATS_H

#include <stdbool.h>
#include "util/format/u_formats.h"

/* Define aliases for the subset formats that are accessible in the ISA. These
 * subsets disregard component mapping and number of components. This
 * constitutes ABI with the compiler.
 */
enum agx_internal_formats {
   AGX_INTERNAL_FORMAT_I8 = PIPE_FORMAT_R8_UINT,
   AGX_INTERNAL_FORMAT_I16 = PIPE_FORMAT_R16_UINT,
   AGX_INTERNAL_FORMAT_I32 = PIPE_FORMAT_R32_UINT,
   AGX_INTERNAL_FORMAT_F16 = PIPE_FORMAT_R16_FLOAT,
   AGX_INTERNAL_FORMAT_U8NORM = PIPE_FORMAT_R8_UNORM,
   AGX_INTERNAL_FORMAT_S8NORM = PIPE_FORMAT_R8_SNORM,
   AGX_INTERNAL_FORMAT_U16NORM = PIPE_FORMAT_R16_UNORM,
   AGX_INTERNAL_FORMAT_S16NORM = PIPE_FORMAT_R16_SNORM,
   AGX_INTERNAL_FORMAT_RGB10A2 = PIPE_FORMAT_R10G10B10A2_UNORM,
   AGX_INTERNAL_FORMAT_SRGBA8 = PIPE_FORMAT_R8G8B8A8_SRGB,
   AGX_INTERNAL_FORMAT_RG11B10F = PIPE_FORMAT_R11G11B10_FLOAT,
   AGX_INTERNAL_FORMAT_RGB9E5 = PIPE_FORMAT_R9G9B9E5_FLOAT
};

/*
 * The architecture load/store instructions support masking, but packed formats
 * are not compatible with masking. Check if a format is packed.
 */
static inline bool
agx_internal_format_supports_mask(enum agx_internal_formats format)
{
   switch (format) {
   case AGX_INTERNAL_FORMAT_RGB10A2:
   case AGX_INTERNAL_FORMAT_RG11B10F:
   case AGX_INTERNAL_FORMAT_RGB9E5:
      return false;
   default:
      return true;
   }
}

#endif
