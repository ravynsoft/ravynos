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

#ifndef LP_BLD_CORO_H
#define LP_BLD_CORO_H

#include <stdbool.h>
#include "util/compiler.h"
#include "gallivm/lp_bld.h"
#include "gallivm/lp_bld_intr.h"

struct gallivm_state;
LLVMValueRef lp_build_coro_id(struct gallivm_state *gallivm);

LLVMValueRef lp_build_coro_size(struct gallivm_state *gallivm);

LLVMValueRef lp_build_coro_begin(struct gallivm_state *gallivm,
                                 LLVMValueRef coro_id, LLVMValueRef mem_ptr);

LLVMValueRef lp_build_coro_free(struct gallivm_state *gallivm,
                                LLVMValueRef coro_id, LLVMValueRef coro_hdl);

void lp_build_coro_end(struct gallivm_state *gallivm,
                       LLVMValueRef coro_hdl);

void lp_build_coro_resume(struct gallivm_state *gallivm, LLVMValueRef coro_hdl);

void lp_build_coro_destroy(struct gallivm_state *gallivm, LLVMValueRef coro_hdl);

LLVMValueRef lp_build_coro_done(struct gallivm_state *gallivm, LLVMValueRef coro_hdl);

LLVMValueRef lp_build_coro_suspend(struct gallivm_state *gallivm, bool last);

LLVMValueRef lp_build_coro_alloc(struct gallivm_state *gallivm, LLVMValueRef id);

LLVMValueRef lp_build_coro_begin_alloc_mem(struct gallivm_state *gallivm, LLVMValueRef coro_id);

LLVMValueRef lp_build_coro_alloc_mem_array(struct gallivm_state *gallivm,
					   LLVMValueRef coro_hdl_ptr, LLVMValueRef coro_idx,
					   LLVMValueRef coro_num_hdls);
void lp_build_coro_free_mem(struct gallivm_state *gallivm, LLVMValueRef coro_id, LLVMValueRef coro_hdl);

struct lp_build_coro_suspend_info {
   LLVMBasicBlockRef suspend;
   LLVMBasicBlockRef cleanup;
};

void lp_build_coro_suspend_switch(struct gallivm_state *gallivm,
                                  const struct lp_build_coro_suspend_info *sus_info,
                                  LLVMBasicBlockRef resume_block,
                                  bool final_suspend);

void lp_build_coro_add_malloc_hooks(struct gallivm_state *gallivm);
void lp_build_coro_declare_malloc_hooks(struct gallivm_state *gallivm);

static inline void lp_build_coro_add_presplit(LLVMValueRef coro)
{
#if LLVM_VERSION_MAJOR >= 15
   lp_add_function_attr(coro, -1, LP_FUNC_ATTR_PRESPLITCORO);
#else
   LLVMAddTargetDependentFunctionAttr(coro, "coroutine.presplit", "0");
#endif
}

#endif
