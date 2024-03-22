/**********************************************************
 * Copyright 2008-2023 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/

#include "git_sha1.h" /* For MESA_GIT_SHA1 */
#include "compiler/nir/nir.h"
#include "util/format/u_format.h"
#include "util/u_memory.h"
#include "util/u_inlines.h"
#include "util/u_process.h"
#include "util/u_screen.h"
#include "util/u_string.h"
#include "util/u_math.h"

#include "svga_winsys.h"
#include "svga_public.h"
#include "svga_context.h"
#include "svga_format.h"
#include "svga_screen.h"
#include "svga_tgsi.h"
#include "svga_resource_texture.h"
#include "svga_resource.h"
#include "svga_debug.h"

#include "svga3d_shaderdefs.h"
#include "VGPU10ShaderTokens.h"

/* NOTE: this constant may get moved into a svga3d*.h header file */
#define SVGA3D_DX_MAX_RESOURCE_SIZE (128 * 1024 * 1024)

#ifndef MESA_GIT_SHA1
#define MESA_GIT_SHA1 "(unknown git revision)"
#endif

#ifdef DEBUG
int SVGA_DEBUG = 0;

static const struct debug_named_value svga_debug_flags[] = {
   { "dma",         DEBUG_DMA, NULL },
   { "tgsi",        DEBUG_TGSI, NULL },
   { "pipe",        DEBUG_PIPE, NULL },
   { "state",       DEBUG_STATE, NULL },
   { "screen",      DEBUG_SCREEN, NULL },
   { "tex",         DEBUG_TEX, NULL },
   { "swtnl",       DEBUG_SWTNL, NULL },
   { "const",       DEBUG_CONSTS, NULL },
   { "viewport",    DEBUG_VIEWPORT, NULL },
   { "views",       DEBUG_VIEWS, NULL },
   { "perf",        DEBUG_PERF, NULL },
   { "flush",       DEBUG_FLUSH, NULL },
   { "sync",        DEBUG_SYNC, NULL },
   { "cache",       DEBUG_CACHE, NULL },
   { "streamout",   DEBUG_STREAMOUT, NULL },
   { "query",       DEBUG_QUERY, NULL },
   { "samplers",    DEBUG_SAMPLERS, NULL },
   { "image",       DEBUG_IMAGE, NULL },
   { "uav",         DEBUG_UAV, NULL },
   { "retry",       DEBUG_RETRY, NULL },
   DEBUG_NAMED_VALUE_END
};
#endif

static const char *
svga_get_vendor( struct pipe_screen *pscreen )
{
   return "VMware, Inc.";
}


static const char *
svga_get_name( struct pipe_screen *pscreen )
{
   const char *build = "", *llvm = "", *mutex = "";
   static char name[100];
#ifdef DEBUG
   /* Only return internal details in the DEBUG version:
    */
   build = "build: DEBUG;";
   mutex = "mutex: " PIPE_ATOMIC ";";
#else
   build = "build: RELEASE;";
#endif
#if DRAW_LLVM_AVAILABLE
   llvm = "LLVM;";
#endif

   snprintf(name, sizeof(name), "SVGA3D; %s %s %s", build, mutex, llvm);
   return name;
}


/** Helper for querying float-valued device cap */
static float
get_float_cap(struct svga_winsys_screen *sws, SVGA3dDevCapIndex cap,
              float defaultVal)
{
   SVGA3dDevCapResult result;
   if (sws->get_cap(sws, cap, &result))
      return result.f;
   else
      return defaultVal;
}


/** Helper for querying uint-valued device cap */
static unsigned
get_uint_cap(struct svga_winsys_screen *sws, SVGA3dDevCapIndex cap,
             unsigned defaultVal)
{
   SVGA3dDevCapResult result;
   if (sws->get_cap(sws, cap, &result))
      return result.u;
   else
      return defaultVal;
}


/** Helper for querying boolean-valued device cap */
static bool
get_bool_cap(struct svga_winsys_screen *sws, SVGA3dDevCapIndex cap,
             bool defaultVal)
{
   SVGA3dDevCapResult result;
   if (sws->get_cap(sws, cap, &result))
      return result.b;
   else
      return defaultVal;
}


static float
svga_get_paramf(struct pipe_screen *screen, enum pipe_capf param)
{
   struct svga_screen *svgascreen = svga_screen(screen);
   struct svga_winsys_screen *sws = svgascreen->sws;

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
      return svgascreen->maxLineWidth;
   case PIPE_CAPF_MAX_LINE_WIDTH_AA:
      return svgascreen->maxLineWidthAA;

   case PIPE_CAPF_MAX_POINT_SIZE:
      FALLTHROUGH;
   case PIPE_CAPF_MAX_POINT_SIZE_AA:
      return svgascreen->maxPointSize;

   case PIPE_CAPF_MAX_TEXTURE_ANISOTROPY:
      return (float) get_uint_cap(sws, SVGA3D_DEVCAP_MAX_TEXTURE_ANISOTROPY, 4);

   case PIPE_CAPF_MAX_TEXTURE_LOD_BIAS:
      return 15.0;

   case PIPE_CAPF_MIN_CONSERVATIVE_RASTER_DILATE:
      FALLTHROUGH;
   case PIPE_CAPF_MAX_CONSERVATIVE_RASTER_DILATE:
      FALLTHROUGH;
   case PIPE_CAPF_CONSERVATIVE_RASTER_DILATE_GRANULARITY:
      return 0.0f;

   }

   debug_printf("Unexpected PIPE_CAPF_ query %u\n", param);
   return 0;
}


