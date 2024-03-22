/*
 * Copyright (C) 2012 Rob Clark <robclark@freedesktop.org>
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#include "pipe/p_defines.h"
#include "pipe/p_screen.h"
#include "pipe/p_state.h"

#include "util/format/u_format.h"
#include "util/format/u_format_s3tc.h"
#include "util/u_debug.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_screen.h"
#include "util/u_string.h"
#include "util/xmlconfig.h"

#include "util/os_time.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "drm-uapi/drm_fourcc.h"
#include <sys/sysinfo.h>

#include "freedreno_fence.h"
#include "freedreno_perfetto.h"
#include "freedreno_query.h"
#include "freedreno_resource.h"
#include "freedreno_screen.h"
#include "freedreno_util.h"

#include "a2xx/fd2_screen.h"
#include "a3xx/fd3_screen.h"
#include "a4xx/fd4_screen.h"
#include "a5xx/fd5_screen.h"
#include "a6xx/fd6_screen.h"

/* for fd_get_driver/device_uuid() */
#include "common/freedreno_uuid.h"

#include "a2xx/ir2.h"
#include "ir3/ir3_descriptor.h"
#include "ir3/ir3_gallium.h"
#include "ir3/ir3_nir.h"

/* clang-format off */
static const struct debug_named_value fd_debug_options[] = {
   {"msgs",      FD_DBG_MSGS,     "Print debug messages"},
   {"disasm",    FD_DBG_DISASM,   "Dump TGSI and adreno shader disassembly (a2xx only, see IR3_SHADER_DEBUG)"},
   {"dclear",    FD_DBG_DCLEAR,   "Mark all state dirty after clear"},
   {"ddraw",     FD_DBG_DDRAW,    "Mark all state dirty after draw"},
   {"noscis",    FD_DBG_NOSCIS,   "Disable scissor optimization"},
   {"direct",    FD_DBG_DIRECT,   "Force inline (SS_DIRECT) state loads"},
   {"gmem",      FD_DBG_GMEM,     "Use gmem rendering when it is permitted"},
   {"perf",      FD_DBG_PERF,     "Enable performance warnings"},
   {"nobin",     FD_DBG_NOBIN,    "Disable hw binning"},
   {"sysmem",    FD_DBG_SYSMEM,   "Use sysmem only rendering (no tiling)"},
   {"serialc",   FD_DBG_SERIALC,  "Disable asynchronous shader compile"},
   {"shaderdb",  FD_DBG_SHADERDB, "Enable shaderdb output"},
   {"flush",     FD_DBG_FLUSH,    "Force flush after every draw"},
   {"deqp",      FD_DBG_DEQP,     "Enable dEQP hacks"},
   {"inorder",   FD_DBG_INORDER,  "Disable reordering for draws/blits"},
   {"bstat",     FD_DBG_BSTAT,    "Print batch stats at context destroy"},
   {"nogrow",    FD_DBG_NOGROW,   "Disable \"growable\" cmdstream buffers, even if kernel supports it"},
   {"lrz",       FD_DBG_LRZ,      "Enable experimental LRZ support (a5xx)"},
   {"noindirect",FD_DBG_NOINDR,   "Disable hw indirect draws (emulate on CPU)"},
   {"noblit",    FD_DBG_NOBLIT,   "Disable blitter (fallback to generic blit path)"},
   {"hiprio",    FD_DBG_HIPRIO,   "Force high-priority context"},
   {"ttile",     FD_DBG_TTILE,    "Enable texture tiling (a2xx/a3xx/a5xx)"},
   {"perfcntrs", FD_DBG_PERFC,    "Expose performance counters"},
   {"noubwc",    FD_DBG_NOUBWC,   "Disable UBWC for all internal buffers"},
   {"nolrz",     FD_DBG_NOLRZ,    "Disable LRZ (a6xx)"},
   {"notile",    FD_DBG_NOTILE,   "Disable tiling for all internal buffers"},
   {"layout",    FD_DBG_LAYOUT,   "Dump resource layouts"},
   {"nofp16",    FD_DBG_NOFP16,   "Disable mediump precision lowering"},
   {"nohw",      FD_DBG_NOHW,     "Disable submitting commands to the HW"},
   {"nosbin",    FD_DBG_NOSBIN,   "Execute GMEM bins in raster order instead of 'S' pattern"},
   DEBUG_NAMED_VALUE_END
};
/* clang-format on */

DEBUG_GET_ONCE_FLAGS_OPTION(fd_mesa_debug, "FD_MESA_DEBUG", fd_debug_options, 0)

int fd_mesa_debug = 0;
bool fd_binning_enabled = true;

static const char *
fd_screen_get_name(struct pipe_screen *pscreen)
{
   return fd_dev_name(fd_screen(pscreen)->dev_id);
}

static const char *
fd_screen_get_vendor(struct pipe_screen *pscreen)
{
   return "freedreno";
}

static const char *
fd_screen_get_device_vendor(struct pipe_screen *pscreen)
{
   return "Qualcomm";
}

static void
fd_get_sample_pixel_grid(struct pipe_screen *pscreen, unsigned sample_count,
                         unsigned *out_width, unsigned *out_height)
{
   *out_width = 1;
   *out_height = 1;
}

static uint64_t
fd_screen_get_timestamp(struct pipe_screen *pscreen)
{
   struct fd_screen *screen = fd_screen(pscreen);

   if (screen->has_timestamp) {
      uint64_t n;
      fd_pipe_get_param(screen->pipe, FD_TIMESTAMP, &n);
      return ticks_to_ns(n);
   } else {
      int64_t cpu_time = os_time_get_nano();
      return cpu_time + screen->cpu_gpu_time_delta;
   }
}

static void
fd_screen_destroy(struct pipe_screen *pscreen)
{
   struct fd_screen *screen = fd_screen(pscreen);

   if (screen->aux_ctx)
      screen->aux_ctx->destroy(screen->aux_ctx);

   if (screen->tess_bo)
      fd_bo_del(screen->tess_bo);

   if (screen->pipe)
      fd_pipe_del(screen->pipe);

   if (screen->dev) {
      fd_device_purge(screen->dev);
      fd_device_del(screen->dev);
   }

   if (screen->ro)
      screen->ro->destroy(screen->ro);

   fd_bc_fini(&screen->batch_cache);
   fd_gmem_screen_fini(pscreen);

   slab_destroy_parent(&screen->transfer_pool);

   simple_mtx_destroy(&screen->lock);

   util_idalloc_mt_fini(&screen->buffer_ids);

   u_transfer_helper_destroy(pscreen->transfer_helper);

   if (screen->compiler)
      ir3_screen_fini(pscreen);

   free(screen->perfcntr_queries);
   free(screen);
}

