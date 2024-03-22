/*
 * Copyright 2017 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "compiler/nir/nir.h"
#include "radeon_uvd_enc.h"
#include "radeon_vce.h"
#include "radeon_video.h"
#include "si_pipe.h"
#include "util/u_cpu_detect.h"
#include "util/u_screen.h"
#include "util/u_video.h"
#include "vl/vl_decoder.h"
#include "vl/vl_video_buffer.h"
#include <sys/utsname.h>

/* The capabilities reported by the kernel has priority
   over the existing logic in si_get_video_param */
#define QUERYABLE_KERNEL   (!!(sscreen->info.drm_minor >= 41))
#define KERNEL_DEC_CAP(codec, attrib)    \
   (codec > PIPE_VIDEO_FORMAT_UNKNOWN && codec <= PIPE_VIDEO_FORMAT_AV1) ? \
   (sscreen->info.dec_caps.codec_info[codec - 1].valid ? \
    sscreen->info.dec_caps.codec_info[codec - 1].attrib : 0) : 0
#define KERNEL_ENC_CAP(codec, attrib)    \
   (codec > PIPE_VIDEO_FORMAT_UNKNOWN && codec <= PIPE_VIDEO_FORMAT_AV1) ? \
   (sscreen->info.enc_caps.codec_info[codec - 1].valid ? \
    sscreen->info.enc_caps.codec_info[codec - 1].attrib : 0) : 0

static const char *si_get_vendor(struct pipe_screen *pscreen)
{
   return "AMD";
}

static const char *si_get_device_vendor(struct pipe_screen *pscreen)
{
   return "AMD";
}

