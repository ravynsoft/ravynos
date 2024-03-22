/*
 * Copyright 2013 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "ac_rtld.h"
#include "amd_kernel_code_t.h"
#include "nir/tgsi_to_nir.h"
#include "si_build_pm4.h"
#include "si_shader_internal.h"
#include "util/u_async_debug.h"
#include "util/u_memory.h"
#include "util/u_upload_mgr.h"
#include "si_tracepoints.h"

#define COMPUTE_DBG(sscreen, fmt, args...)                                                         \
   do {                                                                                            \
      if ((sscreen->debug_flags & DBG(COMPUTE)))                                                   \
         fprintf(stderr, fmt, ##args);                                                             \
   } while (0);

struct dispatch_packet {
   uint16_t header;
   uint16_t setup;
   uint16_t workgroup_size_x;
   uint16_t workgroup_size_y;
   uint16_t workgroup_size_z;
   uint16_t reserved0;
   uint32_t grid_size_x;
   uint32_t grid_size_y;
   uint32_t grid_size_z;
   uint32_t group_segment_size;
   uint64_t kernel_object;
   uint64_t kernarg_address;
   uint64_t reserved2;
};

static const amd_kernel_code_t *si_compute_get_code_object(const struct si_compute *program,
                                                           uint64_t symbol_offset)
{
   const struct si_shader_selector *sel = &program->sel;

   if (program->ir_type != PIPE_SHADER_IR_NATIVE)
      return NULL;

   struct ac_rtld_binary rtld;
   if (!ac_rtld_open(&rtld,
                     (struct ac_rtld_open_info){.info = &sel->screen->info,
                                                .shader_type = MESA_SHADER_COMPUTE,
                                                .num_parts = 1,
                                                .elf_ptrs = &program->shader.binary.code_buffer,
                                                .elf_sizes = &program->shader.binary.code_size}))
      return NULL;

   const amd_kernel_code_t *result = NULL;
   const char *text;
   size_t size;
   if (!ac_rtld_get_section_by_name(&rtld, ".text", &text, &size))
      goto out;

   if (symbol_offset + sizeof(amd_kernel_code_t) > size)
      goto out;

   result = (const amd_kernel_code_t *)(text + symbol_offset);

out:
   ac_rtld_close(&rtld);
   return result;
}

static void code_object_to_config(const amd_kernel_code_t *code_object,
                                  struct ac_shader_config *out_config)
{

   uint32_t rsrc1 = code_object->compute_pgm_resource_registers;
   uint32_t rsrc2 = code_object->compute_pgm_resource_registers >> 32;
   out_config->num_sgprs = code_object->wavefront_sgpr_count;
   out_config->num_vgprs = code_object->workitem_vgpr_count;
   out_config->float_mode = G_00B028_FLOAT_MODE(rsrc1);
   out_config->rsrc1 = rsrc1;
   out_config->lds_size = MAX2(out_config->lds_size, G_00B84C_LDS_SIZE(rsrc2));
   out_config->rsrc2 = rsrc2;
   out_config->scratch_bytes_per_wave =
      align(code_object->workitem_private_segment_byte_size * 64, 1024);
}

/* Asynchronous compute shader compilation. */
static void si_create_compute_state_async(void *job, void *gdata, int thread_index)
{
   struct si_compute *program = (struct si_compute *)job;
   struct si_shader_selector *sel = &program->sel;
   struct si_shader *shader = &program->shader;
   struct ac_llvm_compiler **compiler;
   struct util_debug_callback *debug = &sel->compiler_ctx_state.debug;
   struct si_screen *sscreen = sel->screen;

   assert(!debug->debug_message || debug->async);
   assert(thread_index >= 0);
   assert(thread_index < ARRAY_SIZE(sscreen->compiler));
   compiler = &sscreen->compiler[thread_index];

   if (!sscreen->use_aco && !*compiler)
      *compiler = si_create_llvm_compiler(sscreen);

   assert(program->ir_type == PIPE_SHADER_IR_NIR);
   si_nir_scan_shader(sscreen, sel->nir, &sel->info);

   si_get_active_slot_masks(sscreen, &sel->info, &sel->active_const_and_shader_buffers,
                            &sel->active_samplers_and_images);

   program->shader.is_monolithic = true;
   program->shader.wave_size = si_determine_wave_size(sscreen, &program->shader);

   /* Variable block sizes need 10 bits (1 + log2(SI_MAX_VARIABLE_THREADS_PER_BLOCK)) per dim.
    * We pack them into a single user SGPR.
    */
   unsigned user_sgprs = SI_NUM_RESOURCE_SGPRS + (sel->info.uses_grid_size ? 3 : 0) +
                         (sel->info.uses_variable_block_size ? 1 : 0) +
                         sel->info.base.cs.user_data_components_amd;

   /* Fast path for compute shaders - some descriptors passed via user SGPRs. */
   /* Shader buffers in user SGPRs. */
   for (unsigned i = 0; i < MIN2(3, sel->info.base.num_ssbos) && user_sgprs <= 12; i++) {
      user_sgprs = align(user_sgprs, 4);
      if (i == 0)
         sel->cs_shaderbufs_sgpr_index = user_sgprs;
      user_sgprs += 4;
      sel->cs_num_shaderbufs_in_user_sgprs++;
   }

   /* Images in user SGPRs. */
   unsigned non_fmask_images = u_bit_consecutive(0, sel->info.base.num_images);

   /* Remove images with FMASK from the bitmask.  We only care about the first
    * 3 anyway, so we can take msaa_images[0] and ignore the rest.
    */
   if (sscreen->info.gfx_level < GFX11)
      non_fmask_images &= ~sel->info.base.msaa_images[0];

   for (unsigned i = 0; i < 3 && non_fmask_images & (1 << i); i++) {
      unsigned num_sgprs = BITSET_TEST(sel->info.base.image_buffers, i) ? 4 : 8;

      if (align(user_sgprs, num_sgprs) + num_sgprs > 16)
         break;

      user_sgprs = align(user_sgprs, num_sgprs);
      if (i == 0)
         sel->cs_images_sgpr_index = user_sgprs;
      user_sgprs += num_sgprs;
      sel->cs_num_images_in_user_sgprs++;
   }
   sel->cs_images_num_sgprs = user_sgprs - sel->cs_images_sgpr_index;
   assert(user_sgprs <= 16);

   unsigned char ir_sha1_cache_key[20];
   si_get_ir_cache_key(sel, false, false, shader->wave_size, ir_sha1_cache_key);

   /* Try to load the shader from the shader cache. */
   simple_mtx_lock(&sscreen->shader_cache_mutex);

   if (si_shader_cache_load_shader(sscreen, ir_sha1_cache_key, shader)) {
      simple_mtx_unlock(&sscreen->shader_cache_mutex);

      if (!si_shader_binary_upload(sscreen, shader, 0))
         program->shader.compilation_failed = true;

      si_shader_dump_stats_for_shader_db(sscreen, shader, debug);
      si_shader_dump(sscreen, shader, debug, stderr, true);
   } else {
      simple_mtx_unlock(&sscreen->shader_cache_mutex);

      if (!si_create_shader_variant(sscreen, *compiler, &program->shader, debug)) {
         program->shader.compilation_failed = true;
         return;
      }

      shader->config.rsrc1 = S_00B848_VGPRS((shader->config.num_vgprs - 1) /
                                            ((shader->wave_size == 32 ||
                                              sscreen->info.wave64_vgpr_alloc_granularity == 8) ? 8 : 4)) |
                             S_00B848_DX10_CLAMP(1) |
                             S_00B848_MEM_ORDERED(si_shader_mem_ordered(shader)) |
                             S_00B848_FLOAT_MODE(shader->config.float_mode);

      if (sscreen->info.gfx_level < GFX10) {
         shader->config.rsrc1 |= S_00B848_SGPRS((shader->config.num_sgprs - 1) / 8);
      }

      shader->config.rsrc2 = S_00B84C_USER_SGPR(user_sgprs) |
                             S_00B84C_SCRATCH_EN(shader->config.scratch_bytes_per_wave > 0) |
                             S_00B84C_TGID_X_EN(sel->info.uses_block_id[0]) |
                             S_00B84C_TGID_Y_EN(sel->info.uses_block_id[1]) |
                             S_00B84C_TGID_Z_EN(sel->info.uses_block_id[2]) |
                             S_00B84C_TG_SIZE_EN(sel->info.uses_tg_size) |
                             S_00B84C_TIDIG_COMP_CNT(sel->info.uses_thread_id[2]
                                                        ? 2
                                                        : sel->info.uses_thread_id[1] ? 1 : 0) |
                             S_00B84C_LDS_SIZE(shader->config.lds_size);

      simple_mtx_lock(&sscreen->shader_cache_mutex);
      si_shader_cache_insert_shader(sscreen, ir_sha1_cache_key, shader, true);
      simple_mtx_unlock(&sscreen->shader_cache_mutex);
   }

   ralloc_free(sel->nir);
   sel->nir = NULL;
}

