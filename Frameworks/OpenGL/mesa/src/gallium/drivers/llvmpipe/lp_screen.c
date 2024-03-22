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


#include "util/u_memory.h"
#include "util/u_math.h"
#include "util/u_cpu_detect.h"
#include "util/format/u_format.h"
#include "util/u_screen.h"
#include "util/u_string.h"
#include "util/format/u_format_s3tc.h"
#include "pipe/p_defines.h"
#include "pipe/p_screen.h"
#include "draw/draw_context.h"
#include "gallivm/lp_bld_type.h"
#include "gallivm/lp_bld_nir.h"
#include "util/disk_cache.h"
#include "util/hex.h"
#include "util/os_misc.h"
#include "util/os_time.h"
#include "util/u_helpers.h"
#include "lp_texture.h"
#include "lp_fence.h"
#include "lp_jit.h"
#include "lp_screen.h"
#include "lp_context.h"
#include "lp_debug.h"
#include "lp_public.h"
#include "lp_limits.h"
#include "lp_rast.h"
#include "lp_cs_tpool.h"
#include "lp_flush.h"

#include "frontend/sw_winsys.h"

#include "nir.h"


int LP_DEBUG = 0;

static const struct debug_named_value lp_debug_flags[] = {
   { "pipe", DEBUG_PIPE, NULL },
   { "tgsi", DEBUG_TGSI, NULL },
   { "tex", DEBUG_TEX, NULL },
   { "setup", DEBUG_SETUP, NULL },
   { "rast", DEBUG_RAST, NULL },
   { "query", DEBUG_QUERY, NULL },
   { "screen", DEBUG_SCREEN, NULL },
   { "counters", DEBUG_COUNTERS, NULL },
   { "scene", DEBUG_SCENE, NULL },
   { "fence", DEBUG_FENCE, NULL },
   { "no_fastpath", DEBUG_NO_FASTPATH, NULL },
   { "linear", DEBUG_LINEAR, NULL },
   { "linear2", DEBUG_LINEAR2, NULL },
   { "mem", DEBUG_MEM, NULL },
   { "fs", DEBUG_FS, NULL },
   { "cs", DEBUG_CS, NULL },
   { "accurate_a0", DEBUG_ACCURATE_A0 },
   { "mesh", DEBUG_MESH },
   DEBUG_NAMED_VALUE_END
};

int LP_PERF = 0;
static const struct debug_named_value lp_perf_flags[] = {
   { "texmem",         PERF_TEX_MEM, NULL },
   { "no_mipmap",      PERF_NO_MIPMAPS, NULL },
   { "no_linear",      PERF_NO_LINEAR, NULL },
   { "no_mip_linear",  PERF_NO_MIP_LINEAR, NULL },
   { "no_tex",         PERF_NO_TEX, NULL },
   { "no_blend",       PERF_NO_BLEND, NULL },
   { "no_depth",       PERF_NO_DEPTH, NULL },
   { "no_alphatest",   PERF_NO_ALPHATEST, NULL },
   { "no_rast_linear", PERF_NO_RAST_LINEAR, NULL },
   { "no_shade",       PERF_NO_SHADE, NULL },
   DEBUG_NAMED_VALUE_END
};


static const char *
llvmpipe_get_vendor(struct pipe_screen *screen)
{
   return "Mesa";
}


static const char *
llvmpipe_get_name(struct pipe_screen *screen)
{
   struct llvmpipe_screen *lscreen = llvmpipe_screen(screen);
   return lscreen->renderer_string;
}