static int si_get_param(struct pipe_screen *pscreen, enum pipe_cap param)
{
   struct si_screen *sscreen = (struct si_screen *)pscreen;

   /* Gfx8 (Polaris11) hangs, so don't enable this on Gfx8 and older chips. */
   bool enable_sparse = sscreen->info.gfx_level >= GFX9 &&
      sscreen->info.has_sparse_vm_mappings;

   switch (param) {
   /* Supported features (boolean caps). */
   case PIPE_CAP_ACCELERATED:
   case PIPE_CAP_MAX_DUAL_SOURCE_RENDER_TARGETS:
   case PIPE_CAP_ANISOTROPIC_FILTER:
   case PIPE_CAP_OCCLUSION_QUERY:
   case PIPE_CAP_TEXTURE_MIRROR_CLAMP:
   case PIPE_CAP_TEXTURE_SHADOW_LOD:
   case PIPE_CAP_TEXTURE_MIRROR_CLAMP_TO_EDGE:
   case PIPE_CAP_BLEND_EQUATION_SEPARATE:
   case PIPE_CAP_TEXTURE_SWIZZLE:
   case PIPE_CAP_DEPTH_CLIP_DISABLE:
   case PIPE_CAP_DEPTH_CLIP_DISABLE_SEPARATE:
   case PIPE_CAP_SHADER_STENCIL_EXPORT:
   case PIPE_CAP_VERTEX_ELEMENT_INSTANCE_DIVISOR:
   case PIPE_CAP_FS_COORD_ORIGIN_UPPER_LEFT:
   case PIPE_CAP_FS_COORD_PIXEL_CENTER_HALF_INTEGER:
   case PIPE_CAP_FS_COORD_PIXEL_CENTER_INTEGER:
   case PIPE_CAP_FRAGMENT_SHADER_TEXTURE_LOD:
   case PIPE_CAP_FRAGMENT_SHADER_DERIVATIVES:
   case PIPE_CAP_PRIMITIVE_RESTART:
   case PIPE_CAP_PRIMITIVE_RESTART_FIXED_INDEX:
   case PIPE_CAP_CONDITIONAL_RENDER:
   case PIPE_CAP_TEXTURE_BARRIER:
   case PIPE_CAP_INDEP_BLEND_ENABLE:
   case PIPE_CAP_INDEP_BLEND_FUNC:
   case PIPE_CAP_VERTEX_COLOR_UNCLAMPED:
   case PIPE_CAP_START_INSTANCE:
   case PIPE_CAP_NPOT_TEXTURES:
   case PIPE_CAP_MIXED_FRAMEBUFFER_SIZES:
   case PIPE_CAP_MIXED_COLOR_DEPTH_BITS:
   case PIPE_CAP_VERTEX_COLOR_CLAMPED:
   case PIPE_CAP_FRAGMENT_COLOR_CLAMPED:
   case PIPE_CAP_VS_INSTANCEID:
   case PIPE_CAP_COMPUTE:
   case PIPE_CAP_TEXTURE_BUFFER_OBJECTS:
   case PIPE_CAP_VS_LAYER_VIEWPORT:
   case PIPE_CAP_QUERY_PIPELINE_STATISTICS:
   case PIPE_CAP_SAMPLE_SHADING:
   case PIPE_CAP_DRAW_INDIRECT:
   case PIPE_CAP_CLIP_HALFZ:
   case PIPE_CAP_VS_WINDOW_SPACE_POSITION:
   case PIPE_CAP_POLYGON_OFFSET_CLAMP:
   case PIPE_CAP_MULTISAMPLE_Z_RESOLVE:
   case PIPE_CAP_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION:
   case PIPE_CAP_TGSI_TEXCOORD:
   case PIPE_CAP_FS_FINE_DERIVATIVE:
   case PIPE_CAP_CONDITIONAL_RENDER_INVERTED:
   case PIPE_CAP_TEXTURE_FLOAT_LINEAR:
   case PIPE_CAP_TEXTURE_HALF_FLOAT_LINEAR:
   case PIPE_CAP_DEPTH_BOUNDS_TEST:
   case PIPE_CAP_SAMPLER_VIEW_TARGET:
   case PIPE_CAP_TEXTURE_QUERY_LOD:
   case PIPE_CAP_TEXTURE_GATHER_SM5:
   case PIPE_CAP_TEXTURE_QUERY_SAMPLES:
   case PIPE_CAP_FORCE_PERSAMPLE_INTERP:
   case PIPE_CAP_COPY_BETWEEN_COMPRESSED_AND_PLAIN_FORMATS:
   case PIPE_CAP_FS_POSITION_IS_SYSVAL:
   case PIPE_CAP_FS_FACE_IS_INTEGER_SYSVAL:
   case PIPE_CAP_INVALIDATE_BUFFER:
   case PIPE_CAP_SURFACE_REINTERPRET_BLOCKS:
   case PIPE_CAP_QUERY_BUFFER_OBJECT:
   case PIPE_CAP_QUERY_MEMORY_INFO:
   case PIPE_CAP_SHADER_PACK_HALF_FLOAT:
   case PIPE_CAP_FRAMEBUFFER_NO_ATTACHMENT:
   case PIPE_CAP_ROBUST_BUFFER_ACCESS_BEHAVIOR:
   case PIPE_CAP_POLYGON_OFFSET_UNITS_UNSCALED:
   case PIPE_CAP_STRING_MARKER:
   case PIPE_CAP_CULL_DISTANCE:
   case PIPE_CAP_SHADER_ARRAY_COMPONENTS:
   case PIPE_CAP_SHADER_CAN_READ_OUTPUTS:
   case PIPE_CAP_STREAM_OUTPUT_PAUSE_RESUME:
   case PIPE_CAP_STREAM_OUTPUT_INTERLEAVE_BUFFERS:
   case PIPE_CAP_DOUBLES:
   case PIPE_CAP_TGSI_TEX_TXF_LZ:
   case PIPE_CAP_TES_LAYER_VIEWPORT:
   case PIPE_CAP_BINDLESS_TEXTURE:
   case PIPE_CAP_QUERY_TIMESTAMP:
   case PIPE_CAP_QUERY_TIME_ELAPSED:
   case PIPE_CAP_NIR_SAMPLERS_AS_DEREF:
   case PIPE_CAP_MEMOBJ:
   case PIPE_CAP_LOAD_CONSTBUF:
   case PIPE_CAP_INT64:
   case PIPE_CAP_SHADER_CLOCK:
   case PIPE_CAP_CAN_BIND_CONST_BUFFER_AS_VERTEX:
   case PIPE_CAP_ALLOW_MAPPED_BUFFERS_DURING_EXECUTION:
   case PIPE_CAP_SIGNED_VERTEX_BUFFER_OFFSET:
   case PIPE_CAP_SHADER_BALLOT:
   case PIPE_CAP_SHADER_GROUP_VOTE:
   case PIPE_CAP_FBFETCH:
   case PIPE_CAP_COMPUTE_GRID_INFO_LAST_BLOCK:
   case PIPE_CAP_IMAGE_LOAD_FORMATTED:
   case PIPE_CAP_PREFER_COMPUTE_FOR_MULTIMEDIA:
   case PIPE_CAP_TGSI_DIV:
   case PIPE_CAP_PACKED_UNIFORMS:
   case PIPE_CAP_GL_SPIRV:
   case PIPE_CAP_ALPHA_TO_COVERAGE_DITHER_CONTROL:
   case PIPE_CAP_MAP_UNSYNCHRONIZED_THREAD_SAFE:
   case PIPE_CAP_NO_CLIP_ON_COPY_TEX:
   case PIPE_CAP_SHADER_ATOMIC_INT64:
   case PIPE_CAP_FRONTEND_NOOP:
   case PIPE_CAP_DEMOTE_TO_HELPER_INVOCATION:
   case PIPE_CAP_PREFER_REAL_BUFFER_IN_CONSTBUF0:
   case PIPE_CAP_COMPUTE_SHADER_DERIVATIVES:
   case PIPE_CAP_IMAGE_ATOMIC_INC_WRAP:
   case PIPE_CAP_IMAGE_STORE_FORMATTED:
   case PIPE_CAP_ALLOW_DRAW_OUT_OF_ORDER:
   case PIPE_CAP_QUERY_SO_OVERFLOW:
   case PIPE_CAP_GLSL_TESS_LEVELS_AS_INPUTS:
   case PIPE_CAP_DEVICE_RESET_STATUS_QUERY:
   case PIPE_CAP_TEXTURE_MULTISAMPLE:
   case PIPE_CAP_ALLOW_GLTHREAD_BUFFER_SUBDATA_OPT: /* TODO: remove if it's slow */
   case PIPE_CAP_NULL_TEXTURES:
   case PIPE_CAP_HAS_CONST_BW:
      return 1;

   case PIPE_CAP_TEXTURE_TRANSFER_MODES:
      return PIPE_TEXTURE_TRANSFER_BLIT;

   case PIPE_CAP_DRAW_VERTEX_STATE:
      return !(sscreen->debug_flags & DBG(NO_FAST_DISPLAY_LIST));

   case PIPE_CAP_SHADER_SAMPLES_IDENTICAL:
      return sscreen->info.gfx_level < GFX11;

   case PIPE_CAP_GLSL_ZERO_INIT:
      return 2;

   case PIPE_CAP_GENERATE_MIPMAP:
   case PIPE_CAP_SEAMLESS_CUBE_MAP:
   case PIPE_CAP_SEAMLESS_CUBE_MAP_PER_TEXTURE:
   case PIPE_CAP_CUBE_MAP_ARRAY:
      return sscreen->info.has_3d_cube_border_color_mipmap;

   case PIPE_CAP_POST_DEPTH_COVERAGE:
      return sscreen->info.gfx_level >= GFX10;

   case PIPE_CAP_GRAPHICS:
      return sscreen->info.has_graphics;

   case PIPE_CAP_RESOURCE_FROM_USER_MEMORY:
      return !UTIL_ARCH_BIG_ENDIAN && sscreen->info.has_userptr;

   case PIPE_CAP_DEVICE_PROTECTED_SURFACE:
      return sscreen->info.has_tmz_support;

   case PIPE_CAP_MIN_MAP_BUFFER_ALIGNMENT:
      return SI_MAP_BUFFER_ALIGNMENT;

   case PIPE_CAP_MAX_VERTEX_BUFFERS:
      return SI_MAX_ATTRIBS;

   case PIPE_CAP_CONSTANT_BUFFER_OFFSET_ALIGNMENT:
   case PIPE_CAP_TEXTURE_BUFFER_OFFSET_ALIGNMENT:
   case PIPE_CAP_MAX_TEXTURE_GATHER_COMPONENTS:
   case PIPE_CAP_MAX_STREAM_OUTPUT_BUFFERS:
   case PIPE_CAP_MAX_VERTEX_STREAMS:
   case PIPE_CAP_SHADER_BUFFER_OFFSET_ALIGNMENT:
   case PIPE_CAP_MAX_WINDOW_RECTANGLES:
      return 4;

   case PIPE_CAP_GLSL_FEATURE_LEVEL:
   case PIPE_CAP_GLSL_FEATURE_LEVEL_COMPATIBILITY:
      return 460;

   case PIPE_CAP_MAX_TEXTURE_UPLOAD_MEMORY_BUDGET:
      /* Optimal number for good TexSubImage performance on Polaris10. */
      return 64 * 1024 * 1024;

   case PIPE_CAP_GL_BEGIN_END_BUFFER_SIZE:
      return 4096 * 1024;

   case PIPE_CAP_MAX_TEXEL_BUFFER_ELEMENTS_UINT: {
      unsigned max_texels =
         pscreen->get_param(pscreen, PIPE_CAP_MAX_SHADER_BUFFER_SIZE_UINT);

      /* FYI, BUF_RSRC_WORD2.NUM_RECORDS field limit is UINT32_MAX. */

      /* Gfx8 and older use the size in bytes for bounds checking, and the max element size
       * is 16B. Gfx9 and newer use the VGPR index for bounds checking.
       */
      if (sscreen->info.gfx_level <= GFX8)
         max_texels = MIN2(max_texels, UINT32_MAX / 16);
      else
         /* Gallium has a limitation that it can only bind UINT32_MAX bytes, not texels.
          * TODO: Remove this after the gallium interface is changed. */
         max_texels = MIN2(max_texels, UINT32_MAX / 16);

      return max_texels;
   }

   case PIPE_CAP_MAX_CONSTANT_BUFFER_SIZE_UINT:
   case PIPE_CAP_MAX_SHADER_BUFFER_SIZE_UINT: {
      /* Return 1/4th of the heap size as the maximum because the max size is not practically
       * allocatable. Also, this can only return UINT32_MAX at most.
       */
      unsigned max_size = MIN2((sscreen->info.max_heap_size_kb * 1024ull) / 4, UINT32_MAX);

      /* Allow max 512 MB to pass CTS with a 32-bit build. */
      if (sizeof(void*) == 4)
         max_size = MIN2(max_size, 512 * 1024 * 1024);

      return max_size;
   }

   case PIPE_CAP_MAX_TEXTURE_MB:
      /* Allow 1/4th of the heap size. */
      return sscreen->info.max_heap_size_kb / 1024 / 4;

   case PIPE_CAP_VERTEX_BUFFER_OFFSET_4BYTE_ALIGNED_ONLY:
   case PIPE_CAP_VERTEX_BUFFER_STRIDE_4BYTE_ALIGNED_ONLY:
   case PIPE_CAP_VERTEX_ELEMENT_SRC_OFFSET_4BYTE_ALIGNED_ONLY:
   case PIPE_CAP_PREFER_BACK_BUFFER_REUSE:
   case PIPE_CAP_UMA:
   case PIPE_CAP_PREFER_IMM_ARRAYS_AS_CONSTBUF:
      return 0;

   case PIPE_CAP_PERFORMANCE_MONITOR:
      return sscreen->info.gfx_level >= GFX7 && sscreen->info.gfx_level <= GFX10_3;

   case PIPE_CAP_SPARSE_BUFFER_PAGE_SIZE:
      return enable_sparse ? RADEON_SPARSE_PAGE_SIZE : 0;

   case PIPE_CAP_CONTEXT_PRIORITY_MASK:
      if (!sscreen->info.is_amdgpu)
         return 0;
      return PIPE_CONTEXT_PRIORITY_LOW |
             PIPE_CONTEXT_PRIORITY_MEDIUM |
             PIPE_CONTEXT_PRIORITY_HIGH;

   case PIPE_CAP_FENCE_SIGNAL:
      return sscreen->info.has_syncobj;

   case PIPE_CAP_CONSTBUF0_FLAGS:
      return SI_RESOURCE_FLAG_32BIT;

   case PIPE_CAP_NATIVE_FENCE_FD:
      return sscreen->info.has_fence_to_handle;

   case PIPE_CAP_DRAW_PARAMETERS:
   case PIPE_CAP_MULTI_DRAW_INDIRECT:
   case PIPE_CAP_MULTI_DRAW_INDIRECT_PARAMS:
      return sscreen->has_draw_indirect_multi;

   case PIPE_CAP_MAX_SHADER_PATCH_VARYINGS:
      return 30;

   case PIPE_CAP_MAX_VARYINGS:
   case PIPE_CAP_MAX_GS_INVOCATIONS:
      return 32;

   case PIPE_CAP_TEXTURE_BORDER_COLOR_QUIRK:
      return sscreen->info.gfx_level <= GFX8 ? PIPE_QUIRK_TEXTURE_BORDER_COLOR_SWIZZLE_R600 : 0;

   /* Stream output. */
   case PIPE_CAP_MAX_STREAM_OUTPUT_SEPARATE_COMPONENTS:
   case PIPE_CAP_MAX_STREAM_OUTPUT_INTERLEAVED_COMPONENTS:
      return 32 * 4;

   /* Geometry shader output. */
   case PIPE_CAP_MAX_GEOMETRY_OUTPUT_VERTICES:
      /* gfx9 has to report 256 to make piglit/gs-max-output pass.
       * gfx8 and earlier can do 1024.
       */
      return 256;
   case PIPE_CAP_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS:
      return 4095;

   case PIPE_CAP_MAX_VERTEX_ATTRIB_STRIDE:
      return 2048;

   /* Texturing. */
   case PIPE_CAP_MAX_TEXTURE_2D_SIZE:
      return 16384;
   case PIPE_CAP_MAX_TEXTURE_CUBE_LEVELS:
      if (!sscreen->info.has_3d_cube_border_color_mipmap)
         return 0;
      return 15; /* 16384 */
   case PIPE_CAP_MAX_TEXTURE_3D_LEVELS:
      if (!sscreen->info.has_3d_cube_border_color_mipmap)
         return 0;
      if (sscreen->info.gfx_level >= GFX10)
         return 14;
      /* textures support 8192, but layered rendering supports 2048 */
      return 12;
   case PIPE_CAP_MAX_TEXTURE_ARRAY_LAYERS:
      if (sscreen->info.gfx_level >= GFX10)
         return 8192;
      /* textures support 8192, but layered rendering supports 2048 */
      return 2048;

   /* Sparse texture */
   case PIPE_CAP_MAX_SPARSE_TEXTURE_SIZE:
      return enable_sparse ?
         si_get_param(pscreen, PIPE_CAP_MAX_TEXTURE_2D_SIZE) : 0;
   case PIPE_CAP_MAX_SPARSE_3D_TEXTURE_SIZE:
      return enable_sparse ?
         (1 << (si_get_param(pscreen, PIPE_CAP_MAX_TEXTURE_3D_LEVELS) - 1)) : 0;
   case PIPE_CAP_MAX_SPARSE_ARRAY_TEXTURE_LAYERS:
      return enable_sparse ?
         si_get_param(pscreen, PIPE_CAP_MAX_TEXTURE_ARRAY_LAYERS) : 0;
   case PIPE_CAP_SPARSE_TEXTURE_FULL_ARRAY_CUBE_MIPMAPS:
   case PIPE_CAP_QUERY_SPARSE_TEXTURE_RESIDENCY:
   case PIPE_CAP_CLAMP_SPARSE_TEXTURE_LOD:
      return enable_sparse;

   /* Viewports and render targets. */
   case PIPE_CAP_MAX_VIEWPORTS:
      return SI_MAX_VIEWPORTS;
   case PIPE_CAP_VIEWPORT_SUBPIXEL_BITS:
   case PIPE_CAP_RASTERIZER_SUBPIXEL_BITS:
   case PIPE_CAP_MAX_RENDER_TARGETS:
      return 8;
   case PIPE_CAP_FRAMEBUFFER_MSAA_CONSTRAINTS:
      return sscreen->info.has_eqaa_surface_allocator ? 2 : 0;

   case PIPE_CAP_MIN_TEXTURE_GATHER_OFFSET:
   case PIPE_CAP_MIN_TEXEL_OFFSET:
      return -32;

   case PIPE_CAP_MAX_TEXTURE_GATHER_OFFSET:
   case PIPE_CAP_MAX_TEXEL_OFFSET:
      return 31;

   case PIPE_CAP_ENDIANNESS:
      return PIPE_ENDIAN_LITTLE;

   case PIPE_CAP_VENDOR_ID:
      return ATI_VENDOR_ID;
   case PIPE_CAP_DEVICE_ID:
      return sscreen->info.pci_id;
   case PIPE_CAP_VIDEO_MEMORY:
      return sscreen->info.vram_size_kb >> 10;
   case PIPE_CAP_PCI_GROUP:
      return sscreen->info.pci.domain;
   case PIPE_CAP_PCI_BUS:
      return sscreen->info.pci.bus;
   case PIPE_CAP_PCI_DEVICE:
      return sscreen->info.pci.dev;
   case PIPE_CAP_PCI_FUNCTION:
      return sscreen->info.pci.func;

   case PIPE_CAP_TIMER_RESOLUTION:
      /* Conversion to nanos from cycles per millisecond */
      return DIV_ROUND_UP(1000000, sscreen->info.clock_crystal_freq);

   default:
      return u_pipe_screen_get_param_defaults(pscreen, param);
   }
}

