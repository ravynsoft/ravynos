/*
 * Copyright 2015 Intel Corporation
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice (including the next
 *  paragraph) shall be included in all copies or substantial portions of the
 *  Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

#include <assert.h>

#include "isl.h"
#include "isl_priv.h"
#include "dev/intel_device_info.h"

#include "util/macros.h" /* Needed for MAX3 and MAX2 for format_rgb9e5 */

#include "util/format/format_utils.h"
#include "util/format_r11g11b10f.h"
#include "util/format_rgb9e5.h"
#include "util/format_srgb.h"
#include "util/half_float.h"
#include "util/rounding.h"
#include "util/u_math.h"

struct surface_format_info {
   bool exists;
   /* These fields must fit the largest verx10 value. */
   uint16_t sampling;
   uint16_t filtering;
   uint16_t shadow_compare;
   uint16_t chroma_key;
   uint16_t render_target;
   uint16_t alpha_blend;
   uint16_t input_vb;
   uint16_t streamed_output_vb;
   uint16_t color_processing;
   uint16_t typed_write;
   uint16_t typed_read;
   uint16_t typed_atomics;
   uint16_t ccs_e;
};

/* This macro allows us to write the table almost as it appears in the PRM,
 * while restructuring it to turn it into the C code we want.
 */
#define SF(sampl, filt, shad, ck, rt, ab, vb, so, color, tw, tr, ccs_e, ta, sf) \
   [ISL_FORMAT_##sf] = { true, sampl, filt, shad, ck, rt, ab, vb, so, color, tw, tr, ta, ccs_e},

#define Y 0
#define x 0xFFFF
/**
 * This is the table of support for surface (texture, renderbuffer, and vertex
 * buffer, but not depthbuffer) formats across the various hardware generations.
 *
 * The table is formatted to match the documentation, except that the docs have
 * this ridiculous mapping of Y[*+~^#&] for "supported on DevWhatever".  To put
 * it in our table, here's the mapping:
 *
 * Y*: 45
 * Y+: 45 (g45/gm45)
 * Y~: 50 (gfx5)
 * Y^: 60 (gfx6)
 * Y#: 70 (gfx7)
 *
 * The abbreviations in the header below are:
 * smpl  - Sampling Engine
 * filt  - Sampling Engine Filtering
 * shad  - Sampling Engine Shadow Map
 * CK    - Sampling Engine Chroma Key
 * RT    - Render Target
 * AB    - Alpha Blend Render Target
 * VB    - Input Vertex Buffer
 * SO    - Steamed Output Vertex Buffers (transform feedback)
 * color - Color Processing
 * TW    - Typed Write
 * TR    - Typed Read
 * ccs_e - Lossless Compression Support (gfx9+ only)
 * sf    - Surface Format
 * TA    - Typed Atomics
 *
 * See page 88 of the Sandybridge PRM VOL4_Part1 PDF.
 *
 * As of Ivybridge, the columns are no longer in that table and the
 * information can be found spread across:
 *
 * - VOL2_Part1 section 2.5.11 Format Conversion (vertex fetch).
 * - VOL4_Part1 section 2.12.2.1.2 Sampler Output Channel Mapping.
 * - VOL4_Part1 section 3.9.11 Render Target Write.
 * - Render Target Surface Types [SKL+]
 */
static const struct surface_format_info format_info[] = {
/*    smpl filt  shad  CK   RT   AB   VB   SO color TW   TR  ccs_e  TA */
   SF(  Y,  50,   x,   x,   Y,   Y,   Y,   Y,   x,  70,  90,  90,    x,  R32G32B32A32_FLOAT)
   SF(  Y,   x,   x,   x,   Y,   x,   Y,   Y,   x,  70,  90,  90,    x,  R32G32B32A32_SINT)
   SF(  Y,   x,   x,   x,   Y,   x,   Y,   Y,   x,  70,  90,  90,    x,  R32G32B32A32_UINT)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R32G32B32A32_UNORM)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R32G32B32A32_SNORM)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R64G64_FLOAT)
   SF(  Y,  50,   x,   x, 110, 110,   x,   x,   x,   x,   x, 110,    x,  R32G32B32X32_FLOAT)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R32G32B32A32_SSCALED)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R32G32B32A32_USCALED)
   SF(  x,   x,   x,   x,   x,   x,  75,   x,   x,   x,   x,   x,    x,  R32G32B32A32_SFIXED)
   SF(  x,   x,   x,   x,   x,   x,  80,   x,   x,   x,   x,   x,    x,  R64G64_PASSTHRU)
   SF(  Y,  50,   x,   x,   x,   x,   Y,   Y,   x,   x,   x,   x,    x,  R32G32B32_FLOAT)
   SF(  Y,   x,   x,   x,   x,   x,   Y,   Y,   x,   x,   x,   x,    x,  R32G32B32_SINT)
   SF(  Y,   x,   x,   x,   x,   x,   Y,   Y,   x,   x,   x,   x,    x,  R32G32B32_UINT)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R32G32B32_UNORM)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R32G32B32_SNORM)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R32G32B32_SSCALED)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R32G32B32_USCALED)
   SF(  x,   x,   x,   x,   x,   x,  75,   x,   x,   x,   x,   x,    x,  R32G32B32_SFIXED)
   SF(  Y,   Y,   x,   x,   Y,  45,   Y,   x,  60,  70, 110,  90,    x,  R16G16B16A16_UNORM)
   SF(  Y,   Y,   x,   x,   Y,  60,   Y,   x,   x,  70, 110,  90,    x,  R16G16B16A16_SNORM)
   SF(  Y,   x,   x,   x,   Y,   x,   Y,   x,   x,  70,  90,  90,    x,  R16G16B16A16_SINT)
   SF(  Y,   x,   x,   x,   Y,   x,   Y,   x,   x,  70,  75,  90,    x,  R16G16B16A16_UINT)
   SF(  Y,   Y,   x,   x,   Y,   Y,   Y,   x,   x,  70,  90,  90,    x,  R16G16B16A16_FLOAT)
   SF(  Y,  50,   x,   x,   Y,   Y,   Y,   Y,   x,  70,  90,  90,    x,  R32G32_FLOAT)
   SF(  Y,  70,   x,   x,   Y,   Y,   Y,   Y,   x,   x,   x,   x,    x,  R32G32_FLOAT_LD)
   SF(  Y,   x,   x,   x,   Y,   x,   Y,   Y,   x,  70,  90,  90,    x,  R32G32_SINT)
   SF(  Y,   x,   x,   x,   Y,   x,   Y,   Y,   x,  70,  90,  90,    x,  R32G32_UINT)
   SF(  Y,  50,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  R32_FLOAT_X8X24_TYPELESS)
   SF(  Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  X32_TYPELESS_G8X24_UINT)
   SF(  Y,  50,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  L32A32_FLOAT)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R32G32_UNORM)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R32G32_SNORM)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,   90,  R64_FLOAT)
   SF(  Y,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  R16G16B16X16_UNORM)
   SF(  Y,   Y,   x,   x,  90,  90,   x,   x,   x,   x,   x,  90,    x,  R16G16B16X16_FLOAT)
   SF(  Y,  50,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  A32X32_FLOAT)
   SF(  Y,  50,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  L32X32_FLOAT)
   SF(  Y,  50,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  I32X32_FLOAT)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R16G16B16A16_SSCALED)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R16G16B16A16_USCALED)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R32G32_SSCALED)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R32G32_USCALED)
   SF(  x,   x,   x,   x,   x,   x,  75,   x,   x,   x,   x,   x,    x,  R32G32_SFIXED)
   SF(  x,   x,   x,   x,   x,   x,  80,   x,   x,   x,   x,   x,   90,  R64_PASSTHRU)
   SF(  Y,   Y,   x,   Y,   Y,   Y,   Y,   x,  60,  70, 125,  90,    x,  B8G8R8A8_UNORM)
   SF(  Y,   Y,   x,   x,   Y,   Y,   x,   x,   x,   x,   x, 110,    x,  B8G8R8A8_UNORM_SRGB)
/*    smpl filt  shad  CK   RT   AB   VB   SO color TW   TR  ccs_e  TA */
   SF(  Y,   Y,   x,   x,   Y,   Y,   Y,   x,  60,  70, 125, 110,    x,  R10G10B10A2_UNORM)
   SF(  Y,   Y,   x,   x,   x,   x,   x,   x,  60,   x,   x, 120,    x,  R10G10B10A2_UNORM_SRGB)
   SF(  Y,   x,   x,   x,   Y,   x,   Y,   x,   x,  70, 125, 110,    x,  R10G10B10A2_UINT)
   SF(  Y,   Y,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R10G10B10_SNORM_A2_UNORM)
   SF(  Y,   Y,   x,   x,   Y,   Y,   Y,   x,  60,  70, 110,  90,    x,  R8G8B8A8_UNORM)
   SF(  Y,   Y,   x,   x,   Y,   Y,   x,   x,  60,   x,   x, 110,    x,  R8G8B8A8_UNORM_SRGB)
   SF(  Y,   Y,   x,   x,   Y,  60,   Y,   x,   x,  70, 110,  90,    x,  R8G8B8A8_SNORM)
   SF(  Y,   x,   x,   x,   Y,   x,   Y,   x,   x,  70,  90,  90,    x,  R8G8B8A8_SINT)
   SF(  Y,   x,   x,   x,   Y,   x,   Y,   x,   x,  70,  75,  90,    x,  R8G8B8A8_UINT)
   SF(  Y,   Y,   x,   x,   Y,  45,   Y,   x,   x,  70, 110,  90,    x,  R16G16_UNORM)
   SF(  Y,   Y,   x,   x,   Y,  60,   Y,   x,   x,  70, 110,  90,    x,  R16G16_SNORM)
   SF(  Y,   x,   x,   x,   Y,   x,   Y,   x,   x,  70,  90,  90,    x,  R16G16_SINT)
   SF(  Y,   x,   x,   x,   Y,   x,   Y,   x,   x,  70,  75,  90,    x,  R16G16_UINT)
   SF(  Y,   Y,   x,   x,   Y,   Y,   Y,   x,   x,  70,  90,  90,    x,  R16G16_FLOAT)
   SF(  Y,   Y,   x,   x,   Y,   Y,  75,   x,  60,  70, 125, 110,    x,  B10G10R10A2_UNORM)
   SF(  Y,   Y,   x,   x,   Y,   Y,   x,   x,  60,   x,   x, 110,    x,  B10G10R10A2_UNORM_SRGB)
   SF(  Y,   Y,   x,   x,   Y,   Y,   Y,   x,   x,  70, 125, 110,    x,  R11G11B10_FLOAT)
   SF(120, 120,   x,   x, 120, 120,   x,   x,   x, 125, 125, 120,    x,  R10G10B10_FLOAT_A2_UNORM)
   SF(  Y,   x,   x,   x,   Y,   x,   Y,   Y,   x,  70,  70,  90,   70,  R32_SINT)
   SF(  Y,   x,   x,   x,   Y,   x,   Y,   Y,   x,  70,  70,  90,   70,  R32_UINT)
   SF(  Y,  50,   Y,   x,   Y,   Y,   Y,   Y,   x,  70,  70,  90,  110,  R32_FLOAT)
   SF(  Y,  50,   Y,   x,   x,   x,   x,   x,   x,   x,   x, 120,    x,  R24_UNORM_X8_TYPELESS)
   SF(  Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  X24_TYPELESS_G8_UINT)
   SF(  Y,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  L16A16_UNORM)
   SF(  Y,  50,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  I24X8_UNORM)
   SF(  Y,  50,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  L24X8_UNORM)
   SF(  Y,  50,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  A24X8_UNORM)
   SF(  Y,  50,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  I32_FLOAT)
   SF(  Y,  50,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  L32_FLOAT)
   SF(  Y,  50,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  A32_FLOAT)
   SF(  Y,   Y,   x,   Y,  80,  80,   x,   x,  60,   x,   x,  90,    x,  B8G8R8X8_UNORM)
   SF(  Y,   Y,   x,   x,  80,  80,   x,   x,   x,   x,   x, 110,    x,  B8G8R8X8_UNORM_SRGB)
   SF(  Y,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  R8G8B8X8_UNORM)
   SF(  Y,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  R8G8B8X8_UNORM_SRGB)
   SF(  Y,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  R9G9B9E5_SHAREDEXP)
   SF(  Y,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  B10G10R10X2_UNORM)
   SF(  Y,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  L16A16_FLOAT)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R32_UNORM)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R32_SNORM)
