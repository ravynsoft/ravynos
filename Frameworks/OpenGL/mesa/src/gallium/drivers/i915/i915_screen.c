/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
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

#include "compiler/nir/nir.h"
#include "draw/draw_context.h"
#include "nir/nir_to_tgsi.h"
#include "util/format/u_format.h"
#include "util/format/u_format_s3tc.h"
#include "util/os_misc.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_screen.h"
#include "util/u_string.h"

#include "i915_context.h"
#include "i915_debug.h"
#include "i915_fpc.h"
#include "i915_public.h"
#include "i915_reg.h"
#include "i915_resource.h"
#include "i915_screen.h"
#include "i915_winsys.h"

/*
 * Probe functions
 */

static const char *
i915_get_vendor(struct pipe_screen *screen)
{
   return "Mesa Project";
}

static const char *
i915_get_device_vendor(struct pipe_screen *screen)
{
   return "Intel";
}

static const char *
i915_get_name(struct pipe_screen *screen)
{
   static char buffer[128];
   const char *chipset;

   switch (i915_screen(screen)->iws->pci_id) {
   case PCI_CHIP_I915_G:
      chipset = "915G";
      break;
   case PCI_CHIP_I915_GM:
      chipset = "915GM";
      break;
   case PCI_CHIP_I945_G:
      chipset = "945G";
      break;
   case PCI_CHIP_I945_GM:
      chipset = "945GM";
      break;
   case PCI_CHIP_I945_GME:
      chipset = "945GME";
      break;
   case PCI_CHIP_G33_G:
      chipset = "G33";
      break;
   case PCI_CHIP_Q35_G:
      chipset = "Q35";
      break;
   case PCI_CHIP_Q33_G:
      chipset = "Q33";
      break;
   case PCI_CHIP_PINEVIEW_G:
      chipset = "Pineview G";
      break;
   case PCI_CHIP_PINEVIEW_M:
      chipset = "Pineview M";
      break;
   default:
      chipset = "unknown";
      break;
   }

   snprintf(buffer, sizeof(buffer), "i915 (chipset: %s)", chipset);
   return buffer;
}

static const nir_shader_compiler_options i915_compiler_options = {
   .fdot_replicates = true,
   .fuse_ffma32 = true,
   .lower_bitops = true, /* required for !CAP_INTEGERS nir_to_tgsi */
   .lower_extract_byte = true,
   .lower_extract_word = true,
   .lower_fdiv = true,
   .lower_fdph = true,
   .lower_flrp32 = true,
   .lower_fmod = true,
   .lower_sincos = true,
   .lower_uniforms_to_ubo = true,
   .lower_vector_cmp = true,
   .use_interpolated_input_intrinsics = true,
   .force_indirect_unrolling = nir_var_all,
   .force_indirect_unrolling_sampler = true,
   .max_unroll_iterations = 32,
   .no_integers = true,
   .has_fused_comp_and_csel = true,
};

static const struct nir_shader_compiler_options gallivm_nir_options = {
   .fdot_replicates = true,
   .lower_bitops = true, /* required for !CAP_INTEGERS nir_to_tgsi */
   .lower_scmp = true,
   .lower_flrp32 = true,
   .lower_flrp64 = true,
   .lower_fsat = true,
   .lower_bitfield_insert = true,
   .lower_bitfield_extract = true,
   .lower_fdph = true,
   .lower_ffma16 = true,
   .lower_ffma32 = true,
   .lower_ffma64 = true,
   .lower_fmod = true,
   .lower_hadd = true,
   .lower_uadd_sat = true,
   .lower_usub_sat = true,
   .lower_iadd_sat = true,
   .lower_ldexp = true,
   .lower_pack_snorm_2x16 = true,
   .lower_pack_snorm_4x8 = true,
   .lower_pack_unorm_2x16 = true,
   .lower_pack_unorm_4x8 = true,
   .lower_pack_half_2x16 = true,
   .lower_pack_split = true,
   .lower_unpack_snorm_2x16 = true,
   .lower_unpack_snorm_4x8 = true,
   .lower_unpack_unorm_2x16 = true,
   .lower_unpack_unorm_4x8 = true,
   .lower_unpack_half_2x16 = true,
   .lower_extract_byte = true,
   .lower_extract_word = true,
   .lower_uadd_carry = true,
   .lower_usub_borrow = true,
   .lower_mul_2x32_64 = true,
   .lower_ifind_msb = true,
   .max_unroll_iterations = 32,
   .use_interpolated_input_intrinsics = true,
   .lower_cs_local_index_to_id = true,
   .lower_uniforms_to_ubo = true,
   .lower_vector_cmp = true,
   .lower_device_index_to_zero = true,
   /* .support_16bit_alu = true, */
};