static int
llvmpipe_get_param(struct pipe_screen *screen, enum pipe_cap param)
{
   switch (param) {
   case PIPE_CAP_NPOT_TEXTURES:
   case PIPE_CAP_MIXED_FRAMEBUFFER_SIZES:
   case PIPE_CAP_MIXED_COLOR_DEPTH_BITS:
   case PIPE_CAP_ANISOTROPIC_FILTER:
      return 1;
   case PIPE_CAP_FRAGMENT_SHADER_TEXTURE_LOD:
   case PIPE_CAP_FRAGMENT_SHADER_DERIVATIVES:
      return 1;
   case PIPE_CAP_MAX_DUAL_SOURCE_RENDER_TARGETS:
      return 1;
   case PIPE_CAP_MAX_STREAM_OUTPUT_BUFFERS:
      return PIPE_MAX_SO_BUFFERS;
   case PIPE_CAP_MAX_RENDER_TARGETS:
      return PIPE_MAX_COLOR_BUFS;
   case PIPE_CAP_OCCLUSION_QUERY:
   case PIPE_CAP_QUERY_TIMESTAMP:
   case PIPE_CAP_TIMER_RESOLUTION:
   case PIPE_CAP_QUERY_TIME_ELAPSED:
      return 1;
   case PIPE_CAP_QUERY_PIPELINE_STATISTICS:
      return 1;
   case PIPE_CAP_TEXTURE_MIRROR_CLAMP:
   case PIPE_CAP_TEXTURE_MIRROR_CLAMP_TO_EDGE:
      return 1;
   case PIPE_CAP_TEXTURE_SWIZZLE:
   case PIPE_CAP_TEXTURE_SHADOW_LOD:
      return 1;
   case PIPE_CAP_MAX_TEXTURE_2D_SIZE:
      return 1 << (LP_MAX_TEXTURE_2D_LEVELS - 1);
   case PIPE_CAP_MAX_TEXTURE_3D_LEVELS:
      return LP_MAX_TEXTURE_3D_LEVELS;
   case PIPE_CAP_MAX_TEXTURE_CUBE_LEVELS:
      return LP_MAX_TEXTURE_CUBE_LEVELS;
   case PIPE_CAP_MAX_TEXTURE_ARRAY_LAYERS:
      return LP_MAX_TEXTURE_ARRAY_LAYERS;
   case PIPE_CAP_BLEND_EQUATION_SEPARATE:
      return 1;
   case PIPE_CAP_INDEP_BLEND_ENABLE:
      return 1;
   case PIPE_CAP_INDEP_BLEND_FUNC:
      return 1;
   case PIPE_CAP_FS_COORD_ORIGIN_UPPER_LEFT:
   case PIPE_CAP_FS_COORD_PIXEL_CENTER_INTEGER:
   case PIPE_CAP_FS_COORD_PIXEL_CENTER_HALF_INTEGER:
      return 1;
   case PIPE_CAP_PRIMITIVE_RESTART:
   case PIPE_CAP_PRIMITIVE_RESTART_FIXED_INDEX:
      return 1;
   case PIPE_CAP_DEPTH_CLIP_DISABLE:
      return 1;
   case PIPE_CAP_DEPTH_CLAMP_ENABLE:
      return 1;
   case PIPE_CAP_SHADER_STENCIL_EXPORT:
      return 1;
   case PIPE_CAP_VS_INSTANCEID:
   case PIPE_CAP_VERTEX_ELEMENT_INSTANCE_DIVISOR:
   case PIPE_CAP_START_INSTANCE:
      return 1;
   case PIPE_CAP_SEAMLESS_CUBE_MAP:
   case PIPE_CAP_SEAMLESS_CUBE_MAP_PER_TEXTURE:
      return 1;
   /* this is a lie could support arbitrary large offsets */
   case PIPE_CAP_MIN_TEXTURE_GATHER_OFFSET:
   case PIPE_CAP_MIN_TEXEL_OFFSET:
      return -32;
   case PIPE_CAP_MAX_TEXTURE_GATHER_OFFSET:
   case PIPE_CAP_MAX_TEXEL_OFFSET:
      return 31;
   case PIPE_CAP_CONDITIONAL_RENDER:
   case PIPE_CAP_TEXTURE_BARRIER:
      return 1;
   case PIPE_CAP_MAX_STREAM_OUTPUT_SEPARATE_COMPONENTS:
   case PIPE_CAP_MAX_STREAM_OUTPUT_INTERLEAVED_COMPONENTS:
      return 16*4;
   case PIPE_CAP_MAX_GEOMETRY_OUTPUT_VERTICES:
   case PIPE_CAP_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS:
      return 1024;
   case PIPE_CAP_MAX_VERTEX_STREAMS:
      return 4;
   case PIPE_CAP_MAX_VERTEX_ATTRIB_STRIDE:
      return 2048;
   case PIPE_CAP_STREAM_OUTPUT_PAUSE_RESUME:
   case PIPE_CAP_STREAM_OUTPUT_INTERLEAVE_BUFFERS:
   case PIPE_CAP_VERTEX_COLOR_UNCLAMPED:
   case PIPE_CAP_VERTEX_COLOR_CLAMPED:
      return 1;
   case PIPE_CAP_GLSL_FEATURE_LEVEL_COMPATIBILITY:
   case PIPE_CAP_GLSL_FEATURE_LEVEL:
      return 450;
   case PIPE_CAP_COMPUTE:
      return GALLIVM_COROUTINES;
   case PIPE_CAP_USER_VERTEX_BUFFERS:
      return 1;
   case PIPE_CAP_TGSI_TEXCOORD:
   case PIPE_CAP_DRAW_INDIRECT:
      return 1;

   case PIPE_CAP_CUBE_MAP_ARRAY:
      return 1;
   case PIPE_CAP_CONSTANT_BUFFER_OFFSET_ALIGNMENT:
      return 16;
   case PIPE_CAP_MIN_MAP_BUFFER_ALIGNMENT:
      return 64;
   case PIPE_CAP_TEXTURE_BUFFER_OBJECTS:
      return 1;
   case PIPE_CAP_LINEAR_IMAGE_PITCH_ALIGNMENT:
      return 1;
   case PIPE_CAP_LINEAR_IMAGE_BASE_ADDRESS_ALIGNMENT:
      return 1;
   /* Adressing that many 64bpp texels fits in an i32 so this is a reasonable value */
   case PIPE_CAP_MAX_TEXEL_BUFFER_ELEMENTS_UINT:
      return LP_MAX_TEXEL_BUFFER_ELEMENTS;
   case PIPE_CAP_TEXTURE_BUFFER_OFFSET_ALIGNMENT:
      return 16;
   case PIPE_CAP_TEXTURE_TRANSFER_MODES:
      return 0;
   case PIPE_CAP_MAX_VIEWPORTS:
      return PIPE_MAX_VIEWPORTS;
   case PIPE_CAP_ENDIANNESS:
      return PIPE_ENDIAN_NATIVE;
   case PIPE_CAP_TES_LAYER_VIEWPORT:
   case PIPE_CAP_VS_LAYER_VIEWPORT:
      return 1;
   case PIPE_CAP_MAX_TEXTURE_GATHER_COMPONENTS:
      return 4;
   case PIPE_CAP_VS_WINDOW_SPACE_POSITION:
      return 1;
   case PIPE_CAP_FS_FINE_DERIVATIVE:
      return 1;
   case PIPE_CAP_TGSI_TEX_TXF_LZ:
   case PIPE_CAP_SAMPLER_VIEW_TARGET:
      return 1;
   case PIPE_CAP_FAKE_SW_MSAA:
      return 0;
   case PIPE_CAP_TEXTURE_QUERY_LOD:
   case PIPE_CAP_CONDITIONAL_RENDER_INVERTED:
   case PIPE_CAP_SHADER_ARRAY_COMPONENTS:
   case PIPE_CAP_DOUBLES:
   case PIPE_CAP_INT64:
   case PIPE_CAP_QUERY_SO_OVERFLOW:
   case PIPE_CAP_TGSI_DIV:
      return 1;
   case PIPE_CAP_VENDOR_ID:
      return 0xFFFFFFFF;
   case PIPE_CAP_DEVICE_ID:
      return 0xFFFFFFFF;
   case PIPE_CAP_ACCELERATED:
      return 0;
   case PIPE_CAP_VIDEO_MEMORY: {
      /* XXX: Do we want to return the full amount fo system memory ? */
      uint64_t system_memory;

      if (!os_get_total_physical_memory(&system_memory))
         return 0;

      if (sizeof(void *) == 4)
         /* Cap to 2 GB on 32 bits system. We do this because llvmpipe does
          * eat application memory, which is quite limited on 32 bits. App
          * shouldn't expect too much available memory. */
         system_memory = MIN2(system_memory, 2048 << 20);

      return (int)(system_memory >> 20);
   }
   case PIPE_CAP_UMA:
      return 1;
   case PIPE_CAP_QUERY_MEMORY_INFO:
      return 1;
   case PIPE_CAP_CLIP_HALFZ:
      return 1;
   case PIPE_CAP_POLYGON_OFFSET_CLAMP:
   case PIPE_CAP_TEXTURE_FLOAT_LINEAR:
   case PIPE_CAP_TEXTURE_HALF_FLOAT_LINEAR:
      return 1;
   case PIPE_CAP_CULL_DISTANCE:
      return 1;
   case PIPE_CAP_COPY_BETWEEN_COMPRESSED_AND_PLAIN_FORMATS:
      return 1;
   case PIPE_CAP_MAX_VARYINGS:
      return 32;
   case PIPE_CAP_SHADER_BUFFER_OFFSET_ALIGNMENT:
      return 16;
   case PIPE_CAP_QUERY_BUFFER_OBJECT:
      return 1;
   case PIPE_CAP_DRAW_PARAMETERS:
      return 1;
   case PIPE_CAP_FBFETCH:
      return 8;
   case PIPE_CAP_FBFETCH_COHERENT:
   case PIPE_CAP_FBFETCH_ZS:
   case PIPE_CAP_MULTI_DRAW_INDIRECT:
   case PIPE_CAP_MULTI_DRAW_INDIRECT_PARAMS:
      return 1;
   case PIPE_CAP_DEVICE_RESET_STATUS_QUERY:
   case PIPE_CAP_ROBUST_BUFFER_ACCESS_BEHAVIOR:
      return 1;
   case PIPE_CAP_MAX_SHADER_PATCH_VARYINGS:
      return 32;
   case PIPE_CAP_RASTERIZER_SUBPIXEL_BITS:
      return 8;
   case PIPE_CAP_PCI_GROUP:
   case PIPE_CAP_PCI_BUS:
   case PIPE_CAP_PCI_DEVICE:
   case PIPE_CAP_PCI_FUNCTION:
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
      return LP_MAX_TGSI_SHADER_BUFFER_SIZE;
   case PIPE_CAP_FRAMEBUFFER_NO_ATTACHMENT:
   case PIPE_CAP_TGSI_TG4_COMPONENT_IN_SWIZZLE:
   case PIPE_CAP_FS_FACE_IS_INTEGER_SYSVAL:
   case PIPE_CAP_RESOURCE_FROM_USER_MEMORY:
   case PIPE_CAP_IMAGE_STORE_FORMATTED:
   case PIPE_CAP_IMAGE_LOAD_FORMATTED:
      return 1;
#ifdef PIPE_MEMORY_FD
   case PIPE_CAP_MEMOBJ:
      return 1;
#endif
   case PIPE_CAP_SAMPLER_REDUCTION_MINMAX:
   case PIPE_CAP_TEXTURE_QUERY_SAMPLES:
   case PIPE_CAP_SHADER_GROUP_VOTE:
   case PIPE_CAP_SHADER_BALLOT:
   case PIPE_CAP_IMAGE_ATOMIC_FLOAT_ADD:
   case PIPE_CAP_LOAD_CONSTBUF:
   case PIPE_CAP_TEXTURE_MULTISAMPLE:
   case PIPE_CAP_SAMPLE_SHADING:
   case PIPE_CAP_GL_SPIRV:
   case PIPE_CAP_POST_DEPTH_COVERAGE:
   case PIPE_CAP_SHADER_CLOCK:
   case PIPE_CAP_PACKED_UNIFORMS:
      return 1;
   case PIPE_CAP_SYSTEM_SVM:
      return 1;
   case PIPE_CAP_ATOMIC_FLOAT_MINMAX:
      return LLVM_VERSION_MAJOR >= 15;
   case PIPE_CAP_NIR_IMAGES_AS_DEREF:
      return 0;
   default:
      return u_pipe_screen_get_param_defaults(screen, param);
   }
}


