/*
 * Copyright © Microsoft Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "d3d12_screen.h"

#include "d3d12_bufmgr.h"
#include "d3d12_compiler.h"
#include "d3d12_context.h"
#include "d3d12_debug.h"
#include "d3d12_fence.h"
#ifdef HAVE_GALLIUM_D3D12_VIDEO
#include "d3d12_video_screen.h"
#endif
#include "d3d12_format.h"
#include "d3d12_interop_public.h"
#include "d3d12_residency.h"
#include "d3d12_resource.h"
#include "d3d12_nir_passes.h"

#include "pipebuffer/pb_bufmgr.h"
#include "util/u_debug.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/u_screen.h"
#include "util/u_dl.h"
#include "util/mesa-sha1.h"

#include "nir.h"
#include "frontend/sw_winsys.h"

#include "nir_to_dxil.h"
#include "git_sha1.h"

#ifndef _GAMING_XBOX
#include <directx/d3d12sdklayers.h>
#endif

#include <dxguids/dxguids.h>
static GUID OpenGLOn12CreatorID = { 0x6bb3cd34, 0x0d19, 0x45ab, { 0x97, 0xed, 0xd7, 0x20, 0xba, 0x3d, 0xfc, 0x80 } };

static const struct debug_named_value
d3d12_debug_options[] = {
   { "verbose",      D3D12_DEBUG_VERBOSE,       NULL },
   { "blit",         D3D12_DEBUG_BLIT,          "Trace blit and copy resource calls" },
   { "experimental", D3D12_DEBUG_EXPERIMENTAL,  "Enable experimental shader models feature" },
   { "dxil",         D3D12_DEBUG_DXIL,          "Dump DXIL during program compile" },
   { "disass",       D3D12_DEBUG_DISASS,        "Dump disassambly of created DXIL shader" },
   { "res",          D3D12_DEBUG_RESOURCE,      "Debug resources" },
   { "debuglayer",   D3D12_DEBUG_DEBUG_LAYER,   "Enable debug layer" },
   { "gpuvalidator", D3D12_DEBUG_GPU_VALIDATOR, "Enable GPU validator" },
   { "singleton",    D3D12_DEBUG_SINGLETON,     "Disallow use of device factory" },
   DEBUG_NAMED_VALUE_END
};

DEBUG_GET_ONCE_FLAGS_OPTION(d3d12_debug, "D3D12_DEBUG", d3d12_debug_options, 0)

uint32_t
d3d12_debug;

enum {
    HW_VENDOR_AMD                   = 0x1002,
    HW_VENDOR_INTEL                 = 0x8086,
    HW_VENDOR_MICROSOFT             = 0x1414,
    HW_VENDOR_NVIDIA                = 0x10de,
};

static const char *
d3d12_get_vendor(struct pipe_screen *pscreen)
{
   return "Microsoft Corporation";
}

static const char *
d3d12_get_device_vendor(struct pipe_screen *pscreen)
{
   struct d3d12_screen* screen = d3d12_screen(pscreen);

   switch (screen->vendor_id) {
   case HW_VENDOR_MICROSOFT:
      return "Microsoft";
   case HW_VENDOR_AMD:
      return "AMD";
   case HW_VENDOR_NVIDIA:
      return "NVIDIA";
   case HW_VENDOR_INTEL:
      return "Intel";
   default:
      return "Unknown";
   }
}

static int
d3d12_get_video_mem(struct pipe_screen *pscreen)
{
   struct d3d12_screen* screen = d3d12_screen(pscreen);

   return screen->memory_size_megabytes;
}

static int
d3d12_get_param(struct pipe_screen *pscreen, enum pipe_cap param)
{
   struct d3d12_screen *screen = d3d12_screen(pscreen);

   switch (param) {
   case PIPE_CAP_NPOT_TEXTURES:
      return 1;

   case PIPE_CAP_MAX_DUAL_SOURCE_RENDER_TARGETS:
      /* D3D12 only supports dual-source blending for a single
       * render-target. From the D3D11 functional spec (which also defines
       * this for D3D12):
       *
       * "When Dual Source Color Blending is enabled, the Pixel Shader must
       *  have only a single RenderTarget bound, at slot 0, and must output
       *  both o0 and o1. Writing to other outputs (o2, o3 etc.) produces
       *  undefined results for the corresponding RenderTargets, if bound
       *  illegally."
       *
       * Source: https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm#17.6%20Dual%20Source%20Color%20Blending
       */
      return 1;

   case PIPE_CAP_ANISOTROPIC_FILTER:
      return 1;

   case PIPE_CAP_MAX_RENDER_TARGETS:
      return D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;

   case PIPE_CAP_TEXTURE_SWIZZLE:
      return 1;

   case PIPE_CAP_MAX_TEXEL_BUFFER_ELEMENTS_UINT:
      return 1 << D3D12_REQ_BUFFER_RESOURCE_TEXEL_COUNT_2_TO_EXP;

   case PIPE_CAP_MAX_TEXTURE_2D_SIZE:
      return D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;

   case PIPE_CAP_MAX_TEXTURE_3D_LEVELS:
      static_assert(D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION == (1 << 11),
                    "D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION");
      return 12;

   case PIPE_CAP_MAX_TEXTURE_CUBE_LEVELS:
      return D3D12_REQ_MIP_LEVELS;

   case PIPE_CAP_PRIMITIVE_RESTART:
   case PIPE_CAP_INDEP_BLEND_ENABLE:
   case PIPE_CAP_INDEP_BLEND_FUNC:
   case PIPE_CAP_FRAGMENT_SHADER_TEXTURE_LOD:
   case PIPE_CAP_FRAGMENT_SHADER_DERIVATIVES:
   case PIPE_CAP_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION:
   case PIPE_CAP_VERTEX_BUFFER_STRIDE_4BYTE_ALIGNED_ONLY:
   case PIPE_CAP_MIXED_COLOR_DEPTH_BITS:
      return 1;

   /* We need to do some lowering that requires a link to the sampler */
   case PIPE_CAP_NIR_SAMPLERS_AS_DEREF:
      return 1;

   case PIPE_CAP_NIR_IMAGES_AS_DEREF:
      return 1;

   case PIPE_CAP_MAX_TEXTURE_ARRAY_LAYERS:
      return D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;

   case PIPE_CAP_DEPTH_CLIP_DISABLE:
      return 1;

   case PIPE_CAP_TGSI_TEXCOORD:
      return 1;

   case PIPE_CAP_VERTEX_COLOR_UNCLAMPED:
      return 1;

   case PIPE_CAP_GLSL_FEATURE_LEVEL:
      return 460;
   case PIPE_CAP_GLSL_FEATURE_LEVEL_COMPATIBILITY:
      return 460;
   case PIPE_CAP_ESSL_FEATURE_LEVEL:
      return 310;

   case PIPE_CAP_COMPUTE:
      return 1;

   case PIPE_CAP_TEXTURE_MULTISAMPLE:
      return 1;

   case PIPE_CAP_CUBE_MAP_ARRAY:
      return 1;

   case PIPE_CAP_TEXTURE_BUFFER_OBJECTS:
      return 1;

   case PIPE_CAP_TEXTURE_TRANSFER_MODES:
      return PIPE_TEXTURE_TRANSFER_BLIT;

   case PIPE_CAP_ENDIANNESS:
      return PIPE_ENDIAN_NATIVE; /* unsure */

   case PIPE_CAP_MAX_VIEWPORTS:
      return D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;

   case PIPE_CAP_MIXED_FRAMEBUFFER_SIZES:
      return 1;

   case PIPE_CAP_MAX_TEXTURE_GATHER_COMPONENTS:
      return 4;

   case PIPE_CAP_FS_COORD_PIXEL_CENTER_HALF_INTEGER:
   case PIPE_CAP_FS_COORD_ORIGIN_UPPER_LEFT:
      return 1;

   case PIPE_CAP_FS_FACE_IS_INTEGER_SYSVAL:
      return 1;

   case PIPE_CAP_ACCELERATED:
      return screen->vendor_id != HW_VENDOR_MICROSOFT;

   case PIPE_CAP_VIDEO_MEMORY:
      return d3d12_get_video_mem(pscreen);

   case PIPE_CAP_UMA:
      return screen->architecture.UMA;

   case PIPE_CAP_MAX_VERTEX_ATTRIB_STRIDE:
      return 2048; /* FIXME: no clue how to query this */

   case PIPE_CAP_TEXTURE_FLOAT_LINEAR:
   case PIPE_CAP_TEXTURE_HALF_FLOAT_LINEAR:
      return 1;

   case PIPE_CAP_SHADER_BUFFER_OFFSET_ALIGNMENT:
      return D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT;

   case PIPE_CAP_CONSTANT_BUFFER_OFFSET_ALIGNMENT:
      return D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;

   case PIPE_CAP_PCI_GROUP:
   case PIPE_CAP_PCI_BUS:
   case PIPE_CAP_PCI_DEVICE:
   case PIPE_CAP_PCI_FUNCTION:
      return 0; /* TODO: figure these out */

   case PIPE_CAP_FLATSHADE:
   case PIPE_CAP_ALPHA_TEST:
   case PIPE_CAP_TWO_SIDED_COLOR:
   case PIPE_CAP_CLIP_PLANES:
      return 0;

   case PIPE_CAP_SHADER_STENCIL_EXPORT:
      return screen->opts.PSSpecifiedStencilRefSupported;

   case PIPE_CAP_SEAMLESS_CUBE_MAP:
   case PIPE_CAP_TEXTURE_QUERY_LOD:
   case PIPE_CAP_VS_INSTANCEID:
   case PIPE_CAP_TGSI_TEX_TXF_LZ:
   case PIPE_CAP_OCCLUSION_QUERY:
   case PIPE_CAP_VIEWPORT_TRANSFORM_LOWERED:
   case PIPE_CAP_PSIZ_CLAMPED:
   case PIPE_CAP_BLEND_EQUATION_SEPARATE:
   case PIPE_CAP_CONDITIONAL_RENDER:
   case PIPE_CAP_CONDITIONAL_RENDER_INVERTED:
   case PIPE_CAP_QUERY_TIMESTAMP:
   case PIPE_CAP_VERTEX_ELEMENT_INSTANCE_DIVISOR:
   case PIPE_CAP_VERTEX_ELEMENT_SRC_OFFSET_4BYTE_ALIGNED_ONLY:
   case PIPE_CAP_IMAGE_STORE_FORMATTED:
   case PIPE_CAP_GLSL_TESS_LEVELS_AS_INPUTS:
      return 1;

   case PIPE_CAP_MAX_STREAM_OUTPUT_BUFFERS:
      return D3D12_SO_BUFFER_SLOT_COUNT;

   case PIPE_CAP_MAX_STREAM_OUTPUT_SEPARATE_COMPONENTS:
   case PIPE_CAP_MAX_STREAM_OUTPUT_INTERLEAVED_COMPONENTS:
      return D3D12_SO_OUTPUT_COMPONENT_COUNT;

   /* Geometry shader output. */
   case PIPE_CAP_MAX_GEOMETRY_OUTPUT_VERTICES:
      return D3D12_GS_MAX_OUTPUT_VERTEX_COUNT_ACROSS_INSTANCES;
   case PIPE_CAP_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS:
      return D3D12_REQ_GS_INVOCATION_32BIT_OUTPUT_COMPONENT_LIMIT;

   case PIPE_CAP_MAX_VARYINGS:
      /* Subtract one so that implicit position can be added */
      return D3D12_PS_INPUT_REGISTER_COUNT - 1;

   case PIPE_CAP_NIR_COMPACT_ARRAYS:
      return 1;

   case PIPE_CAP_MAX_COMBINED_SHADER_OUTPUT_RESOURCES:
      if (screen->max_feature_level <= D3D_FEATURE_LEVEL_11_0)
         return D3D12_PS_CS_UAV_REGISTER_COUNT;
      if (screen->opts.ResourceBindingTier <= D3D12_RESOURCE_BINDING_TIER_2)
         return D3D12_UAV_SLOT_COUNT;
      return 0;

   case PIPE_CAP_START_INSTANCE:
   case PIPE_CAP_DRAW_PARAMETERS:
   case PIPE_CAP_DRAW_INDIRECT:
   case PIPE_CAP_MULTI_DRAW_INDIRECT:
   case PIPE_CAP_MULTI_DRAW_INDIRECT_PARAMS:
   case PIPE_CAP_FRAMEBUFFER_NO_ATTACHMENT:
   case PIPE_CAP_SAMPLE_SHADING:
   case PIPE_CAP_STREAM_OUTPUT_PAUSE_RESUME:
   case PIPE_CAP_STREAM_OUTPUT_INTERLEAVE_BUFFERS:
   case PIPE_CAP_INT64:
   case PIPE_CAP_DOUBLES:
   case PIPE_CAP_DEVICE_RESET_STATUS_QUERY:
   case PIPE_CAP_ROBUST_BUFFER_ACCESS_BEHAVIOR:
   case PIPE_CAP_MEMOBJ:
   case PIPE_CAP_FENCE_SIGNAL:
   case PIPE_CAP_TIMELINE_SEMAPHORE_IMPORT:
   case PIPE_CAP_CLIP_HALFZ:
   case PIPE_CAP_VS_LAYER_VIEWPORT:
   case PIPE_CAP_COPY_BETWEEN_COMPRESSED_AND_PLAIN_FORMATS:
   case PIPE_CAP_SHADER_ARRAY_COMPONENTS:
   case PIPE_CAP_TEXTURE_MIRROR_CLAMP_TO_EDGE:
   case PIPE_CAP_QUERY_TIME_ELAPSED:
   case PIPE_CAP_FS_FINE_DERIVATIVE:
   case PIPE_CAP_CULL_DISTANCE:
   case PIPE_CAP_TEXTURE_QUERY_SAMPLES:
   case PIPE_CAP_TEXTURE_BARRIER:
   case PIPE_CAP_GL_SPIRV:
   case PIPE_CAP_POLYGON_OFFSET_CLAMP:
   case PIPE_CAP_SHADER_GROUP_VOTE:
   case PIPE_CAP_QUERY_PIPELINE_STATISTICS:
   case PIPE_CAP_QUERY_SO_OVERFLOW:
      return 1;

   case PIPE_CAP_QUERY_BUFFER_OBJECT:
      return (screen->opts3.WriteBufferImmediateSupportFlags & D3D12_COMMAND_LIST_SUPPORT_FLAG_DIRECT) != 0;

   case PIPE_CAP_MAX_VERTEX_STREAMS:
      return D3D12_SO_BUFFER_SLOT_COUNT;

   case PIPE_CAP_MAX_SHADER_PATCH_VARYINGS:
      /* This is asking about varyings, not total registers, so remove the 2 tess factor registers. */
      return D3D12_HS_OUTPUT_PATCH_CONSTANT_REGISTER_COUNT - 2;

   case PIPE_CAP_MAX_TEXTURE_UPLOAD_MEMORY_BUDGET:
      /* Picking a value in line with other drivers. Without this, we can end up easily hitting OOM
       * if an app just creates, initializes, and destroys resources without explicitly flushing. */
      return 64 * 1024 * 1024;

   case PIPE_CAP_SAMPLER_VIEW_TARGET:
      return screen->opts12.RelaxedFormatCastingSupported;

   default:
      return u_pipe_screen_get_param_defaults(pscreen, param);
   }
}