static uint64_t
get_memory_size(struct fd_screen *screen)
{
   uint64_t system_memory;

   if (!os_get_total_physical_memory(&system_memory))
      return 0;
   if (fd_device_version(screen->dev) >= FD_VERSION_VA_SIZE) {
      uint64_t va_size;
      if (!fd_pipe_get_param(screen->pipe, FD_VA_SIZE, &va_size)) {
         system_memory = MIN2(system_memory, va_size);
      }
   }

   return system_memory;
}

static void
fd_query_memory_info(struct pipe_screen *pscreen,
                     struct pipe_memory_info *info)
{
   unsigned mem = get_memory_size(fd_screen(pscreen)) >> 10;

   memset(info, 0, sizeof(*info));

   info->total_device_memory = mem;
   info->avail_device_memory = mem;
}


/*
TODO either move caps to a2xx/a3xx specific code, or maybe have some
tables for things that differ if the delta is not too much..
 */
static int
fd_screen_get_param(struct pipe_screen *pscreen, enum pipe_cap param)
{
   struct fd_screen *screen = fd_screen(pscreen);

   /* this is probably not totally correct.. but it's a start: */
   switch (param) {
   /* Supported features (boolean caps). */
   case PIPE_CAP_NPOT_TEXTURES:
   case PIPE_CAP_MIXED_FRAMEBUFFER_SIZES:
   case PIPE_CAP_ANISOTROPIC_FILTER:
   case PIPE_CAP_BLEND_EQUATION_SEPARATE:
   case PIPE_CAP_TEXTURE_SWIZZLE:
   case PIPE_CAP_FS_COORD_ORIGIN_UPPER_LEFT:
   case PIPE_CAP_SEAMLESS_CUBE_MAP:
   case PIPE_CAP_VERTEX_COLOR_UNCLAMPED:
   case PIPE_CAP_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION:
   case PIPE_CAP_STRING_MARKER:
   case PIPE_CAP_MIXED_COLOR_DEPTH_BITS:
   case PIPE_CAP_TEXTURE_BARRIER:
   case PIPE_CAP_INVALIDATE_BUFFER:
   case PIPE_CAP_GLSL_TESS_LEVELS_AS_INPUTS:
   case PIPE_CAP_NIR_COMPACT_ARRAYS:
   case PIPE_CAP_TEXTURE_MIRROR_CLAMP_TO_EDGE:
   case PIPE_CAP_GL_SPIRV:
   case PIPE_CAP_FBFETCH_COHERENT:
   case PIPE_CAP_HAS_CONST_BW:
      return 1;

   case PIPE_CAP_COPY_BETWEEN_COMPRESSED_AND_PLAIN_FORMATS:
   case PIPE_CAP_MULTI_DRAW_INDIRECT:
   case PIPE_CAP_DRAW_PARAMETERS:
   case PIPE_CAP_MULTI_DRAW_INDIRECT_PARAMS:
   case PIPE_CAP_DEPTH_BOUNDS_TEST:
      return is_a6xx(screen);

   case PIPE_CAP_VERTEX_BUFFER_OFFSET_4BYTE_ALIGNED_ONLY:
   case PIPE_CAP_VERTEX_BUFFER_STRIDE_4BYTE_ALIGNED_ONLY:
   case PIPE_CAP_VERTEX_ELEMENT_SRC_OFFSET_4BYTE_ALIGNED_ONLY:
      return is_a2xx(screen);

   case PIPE_CAP_FS_COORD_PIXEL_CENTER_INTEGER:
      return is_a2xx(screen);
   case PIPE_CAP_FS_COORD_PIXEL_CENTER_HALF_INTEGER:
      return !is_a2xx(screen);

   case PIPE_CAP_PACKED_UNIFORMS:
      return !is_a2xx(screen);

   case PIPE_CAP_ROBUST_BUFFER_ACCESS_BEHAVIOR:
   case PIPE_CAP_DEVICE_RESET_STATUS_QUERY:
      return screen->has_robustness;

   case PIPE_CAP_COMPUTE:
      return has_compute(screen);

   case PIPE_CAP_TEXTURE_TRANSFER_MODES:
   case PIPE_CAP_PCI_GROUP:
   case PIPE_CAP_PCI_BUS:
   case PIPE_CAP_PCI_DEVICE:
   case PIPE_CAP_PCI_FUNCTION:
      return 0;

   case PIPE_CAP_SUPPORTED_PRIM_MODES:
   case PIPE_CAP_SUPPORTED_PRIM_MODES_WITH_RESTART:
      return screen->primtypes_mask;

   case PIPE_CAP_FRAGMENT_SHADER_TEXTURE_LOD:
   case PIPE_CAP_FRAGMENT_SHADER_DERIVATIVES:
   case PIPE_CAP_PRIMITIVE_RESTART:
   case PIPE_CAP_PRIMITIVE_RESTART_FIXED_INDEX:
   case PIPE_CAP_VS_INSTANCEID:
   case PIPE_CAP_VERTEX_ELEMENT_INSTANCE_DIVISOR:
   case PIPE_CAP_INDEP_BLEND_ENABLE:
   case PIPE_CAP_INDEP_BLEND_FUNC:
   case PIPE_CAP_TEXTURE_BUFFER_OBJECTS:
   case PIPE_CAP_TEXTURE_HALF_FLOAT_LINEAR:
   case PIPE_CAP_CONDITIONAL_RENDER:
   case PIPE_CAP_CONDITIONAL_RENDER_INVERTED:
   case PIPE_CAP_SEAMLESS_CUBE_MAP_PER_TEXTURE:
   case PIPE_CAP_CLIP_HALFZ:
      return is_a3xx(screen) || is_a4xx(screen) || is_a5xx(screen) ||
             is_a6xx(screen);

   case PIPE_CAP_FAKE_SW_MSAA:
      return !fd_screen_get_param(pscreen, PIPE_CAP_TEXTURE_MULTISAMPLE);

   case PIPE_CAP_TEXTURE_MULTISAMPLE:
   case PIPE_CAP_IMAGE_STORE_FORMATTED:
   case PIPE_CAP_IMAGE_LOAD_FORMATTED:
      return is_a5xx(screen) || is_a6xx(screen);

   case PIPE_CAP_SURFACE_SAMPLE_COUNT:
      return is_a6xx(screen);

   case PIPE_CAP_DEPTH_CLIP_DISABLE:
      return is_a3xx(screen) || is_a4xx(screen) || is_a6xx(screen);

   case PIPE_CAP_POST_DEPTH_COVERAGE:
   case PIPE_CAP_DEPTH_CLIP_DISABLE_SEPARATE:
   case PIPE_CAP_DEMOTE_TO_HELPER_INVOCATION:
      return is_a6xx(screen);

   case PIPE_CAP_SAMPLER_REDUCTION_MINMAX:
   case PIPE_CAP_SAMPLER_REDUCTION_MINMAX_ARB:
      return is_a6xx(screen) && screen->info->a6xx.has_sampler_minmax;

   case PIPE_CAP_PROGRAMMABLE_SAMPLE_LOCATIONS:
      return is_a6xx(screen) && screen->info->a6xx.has_sample_locations;

   case PIPE_CAP_POLYGON_OFFSET_CLAMP:
      return is_a4xx(screen) || is_a5xx(screen) || is_a6xx(screen);

   case PIPE_CAP_PREFER_IMM_ARRAYS_AS_CONSTBUF:
      return 0;

   case PIPE_CAP_TEXTURE_BUFFER_OFFSET_ALIGNMENT:
      if (is_a3xx(screen))
         return 16;
      if (is_a4xx(screen) || is_a5xx(screen) || is_a6xx(screen))
         return 64;
      return 0;
   case PIPE_CAP_MAX_TEXEL_BUFFER_ELEMENTS_UINT:
      /* We could possibly emulate more by pretending 2d/rect textures and
       * splitting high bits of index into 2nd dimension..
       */
      if (is_a3xx(screen))
         return A3XX_MAX_TEXEL_BUFFER_ELEMENTS_UINT;

      /* Note that the Vulkan blob on a540 and 640 report a
       * maxTexelBufferElements of just 65536 (the GLES3.2 and Vulkan
       * minimum).
       */
      if (is_a4xx(screen) || is_a5xx(screen) || is_a6xx(screen))
         return A4XX_MAX_TEXEL_BUFFER_ELEMENTS_UINT;

      return 0;

   case PIPE_CAP_TEXTURE_BORDER_COLOR_QUIRK:
      return PIPE_QUIRK_TEXTURE_BORDER_COLOR_SWIZZLE_FREEDRENO;

   case PIPE_CAP_TEXTURE_FLOAT_LINEAR:
   case PIPE_CAP_CUBE_MAP_ARRAY:
   case PIPE_CAP_SAMPLER_VIEW_TARGET:
   case PIPE_CAP_TEXTURE_QUERY_LOD:
      return is_a4xx(screen) || is_a5xx(screen) || is_a6xx(screen);

   case PIPE_CAP_START_INSTANCE:
      /* Note that a5xx can do this, it just can't (at least with
       * current firmware) do draw_indirect with base_instance.
       * Since draw_indirect is needed sooner (gles31 and gl40 vs
       * gl42), hide base_instance on a5xx.  :-/
       */
      return is_a4xx(screen) || is_a6xx(screen);

   case PIPE_CAP_CONSTANT_BUFFER_OFFSET_ALIGNMENT:
      return 64;

   case PIPE_CAP_INT64:
   case PIPE_CAP_DOUBLES:
      return is_ir3(screen);

   case PIPE_CAP_GLSL_FEATURE_LEVEL:
   case PIPE_CAP_GLSL_FEATURE_LEVEL_COMPATIBILITY:
      if (is_a6xx(screen))
         return 460;
      else if (is_ir3(screen))
         return 140;
      else
         return 120;

   case PIPE_CAP_ESSL_FEATURE_LEVEL:
      if (is_a4xx(screen) || is_a5xx(screen) || is_a6xx(screen))
         return 320;
      if (is_ir3(screen))
         return 300;
      return 120;

   case PIPE_CAP_SHADER_BUFFER_OFFSET_ALIGNMENT:
      if (is_a6xx(screen))
         return 64;
      if (is_a5xx(screen))
         return 4;
      if (is_a4xx(screen))
         return 4;
      return 0;

   case PIPE_CAP_MAX_TEXTURE_GATHER_COMPONENTS:
      if (is_a4xx(screen) || is_a5xx(screen) || is_a6xx(screen))
         return 4;
      return 0;

   /* TODO if we need this, do it in nir/ir3 backend to avoid breaking
    * precompile: */
   case PIPE_CAP_FORCE_PERSAMPLE_INTERP:
      return 0;

   case PIPE_CAP_FBFETCH:
      if (fd_device_version(screen->dev) >= FD_VERSION_GMEM_BASE &&
          is_a6xx(screen))
         return screen->max_rts;
      return 0;
   case PIPE_CAP_SAMPLE_SHADING:
      if (is_a6xx(screen))
         return 1;
      return 0;

   case PIPE_CAP_CONTEXT_PRIORITY_MASK:
      return screen->priority_mask;

   case PIPE_CAP_DRAW_INDIRECT:
      if (is_a4xx(screen) || is_a5xx(screen) || is_a6xx(screen))
         return 1;
      return 0;

   case PIPE_CAP_FRAMEBUFFER_NO_ATTACHMENT:
      if (is_a4xx(screen) || is_a5xx(screen) || is_a6xx(screen))
         return 1;
      return 0;

   case PIPE_CAP_LOAD_CONSTBUF:
      /* name is confusing, but this turns on std430 packing */
      if (is_ir3(screen))
         return 1;
      return 0;

   case PIPE_CAP_NIR_IMAGES_AS_DEREF:
      return 0;

   case PIPE_CAP_VS_LAYER_VIEWPORT:
   case PIPE_CAP_TES_LAYER_VIEWPORT:
      return is_a6xx(screen);

   case PIPE_CAP_MAX_VIEWPORTS:
      if (is_a6xx(screen))
         return 16;
      return 1;

   case PIPE_CAP_MAX_VARYINGS:
      return is_a6xx(screen) ? 31 : 16;

   case PIPE_CAP_MAX_SHADER_PATCH_VARYINGS:
      /* We don't really have a limit on this, it all goes into the main
       * memory buffer. Needs to be at least 120 / 4 (minimum requirement
       * for GL_MAX_TESS_PATCH_COMPONENTS).
       */
      return 128;

   case PIPE_CAP_MAX_TEXTURE_UPLOAD_MEMORY_BUDGET:
      return 64 * 1024 * 1024;

   case PIPE_CAP_SHAREABLE_SHADERS:
      if (is_ir3(screen))
         return 1;
      return 0;

   /* Geometry shaders.. */
   case PIPE_CAP_MAX_GEOMETRY_OUTPUT_VERTICES:
      return 256;
   case PIPE_CAP_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS:
      return 2048;
   case PIPE_CAP_MAX_GS_INVOCATIONS:
      return 32;

   /* Only a2xx has the half-border clamp mode in HW, just have mesa/st lower
    * it for later HW.
    */
   case PIPE_CAP_GL_CLAMP:
      return is_a2xx(screen);

   case PIPE_CAP_CLIP_PLANES:
      /* Gens that support GS, have GS lowered into a quasi-VS which confuses
       * the frontend clip-plane lowering.  So we handle this in the backend
       *
       */
      if (pscreen->get_shader_param(pscreen, PIPE_SHADER_GEOMETRY,
                                    PIPE_SHADER_CAP_MAX_INSTRUCTIONS))
         return 1;

      /* On a3xx, there is HW support for GL user clip planes that
       * occasionally has to fall back to shader key-based lowering to clip
       * distances in the VS, and we don't support clip distances so that is
       * always shader-based lowering in the FS.
       *
       * On a4xx, there is no HW support for clip planes, so they are
       * always lowered to clip distances.  We also lack SW support for the
       * HW's clip distances in HW, so we do shader-based lowering in the FS
       * in the driver backend.
       *
       * On a5xx-a6xx, we have the HW clip distances hooked up, so we just let
       * mesa/st lower desktop GL's clip planes to clip distances in the last
       * vertex shader stage.
       *
       * NOTE: but see comment above about geometry shaders
       */
      return !is_a5xx(screen);

   /* Stream output. */
   case PIPE_CAP_MAX_VERTEX_STREAMS:
      if (is_a6xx(screen))  /* has SO + GS */
         return PIPE_MAX_SO_BUFFERS;
      return 0;
   case PIPE_CAP_MAX_STREAM_OUTPUT_BUFFERS:
      if (is_ir3(screen))
         return PIPE_MAX_SO_BUFFERS;
      return 0;
   case PIPE_CAP_STREAM_OUTPUT_PAUSE_RESUME:
   case PIPE_CAP_STREAM_OUTPUT_INTERLEAVE_BUFFERS:
   case PIPE_CAP_FS_POSITION_IS_SYSVAL:
   case PIPE_CAP_TGSI_TEXCOORD:
   case PIPE_CAP_SHADER_ARRAY_COMPONENTS:
   case PIPE_CAP_TEXTURE_QUERY_SAMPLES:
   case PIPE_CAP_FS_FINE_DERIVATIVE:
      if (is_ir3(screen))
         return 1;
      return 0;
   case PIPE_CAP_SHADER_GROUP_VOTE:
      return is_a6xx(screen);
   case PIPE_CAP_FS_FACE_IS_INTEGER_SYSVAL:
      return 1;
   case PIPE_CAP_FS_POINT_IS_SYSVAL:
      return is_a2xx(screen);
   case PIPE_CAP_MAX_STREAM_OUTPUT_SEPARATE_COMPONENTS:
   case PIPE_CAP_MAX_STREAM_OUTPUT_INTERLEAVED_COMPONENTS:
      if (is_ir3(screen))
         return 16 * 4; /* should only be shader out limit? */
      return 0;

   /* Texturing. */
   case PIPE_CAP_MAX_TEXTURE_2D_SIZE:
      if (is_a6xx(screen) || is_a5xx(screen) || is_a4xx(screen))
         return 16384;
      else
         return 8192;
   case PIPE_CAP_MAX_TEXTURE_CUBE_LEVELS:
      if (is_a6xx(screen) || is_a5xx(screen) || is_a4xx(screen))
         return 15;
      else
         return 14;

   case PIPE_CAP_MAX_TEXTURE_3D_LEVELS:
      if (is_a3xx(screen))
         return 11;
      return 12;

   case PIPE_CAP_MAX_TEXTURE_ARRAY_LAYERS:
      if (is_a6xx(screen))
         return 2048;
      return (is_a3xx(screen) || is_a4xx(screen) || is_a5xx(screen))
                ? 256
                : 0;

   /* Render targets. */
   case PIPE_CAP_MAX_RENDER_TARGETS:
      return screen->max_rts;
   case PIPE_CAP_MAX_DUAL_SOURCE_RENDER_TARGETS:
      return (is_a3xx(screen) || is_a6xx(screen)) ? 1 : 0;

   /* Queries. */
   case PIPE_CAP_OCCLUSION_QUERY:
      return is_a3xx(screen) || is_a4xx(screen) || is_a5xx(screen) ||
             is_a6xx(screen);
   case PIPE_CAP_QUERY_TIMESTAMP:
   case PIPE_CAP_QUERY_TIME_ELAPSED:
      /* only a4xx, requires new enough kernel so we know max_freq: */
      return (screen->max_freq > 0) &&
             (is_a4xx(screen) || is_a5xx(screen) || is_a6xx(screen));
   case PIPE_CAP_TIMER_RESOLUTION:
      return ticks_to_ns(1);
   case PIPE_CAP_QUERY_BUFFER_OBJECT:
   case PIPE_CAP_QUERY_SO_OVERFLOW:
   case PIPE_CAP_QUERY_PIPELINE_STATISTICS_SINGLE:
      return is_a6xx(screen);

   case PIPE_CAP_VENDOR_ID:
      return 0x5143;
   case PIPE_CAP_DEVICE_ID:
      return 0xFFFFFFFF;
   case PIPE_CAP_ACCELERATED:
      return 1;

   case PIPE_CAP_VIDEO_MEMORY:
      return (int)(get_memory_size(screen) >> 20);

   case PIPE_CAP_QUERY_MEMORY_INFO: /* Enables GL_ATI_meminfo */
      return get_memory_size(screen) != 0;

   case PIPE_CAP_UMA:
      return 1;
   case PIPE_CAP_MEMOBJ:
      return fd_device_version(screen->dev) >= FD_VERSION_MEMORY_FD;
   case PIPE_CAP_NATIVE_FENCE_FD:
      return fd_device_version(screen->dev) >= FD_VERSION_FENCE_FD;
   case PIPE_CAP_FENCE_SIGNAL:
      return screen->has_syncobj;
   case PIPE_CAP_CULL_DISTANCE:
      return is_a6xx(screen);
   case PIPE_CAP_SHADER_STENCIL_EXPORT:
      return is_a6xx(screen);
   case PIPE_CAP_TWO_SIDED_COLOR:
      return 0;
   case PIPE_CAP_THROTTLE:
      return screen->driconf.enable_throttling;
   default:
      return u_pipe_screen_get_param_defaults(pscreen, param);
   }
}

