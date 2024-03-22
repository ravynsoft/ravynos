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

#ifndef FREEDRENO_UTIL_H_
#define FREEDRENO_UTIL_H_

#include "common/freedreno_common.h"

#include "drm/freedreno_drmif.h"
#include "drm/freedreno_ringbuffer.h"

#include "util/format/u_formats.h"
#include "pipe/p_state.h"
#include "util/compiler.h"
#include "util/half_float.h"
#include "util/log.h"
#ifndef __cplusplus  // TODO fix cpu_trace.h to be c++ friendly
#include "util/perf/cpu_trace.h"
#endif
#include "util/u_debug.h"
#include "util/u_dynarray.h"
#include "util/u_math.h"
#include "util/u_pack_color.h"

#include "adreno_common.xml.h"
#include "adreno_pm4.xml.h"
#include "disasm.h"

#ifdef __cplusplus
extern "C" {
#endif

enum adreno_rb_depth_format fd_pipe2depth(enum pipe_format format);
enum pc_di_index_size fd_pipe2index(enum pipe_format format);
enum pipe_format fd_gmem_restore_format(enum pipe_format format);
enum adreno_rb_blend_factor fd_blend_factor(unsigned factor);
enum adreno_pa_su_sc_draw fd_polygon_mode(unsigned mode);
enum adreno_stencil_op fd_stencil_op(unsigned op);

#define A3XX_MAX_MIP_LEVELS 14

#define A2XX_MAX_RENDER_TARGETS 1
#define A3XX_MAX_RENDER_TARGETS 4
#define A4XX_MAX_RENDER_TARGETS 8
#define A5XX_MAX_RENDER_TARGETS 8
#define A6XX_MAX_RENDER_TARGETS 8

#define MAX_RENDER_TARGETS A6XX_MAX_RENDER_TARGETS

/* clang-format off */
enum fd_debug_flag {
   FD_DBG_MSGS         = BITFIELD_BIT(0),
   FD_DBG_DISASM       = BITFIELD_BIT(1),
   FD_DBG_DCLEAR       = BITFIELD_BIT(2),
   FD_DBG_DDRAW        = BITFIELD_BIT(3),
   FD_DBG_NOSCIS       = BITFIELD_BIT(4),
   FD_DBG_DIRECT       = BITFIELD_BIT(5),
   FD_DBG_GMEM         = BITFIELD_BIT(6),
   FD_DBG_PERF         = BITFIELD_BIT(7),
   FD_DBG_NOBIN        = BITFIELD_BIT(8),
   FD_DBG_SYSMEM       = BITFIELD_BIT(9),
   FD_DBG_SERIALC      = BITFIELD_BIT(10),
   FD_DBG_SHADERDB     = BITFIELD_BIT(11),
   FD_DBG_FLUSH        = BITFIELD_BIT(12),
   FD_DBG_DEQP         = BITFIELD_BIT(13),
   FD_DBG_INORDER      = BITFIELD_BIT(14),
   FD_DBG_BSTAT        = BITFIELD_BIT(15),
   FD_DBG_NOGROW       = BITFIELD_BIT(16),
   FD_DBG_LRZ          = BITFIELD_BIT(17),
   FD_DBG_NOINDR       = BITFIELD_BIT(18),
   FD_DBG_NOBLIT       = BITFIELD_BIT(19),
   FD_DBG_HIPRIO       = BITFIELD_BIT(20),
   FD_DBG_TTILE        = BITFIELD_BIT(21),
   FD_DBG_PERFC        = BITFIELD_BIT(22),
   FD_DBG_NOUBWC       = BITFIELD_BIT(23),
   FD_DBG_NOLRZ        = BITFIELD_BIT(24),
   FD_DBG_NOTILE       = BITFIELD_BIT(25),
   FD_DBG_LAYOUT       = BITFIELD_BIT(26),
   FD_DBG_NOFP16       = BITFIELD_BIT(27),
   FD_DBG_NOHW         = BITFIELD_BIT(28),
   FD_DBG_NOSBIN       = BITFIELD_BIT(29),
};
/* clang-format on */

extern int fd_mesa_debug;
extern bool fd_binning_enabled;

#define FD_DBG(category) unlikely(fd_mesa_debug &FD_DBG_##category)

#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>

#define DBG(fmt, ...)                                                          \
   do {                                                                        \
      if (FD_DBG(MSGS))                                                        \
         mesa_logi("%5d: %s:%d: " fmt, ((pid_t)syscall(SYS_gettid)),           \
                                        __func__, __LINE__,                    \
                                        ##__VA_ARGS__);                        \
   } while (0)

#define perf_debug_message(debug, type, ...)                                   \
   do {                                                                        \
      if (FD_DBG(PERF))                                                        \
         mesa_logw(__VA_ARGS__);                                               \
      struct util_debug_callback *__d = (debug);                               \
      if (__d)                                                                 \
         util_debug_message(__d, type, __VA_ARGS__);                           \
   } while (0)

#define perf_debug_ctx(ctx, ...)                                               \
   do {                                                                        \
      struct fd_context *__c = (ctx);                                          \
      perf_debug_message(__c ? &__c->debug : NULL, PERF_INFO, __VA_ARGS__);    \
   } while (0)

#define perf_debug(...) perf_debug_ctx(NULL, __VA_ARGS__)

#define perf_time_ctx(ctx, limit_ns, fmt, ...)                                 \
   for (struct __perf_time_state __s =                                         \
           {                                                                   \
              .t = -__perf_get_time(ctx),                                      \
           };                                                                  \
        !__s.done; ({                                                          \
           __s.t += __perf_get_time(ctx);                                      \
           __s.done = true;                                                    \
           if (__s.t > (limit_ns)) {                                           \
              perf_debug_ctx(ctx, fmt " (%.03f ms)", ##__VA_ARGS__,            \
                             (double)__s.t / 1000000.0);                       \
           }                                                                   \
        }))

#define perf_time(limit_ns, fmt, ...)                                          \
   perf_time_ctx(NULL, limit_ns, fmt, ##__VA_ARGS__)

struct __perf_time_state {
   int64_t t;
   bool done;
};

/* static inline would be nice here, except 'struct fd_context' is not
 * defined yet:
 */
#define __perf_get_time(ctx)                                                   \
   ((FD_DBG(PERF) || ({                                                        \
        struct fd_context *__c = (ctx);                                        \
        unlikely(__c && __c->debug.debug_message);                             \
     }))                                                                       \
       ? os_time_get_nano()                                                    \
       : 0)

#define DEFINE_CAST(parent, child)                                             \
   static inline struct child *child(struct parent *x)                         \
   {                                                                           \
      return (struct child *)x;                                                \
   }

struct fd_context;

/**
 * A psuedo-variable for defining where various parts of the fd_context
 * can be safely accessed.
 *
 * With threaded_context, certain pctx funcs are called from gallium
 * front-end/state-tracker (eg. CSO creation), while others are called
 * from the driver thread.  Things called from driver thread can safely
 * access anything in the ctx, while things called from the fe/st thread
 * must limit themselves to "safe" things (ie. ctx->screen is safe as it
 * is immutable, but the blitter_context is not).
 */
extern lock_cap_t fd_context_access_cap;

/**
 * Make the annotation a bit less verbose.. mark fields which should only
 * be accessed by driver-thread with 'dt'
 */
#define dt guarded_by(fd_context_access_cap)

/**
 * Annotation for entry-point functions only called in driver thread.
 *
 * For static functions, apply the annotation to the function declaration.
 * Otherwise apply to the function prototype.
 */
#define in_dt assert_cap(fd_context_access_cap)

/**
 * Annotation for internal functions which are only called from entry-
 * point functions (with 'in_dt' annotation) or other internal functions
 * with the 'assert_dt' annotation.
 *
 * For static functions, apply the annotation to the function declaration.
 * Otherwise apply to the function prototype.
 */
#define assert_dt requires_cap(fd_context_access_cap)

/**
 * Special helpers for context access outside of driver thread.  For ex,
 * pctx->get_query_result() is not called on driver thread, but the
 * query is guaranteed to be flushed, or the driver thread queue is
 * guaranteed to be flushed.
 *
 * Use with caution!
 */
static inline void
fd_context_access_begin(struct fd_context *ctx)
   acquire_cap(fd_context_access_cap)
{
}

static inline void
fd_context_access_end(struct fd_context *ctx) release_cap(fd_context_access_cap)
{
}

#define CP_REG(reg) ((0x4 << 16) | ((unsigned int)((reg) - (0x2000))))

static inline uint32_t
DRAW(enum pc_di_primtype prim_type, enum pc_di_src_sel source_select,
     enum pc_di_index_size index_size, enum pc_di_vis_cull_mode vis_cull_mode,
     uint8_t instances)
{
   return (prim_type << 0) | (source_select << 6) | ((index_size & 1) << 11) |
          ((index_size >> 1) << 13) | (vis_cull_mode << 9) | (1 << 14) |
          (instances << 24);
}

static inline uint32_t
DRAW_A20X(enum pc_di_primtype prim_type,
          enum pc_di_face_cull_sel faceness_cull_select,
          enum pc_di_src_sel source_select, enum pc_di_index_size index_size,
          bool pre_fetch_cull_enable, bool grp_cull_enable, uint16_t count)
{
   return (prim_type << 0) | (source_select << 6) |
          (faceness_cull_select << 8) | ((index_size & 1) << 11) |
          ((index_size >> 1) << 13) | (pre_fetch_cull_enable << 14) |
          (grp_cull_enable << 15) | (count << 16);
}

/* for tracking cmdstream positions that need to be patched: */
struct fd_cs_patch {
   uint32_t *cs;
   uint32_t val;
};
#define fd_patch_num_elements(buf) ((buf)->size / sizeof(struct fd_cs_patch))
#define fd_patch_element(buf, i)                                               \
   util_dynarray_element(buf, struct fd_cs_patch, i)

static inline enum pipe_format
pipe_surface_format(struct pipe_surface *psurf)
{
   if (!psurf)
      return PIPE_FORMAT_NONE;
   return psurf->format;
}

static inline bool
fd_surface_half_precision(const struct pipe_surface *psurf)
{
   enum pipe_format format;

   if (!psurf)
      return true;

   format = psurf->format;

   /* colors are provided in consts, which go through cov.f32f16, which will
    * break these values
    */
   if (util_format_is_pure_integer(format))
      return false;

   /* avoid losing precision on 32-bit float formats */
   if (util_format_is_float(format) &&
       util_format_get_component_bits(format, UTIL_FORMAT_COLORSPACE_RGB, 0) ==
          32)
      return false;

   return true;
}

static inline unsigned
fd_sampler_first_level(const struct pipe_sampler_view *view)
{
   if (view->target == PIPE_BUFFER)
      return 0;
   return view->u.tex.first_level;
}

static inline unsigned
fd_sampler_last_level(const struct pipe_sampler_view *view)
{
   if (view->target == PIPE_BUFFER)
      return 0;
   return view->u.tex.last_level;
}

static inline bool
fd_half_precision(struct pipe_framebuffer_state *pfb)
{
   unsigned i;

   for (i = 0; i < pfb->nr_cbufs; i++)
      if (!fd_surface_half_precision(pfb->cbufs[i]))
         return false;

   return true;
}

static inline void emit_marker(struct fd_ringbuffer *ring, int scratch_idx);

/* like OUT_RING() but appends a cmdstream patch point to 'buf' */
static inline void
OUT_RINGP(struct fd_ringbuffer *ring, uint32_t data, struct util_dynarray *buf)
{
   if (LOG_DWORDS) {
      DBG("ring[%p]: OUT_RINGP  %04x:  %08x", ring,
          (uint32_t)(ring->cur - ring->start), data);
   }
   util_dynarray_append(buf, struct fd_cs_patch,
                        ((struct fd_cs_patch){
                           .cs = ring->cur++,
                           .val = data,
                        }));
}

static inline void
__OUT_IB(struct fd_ringbuffer *ring, bool prefetch,
         struct fd_ringbuffer *target)
{
   if (target->cur == target->start)
      return;

   unsigned count = fd_ringbuffer_cmd_count(target);

   /* for debug after a lock up, write a unique counter value
    * to scratch6 for each IB, to make it easier to match up
    * register dumps to cmdstream.  The combination of IB and
    * DRAW (scratch7) is enough to "triangulate" the particular
    * draw that caused lockup.
    */
   emit_marker(ring, 6);

   for (unsigned i = 0; i < count; i++) {
      uint32_t dwords;
      OUT_PKT3(ring, prefetch ? CP_INDIRECT_BUFFER_PFE : CP_INDIRECT_BUFFER_PFD,
               2);
      dwords = fd_ringbuffer_emit_reloc_ring_full(ring, target, i) / 4;
      assert(dwords > 0);
      OUT_RING(ring, dwords);
      OUT_PKT2(ring);
   }

   emit_marker(ring, 6);
}

static inline void
__OUT_IB5(struct fd_ringbuffer *ring, struct fd_ringbuffer *target)
{
   if (target->cur == target->start)
      return;

   unsigned count = fd_ringbuffer_cmd_count(target);

   for (unsigned i = 0; i < count; i++) {
      uint32_t dwords;
      OUT_PKT7(ring, CP_INDIRECT_BUFFER, 3);
      dwords = fd_ringbuffer_emit_reloc_ring_full(ring, target, i) / 4;
      assert(dwords > 0);
      OUT_RING(ring, dwords);
   }
}

/* CP_SCRATCH_REG4 is used to hold base address for query results:
 * Note the scratch register move on a5xx+ but this is only used
 * for pre-a5xx hw queries where we cannot allocate the query buf
 * until the # of tiles is known.
 */
#define HW_QUERY_BASE_REG REG_AXXX_CP_SCRATCH_REG4

#ifdef DEBUG
#define __EMIT_MARKER 1
#else
#define __EMIT_MARKER 0
#endif

static inline void
emit_marker(struct fd_ringbuffer *ring, int scratch_idx)
{
   extern int32_t marker_cnt;
   unsigned reg = REG_AXXX_CP_SCRATCH_REG0 + scratch_idx;
   assert(reg != HW_QUERY_BASE_REG);
   if (reg == HW_QUERY_BASE_REG)
      return;
   if (__EMIT_MARKER) {
      OUT_WFI(ring);
      OUT_PKT0(ring, reg, 1);
      OUT_RING(ring, p_atomic_inc_return(&marker_cnt));
   }
}


/*
 * a3xx+ helpers:
 */

static inline enum a3xx_msaa_samples
fd_msaa_samples(unsigned samples)
{
   switch (samples) {
   default:
      unreachable("Unsupported samples");
   case 0:
   case 1:
      return MSAA_ONE;
   case 2:
      return MSAA_TWO;
   case 4:
      return MSAA_FOUR;
   case 8:
      return MSAA_EIGHT;
   }
}

#define A3XX_MAX_TEXEL_BUFFER_ELEMENTS_UINT (1 << 13)

/* Note that the Vulkan blob on a540 and 640 report a
 * maxTexelBufferElements of just 65536 (the GLES3.2 and Vulkan
 * minimum).
 */
#define A4XX_MAX_TEXEL_BUFFER_ELEMENTS_UINT (1 << 27)

static inline uint32_t
fd_clamp_buffer_size(enum pipe_format format, uint32_t size,
                     unsigned max_texel_buffer_elements)
{
   /* The spec says:
    *    The number of texels in the texel array is then clamped to the value of
    *    the implementation-dependent limit GL_MAX_TEXTURE_BUFFER_SIZE.
    *
    * So compute the number of texels, compare to GL_MAX_TEXTURE_BUFFER_SIZE and update it.
    */
   unsigned blocksize = util_format_get_blocksize(format);
   unsigned elements = MIN2(max_texel_buffer_elements, size / blocksize);

   return elements * blocksize;
}


/*
 * a4xx+ helpers:
 */

static inline enum a4xx_state_block
fd4_stage2shadersb(gl_shader_stage type)
{
   switch (type) {
   case MESA_SHADER_VERTEX:
      return SB4_VS_SHADER;
   case MESA_SHADER_FRAGMENT:
      return SB4_FS_SHADER;
   case MESA_SHADER_COMPUTE:
   case MESA_SHADER_KERNEL:
      return SB4_CS_SHADER;
   default:
      unreachable("bad shader type");
      return (enum a4xx_state_block) ~0;
   }
}

static inline enum a4xx_index_size
fd4_size2indextype(unsigned index_size)
{
   switch (index_size) {
   case 1:
      return INDEX4_SIZE_8_BIT;
   case 2:
      return INDEX4_SIZE_16_BIT;
   case 4:
      return INDEX4_SIZE_32_BIT;
   }
   DBG("unsupported index size: %d", index_size);
   assert(0);
   return INDEX4_SIZE_32_BIT;
}

/* Convert 19.2MHz RBBM always-on timer ticks to ns */
static inline uint64_t
ticks_to_ns(uint64_t ts)
{
   return ts * (1000000000 / 19200000);
}

#ifdef __cplusplus
}
#endif

#endif /* FREEDRENO_UTIL_H_ */
