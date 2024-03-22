/**************************************************************************
 *
 * Copyright 2010 VMware, Inc.
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

#include <stdio.h>
#include <inttypes.h>

#include "util/compiler.h"
#include "util/u_debug.h"
#include "util/u_memory.h"
#include "util/u_string.h"
#include "lp_bld_const.h"
#include "lp_bld_init.h"
#include "lp_bld_const.h"
#include "lp_bld_printf.h"
#include "lp_bld_type.h"

void lp_init_printf_hook(struct gallivm_state *gallivm)
{
   if (gallivm->debug_printf_hook)
      return;
   LLVMTypeRef printf_type = LLVMFunctionType(LLVMInt32TypeInContext(gallivm->context), NULL, 0, 1);
   gallivm->debug_printf_hook = LLVMAddFunction(gallivm->module, "debug_printf", printf_type);
}

/**
 * Generates LLVM IR to call debug_printf.
 */
static LLVMValueRef
lp_build_print_args(struct gallivm_state* gallivm,
                    int argcount,
                    LLVMValueRef* args)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMContextRef context = gallivm->context;
   int i;

   assert(args);
   assert(argcount > 0);
   assert(LLVMTypeOf(args[0]) == LLVMPointerType(LLVMInt8TypeInContext(context), 0));

   /* Cast any float arguments to doubles as printf expects */
   for (i = 1; i < argcount; i++) {
      LLVMTypeRef type = LLVMTypeOf(args[i]);

      if (LLVMGetTypeKind(type) == LLVMFloatTypeKind)
         args[i] = LLVMBuildFPExt(builder, args[i], LLVMDoubleTypeInContext(context), "");
   }

   lp_init_printf_hook(gallivm);
   LLVMTypeRef printf_type = LLVMFunctionType(LLVMInt32TypeInContext(gallivm->context), NULL, 0, 1);
   return LLVMBuildCall2(builder, printf_type, gallivm->debug_printf_hook, args, argcount, "");
}


/**
 * Print a LLVM value of any type
 */
LLVMValueRef
lp_build_print_value(struct gallivm_state *gallivm,
                     const char *msg,
                     LLVMValueRef value)
{
   LLVMBuilderRef builder = gallivm->builder;
   LLVMTypeKind type_kind;
   LLVMTypeRef type_ref;
   LLVMValueRef params[2 + LP_MAX_VECTOR_LENGTH];
   char type_fmt[6] = " %x";
   char format[2 + 5 * LP_MAX_VECTOR_LENGTH + 2] = "%s";
   unsigned length;
   unsigned i;

   type_ref = LLVMTypeOf(value);
   type_kind = LLVMGetTypeKind(type_ref);

   if (type_kind == LLVMVectorTypeKind) {
      length = LLVMGetVectorSize(type_ref);

      type_ref = LLVMGetElementType(type_ref);
      type_kind = LLVMGetTypeKind(type_ref);
   } else {
      length = 1;
   }

   if (type_kind == LLVMFloatTypeKind || type_kind == LLVMDoubleTypeKind || type_kind == LLVMHalfTypeKind) {
      type_fmt[2] = '.';
      type_fmt[3] = '9';
      type_fmt[4] = 'g';
      type_fmt[5] = '\0';
   } else if (type_kind == LLVMIntegerTypeKind) {
      if (LLVMGetIntTypeWidth(type_ref) == 64) {
         snprintf(type_fmt + 2, 3, "%s", PRId64);
      } else if (LLVMGetIntTypeWidth(type_ref) == 8) {
         type_fmt[2] = 'u';
      } else {
         type_fmt[2] = 'i';
      }
   } else if (type_kind == LLVMPointerTypeKind) {
      type_fmt[2] = 'p';
   } else {
      /* Unsupported type */
      assert(0);
   }

   /* Create format string and arguments */
   assert(strlen(format) + strlen(type_fmt) * length + 2 <= sizeof format);

   params[1] = lp_build_const_string(gallivm, msg);
   if (length == 1) {
      strncat(format, type_fmt, sizeof(format) - strlen(format) - 1);
      params[2] = value;
   } else {
      for (i = 0; i < length; ++i) {
         LLVMValueRef param;
         strncat(format, type_fmt, sizeof(format) - strlen(format) - 1);
         param = LLVMBuildExtractElement(builder, value, lp_build_const_int32(gallivm, i), "");
         if (type_kind == LLVMIntegerTypeKind &&
             LLVMGetIntTypeWidth(type_ref) < sizeof(int) * 8) {
            LLVMTypeRef int_type = LLVMIntTypeInContext(gallivm->context, sizeof(int) * 8);
            if (LLVMGetIntTypeWidth(type_ref) == 8) {
               param = LLVMBuildZExt(builder, param, int_type, "");
            } else {
               param = LLVMBuildSExt(builder, param, int_type, "");
            }
         }
         params[2 + i] = param;
      }
   }

   strncat(format, "\n", sizeof(format) - strlen(format) - 1);

   params[0] = lp_build_const_string(gallivm, format);
   return lp_build_print_args(gallivm, 2 + length, params);
}


static unsigned
lp_get_printf_arg_count(const char *fmt)
{
   unsigned count = 0;
   const char *p = fmt;
   int c;

   while ((c = *p++)) {
      if (c != '%')
         continue;
      switch (*p) {
         case '\0':
       continue;
         case '%':
       p++;
       continue;
    case '.':
       if (p[1] == '*' && p[2] == 's') {
          count += 2;
          p += 3;
               continue;
       }
       FALLTHROUGH;
    default:
       count ++;
      }
   }
   return count;
}


/**
 * Generate LLVM IR for a c style printf
 */
LLVMValueRef
lp_build_printf(struct gallivm_state *gallivm,
                const char *fmt, ...)
{
   LLVMValueRef params[50];
   va_list arglist;
   unsigned argcount, i;

   argcount = lp_get_printf_arg_count(fmt);
   assert(ARRAY_SIZE(params) >= argcount + 1);

   va_start(arglist, fmt);
   for (i = 1; i <= argcount; i++) {
      params[i] = va_arg(arglist, LLVMValueRef);
   }
   va_end(arglist);

   params[0] = lp_build_const_string(gallivm, fmt);
   return lp_build_print_args(gallivm, argcount + 1, params);
}