/*    smpl filt  shad  CK   RT   AB   VB   SO color TW   TR  ccs_e  TA */
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R10G10B10X2_USCALED)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R8G8B8A8_SSCALED)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R8G8B8A8_USCALED)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R16G16_SSCALED)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R16G16_USCALED)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R32_SSCALED)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R32_USCALED)
   SF(  Y,   Y,   x,   Y,   Y,   Y,   x,   x,   x,  70,   x, 120,    x,  B5G6R5_UNORM)
   SF(  Y,   Y,   x,   x,   Y,   Y,   x,   x,   x,   x,   x, 120,    x,  B5G6R5_UNORM_SRGB)
   SF(  Y,   Y,   x,   Y,   Y,   Y,   x,   x,   x,  70,   x, 120,    x,  B5G5R5A1_UNORM)
   SF(  Y,   Y,   x,   x,   Y,   Y,   x,   x,   x,   x,   x, 120,    x,  B5G5R5A1_UNORM_SRGB)
   SF(  Y,   Y,   x,   Y,   Y,   Y,   x,   x,   x,  70,   x, 120,    x,  B4G4R4A4_UNORM)
   SF(  Y,   Y,   x,   x,   Y,   Y,   x,   x,   x,   x,   x, 120,    x,  B4G4R4A4_UNORM_SRGB)
   SF(  Y,   Y,   x,   x,   Y,   Y,   Y,   x,   x,  70, 110, 120,    x,  R8G8_UNORM)
   SF(  Y,   Y,   x,   Y,   Y,  60,   Y,   x,   x,  70, 110, 120,    x,  R8G8_SNORM)
   SF(  Y,   x,   x,   x,   Y,   x,   Y,   x,   x,  70,  90, 120,    x,  R8G8_SINT)
   SF(  Y,   x,   x,   x,   Y,   x,   Y,   x,   x,  70,  75, 120,    x,  R8G8_UINT)
   SF(  Y,   Y,   Y,   x,   Y,  45,   Y,   x,  70,  70, 110, 120,    x,  R16_UNORM)
   SF(  Y,   Y,   x,   x,   Y,  60,   Y,   x,   x,  70, 110, 120,    x,  R16_SNORM)
   SF(  Y,   x,   x,   x,   Y,   x,   Y,   x,   x,  70,  90, 120,  120,  R16_SINT)
   SF(  Y,   x,   x,   x,   Y,   x,   Y,   x,   x,  70,  75, 120,  120,  R16_UINT)
   SF(  Y,   Y,   x,   x,   Y,   Y,   Y,   x,   x,  70,  90, 120,  120,  R16_FLOAT)
   SF( 50,  50,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  A8P8_UNORM_PALETTE0)
   SF( 50,  50,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  A8P8_UNORM_PALETTE1)
   SF(  Y,   Y,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  I16_UNORM)
   SF(  Y,   Y,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  L16_UNORM)
   SF(  Y,   Y,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  A16_UNORM)
   SF(  Y,   Y,   x,   Y,   x,   x,   x,   x,   x,   x,   x,   x,    x,  L8A8_UNORM)
   SF(  Y,   Y,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  I16_FLOAT)
   SF(  Y,   Y,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  L16_FLOAT)
   SF(  Y,   Y,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  A16_FLOAT)
   SF( 45,  45,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  L8A8_UNORM_SRGB)
   SF(  Y,   Y,   x,   Y,   x,   x,   x,   x,   x,   x,   x,   x,    x,  R5G5_SNORM_B6_UNORM)
   SF(  x,   x,   x,   x,   Y,   Y,   x,   x,   x,  70,   x, 120,    x,  B5G5R5X1_UNORM)
   SF(  x,   x,   x,   x,   Y,   Y,   x,   x,   x,   x,   x, 120,    x,  B5G5R5X1_UNORM_SRGB)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R8G8_SSCALED)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R8G8_USCALED)
/*    smpl filt  shad  CK   RT   AB   VB   SO color TW   TR  ccs_e  TA */
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R16_SSCALED)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R16_USCALED)
   SF( 50,  50,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  P8A8_UNORM_PALETTE0)
   SF( 50,  50,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  P8A8_UNORM_PALETTE1)
   SF(120, 120,   x,   x, 120, 120,   x,   x,   x,   x,   x, 120,    x,  A1B5G5R5_UNORM)
   /* According to the PRM, A4B4G4R4_UNORM isn't supported until Sky Lake
    * but empirical testing indicates that at least sampling works just fine
    * on Broadwell.
    */
   SF( 80,  80,   x,   x,  90, 120,   x,   x,   x,   x,   x, 120,    x,  A4B4G4R4_UNORM)
   SF( 90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  L8A8_UINT)
   SF( 90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  L8A8_SINT)
   SF(  Y,   Y,   x,  45,   Y,   Y,   Y,   x,   x,  70, 110, 120,    x,  R8_UNORM)
   SF(  Y,   Y,   x,   x,   Y,  60,   Y,   x,   x,  70, 110, 120,    x,  R8_SNORM)
   SF(  Y,   x,   x,   x,   Y,   x,   Y,   x,   x,  70,  90, 120,    x,  R8_SINT)
   SF(  Y,   x,   x,   x,   Y,   x,   Y,   x,   x,  70,  75, 120,    x,  R8_UINT)
   SF(  Y,   Y,   x,   Y,   Y,   Y,   x,   x,   x,  70, 110, 120,    x,  A8_UNORM)
   SF(  Y,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  I8_UNORM)
   SF(  Y,   Y,   x,   Y,   x,   x,   x,   x,   x,   x,   x,   x,    x,  L8_UNORM)
   SF(  Y,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  P4A4_UNORM_PALETTE0)
   SF(  Y,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  A4P4_UNORM_PALETTE0)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R8_SSCALED)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R8_USCALED)
   SF( 45,  45,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  P8_UNORM_PALETTE0)
   SF( 45,  45,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  L8_UNORM_SRGB)
   SF( 45,  45,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  P8_UNORM_PALETTE1)
   SF( 45,  45,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  P4A4_UNORM_PALETTE1)
   SF( 45,  45,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  A4P4_UNORM_PALETTE1)
   SF(  x,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  Y8_UNORM)
   SF( 90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  L8_UINT)
   SF( 90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  L8_SINT)
   SF( 90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  I8_UINT)
   SF( 90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  I8_SINT)
   SF( 45,  45,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  DXT1_RGB_SRGB)
   SF(  Y,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  R1_UNORM)
   SF(  Y,   Y,   x,   Y,   Y,   x,   x,   x,  60,   x,   x, 120,    x,  YCRCB_NORMAL)
   SF(  Y,   Y,   x,   Y,   Y,   x,   x,   x,  60,   x,   x,   x,    x,  YCRCB_SWAPUVY)
   SF( 45,  45,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  P2_UNORM_PALETTE0)
   SF( 45,  45,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  P2_UNORM_PALETTE1)
   SF(  Y,   Y,   x,   Y,   x,   x,   x,   x,   x,   x,   x,   x,    x,  BC1_UNORM)
   SF(  Y,   Y,   x,   Y,   x,   x,   x,   x,   x,   x,   x,   x,    x,  BC2_UNORM)
   SF(  Y,   Y,   x,   Y,   x,   x,   x,   x,   x,   x,   x,   x,    x,  BC3_UNORM)
   SF(  Y,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  BC4_UNORM)
   SF(  Y,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  BC5_UNORM)
   SF(  Y,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  BC1_UNORM_SRGB)
   SF(  Y,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  BC2_UNORM_SRGB)
   SF(  Y,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  BC3_UNORM_SRGB)
   SF(  Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  MONO8)
   SF(  Y,   Y,   x,   x,   Y,   x,   x,   x,  60,   x,   x,   x,    x,  YCRCB_SWAPUV)
   SF(  Y,   Y,   x,   x,   Y,   x,   x,   x,  60,   x,   x, 120,    x,  YCRCB_SWAPY)
   SF(  Y,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  DXT1_RGB)
