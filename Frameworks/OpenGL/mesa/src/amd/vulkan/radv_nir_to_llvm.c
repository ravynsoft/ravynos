/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 *
 * based in part on anv driver which is:
 * Copyright © 2015 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "nir/nir.h"
#include "radv_debug.h"
#include "radv_llvm_helper.h"
#include "radv_private.h"
#include "radv_shader.h"
#include "radv_shader_args.h"

#include "ac_binary.h"
#include "ac_llvm_build.h"
#include "ac_nir.h"
#include "ac_nir_to_llvm.h"
#include "ac_shader_abi.h"
#include "ac_shader_util.h"
#include "sid.h"

struct radv_shader_context {
   struct ac_llvm_context ac;
   const struct nir_shader *shader;
   struct ac_shader_abi abi;
   const struct radv_nir_compiler_options *options;
   const struct radv_shader_info *shader_info;
   const struct radv_shader_args *args;

   gl_shader_stage stage;

   unsigned max_workgroup_size;
   LLVMContextRef context;
   struct ac_llvm_pointer main_function;

   LLVMValueRef descriptor_sets[MAX_SETS];

   LLVMValueRef gs_wave_id;

   uint64_t output_mask;
};

static inline struct radv_shader_context *
radv_shader_context_from_abi(struct ac_shader_abi *abi)
{
   return container_of(abi, struct radv_shader_context, abi);
}

static struct ac_llvm_pointer
create_llvm_function(struct ac_llvm_context *ctx, LLVMModuleRef module, LLVMBuilderRef builder,
                     const struct ac_shader_args *args, enum ac_llvm_calling_convention convention,
                     unsigned max_workgroup_size, const struct radv_nir_compiler_options *options)
{
   struct ac_llvm_pointer main_function = ac_build_main(args, ctx, convention, "main", ctx->voidt, module);

   if (options->info->address32_hi) {
      ac_llvm_add_target_dep_function_attr(main_function.value, "amdgpu-32bit-address-high-bits",
                                           options->info->address32_hi);
   }

   ac_llvm_set_workgroup_size(main_function.value, max_workgroup_size);
   ac_llvm_set_target_features(main_function.value, ctx, true);

   return main_function;
}

static void
load_descriptor_sets(struct radv_shader_context *ctx)
{
   const struct radv_userdata_locations *user_sgprs_locs = &ctx->shader_info->user_sgprs_locs;
   uint32_t mask = ctx->shader_info->desc_set_used_mask;

   if (user_sgprs_locs->shader_data[AC_UD_INDIRECT_DESCRIPTOR_SETS].sgpr_idx != -1) {
      struct ac_llvm_pointer desc_sets = ac_get_ptr_arg(&ctx->ac, &ctx->args->ac, ctx->args->descriptor_sets[0]);
      while (mask) {
         int i = u_bit_scan(&mask);

         ctx->descriptor_sets[i] = ac_build_load_to_sgpr(&ctx->ac, desc_sets, LLVMConstInt(ctx->ac.i32, i, false));
         LLVMSetAlignment(ctx->descriptor_sets[i], 4);
      }
   } else {
      while (mask) {
         int i = u_bit_scan(&mask);

         ctx->descriptor_sets[i] = ac_get_arg(&ctx->ac, ctx->args->descriptor_sets[i]);
      }
   }
}

static enum ac_llvm_calling_convention
get_llvm_calling_convention(LLVMValueRef func, gl_shader_stage stage)
{
   switch (stage) {
   case MESA_SHADER_VERTEX:
   case MESA_SHADER_TESS_EVAL:
      return AC_LLVM_AMDGPU_VS;
      break;
   case MESA_SHADER_GEOMETRY:
      return AC_LLVM_AMDGPU_GS;
      break;
   case MESA_SHADER_TESS_CTRL:
      return AC_LLVM_AMDGPU_HS;
      break;
   case MESA_SHADER_FRAGMENT:
      return AC_LLVM_AMDGPU_PS;
      break;
   case MESA_SHADER_COMPUTE:
      return AC_LLVM_AMDGPU_CS;
      break;
   default:
      unreachable("Unhandle shader type");
   }
}

