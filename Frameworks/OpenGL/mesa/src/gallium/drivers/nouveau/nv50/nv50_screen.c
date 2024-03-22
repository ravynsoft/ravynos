/*
 * Copyright 2010 Christoph Bumiller
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
 */

#include <errno.h>
#include <xf86drm.h>
#include <nouveau_drm.h>
#include "util/format/u_format.h"
#include "util/format/u_format_s3tc.h"
#include "util/u_screen.h"
#include "pipe/p_screen.h"

#include "nv50_ir_driver.h"

#include "nv50/nv50_context.h"
#include "nv50/nv50_screen.h"

#include "nouveau_vp3_video.h"

#include "nv_object.xml.h"

/* affected by LOCAL_WARPS_LOG_ALLOC / LOCAL_WARPS_NO_CLAMP */
#define LOCAL_WARPS_ALLOC 32
/* affected by STACK_WARPS_LOG_ALLOC / STACK_WARPS_NO_CLAMP */
#define STACK_WARPS_ALLOC 32

#define THREADS_IN_WARP 32

static bool
nv50_screen_is_format_supported(struct pipe_screen *pscreen,
                                enum pipe_format format,
                                enum pipe_texture_target target,
                                unsigned sample_count,
                                unsigned storage_sample_count,
                                unsigned bindings)
{
   if (sample_count > 8)
      return false;
   if (!(0x117 & (1 << sample_count))) /* 0, 1, 2, 4 or 8 */
      return false;
   if (sample_count == 8 && util_format_get_blocksizebits(format) >= 128)
      return false;

   if (MAX2(1, sample_count) != MAX2(1, storage_sample_count))
      return false;

   /* Short-circuit the rest of the logic -- this is used by the gallium frontend
    * to determine valid MS levels in a no-attachments scenario.
    */
   if (format == PIPE_FORMAT_NONE && bindings & PIPE_BIND_RENDER_TARGET)
      return true;

   switch (format) {
   case PIPE_FORMAT_Z16_UNORM:
      if (nv50_screen(pscreen)->tesla->oclass < NVA0_3D_CLASS)
         return false;
      break;
   default:
      break;
   }

   if (bindings & PIPE_BIND_LINEAR)
      if (util_format_is_depth_or_stencil(format) ||
          (target != PIPE_TEXTURE_1D &&
           target != PIPE_TEXTURE_2D &&
           target != PIPE_TEXTURE_RECT) ||
          sample_count > 1)
         return false;

   /* shared is always supported */
   bindings &= ~(PIPE_BIND_LINEAR |
                 PIPE_BIND_SHARED);

   if (bindings & PIPE_BIND_INDEX_BUFFER) {
      if (format != PIPE_FORMAT_R8_UINT &&
          format != PIPE_FORMAT_R16_UINT &&
          format != PIPE_FORMAT_R32_UINT)
         return false;
      bindings &= ~PIPE_BIND_INDEX_BUFFER;
   }

   return (( nv50_format_table[format].usage |
            nv50_vertex_format[format].usage) & bindings) == bindings;
}

