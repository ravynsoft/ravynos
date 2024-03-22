/*
 * Copyright 2012 Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Ben Skeggs
 *
 */

#include <xf86drm.h>
#include <nouveau_drm.h>
#include "util/format/u_format.h"
#include "util/format/u_format_s3tc.h"
#include "util/u_screen.h"

#include "nv_object.xml.h"
#include "nv_m2mf.xml.h"
#include "nv30/nv30-40_3d.xml.h"
#include "nv30/nv01_2d.xml.h"

#include "nouveau_fence.h"
#include "nv30/nv30_screen.h"
#include "nv30/nv30_context.h"
#include "nv30/nv30_resource.h"
#include "nv30/nv30_format.h"
#include "nv30/nv30_winsys.h"

#define RANKINE_0397_CHIPSET 0x00000003
#define RANKINE_0497_CHIPSET 0x000001e0
#define RANKINE_0697_CHIPSET 0x00000010
#define CURIE_4097_CHIPSET   0x00000baf
#define CURIE_4497_CHIPSET   0x00005450
#define CURIE_4497_CHIPSET6X 0x00000088

static int
nv30_screen_get_param(struct pipe_screen *pscreen, enum pipe_cap param)
{
   struct nv30_screen *screen = nv30_screen(pscreen);
   struct nouveau_object *eng3d = screen->eng3d;
   struct nouveau_device *dev = nouveau_screen(pscreen)->device;

   switch (param) {
   /* non-boolean capabilities */
   case PIPE_CAP_MAX_RENDER_TARGETS:
      return (eng3d->oclass >= NV40_3D_CLASS) ? 4 : 1;
   case PIPE_CAP_MAX_TEXTURE_2D_SIZE:
      return 4096;
   case PIPE_CAP_MAX_TEXTURE_3D_LEVELS:
      return 10;
   case PIPE_CAP_MAX_TEXTURE_CUBE_LEVELS:
      return 13;
   case PIPE_CAP_GLSL_FEATURE_LEVEL:
   case PIPE_CAP_GLSL_FEATURE_LEVEL_COMPATIBILITY:
      return 120;
   case PIPE_CAP_ENDIANNESS:
      return PIPE_ENDIAN_LITTLE;
   case PIPE_CAP_CONSTANT_BUFFER_OFFSET_ALIGNMENT:
      return 16;
   case PIPE_CAP_MIN_MAP_BUFFER_ALIGNMENT:
      return NOUVEAU_MIN_BUFFER_MAP_ALIGN;
   case PIPE_CAP_MAX_VIEWPORTS:
      return 1;
   case PIPE_CAP_MAX_VERTEX_ATTRIB_STRIDE:
      return 2048;
   case PIPE_CAP_MAX_TEXTURE_UPLOAD_MEMORY_BUDGET:
      return 8 * 1024 * 1024;
   case PIPE_CAP_MAX_VARYINGS:
      return 8;

   /* supported capabilities */
   case PIPE_CAP_ANISOTROPIC_FILTER:
   case PIPE_CAP_OCCLUSION_QUERY:
   case PIPE_CAP_QUERY_TIME_ELAPSED:
   case PIPE_CAP_QUERY_TIMESTAMP:
   case PIPE_CAP_TEXTURE_SWIZZLE:
   case PIPE_CAP_DEPTH_CLIP_DISABLE:
   case PIPE_CAP_FS_COORD_ORIGIN_UPPER_LEFT:
   case PIPE_CAP_FS_COORD_ORIGIN_LOWER_LEFT:
   case PIPE_CAP_FS_COORD_PIXEL_CENTER_HALF_INTEGER:
   case PIPE_CAP_FS_COORD_PIXEL_CENTER_INTEGER:
   case PIPE_CAP_TGSI_TEXCOORD:
   case PIPE_CAP_BUFFER_MAP_PERSISTENT_COHERENT:
   case PIPE_CAP_CLEAR_SCISSORED:
   case PIPE_CAP_VERTEX_BUFFER_OFFSET_4BYTE_ALIGNED_ONLY:
   case PIPE_CAP_VERTEX_BUFFER_STRIDE_4BYTE_ALIGNED_ONLY:
   case PIPE_CAP_VERTEX_ELEMENT_SRC_OFFSET_4BYTE_ALIGNED_ONLY:
   case PIPE_CAP_ALLOW_MAPPED_BUFFERS_DURING_EXECUTION:
   case PIPE_CAP_QUERY_MEMORY_INFO:
      return 1;
   case PIPE_CAP_TEXTURE_TRANSFER_MODES:
      return PIPE_TEXTURE_TRANSFER_BLIT;
   /* nv35 capabilities */
   case PIPE_CAP_DEPTH_BOUNDS_TEST:
      return eng3d->oclass == NV35_3D_CLASS || eng3d->oclass >= NV40_3D_CLASS;
   case PIPE_CAP_SUPPORTED_PRIM_MODES_WITH_RESTART:
   case PIPE_CAP_SUPPORTED_PRIM_MODES:
      return BITFIELD_MASK(MESA_PRIM_COUNT);
   /* nv4x capabilities */
   case PIPE_CAP_BLEND_EQUATION_SEPARATE:
   case PIPE_CAP_NPOT_TEXTURES:
   case PIPE_CAP_CONDITIONAL_RENDER:
   case PIPE_CAP_TEXTURE_MIRROR_CLAMP:
   case PIPE_CAP_TEXTURE_MIRROR_CLAMP_TO_EDGE:
   case PIPE_CAP_PRIMITIVE_RESTART:
   case PIPE_CAP_PRIMITIVE_RESTART_FIXED_INDEX:
      return (eng3d->oclass >= NV40_3D_CLASS) ? 1 : 0;
   /* unsupported */
   case PIPE_CAP_EMULATE_NONFIXED_PRIMITIVE_RESTART:
   case PIPE_CAP_DEPTH_CLIP_DISABLE_SEPARATE:
   case PIPE_CAP_MAX_DUAL_SOURCE_RENDER_TARGETS:
   case PIPE_CAP_FRAGMENT_SHADER_TEXTURE_LOD:
   case PIPE_CAP_FRAGMENT_SHADER_DERIVATIVES:
   case PIPE_CAP_INDEP_BLEND_ENABLE:
   case PIPE_CAP_INDEP_BLEND_FUNC:
   case PIPE_CAP_MAX_TEXTURE_ARRAY_LAYERS:
   case PIPE_CAP_SHADER_STENCIL_EXPORT:
   case PIPE_CAP_VS_INSTANCEID:
   case PIPE_CAP_VERTEX_ELEMENT_INSTANCE_DIVISOR: /* XXX: yes? */
   case PIPE_CAP_MAX_STREAM_OUTPUT_BUFFERS:
   case PIPE_CAP_STREAM_OUTPUT_PAUSE_RESUME:
   case PIPE_CAP_STREAM_OUTPUT_INTERLEAVE_BUFFERS:
   case PIPE_CAP_MIN_TEXEL_OFFSET:
   case PIPE_CAP_MAX_TEXEL_OFFSET:
   case PIPE_CAP_MIN_TEXTURE_GATHER_OFFSET:
   case PIPE_CAP_MAX_TEXTURE_GATHER_OFFSET:
   case PIPE_CAP_MAX_STREAM_OUTPUT_SEPARATE_COMPONENTS:
   case PIPE_CAP_MAX_STREAM_OUTPUT_INTERLEAVED_COMPONENTS:
   case PIPE_CAP_MAX_GEOMETRY_OUTPUT_VERTICES:
   case PIPE_CAP_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS:
   case PIPE_CAP_MAX_VERTEX_STREAMS:
   case PIPE_CAP_TGSI_CAN_COMPACT_CONSTANTS:
   case PIPE_CAP_TEXTURE_BARRIER:
   case PIPE_CAP_SEAMLESS_CUBE_MAP:
   case PIPE_CAP_SEAMLESS_CUBE_MAP_PER_TEXTURE:
   case PIPE_CAP_CUBE_MAP_ARRAY:
   case PIPE_CAP_VERTEX_COLOR_UNCLAMPED:
   case PIPE_CAP_FRAGMENT_COLOR_CLAMPED:
   case PIPE_CAP_VERTEX_COLOR_CLAMPED:
   case PIPE_CAP_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION:
   case PIPE_CAP_MIXED_COLORBUFFER_FORMATS:
   case PIPE_CAP_START_INSTANCE:
   case PIPE_CAP_TEXTURE_MULTISAMPLE:
   case PIPE_CAP_TEXTURE_BUFFER_OBJECTS:
   case PIPE_CAP_TEXTURE_BUFFER_OFFSET_ALIGNMENT:
   case PIPE_CAP_QUERY_PIPELINE_STATISTICS:
   case PIPE_CAP_TEXTURE_BORDER_COLOR_QUIRK:
   case PIPE_CAP_MAX_TEXEL_BUFFER_ELEMENTS_UINT:
   case PIPE_CAP_MIXED_FRAMEBUFFER_SIZES:
   case PIPE_CAP_VS_LAYER_VIEWPORT:
   case PIPE_CAP_MAX_TEXTURE_GATHER_COMPONENTS:
   case PIPE_CAP_TEXTURE_GATHER_SM5:
   case PIPE_CAP_FAKE_SW_MSAA:
   case PIPE_CAP_TEXTURE_QUERY_LOD:
   case PIPE_CAP_SAMPLE_SHADING:
   case PIPE_CAP_TEXTURE_GATHER_OFFSETS:
   case PIPE_CAP_VS_WINDOW_SPACE_POSITION:
   case PIPE_CAP_USER_VERTEX_BUFFERS:
   case PIPE_CAP_COMPUTE:
   case PIPE_CAP_DRAW_INDIRECT:
   case PIPE_CAP_MULTI_DRAW_INDIRECT:
   case PIPE_CAP_MULTI_DRAW_INDIRECT_PARAMS:
   case PIPE_CAP_FS_FINE_DERIVATIVE:
   case PIPE_CAP_CONDITIONAL_RENDER_INVERTED:
   case PIPE_CAP_SAMPLER_VIEW_TARGET:
   case PIPE_CAP_CLIP_HALFZ:
   case PIPE_CAP_POLYGON_OFFSET_CLAMP:
   case PIPE_CAP_MULTISAMPLE_Z_RESOLVE:
   case PIPE_CAP_RESOURCE_FROM_USER_MEMORY:
   case PIPE_CAP_DEVICE_RESET_STATUS_QUERY:
   case PIPE_CAP_MAX_SHADER_PATCH_VARYINGS:
   case PIPE_CAP_TEXTURE_FLOAT_LINEAR:
   case PIPE_CAP_TEXTURE_HALF_FLOAT_LINEAR:
   case PIPE_CAP_TEXTURE_QUERY_SAMPLES:
   case PIPE_CAP_FORCE_PERSAMPLE_INTERP:
   case PIPE_CAP_COPY_BETWEEN_COMPRESSED_AND_PLAIN_FORMATS:
   case PIPE_CAP_SHAREABLE_SHADERS:
   case PIPE_CAP_DRAW_PARAMETERS:
   case PIPE_CAP_SHADER_PACK_HALF_FLOAT:
   case PIPE_CAP_FS_POSITION_IS_SYSVAL:
   case PIPE_CAP_FS_FACE_IS_INTEGER_SYSVAL:
   case PIPE_CAP_SHADER_BUFFER_OFFSET_ALIGNMENT:
   case PIPE_CAP_INVALIDATE_BUFFER:
   case PIPE_CAP_GENERATE_MIPMAP:
   case PIPE_CAP_STRING_MARKER:
   case PIPE_CAP_BUFFER_SAMPLER_VIEW_RGBA_ONLY:
   case PIPE_CAP_SURFACE_REINTERPRET_BLOCKS:
   case PIPE_CAP_QUERY_BUFFER_OBJECT:
   case PIPE_CAP_PCI_GROUP:
   case PIPE_CAP_PCI_BUS:
   case PIPE_CAP_PCI_DEVICE:
   case PIPE_CAP_PCI_FUNCTION:
   case PIPE_CAP_FRAMEBUFFER_NO_ATTACHMENT:
   case PIPE_CAP_ROBUST_BUFFER_ACCESS_BEHAVIOR:
   case PIPE_CAP_CULL_DISTANCE:
   case PIPE_CAP_SHADER_GROUP_VOTE:
   case PIPE_CAP_MAX_WINDOW_RECTANGLES:
   case PIPE_CAP_POLYGON_OFFSET_UNITS_UNSCALED:
   case PIPE_CAP_VIEWPORT_SUBPIXEL_BITS:
   case PIPE_CAP_MIXED_COLOR_DEPTH_BITS:
   case PIPE_CAP_SHADER_ARRAY_COMPONENTS:
   case PIPE_CAP_SHADER_CAN_READ_OUTPUTS:
   case PIPE_CAP_NATIVE_FENCE_FD:
   case PIPE_CAP_FBFETCH:
   case PIPE_CAP_LEGACY_MATH_RULES:
   case PIPE_CAP_DOUBLES:
   case PIPE_CAP_INT64:
   case PIPE_CAP_TGSI_TEX_TXF_LZ:
   case PIPE_CAP_SHADER_CLOCK:
   case PIPE_CAP_POLYGON_MODE_FILL_RECTANGLE:
   case PIPE_CAP_SPARSE_BUFFER_PAGE_SIZE:
   case PIPE_CAP_SHADER_BALLOT:
   case PIPE_CAP_TES_LAYER_VIEWPORT:
   case PIPE_CAP_CAN_BIND_CONST_BUFFER_AS_VERTEX:
   case PIPE_CAP_POST_DEPTH_COVERAGE:
   case PIPE_CAP_BINDLESS_TEXTURE:
   case PIPE_CAP_NIR_SAMPLERS_AS_DEREF:
   case PIPE_CAP_QUERY_SO_OVERFLOW:
   case PIPE_CAP_MEMOBJ:
   case PIPE_CAP_LOAD_CONSTBUF:
   case PIPE_CAP_TILE_RASTER_ORDER:
   case PIPE_CAP_MAX_COMBINED_SHADER_OUTPUT_RESOURCES:
   case PIPE_CAP_FRAMEBUFFER_MSAA_CONSTRAINTS:
   case PIPE_CAP_SIGNED_VERTEX_BUFFER_OFFSET:
   case PIPE_CAP_CONTEXT_PRIORITY_MASK:
   case PIPE_CAP_FENCE_SIGNAL:
   case PIPE_CAP_CONSTBUF0_FLAGS:
   case PIPE_CAP_PACKED_UNIFORMS:
   case PIPE_CAP_CONSERVATIVE_RASTER_POST_SNAP_TRIANGLES:
   case PIPE_CAP_CONSERVATIVE_RASTER_POST_SNAP_POINTS_LINES:
   case PIPE_CAP_CONSERVATIVE_RASTER_PRE_SNAP_TRIANGLES:
   case PIPE_CAP_CONSERVATIVE_RASTER_PRE_SNAP_POINTS_LINES:
   case PIPE_CAP_CONSERVATIVE_RASTER_POST_DEPTH_COVERAGE:
   case PIPE_CAP_MAX_CONSERVATIVE_RASTER_SUBPIXEL_PRECISION_BIAS:
   case PIPE_CAP_PROGRAMMABLE_SAMPLE_LOCATIONS:
   case PIPE_CAP_IMAGE_LOAD_FORMATTED:
   case PIPE_CAP_TGSI_DIV:
   case PIPE_CAP_IMAGE_ATOMIC_INC_WRAP:
   case PIPE_CAP_IMAGE_STORE_FORMATTED:
      return 0;

   case PIPE_CAP_MAX_GS_INVOCATIONS:
      return 32;
   case PIPE_CAP_MAX_SHADER_BUFFER_SIZE_UINT:
      return 1 << 27;
   case PIPE_CAP_VENDOR_ID:
      return 0x10de;
   case PIPE_CAP_DEVICE_ID: {
      uint64_t device_id;
      if (nouveau_getparam(dev, NOUVEAU_GETPARAM_PCI_DEVICE, &device_id)) {
         NOUVEAU_ERR("NOUVEAU_GETPARAM_PCI_DEVICE failed.\n");
         return -1;
      }
      return device_id;
   }
   case PIPE_CAP_ACCELERATED:
      return 1;
   case PIPE_CAP_VIDEO_MEMORY:
      return dev->vram_size >> 20;
   case PIPE_CAP_UMA:
      return 0;
   default:
      return u_pipe_screen_get_param_defaults(pscreen, param);
   }
}