static float
fd_screen_get_paramf(struct pipe_screen *pscreen, enum pipe_capf param)
{
   switch (param) {
   case PIPE_CAPF_MIN_LINE_WIDTH:
   case PIPE_CAPF_MIN_LINE_WIDTH_AA:
   case PIPE_CAPF_MIN_POINT_SIZE:
   case PIPE_CAPF_MIN_POINT_SIZE_AA:
      return 1;
   case PIPE_CAPF_POINT_SIZE_GRANULARITY:
   case PIPE_CAPF_LINE_WIDTH_GRANULARITY:
      return 0.1f;
   case PIPE_CAPF_MAX_LINE_WIDTH:
   case PIPE_CAPF_MAX_LINE_WIDTH_AA:
      /* NOTE: actual value is 127.0f, but this is working around a deqp
       * bug.. dEQP-GLES3.functional.rasterization.primitives.lines_wide
       * uses too small of a render target size, and gets confused when
       * the lines start going offscreen.
       *
       * See: https://code.google.com/p/android/issues/detail?id=206513
       */
      if (FD_DBG(DEQP))
         return 48.0f;
      return 127.0f;
   case PIPE_CAPF_MAX_POINT_SIZE:
   case PIPE_CAPF_MAX_POINT_SIZE_AA:
      return 4092.0f;
   case PIPE_CAPF_MAX_TEXTURE_ANISOTROPY:
      return 16.0f;
   case PIPE_CAPF_MAX_TEXTURE_LOD_BIAS:
      return 15.0f;
   case PIPE_CAPF_MIN_CONSERVATIVE_RASTER_DILATE:
   case PIPE_CAPF_MAX_CONSERVATIVE_RASTER_DILATE:
   case PIPE_CAPF_CONSERVATIVE_RASTER_DILATE_GRANULARITY:
      return 0.0f;
   }
   mesa_loge("unknown paramf %d", param);
   return 0;
}