/*    smpl filt  shad  CK   RT   AB   VB   SO color TW   TR  ccs_e  TA */
   SF(  Y,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  FXT1)
   SF( 75,  75,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R8G8B8_UNORM)
   SF( 75,  75,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R8G8B8_SNORM)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R8G8B8_SSCALED)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R8G8B8_USCALED)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R64G64B64A64_FLOAT)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R64G64B64_FLOAT)
   SF(  Y,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  BC4_SNORM)
   SF(  Y,   Y,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  BC5_SNORM)
   SF( 50,  50,   x,   x,   x,   x,  60,   x,   x,   x,   x,   x,    x,  R16G16B16_FLOAT)
   SF( 75,  75,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R16G16B16_UNORM)
   SF( 75,  75,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R16G16B16_SNORM)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R16G16B16_SSCALED)
   SF(  x,   x,   x,   x,   x,   x,   Y,   x,   x,   x,   x,   x,    x,  R16G16B16_USCALED)
   SF( 70,  70,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  BC6H_SF16)
   SF( 70,  70,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  BC7_UNORM)
   SF( 70,  70,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  BC7_UNORM_SRGB)
   SF( 70,  70,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  BC6H_UF16)
   SF(  x,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  PLANAR_420_8)
   /* The format enum for R8G8B8_UNORM_SRGB first shows up in the HSW PRM but
    * empirical testing indicates that it doesn't actually sRGB decode and
    * acts identical to R8G8B8_UNORM.  It does work on gfx8+.
    */
   SF( 80,  80,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  R8G8B8_UNORM_SRGB)
   SF( 80,  80,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ETC1_RGB8)
   SF( 80,  80,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ETC2_RGB8)
   SF( 80,  80,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  EAC_R11)
   SF( 80,  80,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  EAC_RG11)
   SF( 80,  80,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  EAC_SIGNED_R11)
   SF( 80,  80,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  EAC_SIGNED_RG11)
   SF( 80,  80,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ETC2_SRGB8)
   SF( 90,   x,   x,   x,   x,   x,  75,   x,   x,   x,   x,   x,    x,  R16G16B16_UINT)
   SF( 90,   x,   x,   x,   x,   x,  75,   x,   x,   x,   x,   x,    x,  R16G16B16_SINT)
   SF(  x,   x,   x,   x,   x,   x,  75,   x,   x,   x,   x,   x,    x,  R32_SFIXED)
   SF(  x,   x,   x,   x,   x,   x,  75,   x,   x,   x,   x,   x,    x,  R10G10B10A2_SNORM)
   SF(  x,   x,   x,   x,   x,   x,  75,   x,   x,   x,   x,   x,    x,  R10G10B10A2_USCALED)
   SF(  x,   x,   x,   x,   x,   x,  75,   x,   x,   x,   x,   x,    x,  R10G10B10A2_SSCALED)
   SF(  x,   x,   x,   x,   x,   x,  75,   x,   x,   x,   x,   x,    x,  R10G10B10A2_SINT)
   SF(  x,   x,   x,   x,   x,   x,  75,   x,   x,   x,   x,   x,    x,  B10G10R10A2_SNORM)
   SF(  x,   x,   x,   x,   x,   x,  75,   x,   x,   x,   x,   x,    x,  B10G10R10A2_USCALED)
   SF(  x,   x,   x,   x,   x,   x,  75,   x,   x,   x,   x,   x,    x,  B10G10R10A2_SSCALED)
   SF(  x,   x,   x,   x,   x,   x,  75,   x,   x,   x,   x,   x,    x,  B10G10R10A2_UINT)
   SF(  x,   x,   x,   x,   x,   x,  75,   x,   x,   x,   x,   x,    x,  B10G10R10A2_SINT)
   SF(  x,   x,   x,   x,   x,   x,  80,   x,   x,   x,   x,   x,    x,  R64G64B64A64_PASSTHRU)
   SF(  x,   x,   x,   x,   x,   x,  80,   x,   x,   x,   x,   x,    x,  R64G64B64_PASSTHRU)
   SF( 80,  80,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ETC2_RGB8_PTA)
   SF( 80,  80,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ETC2_SRGB8_PTA)
   SF( 80,  80,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ETC2_EAC_RGBA8)
   SF( 80,  80,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ETC2_EAC_SRGB8_A8)
   SF( 90,   x,   x,   x,   x,   x,  75,   x,   x,   x,   x,   x,    x,  R8G8B8_UINT)
   SF( 90,   x,   x,   x,   x,   x,  75,   x,   x,   x,   x,   x,    x,  R8G8B8_SINT)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_4X4_FLT16)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_5X4_FLT16)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_5X5_FLT16)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_6X5_FLT16)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_6X6_FLT16)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_8X5_FLT16)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_8X6_FLT16)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_8X8_FLT16)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_10X5_FLT16)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_10X6_FLT16)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_10X8_FLT16)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_10X10_FLT16)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_12X10_FLT16)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_12X12_FLT16)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_4X4_U8SRGB)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_5X4_U8SRGB)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_5X5_U8SRGB)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_6X5_U8SRGB)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_6X6_U8SRGB)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_8X5_U8SRGB)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_8X6_U8SRGB)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_8X8_U8SRGB)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_10X5_U8SRGB)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_10X6_U8SRGB)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_10X8_U8SRGB)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_10X10_U8SRGB)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_12X10_U8SRGB)
   SF( 90,  90,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_LDR_2D_12X12_U8SRGB)
   SF(110, 110,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_HDR_2D_4X4_FLT16)
   SF(110, 110,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_HDR_2D_5X4_FLT16)
   SF(110, 110,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_HDR_2D_5X5_FLT16)
   SF(110, 110,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_HDR_2D_6X5_FLT16)
   SF(110, 110,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_HDR_2D_6X6_FLT16)
   SF(110, 110,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_HDR_2D_8X5_FLT16)
   SF(110, 110,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_HDR_2D_8X6_FLT16)
   SF(110, 110,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_HDR_2D_8X8_FLT16)
   SF(110, 110,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_HDR_2D_10X5_FLT16)
   SF(110, 110,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_HDR_2D_10X6_FLT16)
   SF(110, 110,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_HDR_2D_10X8_FLT16)
   SF(110, 110,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_HDR_2D_10X10_FLT16)
   SF(110, 110,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_HDR_2D_12X10_FLT16)
   SF(110, 110,   x,   x,   x,   x,   x,   x,   x,   x,   x,   x,    x,  ASTC_HDR_2D_12X12_FLT16)
};
#undef x
#undef Y