static float si_get_paramf(struct pipe_screen *pscreen, enum pipe_capf param)
{
   struct si_screen *sscreen = (struct si_screen *)pscreen;

   switch (param) {
   case PIPE_CAPF_MIN_LINE_WIDTH:
   case PIPE_CAPF_MIN_LINE_WIDTH_AA:
      return 1; /* due to axis-aligned end caps at line width 1 */
   case PIPE_CAPF_MIN_POINT_SIZE:
   case PIPE_CAPF_MIN_POINT_SIZE_AA:
   case PIPE_CAPF_POINT_SIZE_GRANULARITY:
   case PIPE_CAPF_LINE_WIDTH_GRANULARITY:
      return 1.0 / 8.0; /* due to the register field precision */
   case PIPE_CAPF_MAX_LINE_WIDTH:
   case PIPE_CAPF_MAX_LINE_WIDTH_AA:
      /* This depends on the quant mode, though the precise interactions
       * are unknown. */
      return 2048;
   case PIPE_CAPF_MAX_POINT_SIZE:
   case PIPE_CAPF_MAX_POINT_SIZE_AA:
      return SI_MAX_POINT_SIZE;
   case PIPE_CAPF_MAX_TEXTURE_ANISOTROPY:
      return 16.0f;
   case PIPE_CAPF_MAX_TEXTURE_LOD_BIAS:
      /* This is the maximum value of the LOD_BIAS sampler field. */
      return sscreen->info.gfx_level >= GFX10 ? 31 : 16;
   case PIPE_CAPF_MIN_CONSERVATIVE_RASTER_DILATE:
   case PIPE_CAPF_MAX_CONSERVATIVE_RASTER_DILATE:
   case PIPE_CAPF_CONSERVATIVE_RASTER_DILATE_GRANULARITY:
      return 0.0f;
   }
   return 0.0f;
}

static int si_get_shader_param(struct pipe_screen *pscreen, enum pipe_shader_type shader,
                               enum pipe_shader_cap param)
{
   struct si_screen *sscreen = (struct si_screen *)pscreen;

   if (shader == PIPE_SHADER_MESH ||
       shader == PIPE_SHADER_TASK)
      return 0;

   switch (param) {
   /* Shader limits. */
   case PIPE_SHADER_CAP_MAX_INSTRUCTIONS:
   case PIPE_SHADER_CAP_MAX_ALU_INSTRUCTIONS:
   case PIPE_SHADER_CAP_MAX_TEX_INSTRUCTIONS:
   case PIPE_SHADER_CAP_MAX_TEX_INDIRECTIONS:
   case PIPE_SHADER_CAP_MAX_CONTROL_FLOW_DEPTH:
      return 16384;
   case PIPE_SHADER_CAP_MAX_INPUTS:
      return shader == PIPE_SHADER_VERTEX ? SI_MAX_ATTRIBS : 32;
   case PIPE_SHADER_CAP_MAX_OUTPUTS:
      return shader == PIPE_SHADER_FRAGMENT ? 8 : 32;
   case PIPE_SHADER_CAP_MAX_TEMPS:
      return 256; /* Max native temporaries. */
   case PIPE_SHADER_CAP_MAX_CONST_BUFFER0_SIZE:
      return 1 << 26; /* 64 MB */
   case PIPE_SHADER_CAP_MAX_CONST_BUFFERS:
      return SI_NUM_CONST_BUFFERS;
   case PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS:
   case PIPE_SHADER_CAP_MAX_SAMPLER_VIEWS:
      return SI_NUM_SAMPLERS;
   case PIPE_SHADER_CAP_MAX_SHADER_BUFFERS:
      return SI_NUM_SHADER_BUFFERS;
   case PIPE_SHADER_CAP_MAX_SHADER_IMAGES:
      return SI_NUM_IMAGES;

   case PIPE_SHADER_CAP_SUPPORTED_IRS:
      if (shader == PIPE_SHADER_COMPUTE) {
         return (1 << PIPE_SHADER_IR_NATIVE) |
                (1 << PIPE_SHADER_IR_NIR) |
                (1 << PIPE_SHADER_IR_TGSI);
      }
      return (1 << PIPE_SHADER_IR_TGSI) |
             (1 << PIPE_SHADER_IR_NIR);

   /* Supported boolean features. */
   case PIPE_SHADER_CAP_CONT_SUPPORTED:
   case PIPE_SHADER_CAP_TGSI_SQRT_SUPPORTED:
   case PIPE_SHADER_CAP_INDIRECT_TEMP_ADDR:
   case PIPE_SHADER_CAP_INDIRECT_CONST_ADDR:
   case PIPE_SHADER_CAP_INTEGERS:
   case PIPE_SHADER_CAP_INT64_ATOMICS:
   case PIPE_SHADER_CAP_TGSI_ANY_INOUT_DECL_RANGE:
   case PIPE_SHADER_CAP_INDIRECT_INPUT_ADDR: /* lowered in finalize_nir */
   case PIPE_SHADER_CAP_INDIRECT_OUTPUT_ADDR: /* lowered in finalize_nir */
      return 1;

   case PIPE_SHADER_CAP_FP16_CONST_BUFFERS:
      /* We need f16c for fast FP16 conversions in glUniform. */
      if (!util_get_cpu_caps()->has_f16c)
         return 0;
      FALLTHROUGH;
   case PIPE_SHADER_CAP_FP16:
   case PIPE_SHADER_CAP_FP16_DERIVATIVES:
   case PIPE_SHADER_CAP_GLSL_16BIT_CONSTS:
   case PIPE_SHADER_CAP_INT16:
      return sscreen->info.gfx_level >= GFX8 && sscreen->options.fp16;

   /* Unsupported boolean features. */
   case PIPE_SHADER_CAP_SUBROUTINES:
   case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTERS:
   case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTER_BUFFERS:
      return 0;
   }
   return 0;
}

static const void *si_get_compiler_options(struct pipe_screen *screen, enum pipe_shader_ir ir,
                                           enum pipe_shader_type shader)
{
   struct si_screen *sscreen = (struct si_screen *)screen;

   assert(ir == PIPE_SHADER_IR_NIR);
   return sscreen->nir_options;
}

static void si_get_driver_uuid(struct pipe_screen *pscreen, char *uuid)
{
   ac_compute_driver_uuid(uuid, PIPE_UUID_SIZE);
}

static void si_get_device_uuid(struct pipe_screen *pscreen, char *uuid)
{
   struct si_screen *sscreen = (struct si_screen *)pscreen;

   ac_compute_device_uuid(&sscreen->info, uuid, PIPE_UUID_SIZE);
}

static const char *si_get_name(struct pipe_screen *pscreen)
{
   struct si_screen *sscreen = (struct si_screen *)pscreen;

   return sscreen->renderer_string;
}

static int si_get_video_param_no_video_hw(struct pipe_screen *screen, enum pipe_video_profile profile,
                                          enum pipe_video_entrypoint entrypoint,
                                          enum pipe_video_cap param)
{
   switch (param) {
   case PIPE_VIDEO_CAP_SUPPORTED:
      return vl_profile_supported(screen, profile, entrypoint);
   case PIPE_VIDEO_CAP_NPOT_TEXTURES:
      return 1;
   case PIPE_VIDEO_CAP_MAX_WIDTH:
   case PIPE_VIDEO_CAP_MAX_HEIGHT:
      return vl_video_buffer_max_size(screen);
   case PIPE_VIDEO_CAP_PREFERED_FORMAT:
      return PIPE_FORMAT_NV12;
   case PIPE_VIDEO_CAP_PREFERS_INTERLACED:
      return false;
   case PIPE_VIDEO_CAP_SUPPORTS_INTERLACED:
      return false;
   case PIPE_VIDEO_CAP_SUPPORTS_PROGRESSIVE:
      return true;
   case PIPE_VIDEO_CAP_MAX_LEVEL:
      return vl_level_supported(screen, profile);
   default:
      return 0;
   }
}