static int
fd_screen_get_shader_param(struct pipe_screen *pscreen,
                           enum pipe_shader_type shader,
                           enum pipe_shader_cap param)
{
   struct fd_screen *screen = fd_screen(pscreen);

   switch (shader) {
   case PIPE_SHADER_FRAGMENT:
   case PIPE_SHADER_VERTEX:
      break;
   case PIPE_SHADER_TESS_CTRL:
   case PIPE_SHADER_TESS_EVAL:
   case PIPE_SHADER_GEOMETRY:
      if (is_a6xx(screen))
         break;
      return 0;
   case PIPE_SHADER_COMPUTE:
      if (has_compute(screen))
         break;
      return 0;
   case PIPE_SHADER_TASK:
   case PIPE_SHADER_MESH:
      return 0;
   default:
      mesa_loge("unknown shader type %d", shader);
      return 0;
   }

   /* this is probably not totally correct.. but it's a start: */
   switch (param) {
   case PIPE_SHADER_CAP_MAX_INSTRUCTIONS:
   case PIPE_SHADER_CAP_MAX_ALU_INSTRUCTIONS:
   case PIPE_SHADER_CAP_MAX_TEX_INSTRUCTIONS:
   case PIPE_SHADER_CAP_MAX_TEX_INDIRECTIONS:
      return 16384;
   case PIPE_SHADER_CAP_MAX_CONTROL_FLOW_DEPTH:
      return 8; /* XXX */
   case PIPE_SHADER_CAP_MAX_INPUTS:
      if (shader == PIPE_SHADER_GEOMETRY && is_a6xx(screen))
         return 16;
      return is_a6xx(screen) ?
         (screen->info->a6xx.vs_max_inputs_count) : 16;
   case PIPE_SHADER_CAP_MAX_OUTPUTS:
      return is_a6xx(screen) ? 32 : 16;
   case PIPE_SHADER_CAP_MAX_TEMPS:
      return 64; /* Max native temporaries. */
   case PIPE_SHADER_CAP_MAX_CONST_BUFFER0_SIZE:
      /* NOTE: seems to be limit for a3xx is actually 512 but
       * split between VS and FS.  Use lower limit of 256 to
       * avoid getting into impossible situations:
       */
      return ((is_a3xx(screen) || is_a4xx(screen) || is_a5xx(screen) ||
               is_a6xx(screen))
                 ? 4096
                 : 64) *
             sizeof(float[4]);
   case PIPE_SHADER_CAP_MAX_CONST_BUFFERS:
      return is_ir3(screen) ? 16 : 1;
   case PIPE_SHADER_CAP_CONT_SUPPORTED:
      return 1;
   case PIPE_SHADER_CAP_INDIRECT_INPUT_ADDR:
   case PIPE_SHADER_CAP_INDIRECT_OUTPUT_ADDR:
   case PIPE_SHADER_CAP_INDIRECT_TEMP_ADDR:
   case PIPE_SHADER_CAP_INDIRECT_CONST_ADDR:
      /* a2xx compiler doesn't handle indirect: */
      return is_ir3(screen) ? 1 : 0;
   case PIPE_SHADER_CAP_SUBROUTINES:
   case PIPE_SHADER_CAP_TGSI_ANY_INOUT_DECL_RANGE:
   case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTERS:
   case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTER_BUFFERS:
      return 0;
   case PIPE_SHADER_CAP_TGSI_SQRT_SUPPORTED:
      return 1;
   case PIPE_SHADER_CAP_INTEGERS:
      return is_ir3(screen) ? 1 : 0;
   case PIPE_SHADER_CAP_INT64_ATOMICS:
   case PIPE_SHADER_CAP_FP16_DERIVATIVES:
   case PIPE_SHADER_CAP_FP16_CONST_BUFFERS:
   case PIPE_SHADER_CAP_GLSL_16BIT_CONSTS:
      return 0;
   case PIPE_SHADER_CAP_INT16:
   case PIPE_SHADER_CAP_FP16:
      return (
         (is_a5xx(screen) || is_a6xx(screen)) &&
         (shader == PIPE_SHADER_COMPUTE || shader == PIPE_SHADER_FRAGMENT) &&
         !FD_DBG(NOFP16));
   case PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS:
   case PIPE_SHADER_CAP_MAX_SAMPLER_VIEWS:
      return 16;
   case PIPE_SHADER_CAP_SUPPORTED_IRS:
      return (1 << PIPE_SHADER_IR_NIR) |
             COND(has_compute(screen) && (shader == PIPE_SHADER_COMPUTE),
                  (1 << PIPE_SHADER_IR_NIR_SERIALIZED)) |
             /* tgsi_to_nir doesn't support all stages: */
             COND((shader == PIPE_SHADER_VERTEX) ||
                  (shader == PIPE_SHADER_FRAGMENT) ||
                  (shader == PIPE_SHADER_COMPUTE),
                  (1 << PIPE_SHADER_IR_TGSI));
   case PIPE_SHADER_CAP_MAX_SHADER_BUFFERS:
   case PIPE_SHADER_CAP_MAX_SHADER_IMAGES:
      if (is_a6xx(screen)) {
         if (param == PIPE_SHADER_CAP_MAX_SHADER_BUFFERS) {
            return IR3_BINDLESS_SSBO_COUNT;
         } else {
            return IR3_BINDLESS_IMAGE_COUNT;
         }
      } else if (is_a4xx(screen) || is_a5xx(screen) || is_a6xx(screen)) {
         /* a5xx (and a4xx for that matter) has one state-block
          * for compute-shader SSBO's and another that is shared
          * by VS/HS/DS/GS/FS..  so to simplify things for now
          * just advertise SSBOs for FS and CS.  We could possibly
          * do what blob does, and partition the space for
          * VS/HS/DS/GS/FS.  The blob advertises:
          *
          *   GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS: 4
          *   GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS: 4
          *   GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS: 4
          *   GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS: 4
          *   GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS: 4
          *   GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS: 24
          *   GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS: 24
          *
          * I think that way we could avoid having to patch shaders
          * for actual SSBO indexes by using a static partitioning.
          *
          * Note same state block is used for images and buffers,
          * but images also need texture state for read access
          * (isam/isam.3d)
          */
         switch (shader) {
         case PIPE_SHADER_FRAGMENT:
         case PIPE_SHADER_COMPUTE:
            return 24;
         default:
            return 0;
         }
      }
      return 0;
   }
   mesa_loge("unknown shader param %d", param);
   return 0;
}

