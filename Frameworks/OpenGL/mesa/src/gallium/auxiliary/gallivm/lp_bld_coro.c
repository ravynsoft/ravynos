/**************************************************************************
 *
 * Copyright 2019 Red Hat.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **************************************************************************/

#include <stdint.h>
#include "lp_bld_coro.h"
#include "util/os_memory.h"
#include "lp_bld_init.h"
#include "lp_bld_const.h"
#include "lp_bld_intr.h"
#include "lp_bld_flow.h"

#if LLVM_VERSION_MAJOR < 6
/* not a wrapper, just lets it compile */
static LLVMTypeRef LLVMTokenTypeInContext(LLVMContextRef C)
{
   assert(0);
   return LLVMVoidTypeInContext(C);
}
#endif

LLVMValueRef lp_build_coro_id(struct gallivm_state *gallivm)
{
   LLVMValueRef coro_id_args[4];
   coro_id_args[0] = lp_build_const_int32(gallivm, 0);
   coro_id_args[1] = LLVMConstPointerNull(LLVMPointerType(LLVMInt8TypeInContext(gallivm->context), 0));
   coro_id_args[2] = coro_id_args[1];
   coro_id_args[3] = coro_id_args[1];
   LLVMValueRef coro_id = lp_build_intrinsic(gallivm->builder,
                                             "llvm.coro.id",
                                             LLVMTokenTypeInContext(gallivm->context),
                                             coro_id_args, 4, 0);
   return coro_id;
}

LLVMValueRef lp_build_coro_size(struct gallivm_state *gallivm)
{
   return lp_build_intrinsic(gallivm->builder,
                             "llvm.coro.size.i32",
                             LLVMInt32TypeInContext(gallivm->context),
                             NULL, 0, 0);
}

LLVMValueRef lp_build_coro_begin(struct gallivm_state *gallivm,
                                 LLVMValueRef coro_id, LLVMValueRef mem_ptr)
{
   LLVMValueRef coro_begin_args[2];
   coro_begin_args[0] = coro_id;
   coro_begin_args[1] = mem_ptr;
   LLVMValueRef coro_hdl = lp_build_intrinsic(gallivm->builder,
                                              "llvm.coro.begin",
                                              LLVMPointerType(LLVMInt8TypeInContext(gallivm->context), 0),
                                              coro_begin_args, 2, 0);
   return coro_hdl;
}

LLVMValueRef lp_build_coro_free(struct gallivm_state *gallivm,
                                LLVMValueRef coro_id, LLVMValueRef coro_hdl)
{
   LLVMValueRef coro_free_args[2];
   coro_free_args[0] = coro_id;
   coro_free_args[1] = coro_hdl;
   return lp_build_intrinsic(gallivm->builder,
                             "llvm.coro.free",
                             LLVMPointerType(LLVMInt8TypeInContext(gallivm->context), 0),
                             coro_free_args, 2, 0);
}

void lp_build_coro_end(struct gallivm_state *gallivm, LLVMValueRef coro_hdl)
{
   LLVMValueRef coro_end_args[2];
   coro_end_args[0] = coro_hdl;
   coro_end_args[1] = LLVMConstInt(LLVMInt1TypeInContext(gallivm->context), 0, 0);
   lp_build_intrinsic(gallivm->builder,
                      "llvm.coro.end",
                      LLVMInt1TypeInContext(gallivm->context),
                      coro_end_args, 2, 0);
}

void lp_build_coro_resume(struct gallivm_state *gallivm, LLVMValueRef coro_hdl)
{
   lp_build_intrinsic(gallivm->builder,
                      "llvm.coro.resume",
                      LLVMVoidTypeInContext(gallivm->context),
                      &coro_hdl, 1, 0);
}

void lp_build_coro_destroy(struct gallivm_state *gallivm, LLVMValueRef coro_hdl)
{
   lp_build_intrinsic(gallivm->builder,
                      "llvm.coro.destroy",
                      LLVMVoidTypeInContext(gallivm->context),
                      &coro_hdl, 1, 0);
}

LLVMValueRef lp_build_coro_done(struct gallivm_state *gallivm, LLVMValueRef coro_hdl)
{
   return lp_build_intrinsic(gallivm->builder,
                             "llvm.coro.done",
                             LLVMInt1TypeInContext(gallivm->context),
                             &coro_hdl, 1, 0);
}

LLVMValueRef lp_build_coro_suspend(struct gallivm_state *gallivm, bool last)
{
   LLVMValueRef coro_susp_args[2];
   coro_susp_args[0] = LLVMConstNull(LLVMTokenTypeInContext(gallivm->context));
   coro_susp_args[1] = LLVMConstInt(LLVMInt1TypeInContext(gallivm->context), last, 0);
   LLVMValueRef coro_suspend = lp_build_intrinsic(gallivm->builder,
                                                  "llvm.coro.suspend",
                                                  LLVMInt8TypeInContext(gallivm->context),
                                                  coro_susp_args, 2, 0);
   return coro_suspend;
}

LLVMValueRef lp_build_coro_alloc(struct gallivm_state *gallivm, LLVMValueRef id)
{
   return lp_build_intrinsic(gallivm->builder,
                             "llvm.coro.alloc",
                             LLVMInt1TypeInContext(gallivm->context),
                             &id, 1, 0);
}

static char *
coro_malloc(int size)
{
   return os_malloc_aligned(size, 4096);
}

static void
coro_free(char *ptr)
{
   os_free_aligned(ptr);
}

