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

/**
 * @file
 * Wrapper for LLVM header file #includes.
 */


#ifndef LP_BLD_H
#define LP_BLD_H


/**
 * @file
 * LLVM IR building helpers interfaces.
 *
 * We use LLVM-C bindings for now. They are not documented, but follow the C++
 * interfaces very closely, and appear to be complete enough for code
 * genration. See
 * http://npcontemplation.blogspot.com/2008/06/secret-of-llvm-c-bindings.html
 * for a standalone example.
 */

#include <llvm/Config/llvm-config.h>

#include <llvm-c/Core.h>  



/**
 * Redefine these LLVM entrypoints as invalid macros to make sure we
 * don't accidentally use them.  We need to use the functions which
 * take an explicit LLVMContextRef parameter.
 */
#define LLVMInt1Type ILLEGAL_LLVM_FUNCTION
#define LLVMInt8Type ILLEGAL_LLVM_FUNCTION
#define LLVMInt16Type ILLEGAL_LLVM_FUNCTION
#define LLVMInt32Type ILLEGAL_LLVM_FUNCTION
#define LLVMInt64Type ILLEGAL_LLVM_FUNCTION
#define LLVMIntType ILLEGAL_LLVM_FUNCTION
#define LLVMFloatType ILLEGAL_LLVM_FUNCTION
#define LLVMDoubleType ILLEGAL_LLVM_FUNCTION
#define LLVMX86FP80Type ILLEGAL_LLVM_FUNCTION
#define LLVMFP128Type ILLEGAL_LLVM_FUNCTION
#define LLVMPPCFP128Type ILLEGAL_LLVM_FUNCTION
#define LLVMStructType ILLEGAL_LLVM_FUNCTION
#define LLVMVoidType ILLEGAL_LLVM_FUNCTION
#define LLVMLabelType ILLEGAL_LLVM_FUNCTION
#define LLVMOpaqueType ILLEGAL_LLVM_FUNCTION
#define LLVMUnionType ILLEGAL_LLVM_FUNCTION
#define LLVMMDString ILLEGAL_LLVM_FUNCTION
#define LLVMMDNode ILLEGAL_LLVM_FUNCTION
#define LLVMConstString ILLEGAL_LLVM_FUNCTION
#define LLVMConstStruct ILLEGAL_LLVM_FUNCTION
#define LLVMAppendBasicBlock ILLEGAL_LLVM_FUNCTION
#define LLVMInsertBasicBlock ILLEGAL_LLVM_FUNCTION
#define LLVMCreateBuilder ILLEGAL_LLVM_FUNCTION

#if LLVM_VERSION_MAJOR >= 15
#define GALLIVM_HAVE_CORO 0
#define GALLIVM_USE_NEW_PASS 1
#elif LLVM_VERSION_MAJOR >= 8
#define GALLIVM_HAVE_CORO 1
#define GALLIVM_USE_NEW_PASS 0
#else
#define GALLIVM_HAVE_CORO 0
#define GALLIVM_USE_NEW_PASS 0
#endif

#define GALLIVM_COROUTINES (GALLIVM_HAVE_CORO || GALLIVM_USE_NEW_PASS)

/* LLVM is transitioning to "opaque pointers", and as such deprecates
 * LLVMBuildGEP, LLVMBuildCall, LLVMBuildLoad, replacing them with
 * LLVMBuildGEP2, LLVMBuildCall2, LLVMBuildLoad2 respectivelly.
 * These new functions were added in LLVM 8.0; so for LLVM before 8.0 we
 * simply forward to the non-opaque-pointer variants.
 */
#if LLVM_VERSION_MAJOR < 8

static inline LLVMValueRef
LLVMBuildGEP2(LLVMBuilderRef B, LLVMTypeRef Ty,
              LLVMValueRef Pointer, LLVMValueRef *Indices,
              unsigned NumIndices, const char *Name)
{
   return LLVMBuildGEP(B, Pointer, Indices, NumIndices, Name);
}

static inline LLVMValueRef
LLVMBuildInBoundsGEP2(LLVMBuilderRef B, LLVMTypeRef Ty,
                      LLVMValueRef Pointer, LLVMValueRef *Indices,
                      unsigned NumIndices, const char *Name)
{
   return LLVMBuildInBoundsGEP(B, Pointer, Indices, NumIndices, Name);
}

static inline LLVMValueRef
LLVMBuildLoad2(LLVMBuilderRef B, LLVMTypeRef Ty,
               LLVMValueRef PointerVal, const char *Name)
{
  LLVMValueRef val = LLVMBuildLoad(B, PointerVal, Name);
  return LLVMTypeOf(val) == Ty ? val : LLVMBuildBitCast(B, val, Ty, Name);
}

static inline LLVMValueRef
LLVMBuildCall2(LLVMBuilderRef B, LLVMTypeRef Ty, LLVMValueRef Fn,
               LLVMValueRef *Args, unsigned NumArgs,
               const char *Name)
{
   return LLVMBuildCall(B, Fn, Args, NumArgs, Name);
}

#endif /* LLVM_VERSION_MAJOR < 8 */

#endif /* LP_BLD_H */