/* TODO depending on how much the limits differ for a3xx/a4xx, maybe move this
 * into per-generation backend?
 */
static int
fd_get_compute_param(struct pipe_screen *pscreen, enum pipe_shader_ir ir_type,
                     enum pipe_compute_cap param, void *ret)
{
   struct fd_screen *screen = fd_screen(pscreen);
   const char *const ir = "ir3";

   if (!has_compute(screen))
      return 0;

   struct ir3_compiler *compiler = screen->compiler;

#define RET(x)                                                                 \
   do {                                                                        \
      if (ret)                                                                 \
         memcpy(ret, x, sizeof(x));                                            \
      return sizeof(x);                                                        \
   } while (0)

   switch (param) {
   case PIPE_COMPUTE_CAP_ADDRESS_BITS:
      if (screen->gen >= 5)
         RET((uint32_t[]){64});
      RET((uint32_t[]){32});

   case PIPE_COMPUTE_CAP_IR_TARGET:
      if (ret)
         sprintf(ret, "%s", ir);
      return strlen(ir) * sizeof(char);

   case PIPE_COMPUTE_CAP_GRID_DIMENSION:
      RET((uint64_t[]){3});

   case PIPE_COMPUTE_CAP_MAX_GRID_SIZE:
      RET(((uint64_t[]){65535, 65535, 65535}));

   case PIPE_COMPUTE_CAP_MAX_BLOCK_SIZE:
      RET(((uint64_t[]){1024, 1024, 64}));

   case PIPE_COMPUTE_CAP_MAX_THREADS_PER_BLOCK:
      RET((uint64_t[]){1024});

   case PIPE_COMPUTE_CAP_MAX_GLOBAL_SIZE:
      RET((uint64_t[]){screen->ram_size});

   case PIPE_COMPUTE_CAP_MAX_LOCAL_SIZE:
      RET((uint64_t[]){screen->info->cs_shared_mem_size});

   case PIPE_COMPUTE_CAP_MAX_PRIVATE_SIZE:
   case PIPE_COMPUTE_CAP_MAX_INPUT_SIZE:
      RET((uint64_t[]){4096});

   case PIPE_COMPUTE_CAP_MAX_MEM_ALLOC_SIZE:
      RET((uint64_t[]){screen->ram_size});

   case PIPE_COMPUTE_CAP_MAX_CLOCK_FREQUENCY:
      RET((uint32_t[]){screen->max_freq / 1000000});

   case PIPE_COMPUTE_CAP_MAX_COMPUTE_UNITS:
      RET((uint32_t[]){9999}); // TODO

   case PIPE_COMPUTE_CAP_IMAGES_SUPPORTED:
      RET((uint32_t[]){1});

   case PIPE_COMPUTE_CAP_SUBGROUP_SIZES:
      RET((uint32_t[]){32}); // TODO

   case PIPE_COMPUTE_CAP_MAX_SUBGROUPS:
      RET((uint32_t[]){0}); // TODO

   case PIPE_COMPUTE_CAP_MAX_VARIABLE_THREADS_PER_BLOCK:
      RET((uint64_t[]){ compiler->max_variable_workgroup_size });
   }