static int
svga_get_param(struct pipe_screen *screen, enum pipe_cap param)
{
   struct svga_screen *svgascreen = svga_screen(screen);
   struct svga_winsys_screen *sws = svgascreen->sws;
   SVGA3dDevCapResult result;

   switch (param) {
   case PIPE_CAP_NPOT_TEXTURES:
   case PIPE_CAP_MIXED_FRAMEBUFFER_SIZES:
   case PIPE_CAP_MIXED_COLOR_DEPTH_BITS:
      return 1;
   case PIPE_CAP_MAX_DUAL_SOURCE_RENDER_TARGETS:
      /*
       * "In virtually every OpenGL implementation and hardware,
       * GL_MAX_DUAL_SOURCE_DRAW_BUFFERS is 1"
       * http://www.opengl.org/wiki/Blending
       */
      return sws->have_vgpu10 ? 1 : 0;
   case PIPE_CAP_ANISOTROPIC_FILTER:
      return 1;
   case PIPE_CAP_MAX_RENDER_TARGETS:
      return svgascreen->max_color_buffers;
   case PIPE_CAP_OCCLUSION_QUERY:
      return 1;
   case PIPE_CAP_TEXTURE_BUFFER_OBJECTS:
      return sws->have_vgpu10;
   case PIPE_CAP_TEXTURE_BUFFER_OFFSET_ALIGNMENT:
      return sws->have_vgpu10 ? 16 : 0;

   case PIPE_CAP_TEXTURE_SWIZZLE:
      return 1;
   case PIPE_CAP_CONSTANT_BUFFER_OFFSET_ALIGNMENT:
      return 256;

   case PIPE_CAP_MAX_TEXTURE_2D_SIZE:
      {
         unsigned size = 1 << (SVGA_MAX_TEXTURE_LEVELS - 1);
         if (sws->get_cap(sws, SVGA3D_DEVCAP_MAX_TEXTURE_WIDTH, &result))
            size = MIN2(result.u, size);
         else
            size = 2048;
         if (sws->get_cap(sws, SVGA3D_DEVCAP_MAX_TEXTURE_HEIGHT, &result))
            size = MIN2(result.u, size);
         else
            size = 2048;
         return size;
      }

   case PIPE_CAP_MAX_TEXTURE_3D_LEVELS:
      if (!sws->get_cap(sws, SVGA3D_DEVCAP_MAX_VOLUME_EXTENT, &result))
         return 8;  /* max 128x128x128 */
      return MIN2(util_logbase2(result.u) + 1, SVGA_MAX_TEXTURE_LEVELS);

   case PIPE_CAP_MAX_TEXTURE_CUBE_LEVELS:
      return util_last_bit(screen->get_param(screen, PIPE_CAP_MAX_TEXTURE_2D_SIZE));

   case PIPE_CAP_MAX_TEXTURE_ARRAY_LAYERS:
      return sws->have_sm5 ? SVGA3D_SM5_MAX_SURFACE_ARRAYSIZE :
             (sws->have_vgpu10 ? SVGA3D_SM4_MAX_SURFACE_ARRAYSIZE : 0);

   case PIPE_CAP_BLEND_EQUATION_SEPARATE: /* req. for GL 1.5 */
      return 1;

   case PIPE_CAP_FS_COORD_ORIGIN_UPPER_LEFT:
      return 1;
   case PIPE_CAP_FS_COORD_PIXEL_CENTER_HALF_INTEGER:
      return sws->have_vgpu10;
   case PIPE_CAP_FS_COORD_PIXEL_CENTER_INTEGER:
      return !sws->have_vgpu10;

   case PIPE_CAP_VERTEX_COLOR_UNCLAMPED:
      return 1; /* The color outputs of vertex shaders are not clamped */
   case PIPE_CAP_VERTEX_COLOR_CLAMPED:
      return sws->have_vgpu10;

   case PIPE_CAP_GLSL_FEATURE_LEVEL:
   case PIPE_CAP_GLSL_FEATURE_LEVEL_COMPATIBILITY:
      if (sws->have_gl43) {
         return 430;
      } else if (sws->have_sm5) {
         return 410;
      } else if (sws->have_vgpu10) {
         return 330;
      } else {
         return 120;
      }

   case PIPE_CAP_TEXTURE_TRANSFER_MODES:
      return 0;

   case PIPE_CAP_FRAGMENT_SHADER_TEXTURE_LOD:
   case PIPE_CAP_FRAGMENT_SHADER_DERIVATIVES:
      return 1;

   case PIPE_CAP_DEPTH_CLIP_DISABLE:
   case PIPE_CAP_INDEP_BLEND_ENABLE:
   case PIPE_CAP_CONDITIONAL_RENDER:
   case PIPE_CAP_QUERY_TIMESTAMP:
   case PIPE_CAP_VS_INSTANCEID:
   case PIPE_CAP_VERTEX_ELEMENT_INSTANCE_DIVISOR:
   case PIPE_CAP_SEAMLESS_CUBE_MAP:
   case PIPE_CAP_FAKE_SW_MSAA:
      return sws->have_vgpu10;

   case PIPE_CAP_MAX_STREAM_OUTPUT_BUFFERS:
      return sws->have_vgpu10 ? SVGA3D_DX_MAX_SOTARGETS : 0;
   case PIPE_CAP_MAX_STREAM_OUTPUT_SEPARATE_COMPONENTS:
      return sws->have_vgpu10 ? 4 : 0;
   case PIPE_CAP_MAX_STREAM_OUTPUT_INTERLEAVED_COMPONENTS:
      return sws->have_sm5 ? SVGA3D_MAX_STREAMOUT_DECLS :
             (sws->have_vgpu10 ? SVGA3D_MAX_DX10_STREAMOUT_DECLS : 0);
   case PIPE_CAP_STREAM_OUTPUT_PAUSE_RESUME:
      return sws->have_sm5;
   case PIPE_CAP_STREAM_OUTPUT_INTERLEAVE_BUFFERS:
      return sws->have_sm5;
   case PIPE_CAP_TEXTURE_MULTISAMPLE:
      return svgascreen->ms_samples ? 1 : 0;

   case PIPE_CAP_MAX_TEXEL_BUFFER_ELEMENTS_UINT:
      /* convert bytes to texels for the case of the largest texel
       * size: float[4].
       */
      return SVGA3D_DX_MAX_RESOURCE_SIZE / (4 * sizeof(float));

   case PIPE_CAP_MIN_TEXEL_OFFSET:
      return sws->have_vgpu10 ? VGPU10_MIN_TEXEL_FETCH_OFFSET : 0;
   case PIPE_CAP_MAX_TEXEL_OFFSET:
      return sws->have_vgpu10 ? VGPU10_MAX_TEXEL_FETCH_OFFSET : 0;

   case PIPE_CAP_MIN_TEXTURE_GATHER_OFFSET:
   case PIPE_CAP_MAX_TEXTURE_GATHER_OFFSET:
      return 0;

   case PIPE_CAP_MAX_GEOMETRY_OUTPUT_VERTICES:
      return sws->have_vgpu10 ? 256 : 0;
   case PIPE_CAP_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS:
      return sws->have_vgpu10 ? 1024 : 0;

   case PIPE_CAP_PRIMITIVE_RESTART:
   case PIPE_CAP_PRIMITIVE_RESTART_FIXED_INDEX:
      return 1; /* may be a sw fallback, depending on restart index */

   case PIPE_CAP_GENERATE_MIPMAP:
      return sws->have_generate_mipmap_cmd;

   case PIPE_CAP_NATIVE_FENCE_FD:
      return sws->have_fence_fd;

   case PIPE_CAP_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION:
      return 1;

   case PIPE_CAP_CUBE_MAP_ARRAY:
   case PIPE_CAP_INDEP_BLEND_FUNC:
   case PIPE_CAP_SAMPLE_SHADING:
   case PIPE_CAP_FORCE_PERSAMPLE_INTERP:
   case PIPE_CAP_TEXTURE_QUERY_LOD:
      return sws->have_sm4_1;

   case PIPE_CAP_MAX_TEXTURE_GATHER_COMPONENTS:
      /* SM4_1 supports only single-channel textures where as SM5 supports
       * all four channel textures */
      return sws->have_sm5 ? 4 :
             (sws->have_sm4_1 ? 1 : 0);
   case PIPE_CAP_DRAW_INDIRECT:
      return sws->have_sm5;
   case PIPE_CAP_MAX_VERTEX_STREAMS:
      return sws->have_sm5 ? 4 : 0;
   case PIPE_CAP_COMPUTE:
      return sws->have_gl43;
   case PIPE_CAP_MAX_VARYINGS:
      /* According to the spec, max varyings does not include the components
       * for position, so remove one count from the max for position.
       */
      return sws->have_vgpu10 ? VGPU10_MAX_FS_INPUTS-1 : 10;
   case PIPE_CAP_BUFFER_MAP_PERSISTENT_COHERENT:
      return sws->have_coherent;

   case PIPE_CAP_START_INSTANCE:
      return sws->have_sm5;
   case PIPE_CAP_ROBUST_BUFFER_ACCESS_BEHAVIOR:
      return sws->have_sm5;

   case PIPE_CAP_SAMPLER_VIEW_TARGET:
      return sws->have_gl43;

   case PIPE_CAP_FRAMEBUFFER_NO_ATTACHMENT:
      return sws->have_gl43;

   case PIPE_CAP_CLIP_HALFZ:
      return sws->have_gl43;
   case PIPE_CAP_SHAREABLE_SHADERS:
      return 0;

   case PIPE_CAP_PCI_GROUP:
   case PIPE_CAP_PCI_BUS:
   case PIPE_CAP_PCI_DEVICE:
   case PIPE_CAP_PCI_FUNCTION:
      return 0;
   case PIPE_CAP_SHADER_BUFFER_OFFSET_ALIGNMENT:
      return sws->have_gl43 ? 16 : 0;

   case PIPE_CAP_MAX_COMBINED_SHADER_OUTPUT_RESOURCES:
   case PIPE_CAP_MAX_COMBINED_SHADER_BUFFERS:
      return sws->have_gl43 ? SVGA_MAX_SHADER_BUFFERS : 0;
   case PIPE_CAP_MAX_COMBINED_HW_ATOMIC_COUNTERS:
   case PIPE_CAP_MAX_COMBINED_HW_ATOMIC_COUNTER_BUFFERS:
      return sws->have_gl43 ? SVGA_MAX_ATOMIC_BUFFERS : 0;
   case PIPE_CAP_MIN_MAP_BUFFER_ALIGNMENT:
      return 64;
   case PIPE_CAP_VERTEX_BUFFER_STRIDE_4BYTE_ALIGNED_ONLY:
      return sws->have_vgpu10 ? 0 : 1;
   case PIPE_CAP_VERTEX_ATTRIB_ELEMENT_ALIGNED_ONLY:
      /* This CAP cannot be used with any other alignment-requiring CAPs */
      return sws->have_vgpu10 ? 1 : 0;
   case PIPE_CAP_VERTEX_BUFFER_OFFSET_4BYTE_ALIGNED_ONLY:
   case PIPE_CAP_VERTEX_ELEMENT_SRC_OFFSET_4BYTE_ALIGNED_ONLY:
      return sws->have_vgpu10 ? 0 : 1;
   case PIPE_CAP_MAX_VERTEX_ATTRIB_STRIDE:
      return 2048;
   case PIPE_CAP_MAX_VIEWPORTS:
      assert((!sws->have_vgpu10 && svgascreen->max_viewports == 1) ||
             (sws->have_vgpu10 &&
              svgascreen->max_viewports == SVGA3D_DX_MAX_VIEWPORTS));
      return svgascreen->max_viewports;
   case PIPE_CAP_ENDIANNESS:
      return PIPE_ENDIAN_LITTLE;

   case PIPE_CAP_VENDOR_ID:
      return 0x15ad; /* VMware Inc. */
   case PIPE_CAP_DEVICE_ID:
      if (sws->device_id) {
         return sws->device_id;
      } else {
         return 0x0405; /* assume SVGA II */
      }
   case PIPE_CAP_ACCELERATED:
      return 0; /* XXX: */
   case PIPE_CAP_VIDEO_MEMORY:
      /* XXX: Query the host ? */
      return 1;
   case PIPE_CAP_COPY_BETWEEN_COMPRESSED_AND_PLAIN_FORMATS:
      return sws->have_vgpu10;
   case PIPE_CAP_DOUBLES:
      return sws->have_sm5;
   case PIPE_CAP_UMA:
   case PIPE_CAP_ALLOW_MAPPED_BUFFERS_DURING_EXECUTION:
      return 0;
   case PIPE_CAP_TGSI_DIV:
      return 1;
   case PIPE_CAP_MAX_GS_INVOCATIONS:
      return 32;
   case PIPE_CAP_MAX_SHADER_BUFFER_SIZE_UINT:
      return 1 << 27;
   /* Verify this once protocol is finalized. Setting it to minimum value. */
   case PIPE_CAP_MAX_SHADER_PATCH_VARYINGS:
      return sws->have_sm5 ? 30 : 0;
   case PIPE_CAP_TEXTURE_FLOAT_LINEAR:
      return 1;
   case PIPE_CAP_TEXTURE_HALF_FLOAT_LINEAR:
      return 1;
   case PIPE_CAP_TGSI_TEXCOORD:
      return sws->have_vgpu10 ? 1 : 0;
   case PIPE_CAP_IMAGE_STORE_FORMATTED:
      return sws->have_gl43;
   default:
      return u_pipe_screen_get_param_defaults(screen, param);
   }
}


