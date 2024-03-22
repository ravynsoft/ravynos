/*
 * Copyright 2022 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#ifndef __AGX_NIR_FORMAT_HELPERS_H
#define __AGX_NIR_FORMAT_HELPERS_H

#include "util/format/u_formats.h"
#include "nir_builder.h"
#include "nir_format_convert.h"

static inline nir_def *
nir_sign_extend_if_sint(nir_builder *b, nir_def *x, enum pipe_format format)
{
   if (!util_format_is_pure_sint(format))
      return x;

   const struct util_format_description *desc = util_format_description(format);
   unsigned bits[4] = {0};

   for (unsigned i = 0; i < desc->nr_channels; ++i) {
      assert(desc->channel[i].type == UTIL_FORMAT_TYPE_SIGNED ||
             desc->channel[i].type == UTIL_FORMAT_TYPE_VOID);

      bits[i] = desc->channel[i].size;
   }

   return nir_format_sign_extend_ivec(b, x, bits);
}

#endif