static int
nv50_screen_get_param(struct pipe_screen *pscreen, enum pipe_cap param)
{
   struct nouveau_screen *screen = nouveau_screen(pscreen);
   const uint16_t class_3d = screen->class_3d;
   struct nouveau_device *dev = screen->device;

   switch (param) {
   /* non-boolean caps */
   case PIPE_CAP_MAX_TEXTURE_2D_SIZE:
      return 8192;
   case PIPE_CAP_MAX_TEXTURE_3D_LEVELS:
      return 12;
   case PIPE_CAP_MAX_TEXTURE_CUBE_LEVELS:
      return 14;
   case PIPE_CAP_MAX_TEXTURE_ARRAY_LAYERS:
      return 512;
   case PIPE_CAP_MIN_TEXTURE_GATHER_OFFSET:
   case PIPE_CAP_MIN_TEXEL_OFFSET:
      return -8;
   case PIPE_CAP_MAX_TEXTURE_GATHER_OFFSET:
   case PIPE_CAP_MAX_TEXEL_OFFSET:
      return 7;
   case PIPE_CAP_MAX_TEXEL_BUFFER_ELEMENTS_UINT:
      return 128 * 1024 * 1024;
   case PIPE_CAP_GLSL_FEATURE_LEVEL:
      return 330;
   case PIPE_CAP_GLSL_FEATURE_LEVEL_COMPATIBILITY:
      return 330;
   case PIPE_CAP_ESSL_FEATURE_LEVEL:
      return class_3d >= NVA3_3D_CLASS ? 310 : 300;
   case PIPE_CAP_MAX_RENDER_TARGETS:
      return 8;
   case PIPE_CAP_MAX_DUAL_SOURCE_RENDER_TARGETS:
      return 1;
   case PIPE_CAP_MAX_COMBINED_SHADER_OUTPUT_RESOURCES:
      return NV50_MAX_GLOBALS - 1;
   case PIPE_CAP_VIEWPORT_SUBPIXEL_BITS:
   case PIPE_CAP_RASTERIZER_SUBPIXEL_BITS:
      return 8;
   case PIPE_CAP_MAX_STREAM_OUTPUT_BUFFERS:
      return 4;
   case PIPE_CAP_MAX_STREAM_OUTPUT_INTERLEAVED_COMPONENTS:
      return 64;
   case PIPE_CAP_MAX_STREAM_OUTPUT_SEPARATE_COMPONENTS:
      return 4;
   case PIPE_CAP_MAX_GEOMETRY_OUTPUT_VERTICES:
   case PIPE_CAP_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS:
      return 1024;
   case PIPE_CAP_MAX_VERTEX_STREAMS:
      return 1;
   case PIPE_CAP_MAX_GS_INVOCATIONS:
      return 0;
   case PIPE_CAP_MAX_SHADER_BUFFER_SIZE_UINT:
      return 1 << 27;
   case PIPE_CAP_MAX_VERTEX_ATTRIB_STRIDE:
      return 2048;
   case PIPE_CAP_MAX_VERTEX_ELEMENT_SRC_OFFSET:
      return 2047;
   case PIPE_CAP_CONSTANT_BUFFER_OFFSET_ALIGNMENT:
      return 256;
   case PIPE_CAP_TEXTURE_BUFFER_OFFSET_ALIGNMENT:
      return 16; /* 256 for binding as RT, but that's not possible in GL */
   case PIPE_CAP_SHADER_BUFFER_OFFSET_ALIGNMENT:
      return 256; /* the access limit is aligned to 256 */
   case PIPE_CAP_MIN_MAP_BUFFER_ALIGNMENT:
      return NOUVEAU_MIN_BUFFER_MAP_ALIGN;
   case PIPE_CAP_MAX_VIEWPORTS:
      return NV50_MAX_VIEWPORTS;
   case PIPE_CAP_TEXTURE_BORDER_COLOR_QUIRK:
      return PIPE_QUIRK_TEXTURE_BORDER_COLOR_SWIZZLE_NV50;
   case PIPE_CAP_ENDIANNESS:
      return PIPE_ENDIAN_LITTLE;
   case PIPE_CAP_MAX_TEXTURE_GATHER_COMPONENTS:
      return (class_3d >= NVA3_3D_CLASS) ? 4 : 0;
   case PIPE_CAP_MAX_WINDOW_RECTANGLES:
      return NV50_MAX_WINDOW_RECTANGLES;
   case PIPE_CAP_MAX_TEXTURE_UPLOAD_MEMORY_BUDGET:
      return 16 * 1024 * 1024;
   case PIPE_CAP_MAX_VARYINGS:
      return 15;
   case PIPE_CAP_MAX_VERTEX_BUFFERS:
      return 16;
   case PIPE_CAP_GL_BEGIN_END_BUFFER_SIZE:
      return 512 * 1024; /* TODO: Investigate tuning this */
   case PIPE_CAP_MAX_TEXTURE_MB:
      return 0; /* TODO: use 1/2 of VRAM for this? */

   case PIPE_CAP_SUPPORTED_PRIM_MODES_WITH_RESTART:
   case PIPE_CAP_SUPPORTED_PRIM_MODES:
      return BITFIELD_MASK(MESA_PRIM_COUNT);

   /* supported caps */
   case PIPE_CAP_TEXTURE_MIRROR_CLAMP:
   case PIPE_CAP_TEXTURE_MIRROR_CLAMP_TO_EDGE:
   case PIPE_CAP_TEXTURE_SWIZZLE:
   case PIPE_CAP_NPOT_TEXTURES:
   case PIPE_CAP_MIXED_FRAMEBUFFER_SIZES:
   case PIPE_CAP_MIXED_COLOR_DEPTH_BITS:
   case PIPE_CAP_ANISOTROPIC_FILTER:
   case PIPE_CAP_TEXTURE_BUFFER_OBJECTS:
   case PIPE_CAP_DEPTH_CLIP_DISABLE:
   case PIPE_CAP_FRAGMENT_SHADER_TEXTURE_LOD:
   case PIPE_CAP_FRAGMENT_SHADER_DERIVATIVES:
   case PIPE_CAP_FRAGMENT_COLOR_CLAMPED:
   case PIPE_CAP_VERTEX_COLOR_UNCLAMPED:
   case PIPE_CAP_VERTEX_COLOR_CLAMPED:
   case PIPE_CAP_QUERY_TIMESTAMP:
   case PIPE_CAP_QUERY_TIME_ELAPSED:
   case PIPE_CAP_OCCLUSION_QUERY:
   case PIPE_CAP_BLEND_EQUATION_SEPARATE:
   case PIPE_CAP_INDEP_BLEND_ENABLE:
   case PIPE_CAP_FS_COORD_ORIGIN_UPPER_LEFT:
   case PIPE_CAP_FS_COORD_PIXEL_CENTER_HALF_INTEGER:
   case PIPE_CAP_PRIMITIVE_RESTART:
   case PIPE_CAP_PRIMITIVE_RESTART_FIXED_INDEX:
   case PIPE_CAP_VS_INSTANCEID:
   case PIPE_CAP_VERTEX_ELEMENT_INSTANCE_DIVISOR:
   case PIPE_CAP_CONDITIONAL_RENDER:
   case PIPE_CAP_TEXTURE_BARRIER:
   case PIPE_CAP_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION:
   case PIPE_CAP_START_INSTANCE:
   case PIPE_CAP_USER_VERTEX_BUFFERS:
   case PIPE_CAP_TEXTURE_MULTISAMPLE:
   case PIPE_CAP_FS_FINE_DERIVATIVE:
   case PIPE_CAP_SAMPLER_VIEW_TARGET:
   case PIPE_CAP_CONDITIONAL_RENDER_INVERTED:
   case PIPE_CAP_CLIP_HALFZ:
   case PIPE_CAP_MEMOBJ:
   case PIPE_CAP_POLYGON_OFFSET_CLAMP:
   case PIPE_CAP_QUERY_PIPELINE_STATISTICS:
   case PIPE_CAP_TEXTURE_FLOAT_LINEAR:
   case PIPE_CAP_TEXTURE_HALF_FLOAT_LINEAR:
   case PIPE_CAP_DEPTH_BOUNDS_TEST:
   case PIPE_CAP_TEXTURE_QUERY_SAMPLES:
   case PIPE_CAP_COPY_BETWEEN_COMPRESSED_AND_PLAIN_FORMATS:
   case PIPE_CAP_FS_FACE_IS_INTEGER_SYSVAL:
   case PIPE_CAP_INVALIDATE_BUFFER:
   case PIPE_CAP_STRING_MARKER:
   case PIPE_CAP_CULL_DISTANCE:
   case PIPE_CAP_SHADER_ARRAY_COMPONENTS:
   case PIPE_CAP_LEGACY_MATH_RULES:
   case PIPE_CAP_TGSI_TEX_TXF_LZ:
   case PIPE_CAP_SHADER_CLOCK:
   case PIPE_CAP_CAN_BIND_CONST_BUFFER_AS_VERTEX:
   case PIPE_CAP_TGSI_DIV:
   case PIPE_CAP_CLEAR_SCISSORED:
   case PIPE_CAP_FRAMEBUFFER_NO_ATTACHMENT:
   case PIPE_CAP_COMPUTE:
   case PIPE_CAP_QUERY_MEMORY_INFO:
      return 1;

   case PIPE_CAP_ALPHA_TEST:
      /* nvc0 has fixed function alpha test support, but nv50 doesn't.  If we
       * don't have it, then the frontend will lower it for us.
       */
      return class_3d >= NVC0_3D_CLASS;

   case PIPE_CAP_TEXTURE_TRANSFER_MODES:
      return PIPE_TEXTURE_TRANSFER_BLIT;
   case PIPE_CAP_SEAMLESS_CUBE_MAP:
      return 1; /* class_3d >= NVA0_3D_CLASS; */
   /* supported on nva0+ */
   case PIPE_CAP_STREAM_OUTPUT_PAUSE_RESUME:
      return class_3d >= NVA0_3D_CLASS;
   /* supported on nva3+ */
   case PIPE_CAP_CUBE_MAP_ARRAY:
   case PIPE_CAP_INDEP_BLEND_FUNC:
   case PIPE_CAP_TEXTURE_QUERY_LOD:
   case PIPE_CAP_SAMPLE_SHADING:
   case PIPE_CAP_FORCE_PERSAMPLE_INTERP:
      return class_3d >= NVA3_3D_CLASS;

   case PIPE_CAP_PCI_GROUP:
   case PIPE_CAP_PCI_BUS:
   case PIPE_CAP_PCI_DEVICE:
   case PIPE_CAP_PCI_FUNCTION:
      return 0;

   case PIPE_CAP_MULTISAMPLE_Z_RESOLVE: /* potentially supported on some hw */
   case PIPE_CAP_INTEGER_MULTIPLY_32X16: /* could be done */
   case PIPE_CAP_MAP_UNSYNCHRONIZED_THREAD_SAFE: /* when we fix MT stuff */
   case PIPE_CAP_NIR_IMAGES_AS_DEREF:
   case PIPE_CAP_HARDWARE_GL_SELECT:
      return 0;

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
      return nouveau_screen(pscreen)->is_uma;

   default:
      return u_pipe_screen_get_param_defaults(pscreen, param);
   }
}