static int
vgpu9_get_shader_param(struct pipe_screen *screen,
                       enum pipe_shader_type shader,
                       enum pipe_shader_cap param)
{
   struct svga_screen *svgascreen = svga_screen(screen);
   struct svga_winsys_screen *sws = svgascreen->sws;
   unsigned val;

   assert(!sws->have_vgpu10);

   switch (shader)
   {
   case PIPE_SHADER_FRAGMENT:
      switch (param)
      {
      case PIPE_SHADER_CAP_MAX_INSTRUCTIONS:
      case PIPE_SHADER_CAP_MAX_ALU_INSTRUCTIONS:
         return get_uint_cap(sws,
                             SVGA3D_DEVCAP_MAX_FRAGMENT_SHADER_INSTRUCTIONS,
                             512);
      case PIPE_SHADER_CAP_MAX_TEX_INSTRUCTIONS:
      case PIPE_SHADER_CAP_MAX_TEX_INDIRECTIONS:
         return 512;
      case PIPE_SHADER_CAP_MAX_CONTROL_FLOW_DEPTH:
         return SVGA3D_MAX_NESTING_LEVEL;
      case PIPE_SHADER_CAP_MAX_INPUTS:
         return 10;
      case PIPE_SHADER_CAP_MAX_OUTPUTS:
         return svgascreen->max_color_buffers;
      case PIPE_SHADER_CAP_MAX_CONST_BUFFER0_SIZE:
         return 224 * sizeof(float[4]);
      case PIPE_SHADER_CAP_MAX_CONST_BUFFERS:
         return 1;
      case PIPE_SHADER_CAP_MAX_TEMPS:
         val = get_uint_cap(sws, SVGA3D_DEVCAP_MAX_FRAGMENT_SHADER_TEMPS, 32);
         return MIN2(val, SVGA3D_TEMPREG_MAX);
      case PIPE_SHADER_CAP_INDIRECT_INPUT_ADDR:
         /*
          * Although PS 3.0 has some addressing abilities it can only represent
          * loops that can be statically determined and unrolled. Given we can
          * only handle a subset of the cases that the gallium frontend already
          * does it is better to defer loop unrolling to the gallium frontend.
          */
         return 0;
      case PIPE_SHADER_CAP_CONT_SUPPORTED:
         return 0;
      case PIPE_SHADER_CAP_TGSI_SQRT_SUPPORTED:
         return 0;
      case PIPE_SHADER_CAP_INDIRECT_OUTPUT_ADDR:
      case PIPE_SHADER_CAP_INDIRECT_TEMP_ADDR:
      case PIPE_SHADER_CAP_INDIRECT_CONST_ADDR:
         return 0;
      case PIPE_SHADER_CAP_SUBROUTINES:
         return 0;
      case PIPE_SHADER_CAP_INT64_ATOMICS:
      case PIPE_SHADER_CAP_INTEGERS:
         return 0;
      case PIPE_SHADER_CAP_FP16:
      case PIPE_SHADER_CAP_FP16_DERIVATIVES:
      case PIPE_SHADER_CAP_FP16_CONST_BUFFERS:
      case PIPE_SHADER_CAP_INT16:
      case PIPE_SHADER_CAP_GLSL_16BIT_CONSTS:
         return 0;
      case PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS:
      case PIPE_SHADER_CAP_MAX_SAMPLER_VIEWS:
         return 16;
      case PIPE_SHADER_CAP_SUPPORTED_IRS:
         return (1 << PIPE_SHADER_IR_TGSI) | (1 << PIPE_SHADER_IR_NIR);
      case PIPE_SHADER_CAP_TGSI_ANY_INOUT_DECL_RANGE:
      case PIPE_SHADER_CAP_MAX_SHADER_BUFFERS:
      case PIPE_SHADER_CAP_MAX_SHADER_IMAGES:
      case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTERS:
      case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTER_BUFFERS:
         return 0;
      }
      /* If we get here, we failed to handle a cap above */
      debug_printf("Unexpected fragment shader query %u\n", param);
      return 0;
   case PIPE_SHADER_VERTEX:
      switch (param)
      {
      case PIPE_SHADER_CAP_MAX_INSTRUCTIONS:
      case PIPE_SHADER_CAP_MAX_ALU_INSTRUCTIONS:
         return get_uint_cap(sws, SVGA3D_DEVCAP_MAX_VERTEX_SHADER_INSTRUCTIONS,
                             512);
      case PIPE_SHADER_CAP_MAX_TEX_INSTRUCTIONS:
      case PIPE_SHADER_CAP_MAX_TEX_INDIRECTIONS:
         /* XXX: until we have vertex texture support */
         return 0;
      case PIPE_SHADER_CAP_MAX_CONTROL_FLOW_DEPTH:
         return SVGA3D_MAX_NESTING_LEVEL;
      case PIPE_SHADER_CAP_MAX_INPUTS:
         return 16;
      case PIPE_SHADER_CAP_MAX_OUTPUTS:
         return 10;
      case PIPE_SHADER_CAP_MAX_CONST_BUFFER0_SIZE:
         return 256 * sizeof(float[4]);
      case PIPE_SHADER_CAP_MAX_CONST_BUFFERS:
         return 1;
      case PIPE_SHADER_CAP_MAX_TEMPS:
         val = get_uint_cap(sws, SVGA3D_DEVCAP_MAX_VERTEX_SHADER_TEMPS, 32);
         return MIN2(val, SVGA3D_TEMPREG_MAX);
      case PIPE_SHADER_CAP_CONT_SUPPORTED:
         return 0;
      case PIPE_SHADER_CAP_TGSI_SQRT_SUPPORTED:
         return 0;
      case PIPE_SHADER_CAP_INDIRECT_INPUT_ADDR:
      case PIPE_SHADER_CAP_INDIRECT_OUTPUT_ADDR:
         return 1;
      case PIPE_SHADER_CAP_INDIRECT_TEMP_ADDR:
         return 0;
      case PIPE_SHADER_CAP_INDIRECT_CONST_ADDR:
         return 1;
      case PIPE_SHADER_CAP_SUBROUTINES:
         return 0;
      case PIPE_SHADER_CAP_INT64_ATOMICS:
      case PIPE_SHADER_CAP_INTEGERS:
         return 0;
      case PIPE_SHADER_CAP_FP16:
      case PIPE_SHADER_CAP_FP16_DERIVATIVES:
      case PIPE_SHADER_CAP_FP16_CONST_BUFFERS:
      case PIPE_SHADER_CAP_INT16:
      case PIPE_SHADER_CAP_GLSL_16BIT_CONSTS:
         return 0;
      case PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS:
      case PIPE_SHADER_CAP_MAX_SAMPLER_VIEWS:
         return 0;
      case PIPE_SHADER_CAP_SUPPORTED_IRS:
         return (1 << PIPE_SHADER_IR_TGSI) | (1 << PIPE_SHADER_IR_NIR);
      case PIPE_SHADER_CAP_TGSI_ANY_INOUT_DECL_RANGE:
      case PIPE_SHADER_CAP_MAX_SHADER_BUFFERS:
      case PIPE_SHADER_CAP_MAX_SHADER_IMAGES:
      case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTERS:
      case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTER_BUFFERS:
         return 0;
      }
      /* If we get here, we failed to handle a cap above */
      debug_printf("Unexpected vertex shader query %u\n", param);
      return 0;
   case PIPE_SHADER_GEOMETRY:
   case PIPE_SHADER_COMPUTE:
   case PIPE_SHADER_TESS_CTRL:
   case PIPE_SHADER_TESS_EVAL:
      /* no support for geometry, tess or compute shaders at this time */
      return 0;
   case PIPE_SHADER_MESH:
   case PIPE_SHADER_TASK:
      return 0;
   default:
      debug_printf("Unexpected shader type (%u) query\n", shader);
      return 0;
   }
   return 0;
}


