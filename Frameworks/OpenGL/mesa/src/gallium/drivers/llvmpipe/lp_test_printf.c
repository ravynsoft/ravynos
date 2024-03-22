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


#include <stdlib.h>
#include <stdio.h>

#include "util/u_pointer.h"
#include "gallivm/lp_bld.h"
#include "gallivm/lp_bld_init.h"
#include "gallivm/lp_bld_assert.h"
#include "gallivm/lp_bld_printf.h"

#include "lp_test.h"


struct printf_test_case {
   int foo;
};

void
write_tsv_header(FILE *fp)
{
   fprintf(fp,
           "result\t"
           "format\n");

   fflush(fp);
}



typedef void (*test_printf_t)(int i);


static LLVMValueRef
add_printf_test(struct gallivm_state *gallivm)
{
   LLVMModuleRef module = gallivm->module;
   LLVMTypeRef args[1] = { LLVMIntTypeInContext(gallivm->context, 32) };
   LLVMValueRef func = LLVMAddFunction(module, "test_printf", LLVMFunctionType(LLVMVoidTypeInContext(gallivm->context), args, 1, 0));
   LLVMBuilderRef builder = gallivm->builder;
   LLVMBasicBlockRef block = LLVMAppendBasicBlockInContext(gallivm->context, func, "entry");

   LLVMSetFunctionCallConv(func, LLVMCCallConv);

   LLVMPositionBuilderAtEnd(builder, block);
   lp_build_printf(gallivm, "hello, world\n");
   lp_build_printf(gallivm, "print 5 6: %d %d\n", LLVMConstInt(LLVMInt32TypeInContext(gallivm->context), 5, 0),
				LLVMConstInt(LLVMInt32TypeInContext(gallivm->context), 6, 0));

   /* Also test lp_build_assert().  This should not fail. */
   lp_build_assert(gallivm, LLVMConstInt(LLVMInt32TypeInContext(gallivm->context), 1, 0), "assert(1)");

   LLVMBuildRetVoid(builder);

   gallivm_verify_function(gallivm, func);

   return func;
}


UTIL_ALIGN_STACK
static bool
test_printf(unsigned verbose, FILE *fp,
            const struct printf_test_case *testcase)
{
   LLVMContextRef context;
   struct gallivm_state *gallivm;
   LLVMValueRef test;
   test_printf_t test_printf_func;
   bool success = true;

   context = LLVMContextCreate();
#if LLVM_VERSION_MAJOR == 15
   LLVMContextSetOpaquePointers(context, false);
#endif
   gallivm = gallivm_create("test_module", context, NULL);

   test = add_printf_test(gallivm);

   gallivm_compile_module(gallivm);

   test_printf_func = (test_printf_t) gallivm_jit_function(gallivm, test);

   gallivm_free_ir(gallivm);

   test_printf_func(0);

   gallivm_destroy(gallivm);
   LLVMContextDispose(context);

   return success;
}


bool
test_all(unsigned verbose, FILE *fp)
{
   bool success = true;

   test_printf(verbose, fp, NULL);

   return success;
}


bool
test_some(unsigned verbose, FILE *fp,
          unsigned long n)
{
   return test_all(verbose, fp);
}


bool
test_single(unsigned verbose, FILE *fp)
{
   printf("no test_single()");
   return true;
}