static float
nv30_screen_get_paramf(struct pipe_screen *pscreen, enum pipe_capf param)
{
   struct nv30_screen *screen = nv30_screen(pscreen);
   struct nouveau_object *eng3d = screen->eng3d;

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
      return 10.0;
   case PIPE_CAPF_MAX_POINT_SIZE:
   case PIPE_CAPF_MAX_POINT_SIZE_AA:
      return 64.0;
   case PIPE_CAPF_MAX_TEXTURE_ANISOTROPY:
      return (eng3d->oclass >= NV40_3D_CLASS) ? 16.0 : 8.0;
   case PIPE_CAPF_MAX_TEXTURE_LOD_BIAS:
      return 15.0;
   case PIPE_CAPF_MIN_CONSERVATIVE_RASTER_DILATE:
   case PIPE_CAPF_MAX_CONSERVATIVE_RASTER_DILATE:
   case PIPE_CAPF_CONSERVATIVE_RASTER_DILATE_GRANULARITY:
      return 0.0;
   default:
      debug_printf("unknown paramf %d\n", param);
      return 0;
   }
}

static int
nv30_screen_get_shader_param(struct pipe_screen *pscreen,
                             enum pipe_shader_type shader,
                             enum pipe_shader_cap param)
{
   struct nv30_screen *screen = nv30_screen(pscreen);
   struct nouveau_object *eng3d = screen->eng3d;

