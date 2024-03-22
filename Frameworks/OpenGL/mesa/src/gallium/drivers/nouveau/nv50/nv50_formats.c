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

#if NOUVEAU_DRIVER == 0xc0
# include "nvc0/nvc0_screen.h"
# include "nvc0/nvc0_3d.xml.h"
# include "nvc0/gm107_texture.xml.h"
#else
# include "nv50/nv50_screen.h"
# include "nv50/nv50_3d.xml.h"
#endif
#include "nv50/g80_texture.xml.h"
#include "nv50/g80_defs.xml.h"

#include "pipe/p_defines.h"

/* Abbreviated usage masks:
 * T: texturing
 * R: render target
 * B: render target, blendable
 * C: render target (color), blendable only on nvc0
 * D: scanout/display target, blendable
 * Z: depth/stencil
 * I: image / surface, implies T
 */
#define U_T   PIPE_BIND_SAMPLER_VIEW
#define U_I   PIPE_BIND_SHADER_BUFFER | PIPE_BIND_SHADER_IMAGE | PIPE_BIND_COMPUTE_RESOURCE
#define U_TR  PIPE_BIND_RENDER_TARGET | U_T
#define U_IR  U_TR | U_I
#define U_TB  PIPE_BIND_BLENDABLE | U_TR
#define U_IB  PIPE_BIND_BLENDABLE | U_IR
#define U_TD  PIPE_BIND_SCANOUT | PIPE_BIND_DISPLAY_TARGET | U_TB
#define U_TZ  PIPE_BIND_DEPTH_STENCIL | U_T
#define U_ID  U_TD | U_I
#if NOUVEAU_DRIVER == 0xc0
# define U_TC  U_TB
# define U_IC  U_IB
# define U_t   U_T
#else
# define U_TC  U_TR
# define U_IC  U_IR
# define U_t   0
#endif

#define G80_ZETA_FORMAT_NONE    0
#define G80_SURFACE_FORMAT_NONE 0

#define SF_A(sz) G80_TIC_0_COMPONENTS_SIZES_##sz
#define SF_B(sz) G200_TIC_0_COMPONENTS_SIZES_##sz
#define SF_C(sz) GF100_TIC_0_COMPONENTS_SIZES_##sz
#define SF_D(sz) GM107_TIC2_0_COMPONENTS_SIZES_##sz
#define SF(c, pf, sf, r, g, b, a, t0, t1, t2, t3, sz, u)                \
   [PIPE_FORMAT_##pf] = {                                               \
      sf, {                                                             \
         SF_##c(sz),                                                    \
         G80_TIC_TYPE_##t0,                                             \
         G80_TIC_TYPE_##t1,                                             \
         G80_TIC_TYPE_##t2,                                             \
         G80_TIC_TYPE_##t3,                                             \
         G80_TIC_SOURCE_##r,                                            \
         G80_TIC_SOURCE_##g,                                            \
         G80_TIC_SOURCE_##b,                                            \
         G80_TIC_SOURCE_##a,                                            \
      }, U_##u                                                          \
   }