static int
llvmpipe_get_shader_param(struct pipe_screen *screen,
                          enum pipe_shader_type shader,
                          enum pipe_shader_cap param)
{
   struct llvmpipe_screen *lscreen = llvmpipe_screen(screen);
   switch (shader) {
   case PIPE_SHADER_COMPUTE:
      if ((lscreen->allow_cl) && param == PIPE_SHADER_CAP_SUPPORTED_IRS)
         return ((1 << PIPE_SHADER_IR_TGSI) |
                 (1 << PIPE_SHADER_IR_NIR) |
                 (1 << PIPE_SHADER_IR_NIR_SERIALIZED));
      FALLTHROUGH;
   case PIPE_SHADER_MESH:
   case PIPE_SHADER_TASK:
   case PIPE_SHADER_FRAGMENT:
      return gallivm_get_shader_param(param);
   case PIPE_SHADER_TESS_CTRL:
   case PIPE_SHADER_TESS_EVAL:
      /* Tessellation shader needs llvm coroutines support */
      if (!GALLIVM_COROUTINES)
         return 0;
      FALLTHROUGH;
   case PIPE_SHADER_VERTEX:
   case PIPE_SHADER_GEOMETRY:
      switch (param) {
      case PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS:
         /* At this time, the draw module and llvmpipe driver only
          * support vertex shader texture lookups when LLVM is enabled in
          * the draw module.
          */
         if (debug_get_bool_option("DRAW_USE_LLVM", true))
            return PIPE_MAX_SAMPLERS;
         else
            return 0;
      case PIPE_SHADER_CAP_MAX_SAMPLER_VIEWS:
         if (debug_get_bool_option("DRAW_USE_LLVM", true))
            return PIPE_MAX_SHADER_SAMPLER_VIEWS;
         else
            return 0;
      default:
         return draw_get_shader_param(shader, param);
      }
   default:
      return 0;
   }
}


