/*
 * Copyright © 2017 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * @file crocus_screen.c
 *
 * Screen related driver hooks and capability lists.
 *
 * A program may use multiple rendering contexts (crocus_context), but
 * they all share a common screen (crocus_screen).  Global driver state
 * can be stored in the screen; it may be accessed by multiple threads.
 */

#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "pipe/p_defines.h"
#include "pipe/p_state.h"
#include "pipe/p_context.h"
#include "pipe/p_screen.h"
#include "util/u_debug.h"
#include "util/u_inlines.h"
#include "util/format/u_format.h"
#include "util/u_transfer_helper.h"
#include "util/u_upload_mgr.h"
#include "util/ralloc.h"
#include "util/xmlconfig.h"
#include "drm-uapi/i915_drm.h"
#include "crocus_context.h"
#include "crocus_defines.h"
#include "crocus_fence.h"
#include "crocus_pipe.h"
#include "crocus_resource.h"
#include "crocus_screen.h"
#include "intel/compiler/brw_compiler.h"
#include "intel/common/intel_gem.h"
#include "intel/common/intel_l3_config.h"
#include "intel/common/intel_uuid.h"
#include "crocus_monitor.h"

#define genX_call(devinfo, func, ...)                   \
   switch ((devinfo)->verx10) {                         \
   case 80:                                             \
      gfx8_##func(__VA_ARGS__);                         \
      break;                                            \
   case 75:                                             \
      gfx75_##func(__VA_ARGS__);                        \
      break;                                            \
   case 70:                                             \
      gfx7_##func(__VA_ARGS__);                         \
      break;                                            \
   case 60:                                             \
      gfx6_##func(__VA_ARGS__);                         \
      break;                                            \
   case 50:                                             \
      gfx5_##func(__VA_ARGS__);                         \
      break;                                            \
   case 45:                                             \
      gfx45_##func(__VA_ARGS__);                        \
      break;                                            \
   case 40:                                             \
      gfx4_##func(__VA_ARGS__);                         \
      break;                                            \
   default:                                             \
      unreachable("Unknown hardware generation");       \
   }

static const char *
crocus_get_vendor(struct pipe_screen *pscreen)
{
   return "Intel";
}

static const char *
crocus_get_device_vendor(struct pipe_screen *pscreen)
{
   return "Intel";
}

static void
crocus_get_device_uuid(struct pipe_screen *pscreen, char *uuid)
{
   struct crocus_screen *screen = (struct crocus_screen *)pscreen;

   intel_uuid_compute_device_id((uint8_t *)uuid, &screen->devinfo, PIPE_UUID_SIZE);
}

static void
crocus_get_driver_uuid(struct pipe_screen *pscreen, char *uuid)
{
   struct crocus_screen *screen = (struct crocus_screen *)pscreen;
   const struct intel_device_info *devinfo = &screen->devinfo;

   intel_uuid_compute_driver_id((uint8_t *)uuid, devinfo, PIPE_UUID_SIZE);
}

static const char *
crocus_get_name(struct pipe_screen *pscreen)
{
   struct crocus_screen *screen = (struct crocus_screen *)pscreen;
   const struct intel_device_info *devinfo = &screen->devinfo;
   static char buf[128];

   snprintf(buf, sizeof(buf), "Mesa %s", devinfo->name);
   return buf;
}

static uint64_t
get_aperture_size(int fd)
{
   struct drm_i915_gem_get_aperture aperture = {};
   intel_ioctl(fd, DRM_IOCTL_I915_GEM_GET_APERTURE, &aperture);
   return aperture.aper_size;
}