/* Returns whether the stage is a stage that can be directly before the GS */
static bool
is_pre_gs_stage(gl_shader_stage stage)
{
   return stage == MESA_SHADER_VERTEX || stage == MESA_SHADER_TESS_EVAL;
}

static void
create_function(struct radv_shader_context *ctx, gl_shader_stage stage, bool has_previous_stage)
{
   if (ctx->ac.gfx_level >= GFX10) {
      if (is_pre_gs_stage(stage) && ctx->shader_info->is_ngg) {
         /* On GFX10+, VS and TES are merged into GS for NGG. */
         stage = MESA_SHADER_GEOMETRY;
         has_previous_stage = true;
      }
   }

   ctx->main_function = create_llvm_function(&ctx->ac, ctx->ac.module, ctx->ac.builder, &ctx->args->ac,
                                             get_llvm_calling_convention(ctx->main_function.value, stage),
                                             ctx->max_workgroup_size, ctx->options);

   load_descriptor_sets(ctx);

   if (stage == MESA_SHADER_TESS_CTRL || (stage == MESA_SHADER_VERTEX && ctx->shader_info->vs.as_ls) ||
       ctx->shader_info->is_ngg ||
       /* GFX9 has the ESGS ring buffer in LDS. */
       (stage == MESA_SHADER_GEOMETRY && has_previous_stage)) {
      ac_declare_lds_as_pointer(&ctx->ac);
   }
}

static LLVMValueRef
radv_load_base_vertex(struct ac_shader_abi *abi, bool non_indexed_is_zero)
{
   struct radv_shader_context *ctx = radv_shader_context_from_abi(abi);
   return ac_get_arg(&ctx->ac, ctx->args->ac.base_vertex);
}

static LLVMValueRef
radv_load_rsrc(struct radv_shader_context *ctx, LLVMValueRef ptr, LLVMTypeRef type)
{
   if (ptr && LLVMTypeOf(ptr) == ctx->ac.i32) {
      LLVMValueRef result;

      LLVMTypeRef ptr_type = LLVMPointerType(type, AC_ADDR_SPACE_CONST_32BIT);
      ptr = LLVMBuildIntToPtr(ctx->ac.builder, ptr, ptr_type, "");
      LLVMSetMetadata(ptr, ctx->ac.uniform_md_kind, ctx->ac.empty_md);

      result = LLVMBuildLoad2(ctx->ac.builder, type, ptr, "");
      LLVMSetMetadata(result, ctx->ac.invariant_load_md_kind, ctx->ac.empty_md);

      return result;
   }

   return ptr;
}

static LLVMValueRef
radv_load_ubo(struct ac_shader_abi *abi, LLVMValueRef buffer_ptr)
{
   struct radv_shader_context *ctx = radv_shader_context_from_abi(abi);
   return radv_load_rsrc(ctx, buffer_ptr, ctx->ac.v4i32);
}

static LLVMValueRef
radv_load_ssbo(struct ac_shader_abi *abi, LLVMValueRef buffer_ptr, bool write, bool non_uniform)
{
   struct radv_shader_context *ctx = radv_shader_context_from_abi(abi);
   return radv_load_rsrc(ctx, buffer_ptr, ctx->ac.v4i32);
}

static LLVMValueRef
radv_get_sampler_desc(struct ac_shader_abi *abi, LLVMValueRef index, enum ac_descriptor_type desc_type)
{
   struct radv_shader_context *ctx = radv_shader_context_from_abi(abi);

   /* 3 plane formats always have same size and format for plane 1 & 2, so
    * use the tail from plane 1 so that we can store only the first 16 bytes
    * of the last plane. */
   if (desc_type == AC_DESC_PLANE_2 && index && LLVMTypeOf(index) == ctx->ac.i32) {
      LLVMValueRef plane1_addr = LLVMBuildSub(ctx->ac.builder, index, LLVMConstInt(ctx->ac.i32, 32, false), "");
      LLVMValueRef descriptor1 = radv_load_rsrc(ctx, plane1_addr, ctx->ac.v8i32);
      LLVMValueRef descriptor2 = radv_load_rsrc(ctx, index, ctx->ac.v4i32);

      LLVMValueRef components[8];
      for (unsigned i = 0; i < 4; ++i)
         components[i] = ac_llvm_extract_elem(&ctx->ac, descriptor2, i);

      for (unsigned i = 4; i < 8; ++i)
         components[i] = ac_llvm_extract_elem(&ctx->ac, descriptor1, i);
      return ac_build_gather_values(&ctx->ac, components, 8);
   }

   bool v4 = desc_type == AC_DESC_BUFFER || desc_type == AC_DESC_SAMPLER;
   return radv_load_rsrc(ctx, index, v4 ? ctx->ac.v4i32 : ctx->ac.v8i32);
}