static int
nv50_screen_get_shader_param(struct pipe_screen *pscreen,
                             enum pipe_shader_type shader,
                             enum pipe_shader_cap param)
{
   switch (shader) {
   case PIPE_SHADER_VERTEX:
   case PIPE_SHADER_GEOMETRY:
   case PIPE_SHADER_FRAGMENT:
   case PIPE_SHADER_COMPUTE:
      break;
   default:
      return 0;
   }

   switch (param) {
   case PIPE_SHADER_CAP_MAX_INSTRUCTIONS:
   case PIPE_SHADER_CAP_MAX_ALU_INSTRUCTIONS:
   case PIPE_SHADER_CAP_MAX_TEX_INSTRUCTIONS:
   case PIPE_SHADER_CAP_MAX_TEX_INDIRECTIONS:
      return 16384;
   case PIPE_SHADER_CAP_MAX_CONTROL_FLOW_DEPTH:
      return 4;
   case PIPE_SHADER_CAP_MAX_INPUTS:
      if (shader == PIPE_SHADER_VERTEX)
         return 32;
      return 15;
   case PIPE_SHADER_CAP_MAX_OUTPUTS:
      return 16;
   case PIPE_SHADER_CAP_MAX_CONST_BUFFER0_SIZE:
      return 65536;
   case PIPE_SHADER_CAP_MAX_CONST_BUFFERS:
      return NV50_MAX_PIPE_CONSTBUFS;
   case PIPE_SHADER_CAP_INDIRECT_OUTPUT_ADDR:
      return shader != PIPE_SHADER_FRAGMENT;
   case PIPE_SHADER_CAP_INDIRECT_INPUT_ADDR:
   case PIPE_SHADER_CAP_INDIRECT_TEMP_ADDR:
   case PIPE_SHADER_CAP_INDIRECT_CONST_ADDR:
      return 1;
   case PIPE_SHADER_CAP_MAX_TEMPS:
      return nv50_screen(pscreen)->max_tls_space / ONE_TEMP_SIZE;
   case PIPE_SHADER_CAP_CONT_SUPPORTED:
      return 1;
   case PIPE_SHADER_CAP_TGSI_SQRT_SUPPORTED:
      return 1;
   case PIPE_SHADER_CAP_INT64_ATOMICS:
   case PIPE_SHADER_CAP_FP16:
   case PIPE_SHADER_CAP_FP16_DERIVATIVES:
   case PIPE_SHADER_CAP_FP16_CONST_BUFFERS:
   case PIPE_SHADER_CAP_INT16:
   case PIPE_SHADER_CAP_GLSL_16BIT_CONSTS:
   case PIPE_SHADER_CAP_SUBROUTINES:
      return 0; /* please inline, or provide function declarations */
   case PIPE_SHADER_CAP_INTEGERS:
      return 1;
   case PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS:
      /* The chip could handle more sampler views than samplers */
   case PIPE_SHADER_CAP_MAX_SAMPLER_VIEWS:
      return MIN2(16, PIPE_MAX_SAMPLERS);
   case PIPE_SHADER_CAP_MAX_SHADER_BUFFERS:
      return shader == PIPE_SHADER_COMPUTE ? NV50_MAX_GLOBALS - 1 : 0;
   case PIPE_SHADER_CAP_MAX_SHADER_IMAGES:
      return shader == PIPE_SHADER_COMPUTE ? NV50_MAX_GLOBALS - 1 : 0;
   case PIPE_SHADER_CAP_SUPPORTED_IRS:
      return 1 << PIPE_SHADER_IR_NIR;
   case PIPE_SHADER_CAP_TGSI_ANY_INOUT_DECL_RANGE:
   case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTERS:
   case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTER_BUFFERS:
      return 0;
   default:
      NOUVEAU_ERR("unknown PIPE_SHADER_CAP %d\n", param);
      return 0;
   }
}

static float
nv50_screen_get_paramf(struct pipe_screen *pscreen, enum pipe_capf param)
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
      return 10.0f;
   case PIPE_CAPF_MAX_POINT_SIZE:
   case PIPE_CAPF_MAX_POINT_SIZE_AA:
      return 64.0f;
   case PIPE_CAPF_MAX_TEXTURE_ANISOTROPY:
      return 16.0f;
   case PIPE_CAPF_MAX_TEXTURE_LOD_BIAS:
      return 15.0f;
   case PIPE_CAPF_MIN_CONSERVATIVE_RASTER_DILATE:
   case PIPE_CAPF_MAX_CONSERVATIVE_RASTER_DILATE:
   case PIPE_CAPF_CONSERVATIVE_RASTER_DILATE_GRANULARITY:
      return 0.0f;
   }

   NOUVEAU_ERR("unknown PIPE_CAPF %d\n", param);
   return 0.0f;
}