static int
crocus_get_param(struct pipe_screen *pscreen, enum pipe_cap param)
{
   struct crocus_screen *screen = (struct crocus_screen *)pscreen;
   const struct intel_device_info *devinfo = &screen->devinfo;

   switch (param) {
   case PIPE_CAP_NPOT_TEXTURES:
   case PIPE_CAP_ANISOTROPIC_FILTER:
   case PIPE_CAP_OCCLUSION_QUERY:
   case PIPE_CAP_TEXTURE_SWIZZLE:
   case PIPE_CAP_TEXTURE_MIRROR_CLAMP_TO_EDGE:
   case PIPE_CAP_BLEND_EQUATION_SEPARATE:
   case PIPE_CAP_FRAGMENT_SHADER_TEXTURE_LOD:
   case PIPE_CAP_FRAGMENT_SHADER_DERIVATIVES:
   case PIPE_CAP_PRIMITIVE_RESTART:
   case PIPE_CAP_PRIMITIVE_RESTART_FIXED_INDEX:
   case PIPE_CAP_INDEP_BLEND_ENABLE:
   case PIPE_CAP_FS_COORD_ORIGIN_UPPER_LEFT:
   case PIPE_CAP_FS_COORD_PIXEL_CENTER_INTEGER:
   case PIPE_CAP_DEPTH_CLIP_DISABLE:
   case PIPE_CAP_VS_INSTANCEID:
   case PIPE_CAP_VERTEX_ELEMENT_INSTANCE_DIVISOR:
   case PIPE_CAP_SEAMLESS_CUBE_MAP:
   case PIPE_CAP_SEAMLESS_CUBE_MAP_PER_TEXTURE:
   case PIPE_CAP_CONDITIONAL_RENDER:
   case PIPE_CAP_TEXTURE_BARRIER:
   case PIPE_CAP_VERTEX_COLOR_UNCLAMPED:
   case PIPE_CAP_START_INSTANCE:
   case PIPE_CAP_FORCE_PERSAMPLE_INTERP:
   case PIPE_CAP_MIXED_FRAMEBUFFER_SIZES:
   case PIPE_CAP_VS_LAYER_VIEWPORT:
   case PIPE_CAP_TES_LAYER_VIEWPORT:
   case PIPE_CAP_ACCELERATED:
   case PIPE_CAP_UMA:
   case PIPE_CAP_CLIP_HALFZ:
   case PIPE_CAP_TGSI_TEXCOORD:
   case PIPE_CAP_DEVICE_RESET_STATUS_QUERY:
   case PIPE_CAP_COPY_BETWEEN_COMPRESSED_AND_PLAIN_FORMATS:
   case PIPE_CAP_SIGNED_VERTEX_BUFFER_OFFSET:
   case PIPE_CAP_TEXTURE_FLOAT_LINEAR:
   case PIPE_CAP_TEXTURE_HALF_FLOAT_LINEAR:
   case PIPE_CAP_POLYGON_OFFSET_CLAMP:
   case PIPE_CAP_TGSI_TEX_TXF_LZ:
   case PIPE_CAP_MULTISAMPLE_Z_RESOLVE:
   case PIPE_CAP_SHADER_GROUP_VOTE:
   case PIPE_CAP_VS_WINDOW_SPACE_POSITION:
   case PIPE_CAP_TEXTURE_GATHER_SM5:
   case PIPE_CAP_SHADER_ARRAY_COMPONENTS:
   case PIPE_CAP_GLSL_TESS_LEVELS_AS_INPUTS:
   case PIPE_CAP_NIR_COMPACT_ARRAYS:
   case PIPE_CAP_FS_POSITION_IS_SYSVAL:
   case PIPE_CAP_FS_FACE_IS_INTEGER_SYSVAL:
   case PIPE_CAP_INVALIDATE_BUFFER:
   case PIPE_CAP_SURFACE_REINTERPRET_BLOCKS:
   case PIPE_CAP_FENCE_SIGNAL:
   case PIPE_CAP_DEMOTE_TO_HELPER_INVOCATION:
   case PIPE_CAP_GL_CLAMP:
   case PIPE_CAP_LEGACY_MATH_RULES:
   case PIPE_CAP_NATIVE_FENCE_FD:
      return true;
   case PIPE_CAP_INT64:
   case PIPE_CAP_SHADER_BALLOT:
   case PIPE_CAP_PACKED_UNIFORMS:
      return devinfo->ver == 8;
   case PIPE_CAP_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION:
      return devinfo->ver <= 5;
   case PIPE_CAP_TEXTURE_QUERY_LOD:
   case PIPE_CAP_QUERY_TIME_ELAPSED:
      return devinfo->ver >= 5;
   case PIPE_CAP_DRAW_INDIRECT:
   case PIPE_CAP_MULTI_DRAW_INDIRECT:
   case PIPE_CAP_MULTI_DRAW_INDIRECT_PARAMS:
   case PIPE_CAP_FRAMEBUFFER_NO_ATTACHMENT:
   case PIPE_CAP_FS_FINE_DERIVATIVE:
   case PIPE_CAP_STREAM_OUTPUT_INTERLEAVE_BUFFERS:
   case PIPE_CAP_SHADER_CLOCK:
   case PIPE_CAP_TEXTURE_QUERY_SAMPLES:
   case PIPE_CAP_COMPUTE:
   case PIPE_CAP_SAMPLER_VIEW_TARGET:
   case PIPE_CAP_SHADER_SAMPLES_IDENTICAL:
   case PIPE_CAP_SHADER_PACK_HALF_FLOAT:
   case PIPE_CAP_GL_SPIRV:
   case PIPE_CAP_GL_SPIRV_VARIABLE_POINTERS:
   case PIPE_CAP_COMPUTE_SHADER_DERIVATIVES:
   case PIPE_CAP_DOUBLES:
   case PIPE_CAP_MEMOBJ:
   case PIPE_CAP_IMAGE_STORE_FORMATTED:
   case PIPE_CAP_ALPHA_TO_COVERAGE_DITHER_CONTROL:
      return devinfo->ver >= 7;
   case PIPE_CAP_QUERY_BUFFER_OBJECT:
   case PIPE_CAP_ROBUST_BUFFER_ACCESS_BEHAVIOR:
      return devinfo->verx10 >= 75;
   case PIPE_CAP_CULL_DISTANCE:
   case PIPE_CAP_QUERY_PIPELINE_STATISTICS_SINGLE:
   case PIPE_CAP_STREAM_OUTPUT_PAUSE_RESUME:
   case PIPE_CAP_SAMPLE_SHADING:
   case PIPE_CAP_CUBE_MAP_ARRAY:
   case PIPE_CAP_QUERY_SO_OVERFLOW:
   case PIPE_CAP_TEXTURE_MULTISAMPLE:
   case PIPE_CAP_CONDITIONAL_RENDER_INVERTED:
   case PIPE_CAP_QUERY_TIMESTAMP:
   case PIPE_CAP_TEXTURE_BUFFER_OBJECTS:
   case PIPE_CAP_INDEP_BLEND_FUNC:
   case PIPE_CAP_TEXTURE_SHADOW_LOD:
   case PIPE_CAP_LOAD_CONSTBUF:
   case PIPE_CAP_DRAW_PARAMETERS:
   case PIPE_CAP_CLEAR_SCISSORED:
      return devinfo->ver >= 6;
   case PIPE_CAP_FBFETCH:
      return devinfo->verx10 >= 45 ? BRW_MAX_DRAW_BUFFERS : 0;
   case PIPE_CAP_MAX_DUAL_SOURCE_RENDER_TARGETS:
      /* in theory CL (965gm) can do this */
      return devinfo->verx10 >= 45 ? 1 : 0;
   case PIPE_CAP_MAX_RENDER_TARGETS:
      return BRW_MAX_DRAW_BUFFERS;
   case PIPE_CAP_MAX_TEXTURE_2D_SIZE:
      if (devinfo->ver >= 7)
         return 16384;
      else
         return 8192;
   case PIPE_CAP_MAX_TEXTURE_CUBE_LEVELS:
      if (devinfo->ver >= 7)
         return CROCUS_MAX_MIPLEVELS; /* 16384x16384 */
      else
         return CROCUS_MAX_MIPLEVELS - 1; /* 8192x8192 */
   case PIPE_CAP_MAX_TEXTURE_3D_LEVELS:
      return 12; /* 2048x2048 */
   case PIPE_CAP_MAX_STREAM_OUTPUT_BUFFERS:
      return (devinfo->ver >= 6) ? 4 : 0;
   case PIPE_CAP_MAX_TEXTURE_ARRAY_LAYERS:
      return devinfo->ver >= 7 ? 2048 : 512;
   case PIPE_CAP_MAX_STREAM_OUTPUT_SEPARATE_COMPONENTS:
      return BRW_MAX_SOL_BINDINGS / CROCUS_MAX_SOL_BUFFERS;
   case PIPE_CAP_MAX_STREAM_OUTPUT_INTERLEAVED_COMPONENTS:
      return BRW_MAX_SOL_BINDINGS;
   case PIPE_CAP_GLSL_FEATURE_LEVEL_COMPATIBILITY:
   case PIPE_CAP_GLSL_FEATURE_LEVEL: {
      if (devinfo->verx10 >= 75)
         return 460;
      else if (devinfo->ver >= 7)
         return 420;
      else if (devinfo->ver >= 6)
         return 330;
      return 140;
   }
   case PIPE_CAP_CLIP_PLANES:
      if (devinfo->verx10 < 45)
         return 6;
      else
         return 1; // defaults to MAX (8)
   case PIPE_CAP_CONSTANT_BUFFER_OFFSET_ALIGNMENT:
      /* 3DSTATE_CONSTANT_XS requires the start of UBOs to be 32B aligned */
      return 32;
   case PIPE_CAP_MIN_MAP_BUFFER_ALIGNMENT:
      return CROCUS_MAP_BUFFER_ALIGNMENT;
   case PIPE_CAP_SHADER_BUFFER_OFFSET_ALIGNMENT:
      return devinfo->ver >= 7 ? 4 : 0;
   case PIPE_CAP_MAX_SHADER_BUFFER_SIZE_UINT:
      return devinfo->ver >= 7 ? (1 << 27) : 0;
   case PIPE_CAP_TEXTURE_BUFFER_OFFSET_ALIGNMENT:
      return 16; // XXX: u_screen says 256 is the minimum value...
   case PIPE_CAP_TEXTURE_TRANSFER_MODES:
      return PIPE_TEXTURE_TRANSFER_BLIT;
   case PIPE_CAP_MAX_TEXEL_BUFFER_ELEMENTS_UINT:
      return CROCUS_MAX_TEXTURE_BUFFER_SIZE;
   case PIPE_CAP_MAX_VIEWPORTS:
      return devinfo->ver >= 6 ? 16 : 1;
   case PIPE_CAP_MAX_GEOMETRY_OUTPUT_VERTICES:
      return devinfo->ver >= 6 ? 256 : 0;
   case PIPE_CAP_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS:
      return devinfo->ver >= 6 ? 1024 : 0;
   case PIPE_CAP_MAX_GS_INVOCATIONS:
      return devinfo->ver >= 7 ? 32 : 1;
   case PIPE_CAP_MAX_TEXTURE_GATHER_COMPONENTS:
      if (devinfo->ver >= 7)
         return 4;
      else if (devinfo->ver == 6)
         return 1;
      else
         return 0;
   case PIPE_CAP_MIN_TEXTURE_GATHER_OFFSET:
      if (devinfo->ver >= 7)
         return -32;
      else if (devinfo->ver == 6)
         return -8;
      else
         return 0;
   case PIPE_CAP_MAX_TEXTURE_GATHER_OFFSET:
      if (devinfo->ver >= 7)
         return 31;
      else if (devinfo->ver == 6)
         return 7;
      else
         return 0;
   case PIPE_CAP_MAX_VERTEX_STREAMS:
      return devinfo->ver >= 7 ? 4 : 1;
   case PIPE_CAP_VENDOR_ID:
      return 0x8086;
   case PIPE_CAP_DEVICE_ID:
      return screen->pci_id;
   case PIPE_CAP_VIDEO_MEMORY: {
      /* Once a batch uses more than 75% of the maximum mappable size, we
       * assume that there's some fragmentation, and we start doing extra
       * flushing, etc.  That's the big cliff apps will care about.
       */
      const unsigned gpu_mappable_megabytes =
         (screen->aperture_threshold) / (1024 * 1024);

      const long system_memory_pages = sysconf(_SC_PHYS_PAGES);
      const long system_page_size = sysconf(_SC_PAGE_SIZE);

      if (system_memory_pages <= 0 || system_page_size <= 0)
         return -1;

      const uint64_t system_memory_bytes =
         (uint64_t) system_memory_pages * (uint64_t) system_page_size;

      const unsigned system_memory_megabytes =
         (unsigned) (system_memory_bytes / (1024 * 1024));

      return MIN2(system_memory_megabytes, gpu_mappable_megabytes);
   }
   case PIPE_CAP_MAX_SHADER_PATCH_VARYINGS:
   case PIPE_CAP_MAX_VARYINGS:
      return (screen->devinfo.ver >= 6) ? 32 : 16;
   case PIPE_CAP_RESOURCE_FROM_USER_MEMORY:
      /* AMD_pinned_memory assumes the flexibility of using client memory
       * for any buffer (incl. vertex buffers) which rules out the prospect
       * of using snooped buffers, as using snooped buffers without
       * cogniscience is likely to be detrimental to performance and require
       * extensive checking in the driver for correctness, e.g. to prevent
       * illegal snoop <-> snoop transfers.
       */
      return devinfo->has_llc;
   case PIPE_CAP_THROTTLE:
      return screen->driconf.disable_throttling ? 0 : 1;

   case PIPE_CAP_CONTEXT_PRIORITY_MASK:
      return PIPE_CONTEXT_PRIORITY_LOW |
             PIPE_CONTEXT_PRIORITY_MEDIUM |
             PIPE_CONTEXT_PRIORITY_HIGH;

   case PIPE_CAP_FRONTEND_NOOP:
      return true;
      // XXX: don't hardcode 00:00:02.0 PCI here
   case PIPE_CAP_PCI_GROUP:
      return 0;
   case PIPE_CAP_PCI_BUS:
      return 0;
   case PIPE_CAP_PCI_DEVICE:
      return 2;
   case PIPE_CAP_PCI_FUNCTION:
      return 0;

   case PIPE_CAP_HARDWARE_GL_SELECT:
      return 0;

   case PIPE_CAP_TIMER_RESOLUTION:
      return DIV_ROUND_UP(1000000000ull, devinfo->timestamp_frequency);

   default:
      return u_pipe_screen_get_param_defaults(pscreen, param);
   }
   return 0;
}