static int
vgpu10_get_shader_param(struct pipe_screen *screen,
                        enum pipe_shader_type shader,
                        enum pipe_shader_cap param)
{
   struct svga_screen *svgascreen = svga_screen(screen);
   struct svga_winsys_screen *sws = svgascreen->sws;

   assert(sws->have_vgpu10);
   (void) sws;  /* silence unused var warnings in non-debug builds */

   if (shader == PIPE_SHADER_MESH || shader == PIPE_SHADER_TASK)
      return 0;

   if ((!sws->have_sm5) &&
       (shader == PIPE_SHADER_TESS_CTRL || shader == PIPE_SHADER_TESS_EVAL))
      return 0;

   if ((!sws->have_gl43) && (shader == PIPE_SHADER_COMPUTE))
      return 0;

   /* NOTE: we do not query the device for any caps/limits at this time */

   /* Generally the same limits for vertex, geometry and fragment shaders */
   switch (param) {
   case PIPE_SHADER_CAP_MAX_INSTRUCTIONS:
   case PIPE_SHADER_CAP_MAX_ALU_INSTRUCTIONS:
   case PIPE_SHADER_CAP_MAX_TEX_INSTRUCTIONS:
   case PIPE_SHADER_CAP_MAX_TEX_INDIRECTIONS:
      return 64 * 1024;
   case PIPE_SHADER_CAP_MAX_CONTROL_FLOW_DEPTH:
      return 64;
   case PIPE_SHADER_CAP_MAX_INPUTS:
      if (shader == PIPE_SHADER_FRAGMENT)
         return VGPU10_MAX_FS_INPUTS;
      else if (shader == PIPE_SHADER_GEOMETRY)
         return svgascreen->max_gs_inputs;
      else if (shader == PIPE_SHADER_TESS_CTRL)
         return VGPU11_MAX_HS_INPUT_CONTROL_POINTS;
      else if (shader == PIPE_SHADER_TESS_EVAL)
         return VGPU11_MAX_DS_INPUT_CONTROL_POINTS;
      else
         return svgascreen->max_vs_inputs;
   case PIPE_SHADER_CAP_MAX_OUTPUTS:
      if (shader == PIPE_SHADER_FRAGMENT)
         return VGPU10_MAX_FS_OUTPUTS;
      else if (shader == PIPE_SHADER_GEOMETRY)
         return VGPU10_MAX_GS_OUTPUTS;
      else if (shader == PIPE_SHADER_TESS_CTRL)
         return VGPU11_MAX_HS_OUTPUTS;
      else if (shader == PIPE_SHADER_TESS_EVAL)
         return VGPU11_MAX_DS_OUTPUTS;
      else
         return svgascreen->max_vs_outputs;

   case PIPE_SHADER_CAP_MAX_CONST_BUFFER0_SIZE:
      return VGPU10_MAX_CONSTANT_BUFFER_ELEMENT_COUNT * sizeof(float[4]);
   case PIPE_SHADER_CAP_MAX_CONST_BUFFERS:
      return svgascreen->max_const_buffers;
   case PIPE_SHADER_CAP_MAX_TEMPS:
      return VGPU10_MAX_TEMPS;
   case PIPE_SHADER_CAP_INDIRECT_INPUT_ADDR:
   case PIPE_SHADER_CAP_INDIRECT_OUTPUT_ADDR:
   case PIPE_SHADER_CAP_INDIRECT_TEMP_ADDR:
   case PIPE_SHADER_CAP_INDIRECT_CONST_ADDR:
      return true; /* XXX verify */
   case PIPE_SHADER_CAP_CONT_SUPPORTED:
   case PIPE_SHADER_CAP_TGSI_SQRT_SUPPORTED:
   case PIPE_SHADER_CAP_SUBROUTINES:
   case PIPE_SHADER_CAP_INTEGERS:
      return true;
   case PIPE_SHADER_CAP_FP16:
   case PIPE_SHADER_CAP_FP16_DERIVATIVES:
   case PIPE_SHADER_CAP_FP16_CONST_BUFFERS:
   case PIPE_SHADER_CAP_INT16:
   case PIPE_SHADER_CAP_GLSL_16BIT_CONSTS:
      return false;
   case PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS:
   case PIPE_SHADER_CAP_MAX_SAMPLER_VIEWS:
      return sws->have_gl43 ? PIPE_MAX_SAMPLERS : SVGA3D_DX_MAX_SAMPLERS;
   case PIPE_SHADER_CAP_SUPPORTED_IRS:
      if (sws->have_gl43)
         return (1 << PIPE_SHADER_IR_TGSI) | (1 << PIPE_SHADER_IR_NIR);
      else
         return 0;

   case PIPE_SHADER_CAP_MAX_SHADER_IMAGES:
      return sws->have_gl43 ? SVGA_MAX_IMAGES : 0;

   case PIPE_SHADER_CAP_MAX_SHADER_BUFFERS:
      return sws->have_gl43 ? SVGA_MAX_SHADER_BUFFERS : 0;

   case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTERS:
   case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTER_BUFFERS:
      return sws->have_gl43 ? SVGA_MAX_ATOMIC_BUFFERS : 0;

   case PIPE_SHADER_CAP_TGSI_ANY_INOUT_DECL_RANGE:
   case PIPE_SHADER_CAP_INT64_ATOMICS:
      return 0;
   default:
      debug_printf("Unexpected vgpu10 shader query %u\n", param);
      return 0;
   }
   return 0;
}