static float
d3d12_get_paramf(struct pipe_screen *pscreen, enum pipe_capf param)
{
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
      return 1.0f; /* no clue */

   case PIPE_CAPF_MAX_POINT_SIZE:
   case PIPE_CAPF_MAX_POINT_SIZE_AA:
      return D3D12_MAX_POINT_SIZE;

   case PIPE_CAPF_MAX_TEXTURE_ANISOTROPY:
      return D3D12_MAX_MAXANISOTROPY;

   case PIPE_CAPF_MAX_TEXTURE_LOD_BIAS:
      return 15.99f;

   case PIPE_CAPF_MIN_CONSERVATIVE_RASTER_DILATE:
   case PIPE_CAPF_MAX_CONSERVATIVE_RASTER_DILATE:
   case PIPE_CAPF_CONSERVATIVE_RASTER_DILATE_GRANULARITY:
      return 0.0f; /* not implemented */

   default:
      unreachable("unknown pipe_capf");
   }

   return 0.0;
}

static int
d3d12_get_shader_param(struct pipe_screen *pscreen,
                       enum pipe_shader_type shader,
                       enum pipe_shader_cap param)
{
   struct d3d12_screen *screen = d3d12_screen(pscreen);

   if (shader == PIPE_SHADER_TASK ||
       shader == PIPE_SHADER_MESH)
      return 0;