static void *si_create_compute_state(struct pipe_context *ctx, const struct pipe_compute_state *cso)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_screen *sscreen = (struct si_screen *)ctx->screen;
   struct si_compute *program = CALLOC_STRUCT(si_compute);
   struct si_shader_selector *sel = &program->sel;

   pipe_reference_init(&sel->base.reference, 1);
   sel->stage = MESA_SHADER_COMPUTE;
   sel->screen = sscreen;
   sel->const_and_shader_buf_descriptors_index =
      si_const_and_shader_buffer_descriptors_idx(PIPE_SHADER_COMPUTE);
   sel->sampler_and_images_descriptors_index =
      si_sampler_and_image_descriptors_idx(PIPE_SHADER_COMPUTE);
   sel->info.base.shared_size = cso->static_shared_mem;
   program->shader.selector = &program->sel;
   program->ir_type = cso->ir_type;
   program->input_size = cso->req_input_mem;

   if (cso->ir_type != PIPE_SHADER_IR_NATIVE) {
      if (cso->ir_type == PIPE_SHADER_IR_TGSI) {
         program->ir_type = PIPE_SHADER_IR_NIR;
         sel->nir = tgsi_to_nir(cso->prog, ctx->screen, true);
      } else {
         assert(cso->ir_type == PIPE_SHADER_IR_NIR);
         sel->nir = (struct nir_shader *)cso->prog;
      }

      if (si_can_dump_shader(sscreen, sel->stage, SI_DUMP_INIT_NIR))
         nir_print_shader(sel->nir, stderr);

      sel->compiler_ctx_state.debug = sctx->debug;
      sel->compiler_ctx_state.is_debug_context = sctx->is_debug;
      p_atomic_inc(&sscreen->num_shaders_created);

      si_schedule_initial_compile(sctx, MESA_SHADER_COMPUTE, &sel->ready, &sel->compiler_ctx_state,
                                  program, si_create_compute_state_async);
   } else {
      const struct pipe_binary_program_header *header;
      header = cso->prog;

      program->shader.binary.type = SI_SHADER_BINARY_ELF;
      program->shader.binary.code_size = header->num_bytes;
      program->shader.binary.code_buffer = malloc(header->num_bytes);
      if (!program->shader.binary.code_buffer) {
         FREE(program);
         return NULL;
      }
      memcpy((void *)program->shader.binary.code_buffer, header->blob, header->num_bytes);

      const amd_kernel_code_t *code_object = si_compute_get_code_object(program, 0);
      code_object_to_config(code_object, &program->shader.config);

      if (AMD_HSA_BITS_GET(code_object->code_properties, AMD_CODE_PROPERTY_ENABLE_WAVEFRONT_SIZE32))
         program->shader.wave_size = 32;
      else
         program->shader.wave_size = 64;

      bool ok = si_shader_binary_upload(sctx->screen, &program->shader, 0);
      si_shader_dump(sctx->screen, &program->shader, &sctx->debug, stderr, true);

      if (!ok) {
         fprintf(stderr, "LLVM failed to upload shader\n");
         free((void *)program->shader.binary.code_buffer);
         FREE(program);
         return NULL;
      }
   }

   return program;
}

static void si_get_compute_state_info(struct pipe_context *ctx, void *state,
                                      struct pipe_compute_state_object_info *info)
{
   struct si_compute *program = (struct si_compute *)state;
   struct si_shader_selector *sel = &program->sel;

   assert(program->ir_type != PIPE_SHADER_IR_NATIVE);

   /* Wait because we need the compilation to finish first */
   util_queue_fence_wait(&sel->ready);

   uint8_t wave_size = program->shader.wave_size;
   info->private_memory = DIV_ROUND_UP(program->shader.config.scratch_bytes_per_wave, wave_size);
   info->preferred_simd_size = wave_size;
   info->simd_sizes = wave_size;
   info->max_threads = si_get_max_workgroup_size(&program->shader);
}