static int si_get_video_param(struct pipe_screen *screen, enum pipe_video_profile profile,
                              enum pipe_video_entrypoint entrypoint, enum pipe_video_cap param)
{
   struct si_screen *sscreen = (struct si_screen *)screen;
   enum pipe_video_format codec = u_reduce_video_profile(profile);
   bool fully_supported_profile = ((profile >= PIPE_VIDEO_PROFILE_MPEG4_AVC_BASELINE) &&
                                   (profile <= PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH)) ||
                                  (profile == PIPE_VIDEO_PROFILE_HEVC_MAIN) ||
                                  (profile == PIPE_VIDEO_PROFILE_AV1_MAIN);

   /* Return the capability of Video Post Processor.
    * Have to determine the HW version of VPE.
    * Have to check the HW limitation and
    * Check if the VPE exists and is valid
    */
   if (sscreen->info.ip[AMD_IP_VPE].num_queues && entrypoint == PIPE_VIDEO_ENTRYPOINT_PROCESSING) {

      switch(param) {
      case PIPE_VIDEO_CAP_SUPPORTED:
         return true;
      case PIPE_VIDEO_CAP_MAX_WIDTH:
         return 10240;
      case PIPE_VIDEO_CAP_MAX_HEIGHT:
         return 10240;
      case PIPE_VIDEO_CAP_VPP_MAX_INPUT_WIDTH:
         return 10240;
      case PIPE_VIDEO_CAP_VPP_MAX_INPUT_HEIGHT:
         return 10240;
      case PIPE_VIDEO_CAP_VPP_MIN_INPUT_WIDTH:
         return 16;
      case PIPE_VIDEO_CAP_VPP_MIN_INPUT_HEIGHT:
         return 16;
      case PIPE_VIDEO_CAP_VPP_MAX_OUTPUT_WIDTH:
         return 10240;
      case PIPE_VIDEO_CAP_VPP_MAX_OUTPUT_HEIGHT:
         return 10240;
      case PIPE_VIDEO_CAP_VPP_MIN_OUTPUT_WIDTH:
         return 16;
      case PIPE_VIDEO_CAP_VPP_MIN_OUTPUT_HEIGHT:
         return 16;
      case PIPE_VIDEO_CAP_VPP_ORIENTATION_MODES:
         /* VPE 1st generation does not support orientation
          * Have to determine the version and features of VPE in future.
          */
         return PIPE_VIDEO_VPP_ORIENTATION_DEFAULT;
      case PIPE_VIDEO_CAP_VPP_BLEND_MODES:
         /* VPE 1st generation does not support blending.
          * Have to determine the version and features of VPE in future.
          */
         return PIPE_VIDEO_VPP_BLEND_MODE_NONE;
      case PIPE_VIDEO_CAP_PREFERED_FORMAT:
         return PIPE_FORMAT_NV12;
      case PIPE_VIDEO_CAP_PREFERS_INTERLACED:
         return false;
      case PIPE_VIDEO_CAP_SUPPORTS_PROGRESSIVE:
         return true;
      case PIPE_VIDEO_CAP_REQUIRES_FLUSH_ON_END_FRAME:
         /* true: VPP flush function will be called within vaEndPicture() */
         /* false: VPP flush function will be skipped */
         return false;
      case PIPE_VIDEO_CAP_SUPPORTS_INTERLACED:
         /* for VPE we prefer non-interlaced buffer */
         return false;
      default:
         return 0;
      }
   }

