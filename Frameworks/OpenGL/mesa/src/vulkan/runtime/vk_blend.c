/*
 * Copyright 2023 Valve Corporation
 * Copyright 2021 Collabora Ltd.
 * SPDX-License-Identifier: MIT
 */

#include "vk_blend.h"
#include "util/macros.h"

enum pipe_logicop
vk_logic_op_to_pipe(VkLogicOp in)
{
   switch (in) {
   case VK_LOGIC_OP_CLEAR:
      return PIPE_LOGICOP_CLEAR;
   case VK_LOGIC_OP_AND:
      return PIPE_LOGICOP_AND;
   case VK_LOGIC_OP_AND_REVERSE:
      return PIPE_LOGICOP_AND_REVERSE;
   case VK_LOGIC_OP_COPY:
      return PIPE_LOGICOP_COPY;
   case VK_LOGIC_OP_AND_INVERTED:
      return PIPE_LOGICOP_AND_INVERTED;
   case VK_LOGIC_OP_NO_OP:
      return PIPE_LOGICOP_NOOP;
   case VK_LOGIC_OP_XOR:
      return PIPE_LOGICOP_XOR;
   case VK_LOGIC_OP_OR:
      return PIPE_LOGICOP_OR;
   case VK_LOGIC_OP_NOR:
      return PIPE_LOGICOP_NOR;
   case VK_LOGIC_OP_EQUIVALENT:
      return PIPE_LOGICOP_EQUIV;
   case VK_LOGIC_OP_INVERT:
      return PIPE_LOGICOP_INVERT;
   case VK_LOGIC_OP_OR_REVERSE:
      return PIPE_LOGICOP_OR_REVERSE;
   case VK_LOGIC_OP_COPY_INVERTED:
      return PIPE_LOGICOP_COPY_INVERTED;
   case VK_LOGIC_OP_OR_INVERTED:
      return PIPE_LOGICOP_OR_INVERTED;
   case VK_LOGIC_OP_NAND:
      return PIPE_LOGICOP_NAND;
   case VK_LOGIC_OP_SET:
      return PIPE_LOGICOP_SET;
   default:
      unreachable("Invalid logicop");
   }
}

enum pipe_blend_func
vk_blend_op_to_pipe(VkBlendOp in)
{
   switch (in) {
   case VK_BLEND_OP_ADD:
      return PIPE_BLEND_ADD;
   case VK_BLEND_OP_SUBTRACT:
      return PIPE_BLEND_SUBTRACT;
   case VK_BLEND_OP_REVERSE_SUBTRACT:
      return PIPE_BLEND_REVERSE_SUBTRACT;
   case VK_BLEND_OP_MIN:
      return PIPE_BLEND_MIN;
   case VK_BLEND_OP_MAX:
      return PIPE_BLEND_MAX;
   default:
      unreachable("Invalid blend op");
   }
}

enum pipe_blendfactor
vk_blend_factor_to_pipe(enum VkBlendFactor vk_factor)
{
   switch (vk_factor) {
   case VK_BLEND_FACTOR_ZERO:
      return PIPE_BLENDFACTOR_ZERO;
   case VK_BLEND_FACTOR_ONE:
      return PIPE_BLENDFACTOR_ONE;
   case VK_BLEND_FACTOR_SRC_COLOR:
      return PIPE_BLENDFACTOR_SRC_COLOR;
   case VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
      return PIPE_BLENDFACTOR_INV_SRC_COLOR;
   case VK_BLEND_FACTOR_DST_COLOR:
      return PIPE_BLENDFACTOR_DST_COLOR;
   case VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR:
      return PIPE_BLENDFACTOR_INV_DST_COLOR;
   case VK_BLEND_FACTOR_SRC_ALPHA:
      return PIPE_BLENDFACTOR_SRC_ALPHA;
   case VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
      return PIPE_BLENDFACTOR_INV_SRC_ALPHA;
   case VK_BLEND_FACTOR_DST_ALPHA:
      return PIPE_BLENDFACTOR_DST_ALPHA;
   case VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
      return PIPE_BLENDFACTOR_INV_DST_ALPHA;
   case VK_BLEND_FACTOR_CONSTANT_COLOR:
      return PIPE_BLENDFACTOR_CONST_COLOR;
   case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR:
      return PIPE_BLENDFACTOR_INV_CONST_COLOR;
   case VK_BLEND_FACTOR_CONSTANT_ALPHA:
      return PIPE_BLENDFACTOR_CONST_ALPHA;
   case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA:
      return PIPE_BLENDFACTOR_INV_CONST_ALPHA;
   case VK_BLEND_FACTOR_SRC1_COLOR:
      return PIPE_BLENDFACTOR_SRC1_COLOR;
   case VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR:
      return PIPE_BLENDFACTOR_INV_SRC1_COLOR;
   case VK_BLEND_FACTOR_SRC1_ALPHA:
      return PIPE_BLENDFACTOR_SRC1_ALPHA;
   case VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA:
      return PIPE_BLENDFACTOR_INV_SRC1_ALPHA;
   case VK_BLEND_FACTOR_SRC_ALPHA_SATURATE:
      return PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE;
   default:
      unreachable("Invalid blend factor");
   }
}