   return 0;
}

static const void *
fd_get_compiler_options(struct pipe_screen *pscreen, enum pipe_shader_ir ir,
                        enum pipe_shader_type shader)
{
   struct fd_screen *screen = fd_screen(pscreen);

   if (is_ir3(screen))
      return ir3_get_compiler_options(screen->compiler);

   return ir2_get_compiler_options();
}

static struct disk_cache *
fd_get_disk_shader_cache(struct pipe_screen *pscreen)
{
   struct fd_screen *screen = fd_screen(pscreen);

   if (is_ir3(screen)) {
      struct ir3_compiler *compiler = screen->compiler;
      return compiler->disk_cache;
   }

   return NULL;
}

bool
fd_screen_bo_get_handle(struct pipe_screen *pscreen, struct fd_bo *bo,
                        struct renderonly_scanout *scanout, unsigned stride,
                        struct winsys_handle *whandle)
{
   struct fd_screen *screen = fd_screen(pscreen);

   whandle->stride = stride;

   if (whandle->type == WINSYS_HANDLE_TYPE_SHARED) {
      return fd_bo_get_name(bo, &whandle->handle) == 0;
   } else if (whandle->type == WINSYS_HANDLE_TYPE_KMS) {
      if (screen->ro) {
         return renderonly_get_handle(scanout, whandle);
      } else {
         uint32_t handle = fd_bo_handle(bo);
         if (!handle)
            return false;
         whandle->handle = handle;
         return true;
      }
   } else if (whandle->type == WINSYS_HANDLE_TYPE_FD) {
      int fd = fd_bo_dmabuf(bo);
      if (fd < 0)
         return false;
      whandle->handle = fd;
      return true;
   } else {
      return false;
   }
}

