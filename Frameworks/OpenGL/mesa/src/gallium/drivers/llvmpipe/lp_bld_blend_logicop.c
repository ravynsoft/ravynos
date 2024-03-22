/**************************************************************************
 *
 * Copyright 2009 VMware, Inc.
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


/**
 * @file
 * Blend LLVM IR generation -- logic ops.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 */


#include "pipe/p_state.h"
#include "util/u_debug.h"

#include "lp_bld_blend.h"


LLVMValueRef
lp_build_logicop(LLVMBuilderRef builder,
                 enum pipe_logicop logicop_func,
                 LLVMValueRef src,
                 LLVMValueRef dst)
{
   LLVMTypeRef type;
   LLVMValueRef res;

   type = LLVMTypeOf(src);

   switch (logicop_func) {
   case PIPE_LOGICOP_CLEAR:
      res = LLVMConstNull(type);
      break;
   case PIPE_LOGICOP_NOR:
      res = LLVMBuildNot(builder, LLVMBuildOr(builder, src, dst, ""), "");
      break;
   case PIPE_LOGICOP_AND_INVERTED:
      res = LLVMBuildAnd(builder, LLVMBuildNot(builder, src, ""), dst, "");
      break;
   case PIPE_LOGICOP_COPY_INVERTED:
      res = LLVMBuildNot(builder, src, "");
      break;
   case PIPE_LOGICOP_AND_REVERSE:
      res = LLVMBuildAnd(builder, src, LLVMBuildNot(builder, dst, ""), "");
      break;
   case PIPE_LOGICOP_INVERT:
      res = LLVMBuildNot(builder, dst, "");
      break;
   case PIPE_LOGICOP_XOR:
      res = LLVMBuildXor(builder, src, dst, "");
      break;
   case PIPE_LOGICOP_NAND:
      res = LLVMBuildNot(builder, LLVMBuildAnd(builder, src, dst, ""), "");
      break;
   case PIPE_LOGICOP_AND:
      res = LLVMBuildAnd(builder, src, dst, "");
      break;
   case PIPE_LOGICOP_EQUIV:
      res = LLVMBuildNot(builder, LLVMBuildXor(builder, src, dst, ""), "");
      break;
   case PIPE_LOGICOP_NOOP:
      res = dst;
      break;
   case PIPE_LOGICOP_OR_INVERTED:
      res = LLVMBuildOr(builder, LLVMBuildNot(builder, src, ""), dst, "");
      break;
   case PIPE_LOGICOP_COPY:
      res = src;
      break;
   case PIPE_LOGICOP_OR_REVERSE:
      res = LLVMBuildOr(builder, src, LLVMBuildNot(builder, dst, ""), "");
      break;
   case PIPE_LOGICOP_OR:
      res = LLVMBuildOr(builder, src, dst, "");
      break;
   case PIPE_LOGICOP_SET:
      res = LLVMConstAllOnes(type);
      break;
   default:
      assert(0);
      res = src;
   }

   return res;
}