   switch (param) {
   case PIPE_SHADER_CAP_MAX_INSTRUCTIONS:
   case PIPE_SHADER_CAP_MAX_ALU_INSTRUCTIONS:
   case PIPE_SHADER_CAP_MAX_TEX_INSTRUCTIONS:
   case PIPE_SHADER_CAP_MAX_TEX_INDIRECTIONS:
   case PIPE_SHADER_CAP_MAX_CONTROL_FLOW_DEPTH:
         return INT_MAX;
      return 0;

   case PIPE_SHADER_CAP_MAX_INPUTS:
      switch (shader) {
      case PIPE_SHADER_VERTEX: return D3D12_VS_INPUT_REGISTER_COUNT;
      case PIPE_SHADER_FRAGMENT: return D3D12_PS_INPUT_REGISTER_COUNT;
      case PIPE_SHADER_GEOMETRY: return D3D12_GS_INPUT_REGISTER_COUNT;
      case PIPE_SHADER_TESS_CTRL: return D3D12_HS_CONTROL_POINT_PHASE_INPUT_REGISTER_COUNT;
      case PIPE_SHADER_TESS_EVAL: return D3D12_DS_INPUT_CONTROL_POINT_REGISTER_COUNT;
      case PIPE_SHADER_COMPUTE: return 0;
      default: unreachable("Unexpected shader");
      }
      break;

   case PIPE_SHADER_CAP_MAX_OUTPUTS:
      switch (shader) {
      case PIPE_SHADER_VERTEX: return D3D12_VS_OUTPUT_REGISTER_COUNT;
      case PIPE_SHADER_FRAGMENT: return D3D12_PS_OUTPUT_REGISTER_COUNT;
      case PIPE_SHADER_GEOMETRY: return D3D12_GS_OUTPUT_REGISTER_COUNT;
      case PIPE_SHADER_TESS_CTRL: return D3D12_HS_CONTROL_POINT_PHASE_OUTPUT_REGISTER_COUNT;
      case PIPE_SHADER_TESS_EVAL: return D3D12_DS_OUTPUT_REGISTER_COUNT;
      case PIPE_SHADER_COMPUTE: return 0;
      default: unreachable("Unexpected shader");
      }
      break;

   case PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS:
      if (screen->opts.ResourceBindingTier == D3D12_RESOURCE_BINDING_TIER_1)
         return 16;
      return PIPE_MAX_SAMPLERS;

   case PIPE_SHADER_CAP_MAX_CONST_BUFFER0_SIZE:
      return 65536;

   case PIPE_SHADER_CAP_MAX_CONST_BUFFERS:
      if (screen->opts.ResourceBindingTier < D3D12_RESOURCE_BINDING_TIER_3)
         return 13; /* 15 - 2 for lowered uniforms and state vars*/
      return 15;

   case PIPE_SHADER_CAP_MAX_TEMPS:
      return INT_MAX;

   case PIPE_SHADER_CAP_INDIRECT_INPUT_ADDR:
   case PIPE_SHADER_CAP_INDIRECT_OUTPUT_ADDR:
   case PIPE_SHADER_CAP_INDIRECT_TEMP_ADDR:
   case PIPE_SHADER_CAP_SUBROUTINES:
      return 0; /* not implemented */

   case PIPE_SHADER_CAP_INDIRECT_CONST_ADDR:
   case PIPE_SHADER_CAP_INTEGERS:
      return 1;

   case PIPE_SHADER_CAP_INT64_ATOMICS:
   case PIPE_SHADER_CAP_FP16:
      return 0; /* not implemented */

   case PIPE_SHADER_CAP_TGSI_SQRT_SUPPORTED:
      return 0; /* not implemented */

   case PIPE_SHADER_CAP_MAX_SAMPLER_VIEWS:
      /* Note: This is wrong, but this is the max value that
       * TC can support to avoid overflowing an array.
       */
      return PIPE_MAX_SAMPLERS;

   case PIPE_SHADER_CAP_TGSI_ANY_INOUT_DECL_RANGE:
      return 0; /* no idea */

   case PIPE_SHADER_CAP_MAX_SHADER_BUFFERS:
      return
         (screen->max_feature_level >= D3D_FEATURE_LEVEL_11_1 ||
          screen->opts.ResourceBindingTier >= D3D12_RESOURCE_BINDING_TIER_3) ?
         PIPE_MAX_SHADER_BUFFERS : D3D12_PS_CS_UAV_REGISTER_COUNT;

   case PIPE_SHADER_CAP_SUPPORTED_IRS:
      return 1 << PIPE_SHADER_IR_NIR;

   case PIPE_SHADER_CAP_MAX_SHADER_IMAGES:
      if (!screen->support_shader_images)
         return 0;
      return
         (screen->max_feature_level >= D3D_FEATURE_LEVEL_11_1 ||
          screen->opts.ResourceBindingTier >= D3D12_RESOURCE_BINDING_TIER_3) ?
         PIPE_MAX_SHADER_IMAGES : D3D12_PS_CS_UAV_REGISTER_COUNT;

   case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTERS:
   case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTER_BUFFERS:
   case PIPE_SHADER_CAP_CONT_SUPPORTED:
      return 0; /* not implemented */

   /* should only get here on unhandled cases */
   default: return 0;
   }
}

static int
d3d12_get_compute_param(struct pipe_screen *pscreen,
                        enum pipe_shader_ir ir,
                        enum pipe_compute_cap cap,
                        void *ret)
{
   switch (cap) {
   case PIPE_COMPUTE_CAP_MAX_GRID_SIZE: {
      uint64_t *grid = (uint64_t *)ret;
      grid[0] = grid[1] = grid[2] = D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;
      return sizeof(uint64_t) * 3;
   }
   case PIPE_COMPUTE_CAP_MAX_BLOCK_SIZE: {
      uint64_t *block = (uint64_t *)ret;
      block[0] = D3D12_CS_THREAD_GROUP_MAX_X;
      block[1] = D3D12_CS_THREAD_GROUP_MAX_Y;
      block[2] = D3D12_CS_THREAD_GROUP_MAX_Z;
      return sizeof(uint64_t) * 3;
   }
   case PIPE_COMPUTE_CAP_MAX_VARIABLE_THREADS_PER_BLOCK:
   case PIPE_COMPUTE_CAP_MAX_THREADS_PER_BLOCK:
      *(uint64_t *)ret = D3D12_CS_THREAD_GROUP_MAX_THREADS_PER_GROUP;
      return sizeof(uint64_t);
   case PIPE_COMPUTE_CAP_MAX_LOCAL_SIZE:
      *(uint64_t *)ret = D3D12_CS_TGSM_REGISTER_COUNT /*DWORDs*/ * 4;
      return sizeof(uint64_t);
   default:
      return 0;
   }
}

static bool
d3d12_is_format_supported(struct pipe_screen *pscreen,
                          enum pipe_format format,
                          enum pipe_texture_target target,
                          unsigned sample_count,
                          unsigned storage_sample_count,
                          unsigned bind)
{
   struct d3d12_screen *screen = d3d12_screen(pscreen);

   if (MAX2(1, sample_count) != MAX2(1, storage_sample_count))
      return false;

   if (target == PIPE_BUFFER) {
      /* Replace emulated vertex element formats for the tests */
      format = d3d12_emulated_vtx_format(format);
   } else {
      /* Allow 3-comp 32 bit formats only for BOs (needed for ARB_tbo_rgb32) */
      if ((format == PIPE_FORMAT_R32G32B32_FLOAT ||
           format == PIPE_FORMAT_R32G32B32_SINT ||
           format == PIPE_FORMAT_R32G32B32_UINT))
         return false;
   }

   /* Don't advertise alpha/luminance_alpha formats because they can't be used
    * for render targets (except A8_UNORM) and can't be emulated by R/RG formats.
    * Let the state tracker choose an RGBA format instead. For YUV formats, we
    * want the state tracker to lower these to individual planes. */
   if (format != PIPE_FORMAT_A8_UNORM &&
       (util_format_is_alpha(format) ||
        util_format_is_luminance_alpha(format) ||
        util_format_is_yuv(format)))
      return false;

   if (format == PIPE_FORMAT_NONE) {
      /* For UAV-only rendering, aka ARB_framebuffer_no_attachments */
      switch (sample_count) {
      case 0:
      case 1:
      case 4:
      case 8:
      case 16:
         return true;
      default:
         return false;
      }
   }

   DXGI_FORMAT dxgi_format = d3d12_get_format(format);
   if (dxgi_format == DXGI_FORMAT_UNKNOWN)
      return false;

   enum D3D12_FORMAT_SUPPORT1 dim_support = D3D12_FORMAT_SUPPORT1_NONE;
   switch (target) {
   case PIPE_TEXTURE_1D:
   case PIPE_TEXTURE_1D_ARRAY:
      dim_support = D3D12_FORMAT_SUPPORT1_TEXTURE1D;
      break;
   case PIPE_TEXTURE_2D:
   case PIPE_TEXTURE_RECT:
   case PIPE_TEXTURE_2D_ARRAY:
      dim_support = D3D12_FORMAT_SUPPORT1_TEXTURE2D;
      break;
   case PIPE_TEXTURE_3D:
      dim_support = D3D12_FORMAT_SUPPORT1_TEXTURE3D;
      break;
   case PIPE_TEXTURE_CUBE:
   case PIPE_TEXTURE_CUBE_ARRAY:
      dim_support = D3D12_FORMAT_SUPPORT1_TEXTURECUBE;
      break;
   case PIPE_BUFFER:
      dim_support = D3D12_FORMAT_SUPPORT1_BUFFER;
      break;
   default:
      unreachable("Unknown target");
   }

   if (bind & PIPE_BIND_DISPLAY_TARGET) {
      if (!screen->winsys->is_displaytarget_format_supported(screen->winsys, bind, format))
         return false;
   }

   D3D12_FEATURE_DATA_FORMAT_SUPPORT fmt_info;
   fmt_info.Format = d3d12_get_resource_rt_format(format);
   if (FAILED(screen->dev->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT,
                                               &fmt_info, sizeof(fmt_info))))
      return false;

   if (!(fmt_info.Support1 & dim_support))
      return false;

   if (target == PIPE_BUFFER) {
      if (bind & PIPE_BIND_VERTEX_BUFFER &&
          !(fmt_info.Support1 & D3D12_FORMAT_SUPPORT1_IA_VERTEX_BUFFER))
         return false;

      if (bind & PIPE_BIND_INDEX_BUFFER) {
         if (format != PIPE_FORMAT_R16_UINT &&
             format != PIPE_FORMAT_R32_UINT)
            return false;
      }

      if (sample_count > 0)
         return false;
   } else {
      /* all other targets are texture-targets */
      if (bind & PIPE_BIND_RENDER_TARGET &&
          !(fmt_info.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET))
         return false;

      if (bind & PIPE_BIND_BLENDABLE &&
         !(fmt_info.Support1 & D3D12_FORMAT_SUPPORT1_BLENDABLE))
         return false;

      if (bind & PIPE_BIND_SHADER_IMAGE &&
         (fmt_info.Support2 & (D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD | D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE)) !=
            (D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD | D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE))
         return false;

      D3D12_FEATURE_DATA_FORMAT_SUPPORT fmt_info_sv;
      if (util_format_is_depth_or_stencil(format)) {
         fmt_info_sv.Format = d3d12_get_resource_srv_format(format, target);
         if (FAILED(screen->dev->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT,
                                                     &fmt_info_sv, sizeof(fmt_info_sv))))
            return false;
      } else
         fmt_info_sv = fmt_info;

      if (bind & PIPE_BIND_DEPTH_STENCIL &&
          !(fmt_info.Support1 & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL))
            return false;

      if (sample_count > 0) {
         if (!(fmt_info_sv.Support1 & D3D12_FORMAT_SUPPORT1_MULTISAMPLE_LOAD))
            return false;

         if (!util_is_power_of_two_nonzero(sample_count))
            return false;

         if (bind & PIPE_BIND_SHADER_IMAGE)
            return false;

         D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS ms_info = {};
         ms_info.Format = dxgi_format;
         ms_info.SampleCount = sample_count;
         if (FAILED(screen->dev->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
                                                     &ms_info,
                                                     sizeof(ms_info))) ||
             !ms_info.NumQualityLevels)
            return false;
      }
   }
   return true;
}