#define COMMON_OPTIONS                                                        \
   .lower_extract_byte = true,                                                \
   .lower_extract_word = true,                                                \
   .lower_insert_byte = true,                                                 \
   .lower_insert_word = true,                                                 \
   .lower_int64_options = nir_lower_imul_2x32_64 | nir_lower_divmod64,        \
   .lower_fdph = true,                                                        \
   .lower_flrp64 = true,                                                      \
   .lower_ldexp = true,                                                       \
   .lower_uniforms_to_ubo = true,                                             \
   .lower_vector_cmp = true,                                                  \
   .lower_cs_local_index_to_id = true,                                        \
   .max_unroll_iterations = 32,                                               \
   .use_interpolated_input_intrinsics = true

#define VGPU10_OPTIONS                                                        \
   .lower_doubles_options = nir_lower_dfloor | nir_lower_dsign | nir_lower_dceil | nir_lower_dtrunc | nir_lower_dround_even, \
   .lower_fmod = true,                                                        \
   .lower_fpow = true

static const nir_shader_compiler_options svga_vgpu9_fragment_compiler_options = {
   COMMON_OPTIONS,
   .lower_bitops = true,
   .force_indirect_unrolling = nir_var_all,
   .force_indirect_unrolling_sampler = true,
};

static const nir_shader_compiler_options svga_vgpu9_vertex_compiler_options = {
   COMMON_OPTIONS,
   .lower_bitops = true,
   .force_indirect_unrolling = nir_var_function_temp,
   .force_indirect_unrolling_sampler = true,
};

static const nir_shader_compiler_options svga_vgpu10_compiler_options = {
   COMMON_OPTIONS,
   VGPU10_OPTIONS,
   .force_indirect_unrolling_sampler = true,
};

static const nir_shader_compiler_options svga_gl4_compiler_options = {
   COMMON_OPTIONS,
   VGPU10_OPTIONS,
};

static const void *
svga_get_compiler_options(struct pipe_screen *pscreen,
                          enum pipe_shader_ir ir,
                          enum pipe_shader_type shader)
{
   struct svga_screen *svgascreen = svga_screen(pscreen);
   struct svga_winsys_screen *sws = svgascreen->sws;

   assert(ir == PIPE_SHADER_IR_NIR);

   if (sws->have_gl43 || sws->have_sm5)
      return &svga_gl4_compiler_options;
   else if (sws->have_vgpu10)
      return &svga_vgpu10_compiler_options;
   else {
      if (shader == PIPE_SHADER_FRAGMENT)
         return &svga_vgpu9_fragment_compiler_options;
      else
         return &svga_vgpu9_vertex_compiler_options;
   }
}

static int
svga_get_shader_param(struct pipe_screen *screen, enum pipe_shader_type shader,
                      enum pipe_shader_cap param)
{
   struct svga_screen *svgascreen = svga_screen(screen);
   struct svga_winsys_screen *sws = svgascreen->sws;
   if (sws->have_vgpu10) {
      return vgpu10_get_shader_param(screen, shader, param);
   }
   else {
      return vgpu9_get_shader_param(screen, shader, param);
   }
}