static void si_bind_compute_state(struct pipe_context *ctx, void *state)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_compute *program = (struct si_compute *)state;
   struct si_shader_selector *sel = &program->sel;

   sctx->cs_shader_state.program = program;
   if (!program)
      return;

   /* Wait because we need active slot usage masks. */
   if (program->ir_type != PIPE_SHADER_IR_NATIVE)
      util_queue_fence_wait(&sel->ready);

   si_set_active_descriptors(sctx,
                             SI_DESCS_FIRST_COMPUTE + SI_SHADER_DESCS_CONST_AND_SHADER_BUFFERS,
                             sel->active_const_and_shader_buffers);
   si_set_active_descriptors(sctx, SI_DESCS_FIRST_COMPUTE + SI_SHADER_DESCS_SAMPLERS_AND_IMAGES,
                             sel->active_samplers_and_images);

   sctx->compute_shaderbuf_sgprs_dirty = true;
   sctx->compute_image_sgprs_dirty = true;

   if (unlikely((sctx->screen->debug_flags & DBG(SQTT)) && sctx->sqtt)) {
      uint32_t pipeline_code_hash = _mesa_hash_data_with_seed(
         program->shader.binary.code_buffer,
         program->shader.binary.code_size,
         0);

      if (!si_sqtt_pipeline_is_registered(sctx->sqtt, pipeline_code_hash)) {
         /* Short lived fake pipeline: we don't need to reupload the compute shaders,
          * as we do for the gfx ones so just create a temp pipeline to be able to
          * call si_sqtt_register_pipeline, and then drop it.
          */
         struct si_sqtt_fake_pipeline pipeline = { 0 };
         pipeline.code_hash = pipeline_code_hash;
         pipeline.bo = program->shader.bo;

         si_sqtt_register_pipeline(sctx, &pipeline, true);
      }

      si_sqtt_describe_pipeline_bind(sctx, pipeline_code_hash, 1);
   }
}

static void si_set_global_binding(struct pipe_context *ctx, unsigned first, unsigned n,
                                  struct pipe_resource **resources, uint32_t **handles)
{
   unsigned i;
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_compute *program = sctx->cs_shader_state.program;

   if (first + n > program->max_global_buffers) {
      unsigned old_max = program->max_global_buffers;
      program->max_global_buffers = first + n;
      program->global_buffers = realloc(
         program->global_buffers, program->max_global_buffers * sizeof(program->global_buffers[0]));
      if (!program->global_buffers) {
         fprintf(stderr, "radeonsi: failed to allocate compute global_buffers\n");
         return;
      }

      memset(&program->global_buffers[old_max], 0,
             (program->max_global_buffers - old_max) * sizeof(program->global_buffers[0]));
   }

   if (!resources) {
      for (i = 0; i < n; i++) {
         pipe_resource_reference(&program->global_buffers[first + i], NULL);
      }
      return;
   }

   for (i = 0; i < n; i++) {
      uint64_t va;
      uint32_t offset;
      pipe_resource_reference(&program->global_buffers[first + i], resources[i]);
      va = si_resource(resources[i])->gpu_address;
      offset = util_le32_to_cpu(*handles[i]);
      va += offset;
      va = util_cpu_to_le64(va);
      memcpy(handles[i], &va, sizeof(va));
   }
}

static bool si_setup_compute_scratch_buffer(struct si_context *sctx, struct si_shader *shader)
{
   uint64_t scratch_bo_size, scratch_needed;
   scratch_bo_size = 0;
   scratch_needed = sctx->max_seen_compute_scratch_bytes_per_wave * sctx->screen->info.max_scratch_waves;
   if (sctx->compute_scratch_buffer)
      scratch_bo_size = sctx->compute_scratch_buffer->b.b.width0;

   if (scratch_bo_size < scratch_needed) {
      si_resource_reference(&sctx->compute_scratch_buffer, NULL);

      sctx->compute_scratch_buffer =
         si_aligned_buffer_create(&sctx->screen->b,
                                  PIPE_RESOURCE_FLAG_UNMAPPABLE | SI_RESOURCE_FLAG_DRIVER_INTERNAL |
                                  SI_RESOURCE_FLAG_DISCARDABLE,
                                  PIPE_USAGE_DEFAULT,
                                  scratch_needed, sctx->screen->info.pte_fragment_size);

      if (!sctx->compute_scratch_buffer)
         return false;
   }

   if (sctx->compute_scratch_buffer != shader->scratch_bo && scratch_needed) {
      if (sctx->gfx_level < GFX11 &&
          (sctx->family < CHIP_GFX940 || sctx->screen->info.has_graphics)) {
         uint64_t scratch_va = sctx->compute_scratch_buffer->gpu_address;

         if (!si_shader_binary_upload(sctx->screen, shader, scratch_va))
            return false;
      }
      si_resource_reference(&shader->scratch_bo, sctx->compute_scratch_buffer);
   }

   return true;
}