void
d3d12_deinit_screen(struct d3d12_screen *screen)
{
   if (screen->rtv_pool) {
      d3d12_descriptor_pool_free(screen->rtv_pool);
      screen->rtv_pool = nullptr;
   }
   if (screen->dsv_pool) {
      d3d12_descriptor_pool_free(screen->dsv_pool);
      screen->dsv_pool = nullptr;
   }
   if (screen->view_pool) {
      d3d12_descriptor_pool_free(screen->view_pool);
      screen->view_pool = nullptr;
   }
   if (screen->readback_slab_bufmgr) {
      screen->readback_slab_bufmgr->destroy(screen->readback_slab_bufmgr);
      screen->readback_slab_bufmgr = nullptr;
   }
   if (screen->slab_bufmgr) {
      screen->slab_bufmgr->destroy(screen->slab_bufmgr);
      screen->slab_bufmgr = nullptr;
   }
   if (screen->cache_bufmgr) {
      screen->cache_bufmgr->destroy(screen->cache_bufmgr);
      screen->cache_bufmgr = nullptr;
   }
   if (screen->slab_cache_bufmgr) {
      screen->slab_cache_bufmgr->destroy(screen->slab_cache_bufmgr);
      screen->slab_cache_bufmgr = nullptr;
   }
   if (screen->readback_slab_cache_bufmgr) {
      screen->readback_slab_cache_bufmgr->destroy(screen->readback_slab_cache_bufmgr);
      screen->readback_slab_cache_bufmgr = nullptr;
   }
   if (screen->bufmgr) {
      screen->bufmgr->destroy(screen->bufmgr);
      screen->bufmgr = nullptr;
   }
   d3d12_deinit_residency(screen);
   if (screen->fence) {
      screen->fence->Release();
      screen->fence = nullptr;
   }
   if (screen->cmdqueue) {
      screen->cmdqueue->Release();
      screen->cmdqueue = nullptr;
   }
   if (screen->dev10) {
      screen->dev10->Release();
      screen->dev10 = nullptr;
   }
   if (screen->dev) {
      screen->dev->Release();
      screen->dev = nullptr;
   }
}

void
d3d12_destroy_screen(struct d3d12_screen *screen)
{
   slab_destroy_parent(&screen->transfer_pool);
   mtx_destroy(&screen->submit_mutex);
   mtx_destroy(&screen->descriptor_pool_mutex);
   d3d12_varying_cache_destroy(screen);
   mtx_destroy(&screen->varying_info_mutex);
   if (screen->d3d12_mod)
      util_dl_close(screen->d3d12_mod);
   glsl_type_singleton_decref();
   FREE(screen);
}

static void
d3d12_flush_frontbuffer(struct pipe_screen * pscreen,
                        struct pipe_context *pctx,
                        struct pipe_resource *pres,
                        unsigned level, unsigned layer,
                        void *winsys_drawable_handle,
                        struct pipe_box *sub_box)
{
   struct d3d12_screen *screen = d3d12_screen(pscreen);
   struct sw_winsys *winsys = screen->winsys;
   struct d3d12_resource *res = d3d12_resource(pres);

   if (!winsys || !pctx)
     return;

   assert(res->dt);
   void *map = winsys->displaytarget_map(winsys, res->dt, 0);

   if (map) {
      pctx = threaded_context_unwrap_sync(pctx);
      pipe_transfer *transfer = nullptr;
      void *res_map = pipe_texture_map(pctx, pres, level, layer, PIPE_MAP_READ, 0, 0,
                                        u_minify(pres->width0, level),
                                        u_minify(pres->height0, level),
                                        &transfer);
      if (res_map) {
         util_copy_rect((uint8_t*)map, pres->format, res->dt_stride, 0, 0,
                        transfer->box.width, transfer->box.height,
                        (const uint8_t*)res_map, transfer->stride, 0, 0);
         pipe_texture_unmap(pctx, transfer);
      }
      winsys->displaytarget_unmap(winsys, res->dt);
   }

#if defined(_WIN32) && !defined(_GAMING_XBOX)
   // WindowFromDC is Windows-only, and this method requires an HWND, so only use it on Windows
   ID3D12SharingContract *sharing_contract;
   if (SUCCEEDED(screen->cmdqueue->QueryInterface(IID_PPV_ARGS(&sharing_contract)))) {
      ID3D12Resource *d3d12_res = d3d12_resource_resource(res);
      sharing_contract->Present(d3d12_res, 0, WindowFromDC((HDC)winsys_drawable_handle));
      sharing_contract->Release();
   }
#endif

   winsys->displaytarget_display(winsys, res->dt, winsys_drawable_handle, sub_box);
}

#ifndef _GAMING_XBOX
static ID3D12Debug *
get_debug_interface(util_dl_library *d3d12_mod, ID3D12DeviceFactory *factory)
{
   ID3D12Debug *debug = nullptr;
   if (factory) {
      factory->GetConfigurationInterface(CLSID_D3D12Debug, IID_PPV_ARGS(&debug));
      return debug;
   }

   typedef HRESULT(WINAPI *PFN_D3D12_GET_DEBUG_INTERFACE)(REFIID riid, void **ppFactory);
   PFN_D3D12_GET_DEBUG_INTERFACE D3D12GetDebugInterface;

   D3D12GetDebugInterface = (PFN_D3D12_GET_DEBUG_INTERFACE)util_dl_get_proc_address(d3d12_mod, "D3D12GetDebugInterface");
   if (!D3D12GetDebugInterface) {
      debug_printf("D3D12: failed to load D3D12GetDebugInterface from D3D12.DLL\n");
      return NULL;
   }

   if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)))) {
      debug_printf("D3D12: D3D12GetDebugInterface failed\n");
      return NULL;
   }

   return debug;
}

static void
enable_d3d12_debug_layer(util_dl_library *d3d12_mod, ID3D12DeviceFactory *factory)
{
   ID3D12Debug *debug = get_debug_interface(d3d12_mod, factory);
   if (debug) {
      debug->EnableDebugLayer();
      debug->Release();
   }
}

static void
enable_gpu_validation(util_dl_library *d3d12_mod, ID3D12DeviceFactory *factory)
{
   ID3D12Debug *debug = get_debug_interface(d3d12_mod, factory);
   ID3D12Debug3 *debug3;
   if (debug) {
      if (SUCCEEDED(debug->QueryInterface(IID_PPV_ARGS(&debug3)))) {
         debug3->SetEnableGPUBasedValidation(true);
         debug3->Release();
      }
      debug->Release();
   }
}
#endif

#ifdef _GAMING_XBOX