static float
llvmpipe_get_paramf(struct pipe_screen *screen, enum pipe_capf param)
{
   switch (param) {
   case PIPE_CAPF_MIN_LINE_WIDTH:
   case PIPE_CAPF_MIN_LINE_WIDTH_AA:
   case PIPE_CAPF_MIN_POINT_SIZE:
   case PIPE_CAPF_MIN_POINT_SIZE_AA:
      return 1.0;
   case PIPE_CAPF_POINT_SIZE_GRANULARITY:
   case PIPE_CAPF_LINE_WIDTH_GRANULARITY:
      return 0.1;
   case PIPE_CAPF_MAX_LINE_WIDTH:
      FALLTHROUGH;
   case PIPE_CAPF_MAX_LINE_WIDTH_AA:
      return 255.0; /* arbitrary */
   case PIPE_CAPF_MAX_POINT_SIZE:
      FALLTHROUGH;
   case PIPE_CAPF_MAX_POINT_SIZE_AA:
      return LP_MAX_POINT_WIDTH; /* arbitrary */
   case PIPE_CAPF_MAX_TEXTURE_ANISOTROPY:
      return 16.0; /* not actually signficant at this time */
   case PIPE_CAPF_MAX_TEXTURE_LOD_BIAS:
      return 16.0; /* arbitrary */
   case PIPE_CAPF_MIN_CONSERVATIVE_RASTER_DILATE:
      return 0.0;
   case PIPE_CAPF_MAX_CONSERVATIVE_RASTER_DILATE:
      return 0.0;
   case PIPE_CAPF_CONSERVATIVE_RASTER_DILATE_GRANULARITY:
      return 0.0;
   }
   /* should only get here on unhandled cases */
   debug_printf("Unexpected PIPE_CAP %d query\n", param);
   return 0.0;
}