static void
scan_shader_output_decl(struct radv_shader_context *ctx, struct nir_variable *variable, struct nir_shader *shader,
                        gl_shader_stage stage)
{
   int idx = variable->data.driver_location;
   unsigned attrib_count = glsl_count_attribute_slots(variable->type, false);
   uint64_t mask_attribs;

   if (variable->data.compact) {
      unsigned component_count = variable->data.location_frac + glsl_get_length(variable->type);
      attrib_count = (component_count + 3) / 4;
   }

   mask_attribs = ((1ull << attrib_count) - 1) << idx;

   ctx->output_mask |= mask_attribs;
}

static LLVMValueRef
radv_load_output(struct radv_shader_context *ctx, unsigned index, unsigned chan)
{
   int idx = ac_llvm_reg_index_soa(index, chan);
   LLVMValueRef output = ctx->abi.outputs[idx];
   LLVMTypeRef type = ctx->abi.is_16bit[idx] ? ctx->ac.f16 : ctx->ac.f32;
   return LLVMBuildLoad2(ctx->ac.builder, type, output, "");
}

static void
ac_llvm_finalize_module(struct radv_shader_context *ctx, LLVMPassManagerRef passmgr)
{
   LLVMRunPassManager(passmgr, ctx->ac.module);
   LLVMDisposeBuilder(ctx->ac.builder);

   ac_llvm_context_dispose(&ctx->ac);
}

static void
prepare_gs_input_vgprs(struct radv_shader_context *ctx, bool merged)
{
   if (merged) {
      ctx->gs_wave_id = ac_unpack_param(&ctx->ac, ac_get_arg(&ctx->ac, ctx->args->ac.merged_wave_info), 16, 8);
   } else {
      ctx->gs_wave_id = ac_get_arg(&ctx->ac, ctx->args->ac.gs_wave_id);
   }
}

/* Ensure that the esgs ring is declared.
 *
 * We declare it with 64KB alignment as a hint that the
 * pointer value will always be 0.
 */
static void
declare_esgs_ring(struct radv_shader_context *ctx)
{
   assert(!LLVMGetNamedGlobal(ctx->ac.module, "esgs_ring"));

   LLVMValueRef esgs_ring =
      LLVMAddGlobalInAddressSpace(ctx->ac.module, LLVMArrayType(ctx->ac.i32, 0), "esgs_ring", AC_ADDR_SPACE_LDS);
   LLVMSetLinkage(esgs_ring, LLVMExternalLinkage);
   LLVMSetAlignment(esgs_ring, 64 * 1024);
}

static LLVMValueRef
radv_intrinsic_load(struct ac_shader_abi *abi, nir_intrinsic_instr *intrin)
{
   switch (intrin->intrinsic) {
   case nir_intrinsic_load_base_vertex:
   case nir_intrinsic_load_first_vertex:
      return radv_load_base_vertex(abi, intrin->intrinsic == nir_intrinsic_load_base_vertex);
   default:
      return NULL;
   }
}