enum isl_format
isl_format_for_pipe_format(enum pipe_format pf)
{
   static const enum isl_format table[PIPE_FORMAT_COUNT] = {
      [0 ... PIPE_FORMAT_COUNT-1] = ISL_FORMAT_UNSUPPORTED,

      [PIPE_FORMAT_B8G8R8A8_UNORM]          = ISL_FORMAT_B8G8R8A8_UNORM,
      [PIPE_FORMAT_B8G8R8X8_UNORM]          = ISL_FORMAT_B8G8R8X8_UNORM,
      [PIPE_FORMAT_B5G5R5A1_UNORM]          = ISL_FORMAT_B5G5R5A1_UNORM,
      [PIPE_FORMAT_B4G4R4A4_UNORM]          = ISL_FORMAT_B4G4R4A4_UNORM,
      [PIPE_FORMAT_B5G6R5_UNORM]            = ISL_FORMAT_B5G6R5_UNORM,
      [PIPE_FORMAT_R10G10B10A2_UNORM]       = ISL_FORMAT_R10G10B10A2_UNORM,

      [PIPE_FORMAT_Z16_UNORM]               = ISL_FORMAT_R16_UNORM,
      [PIPE_FORMAT_Z32_UNORM]               = ISL_FORMAT_R32_UNORM,
      [PIPE_FORMAT_Z32_FLOAT]               = ISL_FORMAT_R32_FLOAT,

      /* We translate the combined depth/stencil formats to depth only here */
      [PIPE_FORMAT_Z24_UNORM_S8_UINT]       = ISL_FORMAT_R24_UNORM_X8_TYPELESS,
      [PIPE_FORMAT_Z24X8_UNORM]             = ISL_FORMAT_R24_UNORM_X8_TYPELESS,
      [PIPE_FORMAT_Z32_FLOAT_S8X24_UINT]    = ISL_FORMAT_R32_FLOAT,

      [PIPE_FORMAT_S8_UINT]                 = ISL_FORMAT_R8_UINT,
      [PIPE_FORMAT_X24S8_UINT]              = ISL_FORMAT_R8_UINT,
      [PIPE_FORMAT_X32_S8X24_UINT]          = ISL_FORMAT_R8_UINT,

      [PIPE_FORMAT_R64_FLOAT]               = ISL_FORMAT_R64_FLOAT,
      [PIPE_FORMAT_R64G64_FLOAT]            = ISL_FORMAT_R64G64_FLOAT,
      [PIPE_FORMAT_R64G64B64_FLOAT]         = ISL_FORMAT_R64G64B64_FLOAT,
      [PIPE_FORMAT_R64G64B64A64_FLOAT]      = ISL_FORMAT_R64G64B64A64_FLOAT,
      [PIPE_FORMAT_R32_FLOAT]               = ISL_FORMAT_R32_FLOAT,
      [PIPE_FORMAT_R32G32_FLOAT]            = ISL_FORMAT_R32G32_FLOAT,
      [PIPE_FORMAT_R32G32B32_FLOAT]         = ISL_FORMAT_R32G32B32_FLOAT,
      [PIPE_FORMAT_R32G32B32A32_FLOAT]      = ISL_FORMAT_R32G32B32A32_FLOAT,
      [PIPE_FORMAT_R32_UNORM]               = ISL_FORMAT_R32_UNORM,
      [PIPE_FORMAT_R32G32_UNORM]            = ISL_FORMAT_R32G32_UNORM,
      [PIPE_FORMAT_R32G32B32_UNORM]         = ISL_FORMAT_R32G32B32_UNORM,
      [PIPE_FORMAT_R32G32B32A32_UNORM]      = ISL_FORMAT_R32G32B32A32_UNORM,
      [PIPE_FORMAT_R32_USCALED]             = ISL_FORMAT_R32_USCALED,
      [PIPE_FORMAT_R32G32_USCALED]          = ISL_FORMAT_R32G32_USCALED,
      [PIPE_FORMAT_R32G32B32_USCALED]       = ISL_FORMAT_R32G32B32_USCALED,
      [PIPE_FORMAT_R32G32B32A32_USCALED]    = ISL_FORMAT_R32G32B32A32_USCALED,
      [PIPE_FORMAT_R32_SNORM]               = ISL_FORMAT_R32_SNORM,
      [PIPE_FORMAT_R32G32_SNORM]            = ISL_FORMAT_R32G32_SNORM,
      [PIPE_FORMAT_R32G32B32_SNORM]         = ISL_FORMAT_R32G32B32_SNORM,
      [PIPE_FORMAT_R32G32B32A32_SNORM]      = ISL_FORMAT_R32G32B32A32_SNORM,
      [PIPE_FORMAT_R32_SSCALED]             = ISL_FORMAT_R32_SSCALED,
      [PIPE_FORMAT_R32G32_SSCALED]          = ISL_FORMAT_R32G32_SSCALED,
      [PIPE_FORMAT_R32G32B32_SSCALED]       = ISL_FORMAT_R32G32B32_SSCALED,
      [PIPE_FORMAT_R32G32B32A32_SSCALED]    = ISL_FORMAT_R32G32B32A32_SSCALED,
      [PIPE_FORMAT_R16_UNORM]               = ISL_FORMAT_R16_UNORM,
      [PIPE_FORMAT_R16G16_UNORM]            = ISL_FORMAT_R16G16_UNORM,
      [PIPE_FORMAT_R16G16B16_UNORM]         = ISL_FORMAT_R16G16B16_UNORM,
      [PIPE_FORMAT_R16G16B16A16_UNORM]      = ISL_FORMAT_R16G16B16A16_UNORM,
      [PIPE_FORMAT_R16_USCALED]             = ISL_FORMAT_R16_USCALED,
      [PIPE_FORMAT_R16G16_USCALED]          = ISL_FORMAT_R16G16_USCALED,
      [PIPE_FORMAT_R16G16B16_USCALED]       = ISL_FORMAT_R16G16B16_USCALED,
      [PIPE_FORMAT_R16G16B16A16_USCALED]    = ISL_FORMAT_R16G16B16A16_USCALED,
      [PIPE_FORMAT_R16_SNORM]               = ISL_FORMAT_R16_SNORM,
      [PIPE_FORMAT_R16G16_SNORM]            = ISL_FORMAT_R16G16_SNORM,
      [PIPE_FORMAT_R16G16B16_SNORM]         = ISL_FORMAT_R16G16B16_SNORM,
      [PIPE_FORMAT_R16G16B16A16_SNORM]      = ISL_FORMAT_R16G16B16A16_SNORM,
      [PIPE_FORMAT_R16_SSCALED]             = ISL_FORMAT_R16_SSCALED,
      [PIPE_FORMAT_R16G16_SSCALED]          = ISL_FORMAT_R16G16_SSCALED,
      [PIPE_FORMAT_R16G16B16_SSCALED]       = ISL_FORMAT_R16G16B16_SSCALED,
      [PIPE_FORMAT_R16G16B16A16_SSCALED]    = ISL_FORMAT_R16G16B16A16_SSCALED,
      [PIPE_FORMAT_R8_UNORM]                = ISL_FORMAT_R8_UNORM,
      [PIPE_FORMAT_R8G8_UNORM]              = ISL_FORMAT_R8G8_UNORM,
      [PIPE_FORMAT_R8G8B8_UNORM]            = ISL_FORMAT_R8G8B8_UNORM,
      [PIPE_FORMAT_R8G8B8A8_UNORM]          = ISL_FORMAT_R8G8B8A8_UNORM,
      [PIPE_FORMAT_R8_USCALED]              = ISL_FORMAT_R8_USCALED,
      [PIPE_FORMAT_R8G8_USCALED]            = ISL_FORMAT_R8G8_USCALED,
      [PIPE_FORMAT_R8G8B8_USCALED]          = ISL_FORMAT_R8G8B8_USCALED,
      [PIPE_FORMAT_R8G8B8A8_USCALED]        = ISL_FORMAT_R8G8B8A8_USCALED,
      [PIPE_FORMAT_R8_SNORM]                = ISL_FORMAT_R8_SNORM,
      [PIPE_FORMAT_R8G8_SNORM]              = ISL_FORMAT_R8G8_SNORM,
      [PIPE_FORMAT_R8G8B8_SNORM]            = ISL_FORMAT_R8G8B8_SNORM,
      [PIPE_FORMAT_R8G8B8A8_SNORM]          = ISL_FORMAT_R8G8B8A8_SNORM,
      [PIPE_FORMAT_R8_SSCALED]              = ISL_FORMAT_R8_SSCALED,
      [PIPE_FORMAT_R8G8_SSCALED]            = ISL_FORMAT_R8G8_SSCALED,
      [PIPE_FORMAT_R8G8B8_SSCALED]          = ISL_FORMAT_R8G8B8_SSCALED,
      [PIPE_FORMAT_R8G8B8A8_SSCALED]        = ISL_FORMAT_R8G8B8A8_SSCALED,
      [PIPE_FORMAT_R32_FIXED]               = ISL_FORMAT_R32_SFIXED,
      [PIPE_FORMAT_R32G32_FIXED]            = ISL_FORMAT_R32G32_SFIXED,
      [PIPE_FORMAT_R32G32B32_FIXED]         = ISL_FORMAT_R32G32B32_SFIXED,
      [PIPE_FORMAT_R32G32B32A32_FIXED]      = ISL_FORMAT_R32G32B32A32_SFIXED,
      [PIPE_FORMAT_R16_FLOAT]               = ISL_FORMAT_R16_FLOAT,
      [PIPE_FORMAT_R16G16_FLOAT]            = ISL_FORMAT_R16G16_FLOAT,
      [PIPE_FORMAT_R16G16B16_FLOAT]         = ISL_FORMAT_R16G16B16_FLOAT,
      [PIPE_FORMAT_R16G16B16A16_FLOAT]      = ISL_FORMAT_R16G16B16A16_FLOAT,

      [PIPE_FORMAT_R8G8B8_SRGB]             = ISL_FORMAT_R8G8B8_UNORM_SRGB,
      [PIPE_FORMAT_B8G8R8A8_SRGB]           = ISL_FORMAT_B8G8R8A8_UNORM_SRGB,
      [PIPE_FORMAT_B8G8R8X8_SRGB]           = ISL_FORMAT_B8G8R8X8_UNORM_SRGB,
      [PIPE_FORMAT_R8G8B8A8_SRGB]           = ISL_FORMAT_R8G8B8A8_UNORM_SRGB,

      [PIPE_FORMAT_DXT1_RGB]                = ISL_FORMAT_BC1_UNORM,
      [PIPE_FORMAT_DXT1_RGBA]               = ISL_FORMAT_BC1_UNORM,
      [PIPE_FORMAT_DXT3_RGBA]               = ISL_FORMAT_BC2_UNORM,
      [PIPE_FORMAT_DXT5_RGBA]               = ISL_FORMAT_BC3_UNORM,

      [PIPE_FORMAT_DXT1_SRGB]               = ISL_FORMAT_BC1_UNORM_SRGB,
      [PIPE_FORMAT_DXT1_SRGBA]              = ISL_FORMAT_BC1_UNORM_SRGB,
      [PIPE_FORMAT_DXT3_SRGBA]              = ISL_FORMAT_BC2_UNORM_SRGB,
      [PIPE_FORMAT_DXT5_SRGBA]              = ISL_FORMAT_BC3_UNORM_SRGB,

      [PIPE_FORMAT_RGTC1_UNORM]             = ISL_FORMAT_BC4_UNORM,
      [PIPE_FORMAT_RGTC1_SNORM]             = ISL_FORMAT_BC4_SNORM,
      [PIPE_FORMAT_RGTC2_UNORM]             = ISL_FORMAT_BC5_UNORM,
      [PIPE_FORMAT_RGTC2_SNORM]             = ISL_FORMAT_BC5_SNORM,

      [PIPE_FORMAT_R10G10B10A2_USCALED]     = ISL_FORMAT_R10G10B10A2_USCALED,
      [PIPE_FORMAT_R11G11B10_FLOAT]         = ISL_FORMAT_R11G11B10_FLOAT,
      [PIPE_FORMAT_R9G9B9E5_FLOAT]          = ISL_FORMAT_R9G9B9E5_SHAREDEXP,
      [PIPE_FORMAT_R1_UNORM]                = ISL_FORMAT_R1_UNORM,
      [PIPE_FORMAT_R10G10B10X2_USCALED]     = ISL_FORMAT_R10G10B10X2_USCALED,
      [PIPE_FORMAT_B10G10R10A2_UNORM]       = ISL_FORMAT_B10G10R10A2_UNORM,
      [PIPE_FORMAT_R8G8B8X8_UNORM]          = ISL_FORMAT_R8G8B8X8_UNORM,

      /* Just use red formats for these - they're actually renderable,
       * and faster to sample than the legacy L/I/A/LA formats.
       */
      [PIPE_FORMAT_I8_UNORM]                = ISL_FORMAT_R8_UNORM,
      [PIPE_FORMAT_I8_UINT]                 = ISL_FORMAT_R8_UINT,
      [PIPE_FORMAT_I8_SINT]                 = ISL_FORMAT_R8_SINT,
      [PIPE_FORMAT_I8_SNORM]                = ISL_FORMAT_R8_SNORM,
      [PIPE_FORMAT_I16_UINT]                = ISL_FORMAT_R16_UINT,
      [PIPE_FORMAT_I16_UNORM]               = ISL_FORMAT_R16_UNORM,
      [PIPE_FORMAT_I16_SINT]                = ISL_FORMAT_R16_SINT,
      [PIPE_FORMAT_I16_SNORM]               = ISL_FORMAT_R16_SNORM,
      [PIPE_FORMAT_I16_FLOAT]               = ISL_FORMAT_R16_FLOAT,
      [PIPE_FORMAT_I32_UINT]                = ISL_FORMAT_R32_UINT,
      [PIPE_FORMAT_I32_SINT]                = ISL_FORMAT_R32_SINT,
      [PIPE_FORMAT_I32_FLOAT]               = ISL_FORMAT_R32_FLOAT,

      [PIPE_FORMAT_L8_UINT]                 = ISL_FORMAT_R8_UINT,
      [PIPE_FORMAT_L8_UNORM]                = ISL_FORMAT_R8_UNORM,
      [PIPE_FORMAT_L8_SINT]                 = ISL_FORMAT_R8_SINT,
      [PIPE_FORMAT_L8_SNORM]                = ISL_FORMAT_R8_SNORM,
      [PIPE_FORMAT_L16_UINT]                = ISL_FORMAT_R16_UINT,
      [PIPE_FORMAT_L16_UNORM]               = ISL_FORMAT_R16_UNORM,
      [PIPE_FORMAT_L16_SINT]                = ISL_FORMAT_R16_SINT,
      [PIPE_FORMAT_L16_SNORM]               = ISL_FORMAT_R16_SNORM,
      [PIPE_FORMAT_L16_FLOAT]               = ISL_FORMAT_R16_FLOAT,
      [PIPE_FORMAT_L32_UINT]                = ISL_FORMAT_R32_UINT,
      [PIPE_FORMAT_L32_SINT]                = ISL_FORMAT_R32_SINT,
      [PIPE_FORMAT_L32_FLOAT]               = ISL_FORMAT_R32_FLOAT,

      /* We also map alpha and luminance-alpha formats to red as well,
       * though most of these (other than A8_UNORM) will be non-renderable.
       */
      [PIPE_FORMAT_A8_UINT]                 = ISL_FORMAT_R8_UINT,
      [PIPE_FORMAT_A8_UNORM]                = ISL_FORMAT_R8_UNORM,
      [PIPE_FORMAT_A8_SINT]                 = ISL_FORMAT_R8_SINT,
      [PIPE_FORMAT_A8_SNORM]                = ISL_FORMAT_R8_SNORM,
      [PIPE_FORMAT_A16_UINT]                = ISL_FORMAT_R16_UINT,
      [PIPE_FORMAT_A16_UNORM]               = ISL_FORMAT_R16_UNORM,
      [PIPE_FORMAT_A16_SINT]                = ISL_FORMAT_R16_SINT,
      [PIPE_FORMAT_A16_SNORM]               = ISL_FORMAT_R16_SNORM,
      [PIPE_FORMAT_A16_FLOAT]               = ISL_FORMAT_R16_FLOAT,
      [PIPE_FORMAT_A32_UINT]                = ISL_FORMAT_R32_UINT,
      [PIPE_FORMAT_A32_SINT]                = ISL_FORMAT_R32_SINT,
      [PIPE_FORMAT_A32_FLOAT]               = ISL_FORMAT_R32_FLOAT,

      [PIPE_FORMAT_L8A8_UINT]               = ISL_FORMAT_R8G8_UINT,
      [PIPE_FORMAT_L8A8_UNORM]              = ISL_FORMAT_R8G8_UNORM,
      [PIPE_FORMAT_L8A8_SINT]               = ISL_FORMAT_R8G8_SINT,
      [PIPE_FORMAT_L8A8_SNORM]              = ISL_FORMAT_R8G8_SNORM,
      [PIPE_FORMAT_L16A16_UINT]             = ISL_FORMAT_R16G16_UINT,
      [PIPE_FORMAT_L16A16_UNORM]            = ISL_FORMAT_R16G16_UNORM,
      [PIPE_FORMAT_L16A16_SINT]             = ISL_FORMAT_R16G16_SINT,
      [PIPE_FORMAT_L16A16_SNORM]            = ISL_FORMAT_R16G16_SNORM,
      [PIPE_FORMAT_L16A16_FLOAT]            = ISL_FORMAT_R16G16_FLOAT,
      [PIPE_FORMAT_L32A32_UINT]             = ISL_FORMAT_R32G32_UINT,
      [PIPE_FORMAT_L32A32_SINT]             = ISL_FORMAT_R32G32_SINT,
      [PIPE_FORMAT_L32A32_FLOAT]            = ISL_FORMAT_R32G32_FLOAT,

      /* Sadly, we have to use luminance[-alpha] formats for sRGB decoding. */
      [PIPE_FORMAT_R8_SRGB]                 = ISL_FORMAT_L8_UNORM_SRGB,
      [PIPE_FORMAT_L8_SRGB]                 = ISL_FORMAT_L8_UNORM_SRGB,
      [PIPE_FORMAT_L8A8_SRGB]               = ISL_FORMAT_L8A8_UNORM_SRGB,

      [PIPE_FORMAT_R10G10B10A2_SSCALED]     = ISL_FORMAT_R10G10B10A2_SSCALED,
      [PIPE_FORMAT_R10G10B10A2_SNORM]       = ISL_FORMAT_R10G10B10A2_SNORM,

      [PIPE_FORMAT_B10G10R10A2_USCALED]     = ISL_FORMAT_B10G10R10A2_USCALED,
      [PIPE_FORMAT_B10G10R10A2_SSCALED]     = ISL_FORMAT_B10G10R10A2_SSCALED,
      [PIPE_FORMAT_B10G10R10A2_SNORM]       = ISL_FORMAT_B10G10R10A2_SNORM,

      [PIPE_FORMAT_R8_UINT]                 = ISL_FORMAT_R8_UINT,
      [PIPE_FORMAT_R8G8_UINT]               = ISL_FORMAT_R8G8_UINT,
      [PIPE_FORMAT_R8G8B8_UINT]             = ISL_FORMAT_R8G8B8_UINT,
      [PIPE_FORMAT_R8G8B8A8_UINT]           = ISL_FORMAT_R8G8B8A8_UINT,

      [PIPE_FORMAT_R8_SINT]                 = ISL_FORMAT_R8_SINT,
      [PIPE_FORMAT_R8G8_SINT]               = ISL_FORMAT_R8G8_SINT,
      [PIPE_FORMAT_R8G8B8_SINT]             = ISL_FORMAT_R8G8B8_SINT,
      [PIPE_FORMAT_R8G8B8A8_SINT]           = ISL_FORMAT_R8G8B8A8_SINT,

      [PIPE_FORMAT_R16_UINT]                = ISL_FORMAT_R16_UINT,
      [PIPE_FORMAT_R16G16_UINT]             = ISL_FORMAT_R16G16_UINT,
      [PIPE_FORMAT_R16G16B16_UINT]          = ISL_FORMAT_R16G16B16_UINT,
      [PIPE_FORMAT_R16G16B16A16_UINT]       = ISL_FORMAT_R16G16B16A16_UINT,

      [PIPE_FORMAT_R16_SINT]                = ISL_FORMAT_R16_SINT,
      [PIPE_FORMAT_R16G16_SINT]             = ISL_FORMAT_R16G16_SINT,
      [PIPE_FORMAT_R16G16B16_SINT]          = ISL_FORMAT_R16G16B16_SINT,
      [PIPE_FORMAT_R16G16B16A16_SINT]       = ISL_FORMAT_R16G16B16A16_SINT,

      [PIPE_FORMAT_R32_UINT]                = ISL_FORMAT_R32_UINT,
      [PIPE_FORMAT_R32G32_UINT]             = ISL_FORMAT_R32G32_UINT,
      [PIPE_FORMAT_R32G32B32_UINT]          = ISL_FORMAT_R32G32B32_UINT,
      [PIPE_FORMAT_R32G32B32A32_UINT]       = ISL_FORMAT_R32G32B32A32_UINT,

      [PIPE_FORMAT_R32_SINT]                = ISL_FORMAT_R32_SINT,
      [PIPE_FORMAT_R32G32_SINT]             = ISL_FORMAT_R32G32_SINT,
      [PIPE_FORMAT_R32G32B32_SINT]          = ISL_FORMAT_R32G32B32_SINT,
      [PIPE_FORMAT_R32G32B32A32_SINT]       = ISL_FORMAT_R32G32B32A32_SINT,

      [PIPE_FORMAT_B10G10R10A2_UINT]        = ISL_FORMAT_B10G10R10A2_UINT,

      [PIPE_FORMAT_ETC1_RGB8]               = ISL_FORMAT_ETC1_RGB8,

      /* The formats say YCrCb, but there's no colorspace conversion. */
      [PIPE_FORMAT_R8G8_R8B8_UNORM]         = ISL_FORMAT_YCRCB_NORMAL,
      [PIPE_FORMAT_G8R8_B8R8_UNORM]         = ISL_FORMAT_YCRCB_SWAPY,

      /* We map these formats to help configure media compression. */
      [PIPE_FORMAT_YUYV]                    = ISL_FORMAT_YCRCB_NORMAL,
      [PIPE_FORMAT_UYVY]                    = ISL_FORMAT_YCRCB_SWAPY,
      [PIPE_FORMAT_NV12]                    = ISL_FORMAT_PLANAR_420_8,
      [PIPE_FORMAT_P010]                    = ISL_FORMAT_PLANAR_420_10,
      [PIPE_FORMAT_P012]                    = ISL_FORMAT_PLANAR_420_12,
      [PIPE_FORMAT_P016]                    = ISL_FORMAT_PLANAR_420_16,

      [PIPE_FORMAT_R8G8B8X8_SRGB]           = ISL_FORMAT_R8G8B8X8_UNORM_SRGB,
      [PIPE_FORMAT_B10G10R10X2_UNORM]       = ISL_FORMAT_B10G10R10X2_UNORM,
      [PIPE_FORMAT_R16G16B16X16_UNORM]      = ISL_FORMAT_R16G16B16X16_UNORM,
      [PIPE_FORMAT_R16G16B16X16_FLOAT]      = ISL_FORMAT_R16G16B16X16_FLOAT,
      [PIPE_FORMAT_R32G32B32X32_FLOAT]      = ISL_FORMAT_R32G32B32X32_FLOAT,

      [PIPE_FORMAT_R10G10B10A2_UINT]        = ISL_FORMAT_R10G10B10A2_UINT,

      [PIPE_FORMAT_B5G6R5_SRGB]             = ISL_FORMAT_B5G6R5_UNORM_SRGB,

      [PIPE_FORMAT_BPTC_RGBA_UNORM]         = ISL_FORMAT_BC7_UNORM,
      [PIPE_FORMAT_BPTC_SRGBA]              = ISL_FORMAT_BC7_UNORM_SRGB,
      [PIPE_FORMAT_BPTC_RGB_FLOAT]          = ISL_FORMAT_BC6H_SF16,
      [PIPE_FORMAT_BPTC_RGB_UFLOAT]         = ISL_FORMAT_BC6H_UF16,

      [PIPE_FORMAT_ETC2_RGB8]               = ISL_FORMAT_ETC2_RGB8,
      [PIPE_FORMAT_ETC2_SRGB8]              = ISL_FORMAT_ETC2_SRGB8,
      [PIPE_FORMAT_ETC2_RGB8A1]             = ISL_FORMAT_ETC2_RGB8_PTA,
      [PIPE_FORMAT_ETC2_SRGB8A1]            = ISL_FORMAT_ETC2_SRGB8_PTA,
      [PIPE_FORMAT_ETC2_RGBA8]              = ISL_FORMAT_ETC2_EAC_RGBA8,
      [PIPE_FORMAT_ETC2_SRGBA8]             = ISL_FORMAT_ETC2_EAC_SRGB8_A8,
      [PIPE_FORMAT_ETC2_R11_UNORM]          = ISL_FORMAT_EAC_R11,
      [PIPE_FORMAT_ETC2_R11_SNORM]          = ISL_FORMAT_EAC_SIGNED_R11,
      [PIPE_FORMAT_ETC2_RG11_UNORM]         = ISL_FORMAT_EAC_RG11,
      [PIPE_FORMAT_ETC2_RG11_SNORM]         = ISL_FORMAT_EAC_SIGNED_RG11,

      [PIPE_FORMAT_FXT1_RGB]                = ISL_FORMAT_FXT1,
      [PIPE_FORMAT_FXT1_RGBA]               = ISL_FORMAT_FXT1,

      [PIPE_FORMAT_ASTC_4x4]                = ISL_FORMAT_ASTC_LDR_2D_4X4_FLT16,
      [PIPE_FORMAT_ASTC_5x4]                = ISL_FORMAT_ASTC_LDR_2D_5X4_FLT16,
      [PIPE_FORMAT_ASTC_5x5]                = ISL_FORMAT_ASTC_LDR_2D_5X5_FLT16,
      [PIPE_FORMAT_ASTC_6x5]                = ISL_FORMAT_ASTC_LDR_2D_6X5_FLT16,
      [PIPE_FORMAT_ASTC_6x6]                = ISL_FORMAT_ASTC_LDR_2D_6X6_FLT16,
      [PIPE_FORMAT_ASTC_8x5]                = ISL_FORMAT_ASTC_LDR_2D_8X5_FLT16,
      [PIPE_FORMAT_ASTC_8x6]                = ISL_FORMAT_ASTC_LDR_2D_8X6_FLT16,
      [PIPE_FORMAT_ASTC_8x8]                = ISL_FORMAT_ASTC_LDR_2D_8X8_FLT16,
      [PIPE_FORMAT_ASTC_10x5]               = ISL_FORMAT_ASTC_LDR_2D_10X5_FLT16,
      [PIPE_FORMAT_ASTC_10x6]               = ISL_FORMAT_ASTC_LDR_2D_10X6_FLT16,
      [PIPE_FORMAT_ASTC_10x8]               = ISL_FORMAT_ASTC_LDR_2D_10X8_FLT16,
      [PIPE_FORMAT_ASTC_10x10]              = ISL_FORMAT_ASTC_LDR_2D_10X10_FLT16,
      [PIPE_FORMAT_ASTC_12x10]              = ISL_FORMAT_ASTC_LDR_2D_12X10_FLT16,
      [PIPE_FORMAT_ASTC_12x12]              = ISL_FORMAT_ASTC_LDR_2D_12X12_FLT16,

      [PIPE_FORMAT_ASTC_4x4_SRGB]           = ISL_FORMAT_ASTC_LDR_2D_4X4_U8SRGB,
      [PIPE_FORMAT_ASTC_5x4_SRGB]           = ISL_FORMAT_ASTC_LDR_2D_5X4_U8SRGB,
      [PIPE_FORMAT_ASTC_5x5_SRGB]           = ISL_FORMAT_ASTC_LDR_2D_5X5_U8SRGB,
      [PIPE_FORMAT_ASTC_6x5_SRGB]           = ISL_FORMAT_ASTC_LDR_2D_6X5_U8SRGB,
      [PIPE_FORMAT_ASTC_6x6_SRGB]           = ISL_FORMAT_ASTC_LDR_2D_6X6_U8SRGB,
      [PIPE_FORMAT_ASTC_8x5_SRGB]           = ISL_FORMAT_ASTC_LDR_2D_8X5_U8SRGB,
      [PIPE_FORMAT_ASTC_8x6_SRGB]           = ISL_FORMAT_ASTC_LDR_2D_8X6_U8SRGB,
      [PIPE_FORMAT_ASTC_8x8_SRGB]           = ISL_FORMAT_ASTC_LDR_2D_8X8_U8SRGB,
      [PIPE_FORMAT_ASTC_10x5_SRGB]          = ISL_FORMAT_ASTC_LDR_2D_10X5_U8SRGB,
      [PIPE_FORMAT_ASTC_10x6_SRGB]          = ISL_FORMAT_ASTC_LDR_2D_10X6_U8SRGB,
      [PIPE_FORMAT_ASTC_10x8_SRGB]          = ISL_FORMAT_ASTC_LDR_2D_10X8_U8SRGB,
      [PIPE_FORMAT_ASTC_10x10_SRGB]         = ISL_FORMAT_ASTC_LDR_2D_10X10_U8SRGB,
      [PIPE_FORMAT_ASTC_12x10_SRGB]         = ISL_FORMAT_ASTC_LDR_2D_12X10_U8SRGB,
      [PIPE_FORMAT_ASTC_12x12_SRGB]         = ISL_FORMAT_ASTC_LDR_2D_12X12_U8SRGB,

      [PIPE_FORMAT_A1B5G5R5_UNORM]          = ISL_FORMAT_A1B5G5R5_UNORM,

      /* We support these so that we know the API expects no alpha channel.
       * Otherwise, the state tracker would just give us a format with alpha
       * and we wouldn't know to override the swizzle to 1.
       */
      [PIPE_FORMAT_R16G16B16X16_UINT]       = ISL_FORMAT_R16G16B16A16_UINT,
      [PIPE_FORMAT_R16G16B16X16_SINT]       = ISL_FORMAT_R16G16B16A16_SINT,
      [PIPE_FORMAT_R32G32B32X32_UINT]       = ISL_FORMAT_R32G32B32A32_UINT,
      [PIPE_FORMAT_R32G32B32X32_SINT]       = ISL_FORMAT_R32G32B32A32_SINT,
      [PIPE_FORMAT_R10G10B10X2_SNORM]       = ISL_FORMAT_R10G10B10A2_SNORM,
   };
   assert(pf < PIPE_FORMAT_COUNT);
   return table[pf];
}