static int
llvmpipe_get_compute_param(struct pipe_screen *_screen,
                           enum pipe_shader_ir ir_type,
                           enum pipe_compute_cap param,
                           void *ret)
{
   switch (param) {
   case PIPE_COMPUTE_CAP_IR_TARGET:
      return 0;
   case PIPE_COMPUTE_CAP_MAX_GRID_SIZE:
      if (ret) {
         uint64_t *grid_size = ret;
         grid_size[0] = 65535;
         grid_size[1] = 65535;
         grid_size[2] = 65535;
      }
      return 3 * sizeof(uint64_t) ;
   case PIPE_COMPUTE_CAP_MAX_BLOCK_SIZE:
      if (ret) {
         uint64_t *block_size = ret;
         block_size[0] = 1024;
         block_size[1] = 1024;
         block_size[2] = 1024;
      }
      return 3 * sizeof(uint64_t);
   case PIPE_COMPUTE_CAP_MAX_THREADS_PER_BLOCK:
      if (ret) {
         uint64_t *max_threads_per_block = ret;
         *max_threads_per_block = 1024;
      }
      return sizeof(uint64_t);
   case PIPE_COMPUTE_CAP_MAX_LOCAL_SIZE:
      if (ret) {
         uint64_t *max_local_size = ret;
         *max_local_size = 32768;
      }
      return sizeof(uint64_t);
   case PIPE_COMPUTE_CAP_GRID_DIMENSION:
      if (ret) {
         uint64_t *grid_dim = ret;
         *grid_dim = 3;
      }
      return sizeof(uint64_t);
   case PIPE_COMPUTE_CAP_MAX_GLOBAL_SIZE:
      if (ret) {
         uint64_t *max_global_size = ret;
         *max_global_size = (1ULL << 31);
      }
      return sizeof(uint64_t);
   case PIPE_COMPUTE_CAP_MAX_MEM_ALLOC_SIZE:
      if (ret) {
         uint64_t *max_mem_alloc_size = ret;
         *max_mem_alloc_size = (1ULL << 31);
      }
      return sizeof(uint64_t);
   case PIPE_COMPUTE_CAP_MAX_PRIVATE_SIZE:
      if (ret) {
         uint64_t *max_private = ret;
         *max_private = (1UL << 31);
      }
      return sizeof(uint64_t);
   case PIPE_COMPUTE_CAP_MAX_INPUT_SIZE:
      if (ret) {
         uint64_t *max_input = ret;
         *max_input = 1576;
      }
      return sizeof(uint64_t);
   case PIPE_COMPUTE_CAP_IMAGES_SUPPORTED:
      if (ret) {
         uint32_t *images = ret;
         *images = LP_MAX_TGSI_SHADER_IMAGES;
      }
      return sizeof(uint32_t);
   case PIPE_COMPUTE_CAP_MAX_VARIABLE_THREADS_PER_BLOCK:
      return 0;
   case PIPE_COMPUTE_CAP_SUBGROUP_SIZES:
      if (ret) {
         uint32_t *subgroup_size = ret;
         *subgroup_size = lp_native_vector_width / 32;
      }
      return sizeof(uint32_t);
   case PIPE_COMPUTE_CAP_MAX_SUBGROUPS:
      if (ret) {
         uint32_t *subgroup_size = ret;
         *subgroup_size = 1024 / (lp_native_vector_width / 32);
      }
      return sizeof(uint32_t);
   case PIPE_COMPUTE_CAP_MAX_COMPUTE_UNITS:
      if (ret) {
         uint32_t *max_compute_units = ret;
         *max_compute_units = 8;
      }
      return sizeof(uint32_t);
   case PIPE_COMPUTE_CAP_MAX_CLOCK_FREQUENCY:
      if (ret) {
         uint32_t *max_clock_freq = ret;
         *max_clock_freq = 300;
      }
      return sizeof(uint32_t);
   case PIPE_COMPUTE_CAP_ADDRESS_BITS:
      if (ret) {
         uint32_t *address_bits = ret;
         *address_bits = sizeof(void*) * 8;
      }
      return sizeof(uint32_t);
   }
   return 0;
}


static void
llvmpipe_get_driver_uuid(struct pipe_screen *pscreen, char *uuid)
{
   memset(uuid, 0, PIPE_UUID_SIZE);
   snprintf(uuid, PIPE_UUID_SIZE, "llvmpipeUUID");
}


static void
llvmpipe_get_device_uuid(struct pipe_screen *pscreen, char *uuid)
{
   memset(uuid, 0, PIPE_UUID_SIZE);
   snprintf(uuid, PIPE_UUID_SIZE, "mesa" PACKAGE_VERSION);
}


static const struct nir_shader_compiler_options gallivm_nir_options = {
   .lower_scmp = true,
   .lower_flrp32 = true,
   .lower_flrp64 = true,
   .lower_fsat = true,
   .lower_bitfield_insert = true,
   .lower_bitfield_extract = true,
   .lower_fdot = true,
   .lower_fdph = true,
   .lower_ffma16 = true,
   .lower_ffma32 = true,
   .lower_ffma64 = true,
   .lower_flrp16 = true,
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
   .lower_insert_byte = true,
   .lower_insert_word = true,
   .lower_uadd_carry = true,
   .lower_usub_borrow = true,
   .lower_mul_2x32_64 = true,
   .lower_ifind_msb = true,
   .lower_int64_options = nir_lower_imul_2x32_64,
   .lower_doubles_options = nir_lower_dround_even,
   .max_unroll_iterations = 32,
   .use_interpolated_input_intrinsics = true,
   .lower_to_scalar = true,
   .lower_uniforms_to_ubo = true,
   .lower_vector_cmp = true,
   .lower_device_index_to_zero = true,
   .support_16bit_alu = true,
   .lower_fisnormal = true,
   .lower_fquantize2f16 = true,
   .driver_functions = true,
};


static char *
llvmpipe_finalize_nir(struct pipe_screen *screen,
                      void *nirptr)
{
   struct nir_shader *nir = (struct nir_shader *)nirptr;
   lp_build_opt_nir(nir);
   return NULL;
}


static inline const void *
llvmpipe_get_compiler_options(struct pipe_screen *screen,
                              enum pipe_shader_ir ir,
                              enum pipe_shader_type shader)
{
   assert(ir == PIPE_SHADER_IR_NIR);
   return &gallivm_nir_options;
}


bool
lp_storage_render_image_format_supported(enum pipe_format format)
{
   const struct util_format_description *format_desc = util_format_description(format);

   if (format_desc->colorspace == UTIL_FORMAT_COLORSPACE_SRGB) {
      /* this is a lie actually other formats COULD exist where we would fail */
      if (format_desc->nr_channels < 3)
         return false;
   } else if (format_desc->colorspace != UTIL_FORMAT_COLORSPACE_RGB) {
      return false;
   }

   if (format_desc->layout != UTIL_FORMAT_LAYOUT_PLAIN &&
       format != PIPE_FORMAT_R11G11B10_FLOAT)
      return false;

   assert(format_desc->block.width == 1);
   assert(format_desc->block.height == 1);

   if (format_desc->is_mixed)
      return false;

   if (!format_desc->is_array && !format_desc->is_bitmask &&
       format != PIPE_FORMAT_R11G11B10_FLOAT)
      return false;

   return true;
}