static int
nv50_screen_get_compute_param(struct pipe_screen *pscreen,
                              enum pipe_shader_ir ir_type,
                              enum pipe_compute_cap param, void *data)
{
   struct nv50_screen *screen = nv50_screen(pscreen);
   struct nouveau_device *dev = screen->base.device;

#define RET(x) do {                  \
   if (data)                         \
      memcpy(data, x, sizeof(x));    \
   return sizeof(x);                 \
} while (0)

   switch (param) {
   case PIPE_COMPUTE_CAP_GRID_DIMENSION:
      RET((uint64_t []) { 3 });
   case PIPE_COMPUTE_CAP_MAX_GRID_SIZE:
      RET(((uint64_t []) { 65535, 65535, 65535 }));
   case PIPE_COMPUTE_CAP_MAX_BLOCK_SIZE:
      RET(((uint64_t []) { 512, 512, 64 }));
   case PIPE_COMPUTE_CAP_MAX_THREADS_PER_BLOCK:
      RET((uint64_t []) { 512 });
   case PIPE_COMPUTE_CAP_MAX_GLOBAL_SIZE: /* g0-15[] */
      RET((uint64_t []) { nouveau_device_get_global_mem_size(dev) });
   case PIPE_COMPUTE_CAP_MAX_LOCAL_SIZE: /* s[] */
      RET((uint64_t []) { 16 << 10 });
   case PIPE_COMPUTE_CAP_MAX_PRIVATE_SIZE: /* l[] */
      RET((uint64_t []) { 16 << 10 });
   case PIPE_COMPUTE_CAP_MAX_INPUT_SIZE: /* c[], arbitrary limit */
      RET((uint64_t []) { 4096 });
   case PIPE_COMPUTE_CAP_SUBGROUP_SIZES:
      RET((uint32_t []) { 32 });
   case PIPE_COMPUTE_CAP_MAX_SUBGROUPS:
      RET((uint32_t []) { 0 });
   case PIPE_COMPUTE_CAP_MAX_MEM_ALLOC_SIZE:
      RET((uint64_t []) { nouveau_device_get_global_mem_size(dev) });
   case PIPE_COMPUTE_CAP_IMAGES_SUPPORTED:
      RET((uint32_t []) { 0 });
   case PIPE_COMPUTE_CAP_MAX_COMPUTE_UNITS:
      RET((uint32_t []) { screen->mp_count });
   case PIPE_COMPUTE_CAP_MAX_CLOCK_FREQUENCY:
      RET((uint32_t []) { 512 }); /* FIXME: arbitrary limit */
   case PIPE_COMPUTE_CAP_ADDRESS_BITS:
      RET((uint32_t []) { 32 });
   case PIPE_COMPUTE_CAP_MAX_VARIABLE_THREADS_PER_BLOCK:
      RET((uint64_t []) { 0 });
   default:
      return 0;
   }

#undef RET
}

static void
nv50_screen_destroy(struct pipe_screen *pscreen)
{
   struct nv50_screen *screen = nv50_screen(pscreen);

   if (!nouveau_drm_screen_unref(&screen->base))
      return;

   if (screen->blitter)
      nv50_blitter_destroy(screen);
   if (screen->pm.prog) {
      screen->pm.prog->code = NULL; /* hardcoded, don't FREE */
      nv50_program_destroy(NULL, screen->pm.prog);
      FREE(screen->pm.prog);
   }

   nouveau_bo_ref(NULL, &screen->code);
   nouveau_bo_ref(NULL, &screen->tls_bo);
   nouveau_bo_ref(NULL, &screen->stack_bo);
   nouveau_bo_ref(NULL, &screen->txc);
   nouveau_bo_ref(NULL, &screen->uniforms);
   nouveau_bo_ref(NULL, &screen->fence.bo);

   nouveau_heap_destroy(&screen->vp_code_heap);
   nouveau_heap_destroy(&screen->gp_code_heap);
   nouveau_heap_destroy(&screen->fp_code_heap);

   FREE(screen->tic.entries);

   nouveau_object_del(&screen->tesla);
   nouveau_object_del(&screen->eng2d);
   nouveau_object_del(&screen->m2mf);
   nouveau_object_del(&screen->compute);
   nouveau_object_del(&screen->sync);

   nouveau_screen_fini(&screen->base);
   simple_mtx_destroy(&screen->state_lock);

   FREE(screen);
}

static void
nv50_screen_fence_emit(struct pipe_context *pcontext, u32 *sequence,
                       struct nouveau_bo *wait)
{
   struct nv50_context *nv50 = nv50_context(pcontext);
   struct nv50_screen *screen = nv50->screen;
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   struct nouveau_pushbuf_refn ref = { wait, NOUVEAU_BO_GART | NOUVEAU_BO_RDWR };

   /* we need to do it after possible flush in MARK_RING */
   *sequence = ++screen->base.fence.sequence;

   assert(PUSH_AVAIL(push) + push->rsvd_kick >= 5);
   PUSH_DATA (push, NV50_FIFO_PKHDR(NV50_3D(QUERY_ADDRESS_HIGH), 4));
   PUSH_DATAh(push, screen->fence.bo->offset);
   PUSH_DATA (push, screen->fence.bo->offset);
   PUSH_DATA (push, *sequence);
   PUSH_DATA (push, NV50_3D_QUERY_GET_MODE_WRITE_UNK0 |
                    NV50_3D_QUERY_GET_UNK4 |
                    NV50_3D_QUERY_GET_UNIT_CROP |
                    NV50_3D_QUERY_GET_TYPE_QUERY |
                    NV50_3D_QUERY_GET_QUERY_SELECT_ZERO |
                    NV50_3D_QUERY_GET_SHORT);

   nouveau_pushbuf_refn(push, &ref, 1);
}

static u32
nv50_screen_fence_update(struct pipe_screen *pscreen)
{
   return nv50_screen(pscreen)->fence.map[0];
}