   if (entrypoint == PIPE_VIDEO_ENTRYPOINT_ENCODE) {
      if (!(sscreen->info.ip[AMD_IP_VCE].num_queues ||
            sscreen->info.ip[AMD_IP_UVD_ENC].num_queues ||
            sscreen->info.ip[AMD_IP_VCN_ENC].num_queues))
         return 0;

      if (sscreen->info.vcn_ip_version == VCN_4_0_3)
	 return 0;

      switch (param) {
      case PIPE_VIDEO_CAP_SUPPORTED:
         return (
             /* in case it is explicitly marked as not supported by the kernel */
            ((QUERYABLE_KERNEL && fully_supported_profile) ? KERNEL_ENC_CAP(codec, valid) : 1) &&
            ((codec == PIPE_VIDEO_FORMAT_MPEG4_AVC && profile != PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH10 &&
             (sscreen->info.vcn_ip_version >= VCN_1_0_0 || si_vce_is_fw_version_supported(sscreen))) ||
            (profile == PIPE_VIDEO_PROFILE_HEVC_MAIN &&
             (sscreen->info.vcn_ip_version >= VCN_1_0_0 || si_radeon_uvd_enc_supported(sscreen))) ||
            (profile == PIPE_VIDEO_PROFILE_HEVC_MAIN_10 && sscreen->info.vcn_ip_version >= VCN_2_0_0) ||
            (profile == PIPE_VIDEO_PROFILE_AV1_MAIN &&
	     (sscreen->info.vcn_ip_version >= VCN_4_0_0 && sscreen->info.vcn_ip_version != VCN_4_0_3))));
      case PIPE_VIDEO_CAP_NPOT_TEXTURES:
         return 1;
      case PIPE_VIDEO_CAP_MIN_WIDTH:
         return 256;
      case PIPE_VIDEO_CAP_MIN_HEIGHT:
         return 128;
      case PIPE_VIDEO_CAP_MAX_WIDTH:
         if (codec != PIPE_VIDEO_FORMAT_UNKNOWN && QUERYABLE_KERNEL)
            return KERNEL_ENC_CAP(codec, max_width);
         else
            return (sscreen->info.family < CHIP_TONGA) ? 2048 : 4096;
      case PIPE_VIDEO_CAP_MAX_HEIGHT:
         if (codec != PIPE_VIDEO_FORMAT_UNKNOWN && QUERYABLE_KERNEL)
            return KERNEL_ENC_CAP(codec, max_height);
         else
            return (sscreen->info.family < CHIP_TONGA) ? 1152 : 2304;
      case PIPE_VIDEO_CAP_PREFERED_FORMAT:
         if (profile == PIPE_VIDEO_PROFILE_HEVC_MAIN_10)
            return PIPE_FORMAT_P010;
         else
            return PIPE_FORMAT_NV12;
      case PIPE_VIDEO_CAP_PREFERS_INTERLACED:
         return false;
      case PIPE_VIDEO_CAP_SUPPORTS_INTERLACED:
         return false;
      case PIPE_VIDEO_CAP_SUPPORTS_PROGRESSIVE:
         return true;
      case PIPE_VIDEO_CAP_STACKED_FRAMES:
         return (sscreen->info.family < CHIP_TONGA) ? 1 : 2;
      case PIPE_VIDEO_CAP_MAX_TEMPORAL_LAYERS:
         return (codec == PIPE_VIDEO_FORMAT_MPEG4_AVC &&
                 sscreen->info.vcn_ip_version >= VCN_1_0_0) ? 4 : 0;
      case PIPE_VIDEO_CAP_ENC_QUALITY_LEVEL:
         return (sscreen->info.vcn_ip_version >= VCN_1_0_0) ? 32 : 0;
      case PIPE_VIDEO_CAP_ENC_SUPPORTS_MAX_FRAME_SIZE:
         return (sscreen->info.vcn_ip_version >= VCN_1_0_0) ? 1 : 0;

      case PIPE_VIDEO_CAP_ENC_HEVC_FEATURE_FLAGS:
         if ((sscreen->info.vcn_ip_version >= VCN_1_0_0) &&
               (profile == PIPE_VIDEO_PROFILE_HEVC_MAIN ||
             profile == PIPE_VIDEO_PROFILE_HEVC_MAIN_10)) {
            union pipe_h265_enc_cap_features pipe_features;
            pipe_features.value = 0;

            pipe_features.bits.amp = PIPE_ENC_FEATURE_SUPPORTED;
            pipe_features.bits.strong_intra_smoothing = PIPE_ENC_FEATURE_SUPPORTED;
            pipe_features.bits.constrained_intra_pred = PIPE_ENC_FEATURE_SUPPORTED;
            pipe_features.bits.deblocking_filter_disable
                                                      = PIPE_ENC_FEATURE_SUPPORTED;
            if (sscreen->info.vcn_ip_version >= VCN_2_0_0)
               pipe_features.bits.sao = PIPE_ENC_FEATURE_SUPPORTED;

            return pipe_features.value;
         } else
            return 0;

      case PIPE_VIDEO_CAP_ENC_HEVC_BLOCK_SIZES:
         if (sscreen->info.vcn_ip_version >= VCN_1_0_0 &&
             (profile == PIPE_VIDEO_PROFILE_HEVC_MAIN ||
              profile == PIPE_VIDEO_PROFILE_HEVC_MAIN_10)) {
            union pipe_h265_enc_cap_block_sizes pipe_block_sizes;
            pipe_block_sizes.value = 0;

            pipe_block_sizes.bits.log2_max_coding_tree_block_size_minus3 = 3;
            pipe_block_sizes.bits.log2_min_coding_tree_block_size_minus3 = 3;
            pipe_block_sizes.bits.log2_min_luma_coding_block_size_minus3 = 0;
            pipe_block_sizes.bits.log2_max_luma_transform_block_size_minus2 = 3;
            pipe_block_sizes.bits.log2_min_luma_transform_block_size_minus2 = 0;

            return pipe_block_sizes.value;
         } else
            return 0;

      case PIPE_VIDEO_CAP_ENC_SUPPORTS_ASYNC_OPERATION:
         return (sscreen->info.vcn_ip_version >= VCN_1_0_0) ? 1 : 0;

      case PIPE_VIDEO_CAP_ENC_MAX_SLICES_PER_FRAME:
         return (sscreen->info.vcn_ip_version >= VCN_1_0_0) ? 128 : 1;

      case PIPE_VIDEO_CAP_ENC_SLICES_STRUCTURE:
         if (sscreen->info.vcn_ip_version >= VCN_2_0_0) {
            int value = (PIPE_VIDEO_CAP_SLICE_STRUCTURE_POWER_OF_TWO_ROWS |
                         PIPE_VIDEO_CAP_SLICE_STRUCTURE_EQUAL_ROWS |
                         PIPE_VIDEO_CAP_SLICE_STRUCTURE_EQUAL_MULTI_ROWS);
            return value;
         } else
            return 0;

      case PIPE_VIDEO_CAP_ENC_AV1_FEATURE:
         if (sscreen->info.vcn_ip_version >= VCN_4_0_0 && sscreen->info.vcn_ip_version != VCN_4_0_3) {
            union pipe_av1_enc_cap_features attrib;
            attrib.value = 0;

            attrib.bits.support_128x128_superblock = PIPE_ENC_FEATURE_NOT_SUPPORTED;
            attrib.bits.support_filter_intra = PIPE_ENC_FEATURE_NOT_SUPPORTED;
            attrib.bits.support_intra_edge_filter = PIPE_ENC_FEATURE_NOT_SUPPORTED;
            attrib.bits.support_interintra_compound = PIPE_ENC_FEATURE_NOT_SUPPORTED;
            attrib.bits.support_masked_compound = PIPE_ENC_FEATURE_NOT_SUPPORTED;
            attrib.bits.support_warped_motion = PIPE_ENC_FEATURE_NOT_SUPPORTED;
            attrib.bits.support_palette_mode = PIPE_ENC_FEATURE_SUPPORTED;
            attrib.bits.support_dual_filter = PIPE_ENC_FEATURE_NOT_SUPPORTED;
            attrib.bits.support_jnt_comp = PIPE_ENC_FEATURE_NOT_SUPPORTED;
            attrib.bits.support_ref_frame_mvs = PIPE_ENC_FEATURE_NOT_SUPPORTED;
            attrib.bits.support_superres = PIPE_ENC_FEATURE_NOT_SUPPORTED;
            attrib.bits.support_restoration = PIPE_ENC_FEATURE_NOT_SUPPORTED;
            attrib.bits.support_allow_intrabc = PIPE_ENC_FEATURE_NOT_SUPPORTED;
            attrib.bits.support_cdef_channel_strength = PIPE_ENC_FEATURE_SUPPORTED;

            return attrib.value;
         } else
            return 0;

      case PIPE_VIDEO_CAP_ENC_AV1_FEATURE_EXT1:
         if (sscreen->info.vcn_ip_version >= VCN_4_0_0 && sscreen->info.vcn_ip_version != VCN_4_0_3) {
            union pipe_av1_enc_cap_features_ext1 attrib_ext1;
            attrib_ext1.value = 0;
            attrib_ext1.bits.interpolation_filter = PIPE_VIDEO_CAP_ENC_AV1_INTERPOLATION_FILTER_EIGHT_TAP |
                           PIPE_VIDEO_CAP_ENC_AV1_INTERPOLATION_FILTER_EIGHT_TAP_SMOOTH |
                           PIPE_VIDEO_CAP_ENC_AV1_INTERPOLATION_FILTER_EIGHT_TAP_SHARP |
                           PIPE_VIDEO_CAP_ENC_AV1_INTERPOLATION_FILTER_BILINEAR |
                           PIPE_VIDEO_CAP_ENC_AV1_INTERPOLATION_FILTER_SWITCHABLE;
            attrib_ext1.bits.min_segid_block_size_accepted = 0;
            attrib_ext1.bits.segment_feature_support = 0;

            return attrib_ext1.value;
         } else
            return 0;

      case PIPE_VIDEO_CAP_ENC_AV1_FEATURE_EXT2:
         if (sscreen->info.vcn_ip_version >= VCN_4_0_0 && sscreen->info.vcn_ip_version != VCN_4_0_3) {
            union pipe_av1_enc_cap_features_ext2 attrib_ext2;
            attrib_ext2.value = 0;

           attrib_ext2.bits.tile_size_bytes_minus1 = 1;
           attrib_ext2.bits.obu_size_bytes_minus1 = 1;
           /**
            * tx_mode supported.
            * (tx_mode_support & 0x01) == 1: ONLY_4X4 is supported, 0: not.
            * (tx_mode_support & 0x02) == 1: TX_MODE_LARGEST is supported, 0: not.
            * (tx_mode_support & 0x04) == 1: TX_MODE_SELECT is supported, 0: not.
            */
           attrib_ext2.bits.tx_mode_support = PIPE_VIDEO_CAP_ENC_AV1_TX_MODE_SELECT;
           attrib_ext2.bits.max_tile_num_minus1 = 31;

            return attrib_ext2.value;
         } else
            return 0;
      case PIPE_VIDEO_CAP_ENC_SUPPORTS_TILE:
         if ((sscreen->info.vcn_ip_version >= VCN_4_0_0 && sscreen->info.vcn_ip_version != VCN_4_0_3) &&
              profile == PIPE_VIDEO_PROFILE_AV1_MAIN)
            return 1;
         else
            return 0;
      case PIPE_VIDEO_CAP_EFC_SUPPORTED:
         return ((sscreen->info.family > CHIP_RENOIR) &&
                 !(sscreen->debug_flags & DBG(NO_EFC)));

      case PIPE_VIDEO_CAP_ENC_MAX_REFERENCES_PER_FRAME:
         if (sscreen->info.vcn_ip_version >= VCN_3_0_0) {
            int refPicList0 = 1;
            int refPicList1 = codec == PIPE_VIDEO_FORMAT_MPEG4_AVC ? 1 : 0;
            return refPicList0 | (refPicList1 << 16);
         } else
            return 1;

      case PIPE_VIDEO_CAP_ENC_INTRA_REFRESH:
         if (sscreen->info.vcn_ip_version >= VCN_1_0_0) {
            int value = PIPE_VIDEO_ENC_INTRA_REFRESH_ROW |
                        PIPE_VIDEO_ENC_INTRA_REFRESH_COLUMN |
                        PIPE_VIDEO_ENC_INTRA_REFRESH_P_FRAME;
            return value;
         }
         else
            return 0;

      case PIPE_VIDEO_CAP_ENC_ROI:
         if (sscreen->info.vcn_ip_version >= VCN_1_0_0) {
            union pipe_enc_cap_roi attrib;
            attrib.value = 0;

            attrib.bits.num_roi_regions = PIPE_ENC_ROI_REGION_NUM_MAX;
            attrib.bits.roi_rc_priority_support = PIPE_ENC_FEATURE_NOT_SUPPORTED;
            attrib.bits.roi_rc_qp_delta_support = PIPE_ENC_FEATURE_SUPPORTED;
            return attrib.value;
         }
         else
            return 0;

      default:
         return 0;
      }
   }