static bool
format_info_exists(enum isl_format format)
{
   assert(format != ISL_FORMAT_UNSUPPORTED);
   assert(format < ISL_NUM_FORMATS);
   return format < ARRAY_SIZE(format_info) && format_info[format].exists;
}

bool
isl_format_supports_rendering(const struct intel_device_info *devinfo,
                              enum isl_format format)
{
   if (!format_info_exists(format))
      return false;

   /* If this fails, then we need to update struct surface_format_info */
   assert(devinfo->verx10 <
          (1ul << (8 * sizeof(format_info[format].render_target))));
   return devinfo->verx10 >= format_info[format].render_target;
}

bool
isl_format_supports_alpha_blending(const struct intel_device_info *devinfo,
                                   enum isl_format format)
{
   if (!format_info_exists(format))
      return false;

   return devinfo->verx10 >= format_info[format].alpha_blend;
}

bool
isl_format_supports_sampling(const struct intel_device_info *devinfo,
                             enum isl_format format)
{
   if (!format_info_exists(format))
      return false;

   if (devinfo->platform == INTEL_PLATFORM_BYT) {
      const struct isl_format_layout *fmtl = isl_format_get_layout(format);
      /* Support for ETC1 and ETC2 exists on Bay Trail even though big-core
       * GPUs didn't get it until Broadwell.
       */
      if (fmtl->txc == ISL_TXC_ETC1 || fmtl->txc == ISL_TXC_ETC2)
         return true;
   } else if (devinfo->platform == INTEL_PLATFORM_CHV) {
      /* Support for ASTC LDR theoretically exists on Cherry View even though
       * big-core GPUs didn't get it until Skylake.  However, it's fairly
       * badly broken and requires some nasty workarounds which no Mesa driver
       * has ever implemented.
       */
   } else if (intel_device_info_is_9lp(devinfo)) {
      const struct isl_format_layout *fmtl = isl_format_get_layout(format);
      /* Support for ASTC HDR exists on Broxton even though big-core
       * GPUs didn't get it until Cannonlake.
       */
      if (fmtl->txc == ISL_TXC_ASTC)
         return true;
   } else if (devinfo->verx10 >= 125) {
      const struct isl_format_layout *fmtl = isl_format_get_layout(format);
      /* ASTC & FXT1 support was removed from the hardware on Gfx12.5.
       * Annoyingly, our format_info table doesn't have a concept of things
       * being removed so we handle it as yet another special case.
       *
       * See HSD 1408144932 (ASTC), 1407633611 (FXT1)
       *
       */
      if (fmtl->txc == ISL_TXC_ASTC || fmtl->txc == ISL_TXC_FXT1)
         return false;
   }