static void
nv50_screen_init_hwctx(struct nv50_screen *screen)
{
   struct nouveau_pushbuf *push = screen->base.pushbuf;
   struct nv04_fifo *fifo;
   unsigned i;

   fifo = (struct nv04_fifo *)screen->base.channel->data;

   BEGIN_NV04(push, SUBC_M2MF(NV01_SUBCHAN_OBJECT), 1);
   PUSH_DATA (push, screen->m2mf->handle);
   BEGIN_NV04(push, SUBC_M2MF(NV03_M2MF_DMA_NOTIFY), 3);
   PUSH_DATA (push, screen->sync->handle);
   PUSH_DATA (push, fifo->vram);
   PUSH_DATA (push, fifo->vram);

   BEGIN_NV04(push, SUBC_2D(NV01_SUBCHAN_OBJECT), 1);
   PUSH_DATA (push, screen->eng2d->handle);
   BEGIN_NV04(push, NV50_2D(DMA_NOTIFY), 4);
   PUSH_DATA (push, screen->sync->handle);
   PUSH_DATA (push, fifo->vram);
   PUSH_DATA (push, fifo->vram);
   PUSH_DATA (push, fifo->vram);
   BEGIN_NV04(push, NV50_2D(OPERATION), 1);
   PUSH_DATA (push, NV50_2D_OPERATION_SRCCOPY);
   BEGIN_NV04(push, NV50_2D(CLIP_ENABLE), 1);
   PUSH_DATA (push, 0);
   BEGIN_NV04(push, NV50_2D(COLOR_KEY_ENABLE), 1);
   PUSH_DATA (push, 0);
   BEGIN_NV04(push, NV50_2D(SET_PIXELS_FROM_MEMORY_SAFE_OVERLAP), 1);
   PUSH_DATA (push, 1);
   BEGIN_NV04(push, NV50_2D(COND_MODE), 1);
   PUSH_DATA (push, NV50_2D_COND_MODE_ALWAYS);

   BEGIN_NV04(push, SUBC_3D(NV01_SUBCHAN_OBJECT), 1);
   PUSH_DATA (push, screen->tesla->handle);

   BEGIN_NV04(push, NV50_3D(COND_MODE), 1);
   PUSH_DATA (push, NV50_3D_COND_MODE_ALWAYS);

   BEGIN_NV04(push, NV50_3D(DMA_NOTIFY), 1);
   PUSH_DATA (push, screen->sync->handle);
   BEGIN_NV04(push, NV50_3D(DMA_ZETA), 11);
   for (i = 0; i < 11; ++i)
      PUSH_DATA(push, fifo->vram);
   BEGIN_NV04(push, NV50_3D(DMA_COLOR(0)), NV50_3D_DMA_COLOR__LEN);
   for (i = 0; i < NV50_3D_DMA_COLOR__LEN; ++i)
      PUSH_DATA(push, fifo->vram);

   BEGIN_NV04(push, NV50_3D(REG_MODE), 1);
   PUSH_DATA (push, NV50_3D_REG_MODE_STRIPED);
   BEGIN_NV04(push, NV50_3D(UNK1400_LANES), 1);
   PUSH_DATA (push, 0xf);

   if (debug_get_bool_option("NOUVEAU_SHADER_WATCHDOG", true)) {
      BEGIN_NV04(push, NV50_3D(WATCHDOG_TIMER), 1);
      PUSH_DATA (push, 0x18);
   }

   BEGIN_NV04(push, NV50_3D(ZETA_COMP_ENABLE), 1);
   PUSH_DATA(push, screen->base.drm->version >= 0x01000101);

   BEGIN_NV04(push, NV50_3D(RT_COMP_ENABLE(0)), 8);
   for (i = 0; i < 8; ++i)
      PUSH_DATA(push, screen->base.drm->version >= 0x01000101);

   BEGIN_NV04(push, NV50_3D(RT_CONTROL), 1);
   PUSH_DATA (push, 1);

   BEGIN_NV04(push, NV50_3D(CSAA_ENABLE), 1);
   PUSH_DATA (push, 0);
   BEGIN_NV04(push, NV50_3D(MULTISAMPLE_ENABLE), 1);
   PUSH_DATA (push, 0);
   BEGIN_NV04(push, NV50_3D(MULTISAMPLE_MODE), 1);
   PUSH_DATA (push, NV50_3D_MULTISAMPLE_MODE_MS1);
   BEGIN_NV04(push, NV50_3D(MULTISAMPLE_CTRL), 1);
   PUSH_DATA (push, 0);
   BEGIN_NV04(push, NV50_3D(PRIM_RESTART_WITH_DRAW_ARRAYS), 1);
   PUSH_DATA (push, 1);
   BEGIN_NV04(push, NV50_3D(BLEND_SEPARATE_ALPHA), 1);
   PUSH_DATA (push, 1);

   if (screen->tesla->oclass >= NVA0_3D_CLASS) {
      BEGIN_NV04(push, SUBC_3D(NVA0_3D_TEX_MISC), 1);
      PUSH_DATA (push, 0);
   }

   BEGIN_NV04(push, NV50_3D(SCREEN_Y_CONTROL), 1);
   PUSH_DATA (push, 0);
   BEGIN_NV04(push, NV50_3D(WINDOW_OFFSET_X), 2);
   PUSH_DATA (push, 0);
   PUSH_DATA (push, 0);
   BEGIN_NV04(push, NV50_3D(ZCULL_REGION), 1);
   PUSH_DATA (push, 0x3f);

   BEGIN_NV04(push, NV50_3D(VP_ADDRESS_HIGH), 2);
   PUSH_DATAh(push, screen->code->offset + (NV50_SHADER_STAGE_VERTEX << NV50_CODE_BO_SIZE_LOG2));
   PUSH_DATA (push, screen->code->offset + (NV50_SHADER_STAGE_VERTEX << NV50_CODE_BO_SIZE_LOG2));

   BEGIN_NV04(push, NV50_3D(FP_ADDRESS_HIGH), 2);
   PUSH_DATAh(push, screen->code->offset + (NV50_SHADER_STAGE_FRAGMENT << NV50_CODE_BO_SIZE_LOG2));
   PUSH_DATA (push, screen->code->offset + (NV50_SHADER_STAGE_FRAGMENT << NV50_CODE_BO_SIZE_LOG2));

   BEGIN_NV04(push, NV50_3D(GP_ADDRESS_HIGH), 2);
   PUSH_DATAh(push, screen->code->offset + (NV50_SHADER_STAGE_GEOMETRY << NV50_CODE_BO_SIZE_LOG2));
   PUSH_DATA (push, screen->code->offset + (NV50_SHADER_STAGE_GEOMETRY << NV50_CODE_BO_SIZE_LOG2));

   BEGIN_NV04(push, NV50_3D(LOCAL_ADDRESS_HIGH), 3);
   PUSH_DATAh(push, screen->tls_bo->offset);
   PUSH_DATA (push, screen->tls_bo->offset);
   PUSH_DATA (push, util_logbase2(screen->cur_tls_space / 8));

   BEGIN_NV04(push, NV50_3D(STACK_ADDRESS_HIGH), 3);
   PUSH_DATAh(push, screen->stack_bo->offset);
   PUSH_DATA (push, screen->stack_bo->offset);
   PUSH_DATA (push, 4);

   BEGIN_NV04(push, NV50_3D(CB_DEF_ADDRESS_HIGH), 3);
   PUSH_DATAh(push, screen->uniforms->offset + (0 << 16));
   PUSH_DATA (push, screen->uniforms->offset + (0 << 16));
   PUSH_DATA (push, (NV50_CB_PVP << 16) | 0x0000);

   BEGIN_NV04(push, NV50_3D(CB_DEF_ADDRESS_HIGH), 3);
   PUSH_DATAh(push, screen->uniforms->offset + (1 << 16));
   PUSH_DATA (push, screen->uniforms->offset + (1 << 16));
   PUSH_DATA (push, (NV50_CB_PGP << 16) | 0x0000);

   BEGIN_NV04(push, NV50_3D(CB_DEF_ADDRESS_HIGH), 3);
   PUSH_DATAh(push, screen->uniforms->offset + (2 << 16));
   PUSH_DATA (push, screen->uniforms->offset + (2 << 16));
   PUSH_DATA (push, (NV50_CB_PFP << 16) | 0x0000);

   BEGIN_NV04(push, NV50_3D(CB_DEF_ADDRESS_HIGH), 3);
   PUSH_DATAh(push, screen->uniforms->offset + (4 << 16));
   PUSH_DATA (push, screen->uniforms->offset + (4 << 16));
   PUSH_DATA (push, (NV50_CB_AUX << 16) | (NV50_CB_AUX_SIZE & 0xffff));

   BEGIN_NI04(push, NV50_3D(SET_PROGRAM_CB), 3);
   PUSH_DATA (push, (NV50_CB_AUX << 12) | 0xf01);
   PUSH_DATA (push, (NV50_CB_AUX << 12) | 0xf21);
   PUSH_DATA (push, (NV50_CB_AUX << 12) | 0xf31);

   /* return { 0.0, 0.0, 0.0, 0.0 } on out-of-bounds vtxbuf access */
   BEGIN_NV04(push, NV50_3D(CB_ADDR), 1);
   PUSH_DATA (push, (NV50_CB_AUX_RUNOUT_OFFSET << (8 - 2)) | NV50_CB_AUX);
   BEGIN_NI04(push, NV50_3D(CB_DATA(0)), 4);
   PUSH_DATAf(push, 0.0f);
   PUSH_DATAf(push, 0.0f);
   PUSH_DATAf(push, 0.0f);
   PUSH_DATAf(push, 0.0f);
   BEGIN_NV04(push, NV50_3D(VERTEX_RUNOUT_ADDRESS_HIGH), 2);
   PUSH_DATAh(push, screen->uniforms->offset + (4 << 16) + NV50_CB_AUX_RUNOUT_OFFSET);
   PUSH_DATA (push, screen->uniforms->offset + (4 << 16) + NV50_CB_AUX_RUNOUT_OFFSET);

   /* set the membar offset */
   BEGIN_NV04(push, NV50_3D(CB_ADDR), 1);
   PUSH_DATA (push, (NV50_CB_AUX_MEMBAR_OFFSET << (8 - 2)) | NV50_CB_AUX);
   BEGIN_NI04(push, NV50_3D(CB_DATA(0)), 1);
   PUSH_DATA (push, screen->uniforms->offset + (4 << 16) + NV50_CB_AUX_MEMBAR_OFFSET);

   nv50_upload_ms_info(push);

   /* max TIC (bits 4:8) & TSC bindings, per program type */
   for (i = 0; i < NV50_MAX_3D_SHADER_STAGES; ++i) {
      BEGIN_NV04(push, NV50_3D(TEX_LIMITS(i)), 1);
      PUSH_DATA (push, 0x54);
   }

   BEGIN_NV04(push, NV50_3D(TIC_ADDRESS_HIGH), 3);
   PUSH_DATAh(push, screen->txc->offset);
   PUSH_DATA (push, screen->txc->offset);
   PUSH_DATA (push, NV50_TIC_MAX_ENTRIES - 1);

   BEGIN_NV04(push, NV50_3D(TSC_ADDRESS_HIGH), 3);
   PUSH_DATAh(push, screen->txc->offset + 65536);
   PUSH_DATA (push, screen->txc->offset + 65536);
   PUSH_DATA (push, NV50_TSC_MAX_ENTRIES - 1);

   BEGIN_NV04(push, NV50_3D(LINKED_TSC), 1);
   PUSH_DATA (push, 0);

   BEGIN_NV04(push, NV50_3D(CLIP_RECTS_EN), 1);
   PUSH_DATA (push, 0);
   BEGIN_NV04(push, NV50_3D(CLIP_RECTS_MODE), 1);
   PUSH_DATA (push, NV50_3D_CLIP_RECTS_MODE_INSIDE_ANY);
   BEGIN_NV04(push, NV50_3D(CLIP_RECT_HORIZ(0)), 8 * 2);
   for (i = 0; i < 8 * 2; ++i)
      PUSH_DATA(push, 0);
   BEGIN_NV04(push, NV50_3D(CLIPID_ENABLE), 1);
   PUSH_DATA (push, 0);

   BEGIN_NV04(push, NV50_3D(VIEWPORT_TRANSFORM_EN), 1);
   PUSH_DATA (push, 1);
   for (i = 0; i < NV50_MAX_VIEWPORTS; i++) {
      BEGIN_NV04(push, NV50_3D(DEPTH_RANGE_NEAR(i)), 2);
      PUSH_DATAf(push, 0.0f);
      PUSH_DATAf(push, 1.0f);
      BEGIN_NV04(push, NV50_3D(VIEWPORT_HORIZ(i)), 2);
      PUSH_DATA (push, 8192 << 16);
      PUSH_DATA (push, 8192 << 16);
   }

   BEGIN_NV04(push, NV50_3D(VIEW_VOLUME_CLIP_CTRL), 1);
#ifdef NV50_SCISSORS_CLIPPING
   PUSH_DATA (push, 0x0000);
#else
   PUSH_DATA (push, 0x1080);
#endif

   BEGIN_NV04(push, NV50_3D(CLEAR_FLAGS), 1);
   PUSH_DATA (push, NV50_3D_CLEAR_FLAGS_CLEAR_RECT_VIEWPORT);

   /* We use scissors instead of exact view volume clipping,
    * so they're always enabled.
    */
   for (i = 0; i < NV50_MAX_VIEWPORTS; i++) {
      BEGIN_NV04(push, NV50_3D(SCISSOR_ENABLE(i)), 3);
      PUSH_DATA (push, 1);
      PUSH_DATA (push, 8192 << 16);
      PUSH_DATA (push, 8192 << 16);
   }

   BEGIN_NV04(push, NV50_3D(RASTERIZE_ENABLE), 1);
   PUSH_DATA (push, 1);
   BEGIN_NV04(push, NV50_3D(POINT_RASTER_RULES), 1);
   PUSH_DATA (push, NV50_3D_POINT_RASTER_RULES_OGL);
   BEGIN_NV04(push, NV50_3D(FRAG_COLOR_CLAMP_EN), 1);
   PUSH_DATA (push, 0x11111111);
   BEGIN_NV04(push, NV50_3D(EDGEFLAG), 1);
   PUSH_DATA (push, 1);

   BEGIN_NV04(push, NV50_3D(VB_ELEMENT_BASE), 1);
   PUSH_DATA (push, 0);
   if (screen->base.class_3d >= NV84_3D_CLASS) {
      BEGIN_NV04(push, NV84_3D(VERTEX_ID_BASE), 1);
      PUSH_DATA (push, 0);
   }

   BEGIN_NV04(push, NV50_3D(UNK0FDC), 1);
   PUSH_DATA (push, 1);
   BEGIN_NV04(push, NV50_3D(UNK19C0), 1);
   PUSH_DATA (push, 1);
}

