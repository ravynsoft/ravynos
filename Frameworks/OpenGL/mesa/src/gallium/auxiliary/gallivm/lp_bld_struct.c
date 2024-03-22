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
 * Helper functions for manipulation structures.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 */


#include "util/u_debug.h"
#include "util/u_memory.h"

#include "lp_bld_const.h"
#include "lp_bld_debug.h"
#include "lp_bld_struct.h"

LLVMValueRef
lp_build_struct_get_ptr2(struct gallivm_state *gallivm,
                        LLVMTypeRef ptr_type,
                        LLVMValueRef ptr,
                        unsigned member,
                        const char *name)
{
   LLVMValueRef indices[2];
   LLVMValueRef member_ptr;
   assert(LLVMGetTypeKind(LLVMTypeOf(ptr)) == LLVMPointerTypeKind);
   assert(LLVM_VERSION_MAJOR >= 15 || LLVMGetTypeKind(LLVMGetElementType(LLVMTypeOf(ptr))) == LLVMStructTypeKind);

   indices[0] = lp_build_const_int32(gallivm, 0);
   indices[1] = lp_build_const_int32(gallivm, member);
   member_ptr = LLVMBuildGEP2(gallivm->builder, ptr_type, ptr, indices, ARRAY_SIZE(indices), "");
   lp_build_name(member_ptr, "%s.%s_ptr", LLVMGetValueName(ptr), name);
   return member_ptr;
}

LLVMValueRef
lp_build_struct_get2(struct gallivm_state *gallivm,
                    LLVMTypeRef ptr_type,
                    LLVMValueRef ptr,
                    unsigned member,
                    const char *name)
{
   LLVMValueRef member_ptr;
   LLVMValueRef res;
   assert(LLVMGetTypeKind(LLVMTypeOf(ptr)) == LLVMPointerTypeKind);
   assert(LLVM_VERSION_MAJOR >= 15 || LLVMGetTypeKind(LLVMGetElementType(LLVMTypeOf(ptr))) == LLVMStructTypeKind);
   member_ptr = lp_build_struct_get_ptr2(gallivm, ptr_type, ptr, member, name);
   LLVMTypeRef member_type = LLVMStructGetTypeAtIndex(ptr_type, member);
   res = LLVMBuildLoad2(gallivm->builder, member_type, member_ptr, "");
   lp_build_name(res, "%s.%s", LLVMGetValueName(ptr), name);
   return res;
}

LLVMValueRef
lp_build_array_get_ptr2(struct gallivm_state *gallivm,
                        LLVMTypeRef array_type,
                        LLVMValueRef ptr,
                        LLVMValueRef index)
{
   LLVMValueRef indices[2];
   LLVMValueRef element_ptr;
   assert(LLVMGetTypeKind(LLVMTypeOf(ptr)) == LLVMPointerTypeKind);
   assert(LLVM_VERSION_MAJOR >= 15 || LLVMGetTypeKind(LLVMGetElementType(LLVMTypeOf(ptr))) == LLVMArrayTypeKind);
   indices[0] = lp_build_const_int32(gallivm, 0);
   indices[1] = index;
   element_ptr = LLVMBuildGEP2(gallivm->builder, array_type, ptr, indices, ARRAY_SIZE(indices), "");
#ifdef DEBUG
   lp_build_name(element_ptr, "&%s[%s]", LLVMGetValueName(ptr), LLVMGetValueName(index));
#endif
   return element_ptr;
}


LLVMValueRef
lp_build_array_get2(struct gallivm_state *gallivm,
                    LLVMTypeRef array_type,
                    LLVMValueRef ptr,
                    LLVMValueRef index)
{
   LLVMValueRef element_ptr;
   LLVMValueRef res;
   assert(LLVMGetTypeKind(LLVMTypeOf(ptr)) == LLVMPointerTypeKind);
   assert(LLVM_VERSION_MAJOR >= 15 || LLVMGetTypeKind(LLVMGetElementType(LLVMTypeOf(ptr))) == LLVMArrayTypeKind);
   element_ptr = lp_build_array_get_ptr2(gallivm, array_type, ptr, index);
   LLVMTypeRef element_type = LLVMGetElementType(array_type);
   res = LLVMBuildLoad2(gallivm->builder, element_type, element_ptr, "");
#ifdef DEBUG
   lp_build_name(res, "%s[%s]", LLVMGetValueName(ptr), LLVMGetValueName(index));
#endif
   return res;
}

LLVMValueRef
lp_build_pointer_get_unaligned2(LLVMBuilderRef builder,
                                LLVMTypeRef ptr_type,
                                LLVMValueRef ptr,
                                LLVMValueRef index,
                                unsigned alignment)
{
   LLVMValueRef element_ptr;
   LLVMValueRef res;
   assert(LLVMGetTypeKind(LLVMTypeOf(ptr)) == LLVMPointerTypeKind);
   element_ptr = LLVMBuildGEP2(builder, ptr_type, ptr, &index, 1, "");
   res = LLVMBuildLoad2(builder, ptr_type, element_ptr, "");
   if (alignment)
      LLVMSetAlignment(res, alignment);
#ifdef DEBUG
   lp_build_name(res, "%s[%s]", LLVMGetValueName(ptr), LLVMGetValueName(index));
#endif
   return res;
}


LLVMValueRef
lp_build_pointer_get2(LLVMBuilderRef builder,
                      LLVMTypeRef ptr_type,
                      LLVMValueRef ptr,
                      LLVMValueRef index)
{
  return lp_build_pointer_get_unaligned2(builder, ptr_type, ptr, index, 0);
}

void
lp_build_pointer_set(LLVMBuilderRef builder,
                     LLVMValueRef ptr,
                     LLVMValueRef index,
                     LLVMValueRef value)
{
   LLVMValueRef element_ptr;
   element_ptr = LLVMBuildGEP2(builder, LLVMTypeOf(value), ptr, &index, 1, "");
   LLVMBuildStore(builder, value, element_ptr);
}


void
lp_build_pointer_set_unaligned(LLVMBuilderRef builder,
                               LLVMValueRef ptr,
                               LLVMValueRef index,
                               LLVMValueRef value,
                               unsigned alignment)
{
   LLVMValueRef element_ptr;
   LLVMValueRef instr;
   element_ptr = LLVMBuildGEP2(builder, LLVMTypeOf(value), ptr, &index, 1, "");
   instr = LLVMBuildStore(builder, value, element_ptr);
   LLVMSetAlignment(instr, alignment);
}