static float
crocus_get_paramf(struct pipe_screen *pscreen, enum pipe_capf param)
{
   struct crocus_screen *screen = (struct crocus_screen *)pscreen;
   const struct intel_device_info *devinfo = &screen->devinfo;

   switch (param) {
   case PIPE_CAPF_MIN_LINE_WIDTH:
   case PIPE_CAPF_MIN_LINE_WIDTH_AA:
   case PIPE_CAPF_MIN_POINT_SIZE:
   case PIPE_CAPF_MIN_POINT_SIZE_AA:
      return 1;

   case PIPE_CAPF_POINT_SIZE_GRANULARITY:
   case PIPE_CAPF_LINE_WIDTH_GRANULARITY:
      return 0.1;

   case PIPE_CAPF_MAX_LINE_WIDTH:
   case PIPE_CAPF_MAX_LINE_WIDTH_AA:
      if (devinfo->ver >= 6)
         return 7.375f;
      else
         return 7.0f;

   case PIPE_CAPF_MAX_POINT_SIZE:
   case PIPE_CAPF_MAX_POINT_SIZE_AA:
      return 255.0f;

   case PIPE_CAPF_MAX_TEXTURE_ANISOTROPY:
      return 16.0f;
   case PIPE_CAPF_MAX_TEXTURE_LOD_BIAS:
      return 15.0f;
   case PIPE_CAPF_MIN_CONSERVATIVE_RASTER_DILATE:
   case PIPE_CAPF_MAX_CONSERVATIVE_RASTER_DILATE:
   case PIPE_CAPF_CONSERVATIVE_RASTER_DILATE_GRANULARITY:
      return 0.0f;
   default:
      unreachable("unknown param");
   }
}