static bool
is_format_supported(struct pipe_screen *pscreen,
                    enum pipe_format format,
                    uint64_t modifier)
{
   struct fd_screen *screen = fd_screen(pscreen);
   if (screen->is_format_supported)
      return screen->is_format_supported(pscreen, format, modifier);
   return modifier == DRM_FORMAT_MOD_LINEAR;
}

static void
fd_screen_query_dmabuf_modifiers(struct pipe_screen *pscreen,
                                 enum pipe_format format, int max,
                                 uint64_t *modifiers,
                                 unsigned int *external_only, int *count)
{
   const uint64_t all_modifiers[] = {
      DRM_FORMAT_MOD_LINEAR,
      DRM_FORMAT_MOD_QCOM_COMPRESSED,
      DRM_FORMAT_MOD_QCOM_TILED3,
   };

   int num = 0;

   for (int i = 0; i < ARRAY_SIZE(all_modifiers); i++) {
      if (!is_format_supported(pscreen, format, all_modifiers[i]))
         continue;

      if (num < max) {
         if (modifiers)
            modifiers[num] = all_modifiers[i];

         if (external_only)
            external_only[num] = false;
      }

      num++;
   }

   *count = num;
}

static bool
fd_screen_is_dmabuf_modifier_supported(struct pipe_screen *pscreen,
                                       uint64_t modifier,
                                       enum pipe_format format,
                                       bool *external_only)
{
   return is_format_supported(pscreen, format, modifier);
}

struct fd_bo *
fd_screen_bo_from_handle(struct pipe_screen *pscreen,
                         struct winsys_handle *whandle)
{
   struct fd_screen *screen = fd_screen(pscreen);
   struct fd_bo *bo;

   if (whandle->type == WINSYS_HANDLE_TYPE_SHARED) {
      bo = fd_bo_from_name(screen->dev, whandle->handle);
   } else if (whandle->type == WINSYS_HANDLE_TYPE_KMS) {
      bo = fd_bo_from_handle(screen->dev, whandle->handle, 0);
   } else if (whandle->type == WINSYS_HANDLE_TYPE_FD) {
      bo = fd_bo_from_dmabuf(screen->dev, whandle->handle);
   } else {
      DBG("Attempt to import unsupported handle type %d", whandle->type);
      return NULL;
   }

   if (!bo) {
      DBG("ref name 0x%08x failed", whandle->handle);
      return NULL;
   }

   return bo;
}

static void
_fd_fence_ref(struct pipe_screen *pscreen, struct pipe_fence_handle **ptr,
              struct pipe_fence_handle *pfence)
{
   fd_pipe_fence_ref(ptr, pfence);
}

static void
fd_screen_get_device_uuid(struct pipe_screen *pscreen, char *uuid)
{
   struct fd_screen *screen = fd_screen(pscreen);

   fd_get_device_uuid(uuid, screen->dev_id);
}

static void
fd_screen_get_driver_uuid(struct pipe_screen *pscreen, char *uuid)
{
   fd_get_driver_uuid(uuid);
}

static int
fd_screen_get_fd(struct pipe_screen *pscreen)
{
   struct fd_screen *screen = fd_screen(pscreen);
   return fd_device_fd(screen->dev);
}

struct pipe_screen *
fd_screen_create(int fd,
                 const struct pipe_screen_config *config,
                 struct renderonly *ro)
{
   struct fd_device *dev = fd_device_new_dup(fd);
   if (!dev)
      return NULL;

   struct fd_screen *screen = CALLOC_STRUCT(fd_screen);
   struct pipe_screen *pscreen;
   uint64_t val;

   fd_mesa_debug = debug_get_option_fd_mesa_debug();

   if (FD_DBG(NOBIN))
      fd_binning_enabled = false;

   if (!screen)
      return NULL;

#ifdef HAVE_PERFETTO
   fd_perfetto_init();
#endif

   util_gpuvis_init();

   pscreen = &screen->base;

   screen->dev = dev;
   screen->ro = ro;

   // maybe this should be in context?
   screen->pipe = fd_pipe_new(screen->dev, FD_PIPE_3D);
   if (!screen->pipe) {
      DBG("could not create 3d pipe");
      goto fail;
   }

   if (fd_pipe_get_param(screen->pipe, FD_GMEM_SIZE, &val)) {
      DBG("could not get GMEM size");
      goto fail;
   }
   screen->gmemsize_bytes = debug_get_num_option("FD_MESA_GMEM", val);

   if (fd_device_version(dev) >= FD_VERSION_GMEM_BASE) {
      fd_pipe_get_param(screen->pipe, FD_GMEM_BASE, &screen->gmem_base);
   }

   if (fd_pipe_get_param(screen->pipe, FD_MAX_FREQ, &val)) {
      DBG("could not get gpu freq");
      /* this limits what performance related queries are
       * supported but is not fatal
       */
      screen->max_freq = 0;
   } else {
      screen->max_freq = val;
   }

   if (fd_pipe_get_param(screen->pipe, FD_TIMESTAMP, &val) == 0)
      screen->has_timestamp = true;

   screen->dev_id = fd_pipe_dev_id(screen->pipe);

   if (fd_pipe_get_param(screen->pipe, FD_GPU_ID, &val)) {
      DBG("could not get gpu-id");
      goto fail;
   }
   screen->gpu_id = val;

   if (fd_pipe_get_param(screen->pipe, FD_CHIP_ID, &val)) {
      DBG("could not get chip-id");
      /* older kernels may not have this property: */
      unsigned core = screen->gpu_id / 100;
      unsigned major = (screen->gpu_id % 100) / 10;
      unsigned minor = screen->gpu_id % 10;
      unsigned patch = 0; /* assume the worst */
      val = (patch & 0xff) | ((minor & 0xff) << 8) | ((major & 0xff) << 16) |
            ((core & 0xff) << 24);
   }
   screen->chip_id = val;
   screen->gen = fd_dev_gen(screen->dev_id);