   switch (shader) {
   case PIPE_SHADER_VERTEX:
      switch (param) {
      case PIPE_SHADER_CAP_MAX_INSTRUCTIONS:
      case PIPE_SHADER_CAP_MAX_ALU_INSTRUCTIONS:
         return (eng3d->oclass >= NV40_3D_CLASS) ? 512 : 256;
      case PIPE_SHADER_CAP_MAX_TEX_INSTRUCTIONS:
      case PIPE_SHADER_CAP_MAX_TEX_INDIRECTIONS:
         return (eng3d->oclass >= NV40_3D_CLASS) ? 512 : 0;
      case PIPE_SHADER_CAP_MAX_CONTROL_FLOW_DEPTH:
         return 0;
      case PIPE_SHADER_CAP_MAX_INPUTS:
      case PIPE_SHADER_CAP_MAX_OUTPUTS:
         return 16;
      case PIPE_SHADER_CAP_MAX_CONST_BUFFER0_SIZE:
         return ((eng3d->oclass >= NV40_3D_CLASS) ? (468 - 6): (256 - 6)) * sizeof(float[4]);
      case PIPE_SHADER_CAP_MAX_CONST_BUFFERS:
         return 1;
      case PIPE_SHADER_CAP_MAX_TEMPS:
         return (eng3d->oclass >= NV40_3D_CLASS) ? 32 : 13;
      case PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS:
      case PIPE_SHADER_CAP_MAX_SAMPLER_VIEWS:
         return 0;
      case PIPE_SHADER_CAP_CONT_SUPPORTED:
      case PIPE_SHADER_CAP_TGSI_SQRT_SUPPORTED:
      case PIPE_SHADER_CAP_INDIRECT_INPUT_ADDR:
      case PIPE_SHADER_CAP_INDIRECT_OUTPUT_ADDR:
      case PIPE_SHADER_CAP_INDIRECT_TEMP_ADDR:
      case PIPE_SHADER_CAP_INDIRECT_CONST_ADDR:
      case PIPE_SHADER_CAP_SUBROUTINES:
      case PIPE_SHADER_CAP_INTEGERS:
      case PIPE_SHADER_CAP_INT64_ATOMICS:
      case PIPE_SHADER_CAP_FP16:
      case PIPE_SHADER_CAP_FP16_DERIVATIVES:
      case PIPE_SHADER_CAP_FP16_CONST_BUFFERS:
      case PIPE_SHADER_CAP_INT16:
      case PIPE_SHADER_CAP_GLSL_16BIT_CONSTS:
      case PIPE_SHADER_CAP_TGSI_ANY_INOUT_DECL_RANGE:
      case PIPE_SHADER_CAP_MAX_SHADER_BUFFERS:
      case PIPE_SHADER_CAP_MAX_SHADER_IMAGES:
      case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTERS:
      case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTER_BUFFERS:
         return 0;
      case PIPE_SHADER_CAP_SUPPORTED_IRS:
         return (1 << PIPE_SHADER_IR_NIR) | (1 << PIPE_SHADER_IR_TGSI);
      default:
         debug_printf("unknown vertex shader param %d\n", param);
         return 0;
      }
      break;
   case PIPE_SHADER_FRAGMENT:
      switch (param) {
      case PIPE_SHADER_CAP_MAX_INSTRUCTIONS:
      case PIPE_SHADER_CAP_MAX_ALU_INSTRUCTIONS:
      case PIPE_SHADER_CAP_MAX_TEX_INSTRUCTIONS:
      case PIPE_SHADER_CAP_MAX_TEX_INDIRECTIONS:
         return 4096;
      case PIPE_SHADER_CAP_MAX_CONTROL_FLOW_DEPTH:
         return 0;
      case PIPE_SHADER_CAP_MAX_INPUTS:
         return 8; /* should be possible to do 10 with nv4x */
      case PIPE_SHADER_CAP_MAX_OUTPUTS:
         return 4;
      case PIPE_SHADER_CAP_MAX_CONST_BUFFER0_SIZE:
         return ((eng3d->oclass >= NV40_3D_CLASS) ? 224 : 32) * sizeof(float[4]);
      case PIPE_SHADER_CAP_MAX_CONST_BUFFERS:
         return 1;
      case PIPE_SHADER_CAP_MAX_TEMPS:
         return 32;
      case PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS:
      case PIPE_SHADER_CAP_MAX_SAMPLER_VIEWS:
         return 16;
      case PIPE_SHADER_CAP_CONT_SUPPORTED:
      case PIPE_SHADER_CAP_TGSI_SQRT_SUPPORTED:
      case PIPE_SHADER_CAP_INDIRECT_INPUT_ADDR:
      case PIPE_SHADER_CAP_INDIRECT_OUTPUT_ADDR:
      case PIPE_SHADER_CAP_INDIRECT_TEMP_ADDR:
      case PIPE_SHADER_CAP_INDIRECT_CONST_ADDR:
      case PIPE_SHADER_CAP_SUBROUTINES:
      case PIPE_SHADER_CAP_INTEGERS:
      case PIPE_SHADER_CAP_FP16:
      case PIPE_SHADER_CAP_FP16_DERIVATIVES:
      case PIPE_SHADER_CAP_FP16_CONST_BUFFERS:
      case PIPE_SHADER_CAP_INT16:
      case PIPE_SHADER_CAP_GLSL_16BIT_CONSTS:
      case PIPE_SHADER_CAP_TGSI_ANY_INOUT_DECL_RANGE:
      case PIPE_SHADER_CAP_MAX_SHADER_BUFFERS:
      case PIPE_SHADER_CAP_MAX_SHADER_IMAGES:
      case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTERS:
      case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTER_BUFFERS:
         return 0;
      case PIPE_SHADER_CAP_SUPPORTED_IRS:
         return (1 << PIPE_SHADER_IR_NIR) | (1 << PIPE_SHADER_IR_TGSI);
      default:
         debug_printf("unknown fragment shader param %d\n", param);
         return 0;
      }
      break;
   default:
      return 0;
   }
}