static int
svga_sm5_get_compute_param(struct pipe_screen *screen,
                           enum pipe_shader_ir ir_type,
                           enum pipe_compute_cap param,
                           void *ret)
{
   ASSERTED struct svga_screen *svgascreen = svga_screen(screen);
   ASSERTED struct svga_winsys_screen *sws = svgascreen->sws;
   uint64_t *iret = (uint64_t *)ret;

   assert(sws->have_gl43);

   switch (param) {
   case PIPE_COMPUTE_CAP_MAX_GRID_SIZE:
      iret[0] = 65535;
      iret[1] = 65535;
      iret[2] = 65535;
      return 3 * sizeof(uint64_t);
   case PIPE_COMPUTE_CAP_MAX_BLOCK_SIZE:
      iret[0] = 1024;
      iret[1] = 1024;
      iret[2] = 64;
      return 3 * sizeof(uint64_t);
   case PIPE_COMPUTE_CAP_MAX_THREADS_PER_BLOCK:
      *iret = 1024;
      return sizeof(uint64_t);
   case PIPE_COMPUTE_CAP_MAX_LOCAL_SIZE:
      *iret = 32768;
      return sizeof(uint64_t);
   case PIPE_COMPUTE_CAP_MAX_VARIABLE_THREADS_PER_BLOCK:
      *iret = 0;
      return sizeof(uint64_t);
   default:
      debug_printf("Unexpected compute param %u\n", param);
   }
   return 0;
}

static void
svga_fence_reference(struct pipe_screen *screen,
                     struct pipe_fence_handle **ptr,
                     struct pipe_fence_handle *fence)
{
   struct svga_winsys_screen *sws = svga_screen(screen)->sws;
   sws->fence_reference(sws, ptr, fence);
}


static bool
svga_fence_finish(struct pipe_screen *screen,
                  struct pipe_context *ctx,
                  struct pipe_fence_handle *fence,
                  uint64_t timeout)
{
   struct svga_winsys_screen *sws = svga_screen(screen)->sws;
   bool retVal;

   SVGA_STATS_TIME_PUSH(sws, SVGA_STATS_TIME_FENCEFINISH);

   if (!timeout) {
      retVal = sws->fence_signalled(sws, fence, 0) == 0;
   }
   else {
      SVGA_DBG(DEBUG_DMA|DEBUG_PERF, "%s fence_ptr %p\n",
               __func__, fence);

      retVal = sws->fence_finish(sws, fence, timeout, 0) == 0;
   }

   SVGA_STATS_TIME_POP(sws);

   return retVal;
}


static int
svga_fence_get_fd(struct pipe_screen *screen,
                  struct pipe_fence_handle *fence)
{
   struct svga_winsys_screen *sws = svga_screen(screen)->sws;

   return sws->fence_get_fd(sws, fence, true);
}


static int
svga_get_driver_query_info(struct pipe_screen *screen,
                           unsigned index,
                           struct pipe_driver_query_info *info)
{
#define QUERY(NAME, ENUM, UNITS) \
   {NAME, ENUM, {0}, UNITS, PIPE_DRIVER_QUERY_RESULT_TYPE_AVERAGE, 0, 0x0}

   static const struct pipe_driver_query_info queries[] = {
      /* per-frame counters */
      QUERY("num-draw-calls", SVGA_QUERY_NUM_DRAW_CALLS,
            PIPE_DRIVER_QUERY_TYPE_UINT64),
      QUERY("num-fallbacks", SVGA_QUERY_NUM_FALLBACKS,
            PIPE_DRIVER_QUERY_TYPE_UINT64),
      QUERY("num-flushes", SVGA_QUERY_NUM_FLUSHES,
            PIPE_DRIVER_QUERY_TYPE_UINT64),
      QUERY("num-validations", SVGA_QUERY_NUM_VALIDATIONS,
            PIPE_DRIVER_QUERY_TYPE_UINT64),
      QUERY("map-buffer-time", SVGA_QUERY_MAP_BUFFER_TIME,
            PIPE_DRIVER_QUERY_TYPE_MICROSECONDS),
      QUERY("num-buffers-mapped", SVGA_QUERY_NUM_BUFFERS_MAPPED,
            PIPE_DRIVER_QUERY_TYPE_UINT64),
      QUERY("num-textures-mapped", SVGA_QUERY_NUM_TEXTURES_MAPPED,
            PIPE_DRIVER_QUERY_TYPE_UINT64),
      QUERY("num-bytes-uploaded", SVGA_QUERY_NUM_BYTES_UPLOADED,
            PIPE_DRIVER_QUERY_TYPE_BYTES),
      QUERY("num-command-buffers", SVGA_QUERY_NUM_COMMAND_BUFFERS,
            PIPE_DRIVER_QUERY_TYPE_UINT64),
      QUERY("command-buffer-size", SVGA_QUERY_COMMAND_BUFFER_SIZE,
            PIPE_DRIVER_QUERY_TYPE_BYTES),
      QUERY("flush-time", SVGA_QUERY_FLUSH_TIME,
            PIPE_DRIVER_QUERY_TYPE_MICROSECONDS),
      QUERY("surface-write-flushes", SVGA_QUERY_SURFACE_WRITE_FLUSHES,
            PIPE_DRIVER_QUERY_TYPE_UINT64),
      QUERY("num-readbacks", SVGA_QUERY_NUM_READBACKS,
            PIPE_DRIVER_QUERY_TYPE_UINT64),
      QUERY("num-resource-updates", SVGA_QUERY_NUM_RESOURCE_UPDATES,
            PIPE_DRIVER_QUERY_TYPE_UINT64),
      QUERY("num-buffer-uploads", SVGA_QUERY_NUM_BUFFER_UPLOADS,
            PIPE_DRIVER_QUERY_TYPE_UINT64),
      QUERY("num-const-buf-updates", SVGA_QUERY_NUM_CONST_BUF_UPDATES,
            PIPE_DRIVER_QUERY_TYPE_UINT64),
      QUERY("num-const-updates", SVGA_QUERY_NUM_CONST_UPDATES,
            PIPE_DRIVER_QUERY_TYPE_UINT64),
      QUERY("num-shader-relocations", SVGA_QUERY_NUM_SHADER_RELOCATIONS,
            PIPE_DRIVER_QUERY_TYPE_UINT64),
      QUERY("num-surface-relocations", SVGA_QUERY_NUM_SURFACE_RELOCATIONS,
            PIPE_DRIVER_QUERY_TYPE_UINT64),

      /* running total counters */
      QUERY("memory-used", SVGA_QUERY_MEMORY_USED,
            PIPE_DRIVER_QUERY_TYPE_BYTES),
      QUERY("num-shaders", SVGA_QUERY_NUM_SHADERS,
            PIPE_DRIVER_QUERY_TYPE_UINT64),
      QUERY("num-resources", SVGA_QUERY_NUM_RESOURCES,
            PIPE_DRIVER_QUERY_TYPE_UINT64),
      QUERY("num-state-objects", SVGA_QUERY_NUM_STATE_OBJECTS,
            PIPE_DRIVER_QUERY_TYPE_UINT64),
      QUERY("num-surface-views", SVGA_QUERY_NUM_SURFACE_VIEWS,
            PIPE_DRIVER_QUERY_TYPE_UINT64),
      QUERY("num-generate-mipmap", SVGA_QUERY_NUM_GENERATE_MIPMAP,
            PIPE_DRIVER_QUERY_TYPE_UINT64),
      QUERY("num-failed-allocations", SVGA_QUERY_NUM_FAILED_ALLOCATIONS,
            PIPE_DRIVER_QUERY_TYPE_UINT64),
      QUERY("num-commands-per-draw", SVGA_QUERY_NUM_COMMANDS_PER_DRAW,
            PIPE_DRIVER_QUERY_TYPE_FLOAT),
      QUERY("shader-mem-used", SVGA_QUERY_SHADER_MEM_USED,
            PIPE_DRIVER_QUERY_TYPE_UINT64),
   };
#undef QUERY

