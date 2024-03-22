/*
 * Copyright 2023 Valve Corporation
 * Copyright 2007 VMware, Inc.
 * SPDX-License-Identifier: MIT
 */

#ifndef UTIL_BLEND_H
#define UTIL_BLEND_H

#include <stdbool.h>

#define PIPE_BLENDFACTOR_INVERT_BIT (0x10)

enum pipe_blendfactor {
   PIPE_BLENDFACTOR_ONE = 1,
   PIPE_BLENDFACTOR_SRC_COLOR,
   PIPE_BLENDFACTOR_SRC_ALPHA,
   PIPE_BLENDFACTOR_DST_ALPHA,
   PIPE_BLENDFACTOR_DST_COLOR,
   PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE,
   PIPE_BLENDFACTOR_CONST_COLOR,
   PIPE_BLENDFACTOR_CONST_ALPHA,
   PIPE_BLENDFACTOR_SRC1_COLOR,
   PIPE_BLENDFACTOR_SRC1_ALPHA,

   PIPE_BLENDFACTOR_ZERO = PIPE_BLENDFACTOR_INVERT_BIT | PIPE_BLENDFACTOR_ONE,
   PIPE_BLENDFACTOR_INV_SRC_COLOR,
   PIPE_BLENDFACTOR_INV_SRC_ALPHA,
   PIPE_BLENDFACTOR_INV_DST_ALPHA,
   PIPE_BLENDFACTOR_INV_DST_COLOR,

   /* Intentionally weird wrapping due to Gallium trace parsing this file */
   PIPE_BLENDFACTOR_INV_CONST_COLOR = PIPE_BLENDFACTOR_INVERT_BIT
                                    | PIPE_BLENDFACTOR_CONST_COLOR,
   PIPE_BLENDFACTOR_INV_CONST_ALPHA,
   PIPE_BLENDFACTOR_INV_SRC1_COLOR,
   PIPE_BLENDFACTOR_INV_SRC1_ALPHA,
};

static inline bool
util_blendfactor_is_inverted(enum pipe_blendfactor factor)
{
   /* By construction of the enum */
   return (factor & PIPE_BLENDFACTOR_INVERT_BIT);
}

static inline enum pipe_blendfactor
util_blendfactor_without_invert(enum pipe_blendfactor factor)
{
   /* By construction of the enum */
   return (enum pipe_blendfactor)(factor & ~PIPE_BLENDFACTOR_INVERT_BIT);
}

enum pipe_blend_func {
   PIPE_BLEND_ADD,
   PIPE_BLEND_SUBTRACT,
   PIPE_BLEND_REVERSE_SUBTRACT,
   PIPE_BLEND_MIN,
   PIPE_BLEND_MAX,
};

enum pipe_logicop {
   PIPE_LOGICOP_CLEAR,
   PIPE_LOGICOP_NOR,
   PIPE_LOGICOP_AND_INVERTED,
   PIPE_LOGICOP_COPY_INVERTED,
   PIPE_LOGICOP_AND_REVERSE,
   PIPE_LOGICOP_INVERT,
   PIPE_LOGICOP_XOR,
   PIPE_LOGICOP_NAND,
   PIPE_LOGICOP_AND,
   PIPE_LOGICOP_EQUIV,
   PIPE_LOGICOP_NOOP,
   PIPE_LOGICOP_OR_INVERTED,
   PIPE_LOGICOP_COPY,
   PIPE_LOGICOP_OR_REVERSE,
   PIPE_LOGICOP_OR,
   PIPE_LOGICOP_SET,
};

/**
 * When faking RGBX render target formats with RGBA ones, the blender is still
 * supposed to treat the destination's alpha channel as 1 instead of the
 * garbage that's there. Return a blend factor that will take that into
 * account.
 */
static inline enum pipe_blendfactor
util_blend_dst_alpha_to_one(enum pipe_blendfactor factor)
{
   switch (factor) {
   case PIPE_BLENDFACTOR_DST_ALPHA:
      return PIPE_BLENDFACTOR_ONE;
   case PIPE_BLENDFACTOR_INV_DST_ALPHA:
      return PIPE_BLENDFACTOR_ZERO;
   default:
      return factor;
   }
}

#endif