static bool
nv30_screen_is_format_supported(struct pipe_screen *pscreen,
                                enum pipe_format format,
                                enum pipe_texture_target target,
                                unsigned sample_count,
                                unsigned storage_sample_count,
                                unsigned bindings)
{
   if (sample_count > nv30_screen(pscreen)->max_sample_count)
      return false;

   if (!(0x00000017 & (1 << sample_count)))
      return false;

   if (MAX2(1, sample_count) != MAX2(1, storage_sample_count))
      return false;

   /* No way to render to a swizzled 3d texture. We don't necessarily know if
    * it's swizzled or not here, but we have to assume anyways.
    */
   if (target == PIPE_TEXTURE_3D && (bindings & PIPE_BIND_RENDER_TARGET))
      return false;

   /* shared is always supported */
   bindings &= ~PIPE_BIND_SHARED;

   if (bindings & PIPE_BIND_INDEX_BUFFER) {
      if (format != PIPE_FORMAT_R8_UINT &&
          format != PIPE_FORMAT_R16_UINT &&
          format != PIPE_FORMAT_R32_UINT)
         return false;
      bindings &= ~PIPE_BIND_INDEX_BUFFER;
   }

   return (nv30_format_info(pscreen, format)->bindings & bindings) == bindings;
}

static const nir_shader_compiler_options nv30_base_compiler_options = {
   .fuse_ffma32 = true,
   .fuse_ffma64 = true,
   .lower_bitops = true,
   .lower_extract_byte = true,
   .lower_extract_word = true,
   .lower_fdiv = true,
   .lower_fsat = true,
   .lower_insert_byte = true,
   .lower_insert_word = true,
   .lower_fdph = true,
   .lower_flrp32 = true,
   .lower_flrp64 = true,
   .lower_fmod = true,
   .lower_fpow = true, /* In hardware as of nv40 FS */
   .lower_uniforms_to_ubo = true,
   .lower_vector_cmp = true,
   .force_indirect_unrolling = nir_var_all,
   .force_indirect_unrolling_sampler = true,
   .max_unroll_iterations = 32,
   .no_integers = true,

   .use_interpolated_input_intrinsics = true,
};