static ID3D12Device3 *
create_device(util_dl_library *d3d12_mod, IUnknown *adapter)
{
   D3D12XBOX_PROCESS_DEBUG_FLAGS debugFlags =
      D3D12XBOX_PROCESS_DEBUG_FLAG_ENABLE_COMMON_STATE_PROMOTION; /* For compatibility with desktop D3D12 */

   if (d3d12_debug & D3D12_DEBUG_EXPERIMENTAL) {
      debug_printf("D3D12: experimental shader models are not supported on GDKX\n");
      return nullptr;
   }

   if (d3d12_debug & D3D12_DEBUG_GPU_VALIDATOR) {
      debug_printf("D3D12: gpu validation is not supported on GDKX\n"); /* FIXME: Is this right? */
      return nullptr;
   }

   if (d3d12_debug & D3D12_DEBUG_DEBUG_LAYER)
      debugFlags |= D3D12XBOX_PROCESS_DEBUG_FLAG_DEBUG;

   D3D12XBOX_CREATE_DEVICE_PARAMETERS params = {};
   params.Version = D3D12_SDK_VERSION;
   params.ProcessDebugFlags = debugFlags;
   params.GraphicsCommandQueueRingSizeBytes = D3D12XBOX_DEFAULT_SIZE_BYTES;
   params.GraphicsScratchMemorySizeBytes = D3D12XBOX_DEFAULT_SIZE_BYTES;
   params.ComputeScratchMemorySizeBytes = D3D12XBOX_DEFAULT_SIZE_BYTES;

   ID3D12Device3 *dev = nullptr;

   typedef HRESULT(WINAPI * PFN_D3D12XBOXCREATEDEVICE)(IGraphicsUnknown *, const D3D12XBOX_CREATE_DEVICE_PARAMETERS *, REFIID, void **);
   PFN_D3D12XBOXCREATEDEVICE D3D12XboxCreateDevice =
      (PFN_D3D12XBOXCREATEDEVICE) util_dl_get_proc_address(d3d12_mod, "D3D12XboxCreateDevice");
   if (!D3D12XboxCreateDevice) {
      debug_printf("D3D12: failed to load D3D12XboxCreateDevice from D3D12 DLL\n");
      return NULL;
   }
   if (FAILED(D3D12XboxCreateDevice((IGraphicsUnknown*) adapter, &params, IID_PPV_ARGS(&dev))))
      debug_printf("D3D12: D3D12XboxCreateDevice failed\n");

   return dev;
}

#else

static ID3D12Device3 *
create_device(util_dl_library *d3d12_mod, IUnknown *adapter, ID3D12DeviceFactory *factory)
{

#ifdef _WIN32
   if (d3d12_debug & D3D12_DEBUG_EXPERIMENTAL)
#endif
   {
      if (factory) {
         if (FAILED(factory->EnableExperimentalFeatures(1, &D3D12ExperimentalShaderModels, nullptr, nullptr))) {
            debug_printf("D3D12: failed to enable experimental shader models\n");
            return nullptr;
         }
      } else {
         typedef HRESULT(WINAPI *PFN_D3D12ENABLEEXPERIMENTALFEATURES)(UINT, const IID*, void*, UINT*);
         PFN_D3D12ENABLEEXPERIMENTALFEATURES D3D12EnableExperimentalFeatures =
            (PFN_D3D12ENABLEEXPERIMENTALFEATURES)util_dl_get_proc_address(d3d12_mod, "D3D12EnableExperimentalFeatures");

         if (!D3D12EnableExperimentalFeatures ||
             FAILED(D3D12EnableExperimentalFeatures(1, &D3D12ExperimentalShaderModels, NULL, NULL))) {
            debug_printf("D3D12: failed to enable experimental shader models\n");
            return nullptr;
         }
      }
   }

   ID3D12Device3 *dev = nullptr;
   if (factory) {
      factory->SetFlags(D3D12_DEVICE_FACTORY_FLAG_ALLOW_RETURNING_EXISTING_DEVICE |
         D3D12_DEVICE_FACTORY_FLAG_ALLOW_RETURNING_INCOMPATIBLE_EXISTING_DEVICE);
      if (FAILED(factory->CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&dev))))
         debug_printf("D3D12: D3D12CreateDevice failed\n");
   } else {
      typedef HRESULT(WINAPI *PFN_D3D12CREATEDEVICE)(IUnknown*, D3D_FEATURE_LEVEL, REFIID, void**);
      PFN_D3D12CREATEDEVICE D3D12CreateDevice = (PFN_D3D12CREATEDEVICE)util_dl_get_proc_address(d3d12_mod, "D3D12CreateDevice");
      if (!D3D12CreateDevice) {
         debug_printf("D3D12: failed to load D3D12CreateDevice from D3D12.DLL\n");
         return NULL;
      }
      if (FAILED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&dev))))
         debug_printf("D3D12: D3D12CreateDevice failed\n");
   }

   return dev;
}

#endif /* _GAMING_XBOX */

static bool
can_attribute_at_vertex(struct d3d12_screen *screen)
{
   switch (screen->vendor_id)  {
   case HW_VENDOR_MICROSOFT:
      return true;
   default:
      return screen->opts3.BarycentricsSupported;
   }
}

static bool
can_shader_image_load_all_formats(struct d3d12_screen *screen)
{
   if (!screen->opts.TypedUAVLoadAdditionalFormats)
      return false;

   /* All of these are required by ARB_shader_image_load_store */
   static const DXGI_FORMAT additional_formats[] = {
      DXGI_FORMAT_R16G16B16A16_UNORM,
      DXGI_FORMAT_R16G16B16A16_SNORM,
      DXGI_FORMAT_R32G32_FLOAT,
      DXGI_FORMAT_R32G32_UINT,
      DXGI_FORMAT_R32G32_SINT,
      DXGI_FORMAT_R10G10B10A2_UNORM,
      DXGI_FORMAT_R10G10B10A2_UINT,
      DXGI_FORMAT_R11G11B10_FLOAT,
      DXGI_FORMAT_R8G8B8A8_SNORM,
      DXGI_FORMAT_R16G16_FLOAT,
      DXGI_FORMAT_R16G16_UNORM,
      DXGI_FORMAT_R16G16_UINT,
      DXGI_FORMAT_R16G16_SNORM,
      DXGI_FORMAT_R16G16_SINT,
      DXGI_FORMAT_R8G8_UNORM,
      DXGI_FORMAT_R8G8_UINT,
      DXGI_FORMAT_R8G8_SNORM,
      DXGI_FORMAT_R8G8_SINT,
      DXGI_FORMAT_R16_UNORM,
      DXGI_FORMAT_R16_SNORM,
      DXGI_FORMAT_R8_SNORM,
   };

   for (unsigned i = 0; i < ARRAY_SIZE(additional_formats); ++i) {
      D3D12_FEATURE_DATA_FORMAT_SUPPORT support = { additional_formats[i] };
      if (FAILED(screen->dev->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &support, sizeof(support))) ||
         (support.Support1 & D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW) == D3D12_FORMAT_SUPPORT1_NONE ||
         (support.Support2 & (D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD | D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE)) !=
            (D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD | D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE))
         return false;
   }

   return true;
}

static void
d3d12_init_null_srvs(struct d3d12_screen *screen)
{
   for (unsigned i = 0; i < RESOURCE_DIMENSION_COUNT; ++i) {
      D3D12_SHADER_RESOURCE_VIEW_DESC srv = {};

      srv.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
      srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      switch (i) {
      case RESOURCE_DIMENSION_BUFFER:
      case RESOURCE_DIMENSION_UNKNOWN:
         srv.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
         srv.Buffer.FirstElement = 0;
         srv.Buffer.NumElements = 0;
         srv.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
         srv.Buffer.StructureByteStride = 0;
         break;
      case RESOURCE_DIMENSION_TEXTURE1D:
         srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
         srv.Texture1D.MipLevels = 1;
         srv.Texture1D.MostDetailedMip = 0;
         srv.Texture1D.ResourceMinLODClamp = 0.0f;
         break;
      case RESOURCE_DIMENSION_TEXTURE1DARRAY:
         srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
         srv.Texture1DArray.MipLevels = 1;
         srv.Texture1DArray.ArraySize = 1;
         srv.Texture1DArray.MostDetailedMip = 0;
         srv.Texture1DArray.FirstArraySlice = 0;
         srv.Texture1DArray.ResourceMinLODClamp = 0.0f;
         break;
      case RESOURCE_DIMENSION_TEXTURE2D:
         srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
         srv.Texture2D.MipLevels = 1;
         srv.Texture2D.MostDetailedMip = 0;
         srv.Texture2D.PlaneSlice = 0;
         srv.Texture2D.ResourceMinLODClamp = 0.0f;
         break;
      case RESOURCE_DIMENSION_TEXTURE2DARRAY:
         srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
         srv.Texture2DArray.MipLevels = 1;
         srv.Texture2DArray.ArraySize = 1;
         srv.Texture2DArray.MostDetailedMip = 0;
         srv.Texture2DArray.FirstArraySlice = 0;
         srv.Texture2DArray.PlaneSlice = 0;
         srv.Texture2DArray.ResourceMinLODClamp = 0.0f;
         break;
      case RESOURCE_DIMENSION_TEXTURE2DMS:
         srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
         break;
      case RESOURCE_DIMENSION_TEXTURE2DMSARRAY:
         srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
         srv.Texture2DMSArray.ArraySize = 1;
         srv.Texture2DMSArray.FirstArraySlice = 0;
         break;
      case RESOURCE_DIMENSION_TEXTURE3D:
         srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
         srv.Texture3D.MipLevels = 1;
         srv.Texture3D.MostDetailedMip = 0;
         srv.Texture3D.ResourceMinLODClamp = 0.0f;
         break;
      case RESOURCE_DIMENSION_TEXTURECUBE:
         srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
         srv.TextureCube.MipLevels = 1;
         srv.TextureCube.MostDetailedMip = 0;
         srv.TextureCube.ResourceMinLODClamp = 0.0f;
         break;
      case RESOURCE_DIMENSION_TEXTURECUBEARRAY:
         srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
         srv.TextureCubeArray.MipLevels = 1;
         srv.TextureCubeArray.NumCubes = 1;
         srv.TextureCubeArray.MostDetailedMip = 0;
         srv.TextureCubeArray.First2DArrayFace = 0;
         srv.TextureCubeArray.ResourceMinLODClamp = 0.0f;
         break;
      }

      if (srv.ViewDimension != D3D12_SRV_DIMENSION_UNKNOWN)
      {
         d3d12_descriptor_pool_alloc_handle(screen->view_pool, &screen->null_srvs[i]);
         screen->dev->CreateShaderResourceView(NULL, &srv, screen->null_srvs[i].cpu_handle);
      }
   }
}