   return devinfo->verx10 >= format_info[format].sampling;
}

bool
isl_format_supports_filtering(const struct intel_device_info *devinfo,
                              enum isl_format format)
{
   if (!format_info_exists(format))
      return false;

   if (isl_format_is_compressed(format)) {
      assert(format_info[format].filtering == format_info[format].sampling);
      return isl_format_supports_sampling(devinfo, format);
   }

   return devinfo->verx10 >= format_info[format].filtering;
}

bool
isl_format_supports_vertex_fetch(const struct intel_device_info *devinfo,
                                 enum isl_format format)
{
   if (!format_info_exists(format))
      return false;

   /* For vertex fetch, Bay Trail supports the same set of formats as Haswell
    * but is a superset of Ivy Bridge.
    */
   if (devinfo->platform == INTEL_PLATFORM_BYT)
      return 75 >= format_info[format].input_vb;

   return devinfo->verx10 >= format_info[format].input_vb;
}

/**
 * Returns true if the given format can support typed writes.
 */
bool
isl_format_supports_typed_writes(const struct intel_device_info *devinfo,
                                 enum isl_format format)
{
   if (!format_info_exists(format))
      return false;

   return devinfo->verx10 >= format_info[format].typed_write;
}

/**
 * Returns true if the given format can support typed atomics.
 */
bool
isl_format_supports_typed_atomics(const struct intel_device_info *devinfo,
                                  enum isl_format format)
{
   if (!format_info_exists(format))
      return false;

   return devinfo->verx10 >= format_info[format].typed_atomics;
}

/**
 * Returns true if the given format can support typed reads with format
 * conversion fully handled by hardware.  On Sky Lake, all formats which are
 * supported for typed writes also support typed reads but some of them return
 * the raw image data and don't provide format conversion.
 *
 * For anyone looking to find this data in the PRM, the easiest way to find
 * format tables is to search for R11G11B10.  There are only a few
 * occurrences.
 */
bool
isl_format_supports_typed_reads(const struct intel_device_info *devinfo,
                                enum isl_format format)
{
   if (!format_info_exists(format))
      return false;

   return devinfo->verx10 >= format_info[format].typed_read;
}

/**
 * Returns true if the given format can support single-sample fast clears.
 * This function only checks the format.  In order to determine if a surface
 * supports CCS_E, several other factors need to be considered such as tiling
 * and sample count.  See isl_surf_get_ccs_surf for details.
 */
bool
isl_format_supports_ccs_d(const struct intel_device_info *devinfo,
                          enum isl_format format)
{
   /* Clear-only compression was first added on Ivy Bridge and was last
    * implemented on Ice lake (see BSpec: 43862).
    */
   if (devinfo->ver < 7 || devinfo->ver > 11)
      return false;

   if (!isl_format_supports_rendering(devinfo, format))
      return false;

   const struct isl_format_layout *fmtl = isl_format_get_layout(format);

   /* From the Ivy Bridge PRM, Vol2 Part1 11.7 "MCS Buffer for Render
    * Target(s)", beneath the "Fast Color Clear" bullet (p326):
    *
    *     - MCS buffer for non-MSRT is supported only for RT formats 32bpp,
    *       64bpp, and 128bpp.
    */
   return fmtl->bpb == 32 || fmtl->bpb == 64 || fmtl->bpb == 128;
}

/**
 * Returns true if the given format can support single-sample color
 * compression.  This function only checks the format.  In order to determine
 * if a surface supports CCS_E, several other factors need to be considered
 * such as tiling and sample count.  See isl_surf_get_ccs_surf for details.
 */
bool
isl_format_supports_ccs_e(const struct intel_device_info *devinfo,
                          enum isl_format format)
{
   if (!format_info_exists(format))
      return false;

   /* For simplicity, only report that a format supports CCS_E if blorp can
    * perform bit-for-bit copies with an image of that format while compressed.
    * Unfortunately, R11G11B10_FLOAT is in a compression class of its own, and
    * on ICL, there is no way to copy to/from it which doesn't potentially
    * loose data if one of the bit patterns being copied isn't valid finite
    * floats.
    */
   if (devinfo->ver == 11 && format == ISL_FORMAT_R11G11B10_FLOAT)
      return false;

   return devinfo->verx10 >= format_info[format].ccs_e;
}