   if (!info)
      return ARRAY_SIZE(queries);

   if (index >= ARRAY_SIZE(queries))
      return 0;

   *info = queries[index];
   return 1;
}


static void
init_logging(struct pipe_screen *screen)
{
   struct svga_screen *svgascreen = svga_screen(screen);
   static const char *log_prefix = "Mesa: ";
   char host_log[1000];

   /* Log Version to Host */
   snprintf(host_log, sizeof(host_log) - strlen(log_prefix),
            "%s%s\n", log_prefix, svga_get_name(screen));
   svgascreen->sws->host_log(svgascreen->sws, host_log);

   snprintf(host_log, sizeof(host_log) - strlen(log_prefix),
            "%s" PACKAGE_VERSION MESA_GIT_SHA1, log_prefix);
   svgascreen->sws->host_log(svgascreen->sws, host_log);

   /* If the SVGA_EXTRA_LOGGING env var is set, log the process's command
    * line (program name and arguments).
    */
   if (debug_get_bool_option("SVGA_EXTRA_LOGGING", false)) {
      char cmdline[1000];
      if (util_get_command_line(cmdline, sizeof(cmdline))) {
         snprintf(host_log, sizeof(host_log) - strlen(log_prefix),
                  "%s%s\n", log_prefix, cmdline);
         svgascreen->sws->host_log(svgascreen->sws, host_log);
      }
   }
}


/**
 * no-op logging function to use when SVGA_NO_LOGGING is set.
 */
static void
nop_host_log(struct svga_winsys_screen *sws, const char *message)
{
   /* nothing */
}


static void
svga_destroy_screen( struct pipe_screen *screen )
{
   struct svga_screen *svgascreen = svga_screen(screen);

   svga_screen_cache_cleanup(svgascreen);

   mtx_destroy(&svgascreen->swc_mutex);
   mtx_destroy(&svgascreen->tex_mutex);

   svgascreen->sws->destroy(svgascreen->sws);

   FREE(svgascreen);
}


static int
svga_screen_get_fd( struct pipe_screen *screen )
{
   struct svga_winsys_screen *sws = svga_screen(screen)->sws;

   return sws->get_fd(sws);
}


/**
 * Create a new svga_screen object
 */