static void
d3d12_init_null_uavs(struct d3d12_screen *screen)
{
   for (unsigned i = 0; i < RESOURCE_DIMENSION_COUNT; ++i) {
      D3D12_UNORDERED_ACCESS_VIEW_DESC uav = {};

      uav.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
      switch (i) {
      case RESOURCE_DIMENSION_BUFFER:
      case RESOURCE_DIMENSION_UNKNOWN:
         uav.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
         uav.Buffer.FirstElement = 0;
         uav.Buffer.NumElements = 0;
         uav.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
         uav.Buffer.StructureByteStride = 0;
         uav.Buffer.CounterOffsetInBytes = 0;
         break;
      case RESOURCE_DIMENSION_TEXTURE1D:
         uav.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
         uav.Texture1D.MipSlice = 0;
         break;
      case RESOURCE_DIMENSION_TEXTURE1DARRAY:
         uav.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
         uav.Texture1DArray.MipSlice = 0;
         uav.Texture1DArray.ArraySize = 1;
         uav.Texture1DArray.FirstArraySlice = 0;
         break;
      case RESOURCE_DIMENSION_TEXTURE2D:
         uav.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
         uav.Texture2D.MipSlice = 0;
         uav.Texture2D.PlaneSlice = 0;
         break;
      case RESOURCE_DIMENSION_TEXTURE2DARRAY:
      case RESOURCE_DIMENSION_TEXTURECUBE:
      case RESOURCE_DIMENSION_TEXTURECUBEARRAY:
         uav.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
         uav.Texture2DArray.MipSlice = 0;
         uav.Texture2DArray.ArraySize = 1;
         uav.Texture2DArray.FirstArraySlice = 0;
         uav.Texture2DArray.PlaneSlice = 0;
         break;
      case RESOURCE_DIMENSION_TEXTURE2DMS:
      case RESOURCE_DIMENSION_TEXTURE2DMSARRAY:
         break;
      case RESOURCE_DIMENSION_TEXTURE3D:
         uav.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
         uav.Texture3D.MipSlice = 0;
         uav.Texture3D.FirstWSlice = 0;
         uav.Texture3D.WSize = 1;
         break;
      }

      if (uav.ViewDimension != D3D12_UAV_DIMENSION_UNKNOWN)
      {
         d3d12_descriptor_pool_alloc_handle(screen->view_pool, &screen->null_uavs[i]);
         screen->dev->CreateUnorderedAccessView(NULL, NULL, &uav, screen->null_uavs[i].cpu_handle);
      }
   }
}

static void
d3d12_init_null_rtv(struct d3d12_screen *screen)
{
   D3D12_RENDER_TARGET_VIEW_DESC rtv = {};
   rtv.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
   rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
   rtv.Texture2D.MipSlice = 0;
   rtv.Texture2D.PlaneSlice = 0;
   d3d12_descriptor_pool_alloc_handle(screen->rtv_pool, &screen->null_rtv);
   screen->dev->CreateRenderTargetView(NULL, &rtv, screen->null_rtv.cpu_handle);
}

static void
d3d12_get_adapter_luid(struct pipe_screen *pscreen, char *luid)
{
   struct d3d12_screen *screen = d3d12_screen(pscreen);
   memcpy(luid, &screen->adapter_luid, PIPE_LUID_SIZE);
}

static void
d3d12_get_device_uuid(struct pipe_screen *pscreen, char *uuid)
{
   struct d3d12_screen *screen = d3d12_screen(pscreen);
   memcpy(uuid, &screen->device_uuid, PIPE_UUID_SIZE);
}

static void
d3d12_get_driver_uuid(struct pipe_screen *pscreen, char *uuid)
{
   struct d3d12_screen *screen = d3d12_screen(pscreen);
   memcpy(uuid, &screen->driver_uuid, PIPE_UUID_SIZE);
}

static uint32_t
d3d12_get_node_mask(struct pipe_screen *pscreen)
{
   /* This implementation doesn't support linked adapters */
   return 1;
}

static void
d3d12_create_fence_win32(struct pipe_screen *pscreen, struct pipe_fence_handle **pfence, void *handle, const void *name, enum pipe_fd_type type)
{
   d3d12_fence_reference((struct d3d12_fence **)pfence, nullptr);
   if(type == PIPE_FD_TYPE_TIMELINE_SEMAPHORE)
      *pfence = (struct pipe_fence_handle*) d3d12_open_fence(d3d12_screen(pscreen), handle, name);
}

static void
d3d12_set_fence_timeline_value(struct pipe_screen *pscreen, struct pipe_fence_handle *pfence, uint64_t value)
{
   d3d12_fence(pfence)->value = value;
}

static uint32_t
d3d12_interop_query_device_info(struct pipe_screen *pscreen, uint32_t data_size, void *data)
{
   if (data_size < sizeof(d3d12_interop_device_info) || !data)
      return 0;
   d3d12_interop_device_info *info = (d3d12_interop_device_info *)data;
   struct d3d12_screen *screen = d3d12_screen(pscreen);

   static_assert(sizeof(info->adapter_luid) == sizeof(screen->adapter_luid),
                 "Using uint64_t instead of Windows-specific type");
   memcpy(&info->adapter_luid, &screen->adapter_luid, sizeof(screen->adapter_luid));
   info->device = screen->dev;
   info->queue = screen->cmdqueue;
   return sizeof(*info);
}

static uint32_t
d3d12_interop_export_object(struct pipe_screen *pscreen, struct pipe_resource *res,
                              uint32_t data_size, void *data, bool *need_export_dmabuf)
{
   if (data_size < sizeof(d3d12_interop_resource_info) || !data)
      return 0;
   d3d12_interop_resource_info *info = (d3d12_interop_resource_info *)data;
   
   info->resource = d3d12_resource_underlying(d3d12_resource(res), &info->buffer_offset);
   *need_export_dmabuf = false;
   return sizeof(*info);
}

static int
d3d12_screen_get_fd(struct pipe_screen *pscreen)
{
   struct d3d12_screen *screen = d3d12_screen(pscreen);
   struct sw_winsys *winsys = screen->winsys;

   if (winsys->get_fd)
      return winsys->get_fd(winsys);
   else
      return -1;
}

#ifdef _WIN32
static void* d3d12_fence_get_win32_handle(struct pipe_screen *pscreen,
                                          struct pipe_fence_handle *fence_handle,
                                          uint64_t *fence_value)
{
   struct d3d12_screen *screen = d3d12_screen(pscreen);
   struct d3d12_fence* fence = (struct d3d12_fence*) fence_handle;
   HANDLE shared_handle = nullptr;
   screen->dev->CreateSharedHandle(fence->cmdqueue_fence,
                                   NULL,
                                   GENERIC_ALL,
                                   NULL,
                                   &shared_handle);
   if(shared_handle)
      *fence_value = fence->value;

   return (void*) shared_handle;
}
#endif

