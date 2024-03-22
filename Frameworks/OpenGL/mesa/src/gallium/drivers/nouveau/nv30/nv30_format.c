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

#include "nv30/nv30-40_3d.xml.h"
#include "nv30/nv30_context.h"
#include "nv30/nv30_format.h"

#define NV30_3D_RT_FORMAT_COLOR_X1R5G5B5 2

#define NV30_3D_TEX_FORMAT_FORMAT_A16L16 NV30_3D_TEX_FORMAT_FORMAT_HILO16
#define NV30_3D_TEX_FORMAT_FORMAT_A16L16_RECT NV30_3D_TEX_FORMAT_FORMAT_HILO16_RECT
#define NV30_3D_TEX_FORMAT_FORMAT_RGBA16F 0x00004a00
#define NV30_3D_TEX_FORMAT_FORMAT_RGBA16F_RECT NV30_3D_TEX_FORMAT_FORMAT_RGBA16F
#define NV30_3D_TEX_FORMAT_FORMAT_RGBA32F 0x00004b00
#define NV30_3D_TEX_FORMAT_FORMAT_RGBA32F_RECT NV30_3D_TEX_FORMAT_FORMAT_RGBA32F
#define NV30_3D_TEX_FORMAT_FORMAT_R32F 0x00004c00
#define NV30_3D_TEX_FORMAT_FORMAT_R32F_RECT NV30_3D_TEX_FORMAT_FORMAT_R32F
#define NV30_3D_TEX_FORMAT_FORMAT_DXT1_RECT NV30_3D_TEX_FORMAT_FORMAT_DXT1
#define NV30_3D_TEX_FORMAT_FORMAT_DXT3_RECT NV30_3D_TEX_FORMAT_FORMAT_DXT3
#define NV30_3D_TEX_FORMAT_FORMAT_DXT5_RECT NV30_3D_TEX_FORMAT_FORMAT_DXT5
#define NV30_3D_TEX_FORMAT_FORMAT_RG16F 0xdeadcafe
#define NV30_3D_TEX_FORMAT_FORMAT_RG16F_RECT 0xdeadcafe

#define NV40_3D_TEX_FORMAT_FORMAT_R32F 0x00001c00
#define NV40_3D_TEX_FORMAT_FORMAT_RG16F 0x00001f00

#define ____ 0
#define S___ PIPE_BIND_SAMPLER_VIEW
#define _R__ PIPE_BIND_RENDER_TARGET
#define _B__ PIPE_BIND_RENDER_TARGET | PIPE_BIND_BLENDABLE
#define _D__ PIPE_BIND_SCANOUT | PIPE_BIND_DISPLAY_TARGET | _B__
#define _Z__ PIPE_BIND_DEPTH_STENCIL
#define __V_ PIPE_BIND_VERTEX_BUFFER
#define SR__ (S___ | _R__)
#define SB__ (S___ | _B__)
#define SD__ (S___ | _D__)
#define SZ__ (S___ | _Z__)
#define S_V_ (S___ | __V_)
#define SRV_ (SR__ | __V_)
#define SBV_ (SB__ | __V_)