bool
lp_storage_image_format_supported(enum pipe_format format)
{
   switch (format) {
   case PIPE_FORMAT_R32G32B32A32_FLOAT:
   case PIPE_FORMAT_R16G16B16A16_FLOAT:
   case PIPE_FORMAT_R32G32_FLOAT:
   case PIPE_FORMAT_R16G16_FLOAT:
   case PIPE_FORMAT_R11G11B10_FLOAT:
   case PIPE_FORMAT_R32_FLOAT:
   case PIPE_FORMAT_R16_FLOAT:
   case PIPE_FORMAT_R32G32B32A32_UINT:
   case PIPE_FORMAT_R16G16B16A16_UINT:
   case PIPE_FORMAT_R10G10B10A2_UINT:
   case PIPE_FORMAT_R8G8B8A8_UINT:
   case PIPE_FORMAT_R32G32_UINT:
   case PIPE_FORMAT_R16G16_UINT:
   case PIPE_FORMAT_R8G8_UINT:
   case PIPE_FORMAT_R32_UINT:
   case PIPE_FORMAT_R16_UINT:
   case PIPE_FORMAT_R8_UINT:
   case PIPE_FORMAT_R32G32B32A32_SINT:
   case PIPE_FORMAT_R16G16B16A16_SINT:
   case PIPE_FORMAT_R8G8B8A8_SINT:
   case PIPE_FORMAT_R32G32_SINT:
   case PIPE_FORMAT_R16G16_SINT:
   case PIPE_FORMAT_R8G8_SINT:
   case PIPE_FORMAT_R32_SINT:
   case PIPE_FORMAT_R16_SINT:
   case PIPE_FORMAT_R8_SINT:
   case PIPE_FORMAT_R16G16B16A16_UNORM:
   case PIPE_FORMAT_R10G10B10A2_UNORM:
   case PIPE_FORMAT_R8G8B8A8_UNORM:
   case PIPE_FORMAT_R16G16_UNORM:
   case PIPE_FORMAT_R8G8_UNORM:
   case PIPE_FORMAT_R16_UNORM:
   case PIPE_FORMAT_R8_UNORM:
   case PIPE_FORMAT_R16G16B16A16_SNORM:
   case PIPE_FORMAT_R8G8B8A8_SNORM:
   case PIPE_FORMAT_R16G16_SNORM:
   case PIPE_FORMAT_R8G8_SNORM:
   case PIPE_FORMAT_R16_SNORM:
   case PIPE_FORMAT_R8_SNORM:
   case PIPE_FORMAT_B8G8R8A8_UNORM:
   case PIPE_FORMAT_A8_UNORM:
      return true;
   default:
      return false;
   }
}


/**
 * Query format support for creating a texture, drawing surface, etc.
 * \param format  the format to test
 * \param type  one of PIPE_TEXTURE, PIPE_SURFACE
 */
static bool
llvmpipe_is_format_supported(struct pipe_screen *_screen,
                             enum pipe_format format,
                             enum pipe_texture_target target,
                             unsigned sample_count,
                             unsigned storage_sample_count,
                             unsigned bind)
{
   struct llvmpipe_screen *screen = llvmpipe_screen(_screen);
   struct sw_winsys *winsys = screen->winsys;
   const struct util_format_description *format_desc =
      util_format_description(format);

   assert(target == PIPE_BUFFER ||
          target == PIPE_TEXTURE_1D ||
          target == PIPE_TEXTURE_1D_ARRAY ||
          target == PIPE_TEXTURE_2D ||
          target == PIPE_TEXTURE_2D_ARRAY ||
          target == PIPE_TEXTURE_RECT ||
          target == PIPE_TEXTURE_3D ||
          target == PIPE_TEXTURE_CUBE ||
          target == PIPE_TEXTURE_CUBE_ARRAY);

   if (sample_count != 0 && sample_count != 1 && sample_count != 4)
      return false;

   if (bind & (PIPE_BIND_RENDER_TARGET | PIPE_BIND_SHADER_IMAGE))
      if (!lp_storage_render_image_format_supported(format))
         return false;

   if (bind & PIPE_BIND_SHADER_IMAGE) {
      if (!lp_storage_image_format_supported(format))
         return false;
   }

   if ((bind & (PIPE_BIND_RENDER_TARGET | PIPE_BIND_SAMPLER_VIEW)) &&
       ((bind & PIPE_BIND_DISPLAY_TARGET) == 0)) {
      /* Disable all 3-channel formats, where channel size != 32 bits.
       * In some cases we run into crashes (in generate_unswizzled_blend()),
       * for 3-channel RGB16 variants, there was an apparent LLVM bug.
       * In any case, disabling the shallower 3-channel formats avoids a
       * number of issues with GL_ARB_copy_image support.
       */
      if (format_desc->is_array &&
          format_desc->nr_channels == 3 &&
          format_desc->block.bits != 96) {
         return false;
      }

      /* Disable 64-bit integer formats for RT/samplers.
       * VK CTS crashes with these and they don't make much sense.
       */
      int c = util_format_get_first_non_void_channel(format_desc->format);
      if (c >= 0) {
         if (format_desc->channel[c].pure_integer &&
             format_desc->channel[c].size == 64)
            return false;
      }

   }

   if (!(bind & PIPE_BIND_VERTEX_BUFFER) &&
       util_format_is_scaled(format))
      return false;

   if (bind & PIPE_BIND_DISPLAY_TARGET) {
      if (!winsys->is_displaytarget_format_supported(winsys, bind, format))
         return false;
   }

   if (bind & PIPE_BIND_DEPTH_STENCIL) {
      if (format_desc->layout != UTIL_FORMAT_LAYOUT_PLAIN)
         return false;

      if (format_desc->colorspace != UTIL_FORMAT_COLORSPACE_ZS)
         return false;
   }

   if (format_desc->layout == UTIL_FORMAT_LAYOUT_ASTC ||
       format_desc->layout == UTIL_FORMAT_LAYOUT_ATC) {
      /* Software decoding is not hooked up. */
      return false;
   }

   if (format_desc->layout == UTIL_FORMAT_LAYOUT_ETC &&
       format != PIPE_FORMAT_ETC1_RGB8)
      return false;

   if ((format_desc->layout == UTIL_FORMAT_LAYOUT_SUBSAMPLED ||
        format_desc->layout == UTIL_FORMAT_LAYOUT_PLANAR2 ||
        format_desc->layout == UTIL_FORMAT_LAYOUT_PLANAR3) &&
       target == PIPE_BUFFER)
      return false;

   /*
    * Everything can be supported by u_format
    * (those without fetch_rgba_float might be not but shouldn't hit that)
    */

   return true;
}