static int
crocus_get_shader_param(struct pipe_screen *pscreen,
                        enum pipe_shader_type p_stage,
                        enum pipe_shader_cap param)
{
   gl_shader_stage stage = stage_from_pipe(p_stage);
   struct crocus_screen *screen = (struct crocus_screen *)pscreen;
   const struct intel_device_info *devinfo = &screen->devinfo;

   if (p_stage == PIPE_SHADER_MESH ||
       p_stage == PIPE_SHADER_TASK)
      return 0;

   if (devinfo->ver < 6 &&
       p_stage != PIPE_SHADER_VERTEX &&
       p_stage != PIPE_SHADER_FRAGMENT)
      return 0;

   if (devinfo->ver == 6 &&
       p_stage != PIPE_SHADER_VERTEX &&
       p_stage != PIPE_SHADER_FRAGMENT &&
       p_stage != PIPE_SHADER_GEOMETRY)
      return 0;

   /* this is probably not totally correct.. but it's a start: */
   switch (param) {
   case PIPE_SHADER_CAP_MAX_INSTRUCTIONS:
      return stage == MESA_SHADER_FRAGMENT ? 1024 : 16384;
   case PIPE_SHADER_CAP_MAX_ALU_INSTRUCTIONS:
   case PIPE_SHADER_CAP_MAX_TEX_INSTRUCTIONS:
   case PIPE_SHADER_CAP_MAX_TEX_INDIRECTIONS:
      return stage == MESA_SHADER_FRAGMENT ? 1024 : 0;

   case PIPE_SHADER_CAP_MAX_CONTROL_FLOW_DEPTH:
      return UINT_MAX;

   case PIPE_SHADER_CAP_MAX_INPUTS:
      if (stage == MESA_SHADER_VERTEX ||
          stage == MESA_SHADER_GEOMETRY)
         return 16; /* Gen7 vec4 geom backend */
      return 32;
   case PIPE_SHADER_CAP_MAX_OUTPUTS:
      return 32;
   case PIPE_SHADER_CAP_MAX_CONST_BUFFER0_SIZE:
      return 16 * 1024 * sizeof(float);
   case PIPE_SHADER_CAP_MAX_CONST_BUFFERS:
      return devinfo->ver >= 6 ? 16 : 1;
   case PIPE_SHADER_CAP_MAX_TEMPS:
      return 256; /* GL_MAX_PROGRAM_TEMPORARIES_ARB */
   case PIPE_SHADER_CAP_CONT_SUPPORTED:
      return 0;
   case PIPE_SHADER_CAP_INDIRECT_INPUT_ADDR:
   case PIPE_SHADER_CAP_INDIRECT_OUTPUT_ADDR:
   case PIPE_SHADER_CAP_INDIRECT_TEMP_ADDR:
   case PIPE_SHADER_CAP_INDIRECT_CONST_ADDR:
      /* Lie about these to avoid st/mesa's GLSL IR lowering of indirects,
       * which we don't want.  Our compiler backend will check brw_compiler's
       * options and call nir_lower_indirect_derefs appropriately anyway.
       */
      return true;
   case PIPE_SHADER_CAP_SUBROUTINES:
      return 0;
   case PIPE_SHADER_CAP_INTEGERS:
      return 1;
   case PIPE_SHADER_CAP_INT64_ATOMICS:
   case PIPE_SHADER_CAP_FP16:
      return 0;
   case PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS:
   case PIPE_SHADER_CAP_MAX_SAMPLER_VIEWS:
      return (devinfo->verx10 >= 75) ? CROCUS_MAX_TEXTURE_SAMPLERS : 16;
   case PIPE_SHADER_CAP_MAX_SHADER_IMAGES:
      if (devinfo->ver >= 7 &&
          (p_stage == PIPE_SHADER_FRAGMENT ||
           p_stage == PIPE_SHADER_COMPUTE))
         return CROCUS_MAX_TEXTURE_SAMPLERS;
      return 0;
   case PIPE_SHADER_CAP_MAX_SHADER_BUFFERS:
      return devinfo->ver >= 7 ? (CROCUS_MAX_ABOS + CROCUS_MAX_SSBOS) : 0;
   case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTERS:
   case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTER_BUFFERS:
      return 0;
   case PIPE_SHADER_CAP_SUPPORTED_IRS:
      return 1 << PIPE_SHADER_IR_NIR;
   case PIPE_SHADER_CAP_TGSI_ANY_INOUT_DECL_RANGE:
   case PIPE_SHADER_CAP_TGSI_SQRT_SUPPORTED:
   case PIPE_SHADER_CAP_FP16_DERIVATIVES:
   case PIPE_SHADER_CAP_INT16:
   case PIPE_SHADER_CAP_GLSL_16BIT_CONSTS:
   case PIPE_SHADER_CAP_FP16_CONST_BUFFERS:
      return 0;
   default:
      unreachable("unknown shader param");
   }
}