static bool si_switch_compute_shader(struct si_context *sctx, struct si_compute *program,
                                     struct si_shader *shader, const amd_kernel_code_t *code_object,
                                     unsigned offset, bool *prefetch, unsigned variable_shared_size)
{
   struct radeon_cmdbuf *cs = &sctx->gfx_cs;
   struct ac_shader_config inline_config = {0};
   const struct ac_shader_config *config;
   unsigned rsrc2;
   uint64_t shader_va;
   unsigned stage = shader->selector->info.base.stage;

   *prefetch = false;

   assert(variable_shared_size == 0 || stage == MESA_SHADER_KERNEL || program->ir_type == PIPE_SHADER_IR_NATIVE);
   if (sctx->cs_shader_state.emitted_program == program && sctx->cs_shader_state.offset == offset &&
       sctx->cs_shader_state.variable_shared_size == variable_shared_size)
      return true;

   if (program->ir_type != PIPE_SHADER_IR_NATIVE) {
      config = &shader->config;
   } else {
      code_object_to_config(code_object, &inline_config);
      config = &inline_config;
   }
   /* copy rsrc2 so we don't have to change it inside the si_shader object */
   rsrc2 = config->rsrc2;

   /* only do this for OpenCL */
   if (program->ir_type == PIPE_SHADER_IR_NATIVE || stage == MESA_SHADER_KERNEL) {
      unsigned shared_size = program->sel.info.base.shared_size + variable_shared_size;
      unsigned lds_blocks;

      /* Clover uses the compute API differently than other frontends and expects drivers to parse
       * the shared_size out of the shader headers.
       */
      if (program->ir_type == PIPE_SHADER_IR_NATIVE) {
         lds_blocks = config->lds_size;
      } else {
         lds_blocks = 0;
      }

      /* XXX: We are over allocating LDS.  For GFX6, the shader reports
       * LDS in blocks of 256 bytes, so if there are 4 bytes lds
       * allocated in the shader and 4 bytes allocated by the state
       * tracker, then we will set LDS_SIZE to 512 bytes rather than 256.
       */
      if (sctx->gfx_level <= GFX6) {
         lds_blocks += align(shared_size, 256) >> 8;
      } else {
         lds_blocks += align(shared_size, 512) >> 9;
      }

      /* TODO: use si_multiwave_lds_size_workaround */
      assert(lds_blocks <= 0xFF);

      rsrc2 &= C_00B84C_LDS_SIZE;
      rsrc2 |= S_00B84C_LDS_SIZE(lds_blocks);
   }

   unsigned tmpring_size;
   ac_get_scratch_tmpring_size(&sctx->screen->info,
                               config->scratch_bytes_per_wave,
                               &sctx->max_seen_compute_scratch_bytes_per_wave, &tmpring_size);

   if (!si_setup_compute_scratch_buffer(sctx, shader))
      return false;

   if (shader->scratch_bo) {
      radeon_add_to_buffer_list(sctx, &sctx->gfx_cs, shader->scratch_bo,
                                RADEON_USAGE_READWRITE | RADEON_PRIO_SCRATCH_BUFFER);
   }

   shader_va = shader->bo->gpu_address + offset;
   if (program->ir_type == PIPE_SHADER_IR_NATIVE) {
      /* Shader code is placed after the amd_kernel_code_t
       * struct. */
      shader_va += sizeof(amd_kernel_code_t);
   }

   radeon_add_to_buffer_list(sctx, &sctx->gfx_cs, shader->bo,
                             RADEON_USAGE_READ | RADEON_PRIO_SHADER_BINARY);

   if (sctx->screen->info.has_set_sh_pairs_packed) {
      gfx11_push_compute_sh_reg(R_00B830_COMPUTE_PGM_LO, shader_va >> 8);
      gfx11_opt_push_compute_sh_reg(R_00B848_COMPUTE_PGM_RSRC1,
                                    SI_TRACKED_COMPUTE_PGM_RSRC1, config->rsrc1);
      gfx11_opt_push_compute_sh_reg(R_00B84C_COMPUTE_PGM_RSRC2,
                                    SI_TRACKED_COMPUTE_PGM_RSRC2, rsrc2);
      gfx11_opt_push_compute_sh_reg(R_00B8A0_COMPUTE_PGM_RSRC3,
                                    SI_TRACKED_COMPUTE_PGM_RSRC3,
                                    S_00B8A0_INST_PREF_SIZE(si_get_shader_prefetch_size(shader)));
      gfx11_opt_push_compute_sh_reg(R_00B860_COMPUTE_TMPRING_SIZE,
                                    SI_TRACKED_COMPUTE_TMPRING_SIZE, tmpring_size);
      if (shader->scratch_bo) {
         gfx11_opt_push_compute_sh_reg(R_00B840_COMPUTE_DISPATCH_SCRATCH_BASE_LO,
                                       SI_TRACKED_COMPUTE_DISPATCH_SCRATCH_BASE_LO,
                                       sctx->compute_scratch_buffer->gpu_address >> 8);
         gfx11_opt_push_compute_sh_reg(R_00B844_COMPUTE_DISPATCH_SCRATCH_BASE_HI,
                                       SI_TRACKED_COMPUTE_DISPATCH_SCRATCH_BASE_HI,
                                       sctx->compute_scratch_buffer->gpu_address >> 40);
      }
   } else {
      radeon_begin(cs);
      radeon_set_sh_reg(R_00B830_COMPUTE_PGM_LO, shader_va >> 8);
      radeon_opt_set_sh_reg2(sctx, R_00B848_COMPUTE_PGM_RSRC1,
                             SI_TRACKED_COMPUTE_PGM_RSRC1,
                             config->rsrc1, rsrc2);
      radeon_opt_set_sh_reg(sctx, R_00B860_COMPUTE_TMPRING_SIZE,
                            SI_TRACKED_COMPUTE_TMPRING_SIZE, tmpring_size);

      if (shader->scratch_bo &&
          (sctx->gfx_level >= GFX11 ||
           (sctx->family >= CHIP_GFX940 && !sctx->screen->info.has_graphics))) {
         radeon_opt_set_sh_reg2(sctx, R_00B840_COMPUTE_DISPATCH_SCRATCH_BASE_LO,
                                SI_TRACKED_COMPUTE_DISPATCH_SCRATCH_BASE_LO,
                                sctx->compute_scratch_buffer->gpu_address >> 8,
                                sctx->compute_scratch_buffer->gpu_address >> 40);
      }

      if (sctx->gfx_level >= GFX11) {
         radeon_opt_set_sh_reg(sctx, R_00B8A0_COMPUTE_PGM_RSRC3,
                               SI_TRACKED_COMPUTE_PGM_RSRC3,
                               S_00B8A0_INST_PREF_SIZE(si_get_shader_prefetch_size(shader)));
      }
      radeon_end();
   }

   COMPUTE_DBG(sctx->screen,
               "COMPUTE_PGM_RSRC1: 0x%08x "
               "COMPUTE_PGM_RSRC2: 0x%08x\n",
               config->rsrc1, config->rsrc2);

   sctx->cs_shader_state.emitted_program = program;
   sctx->cs_shader_state.offset = offset;
   sctx->cs_shader_state.variable_shared_size = variable_shared_size;

   *prefetch = true;
   return true;
}