bool
d3d12_init_screen_base(struct d3d12_screen *screen, struct sw_winsys *winsys, LUID *adapter_luid)
{
   glsl_type_singleton_init_or_ref();
   d3d12_debug = debug_get_option_d3d12_debug();

   screen->winsys = winsys;
   if (adapter_luid)
      screen->adapter_luid = *adapter_luid;
   mtx_init(&screen->descriptor_pool_mutex, mtx_plain);
   mtx_init(&screen->submit_mutex, mtx_plain);

   list_inithead(&screen->context_list);
   screen->context_id_count = 16;

   // Fill the array backwards, because we'll pop off the back to assign ids
   for (unsigned i = 0; i < 16; ++i)
      screen->context_id_list[i] = 15 - i;

   d3d12_varying_cache_init(screen);
   mtx_init(&screen->varying_info_mutex, mtx_plain);

   slab_create_parent(&screen->transfer_pool, sizeof(struct d3d12_transfer), 16);

   screen->base.get_vendor = d3d12_get_vendor;
   screen->base.get_device_vendor = d3d12_get_device_vendor;
   screen->base.get_screen_fd = d3d12_screen_get_fd;
   screen->base.get_param = d3d12_get_param;
   screen->base.get_paramf = d3d12_get_paramf;
   screen->base.get_shader_param = d3d12_get_shader_param;
   screen->base.get_compute_param = d3d12_get_compute_param;
   screen->base.is_format_supported = d3d12_is_format_supported;
   screen->base.get_compiler_options = d3d12_get_compiler_options;
   screen->base.context_create = d3d12_context_create;
   screen->base.flush_frontbuffer = d3d12_flush_frontbuffer;
   screen->base.get_device_luid = d3d12_get_adapter_luid;
   screen->base.get_device_uuid = d3d12_get_device_uuid;
   screen->base.get_driver_uuid = d3d12_get_driver_uuid;
   screen->base.get_device_node_mask = d3d12_get_node_mask;
   screen->base.create_fence_win32 = d3d12_create_fence_win32;
   screen->base.set_fence_timeline_value = d3d12_set_fence_timeline_value;
   screen->base.interop_query_device_info = d3d12_interop_query_device_info;
   screen->base.interop_export_object = d3d12_interop_export_object;
#ifdef _WIN32
   screen->base.fence_get_win32_handle = d3d12_fence_get_win32_handle;
#endif

   screen->d3d12_mod = util_dl_open(
      UTIL_DL_PREFIX
#ifdef _GAMING_XBOX_SCARLETT
      "d3d12_xs"
#elif defined(_GAMING_XBOX)
      "d3d12_x"
#else
      "d3d12"
#endif
      UTIL_DL_EXT
   );
   if (!screen->d3d12_mod) {
      debug_printf("D3D12: failed to load D3D12.DLL\n");
      return false;
   }
   return true;
}

#ifdef _WIN32
extern "C" IMAGE_DOS_HEADER __ImageBase;
static const char *
try_find_d3d12core_next_to_self(char *path, size_t path_arr_size)
{
   uint32_t path_size = GetModuleFileNameA((HINSTANCE)&__ImageBase,
                                           path, path_arr_size);
   if (!path_arr_size || path_size == path_arr_size) {
      debug_printf("Unable to get path to self");
      return nullptr;
   }

   auto last_slash = strrchr(path, '\\');
   if (!last_slash) {
      debug_printf("Unable to get path to self");
      return nullptr;
   }

   *(last_slash + 1) = '\0';
   if (strcat_s(path, path_arr_size, "D3D12Core.dll") != 0) {
      debug_printf("Unable to get path to D3D12Core.dll next to self");
      return nullptr;
   }

   if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES) {
      debug_printf("No D3D12Core.dll exists next to self");
      return nullptr;
   }

   return path;
}
#endif

#ifndef _GAMING_XBOX
static ID3D12DeviceFactory *
try_create_device_factory(util_dl_library *d3d12_mod)
{
   if (d3d12_debug & D3D12_DEBUG_SINGLETON)
      return nullptr;

   /* A device factory allows us to isolate things like debug layer enablement from other callers,
    * and can potentially even refer to a different D3D12 redist implementation from others.
    */
   ID3D12DeviceFactory *factory = nullptr;

   typedef HRESULT(WINAPI *PFN_D3D12_GET_INTERFACE)(REFCLSID clsid, REFIID riid, void **ppFactory);
   PFN_D3D12_GET_INTERFACE D3D12GetInterface = (PFN_D3D12_GET_INTERFACE)util_dl_get_proc_address(d3d12_mod, "D3D12GetInterface");
   if (!D3D12GetInterface) {
      debug_printf("D3D12: Failed to retrieve D3D12GetInterface");
      return nullptr;
   }

#ifdef _WIN32
   /* First, try to create a device factory from a DLL-parallel D3D12Core.dll */
   ID3D12SDKConfiguration *sdk_config = nullptr;
   if (SUCCEEDED(D3D12GetInterface(CLSID_D3D12SDKConfiguration, IID_PPV_ARGS(&sdk_config)))) {
      ID3D12SDKConfiguration1 *sdk_config1 = nullptr;
      if (SUCCEEDED(sdk_config->QueryInterface(&sdk_config1))) {
         char self_path[MAX_PATH];
         const char *d3d12core_path = try_find_d3d12core_next_to_self(self_path, sizeof(self_path));
         if (d3d12core_path) {
            if (SUCCEEDED(sdk_config1->CreateDeviceFactory(D3D12_PREVIEW_SDK_VERSION, d3d12core_path, IID_PPV_ARGS(&factory))) ||
                SUCCEEDED(sdk_config1->CreateDeviceFactory(D3D12_SDK_VERSION, d3d12core_path, IID_PPV_ARGS(&factory)))) {
               sdk_config->Release();
               sdk_config1->Release();
               return factory;
            }
         }

         /* Nope, seems we don't have a matching D3D12Core.dll next to ourselves */
         sdk_config1->Release();
      }

      /* It's possible there's a D3D12Core.dll next to the .exe, for development/testing purposes. If so, we'll be notified
       * by environment variables what the relative path is and the version to use.
       */
      const char *d3d12core_relative_path = getenv("D3D12_AGILITY_RELATIVE_PATH");
      const char *d3d12core_sdk_version = getenv("D3D12_AGILITY_SDK_VERSION");
      if (d3d12core_relative_path && d3d12core_sdk_version) {
         (void)sdk_config->SetSDKVersion(atoi(d3d12core_sdk_version), d3d12core_relative_path);
      }
      sdk_config->Release();
   }
#endif

   (void)D3D12GetInterface(CLSID_D3D12DeviceFactory, IID_PPV_ARGS(&factory));
   return factory;
}
#endif

bool
d3d12_init_screen(struct d3d12_screen *screen, IUnknown *adapter)
{
   assert(screen->base.destroy != nullptr);

   // Device can be imported with d3d12_create_dxcore_screen_from_d3d12_device
   if (!screen->dev) {
#ifndef _GAMING_XBOX
      ID3D12DeviceFactory *factory = try_create_device_factory(screen->d3d12_mod);

#ifndef DEBUG
      if (d3d12_debug & D3D12_DEBUG_DEBUG_LAYER)
#endif
         enable_d3d12_debug_layer(screen->d3d12_mod, factory);

      if (d3d12_debug & D3D12_DEBUG_GPU_VALIDATOR)
         enable_gpu_validation(screen->d3d12_mod, factory);

      screen->dev = create_device(screen->d3d12_mod, adapter, factory);

      if (factory)
         factory->Release();
#else
      screen->dev = create_device(screen->d3d12_mod, adapter);
#endif

      if (!screen->dev) {
         debug_printf("D3D12: failed to create device\n");
         return false;
      }
   }
   screen->adapter_luid = GetAdapterLuid(screen->dev);

#ifndef _GAMING_XBOX
   ID3D12InfoQueue *info_queue;
   if (SUCCEEDED(screen->dev->QueryInterface(IID_PPV_ARGS(&info_queue)))) {
      D3D12_MESSAGE_SEVERITY severities[] = {
         D3D12_MESSAGE_SEVERITY_INFO,
         D3D12_MESSAGE_SEVERITY_WARNING,
      };

      D3D12_MESSAGE_ID msg_ids[] = {
         D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
      };

      D3D12_INFO_QUEUE_FILTER NewFilter = {};
      NewFilter.DenyList.NumSeverities = ARRAY_SIZE(severities);
      NewFilter.DenyList.pSeverityList = severities;
      NewFilter.DenyList.NumIDs = ARRAY_SIZE(msg_ids);
      NewFilter.DenyList.pIDList = msg_ids;

      info_queue->PushStorageFilter(&NewFilter);
      info_queue->Release();
   }
#endif /* !_GAMING_XBOX */

   if (FAILED(screen->dev->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS,
                                               &screen->opts,
                                               sizeof(screen->opts)))) {
      debug_printf("D3D12: failed to get device options\n");
      return false;
   }
   if (FAILED(screen->dev->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1,
                                               &screen->opts1,
                                               sizeof(screen->opts1)))) {
      debug_printf("D3D12: failed to get device options\n");
      return false;
   }
   if (FAILED(screen->dev->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS2,
                                               &screen->opts2,
                                               sizeof(screen->opts2)))) {
      debug_printf("D3D12: failed to get device options\n");
      return false;
   }
   if (FAILED(screen->dev->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3,
                                               &screen->opts3,
                                               sizeof(screen->opts3)))) {
      debug_printf("D3D12: failed to get device options\n");
      return false;
   }
   if (FAILED(screen->dev->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS4,
                                               &screen->opts4,
                                               sizeof(screen->opts4)))) {
      debug_printf("D3D12: failed to get device options\n");
      return false;
   }
   screen->dev->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &screen->opts12, sizeof(screen->opts12));
   screen->dev->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS14, &screen->opts14, sizeof(screen->opts14));