   switch (param) {
   case PIPE_VIDEO_CAP_SUPPORTED:
      if (codec != PIPE_VIDEO_FORMAT_JPEG &&
          !(sscreen->info.ip[AMD_IP_UVD].num_queues ||
            ((sscreen->info.vcn_ip_version >= VCN_4_0_0) ?
	      sscreen->info.ip[AMD_IP_VCN_UNIFIED].num_queues :
	      sscreen->info.ip[AMD_IP_VCN_DEC].num_queues)))
         return false;
      if (QUERYABLE_KERNEL && fully_supported_profile &&
          sscreen->info.vcn_ip_version >= VCN_1_0_0)
         return KERNEL_DEC_CAP(codec, valid);
      if (codec < PIPE_VIDEO_FORMAT_MPEG4_AVC &&
          sscreen->info.vcn_ip_version >= VCN_3_0_33)
         return false;

      switch (codec) {
      case PIPE_VIDEO_FORMAT_MPEG12:
         return !(sscreen->info.vcn_ip_version >= VCN_3_0_33 || profile == PIPE_VIDEO_PROFILE_MPEG1);
      case PIPE_VIDEO_FORMAT_MPEG4:
         return !(sscreen->info.vcn_ip_version >= VCN_3_0_33);
      case PIPE_VIDEO_FORMAT_MPEG4_AVC:
         if ((sscreen->info.family == CHIP_POLARIS10 || sscreen->info.family == CHIP_POLARIS11) &&
             sscreen->info.uvd_fw_version < UVD_FW_1_66_16) {
            RVID_ERR("POLARIS10/11 firmware version need to be updated.\n");
            return false;
         }
         return (profile != PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH10);
      case PIPE_VIDEO_FORMAT_VC1:
         return !(sscreen->info.vcn_ip_version >= VCN_3_0_33);
      case PIPE_VIDEO_FORMAT_HEVC:
         /* Carrizo only supports HEVC Main */
         if (sscreen->info.family >= CHIP_STONEY)
            return (profile == PIPE_VIDEO_PROFILE_HEVC_MAIN ||
                    profile == PIPE_VIDEO_PROFILE_HEVC_MAIN_10);
         else if (sscreen->info.family >= CHIP_CARRIZO)
            return profile == PIPE_VIDEO_PROFILE_HEVC_MAIN;
         return false;
      case PIPE_VIDEO_FORMAT_JPEG:
         if (sscreen->info.vcn_ip_version >= VCN_1_0_0) {
            if (!sscreen->info.ip[AMD_IP_VCN_JPEG].num_queues)
               return false;
            else
               return true;
         }
         if (sscreen->info.family < CHIP_CARRIZO || sscreen->info.family >= CHIP_VEGA10)
            return false;
         if (!sscreen->info.is_amdgpu) {
            RVID_ERR("No MJPEG support for the kernel version\n");
            return false;
         }
         return true;
      case PIPE_VIDEO_FORMAT_VP9:
         return sscreen->info.vcn_ip_version >= VCN_1_0_0;
      case PIPE_VIDEO_FORMAT_AV1:
         return sscreen->info.vcn_ip_version >= VCN_3_0_0 && sscreen->info.vcn_ip_version != VCN_3_0_33;
      default:
         return false;
      }
   case PIPE_VIDEO_CAP_NPOT_TEXTURES:
      return 1;
   case PIPE_VIDEO_CAP_MIN_WIDTH:
   case PIPE_VIDEO_CAP_MIN_HEIGHT:
      return 64;
   case PIPE_VIDEO_CAP_MAX_WIDTH:
      if (codec != PIPE_VIDEO_FORMAT_UNKNOWN && QUERYABLE_KERNEL)
            return KERNEL_DEC_CAP(codec, max_width);
      else {
         switch (codec) {
         case PIPE_VIDEO_FORMAT_HEVC:
         case PIPE_VIDEO_FORMAT_VP9:
         case PIPE_VIDEO_FORMAT_AV1:
            return (sscreen->info.vcn_ip_version < VCN_2_0_0) ?
               ((sscreen->info.family < CHIP_TONGA) ? 2048 : 4096) : 8192;
         default:
            return (sscreen->info.family < CHIP_TONGA) ? 2048 : 4096;
         }
      }
   case PIPE_VIDEO_CAP_MAX_HEIGHT:
      if (codec != PIPE_VIDEO_FORMAT_UNKNOWN && QUERYABLE_KERNEL)
            return KERNEL_DEC_CAP(codec, max_height);
      else {
         switch (codec) {
         case PIPE_VIDEO_FORMAT_HEVC:
         case PIPE_VIDEO_FORMAT_VP9:
         case PIPE_VIDEO_FORMAT_AV1:
            return (sscreen->info.vcn_ip_version < VCN_2_0_0) ?
               ((sscreen->info.family < CHIP_TONGA) ? 1152 : 4096) : 4352;
         default:
            return (sscreen->info.family < CHIP_TONGA) ? 1152 : 4096;
         }
      }
   case PIPE_VIDEO_CAP_PREFERED_FORMAT:
      if (profile == PIPE_VIDEO_PROFILE_HEVC_MAIN_10)
         return PIPE_FORMAT_P010;
      else if (profile == PIPE_VIDEO_PROFILE_VP9_PROFILE2)
         return PIPE_FORMAT_P010;
      else
         return PIPE_FORMAT_NV12;

   case PIPE_VIDEO_CAP_PREFERS_INTERLACED:
      return false;
   case PIPE_VIDEO_CAP_SUPPORTS_INTERLACED: {
      enum pipe_video_format format = u_reduce_video_profile(profile);

      if (format >= PIPE_VIDEO_FORMAT_HEVC)
         return false;

      return true;
   }
   case PIPE_VIDEO_CAP_SUPPORTS_PROGRESSIVE:
      return true;
   case PIPE_VIDEO_CAP_MAX_LEVEL:
      if ((profile == PIPE_VIDEO_PROFILE_MPEG2_SIMPLE ||
           profile == PIPE_VIDEO_PROFILE_MPEG2_MAIN ||
           profile == PIPE_VIDEO_PROFILE_MPEG4_ADVANCED_SIMPLE ||
           profile == PIPE_VIDEO_PROFILE_VC1_ADVANCED) &&
          sscreen->info.dec_caps.codec_info[codec - 1].valid) {
         return sscreen->info.dec_caps.codec_info[codec - 1].max_level;
      } else {
         switch (profile) {
         case PIPE_VIDEO_PROFILE_MPEG1:
            return 0;
         case PIPE_VIDEO_PROFILE_MPEG2_SIMPLE:
         case PIPE_VIDEO_PROFILE_MPEG2_MAIN:
            return 3;
         case PIPE_VIDEO_PROFILE_MPEG4_SIMPLE:
            return 3;
         case PIPE_VIDEO_PROFILE_MPEG4_ADVANCED_SIMPLE:
            return 5;
         case PIPE_VIDEO_PROFILE_VC1_SIMPLE:
            return 1;
         case PIPE_VIDEO_PROFILE_VC1_MAIN:
            return 2;
         case PIPE_VIDEO_PROFILE_VC1_ADVANCED:
            return 4;
         case PIPE_VIDEO_PROFILE_MPEG4_AVC_BASELINE:
         case PIPE_VIDEO_PROFILE_MPEG4_AVC_MAIN:
         case PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH:
            return (sscreen->info.family < CHIP_TONGA) ? 41 : 52;
         case PIPE_VIDEO_PROFILE_HEVC_MAIN:
         case PIPE_VIDEO_PROFILE_HEVC_MAIN_10:
            return 186;
         default:
            return 0;
         }
      }
   case PIPE_VIDEO_CAP_SUPPORTS_CONTIGUOUS_PLANES_MAP:
      return true;
   case PIPE_VIDEO_CAP_ROI_CROP_DEC:
      if (codec == PIPE_VIDEO_FORMAT_JPEG &&
          sscreen->info.vcn_ip_version == VCN_4_0_3)
         return true;
      return false;
   default:
      return 0;
   }
}

static bool si_vid_is_format_supported(struct pipe_screen *screen, enum pipe_format format,
                                       enum pipe_video_profile profile,
                                       enum pipe_video_entrypoint entrypoint)
{
   struct si_screen *sscreen = (struct si_screen *)screen;

   if (sscreen->info.ip[AMD_IP_VPE].num_queues && entrypoint == PIPE_VIDEO_ENTRYPOINT_PROCESSING) {
      /* Todo:
       * Unable to confirm whether it is asking for an input or output type
       * Have to modify va frontend for solving this problem
       */
      /* VPE Supported input type */
      if ((format == PIPE_FORMAT_NV12) || (format == PIPE_FORMAT_NV21) || (format == PIPE_FORMAT_P010))
         return true;

      /* VPE Supported output type */
      if ((format == PIPE_FORMAT_A8R8G8B8_UNORM) || (format == PIPE_FORMAT_A8B8G8R8_UNORM) || (format == PIPE_FORMAT_R8G8B8A8_UNORM) ||
          (format == PIPE_FORMAT_B8G8R8A8_UNORM) || (format == PIPE_FORMAT_X8R8G8B8_UNORM) || (format == PIPE_FORMAT_X8B8G8R8_UNORM) ||
          (format == PIPE_FORMAT_R8G8B8X8_UNORM) || (format == PIPE_FORMAT_B8G8R8X8_UNORM) || (format == PIPE_FORMAT_A2R10G10B10_UNORM) ||
          (format == PIPE_FORMAT_A2B10G10R10_UNORM) || (format == PIPE_FORMAT_B10G10R10A2_UNORM) || (format == PIPE_FORMAT_R10G10B10A2_UNORM))
         return true;
   }

   /* HEVC 10 bit decoding should use P010 instead of NV12 if possible */
   if (profile == PIPE_VIDEO_PROFILE_HEVC_MAIN_10)
      return (format == PIPE_FORMAT_NV12) || (format == PIPE_FORMAT_P010) ||
             (format == PIPE_FORMAT_P016);

   /* Vp9 profile 2 supports 10 bit decoding using P016 */
   if (profile == PIPE_VIDEO_PROFILE_VP9_PROFILE2)
      return (format == PIPE_FORMAT_P010) || (format == PIPE_FORMAT_P016);

   if (profile == PIPE_VIDEO_PROFILE_AV1_MAIN && entrypoint == PIPE_VIDEO_ENTRYPOINT_BITSTREAM)
      return (format == PIPE_FORMAT_P010) || (format == PIPE_FORMAT_P016) ||
             (format == PIPE_FORMAT_NV12);

   /* JPEG supports YUV400 and YUV444 */
   if (profile == PIPE_VIDEO_PROFILE_JPEG_BASELINE) {
      switch (format) {
      case PIPE_FORMAT_NV12:
      case PIPE_FORMAT_YUYV:
      case PIPE_FORMAT_L8_UNORM:
      case PIPE_FORMAT_Y8_400_UNORM:
         return true;
      case PIPE_FORMAT_Y8_U8_V8_444_UNORM:
         if (sscreen->info.vcn_ip_version >= VCN_2_0_0)
            return true;
         else
            return false;
      case PIPE_FORMAT_R8G8B8A8_UNORM:
      case PIPE_FORMAT_A8R8G8B8_UNORM:
      case PIPE_FORMAT_R8_G8_B8_UNORM:
         if (sscreen->info.vcn_ip_version == VCN_4_0_3)
            return true;
         else
            return false;
      default:
         return false;
      }
   }

   if ((entrypoint == PIPE_VIDEO_ENTRYPOINT_ENCODE) &&
          (((profile == PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH) &&
          (sscreen->info.vcn_ip_version >= VCN_2_0_0)) ||
          ((profile == PIPE_VIDEO_PROFILE_AV1_MAIN) &&
           (sscreen->info.vcn_ip_version >= VCN_4_0_0 &&
            sscreen->info.vcn_ip_version != VCN_4_0_3))))
      return (format == PIPE_FORMAT_P010 || format == PIPE_FORMAT_NV12);


   /* we can only handle this one with UVD */
   if (profile != PIPE_VIDEO_PROFILE_UNKNOWN)
      return format == PIPE_FORMAT_NV12;