static void setup_scratch_rsrc_user_sgprs(struct si_context *sctx,
                                          const amd_kernel_code_t *code_object, unsigned user_sgpr)
{
   struct radeon_cmdbuf *cs = &sctx->gfx_cs;
   uint64_t scratch_va = sctx->compute_scratch_buffer->gpu_address;

   unsigned max_private_element_size =
      AMD_HSA_BITS_GET(code_object->code_properties, AMD_CODE_PROPERTY_PRIVATE_ELEMENT_SIZE);

   uint32_t scratch_dword0 = scratch_va & 0xffffffff;
   uint32_t scratch_dword1 = S_008F04_BASE_ADDRESS_HI(scratch_va >> 32);

   if (sctx->gfx_level >= GFX11)
      scratch_dword1 |= S_008F04_SWIZZLE_ENABLE_GFX11(1);
   else
      scratch_dword1 |= S_008F04_SWIZZLE_ENABLE_GFX6(1);

   /* Disable address clamping */
   uint32_t scratch_dword2 = 0xffffffff;
   uint32_t index_stride = sctx->cs_shader_state.program->shader.wave_size == 32 ? 2 : 3;
   uint32_t scratch_dword3 = S_008F0C_INDEX_STRIDE(index_stride) | S_008F0C_ADD_TID_ENABLE(1);

   if (sctx->gfx_level >= GFX9) {
      assert(max_private_element_size == 1); /* only 4 bytes on GFX9 */
   } else {
      scratch_dword3 |= S_008F0C_ELEMENT_SIZE(max_private_element_size);

      if (sctx->gfx_level < GFX8) {
         /* BUF_DATA_FORMAT is ignored, but it cannot be
          * BUF_DATA_FORMAT_INVALID. */
         scratch_dword3 |= S_008F0C_DATA_FORMAT(V_008F0C_BUF_DATA_FORMAT_8);
      }
   }

   radeon_begin(cs);
   radeon_set_sh_reg_seq(R_00B900_COMPUTE_USER_DATA_0 + (user_sgpr * 4), 4);
   radeon_emit(scratch_dword0);
   radeon_emit(scratch_dword1);
   radeon_emit(scratch_dword2);
   radeon_emit(scratch_dword3);
   radeon_end();
}

static void si_setup_user_sgprs_co_v2(struct si_context *sctx, const amd_kernel_code_t *code_object,
                                      const struct pipe_grid_info *info, uint64_t kernel_args_va)
{
   struct si_compute *program = sctx->cs_shader_state.program;
   struct radeon_cmdbuf *cs = &sctx->gfx_cs;

   static const enum amd_code_property_mask_t workgroup_count_masks[] = {
      AMD_CODE_PROPERTY_ENABLE_SGPR_GRID_WORKGROUP_COUNT_X,
      AMD_CODE_PROPERTY_ENABLE_SGPR_GRID_WORKGROUP_COUNT_Y,
      AMD_CODE_PROPERTY_ENABLE_SGPR_GRID_WORKGROUP_COUNT_Z};

   unsigned i, user_sgpr = 0;
   if (AMD_HSA_BITS_GET(code_object->code_properties,
                        AMD_CODE_PROPERTY_ENABLE_SGPR_PRIVATE_SEGMENT_BUFFER)) {
      if (code_object->workitem_private_segment_byte_size > 0) {
         setup_scratch_rsrc_user_sgprs(sctx, code_object, user_sgpr);
      }
      user_sgpr += 4;
   }

   radeon_begin(cs);

   if (AMD_HSA_BITS_GET(code_object->code_properties, AMD_CODE_PROPERTY_ENABLE_SGPR_DISPATCH_PTR)) {
      struct dispatch_packet dispatch;
      unsigned dispatch_offset;
      struct si_resource *dispatch_buf = NULL;
      uint64_t dispatch_va;

      /* Upload dispatch ptr */
      memset(&dispatch, 0, sizeof(dispatch));

      dispatch.workgroup_size_x = util_cpu_to_le16(info->block[0]);
      dispatch.workgroup_size_y = util_cpu_to_le16(info->block[1]);
      dispatch.workgroup_size_z = util_cpu_to_le16(info->block[2]);

      dispatch.grid_size_x = util_cpu_to_le32(info->grid[0] * info->block[0]);
      dispatch.grid_size_y = util_cpu_to_le32(info->grid[1] * info->block[1]);
      dispatch.grid_size_z = util_cpu_to_le32(info->grid[2] * info->block[2]);

      dispatch.group_segment_size =
         util_cpu_to_le32(program->sel.info.base.shared_size + info->variable_shared_mem);

      dispatch.kernarg_address = util_cpu_to_le64(kernel_args_va);

      u_upload_data(sctx->b.const_uploader, 0, sizeof(dispatch), 256, &dispatch, &dispatch_offset,
                    (struct pipe_resource **)&dispatch_buf);

      if (!dispatch_buf) {
         fprintf(stderr, "Error: Failed to allocate dispatch "
                         "packet.");
      }
      radeon_add_to_buffer_list(sctx, &sctx->gfx_cs, dispatch_buf,
                                RADEON_USAGE_READ | RADEON_PRIO_CONST_BUFFER);

      dispatch_va = dispatch_buf->gpu_address + dispatch_offset;

      radeon_set_sh_reg_seq(R_00B900_COMPUTE_USER_DATA_0 + (user_sgpr * 4), 2);
      radeon_emit(dispatch_va);
      radeon_emit(S_008F04_BASE_ADDRESS_HI(dispatch_va >> 32) | S_008F04_STRIDE(0));

      si_resource_reference(&dispatch_buf, NULL);
      user_sgpr += 2;
   }

   if (AMD_HSA_BITS_GET(code_object->code_properties,
                        AMD_CODE_PROPERTY_ENABLE_SGPR_KERNARG_SEGMENT_PTR)) {
      radeon_set_sh_reg_seq(R_00B900_COMPUTE_USER_DATA_0 + (user_sgpr * 4), 2);
      radeon_emit(kernel_args_va);
      radeon_emit(S_008F04_BASE_ADDRESS_HI(kernel_args_va >> 32) | S_008F04_STRIDE(0));
      user_sgpr += 2;
   }

   for (i = 0; i < 3 && user_sgpr < 16; i++) {
      if (code_object->code_properties & workgroup_count_masks[i]) {
         radeon_set_sh_reg_seq(R_00B900_COMPUTE_USER_DATA_0 + (user_sgpr * 4), 1);
         radeon_emit(info->grid[i]);
         user_sgpr += 1;
      }
   }
   radeon_end();
}

static bool si_upload_compute_input(struct si_context *sctx, const amd_kernel_code_t *code_object,
                                    const struct pipe_grid_info *info)
{
   struct si_compute *program = sctx->cs_shader_state.program;
   struct si_resource *input_buffer = NULL;
   uint32_t kernel_args_offset = 0;
   uint32_t *kernel_args;
   void *kernel_args_ptr;
   uint64_t kernel_args_va;

   u_upload_alloc(sctx->b.const_uploader, 0, program->input_size,
                  sctx->screen->info.tcc_cache_line_size, &kernel_args_offset,
                  (struct pipe_resource **)&input_buffer, &kernel_args_ptr);

   if (unlikely(!kernel_args_ptr))
      return false;

   kernel_args = (uint32_t *)kernel_args_ptr;
   kernel_args_va = input_buffer->gpu_address + kernel_args_offset;

   memcpy(kernel_args, info->input, program->input_size);

   for (unsigned i = 0; i < program->input_size / 4; i++) {
      COMPUTE_DBG(sctx->screen, "input %u : %u\n", i, kernel_args[i]);
   }

   radeon_add_to_buffer_list(sctx, &sctx->gfx_cs, input_buffer,
                             RADEON_USAGE_READ | RADEON_PRIO_CONST_BUFFER);

   si_setup_user_sgprs_co_v2(sctx, code_object, info, kernel_args_va);
   si_resource_reference(&input_buffer, NULL);
   return true;
}

