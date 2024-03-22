/*
 * Copyright 2020 Valve Corporation
 * SPDX-License-Identifier: MIT
 *
 * Authors:
 *    Jonathan Marek <jonathan@marek.ca>
 */

#ifndef TU_UTIL_H
#define TU_UTIL_H

#include "tu_common.h"

#include "util/macros.h"
#include "util/u_math.h"
#include "util/format/u_format_pack.h"
#include "util/format/u_format_zs.h"
#include "compiler/shader_enums.h"

#include "vk_util.h"

#define TU_DEBUG(name) unlikely(tu_env.debug & TU_DEBUG_##name)

enum tu_debug_flags
{
   TU_DEBUG_STARTUP = 1 << 0,
   TU_DEBUG_NIR = 1 << 1,
   TU_DEBUG_NOBIN = 1 << 3,
   TU_DEBUG_SYSMEM = 1 << 4,
   TU_DEBUG_FORCEBIN = 1 << 5,
   TU_DEBUG_NOUBWC = 1 << 6,
   TU_DEBUG_NOMULTIPOS = 1 << 7,
   TU_DEBUG_NOLRZ = 1 << 8,
   TU_DEBUG_PERFC = 1 << 9,
   TU_DEBUG_FLUSHALL = 1 << 10,
   TU_DEBUG_SYNCDRAW = 1 << 11,
   TU_DEBUG_PUSH_CONSTS_PER_STAGE = 1 << 12,
   TU_DEBUG_GMEM = 1 << 13,
   TU_DEBUG_RAST_ORDER = 1 << 14,
   TU_DEBUG_UNALIGNED_STORE = 1 << 15,
   TU_DEBUG_LAYOUT = 1 << 16,
   TU_DEBUG_LOG_SKIP_GMEM_OPS = 1 << 17,
   TU_DEBUG_PERF = 1 << 18,
   TU_DEBUG_NOLRZFC = 1 << 19,
   TU_DEBUG_DYNAMIC = 1 << 20,
   TU_DEBUG_BOS = 1 << 21,
   TU_DEBUG_3D_LOAD = 1 << 22,
   TU_DEBUG_FDM = 1 << 23,
   TU_DEBUG_NOCONFORM = 1 << 24,
   TU_DEBUG_RD = 1 << 25,
};

struct tu_env {
    uint32_t debug;
};

extern struct tu_env tu_env;

void
tu_env_init(void);

/* Whenever we generate an error, pass it through this function. Useful for
 * debugging, where we can break on it. Only call at error site, not when
 * propagating errors. Might be useful to plug in a stack trace here.
 */

VkResult
__vk_startup_errorf(struct tu_instance *instance,
                    VkResult error,
                    const char *file,
                    int line,
                    const char *format,
                    ...) PRINTFLIKE(5, 6);

/* Prints startup errors if TU_DEBUG=startup is set or on a debug driver
 * build.
 */