static LLVMModuleRef
ac_translate_nir_to_llvm(struct ac_llvm_compiler *ac_llvm, const struct radv_nir_compiler_options *options,
                         const struct radv_shader_info *info, struct nir_shader *const *shaders, int shader_count,
                         const struct radv_shader_args *args)
{
   struct radv_shader_context ctx = {0};
   ctx.args = args;
   ctx.options = options;
   ctx.shader_info = info;

   enum ac_float_mode float_mode = AC_FLOAT_MODE_DEFAULT;

   if (shaders[0]->info.float_controls_execution_mode & FLOAT_CONTROLS_DENORM_FLUSH_TO_ZERO_FP32) {
      float_mode = AC_FLOAT_MODE_DENORM_FLUSH_TO_ZERO;
   }

   bool exports_mrtz = false;
   bool exports_color_null = false;
   if (shaders[0]->info.stage == MESA_SHADER_FRAGMENT) {
      exports_mrtz = info->ps.writes_z || info->ps.writes_stencil || info->ps.writes_sample_mask;
      exports_color_null = !exports_mrtz || (shaders[0]->info.outputs_written & (0xffu << FRAG_RESULT_DATA0));
   }

   ac_llvm_context_init(&ctx.ac, ac_llvm, options->info, float_mode, info->wave_size, info->ballot_bit_size,
                        exports_color_null, exports_mrtz);

   uint32_t length = 1;
   for (uint32_t i = 0; i < shader_count; i++)
      if (shaders[i]->info.name)
         length += strlen(shaders[i]->info.name) + 1;

   char *name = malloc(length);
   if (name) {
      uint32_t offset = 0;
      for (uint32_t i = 0; i < shader_count; i++) {
         if (!shaders[i]->info.name)
            continue;

         strcpy(name + offset, shaders[i]->info.name);
         offset += strlen(shaders[i]->info.name);
         if (i != shader_count - 1)
            name[offset++] = ',';
      }

      LLVMSetSourceFileName(ctx.ac.module, name, offset);
   }

   ctx.context = ctx.ac.context;

   ctx.max_workgroup_size = info->workgroup_size;

   create_function(&ctx, shaders[shader_count - 1]->info.stage, shader_count >= 2);

   ctx.abi.intrinsic_load = radv_intrinsic_load;
   ctx.abi.load_ubo = radv_load_ubo;
   ctx.abi.load_ssbo = radv_load_ssbo;
   ctx.abi.load_sampler_desc = radv_get_sampler_desc;
   ctx.abi.clamp_shadow_reference = false;
   ctx.abi.robust_buffer_access = options->robust_buffer_access_llvm;
   ctx.abi.load_grid_size_from_user_sgpr = args->load_grid_size_from_user_sgpr;

   bool is_ngg = is_pre_gs_stage(shaders[0]->info.stage) && info->is_ngg;
   if (shader_count >= 2 || is_ngg)
      ac_init_exec_full_mask(&ctx.ac);

   if (args->ac.vertex_id.used)
      ctx.abi.vertex_id = ac_get_arg(&ctx.ac, args->ac.vertex_id);
   if (args->ac.vs_rel_patch_id.used)
      ctx.abi.vs_rel_patch_id = ac_get_arg(&ctx.ac, args->ac.vs_rel_patch_id);
   if (args->ac.instance_id.used)
      ctx.abi.instance_id = ac_get_arg(&ctx.ac, args->ac.instance_id);

   if (options->info->has_ls_vgpr_init_bug && shaders[shader_count - 1]->info.stage == MESA_SHADER_TESS_CTRL)
      ac_fixup_ls_hs_input_vgprs(&ctx.ac, &ctx.abi, &args->ac);

   if (is_ngg) {
      if (!info->is_ngg_passthrough)
         declare_esgs_ring(&ctx);

      if (ctx.stage == MESA_SHADER_GEOMETRY) {
         /* Scratch space used by NGG GS for repacking vertices at the end. */
         LLVMTypeRef ai32 = LLVMArrayType(ctx.ac.i32, 8);
         LLVMValueRef gs_ngg_scratch =
            LLVMAddGlobalInAddressSpace(ctx.ac.module, ai32, "ngg_scratch", AC_ADDR_SPACE_LDS);
         LLVMSetInitializer(gs_ngg_scratch, LLVMGetUndef(ai32));
         LLVMSetLinkage(gs_ngg_scratch, LLVMExternalLinkage);
         LLVMSetAlignment(gs_ngg_scratch, 4);

         /* Vertex emit space used by NGG GS for storing all vertex attributes. */
         LLVMValueRef gs_ngg_emit =
            LLVMAddGlobalInAddressSpace(ctx.ac.module, LLVMArrayType(ctx.ac.i32, 0), "ngg_emit", AC_ADDR_SPACE_LDS);
         LLVMSetInitializer(gs_ngg_emit, LLVMGetUndef(ai32));
         LLVMSetLinkage(gs_ngg_emit, LLVMExternalLinkage);
         LLVMSetAlignment(gs_ngg_emit, 4);
      }

      /* GFX10 hang workaround - there needs to be an s_barrier before gs_alloc_req always */
      if (ctx.ac.gfx_level == GFX10 && shader_count == 1)
         ac_build_s_barrier(&ctx.ac, shaders[0]->info.stage);
   }

   for (int shader_idx = 0; shader_idx < shader_count; ++shader_idx) {
      ctx.stage = shaders[shader_idx]->info.stage;
      ctx.shader = shaders[shader_idx];
      ctx.output_mask = 0;

      if (shader_idx && !(shaders[shader_idx]->info.stage == MESA_SHADER_GEOMETRY && info->is_ngg)) {
         /* Execute a barrier before the second shader in
          * a merged shader.
          *
          * Execute the barrier inside the conditional block,
          * so that empty waves can jump directly to s_endpgm,
          * which will also signal the barrier.
          *
          * This is possible in gfx9, because an empty wave
          * for the second shader does not participate in
          * the epilogue. With NGG, empty waves may still
          * be required to export data (e.g. GS output vertices),
          * so we cannot let them exit early.
          *
          * If the shader is TCS and the TCS epilog is present
          * and contains a barrier, it will wait there and then
          * reach s_endpgm.
          */
         ac_build_waitcnt(&ctx.ac, AC_WAIT_LGKM);
         ac_build_s_barrier(&ctx.ac, shaders[shader_idx]->info.stage);
      }

      nir_foreach_shader_out_variable (variable, shaders[shader_idx])
         scan_shader_output_decl(&ctx, variable, shaders[shader_idx], shaders[shader_idx]->info.stage);

      bool check_merged_wave_info = shader_count >= 2 && !(is_ngg && shader_idx == 1);
      LLVMBasicBlockRef merge_block = NULL;

      if (check_merged_wave_info) {
         LLVMValueRef fn = LLVMGetBasicBlockParent(LLVMGetInsertBlock(ctx.ac.builder));
         LLVMBasicBlockRef then_block = LLVMAppendBasicBlockInContext(ctx.ac.context, fn, "");
         merge_block = LLVMAppendBasicBlockInContext(ctx.ac.context, fn, "");

         LLVMValueRef count =
            ac_unpack_param(&ctx.ac, ac_get_arg(&ctx.ac, args->ac.merged_wave_info), 8 * shader_idx, 8);
         LLVMValueRef thread_id = ac_get_thread_id(&ctx.ac);
         LLVMValueRef cond = LLVMBuildICmp(ctx.ac.builder, LLVMIntULT, thread_id, count, "");
         LLVMBuildCondBr(ctx.ac.builder, cond, then_block, merge_block);

         LLVMPositionBuilderAtEnd(ctx.ac.builder, then_block);
      }

      if (shaders[shader_idx]->info.stage == MESA_SHADER_GEOMETRY && !info->is_ngg)
         prepare_gs_input_vgprs(&ctx, shader_count >= 2);

      if (!ac_nir_translate(&ctx.ac, &ctx.abi, &args->ac, shaders[shader_idx])) {
         abort();
      }

      if (check_merged_wave_info) {
         LLVMBuildBr(ctx.ac.builder, merge_block);
         LLVMPositionBuilderAtEnd(ctx.ac.builder, merge_block);
      }
   }

   LLVMBuildRetVoid(ctx.ac.builder);

   if (options->dump_preoptir) {
      fprintf(stderr, "%s LLVM IR:\n\n", radv_get_shader_name(info, shaders[shader_count - 1]->info.stage));
      ac_dump_module(ctx.ac.module);
      fprintf(stderr, "\n");
   }

   ac_llvm_finalize_module(&ctx, ac_llvm->passmgr);

   free(name);

   return ctx.ac.module;
}