static void si_setup_nir_user_data(struct si_context *sctx, const struct pipe_grid_info *info)
{
   struct si_compute *program = sctx->cs_shader_state.program;
   struct si_shader_selector *sel = &program->sel;
   struct radeon_cmdbuf *cs = &sctx->gfx_cs;
   unsigned grid_size_reg = R_00B900_COMPUTE_USER_DATA_0 + 4 * SI_NUM_RESOURCE_SGPRS;
   unsigned block_size_reg = grid_size_reg +
                             /* 12 bytes = 3 dwords. */
                             12 * sel->info.uses_grid_size;
   unsigned cs_user_data_reg = block_size_reg + 4 * program->sel.info.uses_variable_block_size;

   if (sel->info.uses_grid_size && info->indirect) {
      for (unsigned i = 0; i < 3; ++i) {
         si_cp_copy_data(sctx, &sctx->gfx_cs, COPY_DATA_REG, NULL, (grid_size_reg >> 2) + i,
                         COPY_DATA_SRC_MEM, si_resource(info->indirect),
                         info->indirect_offset + 4 * i);
      }
   }

   if (sctx->screen->info.has_set_sh_pairs_packed) {
      if (sel->info.uses_grid_size && !info->indirect) {
         gfx11_push_compute_sh_reg(grid_size_reg, info->grid[0]);
         gfx11_push_compute_sh_reg(grid_size_reg + 4, info->grid[1]);
         gfx11_push_compute_sh_reg(grid_size_reg + 8, info->grid[2]);
      }

      if (sel->info.uses_variable_block_size) {
         uint32_t value = info->block[0] | (info->block[1] << 10) | (info->block[2] << 20);
         gfx11_push_compute_sh_reg(block_size_reg, value);
      }

      if (sel->info.base.cs.user_data_components_amd) {
         unsigned num = sel->info.base.cs.user_data_components_amd;
         for (unsigned i = 0; i < num; i++)
            gfx11_push_compute_sh_reg(cs_user_data_reg + i * 4, sctx->cs_user_data[i]);
      }
   } else {
      radeon_begin(cs);

      if (sel->info.uses_grid_size && !info->indirect) {
         radeon_set_sh_reg_seq(grid_size_reg, 3);
         radeon_emit(info->grid[0]);
         radeon_emit(info->grid[1]);
         radeon_emit(info->grid[2]);
      }

      if (sel->info.uses_variable_block_size) {
         uint32_t value = info->block[0] | (info->block[1] << 10) | (info->block[2] << 20);
         radeon_set_sh_reg(block_size_reg, value);
      }

      if (sel->info.base.cs.user_data_components_amd) {
         unsigned num = sel->info.base.cs.user_data_components_amd;
         radeon_set_sh_reg_seq(cs_user_data_reg, num);
         radeon_emit_array(sctx->cs_user_data, num);
      }
      radeon_end();
   }
}