struct pipe_screen *
svga_screen_create(struct svga_winsys_screen *sws)
{
   struct svga_screen *svgascreen;
   struct pipe_screen *screen;

#ifdef DEBUG
   SVGA_DEBUG = debug_get_flags_option("SVGA_DEBUG", svga_debug_flags, 0 );
#endif

   svgascreen = CALLOC_STRUCT(svga_screen);
   if (!svgascreen)
      goto error1;

   svgascreen->debug.force_level_surface_view =
      debug_get_bool_option("SVGA_FORCE_LEVEL_SURFACE_VIEW", false);
   svgascreen->debug.force_surface_view =
      debug_get_bool_option("SVGA_FORCE_SURFACE_VIEW", false);
   svgascreen->debug.force_sampler_view =
      debug_get_bool_option("SVGA_FORCE_SAMPLER_VIEW", false);
   svgascreen->debug.no_surface_view =
      debug_get_bool_option("SVGA_NO_SURFACE_VIEW", false);
   svgascreen->debug.no_sampler_view =
      debug_get_bool_option("SVGA_NO_SAMPLER_VIEW", false);
   svgascreen->debug.no_cache_index_buffers =
      debug_get_bool_option("SVGA_NO_CACHE_INDEX_BUFFERS", false);

   screen = &svgascreen->screen;

   screen->destroy = svga_destroy_screen;
   screen->get_name = svga_get_name;
   screen->get_vendor = svga_get_vendor;
   screen->get_device_vendor = svga_get_vendor; // TODO actual device vendor
   screen->get_screen_fd = svga_screen_get_fd;
   screen->get_param = svga_get_param;
   screen->get_shader_param = svga_get_shader_param;
   screen->get_compiler_options = svga_get_compiler_options;
   screen->get_paramf = svga_get_paramf;
   screen->get_timestamp = NULL;
   screen->is_format_supported = svga_is_format_supported;
   screen->context_create = svga_context_create;
   screen->fence_reference = svga_fence_reference;
   screen->fence_finish = svga_fence_finish;
   screen->fence_get_fd = svga_fence_get_fd;

   screen->get_driver_query_info = svga_get_driver_query_info;

   screen->get_compute_param = svga_sm5_get_compute_param;

   svgascreen->sws = sws;

   svga_init_screen_resource_functions(svgascreen);

   if (sws->get_hw_version) {
      svgascreen->hw_version = sws->get_hw_version(sws);
   } else {
      svgascreen->hw_version = SVGA3D_HWVERSION_WS65_B1;
   }

   if (svgascreen->hw_version < SVGA3D_HWVERSION_WS8_B1) {
      /* too old for 3D acceleration */
      debug_printf("Hardware version 0x%x is too old for accerated 3D\n",
                   svgascreen->hw_version);
      goto error2;
   }

   if (sws->have_gl43) {
      svgascreen->forcedSampleCount =
         get_uint_cap(sws, SVGA3D_DEVCAP_MAX_FORCED_SAMPLE_COUNT, 0);

      sws->have_gl43 = sws->have_gl43 && (svgascreen->forcedSampleCount >= 4);

      /* Allow a temporary environment variable to enable/disable GL43 support.
       */
      sws->have_gl43 =
         debug_get_bool_option("SVGA_GL43", sws->have_gl43);

      svgascreen->debug.sampler_state_mapping =
         debug_get_bool_option("SVGA_SAMPLER_STATE_MAPPING", false);
   }
   else {
      /* sampler state mapping code is only enabled with GL43
       * due to the limitation in SW Renderer. (VMware bug 2825014)
       */
      svgascreen->debug.sampler_state_mapping = false;
   }

   debug_printf("%s enabled\n",
                sws->have_gl43 ? "SM5+" :
                sws->have_sm5 ? "SM5" :
                sws->have_sm4_1 ? "SM4_1" :
                sws->have_vgpu10 ? "VGPU10" : "VGPU9");

   debug_printf("Mesa: %s %s (%s)\n", svga_get_name(screen),
                PACKAGE_VERSION, MESA_GIT_SHA1);

   /*
    * The D16, D24X8, and D24S8 formats always do an implicit shadow compare
    * when sampled from, where as the DF16, DF24, and D24S8_INT do not.  So
    * we prefer the later when available.
    *
    * This mimics hardware vendors extensions for D3D depth sampling. See also
    * http://aras-p.info/texts/D3D9GPUHacks.html
    */

   {
      bool has_df16, has_df24, has_d24s8_int;
      SVGA3dSurfaceFormatCaps caps;
      SVGA3dSurfaceFormatCaps mask;
      mask.value = 0;
      mask.zStencil = 1;
      mask.texture = 1;

      svgascreen->depth.z16 = SVGA3D_Z_D16;
      svgascreen->depth.x8z24 = SVGA3D_Z_D24X8;
      svgascreen->depth.s8z24 = SVGA3D_Z_D24S8;

      svga_get_format_cap(svgascreen, SVGA3D_Z_DF16, &caps);
      has_df16 = (caps.value & mask.value) == mask.value;

      svga_get_format_cap(svgascreen, SVGA3D_Z_DF24, &caps);
      has_df24 = (caps.value & mask.value) == mask.value;

      svga_get_format_cap(svgascreen, SVGA3D_Z_D24S8_INT, &caps);
      has_d24s8_int = (caps.value & mask.value) == mask.value;

      /* XXX: We might want some other logic here.
       * Like if we only have d24s8_int we should
       * emulate the other formats with that.
       */
      if (has_df16) {
         svgascreen->depth.z16 = SVGA3D_Z_DF16;
      }
      if (has_df24) {
         svgascreen->depth.x8z24 = SVGA3D_Z_DF24;
      }
      if (has_d24s8_int) {
         svgascreen->depth.s8z24 = SVGA3D_Z_D24S8_INT;
      }
   }

   /* Query device caps
    */
   if (sws->have_vgpu10) {
      svgascreen->haveProvokingVertex
         = get_bool_cap(sws, SVGA3D_DEVCAP_DX_PROVOKING_VERTEX, false);
      svgascreen->haveLineSmooth = true;
      svgascreen->maxPointSize = 80.0F;
      svgascreen->max_color_buffers = SVGA3D_DX_MAX_RENDER_TARGETS;

      /* Multisample samples per pixel */
      if (sws->have_sm4_1 && debug_get_bool_option("SVGA_MSAA", true)) {
         if (get_bool_cap(sws, SVGA3D_DEVCAP_MULTISAMPLE_2X, false))
            svgascreen->ms_samples |= 1 << 1;
         if (get_bool_cap(sws, SVGA3D_DEVCAP_MULTISAMPLE_4X, false))
            svgascreen->ms_samples |= 1 << 3;
      }

      if (sws->have_sm5 && debug_get_bool_option("SVGA_MSAA", true)) {
         if (get_bool_cap(sws, SVGA3D_DEVCAP_MULTISAMPLE_8X, false))
            svgascreen->ms_samples |= 1 << 7;
      }

      /* Maximum number of constant buffers */
      if (sws->have_gl43) {
         svgascreen->max_const_buffers = SVGA_MAX_CONST_BUFS;
      }
      else {
         svgascreen->max_const_buffers =
            get_uint_cap(sws, SVGA3D_DEVCAP_DX_MAX_CONSTANT_BUFFERS, 1);
         svgascreen->max_const_buffers = MIN2(svgascreen->max_const_buffers,
                                              SVGA_MAX_CONST_BUFS);
      }

      svgascreen->haveBlendLogicops =
         get_bool_cap(sws, SVGA3D_DEVCAP_LOGIC_BLENDOPS, false);

      screen->is_format_supported = svga_is_dx_format_supported;

      svgascreen->max_viewports = SVGA3D_DX_MAX_VIEWPORTS;

      /* Shader limits */
      if (sws->have_sm4_1) {
         svgascreen->max_vs_inputs  = VGPU10_1_MAX_VS_INPUTS;
         svgascreen->max_vs_outputs = VGPU10_1_MAX_VS_OUTPUTS;
         svgascreen->max_gs_inputs  = VGPU10_1_MAX_GS_INPUTS;
      }
      else {
         svgascreen->max_vs_inputs  = VGPU10_MAX_VS_INPUTS;
         svgascreen->max_vs_outputs = VGPU10_MAX_VS_OUTPUTS;
         svgascreen->max_gs_inputs  = VGPU10_MAX_GS_INPUTS;
      }
   }
   else {
      /* VGPU9 */
      unsigned vs_ver = get_uint_cap(sws, SVGA3D_DEVCAP_VERTEX_SHADER_VERSION,
                                     SVGA3DVSVERSION_NONE);
      unsigned fs_ver = get_uint_cap(sws, SVGA3D_DEVCAP_FRAGMENT_SHADER_VERSION,
                                     SVGA3DPSVERSION_NONE);

      /* we require Shader model 3.0 or later */
      if (fs_ver < SVGA3DPSVERSION_30 || vs_ver < SVGA3DVSVERSION_30) {
         goto error2;
      }

      svgascreen->haveProvokingVertex = false;

      svgascreen->haveLineSmooth =
         get_bool_cap(sws, SVGA3D_DEVCAP_LINE_AA, false);

      svgascreen->maxPointSize =
         get_float_cap(sws, SVGA3D_DEVCAP_MAX_POINT_SIZE, 1.0f);
      /* Keep this to a reasonable size to avoid failures in conform/pntaa.c */
      svgascreen->maxPointSize = MIN2(svgascreen->maxPointSize, 80.0f);

      /* The SVGA3D device always supports 4 targets at this time, regardless
       * of what querying SVGA3D_DEVCAP_MAX_RENDER_TARGETS might return.
       */
      svgascreen->max_color_buffers = 4;

      /* Only support one constant buffer
       */
      svgascreen->max_const_buffers = 1;

      /* No multisampling */
      svgascreen->ms_samples = 0;

      /* Only one viewport */
      svgascreen->max_viewports = 1;

      /* Shader limits */
      svgascreen->max_vs_inputs  = 16;
      svgascreen->max_vs_outputs = 10;
      svgascreen->max_gs_inputs  = 0;
   }

   /* common VGPU9 / VGPU10 caps */
   svgascreen->haveLineStipple =
      get_bool_cap(sws, SVGA3D_DEVCAP_LINE_STIPPLE, false);

   svgascreen->maxLineWidth =
      MAX2(1.0, get_float_cap(sws, SVGA3D_DEVCAP_MAX_LINE_WIDTH, 1.0f));

   svgascreen->maxLineWidthAA =
      MAX2(1.0, get_float_cap(sws, SVGA3D_DEVCAP_MAX_AA_LINE_WIDTH, 1.0f));

   if (0) {
      debug_printf("svga: haveProvokingVertex %u\n",
                   svgascreen->haveProvokingVertex);
      debug_printf("svga: haveLineStip %u  "
                   "haveLineSmooth %u  maxLineWidth %.2f  maxLineWidthAA %.2f\n",
                   svgascreen->haveLineStipple, svgascreen->haveLineSmooth,
                   svgascreen->maxLineWidth, svgascreen->maxLineWidthAA);
      debug_printf("svga: maxPointSize %g\n", svgascreen->maxPointSize);
      debug_printf("svga: msaa samples mask: 0x%x\n", svgascreen->ms_samples);
   }

   (void) mtx_init(&svgascreen->tex_mutex, mtx_plain);
   (void) mtx_init(&svgascreen->swc_mutex, mtx_plain | mtx_recursive);

   svga_screen_cache_init(svgascreen);

   if (debug_get_bool_option("SVGA_NO_LOGGING", false) == true) {
      svgascreen->sws->host_log = nop_host_log;
   } else {
      init_logging(screen);
   }

   return screen;
error2:
   FREE(svgascreen);
error1:
   return NULL;
}


struct svga_winsys_screen *
svga_winsys_screen(struct pipe_screen *screen)
{
   return svga_screen(screen)->sws;
}


#ifdef DEBUG
struct svga_screen *
svga_screen(struct pipe_screen *screen)
{
   assert(screen);
   assert(screen->destroy == svga_destroy_screen);
   return (struct svga_screen *)screen;
}
#endif