static int
crocus_get_compute_param(struct pipe_screen *pscreen,
                         enum pipe_shader_ir ir_type,
                         enum pipe_compute_cap param,
                         void *ret)
{
   struct crocus_screen *screen = (struct crocus_screen *)pscreen;
   const struct intel_device_info *devinfo = &screen->devinfo;

   const uint32_t max_invocations = 32 * devinfo->max_cs_workgroup_threads;

   if (devinfo->ver < 7)
      return 0;
#define RET(x) do {                  \
   if (ret)                          \
      memcpy(ret, x, sizeof(x));     \
   return sizeof(x);                 \
} while (0)

   switch (param) {
   case PIPE_COMPUTE_CAP_ADDRESS_BITS:
      RET((uint32_t []){ 32 });

   case PIPE_COMPUTE_CAP_IR_TARGET:
      if (ret)
         strcpy(ret, "gen");
      return 4;

   case PIPE_COMPUTE_CAP_GRID_DIMENSION:
      RET((uint64_t []) { 3 });

   case PIPE_COMPUTE_CAP_MAX_GRID_SIZE:
      RET(((uint64_t []) { 65535, 65535, 65535 }));

   case PIPE_COMPUTE_CAP_MAX_BLOCK_SIZE:
      /* MaxComputeWorkGroupSize[0..2] */
      RET(((uint64_t []) {max_invocations, max_invocations, max_invocations}));

   case PIPE_COMPUTE_CAP_MAX_THREADS_PER_BLOCK:
      /* MaxComputeWorkGroupInvocations */
      RET((uint64_t []) { max_invocations });

   case PIPE_COMPUTE_CAP_MAX_LOCAL_SIZE:
      /* MaxComputeSharedMemorySize */
      RET((uint64_t []) { 64 * 1024 });

   case PIPE_COMPUTE_CAP_IMAGES_SUPPORTED:
      RET((uint32_t []) { 1 });

   case PIPE_COMPUTE_CAP_SUBGROUP_SIZES:
      RET((uint32_t []) { BRW_SUBGROUP_SIZE });

   case PIPE_COMPUTE_CAP_MAX_VARIABLE_THREADS_PER_BLOCK:
      RET((uint64_t []) { max_invocations });

   case PIPE_COMPUTE_CAP_MAX_MEM_ALLOC_SIZE:
   case PIPE_COMPUTE_CAP_MAX_CLOCK_FREQUENCY:
   case PIPE_COMPUTE_CAP_MAX_COMPUTE_UNITS:
   case PIPE_COMPUTE_CAP_MAX_GLOBAL_SIZE:
   case PIPE_COMPUTE_CAP_MAX_PRIVATE_SIZE:
   case PIPE_COMPUTE_CAP_MAX_INPUT_SIZE:
   case PIPE_COMPUTE_CAP_MAX_SUBGROUPS:

      // XXX: I think these are for Clover...
      return 0;

   default:
      unreachable("unknown compute param");
   }
}