#define _(a,b) [PIPE_FORMAT_##a] = {                                           \
   .bindings = (b),                                                            \
}
const struct nv30_format_info
nv30_format_info_table[PIPE_FORMAT_COUNT] = {
   _(L8_UNORM            , S___),
   _(L8_SNORM            , S___),
   _(L8_SRGB             , S___),
   _(I8_UNORM            , S___),
   _(I8_SNORM            , S___),
   _(A8_UNORM            , S___),
   _(A8_SNORM            , S___),
   _(R8_UNORM            , S_V_),
   _(R8_SNORM            , S___),
   _(B5G5R5X1_UNORM      , SD__),
   _(B5G5R5A1_UNORM      , S___),
   _(B4G4R4X4_UNORM      , S___),
   _(B4G4R4A4_UNORM      , S___),
   _(B5G6R5_UNORM        , SD__),
   _(BGRX8888_UNORM      , SD__),
   _(BGRX8888_SRGB       , S___),
   _(BGRA8888_UNORM      , SD__),
   _(BGRA8888_SRGB       , S___),
   _(R8G8B8A8_UNORM      , __V_),
   _(RGBA8888_SNORM      , S___),
   _(DXT1_RGB            , S___),
   _(DXT1_SRGB           , S___),
   _(DXT1_RGBA           , S___),
   _(DXT1_SRGBA          , S___),
   _(DXT3_RGBA           , S___),
   _(DXT3_SRGBA          , S___),
   _(DXT5_RGBA           , S___),
   _(DXT5_SRGBA          , S___),
   _(L8A8_UNORM          , S___),
   _(L8A8_SRGB           , S___),
   _(R8G8_UNORM          , S_V_),
   _(R8G8_SNORM          , S___),
   _(R8G8B8_UNORM        , __V_),
   _(Z16_UNORM           , SZ__),
   _(X8Z24_UNORM         , SZ__),
   _(S8_UINT_Z24_UNORM   , SZ__),
   _(L16_UNORM           , S___),
   _(L16_SNORM           , S___),
   _(I16_UNORM           , S___),
   _(I16_SNORM           , S___),
   _(A16_UNORM           , S___),
   _(A16_SNORM           , S___),
   _(R16_UNORM           , S___),
   _(R16_SNORM           , S_V_),
   _(R16G16_SNORM        , __V_),
   _(R16G16B16_SNORM     , __V_),
   _(R16G16B16A16_SNORM  , __V_),
   _(R8G8B8A8_USCALED    , __V_),
   _(R16_FLOAT           , __V_),
   _(R16G16_FLOAT        , __V_), //S_V_),
   _(R16G16B16_FLOAT     , __V_),
   _(R16G16B16A16_FLOAT  , __V_), //SBV_),
   _(R16_SSCALED         , __V_),
   _(R16G16_SSCALED      , __V_),
   _(R16G16B16_SSCALED   , __V_),
   _(R16G16B16A16_SSCALED, __V_),
   _(R32_FLOAT           , __V_), //SRV_),
   _(R32G32_FLOAT        , __V_),
   _(R32G32B32_FLOAT     , __V_),
   _(R32G32B32A32_FLOAT  , __V_), //SRV_),
};
#undef _
#undef ____

#define R_(a,b) [PIPE_FORMAT_##a] = {                                          \
   .hw = NV30_3D_RT_FORMAT_COLOR_##b,                                          \
}
#define Z_(a,b) [PIPE_FORMAT_##a] = {                                          \
   .hw = NV30_3D_RT_FORMAT_ZETA_##b,                                           \
}
const struct nv30_format
nv30_format_table[PIPE_FORMAT_COUNT] = {
   R_(B5G5R5X1_UNORM    , X1R5G5B5          ),
   R_(B5G6R5_UNORM      , R5G6B5            ),
   R_(BGRX8888_UNORM    , X8R8G8B8          ),
   R_(BGRA8888_UNORM    , A8R8G8B8          ),
   Z_(Z16_UNORM         , Z16               ),
   Z_(X8Z24_UNORM       , Z24S8             ),
   Z_(S8_UINT_Z24_UNORM , Z24S8             ),
   R_(R16G16B16A16_FLOAT, A16B16G16R16_FLOAT),
   R_(R32G32B32A32_FLOAT, A32B32G32R32_FLOAT),
   R_(R32_FLOAT         , R32_FLOAT         ),
};

#define _(a,b,c) [PIPE_FORMAT_##a] = {                                         \
   .hw = NV30_3D_VTXFMT_TYPE_##b | ((c) << NV30_3D_VTXFMT_SIZE__SHIFT)         \
}
const struct nv30_vtxfmt
nv30_vtxfmt_table[PIPE_FORMAT_COUNT] = {
   _(R8_UNORM            , U8_UNORM   , 1),
   _(R8G8_UNORM          , U8_UNORM   , 2),
   _(R8G8B8_UNORM        , U8_UNORM   , 3),
   _(R8G8B8A8_UNORM      , U8_UNORM   , 4),
   _(R8G8B8A8_USCALED    , U8_USCALED , 4),
   _(R16_SNORM           , V16_SNORM  , 1),
   _(R16G16_SNORM        , V16_SNORM  , 2),
   _(R16G16B16_SNORM     , V16_SNORM  , 3),
   _(R16G16B16A16_SNORM  , V16_SNORM  , 4),
   _(R16_SSCALED         , V16_SSCALED, 1),
   _(R16G16_SSCALED      , V16_SSCALED, 2),
   _(R16G16B16_SSCALED   , V16_SSCALED, 3),
   _(R16G16B16A16_SSCALED, V16_SSCALED, 4),
   _(R16_FLOAT           , V16_FLOAT  , 1),
   _(R16G16_FLOAT        , V16_FLOAT  , 2),
   _(R16G16B16_FLOAT     , V16_FLOAT  , 3),
   _(R16G16B16A16_FLOAT  , V16_FLOAT  , 4),
   _(R32_FLOAT           , V32_FLOAT  , 1),
   _(R32G32_FLOAT        , V32_FLOAT  , 2),
   _(R32G32B32_FLOAT     , V32_FLOAT  , 3),
   _(R32G32B32A32_FLOAT  , V32_FLOAT  , 4),
};
#undef _