static const void *
i915_get_compiler_options(struct pipe_screen *pscreen, enum pipe_shader_ir ir,
                          enum pipe_shader_type shader)
{
   assert(ir == PIPE_SHADER_IR_NIR);
   if (shader == PIPE_SHADER_FRAGMENT)
      return &i915_compiler_options;
   else
      return &gallivm_nir_options;
}

static void
i915_optimize_nir(struct nir_shader *s)
{
   bool progress;

   do {
      progress = false;

      NIR_PASS_V(s, nir_lower_vars_to_ssa);

      NIR_PASS(progress, s, nir_copy_prop);
      NIR_PASS(progress, s, nir_opt_algebraic);
      NIR_PASS(progress, s, nir_opt_constant_folding);
      NIR_PASS(progress, s, nir_opt_remove_phis);
      NIR_PASS(progress, s, nir_opt_conditional_discard);
      NIR_PASS(progress, s, nir_opt_dce);
      NIR_PASS(progress, s, nir_opt_dead_cf);
      NIR_PASS(progress, s, nir_opt_cse);
      NIR_PASS(progress, s, nir_opt_find_array_copies);
      NIR_PASS(progress, s, nir_opt_if, nir_opt_if_optimize_phi_true_false);
      NIR_PASS(progress, s, nir_opt_peephole_select, ~0 /* flatten all IFs. */,
               true, true);
      NIR_PASS(progress, s, nir_opt_algebraic);
      NIR_PASS(progress, s, nir_opt_constant_folding);
      NIR_PASS(progress, s, nir_opt_shrink_stores, true);
      NIR_PASS(progress, s, nir_opt_shrink_vectors);
      NIR_PASS(progress, s, nir_opt_loop);
      NIR_PASS(progress, s, nir_opt_undef);
      NIR_PASS(progress, s, nir_opt_loop_unroll);

   } while (progress);

   NIR_PASS(progress, s, nir_remove_dead_variables, nir_var_function_temp,
            NULL);

   /* Group texture loads together to try to avoid hitting the
    * texture indirection phase limit.
    */
   NIR_PASS_V(s, nir_group_loads, nir_group_all, ~0);
}

static char *
i915_check_control_flow(nir_shader *s)
{
   if (s->info.stage == MESA_SHADER_FRAGMENT) {
      nir_function_impl *impl = nir_shader_get_entrypoint(s);
      nir_block *first = nir_start_block(impl);
      nir_cf_node *next = nir_cf_node_next(&first->cf_node);

      if (next) {
         switch (next->type) {
         case nir_cf_node_if:
            return "if/then statements not supported by i915 fragment shaders, "
                   "should have been flattened by peephole_select.";
         case nir_cf_node_loop:
            return "looping not supported i915 fragment shaders, all loops "
                   "must be statically unrollable.";
         default:
            return "Unknown control flow type";
         }
      }
   }

   return NULL;
}

static char *
i915_finalize_nir(struct pipe_screen *pscreen, void *nir)
{
   nir_shader *s = nir;

   if (s->info.stage == MESA_SHADER_FRAGMENT)
      i915_optimize_nir(s);

   /* st_program.c's parameter list optimization requires that future nir
    * variants don't reallocate the uniform storage, so we have to remove
    * uniforms that occupy storage.  But we don't want to remove samplers,
    * because they're needed for YUV variant lowering.
    */
   nir_remove_dead_derefs(s);
   nir_foreach_uniform_variable_safe (var, s) {
      if (var->data.mode == nir_var_uniform &&
          (glsl_type_get_image_count(var->type) ||
           glsl_type_get_sampler_count(var->type)))
         continue;

      exec_node_remove(&var->node);
   }
   nir_validate_shader(s, "after uniform var removal");

   nir_sweep(s);

   char *msg = i915_check_control_flow(s);
   if (msg) {
      if (I915_DBG_ON(DBG_FS) && (!s->info.internal || NIR_DEBUG(PRINT_INTERNAL))) {
         mesa_logi("failing shader:");
         nir_log_shaderi(s);
      }
      return strdup(msg);
   }

   if (s->info.stage == MESA_SHADER_FRAGMENT)
      return i915_test_fragment_shader_compile(pscreen, s);
   else
      return NULL;
}