static uint64_t
crocus_get_timestamp(struct pipe_screen *pscreen)
{
   struct crocus_screen *screen = (struct crocus_screen *) pscreen;
   uint64_t result;

   if (!intel_gem_read_render_timestamp(crocus_bufmgr_get_fd(screen->bufmgr),
                                        screen->devinfo.kmd_type, &result))
      return 0;

   result = intel_device_info_timebase_scale(&screen->devinfo, result);
   result &= (1ull << TIMESTAMP_BITS) - 1;

   return result;
}

void
crocus_screen_destroy(struct crocus_screen *screen)
{
   u_transfer_helper_destroy(screen->base.transfer_helper);
   crocus_bufmgr_unref(screen->bufmgr);
   disk_cache_destroy(screen->disk_cache);
   close(screen->winsys_fd);
   ralloc_free(screen);
}

static void
crocus_screen_unref(struct pipe_screen *pscreen)
{
   crocus_pscreen_unref(pscreen);
}

static void
crocus_query_memory_info(struct pipe_screen *pscreen,
                         struct pipe_memory_info *info)
{
}

static const void *
crocus_get_compiler_options(struct pipe_screen *pscreen,
                            enum pipe_shader_ir ir,
                            enum pipe_shader_type pstage)
{
   struct crocus_screen *screen = (struct crocus_screen *) pscreen;
   gl_shader_stage stage = stage_from_pipe(pstage);
   assert(ir == PIPE_SHADER_IR_NIR);

