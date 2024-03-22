/*
 * Copyright © 2022 Collabora Ltd.
 * SPDX-License-Identifier: MIT
 */
#include "nil_format.h"

#include "nouveau_device.h"

#include "cl9097.h"
#include "cl9097tex.h"
#include "cla297.h"
#include "clb097.h"
#include "clb097tex.h"

enum nil_format_support_flags {
   NIL_FORMAT_SUPPORTS_TEXTURE_BIT        = BITFIELD_BIT(0),
   NIL_FORMAT_SUPPORTS_BUFFER_BIT         = BITFIELD_BIT(1),
   NIL_FORMAT_SUPPORTS_STORAGE_BIT        = BITFIELD_BIT(2),
   NIL_FORMAT_SUPPORTS_RENDER_BIT         = BITFIELD_BIT(3),
   NIL_FORMAT_SUPPORTS_ALPHA_BLEND_BIT    = BITFIELD_BIT(4),
   NIL_FORMAT_SUPPORTS_DEPTH_STENCIL_BIT  = BITFIELD_BIT(5),
   NIL_FORMAT_SUPPORTS_SCANOUT_BIT        = BITFIELD_BIT(6),
};

struct nil_format_info {
   /* Color of depth/stencil target format */
   unsigned czt:8;
   unsigned support:24;
   struct nil_tic_format tic;
};

/* Abbreviated usage masks:
 * T: texturing
 * B: texture buffer, implies T
 * R: render target
 * A: render target, alpha-blendable
 * C: render target (color), blendable only on nvc0
 * D: scanout/display target, blendable
 * Z: depth/stencil
 * I: image / surface, implies T
 * S: image / surface only
 */
#define U_T   NIL_FORMAT_SUPPORTS_TEXTURE_BIT
#define U_S   NIL_FORMAT_SUPPORTS_STORAGE_BIT
#define U_B   U_T | NIL_FORMAT_SUPPORTS_BUFFER_BIT
#define U_I   U_B | U_S
#define U_TR  NIL_FORMAT_SUPPORTS_RENDER_BIT | U_T
#define U_BR  NIL_FORMAT_SUPPORTS_RENDER_BIT | U_B
#define U_IR  NIL_FORMAT_SUPPORTS_RENDER_BIT | U_I
#define U_TA  NIL_FORMAT_SUPPORTS_ALPHA_BLEND_BIT | U_TR
#define U_IA  NIL_FORMAT_SUPPORTS_ALPHA_BLEND_BIT | U_IR
#define U_TD  NIL_FORMAT_SUPPORTS_SCANOUT_BIT | U_TA
#define U_TZ  NIL_FORMAT_SUPPORTS_DEPTH_STENCIL_BIT | U_T
#define U_ID  U_TD | U_I
#define U_TC  U_TA
#define U_IC  U_IA
#define U_t   U_T

#define SF_A(sz) NV9097_TEXHEAD0_COMPONENT_SIZES_##sz
#define SF_B(sz) NV9097_TEXHEAD0_COMPONENT_SIZES_##sz
#define SF_C(sz) NV9097_TEXHEAD0_COMPONENT_SIZES_##sz
#define SF_D(sz) NVB097_TEXHEAD_BL_COMPONENTS_SIZES_##sz
#define SF(c, pf, sf, r, g, b, a, t0, t1, t2, t3, sz, u)                \
   [PIPE_FORMAT_##pf] = {                                               \
      .czt = sf,                                                        \
      .support = U_##u,                                                 \
      .tic = {                                                          \
         SF_##c(sz),                                                    \
         NV9097_TEXHEAD0_R_DATA_TYPE_NUM_##t0,                          \
         NV9097_TEXHEAD0_G_DATA_TYPE_NUM_##t1,                          \
         NV9097_TEXHEAD0_B_DATA_TYPE_NUM_##t2,                          \
         NV9097_TEXHEAD0_A_DATA_TYPE_NUM_##t3,                          \
         NV9097_TEXHEAD0_X_SOURCE_IN_##r,                               \
         NV9097_TEXHEAD0_Y_SOURCE_IN_##g,                               \
         NV9097_TEXHEAD0_Z_SOURCE_IN_##b,                               \
         NV9097_TEXHEAD0_W_SOURCE_IN_##a,                               \
      }                                                                 \
   }