static int
i915_get_shader_param(struct pipe_screen *screen, enum pipe_shader_type shader,
                      enum pipe_shader_cap cap)
{
   switch (cap) {
   case PIPE_SHADER_CAP_SUPPORTED_IRS:
      return (1 << PIPE_SHADER_IR_NIR) | (1 << PIPE_SHADER_IR_TGSI);

   case PIPE_SHADER_CAP_INTEGERS:
      /* mesa/st requires that this cap is the same across stages, and the FS
       * can't do ints.
       */
      return 0;

   /* i915 can't do these, and even if gallivm NIR can we call nir_to_tgsi
    * manually and TGSI can't.
    */
   case PIPE_SHADER_CAP_INT16:
   case PIPE_SHADER_CAP_FP16:
   case PIPE_SHADER_CAP_FP16_DERIVATIVES:
   case PIPE_SHADER_CAP_FP16_CONST_BUFFERS:
      return 0;

   case PIPE_SHADER_CAP_INDIRECT_TEMP_ADDR:
      /* While draw could normally handle this for the VS, the NIR lowering
       * to regs can't handle our non-native-integers, so we have to lower to
       * if ladders.
       */
      return 0;

   default:
      break;
   }

   switch (shader) {
   case PIPE_SHADER_VERTEX:
      switch (cap) {
      case PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS:
      case PIPE_SHADER_CAP_MAX_SAMPLER_VIEWS:
         return 0;
      case PIPE_SHADER_CAP_MAX_SHADER_BUFFERS:
      case PIPE_SHADER_CAP_MAX_SHADER_IMAGES:
         return 0;
      default:
         return draw_get_shader_param(shader, cap);
      }
   case PIPE_SHADER_FRAGMENT:
      /* XXX: some of these are just shader model 2.0 values, fix this! */
      switch (cap) {
      case PIPE_SHADER_CAP_MAX_INSTRUCTIONS:
         return I915_MAX_ALU_INSN + I915_MAX_TEX_INSN;
      case PIPE_SHADER_CAP_MAX_ALU_INSTRUCTIONS:
         return I915_MAX_ALU_INSN;
      case PIPE_SHADER_CAP_MAX_TEX_INSTRUCTIONS:
         return I915_MAX_TEX_INSN;
      case PIPE_SHADER_CAP_MAX_TEX_INDIRECTIONS:
         return 4;
      case PIPE_SHADER_CAP_MAX_CONTROL_FLOW_DEPTH:
         return 0;
      case PIPE_SHADER_CAP_MAX_INPUTS:
         return 10;
      case PIPE_SHADER_CAP_MAX_OUTPUTS:
         return 1;
      case PIPE_SHADER_CAP_MAX_CONST_BUFFER0_SIZE:
         return 32 * sizeof(float[4]);
      case PIPE_SHADER_CAP_MAX_CONST_BUFFERS:
         return 1;
      case PIPE_SHADER_CAP_MAX_TEMPS:
         /* 16 inter-phase temps, 3 intra-phase temps.  i915c reported 16. too. */
         return 16;
      case PIPE_SHADER_CAP_CONT_SUPPORTED:
      case PIPE_SHADER_CAP_TGSI_SQRT_SUPPORTED:
         return 0;
      case PIPE_SHADER_CAP_INDIRECT_INPUT_ADDR:
      case PIPE_SHADER_CAP_INDIRECT_OUTPUT_ADDR:
      case PIPE_SHADER_CAP_INDIRECT_TEMP_ADDR:
      case PIPE_SHADER_CAP_INDIRECT_CONST_ADDR:
      case PIPE_SHADER_CAP_SUBROUTINES:
         return 0;
      case PIPE_SHADER_CAP_INT64_ATOMICS:
      case PIPE_SHADER_CAP_INT16:
      case PIPE_SHADER_CAP_GLSL_16BIT_CONSTS:
         return 0;
      case PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS:
      case PIPE_SHADER_CAP_MAX_SAMPLER_VIEWS:
         return I915_TEX_UNITS;
      case PIPE_SHADER_CAP_TGSI_ANY_INOUT_DECL_RANGE:
      case PIPE_SHADER_CAP_MAX_SHADER_BUFFERS:
      case PIPE_SHADER_CAP_MAX_SHADER_IMAGES:
      case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTERS:
      case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTER_BUFFERS:
         return 0;

      default:
         debug_printf("%s: Unknown cap %u.\n", __func__, cap);
         return 0;
      }
      break;
   default:
      return 0;
   }
}