static int nv50_tls_alloc(struct nv50_screen *screen, unsigned tls_space,
      uint64_t *tls_size)
{
   struct nouveau_device *dev = screen->base.device;
   int ret;

   assert(tls_space % ONE_TEMP_SIZE == 0);
   screen->cur_tls_space = util_next_power_of_two(tls_space / ONE_TEMP_SIZE) *
         ONE_TEMP_SIZE;
   if (nouveau_mesa_debug)
      debug_printf("allocating space for %u temps\n",
            util_next_power_of_two(tls_space / ONE_TEMP_SIZE));
   *tls_size = screen->cur_tls_space * util_next_power_of_two(screen->TPs) *
         screen->MPsInTP * LOCAL_WARPS_ALLOC * THREADS_IN_WARP;

   ret = nouveau_bo_new(dev, NOUVEAU_BO_VRAM, 1 << 16,
                        *tls_size, NULL, &screen->tls_bo);
   if (ret) {
      NOUVEAU_ERR("Failed to allocate local bo: %d\n", ret);
      return ret;
   }

   return 0;
}

int nv50_tls_realloc(struct nv50_screen *screen, unsigned tls_space)
{
   struct nouveau_pushbuf *push = screen->base.pushbuf;
   int ret;
   uint64_t tls_size;

   if (tls_space < screen->cur_tls_space)
      return 0;
   if (tls_space > screen->max_tls_space) {
      /* fixable by limiting number of warps (LOCAL_WARPS_LOG_ALLOC /
       * LOCAL_WARPS_NO_CLAMP) */
      NOUVEAU_ERR("Unsupported number of temporaries (%u > %u). Fixable if someone cares.\n",
            (unsigned)(tls_space / ONE_TEMP_SIZE),
            (unsigned)(screen->max_tls_space / ONE_TEMP_SIZE));
      return -ENOMEM;
   }

   nouveau_bo_ref(NULL, &screen->tls_bo);
   ret = nv50_tls_alloc(screen, tls_space, &tls_size);
   if (ret)
      return ret;

   BEGIN_NV04(push, NV50_3D(LOCAL_ADDRESS_HIGH), 3);
   PUSH_DATAh(push, screen->tls_bo->offset);
   PUSH_DATA (push, screen->tls_bo->offset);
   PUSH_DATA (push, util_logbase2(screen->cur_tls_space / 8));

   return 1;
}