static const void *
nv30_screen_get_compiler_options(struct pipe_screen *pscreen,
                                 enum pipe_shader_ir ir,
                                 enum pipe_shader_type shader)
{
   struct nv30_screen *screen = nv30_screen(pscreen);
   assert(ir == PIPE_SHADER_IR_NIR);

   /* The FS compiler options are different between nv30 and nv40, and are set
    * up at screen creation time.
    */
   if (shader == PIPE_SHADER_FRAGMENT)
      return &screen->fs_compiler_options;

   return &nv30_base_compiler_options;
}

static void
nv30_screen_fence_emit(struct pipe_context *pcontext, uint32_t *sequence,
                       struct nouveau_bo *wait)
{
   struct nv30_context *nv30 = nv30_context(pcontext);
   struct nv30_screen *screen = nv30->screen;
   struct nouveau_pushbuf *push = nv30->base.pushbuf;
   struct nouveau_pushbuf_refn ref = { wait, NOUVEAU_BO_GART | NOUVEAU_BO_RDWR };

   *sequence = ++screen->base.fence.sequence;

   assert(PUSH_AVAIL(push) + push->rsvd_kick >= 3);
   PUSH_DATA (push, NV30_3D_FENCE_OFFSET |
              (2 /* size */ << 18) | (7 /* subchan */ << 13));
   PUSH_DATA (push, 0);
   PUSH_DATA (push, *sequence);