   return screen->compiler->nir_options[stage];
}

static struct disk_cache *
crocus_get_disk_shader_cache(struct pipe_screen *pscreen)
{
   struct crocus_screen *screen = (struct crocus_screen *) pscreen;
   return screen->disk_cache;
}

static const struct intel_l3_config *
crocus_get_default_l3_config(const struct intel_device_info *devinfo,
                             bool compute)
{
   bool wants_dc_cache = true;
   bool has_slm = compute;
   const struct intel_l3_weights w =
      intel_get_default_l3_weights(devinfo, wants_dc_cache, has_slm);
   return intel_get_l3_config(devinfo, w);
}

static void
crocus_shader_debug_log(void *data, unsigned *id, const char *fmt, ...)
{
   struct util_debug_callback *dbg = data;
   va_list args;

   if (!dbg->debug_message)
      return;

   va_start(args, fmt);
   dbg->debug_message(dbg->data, id, UTIL_DEBUG_TYPE_SHADER_INFO, fmt, args);
   va_end(args);
}

static void
crocus_shader_perf_log(void *data, unsigned *id, const char *fmt, ...)
{
   struct util_debug_callback *dbg = data;
   va_list args;
   va_start(args, fmt);

   if (INTEL_DEBUG(DEBUG_PERF)) {
      va_list args_copy;
      va_copy(args_copy, args);
      vfprintf(stderr, fmt, args_copy);
      va_end(args_copy);
   }

   if (dbg->debug_message) {
      dbg->debug_message(dbg->data, id, UTIL_DEBUG_TYPE_PERF_INFO, fmt, args);
   }

   va_end(args);
}

static int
crocus_screen_get_fd(struct pipe_screen *pscreen)
{
   struct crocus_screen *screen = (struct crocus_screen *)pscreen;

   return screen->winsys_fd;
}