static const void *
nv50_screen_get_compiler_options(struct pipe_screen *pscreen,
                                 enum pipe_shader_ir ir,
                                 enum pipe_shader_type shader)
{
   if (ir == PIPE_SHADER_IR_NIR)
      return nv50_ir_nir_shader_compiler_options(NVISA_G80_CHIPSET, shader);
   return NULL;
}

struct nouveau_screen *
nv50_screen_create(struct nouveau_device *dev)
{
   struct nv50_screen *screen;
   struct pipe_screen *pscreen;
   struct nouveau_object *chan;
   uint64_t value;
   uint32_t tesla_class;
   unsigned stack_size;
   int ret;

   screen = CALLOC_STRUCT(nv50_screen);
   if (!screen)
      return NULL;
   pscreen = &screen->base.base;
   pscreen->destroy = nv50_screen_destroy;

   simple_mtx_init(&screen->state_lock, mtx_plain);
   ret = nouveau_screen_init(&screen->base, dev);
   if (ret) {
      NOUVEAU_ERR("nouveau_screen_init failed: %d\n", ret);
      goto fail;
   }

   /* TODO: Prevent FIFO prefetch before transfer of index buffers and
    *  admit them to VRAM.
    */
   screen->base.vidmem_bindings |= PIPE_BIND_CONSTANT_BUFFER |
      PIPE_BIND_VERTEX_BUFFER;
   screen->base.sysmem_bindings |=
      PIPE_BIND_VERTEX_BUFFER | PIPE_BIND_INDEX_BUFFER;

   screen->base.pushbuf->rsvd_kick = 5;

   chan = screen->base.channel;

   pscreen->context_create = nv50_create;
   pscreen->is_format_supported = nv50_screen_is_format_supported;
   pscreen->get_param = nv50_screen_get_param;
   pscreen->get_shader_param = nv50_screen_get_shader_param;
   pscreen->get_paramf = nv50_screen_get_paramf;
   pscreen->get_compute_param = nv50_screen_get_compute_param;
   pscreen->get_driver_query_info = nv50_screen_get_driver_query_info;
   pscreen->get_driver_query_group_info = nv50_screen_get_driver_query_group_info;

   /* nir stuff */
   pscreen->get_compiler_options = nv50_screen_get_compiler_options;

   nv50_screen_init_resource_functions(pscreen);

   if (screen->base.device->chipset < 0x84 ||
       debug_get_bool_option("NOUVEAU_PMPEG", false)) {
      /* PMPEG */
      nouveau_screen_init_vdec(&screen->base);
   } else if (screen->base.device->chipset < 0x98 ||
              screen->base.device->chipset == 0xa0) {
      /* VP2 */
      screen->base.base.get_video_param = nv84_screen_get_video_param;
      screen->base.base.is_video_format_supported = nv84_screen_video_supported;
   } else {
      /* VP3/4 */
      screen->base.base.get_video_param = nouveau_vp3_screen_get_video_param;
      screen->base.base.is_video_format_supported = nouveau_vp3_screen_video_supported;
   }

   ret = nouveau_bo_new(dev, NOUVEAU_BO_GART | NOUVEAU_BO_MAP, 0, 4096,
                        NULL, &screen->fence.bo);
   if (ret) {
      NOUVEAU_ERR("Failed to allocate fence bo: %d\n", ret);
      goto fail;
   }

   BO_MAP(&screen->base, screen->fence.bo, 0, NULL);
   screen->fence.map = screen->fence.bo->map;
   screen->base.fence.emit = nv50_screen_fence_emit;
   screen->base.fence.update = nv50_screen_fence_update;

   ret = nouveau_object_new(chan, 0xbeef0301, NOUVEAU_NOTIFIER_CLASS,
                            &(struct nv04_notify){ .length = 32 },
                            sizeof(struct nv04_notify), &screen->sync);
   if (ret) {
      NOUVEAU_ERR("Failed to allocate notifier: %d\n", ret);
      goto fail;
   }

   ret = nouveau_object_new(chan, 0xbeef5039, NV50_M2MF_CLASS,
                            NULL, 0, &screen->m2mf);
   if (ret) {
      NOUVEAU_ERR("Failed to allocate PGRAPH context for M2MF: %d\n", ret);
      goto fail;
   }

   ret = nouveau_object_new(chan, 0xbeef502d, NV50_2D_CLASS,
                            NULL, 0, &screen->eng2d);
   if (ret) {
      NOUVEAU_ERR("Failed to allocate PGRAPH context for 2D: %d\n", ret);
      goto fail;
   }

   switch (dev->chipset & 0xf0) {
   case 0x50:
      tesla_class = NV50_3D_CLASS;
      break;
   case 0x80:
   case 0x90:
      tesla_class = NV84_3D_CLASS;
      break;
   case 0xa0:
      switch (dev->chipset) {
      case 0xa0:
      case 0xaa:
      case 0xac:
         tesla_class = NVA0_3D_CLASS;
         break;
      case 0xaf:
         tesla_class = NVAF_3D_CLASS;
         break;
      default:
         tesla_class = NVA3_3D_CLASS;
         break;
      }
      break;
   default:
      NOUVEAU_ERR("Not a known NV50 chipset: NV%02x\n", dev->chipset);
      goto fail;
   }
   screen->base.class_3d = tesla_class;

   ret = nouveau_object_new(chan, 0xbeef5097, tesla_class,
                            NULL, 0, &screen->tesla);
   if (ret) {
      NOUVEAU_ERR("Failed to allocate PGRAPH context for 3D: %d\n", ret);
      goto fail;
   }

   /* This over-allocates by a page. The GP, which would execute at the end of
    * the last page, would trigger faults. The going theory is that it
    * prefetches up to a certain amount.
    */
   ret = nouveau_bo_new(dev, NOUVEAU_BO_VRAM, 1 << 16,
                        (3 << NV50_CODE_BO_SIZE_LOG2) + 0x1000,
                        NULL, &screen->code);
   if (ret) {
      NOUVEAU_ERR("Failed to allocate code bo: %d\n", ret);
      goto fail;
   }

   nouveau_heap_init(&screen->vp_code_heap, 0, 1 << NV50_CODE_BO_SIZE_LOG2);
   nouveau_heap_init(&screen->gp_code_heap, 0, 1 << NV50_CODE_BO_SIZE_LOG2);
   nouveau_heap_init(&screen->fp_code_heap, 0, 1 << NV50_CODE_BO_SIZE_LOG2);

   nouveau_getparam(dev, NOUVEAU_GETPARAM_GRAPH_UNITS, &value);

   screen->TPs = util_bitcount(value & 0xffff);
   screen->MPsInTP = util_bitcount(value & 0x0f000000);

   screen->mp_count = screen->TPs * screen->MPsInTP;

   stack_size = util_next_power_of_two(screen->TPs) * screen->MPsInTP *
         STACK_WARPS_ALLOC * 64 * 8;

   ret = nouveau_bo_new(dev, NOUVEAU_BO_VRAM, 1 << 16, stack_size, NULL,
                        &screen->stack_bo);
   if (ret) {
      NOUVEAU_ERR("Failed to allocate stack bo: %d\n", ret);
      goto fail;
   }

   uint64_t size_of_one_temp = util_next_power_of_two(screen->TPs) *
         screen->MPsInTP * LOCAL_WARPS_ALLOC *  THREADS_IN_WARP *
         ONE_TEMP_SIZE;
   screen->max_tls_space = dev->vram_size / size_of_one_temp * ONE_TEMP_SIZE;
   screen->max_tls_space /= 2; /* half of vram */

   /* hw can address max 64 KiB */
   screen->max_tls_space = MIN2(screen->max_tls_space, 64 << 10);

   uint64_t tls_size;
   unsigned tls_space = 4/*temps*/ * ONE_TEMP_SIZE;
   ret = nv50_tls_alloc(screen, tls_space, &tls_size);
   if (ret)
      goto fail;

   if (nouveau_mesa_debug)
      debug_printf("TPs = %u, MPsInTP = %u, VRAM = %"PRIu64" MiB, tls_size = %"PRIu64" KiB\n",
            screen->TPs, screen->MPsInTP, dev->vram_size >> 20, tls_size >> 10);

   ret = nouveau_bo_new(dev, NOUVEAU_BO_VRAM, 1 << 16, 5 << 16, NULL,
                        &screen->uniforms);
   if (ret) {
      NOUVEAU_ERR("Failed to allocate uniforms bo: %d\n", ret);
      goto fail;
   }

   ret = nouveau_bo_new(dev, NOUVEAU_BO_VRAM, 1 << 16, 3 << 16, NULL,
                        &screen->txc);
   if (ret) {
      NOUVEAU_ERR("Failed to allocate TIC/TSC bo: %d\n", ret);
      goto fail;
   }

   screen->tic.entries = CALLOC(4096, sizeof(void *));
   screen->tsc.entries = screen->tic.entries + 2048;

   if (!nv50_blitter_create(screen))
      goto fail;

   nv50_screen_init_hwctx(screen);

   ret = nv50_screen_compute_setup(screen, screen->base.pushbuf);
   if (ret) {
      NOUVEAU_ERR("Failed to init compute context: %d\n", ret);
      goto fail;
   }

   // submit all initial state
   PUSH_KICK(screen->base.pushbuf);

   return &screen->base;

fail:
   screen->base.base.context_create = NULL;
   return &screen->base;
}

int
nv50_screen_tic_alloc(struct nv50_screen *screen, void *entry)
{
   int i = screen->tic.next;

   while (screen->tic.lock[i / 32] & (1 << (i % 32)))
      i = (i + 1) & (NV50_TIC_MAX_ENTRIES - 1);

   screen->tic.next = (i + 1) & (NV50_TIC_MAX_ENTRIES - 1);

   if (screen->tic.entries[i])
      nv50_tic_entry(screen->tic.entries[i])->id = -1;

   screen->tic.entries[i] = entry;
   return i;
}

int
nv50_screen_tsc_alloc(struct nv50_screen *screen, void *entry)
{
   int i = screen->tsc.next;

   while (screen->tsc.lock[i / 32] & (1 << (i % 32)))
      i = (i + 1) & (NV50_TSC_MAX_ENTRIES - 1);

   screen->tsc.next = (i + 1) & (NV50_TSC_MAX_ENTRIES - 1);

   if (screen->tsc.entries[i])
      nv50_tsc_entry(screen->tsc.entries[i])->id = -1;

   screen->tsc.entries[i] = entry;
   return i;
}