static void si_emit_dispatch_packets(struct si_context *sctx, const struct pipe_grid_info *info)
{
   struct si_screen *sscreen = sctx->screen;
   struct radeon_cmdbuf *cs = &sctx->gfx_cs;
   bool render_cond_bit = sctx->render_cond_enabled;
   unsigned threads_per_threadgroup = info->block[0] * info->block[1] * info->block[2];
   unsigned waves_per_threadgroup =
      DIV_ROUND_UP(threads_per_threadgroup, sctx->cs_shader_state.program->shader.wave_size);
   unsigned threadgroups_per_cu = 1;

   if (sctx->gfx_level >= GFX10 && waves_per_threadgroup == 1)
      threadgroups_per_cu = 2;

   if (unlikely(sctx->sqtt_enabled)) {
      if (info->indirect) {
         si_sqtt_write_event_marker(sctx, &sctx->gfx_cs,
                                    EventCmdDispatchIndirect,
                                    UINT_MAX, UINT_MAX, UINT_MAX);
      } else {
         si_write_event_with_dims_marker(sctx, &sctx->gfx_cs,
                                         EventCmdDispatch,
                                         info->grid[0], info->grid[1], info->grid[2]);
      }
   }

   radeon_begin(cs);
   unsigned compute_resource_limits =
      ac_get_compute_resource_limits(&sscreen->info, waves_per_threadgroup,
                                     sctx->cs_max_waves_per_sh,
                                     threadgroups_per_cu);

   if (sctx->screen->info.has_set_sh_pairs_packed) {
      gfx11_opt_push_compute_sh_reg(R_00B854_COMPUTE_RESOURCE_LIMITS,
                                    SI_TRACKED_COMPUTE_RESOURCE_LIMITS,
                                    compute_resource_limits);
   } else {
      radeon_opt_set_sh_reg(sctx, R_00B854_COMPUTE_RESOURCE_LIMITS,
                            SI_TRACKED_COMPUTE_RESOURCE_LIMITS,
                            compute_resource_limits);
   }

   unsigned dispatch_initiator = S_00B800_COMPUTE_SHADER_EN(1) | S_00B800_FORCE_START_AT_000(1) |
                                 /* If the KMD allows it (there is a KMD hw register for it),
                                  * allow launching waves out-of-order. (same as Vulkan)
                                  * Not available in gfx940.
                                  */
                                 S_00B800_ORDER_MODE(sctx->gfx_level >= GFX7 &&
                                                     (sctx->family < CHIP_GFX940 || sctx->screen->info.has_graphics)) |
                                 S_00B800_CS_W32_EN(sctx->cs_shader_state.program->shader.wave_size == 32);

   const uint *last_block = info->last_block;
   bool partial_block_en = last_block[0] || last_block[1] || last_block[2];
   uint32_t num_threads[3];

   num_threads[0] = S_00B81C_NUM_THREAD_FULL(info->block[0]);
   num_threads[1] = S_00B820_NUM_THREAD_FULL(info->block[1]);
   num_threads[2] = S_00B824_NUM_THREAD_FULL(info->block[2]);

   if (partial_block_en) {
      unsigned partial[3];

      /* If no partial_block, these should be an entire block size, not 0. */
      partial[0] = last_block[0] ? last_block[0] : info->block[0];
      partial[1] = last_block[1] ? last_block[1] : info->block[1];
      partial[2] = last_block[2] ? last_block[2] : info->block[2];

      num_threads[0] |= S_00B81C_NUM_THREAD_PARTIAL(partial[0]);
      num_threads[1] |= S_00B820_NUM_THREAD_PARTIAL(partial[1]);
      num_threads[2] |= S_00B824_NUM_THREAD_PARTIAL(partial[2]);

      dispatch_initiator |= S_00B800_PARTIAL_TG_EN(1);
   }

   if (sctx->screen->info.has_set_sh_pairs_packed) {
      gfx11_opt_push_compute_sh_reg(R_00B81C_COMPUTE_NUM_THREAD_X,
                                    SI_TRACKED_COMPUTE_NUM_THREAD_X, num_threads[0]);
      gfx11_opt_push_compute_sh_reg(R_00B820_COMPUTE_NUM_THREAD_Y,
                                    SI_TRACKED_COMPUTE_NUM_THREAD_Y, num_threads[1]);
      gfx11_opt_push_compute_sh_reg(R_00B824_COMPUTE_NUM_THREAD_Z,
                                    SI_TRACKED_COMPUTE_NUM_THREAD_Z, num_threads[2]);
   } else {
      radeon_opt_set_sh_reg3(sctx, R_00B81C_COMPUTE_NUM_THREAD_X,
                             SI_TRACKED_COMPUTE_NUM_THREAD_X,
                             num_threads[0], num_threads[1], num_threads[2]);
   }

   if (sctx->gfx_level >= GFX11) {
      radeon_end();
      si_emit_buffered_compute_sh_regs(sctx);
      radeon_begin_again(cs);
   }

   if (info->indirect) {
      uint64_t base_va = si_resource(info->indirect)->gpu_address;

      radeon_add_to_buffer_list(sctx, &sctx->gfx_cs, si_resource(info->indirect),
                                RADEON_USAGE_READ | RADEON_PRIO_DRAW_INDIRECT);

      radeon_emit(PKT3(PKT3_SET_BASE, 2, 0) | PKT3_SHADER_TYPE_S(1));
      radeon_emit(1);
      radeon_emit(base_va);
      radeon_emit(base_va >> 32);

      radeon_emit(PKT3(PKT3_DISPATCH_INDIRECT, 1, render_cond_bit) | PKT3_SHADER_TYPE_S(1));
      radeon_emit(info->indirect_offset);
      radeon_emit(dispatch_initiator);
   } else {
      radeon_emit(PKT3(PKT3_DISPATCH_DIRECT, 3, render_cond_bit) | PKT3_SHADER_TYPE_S(1));
      radeon_emit(info->grid[0]);
      radeon_emit(info->grid[1]);
      radeon_emit(info->grid[2]);
      radeon_emit(dispatch_initiator);
   }

   if (unlikely(sctx->sqtt_enabled && sctx->gfx_level >= GFX9)) {
      radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(EVENT_TYPE(V_028A90_THREAD_TRACE_MARKER) | EVENT_INDEX(0));
   }
   radeon_end();
}

static bool si_check_needs_implicit_sync(struct si_context *sctx)
{
   /* If the compute shader is going to read from a texture/image written by a
    * previous draw, we must wait for its completion before continuing.
    * Buffers and image stores (from the draw) are not taken into consideration
    * because that's the app responsibility.
    *
    * The OpenGL 4.6 spec says:
    *
    *    buffer object and texture stores performed by shaders are not
    *    automatically synchronized
    *
    * TODO: Bindless textures are not handled, and thus are not synchronized.
    */
   struct si_shader_info *info = &sctx->cs_shader_state.program->sel.info;
   struct si_samplers *samplers = &sctx->samplers[PIPE_SHADER_COMPUTE];
   unsigned mask = samplers->enabled_mask & info->base.textures_used[0];

   while (mask) {
      int i = u_bit_scan(&mask);
      struct si_sampler_view *sview = (struct si_sampler_view *)samplers->views[i];

      struct si_resource *res = si_resource(sview->base.texture);
      if (sctx->ws->cs_is_buffer_referenced(&sctx->gfx_cs, res->buf,
                                            RADEON_USAGE_NEEDS_IMPLICIT_SYNC))
         return true;
   }

   struct si_images *images = &sctx->images[PIPE_SHADER_COMPUTE];
   mask = u_bit_consecutive(0, info->base.num_images) & images->enabled_mask;

   while (mask) {
      int i = u_bit_scan(&mask);
      struct pipe_image_view *sview = &images->views[i];

      struct si_resource *res = si_resource(sview->resource);
      if (sctx->ws->cs_is_buffer_referenced(&sctx->gfx_cs, res->buf,
                                            RADEON_USAGE_NEEDS_IMPLICIT_SYNC))
         return true;
   }
   return false;
}

