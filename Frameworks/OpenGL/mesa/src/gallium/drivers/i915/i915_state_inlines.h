/**************************************************************************
 *
 * Copyright 2003 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef I915_STATE_INLINES_H
#define I915_STATE_INLINES_H

#include "pipe/p_defines.h"
#include "util/compiler.h"
#include "util/u_debug.h"
#include "i915_reg.h"

static inline unsigned
i915_translate_compare_func(unsigned func)
{
   switch (func) {
   case PIPE_FUNC_NEVER:
      return COMPAREFUNC_NEVER;
   case PIPE_FUNC_LESS:
      return COMPAREFUNC_LESS;
   case PIPE_FUNC_LEQUAL:
      return COMPAREFUNC_LEQUAL;
   case PIPE_FUNC_GREATER:
      return COMPAREFUNC_GREATER;
   case PIPE_FUNC_GEQUAL:
      return COMPAREFUNC_GEQUAL;
   case PIPE_FUNC_NOTEQUAL:
      return COMPAREFUNC_NOTEQUAL;
   case PIPE_FUNC_EQUAL:
      return COMPAREFUNC_EQUAL;
   case PIPE_FUNC_ALWAYS:
      return COMPAREFUNC_ALWAYS;
   default:
      return COMPAREFUNC_ALWAYS;
   }
}

static inline unsigned
i915_translate_shadow_compare_func(unsigned func)
{
   switch (func) {
   case PIPE_FUNC_NEVER:
      return COMPAREFUNC_ALWAYS;
   case PIPE_FUNC_LESS:
      return COMPAREFUNC_LEQUAL;
   case PIPE_FUNC_LEQUAL:
      return COMPAREFUNC_LESS;
   case PIPE_FUNC_GREATER:
      return COMPAREFUNC_GEQUAL;
   case PIPE_FUNC_GEQUAL:
      return COMPAREFUNC_GREATER;
   case PIPE_FUNC_NOTEQUAL:
      return COMPAREFUNC_EQUAL;
   case PIPE_FUNC_EQUAL:
      return COMPAREFUNC_NOTEQUAL;
   case PIPE_FUNC_ALWAYS:
      return COMPAREFUNC_NEVER;
   default:
      return COMPAREFUNC_NEVER;
   }
}

static inline unsigned
i915_translate_stencil_op(unsigned op)
{
   switch (op) {
   case PIPE_STENCIL_OP_KEEP:
      return STENCILOP_KEEP;
   case PIPE_STENCIL_OP_ZERO:
      return STENCILOP_ZERO;
   case PIPE_STENCIL_OP_REPLACE:
      return STENCILOP_REPLACE;
   case PIPE_STENCIL_OP_INCR:
      return STENCILOP_INCRSAT;
   case PIPE_STENCIL_OP_DECR:
      return STENCILOP_DECRSAT;
   case PIPE_STENCIL_OP_INCR_WRAP:
      return STENCILOP_INCR;
   case PIPE_STENCIL_OP_DECR_WRAP:
      return STENCILOP_DECR;
   case PIPE_STENCIL_OP_INVERT:
      return STENCILOP_INVERT;
   default:
      return STENCILOP_ZERO;
   }
}

static inline unsigned
i915_translate_blend_factor(unsigned factor)
{
   switch (factor) {
   case PIPE_BLENDFACTOR_ZERO:
      return BLENDFACT_ZERO;
   case PIPE_BLENDFACTOR_SRC_ALPHA:
      return BLENDFACT_SRC_ALPHA;
   case PIPE_BLENDFACTOR_ONE:
      return BLENDFACT_ONE;
   case PIPE_BLENDFACTOR_SRC_COLOR:
      return BLENDFACT_SRC_COLR;
   case PIPE_BLENDFACTOR_INV_SRC_COLOR:
      return BLENDFACT_INV_SRC_COLR;
   case PIPE_BLENDFACTOR_DST_COLOR:
      return BLENDFACT_DST_COLR;
   case PIPE_BLENDFACTOR_INV_DST_COLOR:
      return BLENDFACT_INV_DST_COLR;
   case PIPE_BLENDFACTOR_INV_SRC_ALPHA:
      return BLENDFACT_INV_SRC_ALPHA;
   case PIPE_BLENDFACTOR_DST_ALPHA:
      return BLENDFACT_DST_ALPHA;
   case PIPE_BLENDFACTOR_INV_DST_ALPHA:
      return BLENDFACT_INV_DST_ALPHA;
   case PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE:
      return BLENDFACT_SRC_ALPHA_SATURATE;
   case PIPE_BLENDFACTOR_CONST_COLOR:
      return BLENDFACT_CONST_COLOR;
   case PIPE_BLENDFACTOR_INV_CONST_COLOR:
      return BLENDFACT_INV_CONST_COLOR;
   case PIPE_BLENDFACTOR_CONST_ALPHA:
      return BLENDFACT_CONST_ALPHA;
   case PIPE_BLENDFACTOR_INV_CONST_ALPHA:
      return BLENDFACT_INV_CONST_ALPHA;
   default:
      return BLENDFACT_ZERO;
   }
}

static inline unsigned
i915_translate_blend_func(unsigned mode)
{
   switch (mode) {
   case PIPE_BLEND_ADD:
      return BLENDFUNC_ADD;
   case PIPE_BLEND_MIN:
      return BLENDFUNC_MIN;
   case PIPE_BLEND_MAX:
      return BLENDFUNC_MAX;
   case PIPE_BLEND_SUBTRACT:
      return BLENDFUNC_SUBTRACT;
   case PIPE_BLEND_REVERSE_SUBTRACT:
      return BLENDFUNC_REVERSE_SUBTRACT;
   default:
      return 0;
   }
}

static inline unsigned
i915_translate_logic_op(unsigned opcode)
{
   switch (opcode) {
   case PIPE_LOGICOP_CLEAR:
      return LOGICOP_CLEAR;
   case PIPE_LOGICOP_AND:
      return LOGICOP_AND;
   case PIPE_LOGICOP_AND_REVERSE:
      return LOGICOP_AND_RVRSE;
   case PIPE_LOGICOP_COPY:
      return LOGICOP_COPY;
   case PIPE_LOGICOP_COPY_INVERTED:
      return LOGICOP_COPY_INV;
   case PIPE_LOGICOP_AND_INVERTED:
      return LOGICOP_AND_INV;
   case PIPE_LOGICOP_NOOP:
      return LOGICOP_NOOP;
   case PIPE_LOGICOP_XOR:
      return LOGICOP_XOR;
   case PIPE_LOGICOP_OR:
      return LOGICOP_OR;
   case PIPE_LOGICOP_OR_INVERTED:
      return LOGICOP_OR_INV;
   case PIPE_LOGICOP_NOR:
      return LOGICOP_NOR;
   case PIPE_LOGICOP_EQUIV:
      return LOGICOP_EQUIV;
   case PIPE_LOGICOP_INVERT:
      return LOGICOP_INV;
   case PIPE_LOGICOP_OR_REVERSE:
      return LOGICOP_OR_RVRSE;
   case PIPE_LOGICOP_NAND:
      return LOGICOP_NAND;
   case PIPE_LOGICOP_SET:
      return LOGICOP_SET;
   default:
      return LOGICOP_SET;
   }
}

static inline bool
i915_validate_vertices(unsigned hw_prim, unsigned nr)
{
   bool ok;

   switch (hw_prim) {
   case PRIM3D_POINTLIST:
      ok = (nr >= 1);
      assert(ok);
      break;
   case PRIM3D_LINELIST:
      ok = (nr >= 2) && (nr % 2) == 0;
      assert(ok);
      break;
   case PRIM3D_LINESTRIP:
      ok = (nr >= 2);
      assert(ok);
      break;
   case PRIM3D_TRILIST:
      ok = (nr >= 3) && (nr % 3) == 0;
      assert(ok);
      break;
   case PRIM3D_TRISTRIP:
      ok = (nr >= 3);
      assert(ok);
      break;
   case PRIM3D_TRIFAN:
      ok = (nr >= 3);
      assert(ok);
      break;
   case PRIM3D_POLY:
      ok = (nr >= 3);
      assert(ok);
      break;
   default:
      assert(0);
      ok = 0;
      break;
   }

   return ok;
}

#endif