static void
ac_diagnostic_handler(LLVMDiagnosticInfoRef di, void *context)
{
   unsigned *retval = (unsigned *)context;
   LLVMDiagnosticSeverity severity = LLVMGetDiagInfoSeverity(di);
   char *description = LLVMGetDiagInfoDescription(di);

   if (severity == LLVMDSError) {
      *retval = 1;
      fprintf(stderr, "LLVM triggered Diagnostic Handler: %s\n", description);
   }

   LLVMDisposeMessage(description);
}

static unsigned
radv_llvm_compile(LLVMModuleRef M, char **pelf_buffer, size_t *pelf_size, struct ac_llvm_compiler *ac_llvm)
{
   unsigned retval = 0;
   LLVMContextRef llvm_ctx;

   /* Setup Diagnostic Handler*/
   llvm_ctx = LLVMGetModuleContext(M);

   LLVMContextSetDiagnosticHandler(llvm_ctx, ac_diagnostic_handler, &retval);

   /* Compile IR*/
   if (!radv_compile_to_elf(ac_llvm, M, pelf_buffer, pelf_size))
      retval = 1;
   return retval;
}

static void
ac_compile_llvm_module(struct ac_llvm_compiler *ac_llvm, LLVMModuleRef llvm_module, struct radv_shader_binary **rbinary,
                       const char *name, const struct radv_nir_compiler_options *options)
{
   char *elf_buffer = NULL;
   size_t elf_size = 0;
   char *llvm_ir_string = NULL;

   if (options->dump_shader) {
      fprintf(stderr, "%s LLVM IR:\n\n", name);
      ac_dump_module(llvm_module);
      fprintf(stderr, "\n");
   }

   if (options->record_ir) {
      char *llvm_ir = LLVMPrintModuleToString(llvm_module);
      llvm_ir_string = strdup(llvm_ir);
      LLVMDisposeMessage(llvm_ir);
   }

   int v = radv_llvm_compile(llvm_module, &elf_buffer, &elf_size, ac_llvm);
   if (v) {
      fprintf(stderr, "compile failed\n");
   }

   LLVMContextRef ctx = LLVMGetModuleContext(llvm_module);
   LLVMDisposeModule(llvm_module);
   LLVMContextDispose(ctx);

   size_t llvm_ir_size = llvm_ir_string ? strlen(llvm_ir_string) : 0;
   size_t alloc_size = sizeof(struct radv_shader_binary_rtld) + elf_size + llvm_ir_size + 1;
   struct radv_shader_binary_rtld *rbin = calloc(1, alloc_size);
   memcpy(rbin->data, elf_buffer, elf_size);
   if (llvm_ir_string)
      memcpy(rbin->data + elf_size, llvm_ir_string, llvm_ir_size + 1);

   rbin->base.type = RADV_BINARY_TYPE_RTLD;
   rbin->base.total_size = alloc_size;
   rbin->elf_size = elf_size;
   rbin->llvm_ir_size = llvm_ir_size;
   *rbinary = &rbin->base;

   free(llvm_ir_string);
   free(elf_buffer);
}