   if (fd_pipe_get_param(screen->pipe, FD_NR_PRIORITIES, &val)) {
      DBG("could not get # of rings");
      screen->priority_mask = 0;
   } else {
      /* # of rings equates to number of unique priority values: */
      screen->priority_mask = (1 << val) - 1;

      /* Lowest numerical value (ie. zero) is highest priority: */
      screen->prio_high = 0;

      /* Highest numerical value is lowest priority: */
      screen->prio_low = val - 1;

      /* Pick midpoint for normal priority.. note that whatever the
       * range of possible priorities, since we divide by 2 the
       * result will either be an integer or an integer plus 0.5,
       * in which case it will round down to an integer, so int
       * division will give us an appropriate result in either
       * case:
       */
      screen->prio_norm = val / 2;
   }

   if (fd_device_version(dev) >= FD_VERSION_ROBUSTNESS)
      screen->has_robustness = true;

   screen->has_syncobj = fd_has_syncobj(screen->dev);

   /* parse driconf configuration now for device specific overrides: */
   driParseConfigFiles(config->options, config->options_info, 0, "msm",
                       NULL, fd_dev_name(screen->dev_id), NULL, 0, NULL, 0);

   screen->driconf.conservative_lrz =
         !driQueryOptionb(config->options, "disable_conservative_lrz");
   screen->driconf.enable_throttling =
         !driQueryOptionb(config->options, "disable_throttling");

   struct sysinfo si;
   sysinfo(&si);
   screen->ram_size = si.totalram;

   DBG("Pipe Info:");
   DBG(" GPU-id:          %s", fd_dev_name(screen->dev_id));
   DBG(" Chip-id:         0x%016"PRIx64, screen->chip_id);
   DBG(" GMEM size:       0x%08x", screen->gmemsize_bytes);

   const struct fd_dev_info info = fd_dev_info(screen->dev_id);
   if (!info.chip) {
      mesa_loge("unsupported GPU: a%03d", screen->gpu_id);
      goto fail;
   }

   screen->dev_info = info;
   screen->info = &screen->dev_info;

   /* explicitly checking for GPU revisions that are known to work.  This
    * may be overly conservative for a3xx, where spoofing the gpu_id with
    * the blob driver seems to generate identical cmdstream dumps.  But
    * on a2xx, there seem to be small differences between the GPU revs
    * so it is probably better to actually test first on real hardware
    * before enabling:
    *
    * If you have a different adreno version, feel free to add it to one
    * of the cases below and see what happens.  And if it works, please
    * send a patch ;-)
    */
   switch (screen->gen) {
   case 2:
      fd2_screen_init(pscreen);
      break;
   case 3:
      fd3_screen_init(pscreen);
      break;
   case 4:
      fd4_screen_init(pscreen);
      break;
   case 5:
      fd5_screen_init(pscreen);
      break;
   case 6:
      fd6_screen_init(pscreen);
      break;
   default:
      mesa_loge("unsupported GPU generation: a%uxx", screen->gen);
      goto fail;
   }

   /* fdN_screen_init() should set this: */
   assert(screen->primtypes);
   screen->primtypes_mask = 0;
   for (unsigned i = 0; i <= MESA_PRIM_COUNT; i++)
      if (screen->primtypes[i])
         screen->primtypes_mask |= (1 << i);

   if (FD_DBG(PERFC)) {
      screen->perfcntr_groups =
         fd_perfcntrs(screen->dev_id, &screen->num_perfcntr_groups);
   }

   /* NOTE: don't enable if we have too old of a kernel to support
    * growable cmdstream buffers, since memory requirement for cmdstream
    * buffers would be too much otherwise.
    */
   if (fd_device_version(dev) >= FD_VERSION_UNLIMITED_CMDS)
      screen->reorder = !FD_DBG(INORDER);

   fd_bc_init(&screen->batch_cache);

   list_inithead(&screen->context_list);

   util_idalloc_mt_init_tc(&screen->buffer_ids);

   (void)simple_mtx_init(&screen->lock, mtx_plain);

   pscreen->destroy = fd_screen_destroy;
   pscreen->get_screen_fd = fd_screen_get_fd;
   pscreen->query_memory_info = fd_query_memory_info;
   pscreen->get_param = fd_screen_get_param;
   pscreen->get_paramf = fd_screen_get_paramf;
   pscreen->get_shader_param = fd_screen_get_shader_param;
   pscreen->get_compute_param = fd_get_compute_param;
   pscreen->get_compiler_options = fd_get_compiler_options;
   pscreen->get_disk_shader_cache = fd_get_disk_shader_cache;

   fd_resource_screen_init(pscreen);
   fd_query_screen_init(pscreen);
   fd_gmem_screen_init(pscreen);

   pscreen->get_name = fd_screen_get_name;
   pscreen->get_vendor = fd_screen_get_vendor;
   pscreen->get_device_vendor = fd_screen_get_device_vendor;

   pscreen->get_sample_pixel_grid = fd_get_sample_pixel_grid;

   pscreen->get_timestamp = fd_screen_get_timestamp;

   pscreen->fence_reference = _fd_fence_ref;
   pscreen->fence_finish = fd_pipe_fence_finish;
   pscreen->fence_get_fd = fd_pipe_fence_get_fd;

   pscreen->query_dmabuf_modifiers = fd_screen_query_dmabuf_modifiers;
   pscreen->is_dmabuf_modifier_supported =
      fd_screen_is_dmabuf_modifier_supported;

   pscreen->get_device_uuid = fd_screen_get_device_uuid;
   pscreen->get_driver_uuid = fd_screen_get_driver_uuid;

   slab_create_parent(&screen->transfer_pool, sizeof(struct fd_transfer), 16);

   simple_mtx_init(&screen->aux_ctx_lock, mtx_plain);

   return pscreen;

fail:
   fd_screen_destroy(pscreen);
   return NULL;
}

struct fd_context *
fd_screen_aux_context_get(struct pipe_screen *pscreen)
{
   struct fd_screen *screen = fd_screen(pscreen);

   simple_mtx_lock(&screen->aux_ctx_lock);

   if (!screen->aux_ctx) {
      screen->aux_ctx = pscreen->context_create(pscreen, NULL, 0);
   }

   return fd_context(screen->aux_ctx);
}

void
fd_screen_aux_context_put(struct pipe_screen *pscreen)
{
   struct fd_screen *screen = fd_screen(pscreen);

   screen->aux_ctx->flush(screen->aux_ctx, NULL, 0);
   simple_mtx_unlock(&screen->aux_ctx_lock);
}