static void si_launch_grid(struct pipe_context *ctx, const struct pipe_grid_info *info)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_screen *sscreen = sctx->screen;
   struct si_compute *program = sctx->cs_shader_state.program;
   const amd_kernel_code_t *code_object = si_compute_get_code_object(program, info->pc);
   int i;
   bool cs_regalloc_hang = sscreen->info.has_cs_regalloc_hang_bug &&
                           info->block[0] * info->block[1] * info->block[2] > 256;

   if (cs_regalloc_hang) {
      sctx->flags |= SI_CONTEXT_PS_PARTIAL_FLUSH | SI_CONTEXT_CS_PARTIAL_FLUSH;
      si_mark_atom_dirty(sctx, &sctx->atoms.s.cache_flush);
   }

   if (program->ir_type != PIPE_SHADER_IR_NATIVE && program->shader.compilation_failed)
      return;

   si_check_dirty_buffers_textures(sctx);

   if (sctx->has_graphics) {
      if (sctx->last_num_draw_calls != sctx->num_draw_calls) {
         si_update_fb_dirtiness_after_rendering(sctx);
         sctx->last_num_draw_calls = sctx->num_draw_calls;

         if (sctx->force_cb_shader_coherent || si_check_needs_implicit_sync(sctx))
            si_make_CB_shader_coherent(sctx, 0,
                                       sctx->framebuffer.CB_has_shader_readable_metadata,
                                       sctx->framebuffer.all_DCC_pipe_aligned);
      }

      if (sctx->gfx_level < GFX11)
         gfx6_decompress_textures(sctx, 1 << PIPE_SHADER_COMPUTE);
      else
         gfx11_decompress_textures(sctx, 1 << PIPE_SHADER_COMPUTE);
   }

   if (info->indirect) {
      /* Indirect buffers use TC L2 on GFX9, but not older hw. */
      if (sctx->gfx_level <= GFX8 && si_resource(info->indirect)->TC_L2_dirty) {
         sctx->flags |= SI_CONTEXT_WB_L2;
         si_mark_atom_dirty(sctx, &sctx->atoms.s.cache_flush);
         si_resource(info->indirect)->TC_L2_dirty = false;
      }
   }

   si_need_gfx_cs_space(sctx, 0);

   /* If we're using a secure context, determine if cs must be secure or not */
   if (unlikely(radeon_uses_secure_bos(sctx->ws))) {
      bool secure = si_compute_resources_check_encrypted(sctx);
      if (secure != sctx->ws->cs_is_secure(&sctx->gfx_cs)) {
         si_flush_gfx_cs(sctx, RADEON_FLUSH_ASYNC_START_NEXT_GFX_IB_NOW |
                               RADEON_FLUSH_TOGGLE_SECURE_SUBMISSION,
                         NULL);
      }
   }
   
   if (u_trace_perfetto_active(&sctx->ds.trace_context))
      trace_si_begin_compute(&sctx->trace);
   
   if (sctx->bo_list_add_all_compute_resources)
      si_compute_resources_add_all_to_bo_list(sctx);

   /* Skipping setting redundant registers on compute queues breaks compute. */
   if (!sctx->has_graphics) {
      BITSET_CLEAR_RANGE(sctx->tracked_regs.reg_saved_mask,
                         SI_FIRST_TRACKED_OTHER_REG, SI_NUM_ALL_TRACKED_REGS - 1);
   }

   /* First emit registers. */
   bool prefetch;
   if (!si_switch_compute_shader(sctx, program, &program->shader, code_object, info->pc, &prefetch,
                                 info->variable_shared_mem))
      return;

   si_emit_compute_shader_pointers(sctx);

   if (program->ir_type == PIPE_SHADER_IR_NATIVE &&
       unlikely(!si_upload_compute_input(sctx, code_object, info)))
      return;

   /* Global buffers */
   for (i = 0; i < program->max_global_buffers; i++) {
      struct si_resource *buffer = si_resource(program->global_buffers[i]);
      if (!buffer) {
         continue;
      }
      radeon_add_to_buffer_list(sctx, &sctx->gfx_cs, buffer,
                                RADEON_USAGE_READWRITE | RADEON_PRIO_SHADER_RW_BUFFER);
   }

   /* Registers that are not read from memory should be set before this: */
   if (sctx->flags)
      si_emit_cache_flush_direct(sctx);

   if (sctx->has_graphics && si_is_atom_dirty(sctx, &sctx->atoms.s.render_cond)) {
      sctx->atoms.s.render_cond.emit(sctx, -1);
      si_set_atom_dirty(sctx, &sctx->atoms.s.render_cond, false);
   }

   /* Prefetch the compute shader to L2. */
   if (sctx->gfx_level >= GFX7 && prefetch)
      si_cp_dma_prefetch(sctx, &program->shader.bo->b.b, 0, program->shader.bo->b.b.width0);

   if (program->ir_type != PIPE_SHADER_IR_NATIVE)
      si_setup_nir_user_data(sctx, info);

   si_emit_dispatch_packets(sctx, info);

   if (unlikely(sctx->current_saved_cs)) {
      si_trace_emit(sctx);
      si_log_compute_state(sctx, sctx->log);
   }

   /* Mark displayable DCC as dirty for bound images. */
   unsigned display_dcc_store_mask = sctx->images[PIPE_SHADER_COMPUTE].display_dcc_store_mask &
                               BITFIELD_MASK(program->sel.info.base.num_images);
   while (display_dcc_store_mask) {
      struct si_texture *tex = (struct si_texture *)
         sctx->images[PIPE_SHADER_COMPUTE].views[u_bit_scan(&display_dcc_store_mask)].resource;

      si_mark_display_dcc_dirty(sctx, tex);
   }

   /* TODO: Bindless images don't set displayable_dcc_dirty after image stores. */

   sctx->compute_is_busy = true;
   sctx->num_compute_calls++;

   if (u_trace_perfetto_active(&sctx->ds.trace_context))
      trace_si_end_compute(&sctx->trace, info->grid[0], info->grid[1], info->grid[2]);
   
   if (cs_regalloc_hang) {
      sctx->flags |= SI_CONTEXT_CS_PARTIAL_FLUSH;
      si_mark_atom_dirty(sctx, &sctx->atoms.s.cache_flush);
   }
}

void si_destroy_compute(struct si_compute *program)
{
   struct si_shader_selector *sel = &program->sel;

   if (program->ir_type != PIPE_SHADER_IR_NATIVE) {
      util_queue_drop_job(&sel->screen->shader_compiler_queue, &sel->ready);
      util_queue_fence_destroy(&sel->ready);
   }

   for (unsigned i = 0; i < program->max_global_buffers; i++)
      pipe_resource_reference(&program->global_buffers[i], NULL);
   FREE(program->global_buffers);

   si_shader_destroy(&program->shader);
   ralloc_free(program->sel.nir);
   FREE(program);
}

static void si_delete_compute_state(struct pipe_context *ctx, void *state)
{
   struct si_compute *program = (struct si_compute *)state;
   struct si_context *sctx = (struct si_context *)ctx;

   if (!state)
      return;

   if (program == sctx->cs_shader_state.program)
      sctx->cs_shader_state.program = NULL;

   if (program == sctx->cs_shader_state.emitted_program)
      sctx->cs_shader_state.emitted_program = NULL;

   si_compute_reference(&program, NULL);
}

static void si_set_compute_resources(struct pipe_context *ctx_, unsigned start, unsigned count,
                                     struct pipe_surface **surfaces)
{
}

void si_init_compute_functions(struct si_context *sctx)
{
   sctx->b.create_compute_state = si_create_compute_state;
   sctx->b.delete_compute_state = si_delete_compute_state;
   sctx->b.bind_compute_state = si_bind_compute_state;
   sctx->b.get_compute_state_info = si_get_compute_state_info;
   sctx->b.set_compute_resources = si_set_compute_resources;
   sctx->b.set_global_binding = si_set_global_binding;
   sctx->b.launch_grid = si_launch_grid;
}