#define NV9097_SET_COLOR_TARGET_FORMAT_V_NONE \
   NV9097_SET_COLOR_TARGET_FORMAT_V_DISABLED

#define C4(c, p, n, r, g, b, a, t, s, u)                                \
   SF(c, p, NV9097_SET_COLOR_TARGET_FORMAT_V_##n, r, g, b, a, t, t, t, t, s, u)

#define ZX(c, p, n, r, t, s, u)                                         \
   SF(c, p, NV9097_SET_ZT_FORMAT_V_##n,                                 \
      r, ZERO, ZERO, ONE_FLOAT, t, UINT, UINT, UINT, s, u)
#define ZS(c, p, n, r, t, s, u)                                         \
   SF(c, p, NV9097_SET_ZT_FORMAT_V_##n,                                 \
      r, ZERO, ZERO, ONE_FLOAT, t, UINT, UINT, UINT, s, u)
#define SZ(c, p, n, r, t, s, u)                                         \
   SF(c, p, NV9097_SET_ZT_FORMAT_V_##n,                                 \
      r, ZERO, ZERO, ONE_FLOAT, UINT, t, UINT, UINT, s, u)
#define SX(c, p, r, s, u)                                               \
   SF(c, p, 0, r, ZERO, ZERO, ONE_INT, UINT, UINT, UINT, UINT, s, u)

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

static const struct nil_format_info nil_format_infos[PIPE_FORMAT_COUNT] =
{
   C4(A, B8G8R8A8_UNORM,   A8R8G8B8,      B, G, R, A, UNORM, A8B8G8R8, ID),
   F3(A, B8G8R8X8_UNORM,   X8R8G8B8,      B, G, R, x, UNORM, A8B8G8R8, TD),
   C4(A, B8G8R8A8_SRGB,    A8RL8GL8BL8,   B, G, R, A, UNORM, A8B8G8R8, TD),
   F3(A, B8G8R8X8_SRGB,    X8RL8GL8BL8,   B, G, R, x, UNORM, A8B8G8R8, TD),
   C4(A, R8G8B8A8_UNORM,   A8B8G8R8,      R, G, B, A, UNORM, A8B8G8R8, IA),
   F3(A, R8G8B8X8_UNORM,   X8B8G8R8,      R, G, B, x, UNORM, A8B8G8R8, TA),
   C4(A, R8G8B8A8_SRGB,    A8BL8GL8RL8,   R, G, B, A, UNORM, A8B8G8R8, TA),
   F3(A, R8G8B8X8_SRGB,    X8BL8GL8RL8,   R, G, B, x, UNORM, A8B8G8R8, TA),

   ZX(B, Z16_UNORM,              Z16,        R, UNORM,   Z16,        TZ),
   ZX(A, Z32_FLOAT,              ZF32,       R, FLOAT,   ZF32,       TZ),
   ZX(A, Z24X8_UNORM,            X8Z24,      R, UNORM,   X8Z24,      TZ),
   SZ(A, X8Z24_UNORM,            Z24S8,      G, UNORM,   Z24S8,      TZ),
   ZS(A, Z24_UNORM_S8_UINT,      S8Z24,      R, UNORM,   S8Z24,      TZ),
   SZ(A, S8_UINT_Z24_UNORM,      Z24S8,      G, UNORM,   Z24S8,      TZ),
   ZS(A, Z32_FLOAT_S8X24_UINT,   ZF32_X24S8, R, FLOAT,   ZF32_X24S8, TZ),

   SX(A, S8_UINT,          R, R8,         T),
   SX(A, X24S8_UINT,       G, G8R24,      T),
   SX(A, S8X24_UINT,       R, G24R8,      T),
   SX(A, X32_S8X24_UINT,   G, R32_B24G8,  T),

   F3(A, B5G6R5_UNORM,     R5G6B5,     B, G, R, x, UNORM,   B5G6R5,     TD),
   C4(A, B5G5R5A1_UNORM,   A1R5G5B5,   B, G, R, A, UNORM,   A1B5G5R5,   TD),
   F3(A, B5G5R5X1_UNORM,   X1R5G5B5,   B, G, R, x, UNORM,   A1B5G5R5,   TD),
   C4(A, A4B4G4R4_UNORM,   NONE,       A, B, G, R, UNORM,   A4B4G4R4,   T),
   C4(A, A4R4G4B4_UNORM,   NONE,       G, B, A, R, UNORM,   A4B4G4R4,   T),
   C4(A, B4G4R4A4_UNORM,   NONE,       B, G, R, A, UNORM,   A4B4G4R4,   T),
   F3(A, B4G4R4X4_UNORM,   NONE,       B, G, R, x, UNORM,   A4B4G4R4,   T),
   C4(A, R4G4B4A4_UNORM,   NONE,       R, G, B, A, UNORM,   A4B4G4R4,   T),
   F3(A, R4G4B4X4_UNORM,   NONE,       R, G, B, x, UNORM,   A4B4G4R4,   T),

   F3(A, R9G9B9E5_FLOAT, NONE, R, G, B, xx, FLOAT, E5B9G9R9_SHAREDEXP, T),

   C4(A, R10G10B10A2_UNORM,   A2B10G10R10,      R, G, B, A, UNORM,   A2B10G10R10, ID),
   F3(A, R10G10B10X2_UNORM,   A2B10G10R10,      R, G, B, x, UNORM,   A2B10G10R10, T),
   C4(A, B10G10R10A2_UNORM,   A2R10G10B10,      B, G, R, A, UNORM,   A2B10G10R10, TA),
   F3(A, B10G10R10X2_UNORM,   A2R10G10B10,      B, G, R, x, UNORM,   A2B10G10R10, T),
   C4(A, R10G10B10A2_SNORM,   NONE,             R, G, B, A, SNORM,   A2B10G10R10, T),
   C4(A, B10G10R10A2_SNORM,   NONE,             B, G, R, A, SNORM,   A2B10G10R10, T),
   C4(A, R10G10B10A2_UINT,    AU2BU10GU10RU10,  R, G, B, A, UINT,    A2B10G10R10, IR),
   C4(A, B10G10R10A2_UINT,    NONE,             B, G, R, A, UINT,    A2B10G10R10, B),

   F3(A, R11G11B10_FLOAT, BF10GF11RF11, R, G, B, x, FLOAT, BF10GF11RF11, IA),

   F3(A, L8_UNORM,   R8,   R, R, R, x, UNORM,   R8,   TA),
   F3(A, L8_SRGB,    NONE, R, R, R, x, UNORM,   R8,   T),
   F3(A, L8_SNORM,   RN8,  R, R, R, x, SNORM,   R8,   TC),
   I3(A, L8_SINT,    RS8,  R, R, R, x, SINT,    R8,   TR),
   I3(A, L8_UINT,    RU8,  R, R, R, x, UINT,    R8,   TR),
   F3(A, L16_UNORM,  R16,  R, R, R, x, UNORM,   R16,  TC),
   F3(A, L16_SNORM,  RN16, R, R, R, x, SNORM,   R16,  TC),
   F3(A, L16_FLOAT,  RF16, R, R, R, x, FLOAT,   R16,  TA),
   I3(A, L16_SINT,   RS16, R, R, R, x, SINT,    R16,  TR),
   I3(A, L16_UINT,   RU16, R, R, R, x, UINT,    R16,  TR),
   F3(A, L32_FLOAT,  RF32, R, R, R, x, FLOAT,   R32,  TA),
   I3(A, L32_SINT,   RS32, R, R, R, x, SINT,    R32,  TR),
   I3(A, L32_UINT,   RU32, R, R, R, x, UINT,    R32,  TR),

   C4(A, I8_UNORM,   R8,   R, R, R, R, UNORM,   R8,   TR),
   C4(A, I8_SNORM,   RN8,  R, R, R, R, SNORM,   R8,   TR),
   C4(A, I8_SINT,    RS8,  R, R, R, R, SINT,    R8,   TR),
   C4(A, I8_UINT,    RU8,  R, R, R, R, UINT,    R8,   TR),
   C4(A, I16_UNORM,  R16,  R, R, R, R, UNORM,   R16,  TR),
   C4(A, I16_SNORM,  RN16, R, R, R, R, SNORM,   R16,  TR),
   C4(A, I16_FLOAT,  RF16, R, R, R, R, FLOAT,   R16,  TR),
   C4(A, I16_SINT,   RS16, R, R, R, R, SINT,    R16,  TR),
   C4(A, I16_UINT,   RU16, R, R, R, R, UINT,    R16,  TR),
   C4(A, I32_FLOAT,  RF32, R, R, R, R, FLOAT,   R32,  TR),
   C4(A, I32_SINT,   RS32, R, R, R, R, SINT,    R32,  TR),
   C4(A, I32_UINT,   RU32, R, R, R, R, UINT,    R32,  TR),

   A1(A, A8_UNORM,   A8,   x, x, x, R, UNORM,   R8,   TA),
   A1(A, A8_SNORM,   RN8,  x, x, x, R, SNORM,   R8,   T),
   A1(A, A8_SINT,    RS8,  x, x, x, R, SINT,    R8,   T),
   A1(A, A8_UINT,    RU8,  x, x, x, R, UINT,    R8,   T),
   A1(A, A16_UNORM,  R16,  x, x, x, R, UNORM,   R16,  T),
   A1(A, A16_SNORM,  RN16, x, x, x, R, SNORM,   R16,  T),
   A1(A, A16_FLOAT,  AF16, x, x, x, R, FLOAT,   R16,  T),
   A1(A, A16_SINT,   RS16, x, x, x, R, SINT,    R16,  T),
   A1(A, A16_UINT,   RU16, x, x, x, R, UINT,    R16,  T),
   A1(A, A32_FLOAT,  AF32, x, x, x, R, FLOAT,   R32,  T),
   A1(A, A32_SINT,   RS32, x, x, x, R, SINT,    R32,  T),
   A1(A, A32_UINT,   RU32, x, x, x, R, UINT,    R32,  T),

   C4(A, L4A4_UNORM,    NONE,       R, R, R, G, UNORM,   G4R4,    T),
   C4(A, L8A8_UNORM,    G8R8,       R, R, R, G, UNORM,   G8R8,    T),
   C4(A, L8A8_SNORM,    GN8RN8,     R, R, R, G, SNORM,   G8R8,    T),
   C4(A, L8A8_SRGB,     NONE,       R, R, R, G, UNORM,   G8R8,    T),
   C4(A, L8A8_SINT,     GS8RS8,     R, R, R, G, SINT,    G8R8,    T),
   C4(A, L8A8_UINT,     GU8RU8,     R, R, R, G, UINT,    G8R8,    T),
   C4(A, L16A16_UNORM,  R16_G16,    R, R, R, G, UNORM,   R16_G16, T),
   C4(A, L16A16_SNORM,  RN16_GN16,  R, R, R, G, SNORM,   R16_G16, T),
   C4(A, L16A16_FLOAT,  RF16_GF16,  R, R, R, G, FLOAT,   R16_G16, T),
   C4(A, L16A16_SINT,   RS16_GS16,  R, R, R, G, SINT,    R16_G16, T),
   C4(A, L16A16_UINT,   RU16_GU16,  R, R, R, G, UINT,    R16_G16, T),
   C4(A, L32A32_FLOAT,  RF32_GF32,  R, R, R, G, FLOAT,   R32_G32, T),
   C4(A, L32A32_SINT,   RS32_GS32,  R, R, R, G, SINT,    R32_G32, T),
   C4(A, L32A32_UINT,   RU32_GU32,  R, R, R, G, UINT,    R32_G32, T),

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

   C4(A, R32G32B32A32_FLOAT,  RF32_GF32_BF32_AF32, R, G, B, A, FLOAT,   R32_G32_B32_A32, IA),
   C4(A, R32G32B32A32_UNORM,  NONE,                R, G, B, A, UNORM,   R32_G32_B32_A32, T),
   C4(A, R32G32B32A32_SNORM,  NONE,                R, G, B, A, SNORM,   R32_G32_B32_A32, T),
   C4(A, R32G32B32A32_SINT,   RS32_GS32_BS32_AS32, R, G, B, A, SINT,    R32_G32_B32_A32, IR),
   C4(A, R32G32B32A32_UINT,   RU32_GU32_BU32_AU32, R, G, B, A, UINT,    R32_G32_B32_A32, IR),
   F3(A, R32G32B32X32_FLOAT,  RF32_GF32_BF32_X32,  R, G, B, x, FLOAT,   R32_G32_B32_A32, TA),
   I3(A, R32G32B32X32_SINT,   RS32_GS32_BS32_X32,  R, G, B, x, SINT,    R32_G32_B32_A32, TR),
   I3(A, R32G32B32X32_UINT,   RU32_GU32_BU32_X32,  R, G, B, x, UINT,    R32_G32_B32_A32, TR),

   F3(C, R32G32B32_FLOAT, NONE, R, G, B, xx, FLOAT, R32_G32_B32, B),
   I3(C, R32G32B32_SINT, NONE, R, G, B, xx, SINT, R32_G32_B32, B),
   I3(C, R32G32B32_UINT, NONE, R, G, B, xx, UINT, R32_G32_B32, B),

   F2(A, R32G32_FLOAT,  RF32_GF32,  R, G, x, x,    FLOAT,   R32_G32, IA),
   F2(A, R32G32_UNORM,  NONE,       R, G, x, x,    UNORM,   R32_G32, T),
   F2(A, R32G32_SNORM,  NONE,       R, G, x, x,    SNORM,   R32_G32, T),
   I2(A, R32G32_SINT,   RS32_GS32,  R, G, x, x,    SINT,    R32_G32, IR),
   I2(A, R32G32_UINT,   RU32_GU32,  R, G, x, x,    UINT,    R32_G32, IR),

   F1(A, R32_FLOAT,  RF32, R, x, x, x, FLOAT,   R32, IA),
   F1(A, R32_UNORM,  NONE, R, x, x, x, UNORM,   R32, T),
   F1(A, R32_SNORM,  NONE, R, x, x, x, SNORM,   R32, T),
   I1(A, R32_SINT,   RS32, R, x, x, x, SINT,    R32, IR),
   I1(A, R32_UINT,   RU32, R, x, x, x, UINT,    R32, IR),

   I2(A, R64_SINT,   NONE, R, G, x, x, SINT,    R32_G32, S),
   I2(A, R64_UINT,   NONE, R, G, x, x, UINT,    R32_G32, S),

   C4(A, R16G16B16A16_FLOAT,  RF16_GF16_BF16_AF16, R, G, B, A,    FLOAT,   R16_G16_B16_A16, IA),
   C4(A, R16G16B16A16_UNORM,  R16_G16_B16_A16,     R, G, B, A,    UNORM,   R16_G16_B16_A16, IC),
   C4(A, R16G16B16A16_SNORM,  RN16_GN16_BN16_AN16, R, G, B, A,    SNORM,   R16_G16_B16_A16, IC),
   C4(A, R16G16B16A16_SINT,   RS16_GS16_BS16_AS16, R, G, B, A,    SINT,    R16_G16_B16_A16, IR),
   C4(A, R16G16B16A16_UINT,   RU16_GU16_BU16_AU16, R, G, B, A,    UINT,    R16_G16_B16_A16, IR),
   F3(A, R16G16B16X16_FLOAT,  RF16_GF16_BF16_X16,  R, G, B, x,    FLOAT,   R16_G16_B16_A16, TA),
   F3(A, R16G16B16X16_UNORM,  R16_G16_B16_A16,     R, G, B, x,    UNORM,   R16_G16_B16_A16, T),
   F3(A, R16G16B16X16_SNORM,  RN16_GN16_BN16_AN16, R, G, B, x,    SNORM,   R16_G16_B16_A16, T),
   I3(A, R16G16B16X16_SINT,   RS16_GS16_BS16_AS16, R, G, B, x,    SINT,    R16_G16_B16_A16, TR),
   I3(A, R16G16B16X16_UINT,   RU16_GU16_BU16_AU16, R, G, B, x,    UINT,    R16_G16_B16_A16, TR),

   F2(A, R16G16_FLOAT,  RF16_GF16,  R, G, x, x, FLOAT,   R16_G16, IA),
   F2(A, R16G16_UNORM,  R16_G16,    R, G, x, x, UNORM,   R16_G16, IC),
   F2(A, R16G16_SNORM,  RN16_GN16,  R, G, x, x, SNORM,   R16_G16, IC),
   I2(A, R16G16_SINT,   RS16_GS16,  R, G, x, x, SINT,    R16_G16, IR),
   I2(A, R16G16_UINT,   RU16_GU16,  R, G, x, x, UINT,    R16_G16, IR),

   F1(A, R16_FLOAT,  RF16, R, x, x, x, FLOAT,   R16, IA),
   F1(A, R16_UNORM,  R16,  R, x, x, x, UNORM,   R16, IC),
   F1(A, R16_SNORM,  RN16, R, x, x, x, SNORM,   R16, IC),
   I1(A, R16_SINT,   RS16, R, x, x, x, SINT,    R16, IR),
   I1(A, R16_UINT,   RU16, R, x, x, x, UINT,    R16, IR),

   C4(A, R8G8B8A8_SNORM,   AN8BN8GN8RN8,  R, G, B, A, SNORM,   A8B8G8R8, IC),
   C4(A, R8G8B8A8_SINT,    AS8BS8GS8RS8,  R, G, B, A, SINT,    A8B8G8R8, IR),
   C4(A, R8G8B8A8_UINT,    AU8BU8GU8RU8,  R, G, B, A, UINT,    A8B8G8R8, IR),
   F3(A, R8G8B8X8_SNORM,   AN8BN8GN8RN8,  R, G, B, x, SNORM,   A8B8G8R8, T),
   I3(A, R8G8B8X8_SINT,    AS8BS8GS8RS8,  R, G, B, x, SINT,    A8B8G8R8, TR),
   I3(A, R8G8B8X8_UINT,    AU8BU8GU8RU8,  R, G, B, x, UINT,    A8B8G8R8, TR),

   F2(A, R8G8_UNORM, G8R8,    R, G, x, x, UNORM,   G8R8, IA),
   F2(A, R8G8_SNORM, GN8RN8,  R, G, x, x, SNORM,   G8R8, IC),
   I2(A, R8G8_SINT,  GS8RS8,  R, G, x, x, SINT,    G8R8, IR),
   I2(A, R8G8_UINT,  GU8RU8,  R, G, x, x, UINT,    G8R8, IR),
#if 0
   /* On Fermi+, the green component doesn't get decoding? */
   F2(A, R8G8_SRGB,  NONE,    R, G, x, x, UNORM,   G8R8, T),
#endif

   F1(A, R8_UNORM,   R8,   R, x, x, x, UNORM,   R8, IA),
   F1(A, R8_SNORM,   RN8,  R, x, x, x, SNORM,   R8, IC),
   I1(A, R8_SINT,    RS8,  R, x, x, x, SINT,    R8, IR),
   I1(A, R8_UINT,    RU8,  R, x, x, x, UINT,    R8, IR),
   F1(A, R8_SRGB,    NONE, R, x, x, x, UNORM,   R8, T),

   F3(A, R8G8_B8G8_UNORM, NONE, R, G, B, xx, UNORM, G8B8G8R8, T),
   F3(A, G8R8_B8R8_UNORM, NONE, G, R, B, xx, UNORM, G8B8G8R8, T),
   F3(A, G8R8_G8B8_UNORM, NONE, R, G, B, xx, UNORM, B8G8R8G8, T),
   F3(A, R8G8_R8B8_UNORM, NONE, G, R, B, xx, UNORM, B8G8R8G8, T),
   F3(A, G8B8_G8R8_UNORM, NONE, B, G, R, xx, UNORM, B8G8R8G8, T),
   F3(A, B8G8_R8G8_UNORM, NONE, B, G, R, xx, UNORM, G8B8G8R8, T),

   F1(A, R1_UNORM, NONE, R, xx, xx, xx, UNORM, R1, T),

   C4(A, R4A4_UNORM, NONE, R, ZERO, ZERO, G, UNORM, G4R4, T),
   C4(A, R8A8_UNORM, NONE, R, ZERO, ZERO, G, UNORM, G8R8, T),
   C4(A, A4R4_UNORM, NONE, G, ZERO, ZERO, R, UNORM, G4R4, T),
   C4(A, A8R8_UNORM, NONE, G, ZERO, ZERO, R, UNORM, G8R8, T),

   SF(A, R8SG8SB8UX8U_NORM, 0, R, G, B, ONE_FLOAT, SNORM, SNORM, UNORM, UNORM, A8B8G8R8, T),
   SF(A, R5SG5SB6U_NORM, 0, R, G, B, ONE_FLOAT, SNORM, SNORM, UNORM, UNORM, B6G5R5, T),
};

bool
nil_format_supports_texturing(struct nv_device_info *dev,
                              enum pipe_format format)
{
   assert(format < PIPE_FORMAT_COUNT);
   const struct nil_format_info *fmt = &nil_format_infos[format];
   if (!(fmt->support & NIL_FORMAT_SUPPORTS_TEXTURE_BIT))
      return false;

   /* The image/texture hardware doesn't clamp 2-bit SNORM alpha components to
    * [-1, 1] which is out-of-spec according to Vulkan.  Fortunately, as of
    * September 14, 2022, according to vulkan.gpuinfo.org, these formats are
    * only supported by lavapipe and a handful of mobile phones.
    */
   if (fmt->tic.comp_sizes == NV9097_TEXHEAD0_COMPONENT_SIZES_A2B10G10R10 &&
       fmt->tic.type_r == NV9097_TEXHEAD0_A_DATA_TYPE_NUM_SNORM)
      return false;

   const struct util_format_description *desc = util_format_description(format);
   if (desc->layout == UTIL_FORMAT_LAYOUT_ETC ||
       desc->layout == UTIL_FORMAT_LAYOUT_ASTC) {
      return dev->type == NV_DEVICE_TYPE_SOC && dev->cls_eng3d >= KEPLER_C;
   }

   return true;
}

bool
nil_format_supports_filtering(struct nv_device_info *dev,
                              enum pipe_format format)
{
   return nil_format_supports_texturing(dev, format) &&
          !util_format_is_pure_integer(format);
}

bool
nil_format_supports_buffer(struct nv_device_info *dev,
                           enum pipe_format format)
{
   assert(format < PIPE_FORMAT_COUNT);
   const struct nil_format_info *fmt = &nil_format_infos[format];
   return fmt->support & NIL_FORMAT_SUPPORTS_BUFFER_BIT;
}

bool
nil_format_supports_storage(struct nv_device_info *dev,
                            enum pipe_format format)
{
   if ((format == PIPE_FORMAT_R64_UINT || format == PIPE_FORMAT_R64_SINT) &&
       dev->cls_eng3d < MAXWELL_A)
      return false;

   assert(format < PIPE_FORMAT_COUNT);
   const struct nil_format_info *fmt = &nil_format_infos[format];
   return fmt->support & NIL_FORMAT_SUPPORTS_STORAGE_BIT;
}

bool
nil_format_supports_color_targets(struct nv_device_info *dev,
                                  enum pipe_format format)
{
   assert(format < PIPE_FORMAT_COUNT);
   const struct nil_format_info *fmt = &nil_format_infos[format];
   return fmt->support & NIL_FORMAT_SUPPORTS_RENDER_BIT;
}

bool
nil_format_supports_blending(struct nv_device_info *dev,
                             enum pipe_format format)
{
   assert(format < PIPE_FORMAT_COUNT);
   const struct nil_format_info *fmt = &nil_format_infos[format];
   return fmt->support & NIL_FORMAT_SUPPORTS_ALPHA_BLEND_BIT;
}

bool
nil_format_supports_depth_stencil(struct nv_device_info *dev,
                                  enum pipe_format format)
{
   assert(format < PIPE_FORMAT_COUNT);
   const struct nil_format_info *fmt = &nil_format_infos[format];
   return fmt->support & NIL_FORMAT_SUPPORTS_DEPTH_STENCIL_BIT;
}

uint8_t
nil_format_to_color_target(enum pipe_format format)
{
   assert(format < PIPE_FORMAT_COUNT);
   const struct nil_format_info *fmt = &nil_format_infos[format];
   assert(fmt->support & NIL_FORMAT_SUPPORTS_RENDER_BIT);
   return fmt->czt;
}

uint8_t
nil_format_to_depth_stencil(enum pipe_format format)
{
   assert(format < PIPE_FORMAT_COUNT);
   const struct nil_format_info *fmt = &nil_format_infos[format];
   assert(fmt->support & NIL_FORMAT_SUPPORTS_DEPTH_STENCIL_BIT);
   return fmt->czt;
}

const struct nil_tic_format *
nil_tic_format_for_pipe(enum pipe_format format)
{
   assert(format < PIPE_FORMAT_COUNT);
   const struct nil_format_info *fmt = &nil_format_infos[format];
   return fmt->tic.comp_sizes == 0 ? NULL : &fmt->tic;
}