bool
isl_format_supports_multisampling(const struct intel_device_info *devinfo,
                                  enum isl_format format)
{
   /* From the Sandybridge PRM, Volume 4 Part 1 p72, SURFACE_STATE, Surface
    * Format:
    *
    *    If Number of Multisamples is set to a value other than
    *    MULTISAMPLECOUNT_1, this field cannot be set to the following
    *    formats:
    *
    *       - any format with greater than 64 bits per element
    *       - any compressed texture format (BC*)
    *       - any YCRCB* format
    *
    * The restriction on the format's size is removed on Broadwell. Moreover,
    * empirically it looks that even IvyBridge can handle multisampled surfaces
    * with format sizes all the way to 128-bits (RGBA32F, RGBA32I, RGBA32UI).
    *
    * Also, there is an exception for HiZ which we treat as a compressed
    * format and is allowed to be multisampled on Broadwell and earlier.
    */
   if (format == ISL_FORMAT_HIZ) {
      /* On SKL+, HiZ is always single-sampled even when the primary surface
       * is multisampled.  See also isl_surf_get_hiz_surf().
       */
      return devinfo->ver <= 8;
   } else if (devinfo->ver == 7 && isl_format_has_sint_channel(format)) {
      /* From the Ivy Bridge PRM, Vol4 Part1 p73 ("Number of Multisamples"):
       *
       *   This field must be set to MULTISAMPLECOUNT_1 for SINT MSRTs when
       *   all RT channels are not written
       *
       * From the Ivy Bridge PRM, Vol4 Part1 p77 ("MCS Enable"):
       *
       *   This field must be set to 0 for all SINT MSRTs when all RT channels
       *   are not written
       *
       * Disable multisampling support now as we don't handle the case when
       * one of the render target channels is disabled.
       */
      return false;
   } else if (devinfo->ver < 7 && isl_format_get_layout(format)->bpb > 64) {
      return false;
   } else if (isl_format_is_compressed(format)) {
      return false;
   } else if (isl_format_is_yuv(format)) {
      return false;
   } else {
      return true;
   }
}

/**
 * Returns true if the two formats are component size compatible meaning that
 * each component from one format has the same number of bits as the other
 * format.
 *
 * This is useful to check whether an image used with 2 different formats can
 * be fast cleared with a non 0 clear color.
 */
bool
isl_formats_have_same_bits_per_channel(enum isl_format format1,
                                       enum isl_format format2)
{
   const struct isl_format_layout *fmtl1 = isl_format_get_layout(format1);
   const struct isl_format_layout *fmtl2 = isl_format_get_layout(format2);

   return fmtl1->channels.r.bits == fmtl2->channels.r.bits &&
          fmtl1->channels.g.bits == fmtl2->channels.g.bits &&
          fmtl1->channels.b.bits == fmtl2->channels.b.bits &&
          fmtl1->channels.a.bits == fmtl2->channels.a.bits &&
          fmtl1->channels.l.bits == fmtl2->channels.l.bits &&
          fmtl1->channels.i.bits == fmtl2->channels.i.bits &&
          fmtl1->channels.p.bits == fmtl2->channels.p.bits;
}

/**
 * Returns true if the two formats are "CCS_E compatible" meaning that you can
 * render in one format with CCS_E enabled and then texture using the other
 * format without needing a resolve.
 *
 * Note: Even if the formats are compatible, special care must be taken if a
 * clear color is involved because the encoding of the clear color is heavily
 * format-dependent.
 */
bool
isl_formats_are_ccs_e_compatible(const struct intel_device_info *devinfo,
                                 enum isl_format format1,
                                 enum isl_format format2)
{
   /* They must support CCS_E */
   if (!isl_format_supports_ccs_e(devinfo, format1) ||
       !isl_format_supports_ccs_e(devinfo, format2))
      return false;

   /* On TGL+, drivers may specify a compression format independently from the
    * surface format. So, even if the surface format changes, hardware is
    * still able to determine how to access the CCS.
    */
   if (devinfo->ver >= 12)
      return true;

   /* The compression used by CCS is not dependent on the actual data encoding
    * of the format but only depends on the bit-layout of the channels.
    */
   return isl_formats_have_same_bits_per_channel(format1, format2);
}

static bool
isl_format_has_channel_type(enum isl_format fmt, enum isl_base_type type)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(fmt);

   return fmtl->channels.r.type == type ||
          fmtl->channels.g.type == type ||
          fmtl->channels.b.type == type ||
          fmtl->channels.a.type == type ||
          fmtl->channels.l.type == type ||
          fmtl->channels.i.type == type ||
          fmtl->channels.p.type == type;
}

bool
isl_format_has_unorm_channel(enum isl_format fmt)
{
   return isl_format_has_channel_type(fmt, ISL_UNORM);
}

bool
isl_format_has_snorm_channel(enum isl_format fmt)
{
   return isl_format_has_channel_type(fmt, ISL_SNORM);
}

bool
isl_format_has_ufloat_channel(enum isl_format fmt)
{
   return isl_format_has_channel_type(fmt, ISL_UFLOAT);
}

bool
isl_format_has_sfloat_channel(enum isl_format fmt)
{
   return isl_format_has_channel_type(fmt, ISL_SFLOAT);
}

bool
isl_format_has_uint_channel(enum isl_format fmt)
{
   return isl_format_has_channel_type(fmt, ISL_UINT);
}

bool
isl_format_has_sint_channel(enum isl_format fmt)
{
   return isl_format_has_channel_type(fmt, ISL_SINT);
}

bool
isl_format_has_color_component(enum isl_format fmt, int component)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(fmt);
   const uint8_t intensity = fmtl->channels.i.bits;
   const uint8_t luminance = fmtl->channels.l.bits;

   switch (component) {
   case 0:
      return (fmtl->channels.r.bits + intensity + luminance) > 0;
   case 1:
      return (fmtl->channels.g.bits + intensity + luminance) > 0;
   case 2:
      return (fmtl->channels.b.bits + intensity + luminance) > 0;
   case 3:
      return (fmtl->channels.a.bits + intensity) > 0;
   default:
      assert(!"Invalid color component: must be 0..3");
      return false;
   }
}

unsigned
isl_format_get_num_channels(enum isl_format fmt)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(fmt);

   assert(fmtl->channels.p.bits == 0);

   return (fmtl->channels.r.bits > 0) +
          (fmtl->channels.g.bits > 0) +
          (fmtl->channels.b.bits > 0) +
          (fmtl->channels.a.bits > 0) +
          (fmtl->channels.l.bits > 0) +
          (fmtl->channels.i.bits > 0);
}

uint32_t
isl_format_get_depth_format(enum isl_format fmt, bool has_stencil)
{
   switch (fmt) {
   default:
      unreachable("bad isl depth format");
   case ISL_FORMAT_R32_FLOAT_X8X24_TYPELESS:
      assert(has_stencil);
      return 0; /* D32_FLOAT_S8X24_UINT */
   case ISL_FORMAT_R32_FLOAT:
      assert(!has_stencil);
      return 1; /* D32_FLOAT */
   case ISL_FORMAT_R24_UNORM_X8_TYPELESS:
      if (has_stencil) {
         return 2; /* D24_UNORM_S8_UINT */
      } else {
         return 3; /* D24_UNORM_X8_UINT */
      }
   case ISL_FORMAT_R16_UNORM:
      assert(!has_stencil);
      return 5; /* D16_UNORM */
   }
}

enum isl_format
isl_format_rgb_to_rgba(enum isl_format rgb)
{
   assert(isl_format_is_rgb(rgb));

   switch (rgb) {
   case ISL_FORMAT_R32G32B32_FLOAT:    return ISL_FORMAT_R32G32B32A32_FLOAT;
   case ISL_FORMAT_R32G32B32_SINT:     return ISL_FORMAT_R32G32B32A32_SINT;
   case ISL_FORMAT_R32G32B32_UINT:     return ISL_FORMAT_R32G32B32A32_UINT;
   case ISL_FORMAT_R32G32B32_UNORM:    return ISL_FORMAT_R32G32B32A32_UNORM;
   case ISL_FORMAT_R32G32B32_SNORM:    return ISL_FORMAT_R32G32B32A32_SNORM;
   case ISL_FORMAT_R32G32B32_SSCALED:  return ISL_FORMAT_R32G32B32A32_SSCALED;
   case ISL_FORMAT_R32G32B32_USCALED:  return ISL_FORMAT_R32G32B32A32_USCALED;
   case ISL_FORMAT_R32G32B32_SFIXED:   return ISL_FORMAT_R32G32B32A32_SFIXED;
   case ISL_FORMAT_R8G8B8_UNORM:       return ISL_FORMAT_R8G8B8A8_UNORM;
   case ISL_FORMAT_R8G8B8_SNORM:       return ISL_FORMAT_R8G8B8A8_SNORM;
   case ISL_FORMAT_R8G8B8_SSCALED:     return ISL_FORMAT_R8G8B8A8_SSCALED;
   case ISL_FORMAT_R8G8B8_USCALED:     return ISL_FORMAT_R8G8B8A8_USCALED;
   case ISL_FORMAT_R16G16B16_FLOAT:    return ISL_FORMAT_R16G16B16A16_FLOAT;
   case ISL_FORMAT_R16G16B16_UNORM:    return ISL_FORMAT_R16G16B16A16_UNORM;
   case ISL_FORMAT_R16G16B16_SNORM:    return ISL_FORMAT_R16G16B16A16_SNORM;
   case ISL_FORMAT_R16G16B16_SSCALED:  return ISL_FORMAT_R16G16B16A16_SSCALED;
   case ISL_FORMAT_R16G16B16_USCALED:  return ISL_FORMAT_R16G16B16A16_USCALED;
   case ISL_FORMAT_R8G8B8_UNORM_SRGB:  return ISL_FORMAT_R8G8B8A8_UNORM_SRGB;
   case ISL_FORMAT_R16G16B16_UINT:     return ISL_FORMAT_R16G16B16A16_UINT;
   case ISL_FORMAT_R16G16B16_SINT:     return ISL_FORMAT_R16G16B16A16_SINT;
   case ISL_FORMAT_R8G8B8_UINT:        return ISL_FORMAT_R8G8B8A8_UINT;
   case ISL_FORMAT_R8G8B8_SINT:        return ISL_FORMAT_R8G8B8A8_SINT;
   default:
      return ISL_FORMAT_UNSUPPORTED;
   }
}

enum isl_format
isl_format_rgb_to_rgbx(enum isl_format rgb)
{
   assert(isl_format_is_rgb(rgb));

   switch (rgb) {
   case ISL_FORMAT_R32G32B32_FLOAT:
      return ISL_FORMAT_R32G32B32X32_FLOAT;
   case ISL_FORMAT_R16G16B16_UNORM:
      return ISL_FORMAT_R16G16B16X16_UNORM;
   case ISL_FORMAT_R16G16B16_FLOAT:
      return ISL_FORMAT_R16G16B16X16_FLOAT;
   case ISL_FORMAT_R8G8B8_UNORM:
      return ISL_FORMAT_R8G8B8X8_UNORM;
   case ISL_FORMAT_R8G8B8_UNORM_SRGB:
      return ISL_FORMAT_R8G8B8X8_UNORM_SRGB;
   default:
      return ISL_FORMAT_UNSUPPORTED;
   }
}