#define vk_startup_errorf(instance, error, format, ...) \
   __vk_startup_errorf(instance, error, \
                       __FILE__, __LINE__, format, ##__VA_ARGS__)

void
__tu_finishme(const char *file, int line, const char *format, ...)
   PRINTFLIKE(3, 4);

/**
 * Print a FINISHME message, including its source location.
 */
#define tu_finishme(format, ...)                                             \
   do {                                                                      \
      static bool reported = false;                                          \
      if (!reported) {                                                       \
         __tu_finishme(__FILE__, __LINE__, format, ##__VA_ARGS__);           \
         reported = true;                                                    \
      }                                                                      \
   } while (0)

#define tu_stub()                                                            \
   do {                                                                      \
      tu_finishme("stub %s", __func__);                                      \
   } while (0)

void
tu_framebuffer_tiling_config(struct tu_framebuffer *fb,
                             const struct tu_device *device,
                             const struct tu_render_pass *pass);

#define TU_STAGE_MASK ((1 << MESA_SHADER_STAGES) - 1)

#define tu_foreach_stage(stage, stage_bits)                                  \
   for (gl_shader_stage stage,                                               \
        __tmp = (gl_shader_stage) ((stage_bits) &TU_STAGE_MASK);             \
        stage = (gl_shader_stage) (__builtin_ffs(__tmp) - 1), __tmp;         \
        __tmp = (gl_shader_stage) (__tmp & ~(1 << (stage))))

static inline enum a3xx_msaa_samples
tu_msaa_samples(uint32_t samples)
{
   assert(__builtin_popcount(samples) == 1);
   return (enum a3xx_msaa_samples) util_logbase2(samples);
}

static inline uint32_t
tu6_stage2opcode(gl_shader_stage stage)
{
   if (stage == MESA_SHADER_FRAGMENT || stage == MESA_SHADER_COMPUTE)
      return CP_LOAD_STATE6_FRAG;
   return CP_LOAD_STATE6_GEOM;
}

static inline enum a6xx_state_block
tu6_stage2texsb(gl_shader_stage stage)
{
   return (enum a6xx_state_block) (SB6_VS_TEX + stage);
}

static inline enum a6xx_state_block
tu6_stage2shadersb(gl_shader_stage stage)
{
   return (enum a6xx_state_block) (SB6_VS_SHADER + stage);
}

static inline enum a3xx_rop_code
tu6_rop(VkLogicOp op)
{
   /* note: hw enum matches the VK enum, but with the 4 bits reversed */
   static const enum a3xx_rop_code lookup[] = {
      [VK_LOGIC_OP_CLEAR]           = ROP_CLEAR,
      [VK_LOGIC_OP_AND]             = ROP_AND,
      [VK_LOGIC_OP_AND_REVERSE]     = ROP_AND_REVERSE,
      [VK_LOGIC_OP_COPY]            = ROP_COPY,
      [VK_LOGIC_OP_AND_INVERTED]    = ROP_AND_INVERTED,
      [VK_LOGIC_OP_NO_OP]           = ROP_NOOP,
      [VK_LOGIC_OP_XOR]             = ROP_XOR,
      [VK_LOGIC_OP_OR]              = ROP_OR,
      [VK_LOGIC_OP_NOR]             = ROP_NOR,
      [VK_LOGIC_OP_EQUIVALENT]      = ROP_EQUIV,
      [VK_LOGIC_OP_INVERT]          = ROP_INVERT,
      [VK_LOGIC_OP_OR_REVERSE]      = ROP_OR_REVERSE,
      [VK_LOGIC_OP_COPY_INVERTED]   = ROP_COPY_INVERTED,
      [VK_LOGIC_OP_OR_INVERTED]     = ROP_OR_INVERTED,
      [VK_LOGIC_OP_NAND]            = ROP_NAND,
      [VK_LOGIC_OP_SET]             = ROP_SET,
   };
   assert(op < ARRAY_SIZE(lookup));
   return lookup[op];
}

static inline bool
tu6_primtype_line(enum pc_di_primtype type)
{
    switch(type) {
    case DI_PT_LINELIST:
    case DI_PT_LINESTRIP:
    case DI_PT_LINE_ADJ:
    case DI_PT_LINESTRIP_ADJ:
       return true;
    default:
       return false;
    }
}

static inline bool
tu6_primtype_patches(enum pc_di_primtype type)
{
   return type >= DI_PT_PATCHES0 && type <= DI_PT_PATCHES31;
}

static inline enum pc_di_primtype
tu6_primtype(VkPrimitiveTopology topology)
{
   static const enum pc_di_primtype lookup[] = {
      [VK_PRIMITIVE_TOPOLOGY_POINT_LIST]                    = DI_PT_POINTLIST,
      [VK_PRIMITIVE_TOPOLOGY_LINE_LIST]                     = DI_PT_LINELIST,
      [VK_PRIMITIVE_TOPOLOGY_LINE_STRIP]                    = DI_PT_LINESTRIP,
      [VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST]                 = DI_PT_TRILIST,
      [VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP]                = DI_PT_TRISTRIP,
      [VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN]                  = DI_PT_TRIFAN,
      [VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY]      = DI_PT_LINE_ADJ,
      [VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY]     = DI_PT_LINESTRIP_ADJ,
      [VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY]  = DI_PT_TRI_ADJ,
      [VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY] = DI_PT_TRISTRIP_ADJ,
      /* Return PATCH0 and update in tu_pipeline_builder_parse_tessellation */
      [VK_PRIMITIVE_TOPOLOGY_PATCH_LIST]                    = DI_PT_PATCHES0,
   };
   assert(topology < ARRAY_SIZE(lookup));
   return lookup[topology];
}

static inline enum adreno_compare_func
tu6_compare_func(VkCompareOp op)
{
   return (enum adreno_compare_func) op;
}

static inline enum adreno_stencil_op
tu6_stencil_op(VkStencilOp op)
{
   return (enum adreno_stencil_op) op;
}

static inline enum adreno_rb_blend_factor
tu6_blend_factor(VkBlendFactor factor)
{
   static const enum adreno_rb_blend_factor lookup[] = {
      [VK_BLEND_FACTOR_ZERO]                    = FACTOR_ZERO,
      [VK_BLEND_FACTOR_ONE]                     = FACTOR_ONE,
      [VK_BLEND_FACTOR_SRC_COLOR]               = FACTOR_SRC_COLOR,
      [VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR]     = FACTOR_ONE_MINUS_SRC_COLOR,
      [VK_BLEND_FACTOR_DST_COLOR]               = FACTOR_DST_COLOR,
      [VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR]     = FACTOR_ONE_MINUS_DST_COLOR,
      [VK_BLEND_FACTOR_SRC_ALPHA]               = FACTOR_SRC_ALPHA,
      [VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA]     = FACTOR_ONE_MINUS_SRC_ALPHA,
      [VK_BLEND_FACTOR_DST_ALPHA]               = FACTOR_DST_ALPHA,
      [VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA]     = FACTOR_ONE_MINUS_DST_ALPHA,
      [VK_BLEND_FACTOR_CONSTANT_COLOR]          = FACTOR_CONSTANT_COLOR,
      [VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR]= FACTOR_ONE_MINUS_CONSTANT_COLOR,
      [VK_BLEND_FACTOR_CONSTANT_ALPHA]          = FACTOR_CONSTANT_ALPHA,
      [VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA]= FACTOR_ONE_MINUS_CONSTANT_ALPHA,
      [VK_BLEND_FACTOR_SRC_ALPHA_SATURATE]      = FACTOR_SRC_ALPHA_SATURATE,
      [VK_BLEND_FACTOR_SRC1_COLOR]              = FACTOR_SRC1_COLOR,
      [VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR]    = FACTOR_ONE_MINUS_SRC1_COLOR,
      [VK_BLEND_FACTOR_SRC1_ALPHA]              = FACTOR_SRC1_ALPHA,
      [VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA]    = FACTOR_ONE_MINUS_SRC1_ALPHA,
   };
   assert(factor < ARRAY_SIZE(lookup));
   return lookup[factor];
}

static inline bool
tu_blend_factor_is_dual_src(VkBlendFactor factor)
{
   switch (factor) {
   case VK_BLEND_FACTOR_SRC1_COLOR:
   case VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR:
   case VK_BLEND_FACTOR_SRC1_ALPHA:
   case VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA:
      return true;
   default:
      return false;
   }
}

static inline enum a3xx_rb_blend_opcode
tu6_blend_op(VkBlendOp op)
{
   return (enum a3xx_rb_blend_opcode) op;
}

static inline enum a6xx_tex_type
tu6_tex_type(VkImageViewType type, bool storage)
{
   switch (type) {
   default:
   case VK_IMAGE_VIEW_TYPE_1D:
   case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
      return A6XX_TEX_1D;
   case VK_IMAGE_VIEW_TYPE_2D:
   case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
      return A6XX_TEX_2D;
   case VK_IMAGE_VIEW_TYPE_3D:
      return A6XX_TEX_3D;
   case VK_IMAGE_VIEW_TYPE_CUBE:
   case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
      return storage ? A6XX_TEX_2D : A6XX_TEX_CUBE;
   }
}

static inline enum a6xx_tex_clamp
tu6_tex_wrap(VkSamplerAddressMode address_mode)
{
   static const enum a6xx_tex_clamp lookup[] = {
      [VK_SAMPLER_ADDRESS_MODE_REPEAT]                = A6XX_TEX_REPEAT,
      [VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT]       = A6XX_TEX_MIRROR_REPEAT,
      [VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE]         = A6XX_TEX_CLAMP_TO_EDGE,
      [VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER]       = A6XX_TEX_CLAMP_TO_BORDER,
      [VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE]  = A6XX_TEX_MIRROR_CLAMP,
   };
   assert(address_mode < ARRAY_SIZE(lookup));
   return lookup[address_mode];
}

static inline enum a6xx_tex_filter
tu6_tex_filter(VkFilter filter, unsigned aniso)
{
   switch (filter) {
   case VK_FILTER_NEAREST:
      return A6XX_TEX_NEAREST;
   case VK_FILTER_LINEAR:
      return aniso ? A6XX_TEX_ANISO : A6XX_TEX_LINEAR;
   case VK_FILTER_CUBIC_EXT:
      return A6XX_TEX_CUBIC;
   default:
      unreachable("illegal texture filter");
      break;
   }
}

static inline enum a6xx_reduction_mode
tu6_reduction_mode(VkSamplerReductionMode reduction_mode)
{
   return (enum a6xx_reduction_mode) reduction_mode;
}

static inline enum a6xx_depth_format
tu6_pipe2depth(VkFormat format)
{
   switch (format) {
   case VK_FORMAT_D16_UNORM:
      return DEPTH6_16;
   case VK_FORMAT_X8_D24_UNORM_PACK32:
   case VK_FORMAT_D24_UNORM_S8_UINT:
      return DEPTH6_24_8;
   case VK_FORMAT_D32_SFLOAT:
   case VK_FORMAT_D32_SFLOAT_S8_UINT:
   case VK_FORMAT_S8_UINT:
      return DEPTH6_32;
   default:
      return DEPTH6_NONE;
   }
}

static inline enum a6xx_polygon_mode
tu6_polygon_mode(VkPolygonMode mode)
{
   switch (mode) {
   case VK_POLYGON_MODE_POINT:
      return POLYMODE6_POINTS;
   case VK_POLYGON_MODE_LINE:
      return POLYMODE6_LINES;
   case VK_POLYGON_MODE_FILL:
      return POLYMODE6_TRIANGLES;
   default:
      unreachable("bad polygon mode");
   }
}

struct bcolor_entry {
   alignas(128) uint32_t fp32[4];
   uint64_t ui16;
   uint64_t si16;
   uint64_t fp16;
   uint16_t rgb565;
   uint16_t rgb5a1;
   uint16_t rgba4;
   uint8_t __pad0[2];
   uint32_t ui8;
   uint32_t si8;
   uint32_t rgb10a2;
   uint32_t z24; /* also s8? */
   uint64_t srgb;
   uint8_t  __pad1[56];
};
static_assert(alignof(struct bcolor_entry) == 128, "");

/* vulkan does not want clamping of integer clear values, differs from u_format
 * see spec for VkClearColorValue
 */
static inline void
pack_int8(uint32_t *dst, const uint32_t *val)
{
   *dst = (val[0] & 0xff) |
          (val[1] & 0xff) << 8 |
          (val[2] & 0xff) << 16 |
          (val[3] & 0xff) << 24;
}

static inline void
pack_int10_2(uint32_t *dst, const uint32_t *val)
{
   *dst = (val[0] & 0x3ff) |
          (val[1] & 0x3ff) << 10 |
          (val[2] & 0x3ff) << 20 |
          (val[3] & 0x3)   << 30;
}

static inline void
pack_int16(uint32_t *dst, const uint32_t *val)
{
   dst[0] = (val[0] & 0xffff) |
            (val[1] & 0xffff) << 16;
   dst[1] = (val[2] & 0xffff) |
            (val[3] & 0xffff) << 16;
}

static inline void
tu6_pack_border_color(struct bcolor_entry *bcolor, const VkClearColorValue *val, bool is_int)
{
   memcpy(bcolor->fp32, val, 4 * sizeof(float));
   if (is_int) {
      pack_int16((uint32_t*) &bcolor->fp16, val->uint32);
      return;
   }
#define PACK_F(x, type) util_format_##type##_pack_rgba_float \
   ( (uint8_t*) (&bcolor->x), 0, val->float32, 0, 1, 1)
   PACK_F(ui16, r16g16b16a16_unorm);
   PACK_F(si16, r16g16b16a16_snorm);
   PACK_F(fp16, r16g16b16a16_float);
   PACK_F(rgb565, r5g6b5_unorm);
   PACK_F(rgb5a1, r5g5b5a1_unorm);
   PACK_F(rgba4, r4g4b4a4_unorm);
   PACK_F(ui8, r8g8b8a8_unorm);
   PACK_F(si8, r8g8b8a8_snorm);
   PACK_F(rgb10a2, r10g10b10a2_unorm);
   util_format_z24x8_unorm_pack_z_float((uint8_t*) &bcolor->z24,
                                        0, val->float32, 0, 1, 1);
   PACK_F(srgb, r16g16b16a16_float); /* TODO: clamp? */
#undef PACK_F
}

void
tu_dbg_log_gmem_load_store_skips(struct tu_device *device);

#define perf_debug(device, fmt, ...) do {                               \
   if (TU_DEBUG(PERF))                                                  \
      mesa_log(MESA_LOG_WARN, (MESA_LOG_TAG), (fmt), ##__VA_ARGS__);    \
} while(0)

#define sizeof_field(s, field) sizeof(((s *) NULL)->field)

#define offsetof_arr(s, field, idx)                                          \
   (offsetof(s, field) + sizeof_field(s, field[0]) * (idx))

#endif /* TU_UTIL_H */