static void
radv_compile_nir_shader(struct ac_llvm_compiler *ac_llvm, const struct radv_nir_compiler_options *options,
                        const struct radv_shader_info *info, struct radv_shader_binary **rbinary,
                        const struct radv_shader_args *args, struct nir_shader *const *nir, int nir_count)
{

   LLVMModuleRef llvm_module;

   llvm_module = ac_translate_nir_to_llvm(ac_llvm, options, info, nir, nir_count, args);

   ac_compile_llvm_module(ac_llvm, llvm_module, rbinary, radv_get_shader_name(info, nir[nir_count - 1]->info.stage),
                          options);
}

void
llvm_compile_shader(const struct radv_nir_compiler_options *options, const struct radv_shader_info *info,
                    unsigned shader_count, struct nir_shader *const *shaders, struct radv_shader_binary **binary,
                    const struct radv_shader_args *args)
{
   enum ac_target_machine_options tm_options = 0;
   struct ac_llvm_compiler ac_llvm;

   tm_options |= AC_TM_SUPPORTS_SPILL;
   if (options->check_ir)
      tm_options |= AC_TM_CHECK_IR;

   radv_init_llvm_compiler(&ac_llvm, options->info->family, tm_options, info->wave_size);

   radv_compile_nir_shader(&ac_llvm, options, info, binary, args, shaders, shader_count);
}