   return vl_video_buffer_is_format_supported(screen, format, profile, entrypoint);
}

static unsigned get_max_threads_per_block(struct si_screen *screen, enum pipe_shader_ir ir_type)
{
   if (ir_type == PIPE_SHADER_IR_NATIVE)
      return 256;

   /* LLVM only supports 1024 threads per block. */
   return 1024;
}

static int si_get_compute_param(struct pipe_screen *screen, enum pipe_shader_ir ir_type,
                                enum pipe_compute_cap param, void *ret)
{
   struct si_screen *sscreen = (struct si_screen *)screen;

   // TODO: select these params by asic
   switch (param) {
   case PIPE_COMPUTE_CAP_IR_TARGET: {
      const char *gpu, *triple;

      triple = "amdgcn-mesa-mesa3d";
      gpu = ac_get_llvm_processor_name(sscreen->info.family);
      if (ret) {
         sprintf(ret, "%s-%s", gpu, triple);
      }
      /* +2 for dash and terminating NIL byte */
      return (strlen(triple) + strlen(gpu) + 2) * sizeof(char);
   }
   case PIPE_COMPUTE_CAP_GRID_DIMENSION:
      if (ret) {
         uint64_t *grid_dimension = ret;
         grid_dimension[0] = 3;
      }
      return 1 * sizeof(uint64_t);

   case PIPE_COMPUTE_CAP_MAX_GRID_SIZE:
      if (ret) {
         uint64_t *grid_size = ret;
         /* Use this size, so that internal counters don't overflow 64 bits. */
         grid_size[0] = UINT32_MAX;
         grid_size[1] = UINT16_MAX;
         grid_size[2] = UINT16_MAX;
      }
      return 3 * sizeof(uint64_t);

   case PIPE_COMPUTE_CAP_MAX_BLOCK_SIZE:
      if (ret) {
         uint64_t *block_size = ret;
         unsigned threads_per_block = get_max_threads_per_block(sscreen, ir_type);
         block_size[0] = threads_per_block;
         block_size[1] = threads_per_block;
         block_size[2] = threads_per_block;
      }
      return 3 * sizeof(uint64_t);

   case PIPE_COMPUTE_CAP_MAX_THREADS_PER_BLOCK:
      if (ret) {
         uint64_t *max_threads_per_block = ret;
         *max_threads_per_block = get_max_threads_per_block(sscreen, ir_type);
      }
      return sizeof(uint64_t);
   case PIPE_COMPUTE_CAP_ADDRESS_BITS:
      if (ret) {
         uint32_t *address_bits = ret;
         address_bits[0] = 64;
      }
      return 1 * sizeof(uint32_t);

   case PIPE_COMPUTE_CAP_MAX_GLOBAL_SIZE:
      if (ret) {
         uint64_t *max_global_size = ret;
         uint64_t max_mem_alloc_size;

         si_get_compute_param(screen, ir_type, PIPE_COMPUTE_CAP_MAX_MEM_ALLOC_SIZE,
                              &max_mem_alloc_size);

         /* In OpenCL, the MAX_MEM_ALLOC_SIZE must be at least
          * 1/4 of the MAX_GLOBAL_SIZE.  Since the
          * MAX_MEM_ALLOC_SIZE is fixed for older kernels,
          * make sure we never report more than
          * 4 * MAX_MEM_ALLOC_SIZE.
          */
         *max_global_size =
            MIN2(4 * max_mem_alloc_size, sscreen->info.max_heap_size_kb * 1024ull);
      }
      return sizeof(uint64_t);

   case PIPE_COMPUTE_CAP_MAX_LOCAL_SIZE:
      if (ret) {
         uint64_t *max_local_size = ret;
         /* Value reported by the closed source driver. */
         if (sscreen->info.gfx_level == GFX6)
            *max_local_size = 32 * 1024;
         else
            *max_local_size = 64 * 1024;
      }
      return sizeof(uint64_t);

   case PIPE_COMPUTE_CAP_MAX_INPUT_SIZE:
      if (ret) {
         uint64_t *max_input_size = ret;
         /* Value reported by the closed source driver. */
         *max_input_size = 1024;
      }
      return sizeof(uint64_t);

   case PIPE_COMPUTE_CAP_MAX_MEM_ALLOC_SIZE:
      if (ret) {
         uint64_t *max_mem_alloc_size = ret;

         /* Return 1/4 of the heap size as the maximum because the max size is not practically
          * allocatable.
          */
         *max_mem_alloc_size = (sscreen->info.max_heap_size_kb / 4) * 1024ull;
      }
      return sizeof(uint64_t);

   case PIPE_COMPUTE_CAP_MAX_CLOCK_FREQUENCY:
      if (ret) {
         uint32_t *max_clock_frequency = ret;
         *max_clock_frequency = sscreen->info.max_gpu_freq_mhz;
      }
      return sizeof(uint32_t);

   case PIPE_COMPUTE_CAP_MAX_COMPUTE_UNITS:
      if (ret) {
         uint32_t *max_compute_units = ret;
         *max_compute_units = sscreen->info.num_cu;
      }
      return sizeof(uint32_t);

   case PIPE_COMPUTE_CAP_IMAGES_SUPPORTED:
      if (ret) {
         uint32_t *images_supported = ret;
         *images_supported = 0;
      }
      return sizeof(uint32_t);
   case PIPE_COMPUTE_CAP_MAX_PRIVATE_SIZE:
      break; /* unused */
   case PIPE_COMPUTE_CAP_MAX_SUBGROUPS: {
      if (ret) {
         uint32_t *max_subgroups = ret;
         unsigned threads = get_max_threads_per_block(sscreen, ir_type);
         unsigned subgroup_size;

         if (sscreen->debug_flags & DBG(W64_CS) || sscreen->info.gfx_level < GFX10)
            subgroup_size = 64;
         else
            subgroup_size = 32;

         *max_subgroups = threads / subgroup_size;
      }
      return sizeof(uint32_t);
   }
   case PIPE_COMPUTE_CAP_SUBGROUP_SIZES:
      if (ret) {
         uint32_t *subgroup_size = ret;
         if (sscreen->debug_flags & DBG(W32_CS))
            *subgroup_size = 32;
         else if (sscreen->debug_flags & DBG(W64_CS))
            *subgroup_size = 64;
         else
            *subgroup_size = sscreen->info.gfx_level < GFX10 ? 64 : 64 | 32;
      }
      return sizeof(uint32_t);
   case PIPE_COMPUTE_CAP_MAX_VARIABLE_THREADS_PER_BLOCK:
      if (ret) {
         uint64_t *max_variable_threads_per_block = ret;
         if (ir_type == PIPE_SHADER_IR_NATIVE)
            *max_variable_threads_per_block = 0;
         else
            *max_variable_threads_per_block = SI_MAX_VARIABLE_THREADS_PER_BLOCK;
      }
      return sizeof(uint64_t);
   }

   fprintf(stderr, "unknown PIPE_COMPUTE_CAP %d\n", param);
   return 0;
}

static uint64_t si_get_timestamp(struct pipe_screen *screen)
{
   struct si_screen *sscreen = (struct si_screen *)screen;

   return 1000000 * sscreen->ws->query_value(sscreen->ws, RADEON_TIMESTAMP) /
          sscreen->info.clock_crystal_freq;
}

static void si_query_memory_info(struct pipe_screen *screen, struct pipe_memory_info *info)
{
   struct si_screen *sscreen = (struct si_screen *)screen;
   struct radeon_winsys *ws = sscreen->ws;
   unsigned vram_usage, gtt_usage;

   info->total_device_memory = sscreen->info.vram_size_kb;
   info->total_staging_memory = sscreen->info.gart_size_kb;

   /* The real TTM memory usage is somewhat random, because:
    *
    * 1) TTM delays freeing memory, because it can only free it after
    *    fences expire.
    *
    * 2) The memory usage can be really low if big VRAM evictions are
    *    taking place, but the real usage is well above the size of VRAM.
    *
    * Instead, return statistics of this process.
    */
   vram_usage = ws->query_value(ws, RADEON_VRAM_USAGE) / 1024;
   gtt_usage = ws->query_value(ws, RADEON_GTT_USAGE) / 1024;

   info->avail_device_memory =
      vram_usage <= info->total_device_memory ? info->total_device_memory - vram_usage : 0;
   info->avail_staging_memory =
      gtt_usage <= info->total_staging_memory ? info->total_staging_memory - gtt_usage : 0;

   info->device_memory_evicted = ws->query_value(ws, RADEON_NUM_BYTES_MOVED) / 1024;

   if (sscreen->info.is_amdgpu)
      info->nr_device_memory_evictions = ws->query_value(ws, RADEON_NUM_EVICTIONS);
   else
      /* Just return the number of evicted 64KB pages. */
      info->nr_device_memory_evictions = info->device_memory_evicted / 64;
}

static struct disk_cache *si_get_disk_shader_cache(struct pipe_screen *pscreen)
{
   struct si_screen *sscreen = (struct si_screen *)pscreen;

   return sscreen->disk_shader_cache;
}

static void si_init_renderer_string(struct si_screen *sscreen)
{
   char first_name[256], second_name[32] = {}, kernel_version[128] = {};
   struct utsname uname_data;

   snprintf(first_name, sizeof(first_name), "%s",
            sscreen->info.marketing_name ? sscreen->info.marketing_name : sscreen->info.name);
   snprintf(second_name, sizeof(second_name), "%s, ", sscreen->info.lowercase_name);

   if (uname(&uname_data) == 0)
      snprintf(kernel_version, sizeof(kernel_version), ", %s", uname_data.release);

   const char *compiler_name =
#if LLVM_AVAILABLE
      !sscreen->use_aco ? "LLVM " MESA_LLVM_VERSION_STRING :
#endif
      "ACO";

   snprintf(sscreen->renderer_string, sizeof(sscreen->renderer_string),
            "%s (radeonsi, %s%s, DRM %i.%i%s)", first_name, second_name, compiler_name,
            sscreen->info.drm_major, sscreen->info.drm_minor, kernel_version);
}

