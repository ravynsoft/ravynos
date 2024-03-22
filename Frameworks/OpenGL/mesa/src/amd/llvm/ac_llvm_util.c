/*
 * Copyright 2014 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */
/* based on pieces from si_pipe.c and radeon_llvm_emit.c */
#include "ac_llvm_util.h"

#include "ac_llvm_build.h"
#include "c11/threads.h"
#include "util/bitscan.h"
#include "util/u_math.h"
#include <llvm-c/Core.h>
#include <llvm-c/Support.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void ac_init_llvm_target(void)
{
   LLVMInitializeAMDGPUTargetInfo();
   LLVMInitializeAMDGPUTarget();
   LLVMInitializeAMDGPUTargetMC();
   LLVMInitializeAMDGPUAsmPrinter();

   /* For inline assembly. */
   LLVMInitializeAMDGPUAsmParser();

   /* For ACO disassembly. */
   LLVMInitializeAMDGPUDisassembler();

   const char *argv[] = {
      /* error messages prefix */
      "mesa",
      "-amdgpu-atomic-optimizations=true",
   };

   ac_reset_llvm_all_options_occurrences();
   LLVMParseCommandLineOptions(ARRAY_SIZE(argv), argv, NULL);

   ac_llvm_run_atexit_for_destructors();
}

PUBLIC void ac_init_shared_llvm_once(void)
{
   static once_flag ac_init_llvm_target_once_flag = ONCE_FLAG_INIT;
   call_once(&ac_init_llvm_target_once_flag, ac_init_llvm_target);
}

#if !LLVM_IS_SHARED
static once_flag ac_init_static_llvm_target_once_flag = ONCE_FLAG_INIT;
static void ac_init_static_llvm_once(void)
{
   call_once(&ac_init_static_llvm_target_once_flag, ac_init_llvm_target);
}
#endif

void ac_init_llvm_once(void)
{
#if LLVM_IS_SHARED
   ac_init_shared_llvm_once();
#else
   ac_init_static_llvm_once();
#endif
}

LLVMTargetRef ac_get_llvm_target(const char *triple)
{
   LLVMTargetRef target = NULL;
   char *err_message = NULL;

   if (LLVMGetTargetFromTriple(triple, &target, &err_message)) {
      fprintf(stderr, "Cannot find target for triple %s ", triple);
      if (err_message) {
         fprintf(stderr, "%s\n", err_message);
      }
      LLVMDisposeMessage(err_message);
      return NULL;
   }
   return target;
}

static LLVMTargetMachineRef ac_create_target_machine(enum radeon_family family,
                                                     enum ac_target_machine_options tm_options,
                                                     LLVMCodeGenOptLevel level,
                                                     const char **out_triple)
{
   assert(family >= CHIP_TAHITI);
   const char *triple = (tm_options & AC_TM_SUPPORTS_SPILL) ? "amdgcn-mesa-mesa3d" : "amdgcn--";
   LLVMTargetRef target = ac_get_llvm_target(triple);
   const char *name = ac_get_llvm_processor_name(family);

   LLVMTargetMachineRef tm =
      LLVMCreateTargetMachine(target, triple, name, "", level,
                              LLVMRelocDefault, LLVMCodeModelDefault);

   if (!ac_is_llvm_processor_supported(tm, name)) {
      LLVMDisposeTargetMachine(tm);
      fprintf(stderr, "amd: LLVM doesn't support %s, bailing out...\n", name);
      return NULL;
   }

   if (out_triple)
      *out_triple = triple;

   return tm;
}

LLVMAttributeRef ac_get_llvm_attribute(LLVMContextRef ctx, const char *str)
{
   return LLVMCreateEnumAttribute(ctx, LLVMGetEnumAttributeKindForName(str, strlen(str)), 0);
}

void ac_add_function_attr(LLVMContextRef ctx, LLVMValueRef function, int attr_idx,
                          const char *attr)
{
   assert(LLVMIsAFunction(function));
   LLVMAddAttributeAtIndex(function, attr_idx, ac_get_llvm_attribute(ctx, attr));
}

void ac_dump_module(LLVMModuleRef module)
{
   char *str = LLVMPrintModuleToString(module);
   fprintf(stderr, "%s", str);
   LLVMDisposeMessage(str);
}

void ac_llvm_add_target_dep_function_attr(LLVMValueRef F, const char *name, unsigned value)
{
   char str[16];

   snprintf(str, sizeof(str), "0x%x", value);
   LLVMAddTargetDependentFunctionAttr(F, name, str);
}

void ac_llvm_set_workgroup_size(LLVMValueRef F, unsigned size)
{
   if (!size)
      return;

   char str[32];
   snprintf(str, sizeof(str), "%u,%u", size, size);
   LLVMAddTargetDependentFunctionAttr(F, "amdgpu-flat-work-group-size", str);
}

void ac_llvm_set_target_features(LLVMValueRef F, struct ac_llvm_context *ctx, bool wgp_mode)
{
   char features[2048];

   snprintf(features, sizeof(features), "+DumpCode%s%s%s",
            /* GFX9 has broken VGPR indexing, so always promote alloca to scratch. */
            ctx->gfx_level == GFX9 ? ",-promote-alloca" : "",
            /* Wave32 is the default. */
            ctx->gfx_level >= GFX10 && ctx->wave_size == 64 ?
               ",+wavefrontsize64,-wavefrontsize32" : "",
            ctx->gfx_level >= GFX10 && !wgp_mode ? ",+cumode" : "");

   LLVMAddTargetDependentFunctionAttr(F, "target-features", features);
}

bool ac_init_llvm_compiler(struct ac_llvm_compiler *compiler, enum radeon_family family,
                           enum ac_target_machine_options tm_options)
{
   const char *triple;
   memset(compiler, 0, sizeof(*compiler));

   compiler->tm = ac_create_target_machine(family, tm_options, LLVMCodeGenLevelDefault, &triple);
   if (!compiler->tm)
      return false;

   if (tm_options & AC_TM_CREATE_LOW_OPT) {
      compiler->low_opt_tm =
         ac_create_target_machine(family, tm_options, LLVMCodeGenLevelLess, NULL);
      if (!compiler->low_opt_tm)
         goto fail;
   }

   compiler->target_library_info = ac_create_target_library_info(triple);
   if (!compiler->target_library_info)
      goto fail;

   compiler->passmgr =
      ac_create_passmgr(compiler->target_library_info, tm_options & AC_TM_CHECK_IR);
   if (!compiler->passmgr)
      goto fail;

   return true;
fail:
   ac_destroy_llvm_compiler(compiler);
   return false;
}

void ac_destroy_llvm_compiler(struct ac_llvm_compiler *compiler)
{
   ac_destroy_llvm_passes(compiler->passes);
   ac_destroy_llvm_passes(compiler->low_opt_passes);

   if (compiler->passmgr)
      LLVMDisposePassManager(compiler->passmgr);
   if (compiler->target_library_info)
      ac_dispose_target_library_info(compiler->target_library_info);
   if (compiler->low_opt_tm)
      LLVMDisposeTargetMachine(compiler->low_opt_tm);
   if (compiler->tm)
      LLVMDisposeTargetMachine(compiler->tm);
}