void lp_build_coro_add_malloc_hooks(struct gallivm_state *gallivm)
{
   assert(gallivm->engine);

   assert(gallivm->coro_malloc_hook);
   assert(gallivm->coro_free_hook);
   LLVMAddGlobalMapping(gallivm->engine, gallivm->coro_malloc_hook, coro_malloc);
   LLVMAddGlobalMapping(gallivm->engine, gallivm->coro_free_hook, coro_free);
}

void lp_build_coro_declare_malloc_hooks(struct gallivm_state *gallivm)
{
   LLVMTypeRef int32_type = LLVMInt32TypeInContext(gallivm->context);
   LLVMTypeRef mem_ptr_type = LLVMPointerType(LLVMInt8TypeInContext(gallivm->context), 0);
   LLVMTypeRef malloc_type = LLVMFunctionType(mem_ptr_type, &int32_type, 1, 0);
   gallivm->coro_malloc_hook_type = malloc_type;
   gallivm->coro_malloc_hook = LLVMAddFunction(gallivm->module, "coro_malloc", malloc_type);
   LLVMTypeRef free_type = LLVMFunctionType(LLVMVoidTypeInContext(gallivm->context), &mem_ptr_type, 1, 0);
   gallivm->coro_free_hook_type = free_type;
   gallivm->coro_free_hook = LLVMAddFunction(gallivm->module, "coro_free", free_type);
}

LLVMValueRef lp_build_coro_begin_alloc_mem(struct gallivm_state *gallivm, LLVMValueRef coro_id)
{
   LLVMTypeRef mem_ptr_type = LLVMPointerType(LLVMInt8TypeInContext(gallivm->context), 0);
   LLVMValueRef do_alloc = lp_build_coro_alloc(gallivm, coro_id);
   struct lp_build_if_state if_state_coro;
   lp_build_if(&if_state_coro, gallivm, do_alloc);
   LLVMValueRef coro_size = lp_build_coro_size(gallivm);
   LLVMValueRef alloc_mem;

   assert(gallivm->coro_malloc_hook);
   LLVMTypeRef malloc_type =
         LLVMFunctionType(mem_ptr_type,
                          (LLVMTypeRef[]){LLVMInt32TypeInContext(gallivm->context)}, 1, 0);
   alloc_mem = LLVMBuildCall2(gallivm->builder, malloc_type, gallivm->coro_malloc_hook, &coro_size, 1, "");
   lp_build_endif(&if_state_coro);

   LLVMValueRef phi = LLVMBuildPhi(gallivm->builder, mem_ptr_type, "");
   LLVMValueRef zero_bool = LLVMConstNull(mem_ptr_type);
   LLVMAddIncoming(phi, &alloc_mem, &if_state_coro.true_block, 1);
   LLVMAddIncoming(phi, &zero_bool, &if_state_coro.entry_block, 1);

   LLVMValueRef coro_hdl = lp_build_coro_begin(gallivm, coro_id, phi);
   return coro_hdl;
}

LLVMValueRef lp_build_coro_alloc_mem_array(struct gallivm_state *gallivm,
					   LLVMValueRef coro_hdl_ptr, LLVMValueRef coro_idx,
					   LLVMValueRef coro_num_hdls)
{
   LLVMTypeRef mem_ptr_type = LLVMPointerType(LLVMInt8TypeInContext(gallivm->context), 0);
   LLVMValueRef alloced_ptr = LLVMBuildLoad2(gallivm->builder, mem_ptr_type, coro_hdl_ptr, "");

   LLVMValueRef not_alloced = LLVMBuildICmp(gallivm->builder, LLVMIntEQ, alloced_ptr, LLVMConstNull(mem_ptr_type), "");
   LLVMValueRef coro_size = lp_build_coro_size(gallivm);

   struct lp_build_if_state if_state_coro;
   lp_build_if(&if_state_coro, gallivm, not_alloced);

   LLVMValueRef alloc_mem;
   LLVMValueRef alloc_size = LLVMBuildMul(gallivm->builder, coro_num_hdls, coro_size, "");
   assert(gallivm->coro_malloc_hook);
   assert(gallivm->coro_malloc_hook_type);
   alloc_mem = LLVMBuildCall2(gallivm->builder, gallivm->coro_malloc_hook_type, gallivm->coro_malloc_hook, &alloc_size, 1, "");
   LLVMBuildStore(gallivm->builder, alloc_mem, coro_hdl_ptr);
   lp_build_endif(&if_state_coro);

   return LLVMBuildMul(gallivm->builder, coro_size, coro_idx, "");
}

void lp_build_coro_free_mem(struct gallivm_state *gallivm, LLVMValueRef coro_id, LLVMValueRef coro_hdl)
{
   LLVMValueRef alloc_mem = lp_build_coro_free(gallivm, coro_id, coro_hdl);

   assert(gallivm->coro_free_hook);
   assert(gallivm->coro_free_hook_type);
   alloc_mem = LLVMBuildCall2(gallivm->builder, gallivm->coro_free_hook_type, gallivm->coro_free_hook, &alloc_mem, 1, "");
}

void lp_build_coro_suspend_switch(struct gallivm_state *gallivm, const struct lp_build_coro_suspend_info *sus_info,
                                  LLVMBasicBlockRef resume_block, bool final_suspend)
{
   LLVMValueRef coro_suspend = lp_build_coro_suspend(gallivm, final_suspend);
   LLVMValueRef myswitch = LLVMBuildSwitch(gallivm->builder, coro_suspend,
                                           sus_info->suspend, resume_block ? 2 : 1);
   LLVMAddCase(myswitch, LLVMConstInt(LLVMInt8TypeInContext(gallivm->context), 1, 0), sus_info->cleanup);
   if (resume_block)
      LLVMAddCase(myswitch, LLVMConstInt(LLVMInt8TypeInContext(gallivm->context), 0, 0), resume_block);
}