static void
llvmpipe_flush_frontbuffer(struct pipe_screen *_screen,
                           struct pipe_context *_pipe,
                           struct pipe_resource *resource,
                           unsigned level, unsigned layer,
                           void *context_private,
                           struct pipe_box *sub_box)
{
   struct llvmpipe_screen *screen = llvmpipe_screen(_screen);
   struct sw_winsys *winsys = screen->winsys;
   struct llvmpipe_resource *texture = llvmpipe_resource(resource);

   assert(texture->dt);

   if (texture->dt) {
      if (_pipe)
         llvmpipe_flush_resource(_pipe, resource, 0, true, true,
                                 false, "frontbuffer");
      winsys->displaytarget_display(winsys, texture->dt,
                                    context_private, sub_box);
   }
}


static void
llvmpipe_destroy_screen(struct pipe_screen *_screen)
{
   struct llvmpipe_screen *screen = llvmpipe_screen(_screen);

   if (screen->cs_tpool)
      lp_cs_tpool_destroy(screen->cs_tpool);

   if (screen->rast)
      lp_rast_destroy(screen->rast);

   lp_jit_screen_cleanup(screen);

   disk_cache_destroy(screen->disk_shader_cache);

   glsl_type_singleton_decref();

   mtx_destroy(&screen->rast_mutex);
   mtx_destroy(&screen->cs_mutex);
   FREE(screen);
}


/**
 * Fence reference counting.
 */
static void
llvmpipe_fence_reference(struct pipe_screen *screen,
                         struct pipe_fence_handle **ptr,
                         struct pipe_fence_handle *fence)
{
   struct lp_fence **old = (struct lp_fence **) ptr;
   struct lp_fence *f = (struct lp_fence *) fence;

   lp_fence_reference(old, f);
}


/**
 * Wait for the fence to finish.
 */
static bool
llvmpipe_fence_finish(struct pipe_screen *screen,
                      struct pipe_context *ctx,
                      struct pipe_fence_handle *fence_handle,
                      uint64_t timeout)
{
   struct lp_fence *f = (struct lp_fence *) fence_handle;

   if (!timeout)
      return lp_fence_signalled(f);

   if (!lp_fence_signalled(f)) {
      if (timeout != OS_TIMEOUT_INFINITE)
         return lp_fence_timedwait(f, timeout);

      lp_fence_wait(f);
   }
   return true;
}


static void
update_cache_sha1_cpu(struct mesa_sha1 *ctx)
{
   const struct util_cpu_caps_t *cpu_caps = util_get_cpu_caps();
   /*
    * Don't need the cpu cache affinity stuff. The rest
    * is contained in first 5 dwords.
    */
   STATIC_ASSERT(offsetof(struct util_cpu_caps_t, num_L3_caches)
                 == 5 * sizeof(uint32_t));
   _mesa_sha1_update(ctx, cpu_caps, 5 * sizeof(uint32_t));
}


static void
lp_disk_cache_create(struct llvmpipe_screen *screen)
{
   struct mesa_sha1 ctx;
   unsigned gallivm_perf = gallivm_get_perf_flags();
   unsigned char sha1[20];
   char cache_id[20 * 2 + 1];
   _mesa_sha1_init(&ctx);

   if (!disk_cache_get_function_identifier(lp_disk_cache_create, &ctx) ||
       !disk_cache_get_function_identifier(LLVMLinkInMCJIT, &ctx))
      return;

   _mesa_sha1_update(&ctx, &gallivm_perf, sizeof(gallivm_perf));
   update_cache_sha1_cpu(&ctx);
   _mesa_sha1_final(&ctx, sha1);
   mesa_bytes_to_hex(cache_id, sha1, 20);

   screen->disk_shader_cache = disk_cache_create("llvmpipe", cache_id, 0);
}


static struct disk_cache *
lp_get_disk_shader_cache(struct pipe_screen *_screen)
{
   struct llvmpipe_screen *screen = llvmpipe_screen(_screen);

   return screen->disk_shader_cache;
}