#ifndef _GAMING_XBOX
   screen->dev->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS19, &screen->opts19, sizeof(screen->opts19));
#endif

   screen->architecture.NodeIndex = 0;
   if (FAILED(screen->dev->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE,
                                               &screen->architecture,
                                               sizeof(screen->architecture)))) {
      debug_printf("D3D12: failed to get device architecture\n");
      return false;
   }

   D3D12_FEATURE_DATA_FEATURE_LEVELS feature_levels;
   static const D3D_FEATURE_LEVEL levels[] = {
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_11_1,
      D3D_FEATURE_LEVEL_12_0,
      D3D_FEATURE_LEVEL_12_1,
   };
   feature_levels.NumFeatureLevels = ARRAY_SIZE(levels);
   feature_levels.pFeatureLevelsRequested = levels;
   if (FAILED(screen->dev->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS,
                                               &feature_levels,
                                               sizeof(feature_levels)))) {
      debug_printf("D3D12: failed to get device feature levels\n");
      return false;
   }
   screen->max_feature_level = feature_levels.MaxSupportedFeatureLevel;

   static const D3D_SHADER_MODEL valid_shader_models[] = {
      D3D_SHADER_MODEL_6_7, D3D_SHADER_MODEL_6_6, D3D_SHADER_MODEL_6_5, D3D_SHADER_MODEL_6_4,
      D3D_SHADER_MODEL_6_3, D3D_SHADER_MODEL_6_2, D3D_SHADER_MODEL_6_1, D3D_SHADER_MODEL_6_0,
   };
   for (UINT i = 0; i < ARRAY_SIZE(valid_shader_models); ++i) {
      D3D12_FEATURE_DATA_SHADER_MODEL shader_model = { valid_shader_models[i] };
      if (SUCCEEDED(screen->dev->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shader_model, sizeof(shader_model)))) {
         static_assert(D3D_SHADER_MODEL_6_0 == 0x60 && SHADER_MODEL_6_0 == 0x60000, "Validating math below");
         static_assert(D3D_SHADER_MODEL_6_7 == 0x67 && SHADER_MODEL_6_7 == 0x60007, "Validating math below");
         screen->max_shader_model = static_cast<dxil_shader_model>(((shader_model.HighestShaderModel & 0xf0) << 12) |
                                                                   (shader_model.HighestShaderModel & 0xf));
         break;
      }
   }

   D3D12_COMMAND_QUEUE_DESC queue_desc;
   queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
   queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
   queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
   queue_desc.NodeMask = 0;

#ifndef _GAMING_XBOX
   ID3D12Device9 *device9;
   if (SUCCEEDED(screen->dev->QueryInterface(&device9))) {
      if (FAILED(device9->CreateCommandQueue1(&queue_desc, OpenGLOn12CreatorID,
                                              IID_PPV_ARGS(&screen->cmdqueue))))
         return false;
      device9->Release();
   } else
#endif
   {
      if (FAILED(screen->dev->CreateCommandQueue(&queue_desc,
                                                 IID_PPV_ARGS(&screen->cmdqueue))))
         return false;
   }

   if (FAILED(screen->dev->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&screen->fence))))
      return false;

   if (!d3d12_init_residency(screen))
      return false;

   UINT64 timestamp_freq;
   if (FAILED(screen->cmdqueue->GetTimestampFrequency(&timestamp_freq)))
       timestamp_freq = 10000000;
   screen->timestamp_multiplier = 1000000000.0f / timestamp_freq;

   d3d12_screen_fence_init(&screen->base);
   d3d12_screen_resource_init(&screen->base);
#ifdef HAVE_GALLIUM_D3D12_VIDEO
   d3d12_screen_video_init(&screen->base);
#endif

   struct pb_desc desc;
   desc.alignment = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;
   desc.usage = (pb_usage_flags)(PB_USAGE_CPU_WRITE | PB_USAGE_GPU_READ);

   screen->bufmgr = d3d12_bufmgr_create(screen);
   if (!screen->bufmgr)
      return false;

   screen->cache_bufmgr = pb_cache_manager_create(screen->bufmgr, 0xfffff, 2, 0, 512 * 1024 * 1024);
   if (!screen->cache_bufmgr)
      return false;

   screen->slab_cache_bufmgr = pb_cache_manager_create(screen->bufmgr, 0xfffff, 2, 0, 512 * 1024 * 1024);
   if (!screen->slab_cache_bufmgr)
      return false;

   screen->slab_bufmgr = pb_slab_range_manager_create(screen->slab_cache_bufmgr, 16,
                                                      D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
                                                      D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
                                                      &desc);
   if (!screen->slab_bufmgr)
      return false;
   
   screen->readback_slab_cache_bufmgr = pb_cache_manager_create(screen->bufmgr, 0xfffff, 2, 0, 512 * 1024 * 1024);
   if (!screen->readback_slab_cache_bufmgr)
      return false;

   desc.usage = (pb_usage_flags)(PB_USAGE_CPU_READ_WRITE | PB_USAGE_GPU_WRITE);
   screen->readback_slab_bufmgr = pb_slab_range_manager_create(screen->readback_slab_cache_bufmgr, 16,
                                                               D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
                                                               D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
                                                               &desc);
   if (!screen->readback_slab_bufmgr)
      return false;

   screen->rtv_pool = d3d12_descriptor_pool_new(screen,
                                                D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
                                                64);
   screen->dsv_pool = d3d12_descriptor_pool_new(screen,
                                                D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
                                                64);
   screen->view_pool = d3d12_descriptor_pool_new(screen,
                                                 D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                                                 1024);
   if (!screen->rtv_pool || !screen->dsv_pool || !screen->view_pool)
      return false;

   d3d12_init_null_srvs(screen);
   d3d12_init_null_uavs(screen);
   d3d12_init_null_rtv(screen);

   screen->have_load_at_vertex = can_attribute_at_vertex(screen);
   screen->support_shader_images = can_shader_image_load_all_formats(screen);

#ifndef _GAMING_XBOX
   ID3D12Device8 *dev8;
   if (SUCCEEDED(screen->dev->QueryInterface(&dev8))) {
      dev8->Release();
      screen->support_create_not_resident = true;
   }
   screen->dev->QueryInterface(&screen->dev10);
#endif

   static constexpr uint64_t known_good_warp_version = 10ull << 48 | 22000ull << 16;
   bool warp_with_broken_int64 =
      (screen->vendor_id == HW_VENDOR_MICROSOFT && screen->driver_version < known_good_warp_version);
   unsigned supported_int_sizes = 32 | (screen->opts1.Int64ShaderOps && !warp_with_broken_int64 ? 64 : 0);
   unsigned supported_float_sizes = 32 | (screen->opts.DoublePrecisionFloatShaderOps ? 64 : 0);
   dxil_get_nir_compiler_options(&screen->nir_options,
                                 screen->max_shader_model,
                                 supported_int_sizes,
                                 supported_float_sizes);

   const char *mesa_version = "Mesa " PACKAGE_VERSION MESA_GIT_SHA1;
   struct mesa_sha1 sha1_ctx;
   uint8_t sha1[SHA1_DIGEST_LENGTH];
   STATIC_ASSERT(PIPE_UUID_SIZE <= sizeof(sha1));

   /* The driver UUID is used for determining sharability of images and memory
    * between two instances in separate processes.  People who want to
    * share memory need to also check the device UUID or LUID so all this
    * needs to be is the build-id.
    */
   _mesa_sha1_compute(mesa_version, strlen(mesa_version), sha1);
   memcpy(screen->driver_uuid, sha1, PIPE_UUID_SIZE);

   /* The device UUID uniquely identifies the given device within the machine. */
   _mesa_sha1_init(&sha1_ctx);
   _mesa_sha1_update(&sha1_ctx, &screen->vendor_id, sizeof(screen->vendor_id));
   _mesa_sha1_update(&sha1_ctx, &screen->device_id, sizeof(screen->device_id));
   _mesa_sha1_update(&sha1_ctx, &screen->subsys_id, sizeof(screen->subsys_id));
   _mesa_sha1_update(&sha1_ctx, &screen->revision, sizeof(screen->revision));
   _mesa_sha1_final(&sha1_ctx, sha1);
   memcpy(screen->device_uuid, sha1, PIPE_UUID_SIZE);

   return true;
}