struct pipe_screen *
crocus_screen_create(int fd, const struct pipe_screen_config *config)
{
   struct crocus_screen *screen = rzalloc(NULL, struct crocus_screen);
   if (!screen)
      return NULL;

   if (!intel_get_device_info_from_fd(fd, &screen->devinfo))
      return NULL;
   screen->pci_id = screen->devinfo.pci_device_id;

   if (screen->devinfo.ver > 8)
      return NULL;

   if (screen->devinfo.ver == 8) {
      /* bind to cherryview or bdw if forced */
      if (screen->devinfo.platform != INTEL_PLATFORM_CHV &&
          !getenv("CROCUS_GEN8"))
         return NULL;
   }

   p_atomic_set(&screen->refcount, 1);

   screen->aperture_bytes = get_aperture_size(fd);
   screen->aperture_threshold = screen->aperture_bytes * 3 / 4;

   driParseConfigFiles(config->options, config->options_info, 0, "crocus",
                       NULL, NULL, NULL, 0, NULL, 0);

   bool bo_reuse = false;
   int bo_reuse_mode = driQueryOptioni(config->options, "bo_reuse");
   switch (bo_reuse_mode) {
   case DRI_CONF_BO_REUSE_DISABLED:
      break;
   case DRI_CONF_BO_REUSE_ALL:
      bo_reuse = true;
      break;
   }

   screen->bufmgr = crocus_bufmgr_get_for_fd(&screen->devinfo, fd, bo_reuse);
   if (!screen->bufmgr)
      return NULL;
   screen->fd = crocus_bufmgr_get_fd(screen->bufmgr);
   screen->winsys_fd = fd;

   brw_process_intel_debug_variable();

   screen->driconf.dual_color_blend_by_location =
      driQueryOptionb(config->options, "dual_color_blend_by_location");
   screen->driconf.disable_throttling =
      driQueryOptionb(config->options, "disable_throttling");
   screen->driconf.always_flush_cache =
      driQueryOptionb(config->options, "always_flush_cache");
   screen->driconf.limit_trig_input_range =
      driQueryOptionb(config->options, "limit_trig_input_range");
   screen->driconf.lower_depth_range_rate =
      driQueryOptionf(config->options, "lower_depth_range_rate");

   screen->precompile = debug_get_bool_option("shader_precompile", true);

   isl_device_init(&screen->isl_dev, &screen->devinfo);

   screen->compiler = brw_compiler_create(screen, &screen->devinfo);
   screen->compiler->shader_debug_log = crocus_shader_debug_log;
   screen->compiler->shader_perf_log = crocus_shader_perf_log;
   screen->compiler->supports_shader_constants = false;
   screen->compiler->constant_buffer_0_is_relative = true;

   if (screen->devinfo.ver >= 7) {
      screen->l3_config_3d = crocus_get_default_l3_config(&screen->devinfo, false);
      screen->l3_config_cs = crocus_get_default_l3_config(&screen->devinfo, true);
   }

   crocus_disk_cache_init(screen);

   slab_create_parent(&screen->transfer_pool,
                      sizeof(struct crocus_transfer), 64);

   struct pipe_screen *pscreen = &screen->base;

   crocus_init_screen_fence_functions(pscreen);
   crocus_init_screen_resource_functions(pscreen);

   pscreen->destroy = crocus_screen_unref;
   pscreen->get_name = crocus_get_name;
   pscreen->get_vendor = crocus_get_vendor;
   pscreen->get_device_vendor = crocus_get_device_vendor;
   pscreen->get_screen_fd = crocus_screen_get_fd;
   pscreen->get_param = crocus_get_param;
   pscreen->get_shader_param = crocus_get_shader_param;
   pscreen->get_compute_param = crocus_get_compute_param;
   pscreen->get_paramf = crocus_get_paramf;
   pscreen->get_compiler_options = crocus_get_compiler_options;
   pscreen->get_device_uuid = crocus_get_device_uuid;
   pscreen->get_driver_uuid = crocus_get_driver_uuid;
   pscreen->get_disk_shader_cache = crocus_get_disk_shader_cache;
   pscreen->is_format_supported = crocus_is_format_supported;
   pscreen->context_create = crocus_create_context;
   pscreen->get_timestamp = crocus_get_timestamp;
   pscreen->query_memory_info = crocus_query_memory_info;
   pscreen->get_driver_query_group_info = crocus_get_monitor_group_info;
   pscreen->get_driver_query_info = crocus_get_monitor_info;

   genX_call(&screen->devinfo, crocus_init_screen_state, screen);
   genX_call(&screen->devinfo, crocus_init_screen_query, screen);
   return pscreen;
}