enum isl_format
isl_format_rgbx_to_rgba(enum isl_format rgbx)
{
   assert(isl_format_is_rgbx(rgbx));

   switch (rgbx) {
   case ISL_FORMAT_R32G32B32X32_FLOAT:
      return ISL_FORMAT_R32G32B32A32_FLOAT;
   case ISL_FORMAT_R16G16B16X16_UNORM:
      return ISL_FORMAT_R16G16B16A16_UNORM;
   case ISL_FORMAT_R16G16B16X16_FLOAT:
      return ISL_FORMAT_R16G16B16A16_FLOAT;
   case ISL_FORMAT_B8G8R8X8_UNORM:
      return ISL_FORMAT_B8G8R8A8_UNORM;
   case ISL_FORMAT_B8G8R8X8_UNORM_SRGB:
      return ISL_FORMAT_B8G8R8A8_UNORM_SRGB;
   case ISL_FORMAT_R8G8B8X8_UNORM:
      return ISL_FORMAT_R8G8B8A8_UNORM;
   case ISL_FORMAT_R8G8B8X8_UNORM_SRGB:
      return ISL_FORMAT_R8G8B8A8_UNORM_SRGB;
   case ISL_FORMAT_B10G10R10X2_UNORM:
      return ISL_FORMAT_B10G10R10A2_UNORM;
   case ISL_FORMAT_B5G5R5X1_UNORM:
      return ISL_FORMAT_B5G5R5A1_UNORM;
   case ISL_FORMAT_B5G5R5X1_UNORM_SRGB:
      return ISL_FORMAT_B5G5R5A1_UNORM_SRGB;
   default:
      assert(!"Invalid RGBX format");
      return rgbx;
   }
}

/*
 * Xe2 allows route of LD messages from Sampler to LSC to improve performance
 * when some restrictions are met, here checking the format restrictions.
 *
 * RENDER_SURFACE_STATE::Enable Sampler Route to LSC:
 *   "The Surface Format is one of the following:
 *
 *     R8_UNORM, R8G8_UNORM, R16_UNORM, R16G16_UNORM, R16G16B16A16_UNORM
 *     R16_FLOAT, R16G16_FLOAT, R16G16B16A16_FLOAT
 *     R32_FLOAT, R32G32_FLOAT, R32G32B32A32_FLOAT, R32_UINT, R32G32_UINT, R32G32B32A32_UINT
 *     R10G10B10A2_UNORM, R11G11B10_FLOAT
 *   "
 */
bool
isl_format_support_sampler_route_to_lsc(enum isl_format fmt)
{
   switch (fmt) {
   case ISL_FORMAT_R8_UNORM:
   case ISL_FORMAT_R8G8_UNORM:
   case ISL_FORMAT_R16_UNORM:
   case ISL_FORMAT_R16G16_UNORM:
   case ISL_FORMAT_R16G16B16A16_UNORM:
   case ISL_FORMAT_R16_FLOAT:
   case ISL_FORMAT_R16G16_FLOAT:
   case ISL_FORMAT_R16G16B16A16_FLOAT:
   case ISL_FORMAT_R32_FLOAT:
   case ISL_FORMAT_R32G32_FLOAT:
   case ISL_FORMAT_R32G32B32A32_FLOAT:
   case ISL_FORMAT_R32_UINT:
   case ISL_FORMAT_R32G32_UINT:
   case ISL_FORMAT_R32G32B32A32_UINT:
   case ISL_FORMAT_R10G10B10A2_UNORM:
   case ISL_FORMAT_R11G11B10_FLOAT:
      return true;
   default:
      return false;
   }
}

static inline void
pack_channel(const union isl_color_value *value, unsigned i,
             const struct isl_channel_layout *layout,
             enum isl_colorspace colorspace,
             uint32_t data_out[4])
{
   if (layout->type == ISL_VOID)
      return;

   if (colorspace == ISL_COLORSPACE_SRGB)
      assert(layout->type == ISL_UNORM);

   uint32_t packed;
   switch (layout->type) {
   case ISL_UNORM:
      if (colorspace == ISL_COLORSPACE_SRGB) {
         if (layout->bits == 8) {
            packed = util_format_linear_float_to_srgb_8unorm(value->f32[i]);
         } else {
            float srgb = util_format_linear_to_srgb_float(value->f32[i]);
            packed = _mesa_float_to_unorm(srgb, layout->bits);
         }
      } else {
         packed = _mesa_float_to_unorm(value->f32[i], layout->bits);
      }
      break;
   case ISL_SNORM:
      packed = _mesa_float_to_snorm(value->f32[i], layout->bits);
      break;
   case ISL_SFLOAT:
      assert(layout->bits == 16 || layout->bits == 32);
      if (layout->bits == 16) {
         packed = _mesa_float_to_half(value->f32[i]);
      } else {
         packed = value->u32[i];
      }
      break;
   case ISL_UINT:
      packed = MIN(value->u32[i], u_uintN_max(layout->bits));
      break;
   case ISL_SINT:
      packed = CLAMP(value->u32[i], u_intN_min(layout->bits),
                     u_intN_max(layout->bits));
      break;

   default:
      unreachable("Invalid channel type");
   }

   unsigned dword = layout->start_bit / 32;
   unsigned bit = layout->start_bit % 32;
   assert(bit + layout->bits <= 32);
   data_out[dword] |= (packed & u_uintN_max(layout->bits)) << bit;
}

/**
 * Take an isl_color_value and pack it into the actual bits as specified by
 * the isl_format.  This function is very slow for a format conversion
 * function but should be fine for a single pixel worth of data.
 */
void
isl_color_value_pack(const union isl_color_value *value,
                     enum isl_format format,
                     uint32_t *data_out)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(format);
   assert(fmtl->colorspace == ISL_COLORSPACE_LINEAR ||
          fmtl->colorspace == ISL_COLORSPACE_SRGB);
   assert(!isl_format_is_compressed(format));

   memset(data_out, 0, isl_align(fmtl->bpb, 32) / 8);

   if (format == ISL_FORMAT_R9G9B9E5_SHAREDEXP) {
      data_out[0] = float3_to_rgb9e5(value->f32);
      return;
   } else if (format == ISL_FORMAT_R11G11B10_FLOAT) {
      data_out[0] = float3_to_r11g11b10f(value->f32);
      return;
   }

   pack_channel(value, 0, &fmtl->channels.r, fmtl->colorspace, data_out);
   pack_channel(value, 1, &fmtl->channels.g, fmtl->colorspace, data_out);
   pack_channel(value, 2, &fmtl->channels.b, fmtl->colorspace, data_out);
   pack_channel(value, 3, &fmtl->channels.a, ISL_COLORSPACE_LINEAR, data_out);
   pack_channel(value, 0, &fmtl->channels.l, fmtl->colorspace, data_out);
   pack_channel(value, 0, &fmtl->channels.i, ISL_COLORSPACE_LINEAR, data_out);
   assert(fmtl->channels.p.bits == 0);
}

static inline void
unpack_channel(union isl_color_value *value,
               unsigned start, unsigned count,
               const struct isl_channel_layout *layout,
               enum isl_colorspace colorspace,
               const uint32_t *data_in)
{
   if (layout->type == ISL_VOID)
      return;

   unsigned dword = layout->start_bit / 32;
   unsigned bit = layout->start_bit % 32;
   assert(bit + layout->bits <= 32);
   uint32_t packed = (data_in[dword] >> bit) & u_uintN_max(layout->bits);

   union {
      uint32_t u32;
      float f32;
   } unpacked;

   if (colorspace == ISL_COLORSPACE_SRGB)
      assert(layout->type == ISL_UNORM);

   switch (layout->type) {
   case ISL_UNORM:
      if (colorspace == ISL_COLORSPACE_SRGB) {
         if (layout->bits == 8) {
            unpacked.f32 = util_format_srgb_8unorm_to_linear_float(packed);
         } else {
            float srgb = _mesa_unorm_to_float(packed, layout->bits);
            unpacked.f32 = util_format_srgb_to_linear_float(srgb);
         }
      } else {
         unpacked.f32 = _mesa_unorm_to_float(packed, layout->bits);
      }
      break;
   case ISL_SNORM:
      unpacked.f32 = _mesa_snorm_to_float(util_sign_extend(packed, layout->bits),
                                          layout->bits);
      break;
   case ISL_SFLOAT:
      assert(layout->bits == 16 || layout->bits == 32);
      if (layout->bits == 16) {
         unpacked.f32 = _mesa_half_to_float(packed);
      } else {
         unpacked.u32 = packed;
      }
      break;
   case ISL_UINT:
      unpacked.u32 = packed;
      break;
   case ISL_SINT:
      unpacked.u32 = util_sign_extend(packed, layout->bits);
      break;

   default:
      unreachable("Invalid channel type");
   }

   for (unsigned i = 0; i < count; i++)
      value->u32[start + i] = unpacked.u32;
}

/**
 * Unpack an isl_color_value from the actual bits as specified by
 * the isl_format.  This function is very slow for a format conversion
 * function but should be fine for a single pixel worth of data.
 */
void
isl_color_value_unpack(union isl_color_value *value,
                       enum isl_format format,
                       const uint32_t *data_in)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(format);
   assert(fmtl->colorspace == ISL_COLORSPACE_LINEAR ||
          fmtl->colorspace == ISL_COLORSPACE_SRGB);
   assert(!isl_format_is_compressed(format));

   /* Default to opaque black. */
   memset(value, 0, sizeof(*value));
   if (isl_format_has_int_channel(format)) {
      value->u32[3] = 1u;
   } else {
      value->f32[3] = 1.0f;
   }

   if (format == ISL_FORMAT_R9G9B9E5_SHAREDEXP) {
      rgb9e5_to_float3(data_in[0], value->f32);
      return;
   } else if (format == ISL_FORMAT_R11G11B10_FLOAT) {
      r11g11b10f_to_float3(data_in[0], value->f32);
      return;
   }

   unpack_channel(value, 0, 1, &fmtl->channels.r, fmtl->colorspace, data_in);
   unpack_channel(value, 1, 1, &fmtl->channels.g, fmtl->colorspace, data_in);
   unpack_channel(value, 2, 1, &fmtl->channels.b, fmtl->colorspace, data_in);
   unpack_channel(value, 3, 1, &fmtl->channels.a, ISL_COLORSPACE_LINEAR, data_in);
   unpack_channel(value, 0, 3, &fmtl->channels.l, fmtl->colorspace, data_in);
   unpack_channel(value, 0, 4, &fmtl->channels.i, ISL_COLORSPACE_LINEAR, data_in);
   assert(fmtl->channels.p.bits == 0);
}