#define SWZ_OUT_0 0
#define SWZ_OUT_1 1
#define SWZ_OUT_C 2

#define SWZ_SRC_0 3
#define SWZ_SRC_1 2
#define SWZ_SRC_2 1
#define SWZ_SRC_3 0
#define SWZ_SRC_x 0

#define NONE 0x00000000
#define SRGB 0x00700000

#define ____ 0x00000000
#define SSSS 0xf0000000

#define _(a,b,c,d,e,f,g,h,i,j,k,l,m) [PIPE_FORMAT_##a] = {                     \
   .nv30 = NV30_3D_TEX_FORMAT_FORMAT_##b,                                      \
   .nv30_rect = NV30_3D_TEX_FORMAT_FORMAT_##b##_RECT,                          \
   .nv40 = NV40_3D_TEX_FORMAT_FORMAT_##b,                                      \
   .swz[0] = { SWZ_OUT_##d, SWZ_SRC_##h },                                     \
   .swz[1] = { SWZ_OUT_##e, SWZ_SRC_##i },                                     \
   .swz[2] = { SWZ_OUT_##f, SWZ_SRC_##j },                                     \
   .swz[3] = { SWZ_OUT_##g, SWZ_SRC_##k },                                     \
   .swz[4] = { SWZ_OUT_0, SWZ_SRC_x },                                         \
   .swz[5] = { SWZ_OUT_1, SWZ_SRC_x },                                         \
   .swizzle = (c) * 0x00010000,                                                \
   .wrap =  (l),                                                               \
   .filter = (m),                                                              \
}
const struct nv30_texfmt
nv30_texfmt_table[PIPE_FORMAT_COUNT] = {
   _(L8_UNORM          , L8      , 0, C, C, C, 1, 0, 0, 0, x, NONE, ____),
   _(L8_SNORM          , L8      , 0, C, C, C, 1, 0, 0, 0, x, NONE, SSSS),
   _(L8_SRGB           , L8      , 0, C, C, C, 1, 0, 0, 0, x, SRGB, ____),
   _(I8_UNORM          , L8      , 0, C, C, C, C, 0, 0, 0, 0, NONE, ____),
   _(I8_SNORM          , L8      , 0, C, C, C, C, 0, 0, 0, 0, NONE, SSSS),
   _(A8_UNORM          , L8      , 0, 0, 0, 0, C, x, x, x, 0, NONE, ____),
   _(A8_SNORM          , L8      , 0, 0, 0, 0, C, x, x, x, 0, NONE, SSSS),
   _(R8_UNORM          , L8      , 0, C, 0, 0, 1, 0, x, x, x, NONE, ____),
   _(R8_SNORM          , L8      , 0, C, 0, 0, 1, 0, x, x, x, NONE, SSSS),
   _(B5G5R5X1_UNORM    , A1R5G5B5, 0, C, C, C, 1, 2, 1, 0, x, NONE, ____),
   _(B5G5R5A1_UNORM    , A1R5G5B5, 0, C, C, C, C, 2, 1, 0, 3, NONE, ____),
   _(B4G4R4X4_UNORM    , A4R4G4B4, 0, C, C, C, 1, 2, 1, 0, x, NONE, ____),
   _(B4G4R4A4_UNORM    , A4R4G4B4, 0, C, C, C, C, 2, 1, 0, 3, NONE, ____),
   _(B5G6R5_UNORM      , R5G6B5  , 0, C, C, C, 1, 2, 1, 0, x, NONE, ____),
   _(BGRX8888_UNORM    , A8R8G8B8, 0, C, C, C, 1, 2, 1, 0, x, NONE, ____),
   _(BGRX8888_SRGB     , A8R8G8B8, 0, C, C, C, 1, 2, 1, 0, x, SRGB, ____),
   _(BGRA8888_UNORM    , A8R8G8B8, 0, C, C, C, C, 2, 1, 0, 3, NONE, ____),
   _(BGRA8888_SRGB     , A8R8G8B8, 0, C, C, C, C, 2, 1, 0, 3, SRGB, ____),
   _(RGBA8888_SNORM    , A8R8G8B8, 0, C, C, C, C, 0, 1, 2, 3, NONE, SSSS),
   _(DXT1_RGB          , DXT1    , 0, C, C, C, 1, 2, 1, 0, x, NONE, ____),
   _(DXT1_SRGB         , DXT1    , 0, C, C, C, 1, 2, 1, 0, x, SRGB, ____),
   _(DXT1_RGBA         , DXT1    , 0, C, C, C, C, 2, 1, 0, 3, NONE, ____),
   _(DXT1_SRGBA        , DXT1    , 0, C, C, C, C, 2, 1, 0, 3, SRGB, ____),
   _(DXT3_RGBA         , DXT3    , 0, C, C, C, C, 2, 1, 0, 3, NONE, ____),
   _(DXT3_SRGBA        , DXT3    , 0, C, C, C, C, 2, 1, 0, 3, SRGB, ____),
   _(DXT5_RGBA         , DXT5    , 0, C, C, C, C, 2, 1, 0, 3, NONE, ____),
   _(DXT5_SRGBA        , DXT5    , 0, C, C, C, C, 2, 1, 0, 3, SRGB, ____),
   _(L8A8_UNORM        , A8L8    , 0, C, C, C, C, 0, 0, 0, 3, NONE, ____),
   _(L8A8_SRGB         , A8L8    , 0, C, C, C, C, 0, 0, 0, 3, SRGB, ____),
   _(R8G8_UNORM        , A8L8    , 0, C, C, 0, 1, 0, 3, x, x, NONE, ____),
   _(R8G8_SNORM        , A8L8    , 0, C, C, 0, 1, 0, 3, x, x, NONE, SSSS),
   _(Z16_UNORM         , Z16     , 0, C, C, C, 1, 3, 3, 3, x, NONE, ____),
   _(X8Z24_UNORM       , Z24     , 0, C, C, C, 1, 3, 3, 3, x, NONE, ____),
   _(S8_UINT_Z24_UNORM , Z24     , 0, C, C, C, 1, 3, 3, 3, x, NONE, ____),
   _(L16_UNORM         , A16     , 0, C, C, C, 1, 1, 1, 1, 1, NONE, ____),
   _(L16_SNORM         , A16     , 0, C, C, C, 1, 1, 1, 1, 1, NONE, SSSS),
   _(I16_UNORM         , A16     , 0, C, C, C, C, 1, 1, 1, 1, NONE, ____),
   _(I16_SNORM         , A16     , 0, C, C, C, C, 1, 1, 1, 1, NONE, SSSS),
   _(A16_UNORM         , A16     , 0, 0, 0, 0, C, 1, 1, 1, 1, NONE, ____),
   _(A16_SNORM         , A16     , 0, 0, 0, 0, C, 1, 1, 1, 1, NONE, SSSS),
   _(R16_UNORM         , A16     , 0, C, 0, 0, 1, 1, 1, 1, 1, NONE, ____),
   _(R16_SNORM         , A16     , 0, C, 0, 0, 1, 1, 1, 1, 1, NONE, SSSS),
   _(R16G16_FLOAT      , RG16F   , 0, C, C, 0, 1, 2, 1, 0, 3, NONE, ____),
   _(R16G16B16A16_FLOAT, RGBA16F , 0, C, C, C, C, 2, 1, 0, 3, NONE, ____),
   _(R32_FLOAT         , R32F    , 0, C, 0, 0, 1, 2, 1, 0, 3, NONE, ____),
   _(R32G32B32A32_FLOAT, RGBA32F , 0, C, C, C, C, 2, 1, 0, 3, NONE, ____),
};
#undef _