   nouveau_pushbuf_refn(push, &ref, 1);
}

static uint32_t
nv30_screen_fence_update(struct pipe_screen *pscreen)
{
   struct nv30_screen *screen = nv30_screen(pscreen);
   struct nv04_notify *fence = screen->fence->data;
   return *(uint32_t *)((char *)screen->notify->map + fence->offset);
}

static void
nv30_screen_destroy(struct pipe_screen *pscreen)
{
   struct nv30_screen *screen = nv30_screen(pscreen);

   if (!nouveau_drm_screen_unref(&screen->base))
      return;

   nouveau_bo_ref(NULL, &screen->notify);

   nouveau_heap_destroy(&screen->query_heap);
   nouveau_heap_destroy(&screen->vp_exec_heap);
   nouveau_heap_destroy(&screen->vp_data_heap);

   nouveau_object_del(&screen->query);
   nouveau_object_del(&screen->fence);
   nouveau_object_del(&screen->ntfy);

   nouveau_object_del(&screen->sifm);
   nouveau_object_del(&screen->swzsurf);
   nouveau_object_del(&screen->surf2d);
   nouveau_object_del(&screen->m2mf);
   nouveau_object_del(&screen->eng3d);
   nouveau_object_del(&screen->null);

   nouveau_screen_fini(&screen->base);
   FREE(screen);
}

#define FAIL_SCREEN_INIT(str, err)                    \
   do {                                               \
      NOUVEAU_ERR(str, err);                          \
      screen->base.base.context_create = NULL;        \
      return &screen->base;                           \
   } while(0)