static int
i915_get_param(struct pipe_screen *screen, enum pipe_cap cap)
{
   struct i915_screen *is = i915_screen(screen);

   switch (cap) {
   /* Supported features (boolean caps). */
   case PIPE_CAP_ANISOTROPIC_FILTER:
   case PIPE_CAP_NPOT_TEXTURES:
   case PIPE_CAP_MIXED_FRAMEBUFFER_SIZES:
   case PIPE_CAP_PRIMITIVE_RESTART: /* draw module */
   case PIPE_CAP_PRIMITIVE_RESTART_FIXED_INDEX:
   case PIPE_CAP_VERTEX_ELEMENT_INSTANCE_DIVISOR:
   case PIPE_CAP_BLEND_EQUATION_SEPARATE:
   case PIPE_CAP_VS_INSTANCEID:
   case PIPE_CAP_VERTEX_COLOR_CLAMPED:
   case PIPE_CAP_USER_VERTEX_BUFFERS:
   case PIPE_CAP_MIXED_COLOR_DEPTH_BITS:
   case PIPE_CAP_TGSI_TEXCOORD:
      return 1;

   case PIPE_CAP_TEXTURE_TRANSFER_MODES:
   case PIPE_CAP_PCI_GROUP:
   case PIPE_CAP_PCI_BUS:
   case PIPE_CAP_PCI_DEVICE:
   case PIPE_CAP_PCI_FUNCTION:
      return 0;

   case PIPE_CAP_ALLOW_MAPPED_BUFFERS_DURING_EXECUTION:
      return 0;

   case PIPE_CAP_SHAREABLE_SHADERS:
      /* Can't expose shareable shaders because the draw shaders reference the
       * draw module's state, which is per-context.
       */
      return 0;

   case PIPE_CAP_MAX_GS_INVOCATIONS:
      return 32;

   case PIPE_CAP_MAX_SHADER_BUFFER_SIZE_UINT:
      return 1 << 27;

   case PIPE_CAP_MAX_VIEWPORTS:
      return 1;

   case PIPE_CAP_MIN_MAP_BUFFER_ALIGNMENT:
      return 64;

   case PIPE_CAP_GLSL_FEATURE_LEVEL:
   case PIPE_CAP_GLSL_FEATURE_LEVEL_COMPATIBILITY:
      return 120;

   case PIPE_CAP_CONSTANT_BUFFER_OFFSET_ALIGNMENT:
      return 16;

   /* Texturing. */
   case PIPE_CAP_MAX_TEXTURE_2D_SIZE:
      return 1 << (I915_MAX_TEXTURE_2D_LEVELS - 1);
   case PIPE_CAP_MAX_TEXTURE_3D_LEVELS:
      return I915_MAX_TEXTURE_3D_LEVELS;
   case PIPE_CAP_MAX_TEXTURE_CUBE_LEVELS:
      return I915_MAX_TEXTURE_2D_LEVELS;

   /* Render targets. */
   case PIPE_CAP_MAX_RENDER_TARGETS:
      return 1;

   case PIPE_CAP_MAX_VERTEX_ATTRIB_STRIDE:
      return 2048;

   /* Fragment coordinate conventions. */
   case PIPE_CAP_FS_COORD_ORIGIN_UPPER_LEFT:
   case PIPE_CAP_FS_COORD_PIXEL_CENTER_HALF_INTEGER:
      return 1;
   case PIPE_CAP_ENDIANNESS:
      return PIPE_ENDIAN_LITTLE;
   case PIPE_CAP_MAX_VARYINGS:
      return 10;

   case PIPE_CAP_NIR_IMAGES_AS_DEREF:
      return 0;

   case PIPE_CAP_VENDOR_ID:
      return 0x8086;
   case PIPE_CAP_DEVICE_ID:
      return is->iws->pci_id;
   case PIPE_CAP_ACCELERATED:
      return 1;
   case PIPE_CAP_VIDEO_MEMORY: {
      /* Once a batch uses more than 75% of the maximum mappable size, we
       * assume that there's some fragmentation, and we start doing extra
       * flushing, etc.  That's the big cliff apps will care about.
       */
      const int gpu_mappable_megabytes =
         is->iws->aperture_size(is->iws) * 3 / 4;
      uint64_t system_memory;

      if (!os_get_total_physical_memory(&system_memory))
         return 0;

      return MIN2(gpu_mappable_megabytes, (int)(system_memory >> 20));
   }
   case PIPE_CAP_UMA:
      return 1;

   default:
      return u_pipe_screen_get_param_defaults(screen, cap);
   }
}