static int si_get_screen_fd(struct pipe_screen *screen)
{
   struct si_screen *sscreen = (struct si_screen *)screen;
   struct radeon_winsys *ws = sscreen->ws;

   return ws->get_fd(ws);
}

void si_init_screen_get_functions(struct si_screen *sscreen)
{
   sscreen->b.get_name = si_get_name;
   sscreen->b.get_vendor = si_get_vendor;
   sscreen->b.get_device_vendor = si_get_device_vendor;
   sscreen->b.get_screen_fd = si_get_screen_fd;
   sscreen->b.get_param = si_get_param;
   sscreen->b.get_paramf = si_get_paramf;
   sscreen->b.get_compute_param = si_get_compute_param;
   sscreen->b.get_timestamp = si_get_timestamp;
   sscreen->b.get_shader_param = si_get_shader_param;
   sscreen->b.get_compiler_options = si_get_compiler_options;
   sscreen->b.get_device_uuid = si_get_device_uuid;
   sscreen->b.get_driver_uuid = si_get_driver_uuid;
   sscreen->b.query_memory_info = si_query_memory_info;
   sscreen->b.get_disk_shader_cache = si_get_disk_shader_cache;

   if (sscreen->info.ip[AMD_IP_UVD].num_queues ||
       ((sscreen->info.vcn_ip_version >= VCN_4_0_0) ?
	 sscreen->info.ip[AMD_IP_VCN_UNIFIED].num_queues : sscreen->info.ip[AMD_IP_VCN_DEC].num_queues) ||
       sscreen->info.ip[AMD_IP_VCN_JPEG].num_queues || sscreen->info.ip[AMD_IP_VCE].num_queues ||
       sscreen->info.ip[AMD_IP_UVD_ENC].num_queues || sscreen->info.ip[AMD_IP_VCN_ENC].num_queues ||
       sscreen->info.ip[AMD_IP_VPE].num_queues) {
      sscreen->b.get_video_param = si_get_video_param;
      sscreen->b.is_video_format_supported = si_vid_is_format_supported;
   } else {
      sscreen->b.get_video_param = si_get_video_param_no_video_hw;
      sscreen->b.is_video_format_supported = vl_video_buffer_is_format_supported;
   }

   si_init_renderer_string(sscreen);

   bool use_fma32 =
      sscreen->info.gfx_level >= GFX10_3 ||
      (sscreen->info.family >= CHIP_GFX940 && !sscreen->info.has_graphics) ||
      /* fma32 is too slow for gpu < gfx9, so apply the option only for gpu >= gfx9 */
      (sscreen->info.gfx_level >= GFX9 && sscreen->options.force_use_fma32);

   const struct nir_shader_compiler_options nir_options = {
      .vertex_id_zero_based = true,
      .lower_scmp = true,
      .lower_flrp16 = true,
      .lower_flrp32 = true,
      .lower_flrp64 = true,
      .lower_fdiv = true,
      .lower_bitfield_insert = true,
      .lower_bitfield_extract = true,
      /*        |---------------------------------- Performance & Availability --------------------------------|
       *        |MAD/MAC/MADAK/MADMK|MAD_LEGACY|MAC_LEGACY|    FMA     |FMAC/FMAAK/FMAMK|FMA_LEGACY|PK_FMA_F16,|Best choice
       * Arch   |    F32,F16,F64    | F32,F16  | F32,F16  |F32,F16,F64 |    F32,F16     | F32,F16  |PK_FMAC_F16|F16,F32,F64
       * ------------------------------------------------------------------------------------------------------------------
       * gfx6,7 |     1 , - , -     |  1 , -   |  1 , -   |1/4, - ,1/16|     - , -      |  - , -   |   - , -   | - ,MAD,FMA
       * gfx8   |     1 , 1 , -     |  1 , -   |  - , -   |1/4, 1 ,1/16|     - , -      |  - , -   |   - , -   |MAD,MAD,FMA
       * gfx9   |     1 ,1|0, -     |  1 , -   |  - , -   | 1 , 1 ,1/16|    0|1, -      |  - , 1   |   2 , -   |FMA,MAD,FMA
       * gfx10  |     1 , - , -     |  1 , -   |  1 , -   | 1 , 1 ,1/16|     1 , 1      |  - , -   |   2 , 2   |FMA,MAD,FMA
       * gfx10.3|     - , - , -     |  - , -   |  - , -   | 1 , 1 ,1/16|     1 , 1      |  1 , -   |   2 , 2   |  all FMA
       *
       * Tahiti, Hawaii, Carrizo, Vega20: FMA_F32 is full rate, FMA_F64 is 1/4
       * gfx9 supports MAD_F16 only on Vega10, Raven, Raven2, Renoir.
       * gfx9 supports FMAC_F32 only on Vega20, but doesn't support FMAAK and FMAMK.
       *
       * gfx8 prefers MAD for F16 because of MAC/MADAK/MADMK.
       * gfx9 and newer prefer FMA for F16 because of the packed instruction.
       * gfx10 and older prefer MAD for F32 because of the legacy instruction.
       */
      .lower_ffma16 = sscreen->info.gfx_level < GFX9,
      .lower_ffma32 = !use_fma32,
      .lower_ffma64 = false,
      .fuse_ffma16 = sscreen->info.gfx_level >= GFX9,
      .fuse_ffma32 = use_fma32,
      .fuse_ffma64 = true,
      .lower_fmod = true,
      .lower_fpow = true,
      .lower_ineg = true,
      .lower_pack_snorm_4x8 = true,
      .lower_pack_unorm_4x8 = true,
      .lower_pack_half_2x16 = true,
      .lower_pack_64_2x32 = true,
      .lower_pack_64_4x16 = true,
      .lower_pack_32_2x16 = true,
      .lower_unpack_snorm_2x16 = true,
      .lower_unpack_snorm_4x8 = true,
      .lower_unpack_unorm_2x16 = true,
      .lower_unpack_unorm_4x8 = true,
      .lower_unpack_half_2x16 = true,
      .lower_extract_byte = true,
      .lower_extract_word = true,
      .lower_insert_byte = true,
      .lower_insert_word = true,
      .lower_hadd = true,
      .lower_hadd64 = true,
      .lower_fisnormal = true,
      .lower_to_scalar = true,
      .lower_to_scalar_filter = sscreen->info.has_packed_math_16bit ?
                                   si_alu_to_scalar_packed_math_filter : NULL,
      .has_sdot_4x8 = sscreen->info.has_accelerated_dot_product,
      .has_sudot_4x8 = sscreen->info.has_accelerated_dot_product && sscreen->info.gfx_level >= GFX11,
      .has_udot_4x8 = sscreen->info.has_accelerated_dot_product,
      .has_sdot_4x8_sat = sscreen->info.has_accelerated_dot_product,
      .has_sudot_4x8_sat = sscreen->info.has_accelerated_dot_product && sscreen->info.gfx_level >= GFX11,
      .has_udot_4x8_sat = sscreen->info.has_accelerated_dot_product,
      .has_dot_2x16 = sscreen->info.has_accelerated_dot_product && sscreen->info.gfx_level < GFX11,
      .has_bfe = true,
      .has_bfm = true,
      .has_bitfield_select = true,
      .optimize_sample_mask_in = true,
      .max_unroll_iterations = 128,
      .max_unroll_iterations_aggressive = 128,
      .use_interpolated_input_intrinsics = true,
      .lower_uniforms_to_ubo = true,
      .support_16bit_alu = sscreen->info.gfx_level >= GFX8,
      .vectorize_vec2_16bit = sscreen->info.has_packed_math_16bit,
      .pack_varying_options =
         nir_pack_varying_interp_mode_none |
         nir_pack_varying_interp_mode_smooth |
         nir_pack_varying_interp_mode_noperspective |
         nir_pack_varying_interp_loc_center |
         nir_pack_varying_interp_loc_sample |
         nir_pack_varying_interp_loc_centroid,
      .lower_io_variables = true,
      /* HW supports indirect indexing for: | Enabled in driver
       * -------------------------------------------------------
       * TCS inputs                         | Yes
       * TES inputs                         | Yes
       * GS inputs                          | No
       * -------------------------------------------------------
       * VS outputs before TCS              | No
       * TCS outputs                        | Yes
       * VS/TES outputs before GS           | No
       */
      .support_indirect_inputs = BITFIELD_BIT(MESA_SHADER_TESS_CTRL) |
                                 BITFIELD_BIT(MESA_SHADER_TESS_EVAL),
      .support_indirect_outputs = BITFIELD_BIT(MESA_SHADER_TESS_CTRL),
      .lower_int64_options =
         nir_lower_imul64 | nir_lower_imul_high64 | nir_lower_imul_2x32_64 |
         nir_lower_divmod64 | nir_lower_minmax64 | nir_lower_iabs64 |
         nir_lower_iadd_sat64 | nir_lower_conv64,

      /* For OpenGL, rounding mode is undefined. We want fast packing with v_cvt_pkrtz_f16,
       * but if we use it, all f32->f16 conversions have to round towards zero,
       * because both scalar and vec2 down-conversions have to round equally.
       *
       * For OpenCL, rounding mode is explicit. This will only lower f2f16 to f2f16_rtz
       * when execution mode is rtz instead of rtne.
       */
      .force_f2f16_rtz = true,
      .lower_layer_fs_input_to_sysval = true,
   };
   *sscreen->nir_options = nir_options;
}