static int
llvmpipe_screen_get_fd(struct pipe_screen *_screen)
{
   struct llvmpipe_screen *screen = llvmpipe_screen(_screen);
   struct sw_winsys *winsys = screen->winsys;

   if (winsys->get_fd)
      return winsys->get_fd(winsys);
   else
      return -1;
}


void
lp_disk_cache_find_shader(struct llvmpipe_screen *screen,
                          struct lp_cached_code *cache,
                          unsigned char ir_sha1_cache_key[20])
{
   unsigned char sha1[CACHE_KEY_SIZE];

   if (!screen->disk_shader_cache)
      return;
   disk_cache_compute_key(screen->disk_shader_cache, ir_sha1_cache_key,
                          20, sha1);

   size_t binary_size;
   uint8_t *buffer = disk_cache_get(screen->disk_shader_cache,
                                    sha1, &binary_size);
   if (!buffer) {
      cache->data_size = 0;
      return;
   }
   cache->data_size = binary_size;
   cache->data = buffer;
}


void
lp_disk_cache_insert_shader(struct llvmpipe_screen *screen,
                            struct lp_cached_code *cache,
                            unsigned char ir_sha1_cache_key[20])
{
   unsigned char sha1[CACHE_KEY_SIZE];

   if (!screen->disk_shader_cache || !cache->data_size || cache->dont_cache)
      return;
   disk_cache_compute_key(screen->disk_shader_cache, ir_sha1_cache_key,
                          20, sha1);
   disk_cache_put(screen->disk_shader_cache, sha1, cache->data,
                  cache->data_size, NULL);
}


bool
llvmpipe_screen_late_init(struct llvmpipe_screen *screen)
{
   bool ret = true;
   mtx_lock(&screen->late_mutex);

   if (screen->late_init_done)
      goto out;

   screen->rast = lp_rast_create(screen->num_threads);
   if (!screen->rast) {
      ret = false;
      goto out;
   }

   screen->cs_tpool = lp_cs_tpool_create(screen->num_threads);
   if (!screen->cs_tpool) {
      lp_rast_destroy(screen->rast);
      ret = false;
      goto out;
   }

   if (!lp_jit_screen_init(screen)) {
      ret = false;
      goto out;
   }

   lp_build_init(); /* get lp_native_vector_width initialised */

   lp_disk_cache_create(screen);
   screen->late_init_done = true;
out:
   mtx_unlock(&screen->late_mutex);
   return ret;
}


/**
 * Create a new pipe_screen object
 * Note: we're not presently subclassing pipe_screen (no llvmpipe_screen).
 */
struct pipe_screen *
llvmpipe_create_screen(struct sw_winsys *winsys)
{
   struct llvmpipe_screen *screen;

   glsl_type_singleton_init_or_ref();

   LP_DEBUG = debug_get_flags_option("LP_DEBUG", lp_debug_flags, 0 );

   LP_PERF = debug_get_flags_option("LP_PERF", lp_perf_flags, 0 );

   screen = CALLOC_STRUCT(llvmpipe_screen);
   if (!screen)
      return NULL;

   screen->winsys = winsys;

   screen->base.destroy = llvmpipe_destroy_screen;

   screen->base.get_name = llvmpipe_get_name;
   screen->base.get_vendor = llvmpipe_get_vendor;
   screen->base.get_device_vendor = llvmpipe_get_vendor; // TODO should be the CPU vendor
   screen->base.get_screen_fd = llvmpipe_screen_get_fd;
   screen->base.get_param = llvmpipe_get_param;
   screen->base.get_shader_param = llvmpipe_get_shader_param;
   screen->base.get_compute_param = llvmpipe_get_compute_param;
   screen->base.get_paramf = llvmpipe_get_paramf;
   screen->base.get_compiler_options = llvmpipe_get_compiler_options;
   screen->base.is_format_supported = llvmpipe_is_format_supported;

   screen->base.context_create = llvmpipe_create_context;
   screen->base.flush_frontbuffer = llvmpipe_flush_frontbuffer;
   screen->base.fence_reference = llvmpipe_fence_reference;
   screen->base.fence_finish = llvmpipe_fence_finish;

   screen->base.get_timestamp = u_default_get_timestamp;

   screen->base.query_memory_info = util_sw_query_memory_info;

   screen->base.get_driver_uuid = llvmpipe_get_driver_uuid;
   screen->base.get_device_uuid = llvmpipe_get_device_uuid;

   screen->base.finalize_nir = llvmpipe_finalize_nir;

   screen->base.get_disk_shader_cache = lp_get_disk_shader_cache;
   llvmpipe_init_screen_resource_funcs(&screen->base);

   screen->allow_cl = !!getenv("LP_CL");
   screen->num_threads = util_get_cpu_caps()->nr_cpus > 1
      ? util_get_cpu_caps()->nr_cpus : 0;
   screen->num_threads = debug_get_num_option("LP_NUM_THREADS",
                                              screen->num_threads);
   screen->num_threads = MIN2(screen->num_threads, LP_MAX_THREADS);


   snprintf(screen->renderer_string, sizeof(screen->renderer_string),
            "llvmpipe (LLVM " MESA_LLVM_VERSION_STRING ", %u bits)",
            lp_build_init_native_width() );

   list_inithead(&screen->ctx_list);
   (void) mtx_init(&screen->ctx_mutex, mtx_plain);
   (void) mtx_init(&screen->cs_mutex, mtx_plain);
   (void) mtx_init(&screen->rast_mutex, mtx_plain);

   (void) mtx_init(&screen->late_mutex, mtx_plain);

   return &screen->base;
}