static float
i915_get_paramf(struct pipe_screen *screen, enum pipe_capf cap)
{
   switch (cap) {
   case PIPE_CAPF_MIN_LINE_WIDTH:
   case PIPE_CAPF_MIN_LINE_WIDTH_AA:
   case PIPE_CAPF_MIN_POINT_SIZE:
   case PIPE_CAPF_MIN_POINT_SIZE_AA:
      return 1;

   case PIPE_CAPF_POINT_SIZE_GRANULARITY:
   case PIPE_CAPF_LINE_WIDTH_GRANULARITY:
      return 0.1;

   case PIPE_CAPF_MAX_LINE_WIDTH:
      FALLTHROUGH;
   case PIPE_CAPF_MAX_LINE_WIDTH_AA:
      return 7.5;

   case PIPE_CAPF_MAX_POINT_SIZE:
      FALLTHROUGH;
   case PIPE_CAPF_MAX_POINT_SIZE_AA:
      return 255.0;

   case PIPE_CAPF_MAX_TEXTURE_ANISOTROPY:
      return 4.0;

   case PIPE_CAPF_MAX_TEXTURE_LOD_BIAS:
      return 16.0;

   case PIPE_CAPF_MIN_CONSERVATIVE_RASTER_DILATE:
      FALLTHROUGH;
   case PIPE_CAPF_MAX_CONSERVATIVE_RASTER_DILATE:
      FALLTHROUGH;
   case PIPE_CAPF_CONSERVATIVE_RASTER_DILATE_GRANULARITY:
      return 0.0f;

   default:
      debug_printf("%s: Unknown cap %u.\n", __func__, cap);
      return 0;
   }
}

bool
i915_is_format_supported(struct pipe_screen *screen, enum pipe_format format,
                         enum pipe_texture_target target, unsigned sample_count,
                         unsigned storage_sample_count, unsigned tex_usage)
{
   static const enum pipe_format tex_supported[] = {
      PIPE_FORMAT_B8G8R8A8_UNORM, PIPE_FORMAT_B8G8R8A8_SRGB,
      PIPE_FORMAT_B8G8R8X8_UNORM, PIPE_FORMAT_R8G8B8A8_UNORM,
      PIPE_FORMAT_R8G8B8X8_UNORM, PIPE_FORMAT_B4G4R4A4_UNORM,
      PIPE_FORMAT_B5G6R5_UNORM, PIPE_FORMAT_B5G5R5A1_UNORM,
      PIPE_FORMAT_B10G10R10A2_UNORM, PIPE_FORMAT_L8_UNORM, PIPE_FORMAT_A8_UNORM,
      PIPE_FORMAT_I8_UNORM, PIPE_FORMAT_L8A8_UNORM, PIPE_FORMAT_UYVY,
      PIPE_FORMAT_YUYV,
      /* XXX why not?
      PIPE_FORMAT_Z16_UNORM, */
      PIPE_FORMAT_DXT1_RGB, PIPE_FORMAT_DXT1_SRGB, PIPE_FORMAT_DXT1_RGBA,
      PIPE_FORMAT_DXT1_SRGBA, PIPE_FORMAT_DXT3_RGBA, PIPE_FORMAT_DXT3_SRGBA,
      PIPE_FORMAT_DXT5_RGBA, PIPE_FORMAT_DXT5_SRGBA, PIPE_FORMAT_Z24X8_UNORM,
      PIPE_FORMAT_FXT1_RGB, PIPE_FORMAT_FXT1_RGBA,
      PIPE_FORMAT_Z24_UNORM_S8_UINT, PIPE_FORMAT_NONE /* list terminator */
   };
   static const enum pipe_format render_supported[] = {
      PIPE_FORMAT_B8G8R8A8_UNORM, PIPE_FORMAT_B8G8R8X8_UNORM,
      PIPE_FORMAT_R8G8B8A8_UNORM, PIPE_FORMAT_R8G8B8X8_UNORM,
      PIPE_FORMAT_B5G6R5_UNORM,   PIPE_FORMAT_B5G5R5A1_UNORM,
      PIPE_FORMAT_B4G4R4A4_UNORM, PIPE_FORMAT_B10G10R10A2_UNORM,
      PIPE_FORMAT_L8_UNORM,       PIPE_FORMAT_A8_UNORM,
      PIPE_FORMAT_I8_UNORM,       PIPE_FORMAT_NONE /* list terminator */
   };
   static const enum pipe_format depth_supported[] = {
      /* XXX why not?
      PIPE_FORMAT_Z16_UNORM, */
      PIPE_FORMAT_Z24X8_UNORM, PIPE_FORMAT_Z24_UNORM_S8_UINT,
      PIPE_FORMAT_NONE /* list terminator */
   };
   const enum pipe_format *list;
   uint32_t i;

   if (sample_count > 1)
      return false;

   if (MAX2(1, sample_count) != MAX2(1, storage_sample_count))
      return false;

   if (tex_usage & PIPE_BIND_DEPTH_STENCIL)
      list = depth_supported;
   else if (tex_usage & PIPE_BIND_RENDER_TARGET)
      list = render_supported;
   else if (tex_usage & PIPE_BIND_SAMPLER_VIEW)
      list = tex_supported;
   else
      return true; /* PIPE_BIND_{VERTEX,INDEX}_BUFFER */

   for (i = 0; list[i] != PIPE_FORMAT_NONE; i++) {
      if (list[i] == format)
         return true;
   }

   return false;
}