#define C4(c, p, n, r, g, b, a, t, s, u)                                \
   SF(c, p, G80_SURFACE_FORMAT_##n, r, g, b, a, t, t, t, t, s, u)

#define ZX(c, p, n, r, g, b, a, t, s, u)                                \
   SF(c, p, G80_ZETA_FORMAT_##n,                                        \
      r, g, b, ONE_FLOAT, t, UINT, UINT, UINT, s, u)
#define ZS(c, p, n, r, g, b, a, t, s, u)                                \
   SF(c, p, G80_ZETA_FORMAT_##n,                                        \
      r, g, b, ONE_FLOAT, t, UINT, UINT, UINT, s, u)
#define SZ(c, p, n, r, g, b, a, t, s, u)                                \
   SF(c, p, G80_ZETA_FORMAT_##n,                                        \
      r, g, b, ONE_FLOAT, UINT, t, UINT, UINT, s, u)
#define SX(c, p, r, s, u)                                               \
   SF(c, p, G80_ZETA_FORMAT_NONE,                                       \
      r, r, r, r, UINT, UINT, UINT, UINT, s, u)

#define F3(c, p, n, r, g, b, a, t, s, u)         \
   C4(c, p, n, r, g, b, ONE_FLOAT, t, s, u)
#define I3(c, p, n, r, g, b, a, t, s, u)         \
   C4(c, p, n, r, g, b, ONE_INT, t, s, u)

#define F2(c, p, n, r, g, b, a, t, s, u)         \
   C4(c, p, n, r, g, ZERO, ONE_FLOAT, t, s, u)
#define I2(c, p, n, r, g, b, a, t, s, u)         \
   C4(c, p, n, r, g, ZERO, ONE_INT, t, s, u)

#define F1(c, p, n, r, g, b, a, t, s, u)         \
   C4(c, p, n, r, ZERO, ZERO, ONE_FLOAT, t, s, u)
#define I1(c, p, n, r, g, b, a, t, s, u)         \
   C4(c, p, n, r, ZERO, ZERO, ONE_INT, t, s, u)

#define A1(c, p, n, r, g, b, a, t, s, u)         \
   C4(c, p, n, ZERO, ZERO, ZERO, a, t, s, u)

#if NOUVEAU_DRIVER == 0xc0
const struct nvc0_format nvc0_format_table[PIPE_FORMAT_COUNT] =
#else
const struct nv50_format nv50_format_table[PIPE_FORMAT_COUNT] =
#endif
{
   C4(A, B8G8R8A8_UNORM, BGRA8_UNORM, B, G, R, A, UNORM, A8B8G8R8, ID),
   F3(A, B8G8R8X8_UNORM, BGRX8_UNORM, B, G, R, xx, UNORM, A8B8G8R8, TD),
   C4(A, B8G8R8A8_SRGB, BGRA8_SRGB, B, G, R, A, UNORM, A8B8G8R8, TD),
   F3(A, B8G8R8X8_SRGB, BGRX8_SRGB, B, G, R, xx, UNORM, A8B8G8R8, TD),
   C4(A, R8G8B8A8_UNORM, RGBA8_UNORM, R, G, B, A, UNORM, A8B8G8R8, IB),
   F3(A, R8G8B8X8_UNORM, RGBX8_UNORM, R, G, B, xx, UNORM, A8B8G8R8, TB),
   C4(A, R8G8B8A8_SRGB, RGBA8_SRGB, R, G, B, A, UNORM, A8B8G8R8, TB),
   F3(A, R8G8B8X8_SRGB, RGBX8_SRGB, R, G, B, xx, UNORM, A8B8G8R8, TB),

   ZX(B, Z16_UNORM, Z16_UNORM, R, R, R, xx, UNORM, Z16, TZ),
   ZX(A, Z32_FLOAT, Z32_FLOAT, R, R, R, xx, FLOAT, ZF32, TZ),
   ZX(A, Z24X8_UNORM, Z24_X8_UNORM, R, R, R, xx, UNORM, X8Z24, TZ),
   SZ(A, X8Z24_UNORM, S8_Z24_UNORM, G, G, G, xx, UNORM, Z24S8, TZ),
   ZS(A, Z24_UNORM_S8_UINT, Z24_S8_UNORM, R, R, R, xx, UNORM, S8Z24, TZ),
   SZ(A, S8_UINT_Z24_UNORM, S8_Z24_UNORM, G, G, G, xx, UNORM, Z24S8, TZ),
   ZS(A, Z32_FLOAT_S8X24_UINT, Z32_S8_X24_FLOAT, R, R, R, xx, FLOAT, ZF32_X24S8, TZ),

   SX(A, S8_UINT, R, R8, T),
   SX(A, X24S8_UINT, G, G8R24, T),
   SX(A, S8X24_UINT, R, G24R8, T),
   SX(A, X32_S8X24_UINT, G, R32_B24G8, T),

   F3(A, B5G6R5_UNORM, B5G6R5_UNORM, B, G, R, xx, UNORM, B5G6R5, TD),
   C4(A, B5G5R5A1_UNORM, BGR5_A1_UNORM, B, G, R, A, UNORM, A1B5G5R5, TD),
   F3(A, B5G5R5X1_UNORM, BGR5_X1_UNORM, B, G, R, xx, UNORM, A1B5G5R5, TD),
   C4(A, B4G4R4A4_UNORM, NONE, B, G, R, A, UNORM, A4B4G4R4, T),
   F3(A, B4G4R4X4_UNORM, NONE, B, G, R, xx, UNORM, A4B4G4R4, T),
   F3(A, R9G9B9E5_FLOAT, NONE, R, G, B, xx, FLOAT, E5B9G9R9_SHAREDEXP, T),

   C4(A, R10G10B10A2_UNORM, RGB10_A2_UNORM, R, G, B, A, UNORM, A2B10G10R10, ID),
   F3(A, R10G10B10X2_UNORM, RGB10_A2_UNORM, R, G, B, xx, UNORM, A2B10G10R10, T),
   C4(A, B10G10R10A2_UNORM, BGR10_A2_UNORM, B, G, R, A, UNORM, A2B10G10R10, TB),
   F3(A, B10G10R10X2_UNORM, BGR10_A2_UNORM, B, G, R, xx, UNORM, A2B10G10R10, T),
   C4(A, R10G10B10A2_SNORM, NONE, R, G, B, A, SNORM, A2B10G10R10, T),
   C4(A, B10G10R10A2_SNORM, NONE, B, G, R, A, SNORM, A2B10G10R10, T),
   C4(A, R10G10B10A2_UINT, RGB10_A2_UINT, R, G, B, A, UINT, A2B10G10R10, TR),
   C4(A, B10G10R10A2_UINT, RGB10_A2_UINT, B, G, R, A, UINT, A2B10G10R10, T),

   F3(A, R11G11B10_FLOAT, R11G11B10_FLOAT, R, G, B, xx, FLOAT, BF10GF11RF11, IB),

   F3(A, L8_UNORM, R8_UNORM, R, R, R, xx, UNORM, R8, TB),
   F3(A, L8_SRGB, NONE, R, R, R, xx, UNORM, R8, T),
   F3(A, L8_SNORM, R8_SNORM, R, R, R, xx, SNORM, R8, TC),
   I3(A, L8_SINT, R8_SINT, R, R, R, xx, SINT, R8, TR),
   I3(A, L8_UINT, R8_UINT, R, R, R, xx, UINT, R8, TR),
   F3(A, L16_UNORM, R16_UNORM, R, R, R, xx, UNORM, R16, TC),
   F3(A, L16_SNORM, R16_SNORM, R, R, R, xx, SNORM, R16, TC),
   F3(A, L16_FLOAT, R16_FLOAT, R, R, R, xx, FLOAT, R16, TB),
   I3(A, L16_SINT, R16_SINT, R, R, R, xx, SINT, R16, TR),
   I3(A, L16_UINT, R16_UINT, R, R, R, xx, UINT, R16, TR),
   F3(A, L32_FLOAT, R32_FLOAT, R, R, R, xx, FLOAT, R32, TB),
   I3(A, L32_SINT, R32_SINT, R, R, R, xx, SINT, R32, TR),
   I3(A, L32_UINT, R32_UINT, R, R, R, xx, UINT, R32, TR),

   C4(A, I8_UNORM, R8_UNORM, R, R, R, R, UNORM, R8, TR),
   C4(A, I8_SNORM, R8_SNORM, R, R, R, R, SNORM, R8, TR),
   C4(A, I8_SINT, R8_SINT, R, R, R, R, SINT, R8, TR),
   C4(A, I8_UINT, R8_UINT, R, R, R, R, UINT, R8, TR),
   C4(A, I16_UNORM, R16_UNORM, R, R, R, R, UNORM, R16, TR),
   C4(A, I16_SNORM, R16_SNORM, R, R, R, R, SNORM, R16, TR),
   C4(A, I16_FLOAT, R16_FLOAT, R, R, R, R, FLOAT, R16, TR),
   C4(A, I16_SINT, R16_SINT, R, R, R, R, SINT, R16, TR),
   C4(A, I16_UINT, R16_UINT, R, R, R, R, UINT, R16, TR),
   C4(A, I32_FLOAT, R32_FLOAT, R, R, R, R, FLOAT, R32, TR),
   C4(A, I32_SINT, R32_SINT, R, R, R, R, SINT, R32, TR),
   C4(A, I32_UINT, R32_UINT, R, R, R, R, UINT, R32, TR),

   A1(A, A8_UNORM, A8_UNORM, xx, xx, xx, R, UNORM, R8, TB),
   A1(A, A8_SNORM, R8_SNORM, xx, xx, xx, R, SNORM, R8, T),
   A1(A, A8_SINT, R8_SINT, xx, xx, xx, R, SINT, R8, T),
   A1(A, A8_UINT, R8_UINT, xx, xx, xx, R, UINT, R8, T),
   A1(A, A16_UNORM, R16_UNORM, xx, xx, xx, R, UNORM, R16, T),
   A1(A, A16_SNORM, R16_SNORM, xx, xx, xx, R, SNORM, R16, T),
   A1(A, A16_FLOAT, R16_FLOAT, xx, xx, xx, R, FLOAT, R16, T),
   A1(A, A16_SINT, R16_SINT, xx, xx, xx, R, SINT, R16, T),
   A1(A, A16_UINT, R16_UINT, xx, xx, xx, R, UINT, R16, T),
   A1(A, A32_FLOAT, R32_FLOAT, xx, xx, xx, R, FLOAT, R32, T),
   A1(A, A32_SINT, R32_SINT, xx, xx, xx, R, SINT, R32, T),
   A1(A, A32_UINT, R32_UINT, xx, xx, xx, R, UINT, R32, T),

   C4(A, L4A4_UNORM, NONE, R, R, R, G, UNORM, G4R4, T),
   C4(A, L8A8_UNORM, RG8_UNORM, R, R, R, G, UNORM, G8R8, T),
   C4(A, L8A8_SNORM, RG8_SNORM, R, R, R, G, SNORM, G8R8, T),
   C4(A, L8A8_SRGB, NONE, R, R, R, G, UNORM, G8R8, T),
   C4(A, L8A8_SINT, RG8_SINT, R, R, R, G, SINT, G8R8, T),
   C4(A, L8A8_UINT, RG8_UINT, R, R, R, G, UINT, G8R8, T),
   C4(A, L16A16_UNORM, RG16_UNORM, R, R, R, G, UNORM, R16_G16, T),
   C4(A, L16A16_SNORM, RG16_SNORM, R, R, R, G, SNORM, R16_G16, T),
   C4(A, L16A16_FLOAT, RG16_FLOAT, R, R, R, G, FLOAT, R16_G16, T),
   C4(A, L16A16_SINT, RG16_SINT, R, R, R, G, SINT, R16_G16, T),
   C4(A, L16A16_UINT, RG16_UINT, R, R, R, G, UINT, R16_G16, T),
   C4(A, L32A32_FLOAT, RG32_FLOAT, R, R, R, G, FLOAT, R32_G32, T),
   C4(A, L32A32_SINT, RG32_SINT, R, R, R, G, SINT, R32_G32, T),
   C4(A, L32A32_UINT, RG32_UINT, R, R, R, G, UINT, R32_G32, T),

   F3(A, DXT1_RGB,   NONE, R, G, B, xx, UNORM, DXT1, T),
   F3(A, DXT1_SRGB,  NONE, R, G, B, xx, UNORM, DXT1, T),
   C4(A, DXT1_RGBA,  NONE, R, G, B, A, UNORM, DXT1, T),
   C4(A, DXT1_SRGBA, NONE, R, G, B, A, UNORM, DXT1, T),
   C4(A, DXT3_RGBA,  NONE, R, G, B, A, UNORM, DXT23, T),
   C4(A, DXT3_SRGBA, NONE, R, G, B, A, UNORM, DXT23, T),
   C4(A, DXT5_RGBA,  NONE, R, G, B, A, UNORM, DXT45, T),
   C4(A, DXT5_SRGBA, NONE, R, G, B, A, UNORM, DXT45, T),

   F1(A, RGTC1_UNORM, NONE, R, xx, xx, xx, UNORM, DXN1, T),
   F1(A, RGTC1_SNORM, NONE, R, xx, xx, xx, SNORM, DXN1, T),
   F2(A, RGTC2_UNORM, NONE, R, G, xx, xx, UNORM, DXN2, T),
   F2(A, RGTC2_SNORM, NONE, R, G, xx, xx, SNORM, DXN2, T),
   F3(A, LATC1_UNORM, NONE, R, R, R, xx, UNORM, DXN1, T),
   F3(A, LATC1_SNORM, NONE, R, R, R, xx, SNORM, DXN1, T),
   C4(A, LATC2_UNORM, NONE, R, R, R, G, UNORM, DXN2, T),
   C4(A, LATC2_SNORM, NONE, R, R, R, G, SNORM, DXN2, T),

   C4(C, BPTC_RGBA_UNORM, NONE, R, G, B, A, UNORM, BC7U, t),
   C4(C, BPTC_SRGBA,      NONE, R, G, B, A, UNORM, BC7U, t),
   F3(C, BPTC_RGB_FLOAT,  NONE, R, G, B, xx, FLOAT, BC6H_SF16, t),
   F3(C, BPTC_RGB_UFLOAT, NONE, R, G, B, xx, FLOAT, BC6H_UF16, t),

#if NOUVEAU_DRIVER == 0xc0
   F3(D, ETC1_RGB8,       NONE, R,  G,  B, xx, UNORM, ETC2_RGB,     t),
   F3(D, ETC2_RGB8,       NONE, R,  G,  B, xx, UNORM, ETC2_RGB,     t),
   F3(D, ETC2_SRGB8,      NONE, R,  G,  B, xx, UNORM, ETC2_RGB,     t),
   C4(D, ETC2_RGB8A1,     NONE, R,  G,  B,  A, UNORM, ETC2_RGB_PTA, t),
   C4(D, ETC2_SRGB8A1,    NONE, R,  G,  B,  A, UNORM, ETC2_RGB_PTA, t),
   C4(D, ETC2_RGBA8,      NONE, R,  G,  B,  A, UNORM, ETC2_RGBA,    t),
   C4(D, ETC2_SRGBA8,     NONE, R,  G,  B,  A, UNORM, ETC2_RGBA,    t),
   F1(D, ETC2_R11_UNORM,  NONE, R, xx, xx, xx, UNORM, EAC,          t),
   F1(D, ETC2_R11_SNORM,  NONE, R, xx, xx, xx, SNORM, EAC,          t),
   F2(D, ETC2_RG11_UNORM, NONE, R,  G, xx, xx, UNORM, EACX2,        t),
   F2(D, ETC2_RG11_SNORM, NONE, R,  G, xx, xx, SNORM, EACX2,        t),

   C4(D, ASTC_4x4,        NONE, R, G, B, A, UNORM, ASTC_2D_4X4,   t),
   C4(D, ASTC_5x4,        NONE, R, G, B, A, UNORM, ASTC_2D_5X4,   t),
   C4(D, ASTC_5x5,        NONE, R, G, B, A, UNORM, ASTC_2D_5X5,   t),
   C4(D, ASTC_6x5,        NONE, R, G, B, A, UNORM, ASTC_2D_6X5,   t),
   C4(D, ASTC_6x6,        NONE, R, G, B, A, UNORM, ASTC_2D_6X6,   t),
   C4(D, ASTC_8x5,        NONE, R, G, B, A, UNORM, ASTC_2D_8X5,   t),
   C4(D, ASTC_8x6,        NONE, R, G, B, A, UNORM, ASTC_2D_8X6,   t),
   C4(D, ASTC_8x8,        NONE, R, G, B, A, UNORM, ASTC_2D_8X8,   t),
   C4(D, ASTC_10x5,       NONE, R, G, B, A, UNORM, ASTC_2D_10X5,  t),
   C4(D, ASTC_10x6,       NONE, R, G, B, A, UNORM, ASTC_2D_10X6,  t),
   C4(D, ASTC_10x8,       NONE, R, G, B, A, UNORM, ASTC_2D_10X8,  t),
   C4(D, ASTC_10x10,      NONE, R, G, B, A, UNORM, ASTC_2D_10X10, t),
   C4(D, ASTC_12x10,      NONE, R, G, B, A, UNORM, ASTC_2D_12X10, t),
   C4(D, ASTC_12x12,      NONE, R, G, B, A, UNORM, ASTC_2D_12X12, t),

   C4(D, ASTC_4x4_SRGB,   NONE, R, G, B, A, UNORM, ASTC_2D_4X4,   t),
   C4(D, ASTC_5x4_SRGB,   NONE, R, G, B, A, UNORM, ASTC_2D_5X4,   t),
   C4(D, ASTC_5x5_SRGB,   NONE, R, G, B, A, UNORM, ASTC_2D_5X5,   t),
   C4(D, ASTC_6x5_SRGB,   NONE, R, G, B, A, UNORM, ASTC_2D_6X5,   t),
   C4(D, ASTC_6x6_SRGB,   NONE, R, G, B, A, UNORM, ASTC_2D_6X6,   t),
   C4(D, ASTC_8x5_SRGB,   NONE, R, G, B, A, UNORM, ASTC_2D_8X5,   t),
   C4(D, ASTC_8x6_SRGB,   NONE, R, G, B, A, UNORM, ASTC_2D_8X6,   t),
   C4(D, ASTC_8x8_SRGB,   NONE, R, G, B, A, UNORM, ASTC_2D_8X8,   t),
   C4(D, ASTC_10x5_SRGB,  NONE, R, G, B, A, UNORM, ASTC_2D_10X5,  t),
   C4(D, ASTC_10x6_SRGB,  NONE, R, G, B, A, UNORM, ASTC_2D_10X6,  t),
   C4(D, ASTC_10x8_SRGB,  NONE, R, G, B, A, UNORM, ASTC_2D_10X8,  t),
   C4(D, ASTC_10x10_SRGB, NONE, R, G, B, A, UNORM, ASTC_2D_10X10, t),
   C4(D, ASTC_12x10_SRGB, NONE, R, G, B, A, UNORM, ASTC_2D_12X10, t),
   C4(D, ASTC_12x12_SRGB, NONE, R, G, B, A, UNORM, ASTC_2D_12X12, t),
#endif

   C4(A, R32G32B32A32_FLOAT, RGBA32_FLOAT, R, G, B, A, FLOAT, R32_G32_B32_A32, IB),
   C4(A, R32G32B32A32_UNORM, NONE, R, G, B, A, UNORM, R32_G32_B32_A32, T),
   C4(A, R32G32B32A32_SNORM, NONE, R, G, B, A, SNORM, R32_G32_B32_A32, T),
   C4(A, R32G32B32A32_SINT, RGBA32_SINT, R, G, B, A, SINT, R32_G32_B32_A32, IR),
   C4(A, R32G32B32A32_UINT, RGBA32_UINT, R, G, B, A, UINT, R32_G32_B32_A32, IR),
   F3(A, R32G32B32X32_FLOAT, RGBX32_FLOAT, R, G, B, xx, FLOAT, R32_G32_B32_A32, TB),
   I3(A, R32G32B32X32_SINT, RGBX32_SINT, R, G, B, xx, SINT, R32_G32_B32_A32, TR),
   I3(A, R32G32B32X32_UINT, RGBX32_UINT, R, G, B, xx, UINT, R32_G32_B32_A32, TR),

   F3(C, R32G32B32_FLOAT, NONE, R, G, B, xx, FLOAT, R32_G32_B32, t),
   I3(C, R32G32B32_SINT, NONE, R, G, B, xx, SINT, R32_G32_B32, t),
   I3(C, R32G32B32_UINT, NONE, R, G, B, xx, UINT, R32_G32_B32, t),

   F2(A, R32G32_FLOAT, RG32_FLOAT, R, G, xx, xx, FLOAT, R32_G32, IB),
   F2(A, R32G32_UNORM, NONE, R, G, xx, xx, UNORM, R32_G32, T),
   F2(A, R32G32_SNORM, NONE, R, G, xx, xx, SNORM, R32_G32, T),
   I2(A, R32G32_SINT, RG32_SINT, R, G, xx, xx, SINT, R32_G32, IR),
   I2(A, R32G32_UINT, RG32_UINT, R, G, xx, xx, UINT, R32_G32, IR),

   F1(A, R32_FLOAT, R32_FLOAT, R, xx, xx, xx, FLOAT, R32, IB),
   F1(A, R32_UNORM, NONE, R, xx, xx, xx, UNORM, R32, T),
   F1(A, R32_SNORM, NONE, R, xx, xx, xx, SNORM, R32, T),
   I1(A, R32_SINT, R32_SINT, R, xx, xx, xx, SINT, R32, IR),
   I1(A, R32_UINT, R32_UINT, R, xx, xx, xx, UINT, R32, IR),

   C4(A, R16G16B16A16_FLOAT, RGBA16_FLOAT, R, G, B, A, FLOAT, R16_G16_B16_A16, IB),
   C4(A, R16G16B16A16_UNORM, RGBA16_UNORM, R, G, B, A, UNORM, R16_G16_B16_A16, IC),
   C4(A, R16G16B16A16_SNORM, RGBA16_SNORM, R, G, B, A, SNORM, R16_G16_B16_A16, IC),
   C4(A, R16G16B16A16_SINT, RGBA16_SINT, R, G, B, A, SINT, R16_G16_B16_A16, IR),
   C4(A, R16G16B16A16_UINT, RGBA16_UINT, R, G, B, A, UINT, R16_G16_B16_A16, IR),
   F3(A, R16G16B16X16_FLOAT, RGBX16_FLOAT, R, G, B, xx, FLOAT, R16_G16_B16_A16, TB),
   F3(A, R16G16B16X16_UNORM, RGBA16_UNORM, R, G, B, xx, UNORM, R16_G16_B16_A16, T),
   F3(A, R16G16B16X16_SNORM, RGBA16_SNORM, R, G, B, xx, SNORM, R16_G16_B16_A16, T),
   I3(A, R16G16B16X16_SINT, RGBA16_SINT, R, G, B, xx, SINT, R16_G16_B16_A16, TR),
   I3(A, R16G16B16X16_UINT, RGBA16_UINT, R, G, B, xx, UINT, R16_G16_B16_A16, TR),

   F2(A, R16G16_FLOAT, RG16_FLOAT, R, G, xx, xx, FLOAT, R16_G16, IB),
   F2(A, R16G16_UNORM, RG16_UNORM, R, G, xx, xx, UNORM, R16_G16, IC),
   F2(A, R16G16_SNORM, RG16_SNORM, R, G, xx, xx, SNORM, R16_G16, IC),
   I2(A, R16G16_SINT, RG16_SINT, R, G, xx, xx, SINT, R16_G16, IR),
   I2(A, R16G16_UINT, RG16_UINT, R, G, xx, xx, UINT, R16_G16, IR),

   F1(A, R16_FLOAT, R16_FLOAT, R, xx, xx, xx, FLOAT, R16, IB),
   F1(A, R16_UNORM, R16_UNORM, R, xx, xx, xx, UNORM, R16, IC),
   F1(A, R16_SNORM, R16_SNORM, R, xx, xx, xx, SNORM, R16, IC),
   I1(A, R16_SINT, R16_SINT, R, xx, xx, xx, SINT, R16, IR),
   I1(A, R16_UINT, R16_UINT, R, xx, xx, xx, UINT, R16, IR),

   C4(A, R8G8B8A8_SNORM, RGBA8_SNORM, R, G, B, A, SNORM, A8B8G8R8, IC),
   C4(A, R8G8B8A8_SINT, RGBA8_SINT, R, G, B, A, SINT, A8B8G8R8, IR),
   C4(A, R8G8B8A8_UINT, RGBA8_UINT, R, G, B, A, UINT, A8B8G8R8, IR),
   F3(A, R8G8B8X8_SNORM, RGBA8_SNORM, R, G, B, xx, SNORM, A8B8G8R8, T),
   I3(A, R8G8B8X8_SINT, RGBA8_SINT, R, G, B, xx, SINT, A8B8G8R8, TR),
   I3(A, R8G8B8X8_UINT, RGBA8_UINT, R, G, B, xx, UINT, A8B8G8R8, TR),

   F2(A, R8G8_UNORM, RG8_UNORM, R, G, xx, xx, UNORM, G8R8, IB),
   F2(A, R8G8_SNORM, RG8_SNORM, R, G, xx, xx, SNORM, G8R8, IC),
   I2(A, R8G8_SINT, RG8_SINT, R, G, xx, xx, SINT, G8R8, IR),
   I2(A, R8G8_UINT, RG8_UINT, R, G, xx, xx, UINT, G8R8, IR),
#if NOUVEAU_DRIVER < 0xc0
   /* On Fermi+, the green component doesn't get decoding? */
   F2(A, R8G8_SRGB, NONE, R, G, xx, xx, UNORM, G8R8, T),
#endif

   F1(A, R8_UNORM, R8_UNORM, R, xx, xx, xx, UNORM, R8, IB),
   F1(A, R8_SNORM, R8_SNORM, R, xx, xx, xx, SNORM, R8, IC),
   I1(A, R8_SINT, R8_SINT, R, xx, xx, xx, SINT, R8, IR),
   I1(A, R8_UINT, R8_UINT, R, xx, xx, xx, UINT, R8, IR),
   F1(A, R8_SRGB, NONE, R, xx, xx, xx, UNORM, R8, T),

   F3(A, R8G8_B8G8_UNORM, NONE, R, G, B, xx, UNORM, G8B8G8R8, T),
   F3(A, G8R8_B8R8_UNORM, NONE, G, R, B, xx, UNORM, G8B8G8R8, T),
   F3(A, G8R8_G8B8_UNORM, NONE, R, G, B, xx, UNORM, B8G8R8G8, T),
   F3(A, R8G8_R8B8_UNORM, NONE, G, R, B, xx, UNORM, B8G8R8G8, T),

   F1(A, R1_UNORM, BITMAP, R, xx, xx, xx, UNORM, R1, T),

   C4(A, R4A4_UNORM, NONE, R, ZERO, ZERO, G, UNORM, G4R4, T),
   C4(A, R8A8_UNORM, NONE, R, ZERO, ZERO, G, UNORM, G8R8, T),
   C4(A, A4R4_UNORM, NONE, G, ZERO, ZERO, R, UNORM, G4R4, T),
   C4(A, A8R8_UNORM, NONE, G, ZERO, ZERO, R, UNORM, G8R8, T),

   SF(A, R8SG8SB8UX8U_NORM, 0, R, G, B, ONE_FLOAT, SNORM, SNORM, UNORM, UNORM, A8B8G8R8, T),
   SF(A, R5SG5SB6U_NORM, 0, R, G, B, ONE_FLOAT, SNORM, SNORM, UNORM, UNORM, B6G5R5, T),
};

#if NOUVEAU_DRIVER == 0xc0
# define NVXX_3D_VAF_SIZE(s) NVC0_3D_VERTEX_ATTRIB_FORMAT_SIZE_##s
# define NVXX_3D_VAF_TYPE(t) NVC0_3D_VERTEX_ATTRIB_FORMAT_TYPE_##t
#else
# define NVXX_3D_VAF_SIZE(s) NV50_3D_VERTEX_ARRAY_ATTRIB_FORMAT_##s
# define NVXX_3D_VAF_TYPE(t) NV50_3D_VERTEX_ARRAY_ATTRIB_TYPE_##t
#endif

#define VF_x(pf, type, size, bgra) 0
#define VF_A(pf, type, size, bgra)                                      \
      NVXX_3D_VAF_SIZE(size) | NVXX_3D_VAF_TYPE(type) | (bgra << 31)
#define VF(c, pf, type, size, bgra)                                     \
   [PIPE_FORMAT_##pf] = {                                               \
      VF_##c(pf, type, size, bgra),                                     \
      PIPE_BIND_VERTEX_BUFFER                                           \
   }

#if NOUVEAU_DRIVER == 0xc0
const struct nvc0_vertex_format nvc0_vertex_format[PIPE_FORMAT_COUNT] =
#else
const struct nv50_vertex_format nv50_vertex_format[PIPE_FORMAT_COUNT] =
#endif
{
   VF(A, B8G8R8A8_UNORM, UNORM, 8_8_8_8, 1),
   VF(A, R8G8B8A8_UNORM, UNORM, 8_8_8_8, 0),

   VF(A, R10G10B10A2_UNORM, UNORM, 10_10_10_2, 0),
   VF(A, B10G10R10A2_UNORM, UNORM, 10_10_10_2, 1),
   VF(A, R10G10B10A2_SNORM, SNORM, 10_10_10_2, 0),
   VF(A, B10G10R10A2_SNORM, SNORM, 10_10_10_2, 1),
   VF(A, R10G10B10A2_UINT, UINT, 10_10_10_2, 0),
   VF(A, B10G10R10A2_UINT, UINT, 10_10_10_2, 1),

   VF(A, R11G11B10_FLOAT, FLOAT, 11_11_10, 0),

   VF(A, R32G32B32A32_FLOAT, FLOAT, 32_32_32_32, 0),
   VF(A, R32G32B32A32_UNORM, UNORM, 32_32_32_32, 0),
   VF(A, R32G32B32A32_SNORM, SNORM, 32_32_32_32, 0),
   VF(A, R32G32B32A32_SINT, SINT, 32_32_32_32, 0),
   VF(A, R32G32B32A32_UINT, UINT, 32_32_32_32, 0),

   VF(A, R32G32_FLOAT, FLOAT, 32_32, 0),
   VF(A, R32G32_UNORM, UNORM, 32_32, 0),
   VF(A, R32G32_SNORM, SNORM, 32_32, 0),
   VF(A, R32G32_SINT, SINT, 32_32, 0),
   VF(A, R32G32_UINT, UINT, 32_32, 0),

   VF(A, R32_FLOAT, FLOAT, 32, 0),
   VF(A, R32_UNORM, UNORM, 32, 0),
   VF(A, R32_SNORM, SNORM, 32, 0),
   VF(A, R32_SINT, SINT, 32, 0),
   VF(A, R32_UINT, UINT, 32, 0),

   VF(A, R16G16B16A16_FLOAT, FLOAT, 16_16_16_16, 0),
   VF(A, R16G16B16A16_UNORM, UNORM, 16_16_16_16, 0),
   VF(A, R16G16B16A16_SNORM, SNORM, 16_16_16_16, 0),
   VF(A, R16G16B16A16_SINT, SINT, 16_16_16_16, 0),
   VF(A, R16G16B16A16_UINT, UINT, 16_16_16_16, 0),

   VF(A, R16G16_FLOAT, FLOAT, 16_16, 0),
   VF(A, R16G16_UNORM, UNORM, 16_16, 0),
   VF(A, R16G16_SNORM, SNORM, 16_16, 0),
   VF(A, R16G16_SINT, SINT, 16_16, 0),
   VF(A, R16G16_UINT, UINT, 16_16, 0),

   VF(A, R16_FLOAT, FLOAT, 16, 0),
   VF(A, R16_UNORM, UNORM, 16, 0),
   VF(A, R16_SNORM, SNORM, 16, 0),
   VF(A, R16_SINT, SINT, 16, 0),
   VF(A, R16_UINT, UINT, 16, 0),

   VF(A, R8G8B8A8_SNORM, SNORM, 8_8_8_8, 0),
   VF(A, R8G8B8A8_SINT, SINT, 8_8_8_8, 0),
   VF(A, R8G8B8A8_UINT, UINT, 8_8_8_8, 0),

   VF(A, R8G8_UNORM, UNORM, 8_8, 0),
   VF(A, R8G8_SNORM, SNORM, 8_8, 0),
   VF(A, R8G8_SINT, SINT, 8_8, 0),
   VF(A, R8G8_UINT, UINT, 8_8, 0),

   VF(A, R8_UNORM, UNORM, 8, 0),
   VF(A, R8_SNORM, SNORM, 8, 0),
   VF(A, R8_SINT, SINT, 8, 0),
   VF(A, R8_UINT, UINT, 8, 0),

   VF(A, R32G32B32A32_SSCALED, SSCALED, 32_32_32_32, 0),
   VF(A, R32G32B32A32_USCALED, USCALED, 32_32_32_32, 0),
   VF(A, R32G32B32_FLOAT, FLOAT, 32_32_32, 0),
   VF(A, R32G32B32_UNORM, UNORM, 32_32_32, 0),
   VF(A, R32G32B32_SNORM, SNORM, 32_32_32, 0),
   VF(A, R32G32B32_SINT, SINT, 32_32_32, 0),
   VF(A, R32G32B32_UINT, UINT, 32_32_32, 0),
   VF(A, R32G32B32_SSCALED, SSCALED, 32_32_32, 0),
   VF(A, R32G32B32_USCALED, USCALED, 32_32_32, 0),
   VF(A, R32G32_SSCALED, SSCALED, 32_32, 0),
   VF(A, R32G32_USCALED, USCALED, 32_32, 0),
   VF(A, R32_SSCALED, SSCALED, 32, 0),
   VF(A, R32_USCALED, USCALED, 32, 0),

   VF(A, R16G16B16A16_SSCALED, SSCALED, 16_16_16_16, 0),
   VF(A, R16G16B16A16_USCALED, USCALED, 16_16_16_16, 0),
   VF(A, R16G16B16_FLOAT, FLOAT, 16_16_16, 0),
   VF(A, R16G16B16_UNORM, UNORM, 16_16_16, 0),
   VF(A, R16G16B16_SNORM, SNORM, 16_16_16, 0),
   VF(A, R16G16B16_SINT, SINT, 16_16_16, 0),
   VF(A, R16G16B16_UINT, UINT, 16_16_16, 0),
   VF(A, R16G16B16_SSCALED, SSCALED, 16_16_16, 0),
   VF(A, R16G16B16_USCALED, USCALED, 16_16_16, 0),
   VF(A, R16G16_SSCALED, SSCALED, 16_16, 0),
   VF(A, R16G16_USCALED, USCALED, 16_16, 0),
   VF(A, R16_SSCALED, SSCALED, 16, 0),
   VF(A, R16_USCALED, USCALED, 16, 0),

   VF(A, R10G10B10A2_USCALED, USCALED, 10_10_10_2, 0),
   VF(A, R10G10B10A2_SSCALED, SSCALED, 10_10_10_2, 0),
   VF(A, B10G10R10A2_USCALED, USCALED, 10_10_10_2, 1),
   VF(A, B10G10R10A2_SSCALED, SSCALED, 10_10_10_2, 1),

   VF(A, R8G8B8A8_SSCALED, SSCALED, 8_8_8_8, 0),
   VF(A, R8G8B8A8_USCALED, USCALED, 8_8_8_8, 0),
   VF(A, R8G8B8_UNORM, UNORM, 8_8_8, 0),
   VF(A, R8G8B8_SNORM, SNORM, 8_8_8, 0),
   VF(A, R8G8B8_SINT, SINT, 8_8_8, 0),
   VF(A, R8G8B8_UINT, UINT, 8_8_8, 0),
   VF(A, R8G8B8_SSCALED, SSCALED, 8_8_8, 0),
   VF(A, R8G8B8_USCALED, USCALED, 8_8_8, 0),
   VF(A, R8G8_SSCALED, SSCALED, 8_8, 0),
   VF(A, R8G8_USCALED, USCALED, 8_8, 0),
   VF(A, R8_SSCALED, SSCALED, 8, 0),
   VF(A, R8_USCALED, USCALED, 8, 0),

   /* FIXED types: not supported natively, converted on VBO push */

   VF(x, R32G32B32A32_FIXED, xx, xx, xx),
   VF(x, R32G32B32_FIXED, xx, xx, xx),
   VF(x, R32G32_FIXED, xx, xx, xx),
   VF(x, R32_FIXED, xx, xx, xx),

   VF(x, R64G64B64A64_FLOAT, xx, xx, xx),
   VF(x, R64G64B64_FLOAT, xx, xx, xx),
   VF(x, R64G64_FLOAT, xx, xx, xx),
   VF(x, R64_FLOAT, xx, xx, xx),
};