struct nouveau_screen *
nv30_screen_create(struct nouveau_device *dev)
{
   struct nv30_screen *screen;
   struct pipe_screen *pscreen;
   struct nouveau_pushbuf *push;
   struct nv04_fifo *fifo;
   unsigned oclass = 0;
   int ret, i;

   switch (dev->chipset & 0xf0) {
   case 0x30:
      if (RANKINE_0397_CHIPSET & (1 << (dev->chipset & 0x0f)))
         oclass = NV30_3D_CLASS;
      else
      if (RANKINE_0697_CHIPSET & (1 << (dev->chipset & 0x0f)))
         oclass = NV34_3D_CLASS;
      else
      if (RANKINE_0497_CHIPSET & (1 << (dev->chipset & 0x0f)))
         oclass = NV35_3D_CLASS;
      break;
   case 0x40:
      if (CURIE_4097_CHIPSET & (1 << (dev->chipset & 0x0f)))
         oclass = NV40_3D_CLASS;
      else
      if (CURIE_4497_CHIPSET & (1 << (dev->chipset & 0x0f)))
         oclass = NV44_3D_CLASS;
      break;
   case 0x60:
      if (CURIE_4497_CHIPSET6X & (1 << (dev->chipset & 0x0f)))
         oclass = NV44_3D_CLASS;
      break;
   default:
      break;
   }

   if (!oclass) {
      NOUVEAU_ERR("unknown 3d class for 0x%02x\n", dev->chipset);
      return NULL;
   }

   screen = CALLOC_STRUCT(nv30_screen);
   if (!screen)
      return NULL;

   pscreen = &screen->base.base;
   pscreen->destroy = nv30_screen_destroy;

   /*
    * Some modern apps try to use msaa without keeping in mind the
    * restrictions on videomem of older cards. Resulting in dmesg saying:
    * [ 1197.850642] nouveau E[soffice.bin[3785]] fail ttm_validate
    * [ 1197.850648] nouveau E[soffice.bin[3785]] validating bo list
    * [ 1197.850654] nouveau E[soffice.bin[3785]] validate: -12
    *
    * Because we are running out of video memory, after which the program
    * using the msaa visual freezes, and eventually the entire system freezes.
    *
    * To work around this we do not allow msaa visauls by default and allow
    * the user to override this via NV30_MAX_MSAA.
    */
   screen->max_sample_count = debug_get_num_option("NV30_MAX_MSAA", 0);
   if (screen->max_sample_count > 4)
      screen->max_sample_count = 4;

   pscreen->get_param = nv30_screen_get_param;
   pscreen->get_paramf = nv30_screen_get_paramf;
   pscreen->get_shader_param = nv30_screen_get_shader_param;
   pscreen->context_create = nv30_context_create;
   pscreen->is_format_supported = nv30_screen_is_format_supported;
   pscreen->get_compiler_options = nv30_screen_get_compiler_options;

   nv30_resource_screen_init(pscreen);
   nouveau_screen_init_vdec(&screen->base);

   screen->base.fence.emit = nv30_screen_fence_emit;
   screen->base.fence.update = nv30_screen_fence_update;

   ret = nouveau_screen_init(&screen->base, dev);
   if (ret)
      FAIL_SCREEN_INIT("nv30_screen_init failed: %d\n", ret);

   screen->base.vidmem_bindings |= PIPE_BIND_VERTEX_BUFFER;
   screen->base.sysmem_bindings |= PIPE_BIND_VERTEX_BUFFER;
   if (oclass == NV40_3D_CLASS) {
      screen->base.vidmem_bindings |= PIPE_BIND_INDEX_BUFFER;
      screen->base.sysmem_bindings |= PIPE_BIND_INDEX_BUFFER;
   }

   screen->fs_compiler_options = nv30_base_compiler_options;
   screen->fs_compiler_options.lower_fsat = false;
   if (oclass >= NV40_3D_CLASS)
      screen->fs_compiler_options.lower_fpow = false;

   fifo = screen->base.channel->data;
   push = screen->base.pushbuf;
   push->rsvd_kick = 16;

   ret = nouveau_object_new(screen->base.channel, 0x00000000, NV01_NULL_CLASS,
                            NULL, 0, &screen->null);
   if (ret)
      FAIL_SCREEN_INIT("error allocating null object: %d\n", ret);

   /* DMA_FENCE refuses to accept DMA objects with "adjust" filled in,
    * this means that the address pointed at by the DMA object must
    * be 4KiB aligned, which means this object needs to be the first
    * one allocated on the channel.
    */
   ret = nouveau_object_new(screen->base.channel, 0xbeef1e00,
                            NOUVEAU_NOTIFIER_CLASS, &(struct nv04_notify) {
                            .length = 32 }, sizeof(struct nv04_notify),
                            &screen->fence);
   if (ret)
      FAIL_SCREEN_INIT("error allocating fence notifier: %d\n", ret);

   /* DMA_NOTIFY object, we don't actually use this but M2MF fails without */
   ret = nouveau_object_new(screen->base.channel, 0xbeef0301,
                            NOUVEAU_NOTIFIER_CLASS, &(struct nv04_notify) {
                            .length = 32 }, sizeof(struct nv04_notify),
                            &screen->ntfy);
   if (ret)
      FAIL_SCREEN_INIT("error allocating sync notifier: %d\n", ret);

   /* DMA_QUERY, used to implement occlusion queries, we attempt to allocate
    * the remainder of the "notifier block" assigned by the kernel for
    * use as query objects
    */
   ret = nouveau_object_new(screen->base.channel, 0xbeef0351,
                            NOUVEAU_NOTIFIER_CLASS, &(struct nv04_notify) {
                            .length = 4096 - 128 }, sizeof(struct nv04_notify),
                            &screen->query);
   if (ret)
      FAIL_SCREEN_INIT("error allocating query notifier: %d\n", ret);

   ret = nouveau_heap_init(&screen->query_heap, 0, 4096 - 128);
   if (ret)
      FAIL_SCREEN_INIT("error creating query heap: %d\n", ret);

   list_inithead(&screen->queries);

   /* Vertex program resources (code/data), currently 6 of the constant
    * slots are reserved to implement user clipping planes
    */
   if (oclass < NV40_3D_CLASS) {
      nouveau_heap_init(&screen->vp_exec_heap, 0, 256);
      nouveau_heap_init(&screen->vp_data_heap, 6, 256 - 6);
   } else {
      nouveau_heap_init(&screen->vp_exec_heap, 0, 512);
      nouveau_heap_init(&screen->vp_data_heap, 6, 468 - 6);
   }

   ret = nouveau_bo_wrap(screen->base.device, fifo->notify, &screen->notify);
   if (ret == 0)
      ret = BO_MAP(&screen->base, screen->notify, 0, screen->base.client);
   if (ret)
      FAIL_SCREEN_INIT("error mapping notifier memory: %d\n", ret);

   ret = nouveau_object_new(screen->base.channel, 0xbeef3097, oclass,
                            NULL, 0, &screen->eng3d);
   if (ret)
      FAIL_SCREEN_INIT("error allocating 3d object: %d\n", ret);

   BEGIN_NV04(push, NV01_SUBC(3D, OBJECT), 1);
   PUSH_DATA (push, screen->eng3d->handle);
   BEGIN_NV04(push, NV30_3D(DMA_NOTIFY), 13);
   PUSH_DATA (push, screen->ntfy->handle);
   PUSH_DATA (push, fifo->vram);     /* TEXTURE0 */
   PUSH_DATA (push, fifo->gart);     /* TEXTURE1 */
   PUSH_DATA (push, fifo->vram);     /* COLOR1 */
   PUSH_DATA (push, screen->null->handle);  /* UNK190 */
   PUSH_DATA (push, fifo->vram);     /* COLOR0 */
   PUSH_DATA (push, fifo->vram);     /* ZETA */
   PUSH_DATA (push, fifo->vram);     /* VTXBUF0 */
   PUSH_DATA (push, fifo->gart);     /* VTXBUF1 */
   PUSH_DATA (push, screen->fence->handle);  /* FENCE */
   PUSH_DATA (push, screen->query->handle);  /* QUERY - intr 0x80 if nullobj */
   PUSH_DATA (push, screen->null->handle);  /* UNK1AC */
   PUSH_DATA (push, screen->null->handle);  /* UNK1B0 */
   if (screen->eng3d->oclass < NV40_3D_CLASS) {
      BEGIN_NV04(push, SUBC_3D(0x03b0), 1);
      PUSH_DATA (push, 0x00100000);
      BEGIN_NV04(push, SUBC_3D(0x1d80), 1);
      PUSH_DATA (push, 3);

      BEGIN_NV04(push, SUBC_3D(0x1e98), 1);
      PUSH_DATA (push, 0);
      BEGIN_NV04(push, SUBC_3D(0x17e0), 3);
      PUSH_DATA (push, fui(0.0));
      PUSH_DATA (push, fui(0.0));
      PUSH_DATA (push, fui(1.0));
      BEGIN_NV04(push, SUBC_3D(0x1f80), 16);
      for (i = 0; i < 16; i++)
         PUSH_DATA (push, (i == 8) ? 0x0000ffff : 0);

      BEGIN_NV04(push, NV30_3D(RC_ENABLE), 1);
      PUSH_DATA (push, 0);
   } else {
      BEGIN_NV04(push, NV40_3D(DMA_COLOR2), 2);
      PUSH_DATA (push, fifo->vram);
      PUSH_DATA (push, fifo->vram);  /* COLOR3 */

      BEGIN_NV04(push, SUBC_3D(0x1450), 1);
      PUSH_DATA (push, 0x00000004);

      BEGIN_NV04(push, SUBC_3D(0x1ea4), 3); /* ZCULL */
      PUSH_DATA (push, 0x00000010);
      PUSH_DATA (push, 0x01000100);
      PUSH_DATA (push, 0xff800006);

      /* vtxprog output routing */
      BEGIN_NV04(push, SUBC_3D(0x1fc4), 1);
      PUSH_DATA (push, 0x06144321);
      BEGIN_NV04(push, SUBC_3D(0x1fc8), 2);
      PUSH_DATA (push, 0xedcba987);
      PUSH_DATA (push, 0x0000006f);
      BEGIN_NV04(push, SUBC_3D(0x1fd0), 1);
      PUSH_DATA (push, 0x00171615);
      BEGIN_NV04(push, SUBC_3D(0x1fd4), 1);
      PUSH_DATA (push, 0x001b1a19);

      BEGIN_NV04(push, SUBC_3D(0x1ef8), 1);
      PUSH_DATA (push, 0x0020ffff);
      BEGIN_NV04(push, SUBC_3D(0x1d64), 1);
      PUSH_DATA (push, 0x01d300d4);

      BEGIN_NV04(push, NV40_3D(MIPMAP_ROUNDING), 1);
      PUSH_DATA (push, NV40_3D_MIPMAP_ROUNDING_MODE_DOWN);
   }

   ret = nouveau_object_new(screen->base.channel, 0xbeef3901, NV03_M2MF_CLASS,
                            NULL, 0, &screen->m2mf);
   if (ret)
      FAIL_SCREEN_INIT("error allocating m2mf object: %d\n", ret);

   BEGIN_NV04(push, NV01_SUBC(M2MF, OBJECT), 1);
   PUSH_DATA (push, screen->m2mf->handle);
   BEGIN_NV04(push, NV03_M2MF(DMA_NOTIFY), 1);
   PUSH_DATA (push, screen->ntfy->handle);

   ret = nouveau_object_new(screen->base.channel, 0xbeef6201,
                            NV10_SURFACE_2D_CLASS, NULL, 0, &screen->surf2d);
   if (ret)
      FAIL_SCREEN_INIT("error allocating surf2d object: %d\n", ret);

   BEGIN_NV04(push, NV01_SUBC(SF2D, OBJECT), 1);
   PUSH_DATA (push, screen->surf2d->handle);
   BEGIN_NV04(push, NV04_SF2D(DMA_NOTIFY), 1);
   PUSH_DATA (push, screen->ntfy->handle);

   if (dev->chipset < 0x40)
      oclass = NV30_SURFACE_SWZ_CLASS;
   else
      oclass = NV40_SURFACE_SWZ_CLASS;

   ret = nouveau_object_new(screen->base.channel, 0xbeef5201, oclass,
                            NULL, 0, &screen->swzsurf);
   if (ret)
      FAIL_SCREEN_INIT("error allocating swizzled surface object: %d\n", ret);

   BEGIN_NV04(push, NV01_SUBC(SSWZ, OBJECT), 1);
   PUSH_DATA (push, screen->swzsurf->handle);
   BEGIN_NV04(push, NV04_SSWZ(DMA_NOTIFY), 1);
   PUSH_DATA (push, screen->ntfy->handle);

   if (dev->chipset < 0x40)
      oclass = NV30_SIFM_CLASS;
   else
      oclass = NV40_SIFM_CLASS;

   ret = nouveau_object_new(screen->base.channel, 0xbeef7701, oclass,
                            NULL, 0, &screen->sifm);
   if (ret)
      FAIL_SCREEN_INIT("error allocating scaled image object: %d\n", ret);

   BEGIN_NV04(push, NV01_SUBC(SIFM, OBJECT), 1);
   PUSH_DATA (push, screen->sifm->handle);
   BEGIN_NV04(push, NV03_SIFM(DMA_NOTIFY), 1);
   PUSH_DATA (push, screen->ntfy->handle);
   BEGIN_NV04(push, NV05_SIFM(COLOR_CONVERSION), 1);
   PUSH_DATA (push, NV05_SIFM_COLOR_CONVERSION_TRUNCATE);
   PUSH_KICK (push);

   return &screen->base;
}