/*
 * Fence functions
 */

static void
i915_fence_reference(struct pipe_screen *screen, struct pipe_fence_handle **ptr,
                     struct pipe_fence_handle *fence)
{
   struct i915_screen *is = i915_screen(screen);

   is->iws->fence_reference(is->iws, ptr, fence);
}

static bool
i915_fence_finish(struct pipe_screen *screen, struct pipe_context *ctx,
                  struct pipe_fence_handle *fence, uint64_t timeout)
{
   struct i915_screen *is = i915_screen(screen);

   if (!timeout)
      return is->iws->fence_signalled(is->iws, fence) == 1;

   return is->iws->fence_finish(is->iws, fence) == 1;
}

/*
 * Generic functions
 */

static void
i915_destroy_screen(struct pipe_screen *screen)
{
   struct i915_screen *is = i915_screen(screen);

   if (is->iws)
      is->iws->destroy(is->iws);

   FREE(is);
}

static int
i915_screen_get_fd(struct pipe_screen *screen)
{
   struct i915_screen *is = i915_screen(screen);

   return is->iws->get_fd(is->iws);
}

/**
 * Create a new i915_screen object
 */
struct pipe_screen *
i915_screen_create(struct i915_winsys *iws)
{
   struct i915_screen *is = CALLOC_STRUCT(i915_screen);

   if (!is)
      return NULL;

   switch (iws->pci_id) {
   case PCI_CHIP_I915_G:
   case PCI_CHIP_I915_GM:
      is->is_i945 = false;
      break;

   case PCI_CHIP_I945_G:
   case PCI_CHIP_I945_GM:
   case PCI_CHIP_I945_GME:
   case PCI_CHIP_G33_G:
   case PCI_CHIP_Q33_G:
   case PCI_CHIP_Q35_G:
   case PCI_CHIP_PINEVIEW_G:
   case PCI_CHIP_PINEVIEW_M:
      is->is_i945 = true;
      break;

   default:
      debug_printf("%s: unknown pci id 0x%x, cannot create screen\n", __func__,
                   iws->pci_id);
      FREE(is);
      return NULL;
   }

   is->iws = iws;

   is->base.destroy = i915_destroy_screen;

   is->base.get_name = i915_get_name;
   is->base.get_vendor = i915_get_vendor;
   is->base.get_device_vendor = i915_get_device_vendor;
   is->base.get_screen_fd = i915_screen_get_fd;
   is->base.get_param = i915_get_param;
   is->base.get_shader_param = i915_get_shader_param;
   is->base.get_paramf = i915_get_paramf;
   is->base.get_compiler_options = i915_get_compiler_options;
   is->base.finalize_nir = i915_finalize_nir;
   is->base.is_format_supported = i915_is_format_supported;

   is->base.context_create = i915_create_context;

   is->base.fence_reference = i915_fence_reference;
   is->base.fence_finish = i915_fence_finish;

   i915_init_screen_resource_functions(is);

   i915_debug_init(is);

   return &is->base;
}
