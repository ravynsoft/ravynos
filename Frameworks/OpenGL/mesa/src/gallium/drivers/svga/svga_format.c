/**********************************************************
 * Copyright 2011 VMware, Inc.  All rights reserved.
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


#include "util/format/u_formats.h"
#include "util/u_debug.h"
#include "util/format/u_format.h"
#include "util/u_memory.h"

#include "svga_winsys.h"
#include "svga_screen.h"
#include "svga_format.h"


/** Describes mapping from gallium formats to SVGA vertex/pixel formats */
struct vgpu10_format_entry
{
   SVGA3dSurfaceFormat vertex_format;
   SVGA3dSurfaceFormat pixel_format;
   SVGA3dSurfaceFormat view_format;   /* view format for texture buffer */
   unsigned flags;
};

struct format_compat_entry
{
   enum pipe_format pformat;
   const SVGA3dSurfaceFormat *compat_format;
};


/**
 * Table mapping Gallium formats to SVGA3d vertex/pixel formats.
 * Note: the table is ordered according to PIPE_FORMAT_x order.
 */
static const struct vgpu10_format_entry format_conversion_table[] =
{
   /* Gallium format                    SVGA3D vertex format        SVGA3D pixel format          SVGA3D texbuf view format    Flags */
   [ PIPE_FORMAT_B8G8R8A8_UNORM ] =        { SVGA3D_B8G8R8A8_UNORM,      SVGA3D_B8G8R8A8_UNORM,       SVGA3D_B8G8R8A8_UNORM,       TF_GEN_MIPS },
   [ PIPE_FORMAT_B8G8R8X8_UNORM ] =        { SVGA3D_FORMAT_INVALID,      SVGA3D_B8G8R8X8_UNORM,       SVGA3D_B8G8R8X8_UNORM,       TF_GEN_MIPS },
   [ PIPE_FORMAT_B5G5R5A1_UNORM ] =        { SVGA3D_FORMAT_INVALID,      SVGA3D_B5G5R5A1_UNORM,       SVGA3D_B5G5R5A1_UNORM,       TF_GEN_MIPS },
   [ PIPE_FORMAT_B5G6R5_UNORM ] =          { SVGA3D_FORMAT_INVALID,      SVGA3D_B5G6R5_UNORM,         SVGA3D_B5G6R5_UNORM,         TF_GEN_MIPS },
   [ PIPE_FORMAT_R10G10B10A2_UNORM ] =     { SVGA3D_R10G10B10A2_UNORM,   SVGA3D_R10G10B10A2_UNORM,    SVGA3D_R10G10B10A2_UNORM,    TF_GEN_MIPS | TF_UAV },
   [ PIPE_FORMAT_L8_UNORM ] =              { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R8_UNORM,             TF_XXX1 },
   [ PIPE_FORMAT_A8_UNORM ] =              { SVGA3D_FORMAT_INVALID,      SVGA3D_A8_UNORM,             SVGA3D_R8_UNORM,             TF_GEN_MIPS | TF_000X | TF_UAV },
   [ PIPE_FORMAT_I8_UNORM ] =              { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R8_UNORM,             TF_XXXX },
   [ PIPE_FORMAT_L8A8_UNORM ] =            { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R8G8_UNORM,           TF_XXXY },
   [ PIPE_FORMAT_L16_UNORM ] =             { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R16_UNORM,            TF_XXX1 },
   [ PIPE_FORMAT_Z16_UNORM ] =             { SVGA3D_FORMAT_INVALID,      SVGA3D_D16_UNORM,            SVGA3D_D16_UNORM,            0 },
   [ PIPE_FORMAT_Z32_FLOAT ] =             { SVGA3D_FORMAT_INVALID,      SVGA3D_D32_FLOAT,            SVGA3D_D32_FLOAT,            0 },
   [ PIPE_FORMAT_Z24_UNORM_S8_UINT ] =     { SVGA3D_FORMAT_INVALID,      SVGA3D_D24_UNORM_S8_UINT,    SVGA3D_D24_UNORM_S8_UINT,    0 },
   [ PIPE_FORMAT_Z24X8_UNORM ] =           { SVGA3D_FORMAT_INVALID,      SVGA3D_D24_UNORM_S8_UINT,    SVGA3D_D24_UNORM_S8_UINT,    0 },
   [ PIPE_FORMAT_R32_FLOAT ] =             { SVGA3D_R32_FLOAT,           SVGA3D_R32_FLOAT,            SVGA3D_R32_FLOAT,            TF_GEN_MIPS | TF_UAV },
   [ PIPE_FORMAT_R32G32_FLOAT ] =          { SVGA3D_R32G32_FLOAT,        SVGA3D_R32G32_FLOAT,         SVGA3D_R32G32_FLOAT,         TF_GEN_MIPS | TF_UAV },
   [ PIPE_FORMAT_R32G32B32_FLOAT ] =       { SVGA3D_R32G32B32_FLOAT,     SVGA3D_R32G32B32_FLOAT,      SVGA3D_R32G32B32_FLOAT,      TF_GEN_MIPS },
   [ PIPE_FORMAT_R32G32B32A32_FLOAT ] =    { SVGA3D_R32G32B32A32_FLOAT,  SVGA3D_R32G32B32A32_FLOAT,   SVGA3D_R32G32B32A32_FLOAT,   TF_GEN_MIPS | TF_UAV },
   [ PIPE_FORMAT_R32_USCALED ] =           { SVGA3D_R32_UINT,            SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_U_TO_F_CAST },
   [ PIPE_FORMAT_R32G32_USCALED ] =        { SVGA3D_R32G32_UINT,         SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_U_TO_F_CAST },
   [ PIPE_FORMAT_R32G32B32_USCALED ] =     { SVGA3D_R32G32B32_UINT,      SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_U_TO_F_CAST },
   [ PIPE_FORMAT_R32G32B32A32_USCALED ] =  { SVGA3D_R32G32B32A32_UINT,   SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_U_TO_F_CAST },
   [ PIPE_FORMAT_R32_SSCALED ] =           { SVGA3D_R32_SINT,            SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_I_TO_F_CAST },
   [ PIPE_FORMAT_R32G32_SSCALED ] =        { SVGA3D_R32G32_SINT,         SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_I_TO_F_CAST },
   [ PIPE_FORMAT_R32G32B32_SSCALED ] =     { SVGA3D_R32G32B32_SINT,      SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_I_TO_F_CAST },
   [ PIPE_FORMAT_R32G32B32A32_SSCALED ] =  { SVGA3D_R32G32B32A32_SINT,   SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_I_TO_F_CAST },
   [ PIPE_FORMAT_R16_UNORM ] =             { SVGA3D_R16_UNORM,           SVGA3D_R16_UNORM,            SVGA3D_R16_UNORM,            TF_GEN_MIPS | TF_UAV },
   [ PIPE_FORMAT_R16G16_UNORM ] =          { SVGA3D_R16G16_UNORM,        SVGA3D_R16G16_UNORM,         SVGA3D_R16G16_UNORM,         TF_GEN_MIPS | TF_UAV },
   [ PIPE_FORMAT_R16G16B16_UNORM ] =       { SVGA3D_R16G16B16A16_UNORM,  SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_W_TO_1 },
   [ PIPE_FORMAT_R16G16B16A16_UNORM ] =    { SVGA3D_R16G16B16A16_UNORM,  SVGA3D_R16G16B16A16_UNORM,   SVGA3D_R16G16B16A16_UNORM,   TF_GEN_MIPS | TF_UAV },
   [ PIPE_FORMAT_R16_USCALED ] =           { SVGA3D_R16_UINT,            SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_U_TO_F_CAST },
   [ PIPE_FORMAT_R16G16_USCALED ] =        { SVGA3D_R16G16_UINT,         SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_U_TO_F_CAST },
   [ PIPE_FORMAT_R16G16B16_USCALED ] =     { SVGA3D_R16G16B16A16_UINT,   SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_W_TO_1 | VF_U_TO_F_CAST },
   [ PIPE_FORMAT_R16G16B16A16_USCALED ] =  { SVGA3D_R16G16B16A16_UINT,   SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_U_TO_F_CAST },
   [ PIPE_FORMAT_R16_SNORM ] =             { SVGA3D_R16_SNORM,           SVGA3D_R16_SNORM,            SVGA3D_R16_SNORM,            TF_UAV },
   [ PIPE_FORMAT_R16G16_SNORM ] =          { SVGA3D_R16G16_SNORM,        SVGA3D_R16G16_SNORM,         SVGA3D_R16G16_SNORM,         TF_UAV },
   [ PIPE_FORMAT_R16G16B16_SNORM ] =       { SVGA3D_R16G16B16A16_SNORM,  SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_W_TO_1 },
   [ PIPE_FORMAT_R16G16B16A16_SNORM ] =    { SVGA3D_R16G16B16A16_SNORM,  SVGA3D_R16G16B16A16_SNORM,   SVGA3D_R16G16B16A16_SNORM,   TF_UAV },
   [ PIPE_FORMAT_R16_SSCALED ] =           { SVGA3D_R16_SINT,            SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_I_TO_F_CAST },
   [ PIPE_FORMAT_R16G16_SSCALED ] =        { SVGA3D_R16G16_SINT,         SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_I_TO_F_CAST },
   [ PIPE_FORMAT_R16G16B16_SSCALED ] =     { SVGA3D_R16G16B16A16_SINT,   SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_W_TO_1 | VF_I_TO_F_CAST },
   [ PIPE_FORMAT_R16G16B16A16_SSCALED ] =  { SVGA3D_R16G16B16A16_SINT,   SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_I_TO_F_CAST },
   [ PIPE_FORMAT_R8_UNORM ] =              { SVGA3D_R8_UNORM,            SVGA3D_R8_UNORM,             SVGA3D_R8_UNORM,             TF_GEN_MIPS | TF_UAV },
   [ PIPE_FORMAT_R8G8_UNORM ] =            { SVGA3D_R8G8_UNORM,          SVGA3D_R8G8_UNORM,           SVGA3D_R8G8_UNORM,           TF_GEN_MIPS | TF_UAV },
   [ PIPE_FORMAT_R8G8B8_UNORM ] =          { SVGA3D_R8G8B8A8_UNORM,      SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_W_TO_1 },
   [ PIPE_FORMAT_R8G8B8A8_UNORM ] =        { SVGA3D_R8G8B8A8_UNORM,      SVGA3D_R8G8B8A8_UNORM,       SVGA3D_R8G8B8A8_UNORM,       TF_GEN_MIPS | TF_UAV },
   [ PIPE_FORMAT_R8_USCALED ] =            { SVGA3D_R8_UINT,             SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_U_TO_F_CAST },
   [ PIPE_FORMAT_R8G8_USCALED ] =          { SVGA3D_R8G8_UINT,           SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_U_TO_F_CAST },
   [ PIPE_FORMAT_R8G8B8_USCALED ] =        { SVGA3D_R8G8B8A8_UINT,       SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_W_TO_1 | VF_U_TO_F_CAST },
   [ PIPE_FORMAT_R8G8B8A8_USCALED ] =      { SVGA3D_R8G8B8A8_UINT,       SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_U_TO_F_CAST },
   [ PIPE_FORMAT_R8_SNORM ] =              { SVGA3D_R8_SNORM,            SVGA3D_R8_SNORM,             SVGA3D_R8_SNORM,             TF_UAV },
   [ PIPE_FORMAT_R8G8_SNORM ] =            { SVGA3D_R8G8_SNORM,          SVGA3D_R8G8_SNORM,           SVGA3D_R8G8_SNORM,           TF_UAV },
   [ PIPE_FORMAT_R8G8B8_SNORM ] =          { SVGA3D_R8G8B8A8_SNORM,      SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_W_TO_1 },
   [ PIPE_FORMAT_R8G8B8A8_SNORM ] =        { SVGA3D_R8G8B8A8_SNORM,      SVGA3D_R8G8B8A8_SNORM,       SVGA3D_R8G8B8A8_SNORM,       TF_UAV },
   [ PIPE_FORMAT_R8_SSCALED ] =            { SVGA3D_R8_SINT,             SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_I_TO_F_CAST },
   [ PIPE_FORMAT_R8G8_SSCALED ] =          { SVGA3D_R8G8_SINT,           SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_I_TO_F_CAST },
   [ PIPE_FORMAT_R8G8B8_SSCALED ] =        { SVGA3D_R8G8B8A8_SINT,       SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_W_TO_1 | VF_I_TO_F_CAST },
   [ PIPE_FORMAT_R8G8B8A8_SSCALED ] =      { SVGA3D_R8G8B8A8_SINT,       SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_I_TO_F_CAST },
   [ PIPE_FORMAT_R16_FLOAT ] =             { SVGA3D_R16_FLOAT,           SVGA3D_R16_FLOAT,            SVGA3D_R16_FLOAT,            TF_GEN_MIPS | TF_UAV },
   [ PIPE_FORMAT_R16G16_FLOAT ] =          { SVGA3D_R16G16_FLOAT,        SVGA3D_R16G16_FLOAT,         SVGA3D_R16G16_FLOAT,         TF_GEN_MIPS | TF_UAV },
   [ PIPE_FORMAT_R16G16B16_FLOAT ] =       { SVGA3D_R16G16B16A16_FLOAT,  SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_W_TO_1 },
   [ PIPE_FORMAT_R16G16B16A16_FLOAT ] =    { SVGA3D_R16G16B16A16_FLOAT,  SVGA3D_R16G16B16A16_FLOAT,   SVGA3D_R16G16B16A16_FLOAT,   TF_GEN_MIPS | TF_UAV },
   [ PIPE_FORMAT_B8G8R8A8_SRGB ] =         { SVGA3D_FORMAT_INVALID,      SVGA3D_B8G8R8A8_UNORM_SRGB,  SVGA3D_FORMAT_INVALID,       TF_GEN_MIPS },
   [ PIPE_FORMAT_B8G8R8X8_SRGB ] =         { SVGA3D_FORMAT_INVALID,      SVGA3D_B8G8R8X8_UNORM_SRGB,  SVGA3D_FORMAT_INVALID,       TF_GEN_MIPS },
   [ PIPE_FORMAT_R8G8B8A8_SRGB ] =         { SVGA3D_FORMAT_INVALID,      SVGA3D_R8G8B8A8_UNORM_SRGB,  SVGA3D_FORMAT_INVALID,       TF_GEN_MIPS },
   [ PIPE_FORMAT_DXT1_RGB ] =              { SVGA3D_FORMAT_INVALID,      SVGA3D_BC1_UNORM,            SVGA3D_FORMAT_INVALID,       0 },
   [ PIPE_FORMAT_DXT1_RGBA ] =             { SVGA3D_FORMAT_INVALID,      SVGA3D_BC1_UNORM,            SVGA3D_FORMAT_INVALID,       0 },
   [ PIPE_FORMAT_DXT3_RGBA ] =             { SVGA3D_FORMAT_INVALID,      SVGA3D_BC2_UNORM,            SVGA3D_FORMAT_INVALID,       0 },
   [ PIPE_FORMAT_DXT5_RGBA ] =             { SVGA3D_FORMAT_INVALID,      SVGA3D_BC3_UNORM,            SVGA3D_FORMAT_INVALID,       0 },
   [ PIPE_FORMAT_DXT1_SRGB ] =             { SVGA3D_FORMAT_INVALID,      SVGA3D_BC1_UNORM_SRGB,       SVGA3D_FORMAT_INVALID,       0 },
   [ PIPE_FORMAT_DXT1_SRGBA ] =            { SVGA3D_FORMAT_INVALID,      SVGA3D_BC1_UNORM_SRGB,       SVGA3D_FORMAT_INVALID,       0 },
   [ PIPE_FORMAT_DXT3_SRGBA ] =            { SVGA3D_FORMAT_INVALID,      SVGA3D_BC2_UNORM_SRGB,       SVGA3D_FORMAT_INVALID,       0 },
   [ PIPE_FORMAT_DXT5_SRGBA ] =            { SVGA3D_FORMAT_INVALID,      SVGA3D_BC3_UNORM_SRGB,       SVGA3D_FORMAT_INVALID,       0 },
   [ PIPE_FORMAT_RGTC1_UNORM ] =           { SVGA3D_FORMAT_INVALID,      SVGA3D_BC4_UNORM,            SVGA3D_FORMAT_INVALID,       0 },
   [ PIPE_FORMAT_RGTC1_SNORM ] =           { SVGA3D_FORMAT_INVALID,      SVGA3D_BC4_SNORM,            SVGA3D_FORMAT_INVALID,       0 },
   [ PIPE_FORMAT_RGTC2_UNORM ] =           { SVGA3D_FORMAT_INVALID,      SVGA3D_BC5_UNORM,            SVGA3D_FORMAT_INVALID,       0 },
   [ PIPE_FORMAT_RGTC2_SNORM ] =           { SVGA3D_FORMAT_INVALID,      SVGA3D_BC5_SNORM,            SVGA3D_FORMAT_INVALID,       0 },
   [ PIPE_FORMAT_R10G10B10A2_USCALED ] =   { SVGA3D_R10G10B10A2_UNORM,   SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_PUINT_TO_USCALED },
   [ PIPE_FORMAT_R11G11B10_FLOAT ] =       { SVGA3D_FORMAT_INVALID,      SVGA3D_R11G11B10_FLOAT,      SVGA3D_R11G11B10_FLOAT,      TF_GEN_MIPS | TF_UAV },
   [ PIPE_FORMAT_R9G9B9E5_FLOAT ] =        { SVGA3D_FORMAT_INVALID,      SVGA3D_R9G9B9E5_SHAREDEXP,   SVGA3D_FORMAT_INVALID,       0 },
   [ PIPE_FORMAT_Z32_FLOAT_S8X24_UINT ] =  { SVGA3D_FORMAT_INVALID,      SVGA3D_D32_FLOAT_S8X24_UINT, SVGA3D_FORMAT_INVALID,       0 },
   [ PIPE_FORMAT_B10G10R10A2_UNORM ] =     { SVGA3D_R10G10B10A2_UNORM,   SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_BGRA },
   [ PIPE_FORMAT_L16A16_UNORM ] =          { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R16G16_UNORM,         TF_XXXY },
   [ PIPE_FORMAT_A16_UNORM ] =             { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R16_UNORM,            TF_000X },
   [ PIPE_FORMAT_I16_UNORM ] =             { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R16_UNORM,            TF_XXXX },
   [ PIPE_FORMAT_A16_FLOAT ] =             { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R16_FLOAT,            TF_000X },
   [ PIPE_FORMAT_L16_FLOAT ] =             { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R16_FLOAT,            TF_XXX1 },
   [ PIPE_FORMAT_L16A16_FLOAT ] =          { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R16G16_FLOAT,         TF_XXXY },
   [ PIPE_FORMAT_I16_FLOAT ] =             { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R16_FLOAT,            TF_XXXX },
   [ PIPE_FORMAT_A32_FLOAT ] =             { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R32_FLOAT,            TF_000X },
   [ PIPE_FORMAT_L32_FLOAT ] =             { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R32_FLOAT,            TF_XXX1 },
   [ PIPE_FORMAT_L32A32_FLOAT ] =          { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R32G32_FLOAT,         TF_XXXY },
   [ PIPE_FORMAT_I32_FLOAT ] =             { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R32_FLOAT,            TF_XXXX },
   [ PIPE_FORMAT_R10G10B10A2_SSCALED ] =   { SVGA3D_R32_UINT,            SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_PUINT_TO_SSCALED },
   [ PIPE_FORMAT_R10G10B10A2_SNORM ] =     { SVGA3D_R10G10B10A2_UNORM,   SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_PUINT_TO_SNORM },
   [ PIPE_FORMAT_B10G10R10A2_USCALED ] =   { SVGA3D_R10G10B10A2_UNORM,   SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_BGRA | VF_PUINT_TO_USCALED },
   [ PIPE_FORMAT_B10G10R10A2_SSCALED ] =   { SVGA3D_R32_UINT,            SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_BGRA | VF_PUINT_TO_SSCALED },
   [ PIPE_FORMAT_B10G10R10A2_SNORM ] =     { SVGA3D_R10G10B10A2_UNORM,   SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_BGRA | VF_PUINT_TO_SNORM },
   [ PIPE_FORMAT_R8_UINT ] =               { SVGA3D_R8_UINT,             SVGA3D_R8_UINT,              SVGA3D_R8_UINT,              TF_UAV },
   [ PIPE_FORMAT_R8G8_UINT ] =             { SVGA3D_R8G8_UINT,           SVGA3D_R8G8_UINT,            SVGA3D_R8G8_UINT,            TF_UAV },
   [ PIPE_FORMAT_R8G8B8_UINT ] =           { SVGA3D_R8G8B8A8_UINT,       SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_W_TO_1 },
   [ PIPE_FORMAT_R8G8B8A8_UINT ] =         { SVGA3D_R8G8B8A8_UINT,       SVGA3D_R8G8B8A8_UINT,        SVGA3D_R8G8B8A8_UINT,        TF_UAV },
   [ PIPE_FORMAT_R8_SINT ] =               { SVGA3D_R8_SINT,             SVGA3D_R8_SINT,              SVGA3D_R8_SINT,              TF_UAV },
   [ PIPE_FORMAT_R8G8_SINT ] =             { SVGA3D_R8G8_SINT,           SVGA3D_R8G8_SINT,            SVGA3D_R8G8_SINT,            TF_UAV },
   [ PIPE_FORMAT_R8G8B8_SINT ] =           { SVGA3D_R8G8B8A8_SINT,       SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_W_TO_1 },
   [ PIPE_FORMAT_R8G8B8A8_SINT ] =         { SVGA3D_R8G8B8A8_SINT,       SVGA3D_R8G8B8A8_SINT,        SVGA3D_R8G8B8A8_SINT,        TF_UAV },
   [ PIPE_FORMAT_R16_UINT ] =              { SVGA3D_R16_UINT,            SVGA3D_R16_UINT,             SVGA3D_R16_UINT,             TF_UAV },
   [ PIPE_FORMAT_R16G16_UINT ] =           { SVGA3D_R16G16_UINT,         SVGA3D_R16G16_UINT,          SVGA3D_R16G16_UINT,          TF_UAV },
   [ PIPE_FORMAT_R16G16B16_UINT ] =        { SVGA3D_R16G16B16A16_UINT,   SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_W_TO_1 },
   [ PIPE_FORMAT_R16G16B16A16_UINT ] =     { SVGA3D_R16G16B16A16_UINT,   SVGA3D_R16G16B16A16_UINT,    SVGA3D_R16G16B16A16_UINT,    TF_UAV },
   [ PIPE_FORMAT_R16_SINT ] =              { SVGA3D_R16_SINT,            SVGA3D_R16_SINT,             SVGA3D_R16_SINT,             TF_UAV },
   [ PIPE_FORMAT_R16G16_SINT ] =           { SVGA3D_R16G16_SINT,         SVGA3D_R16G16_SINT,          SVGA3D_R16G16_SINT,          TF_UAV },
   [ PIPE_FORMAT_R16G16B16_SINT ] =        { SVGA3D_R16G16B16A16_SINT,   SVGA3D_FORMAT_INVALID,       SVGA3D_FORMAT_INVALID,       VF_W_TO_1 },
   [ PIPE_FORMAT_R16G16B16A16_SINT ] =     { SVGA3D_R16G16B16A16_SINT,   SVGA3D_R16G16B16A16_SINT,    SVGA3D_R16G16B16A16_SINT,    TF_UAV },
   [ PIPE_FORMAT_R32_UINT ] =              { SVGA3D_R32_UINT,            SVGA3D_R32_UINT,             SVGA3D_R32_UINT,             TF_UAV },
   [ PIPE_FORMAT_R32G32_UINT ] =           { SVGA3D_R32G32_UINT,         SVGA3D_R32G32_UINT,          SVGA3D_R32G32_UINT,          TF_UAV },
   [ PIPE_FORMAT_R32G32B32_UINT ] =        { SVGA3D_R32G32B32_UINT,      SVGA3D_R32G32B32_UINT,       SVGA3D_R32G32B32_UINT,       0 },
   [ PIPE_FORMAT_R32G32B32A32_UINT ] =     { SVGA3D_R32G32B32A32_UINT,   SVGA3D_R32G32B32A32_UINT,    SVGA3D_R32G32B32A32_UINT,    TF_UAV },
   [ PIPE_FORMAT_R32_SINT ] =              { SVGA3D_R32_SINT,            SVGA3D_R32_SINT,             SVGA3D_R32_SINT,             TF_UAV },
   [ PIPE_FORMAT_R32G32_SINT ] =           { SVGA3D_R32G32_SINT,         SVGA3D_R32G32_SINT,          SVGA3D_R32G32_SINT,          TF_UAV },
   [ PIPE_FORMAT_R32G32B32_SINT ] =        { SVGA3D_R32G32B32_SINT,      SVGA3D_R32G32B32_SINT,       SVGA3D_R32G32B32_SINT,       0 },
   [ PIPE_FORMAT_R32G32B32A32_SINT ] =     { SVGA3D_R32G32B32A32_SINT,   SVGA3D_R32G32B32A32_SINT,    SVGA3D_R32G32B32A32_SINT,    TF_UAV },
   [ PIPE_FORMAT_A8_UINT ] =               { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R8_UINT,              TF_000X },
   [ PIPE_FORMAT_I8_UINT ] =               { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R8_UINT,              TF_XXXX },
   [ PIPE_FORMAT_L8_UINT ] =               { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R8_UINT,              TF_XXX1 },
   [ PIPE_FORMAT_L8A8_UINT ] =             { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R8G8_UINT,            TF_XXXY },
   [ PIPE_FORMAT_A8_SINT ] =               { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R8_SINT,              TF_000X },
   [ PIPE_FORMAT_I8_SINT ] =               { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R8_SINT,              TF_XXXX },
   [ PIPE_FORMAT_L8_SINT ] =               { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R8_SINT,              TF_XXX1 },
   [ PIPE_FORMAT_L8A8_SINT ] =             { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R8G8_SINT,            TF_XXXY },
   [ PIPE_FORMAT_A16_UINT ] =              { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R16_UINT,             TF_000X },
   [ PIPE_FORMAT_I16_UINT ] =              { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R16_UINT,             TF_XXXX },
   [ PIPE_FORMAT_L16_UINT ] =              { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R16_UINT,             TF_XXX1 },
   [ PIPE_FORMAT_L16A16_UINT ] =           { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R16G16_UINT,          TF_XXXY },
   [ PIPE_FORMAT_A16_SINT ] =              { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R16_SINT,             TF_000X },
   [ PIPE_FORMAT_I16_SINT ] =              { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R16_SINT,             TF_XXXX },
   [ PIPE_FORMAT_L16_SINT ] =              { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R16_SINT,             TF_XXX1 },
   [ PIPE_FORMAT_L16A16_SINT ] =           { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R16G16_SINT,          TF_XXXY },
   [ PIPE_FORMAT_A32_UINT ] =              { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R32_UINT,             TF_000X },
   [ PIPE_FORMAT_I32_UINT ] =              { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R32_UINT,             TF_XXXX },
   [ PIPE_FORMAT_L32_UINT ] =              { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R32_UINT,             TF_XXX1 },
   [ PIPE_FORMAT_L32A32_UINT ] =           { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R32G32_UINT,          TF_XXXY },
   [ PIPE_FORMAT_A32_SINT ] =              { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R32_SINT,             TF_000X },
   [ PIPE_FORMAT_I32_SINT ] =              { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R32_SINT,             TF_XXXX },
   [ PIPE_FORMAT_L32_SINT ] =              { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R32_SINT,             TF_XXX1 },
   [ PIPE_FORMAT_L32A32_SINT ] =           { SVGA3D_FORMAT_INVALID,      SVGA3D_FORMAT_INVALID,       SVGA3D_R32G32_SINT,          TF_XXXY },
   [ PIPE_FORMAT_R10G10B10A2_UINT ] =      { SVGA3D_R10G10B10A2_UINT,    SVGA3D_R10G10B10A2_UINT,     SVGA3D_R10G10B10A2_UINT,     TF_UAV },
   [ PIPE_FORMAT_BPTC_RGBA_UNORM ] =       { SVGA3D_FORMAT_INVALID,      SVGA3D_BC7_UNORM,            SVGA3D_FORMAT_INVALID,       TF_SM5 },
   [ PIPE_FORMAT_BPTC_SRGBA ] =            { SVGA3D_FORMAT_INVALID,      SVGA3D_BC7_UNORM_SRGB,       SVGA3D_FORMAT_INVALID,       TF_SM5 },
   [ PIPE_FORMAT_BPTC_RGB_FLOAT ] =        { SVGA3D_FORMAT_INVALID,      SVGA3D_BC6H_SF16,            SVGA3D_FORMAT_INVALID,       TF_SM5 },
   [ PIPE_FORMAT_BPTC_RGB_UFLOAT ] =       { SVGA3D_FORMAT_INVALID,      SVGA3D_BC6H_UF16,            SVGA3D_FORMAT_INVALID,       TF_SM5 },
   [ PIPE_FORMAT_X24S8_UINT ] =            { SVGA3D_FORMAT_INVALID,      SVGA3D_X24_G8_UINT,          SVGA3D_FORMAT_INVALID,       0 },
   [ PIPE_FORMAT_X32_S8X24_UINT ] =        { SVGA3D_FORMAT_INVALID,      SVGA3D_X32_G8X24_UINT,       SVGA3D_FORMAT_INVALID,       0 },
   /* Must specify following entry to give the sense of size of format_conversion_table[] */
   [ PIPE_FORMAT_COUNT ] = {SVGA3D_FORMAT_INVALID, SVGA3D_FORMAT_INVALID,    SVGA3D_FORMAT_INVALID,       0 },
};


static const struct vgpu10_format_entry *
svga_format_entry(enum pipe_format format)
{
   /* Sparse filling of the table requires this. */
   STATIC_ASSERT(SVGA3D_FORMAT_INVALID == 0);
   assert(format < ARRAY_SIZE(format_conversion_table));
   if (format >= ARRAY_SIZE(format_conversion_table))
      return &format_conversion_table[PIPE_FORMAT_NONE];
   else
      return &format_conversion_table[format];
}

/**
 * Translate a gallium vertex format to a vgpu10 vertex format.
 * Also, return any special vertex format flags.
 */
void
svga_translate_vertex_format_vgpu10(enum pipe_format format,
                                    SVGA3dSurfaceFormat *svga_format,
                                    unsigned *vf_flags)
{
   const struct vgpu10_format_entry *entry = svga_format_entry(format);

   *svga_format = entry->vertex_format;
   *vf_flags = entry->flags;
}


/**
 * Translate a gallium pixel format to a vgpu10 format
 * to be used in a shader resource view for a texture buffer.
 * Also return any special texture format flags such as
 * any special swizzle mask.
 */
void
svga_translate_texture_buffer_view_format(enum pipe_format format,
                                          SVGA3dSurfaceFormat *svga_format,
                                          unsigned *tf_flags)
{
   const struct vgpu10_format_entry *entry = svga_format_entry(format);

   *svga_format = entry->view_format;
   *tf_flags = entry->flags;
}


/**
 * Translate a gallium scanout format to a svga format valid
 * for screen target surface.
 */
static SVGA3dSurfaceFormat
svga_translate_screen_target_format_vgpu10(enum pipe_format format)
{
   switch (format) {
   case PIPE_FORMAT_B8G8R8A8_UNORM:
      return SVGA3D_B8G8R8A8_UNORM;
   case PIPE_FORMAT_B8G8R8X8_UNORM:
      return SVGA3D_B8G8R8X8_UNORM;
   case PIPE_FORMAT_B5G6R5_UNORM:
      return SVGA3D_R5G6B5;
   case PIPE_FORMAT_B5G5R5A1_UNORM:
      return SVGA3D_A1R5G5B5;
   default:
      debug_printf("Invalid format %s specified for screen target\n",
                   svga_format_name(format));
      return SVGA3D_FORMAT_INVALID;
   }
}

/*
 * Translate from gallium format to SVGA3D format.
 */
SVGA3dSurfaceFormat
svga_translate_format(const struct svga_screen *ss,
                      enum pipe_format format,
                      unsigned bind)
{
   const struct vgpu10_format_entry *entry = svga_format_entry(format);

   if (ss->sws->have_vgpu10) {
      if (bind & (PIPE_BIND_VERTEX_BUFFER | PIPE_BIND_INDEX_BUFFER)) {
         return entry->vertex_format;
      }
      else if (bind & PIPE_BIND_SCANOUT) {
         return svga_translate_screen_target_format_vgpu10(format);
      }
      else if (bind & PIPE_BIND_SHADER_IMAGE) {
         if (format_conversion_table[format].flags & TF_UAV)
            return format_conversion_table[format].pixel_format;
         else
            return SVGA3D_FORMAT_INVALID;
      }
      else {
         if ((format_conversion_table[format].flags & TF_SM5) &&
             !ss->sws->have_sm5)
            return SVGA3D_FORMAT_INVALID;
         else
            return entry->pixel_format;
      }
   }

   switch(format) {
   case PIPE_FORMAT_B8G8R8A8_UNORM:
      return SVGA3D_A8R8G8B8;
   case PIPE_FORMAT_B8G8R8X8_UNORM:
      return SVGA3D_X8R8G8B8;

   /* sRGB required for GL2.1 */
   case PIPE_FORMAT_B8G8R8A8_SRGB:
      return SVGA3D_A8R8G8B8;
   case PIPE_FORMAT_DXT1_SRGB:
   case PIPE_FORMAT_DXT1_SRGBA:
      return SVGA3D_DXT1;
   case PIPE_FORMAT_DXT3_SRGBA:
      return SVGA3D_DXT3;
   case PIPE_FORMAT_DXT5_SRGBA:
      return SVGA3D_DXT5;

   case PIPE_FORMAT_B5G6R5_UNORM:
      return SVGA3D_R5G6B5;
   case PIPE_FORMAT_B5G5R5A1_UNORM:
      return SVGA3D_A1R5G5B5;
   case PIPE_FORMAT_B4G4R4A4_UNORM:
      return SVGA3D_A4R4G4B4;

   case PIPE_FORMAT_R16G16B16A16_UNORM:
      return SVGA3D_A16B16G16R16;

   case PIPE_FORMAT_Z16_UNORM:
      assert(!ss->sws->have_vgpu10);
      return bind & PIPE_BIND_SAMPLER_VIEW ? ss->depth.z16 : SVGA3D_Z_D16;
   case PIPE_FORMAT_S8_UINT_Z24_UNORM:
      assert(!ss->sws->have_vgpu10);
      return bind & PIPE_BIND_SAMPLER_VIEW ? ss->depth.s8z24 : SVGA3D_Z_D24S8;
   case PIPE_FORMAT_X8Z24_UNORM:
      assert(!ss->sws->have_vgpu10);
      return bind & PIPE_BIND_SAMPLER_VIEW ? ss->depth.x8z24 : SVGA3D_Z_D24X8;

   case PIPE_FORMAT_A8_UNORM:
      return SVGA3D_ALPHA8;
   case PIPE_FORMAT_L8_UNORM:
      return SVGA3D_LUMINANCE8;

   case PIPE_FORMAT_DXT1_RGB:
   case PIPE_FORMAT_DXT1_RGBA:
      return SVGA3D_DXT1;
   case PIPE_FORMAT_DXT3_RGBA:
      return SVGA3D_DXT3;
   case PIPE_FORMAT_DXT5_RGBA:
      return SVGA3D_DXT5;

   /* Float formats (only 1, 2 and 4-component formats supported) */
   case PIPE_FORMAT_R32_FLOAT:
      return SVGA3D_R_S23E8;
   case PIPE_FORMAT_R32G32_FLOAT:
      return SVGA3D_RG_S23E8;
   case PIPE_FORMAT_R32G32B32A32_FLOAT:
      return SVGA3D_ARGB_S23E8;
   case PIPE_FORMAT_R16_FLOAT:
      return SVGA3D_R_S10E5;
   case PIPE_FORMAT_R16G16_FLOAT:
      return SVGA3D_RG_S10E5;
   case PIPE_FORMAT_R16G16B16A16_FLOAT:
      return SVGA3D_ARGB_S10E5;

   case PIPE_FORMAT_Z32_UNORM:
      /* SVGA3D_Z_D32 is not yet unsupported */
      FALLTHROUGH;
   default:
      return SVGA3D_FORMAT_INVALID;
   }
}


/*
 * Format capability description entry.
 */
struct format_cap {
   const char *name;

   SVGA3dSurfaceFormat format;

   /*
    * Capability index corresponding to the format.
    */
   SVGA3dDevCapIndex devcap;

   /* size of each pixel/block */
   unsigned block_width, block_height, block_bytes;

   /*
    * Mask of supported SVGA3dFormatOp operations, to be inferred when the
    * capability is not explicitly present.
    */
   uint32 defaultOperations;
};


/*
 * Format capability description table.
 *
 * Ordered by increasing SVGA3dSurfaceFormat value, but with gaps.
 */
static const struct format_cap format_cap_table[] = {
   {
      "SVGA3D_FORMAT_INVALID",
      SVGA3D_FORMAT_INVALID, 0, 0, 0, 0, 0
   },
   {
      "SVGA3D_X8R8G8B8",
      SVGA3D_X8R8G8B8,
      SVGA3D_DEVCAP_SURFACEFMT_X8R8G8B8,
      1, 1, 4,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE |
      SVGA3DFORMAT_OP_VOLUMETEXTURE |
      SVGA3DFORMAT_OP_DISPLAYMODE |
      SVGA3DFORMAT_OP_OFFSCREEN_RENDERTARGET
   },
   {
      "SVGA3D_A8R8G8B8",
      SVGA3D_A8R8G8B8,
      SVGA3D_DEVCAP_SURFACEFMT_A8R8G8B8,
      1, 1, 4,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE |
      SVGA3DFORMAT_OP_VOLUMETEXTURE |
      SVGA3DFORMAT_OP_OFFSCREEN_RENDERTARGET
   },
   {
      "SVGA3D_R5G6B5",
      SVGA3D_R5G6B5,
      SVGA3D_DEVCAP_SURFACEFMT_R5G6B5,
      1, 1, 2,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE |
      SVGA3DFORMAT_OP_VOLUMETEXTURE |
      SVGA3DFORMAT_OP_DISPLAYMODE |
      SVGA3DFORMAT_OP_OFFSCREEN_RENDERTARGET
   },
   {
      "SVGA3D_X1R5G5B5",
      SVGA3D_X1R5G5B5,
      SVGA3D_DEVCAP_SURFACEFMT_X1R5G5B5,
      1, 1, 2,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE |
      SVGA3DFORMAT_OP_VOLUMETEXTURE |
      SVGA3DFORMAT_OP_OFFSCREEN_RENDERTARGET
   },
   {
      "SVGA3D_A1R5G5B5",
      SVGA3D_A1R5G5B5,
      SVGA3D_DEVCAP_SURFACEFMT_A1R5G5B5,
      1, 1, 2,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE |
      SVGA3DFORMAT_OP_VOLUMETEXTURE |
      SVGA3DFORMAT_OP_OFFSCREEN_RENDERTARGET
   },
   {
      "SVGA3D_A4R4G4B4",
      SVGA3D_A4R4G4B4,
      SVGA3D_DEVCAP_SURFACEFMT_A4R4G4B4,
      1, 1, 2,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE |
      SVGA3DFORMAT_OP_VOLUMETEXTURE |
      SVGA3DFORMAT_OP_OFFSCREEN_RENDERTARGET
   },
   {
      /*
       * SVGA3D_Z_D32 is not yet supported, and has no corresponding
       * SVGA3D_DEVCAP_xxx.
       */
      "SVGA3D_Z_D32",
      SVGA3D_Z_D32, 0, 0, 0, 0, 0
   },
   {
      "SVGA3D_Z_D16",
      SVGA3D_Z_D16,
      SVGA3D_DEVCAP_SURFACEFMT_Z_D16,
      1, 1, 2,
      SVGA3DFORMAT_OP_ZSTENCIL
   },
   {
      "SVGA3D_Z_D24S8",
      SVGA3D_Z_D24S8,
      SVGA3D_DEVCAP_SURFACEFMT_Z_D24S8,
      1, 1, 4,
      SVGA3DFORMAT_OP_ZSTENCIL
   },
   {
      "SVGA3D_Z_D15S1",
      SVGA3D_Z_D15S1,
      SVGA3D_DEVCAP_MAX,
      1, 1, 2,
      SVGA3DFORMAT_OP_ZSTENCIL
   },
   {
      "SVGA3D_LUMINANCE8",
      SVGA3D_LUMINANCE8,
      SVGA3D_DEVCAP_SURFACEFMT_LUMINANCE8,
      1, 1, 1,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE |
      SVGA3DFORMAT_OP_VOLUMETEXTURE
   },
   {
      /*
       * SVGA3D_LUMINANCE4_ALPHA4 is not supported, and has no corresponding
       * SVGA3D_DEVCAP_xxx.
       */
      "SVGA3D_LUMINANCE4_ALPHA4",
      SVGA3D_LUMINANCE4_ALPHA4, 0, 0, 0, 0, 0
   },
   {
      "SVGA3D_LUMINANCE16",
      SVGA3D_LUMINANCE16,
      SVGA3D_DEVCAP_SURFACEFMT_LUMINANCE16,
      1, 1, 2,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE |
      SVGA3DFORMAT_OP_VOLUMETEXTURE
   },
   {
      "SVGA3D_LUMINANCE8_ALPHA8",
      SVGA3D_LUMINANCE8_ALPHA8,
      SVGA3D_DEVCAP_SURFACEFMT_LUMINANCE8_ALPHA8,
      1, 1, 2,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE |
      SVGA3DFORMAT_OP_VOLUMETEXTURE
   },
   {
      "SVGA3D_DXT1",
      SVGA3D_DXT1,
      SVGA3D_DEVCAP_SURFACEFMT_DXT1,
      4, 4, 8,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE
   },
   {
      "SVGA3D_DXT2",
      SVGA3D_DXT2,
      SVGA3D_DEVCAP_SURFACEFMT_DXT2,
      4, 4, 8,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE
   },
   {
      "SVGA3D_DXT3",
      SVGA3D_DXT3,
      SVGA3D_DEVCAP_SURFACEFMT_DXT3,
      4, 4, 16,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE
   },
   {
      "SVGA3D_DXT4",
      SVGA3D_DXT4,
      SVGA3D_DEVCAP_SURFACEFMT_DXT4,
      4, 4, 16,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE
   },
   {
      "SVGA3D_DXT5",
      SVGA3D_DXT5,
      SVGA3D_DEVCAP_SURFACEFMT_DXT5,
      4, 4, 8,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE
   },
   {
      "SVGA3D_BUMPU8V8",
      SVGA3D_BUMPU8V8,
      SVGA3D_DEVCAP_SURFACEFMT_BUMPU8V8,
      1, 1, 2,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE |
      SVGA3DFORMAT_OP_VOLUMETEXTURE
   },
   {
      /*
       * SVGA3D_BUMPL6V5U5 is unsupported; it has no corresponding
       * SVGA3D_DEVCAP_xxx.
       */
      "SVGA3D_BUMPL6V5U5",
      SVGA3D_BUMPL6V5U5, 0, 0, 0, 0, 0
   },
   {
      "SVGA3D_BUMPX8L8V8U8",
      SVGA3D_BUMPX8L8V8U8,
      SVGA3D_DEVCAP_SURFACEFMT_BUMPX8L8V8U8,
      1, 1, 4,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE
   },
   {
      "SVGA3D_FORMAT_DEAD1",
      SVGA3D_FORMAT_DEAD1, 0, 0, 0, 0, 0
   },
   {
      "SVGA3D_ARGB_S10E5",
      SVGA3D_ARGB_S10E5,
      SVGA3D_DEVCAP_SURFACEFMT_ARGB_S10E5,
      1, 1, 2,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE |
      SVGA3DFORMAT_OP_VOLUMETEXTURE |
      SVGA3DFORMAT_OP_OFFSCREEN_RENDERTARGET
   },
   {
      "SVGA3D_ARGB_S23E8",
      SVGA3D_ARGB_S23E8,
      SVGA3D_DEVCAP_SURFACEFMT_ARGB_S23E8,
      1, 1, 4,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE |
      SVGA3DFORMAT_OP_VOLUMETEXTURE |
      SVGA3DFORMAT_OP_OFFSCREEN_RENDERTARGET
   },
   {
      "SVGA3D_A2R10G10B10",
      SVGA3D_A2R10G10B10,
      SVGA3D_DEVCAP_SURFACEFMT_A2R10G10B10,
      1, 1, 4,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE |
      SVGA3DFORMAT_OP_VOLUMETEXTURE |
      SVGA3DFORMAT_OP_OFFSCREEN_RENDERTARGET
   },
   {
      /*
       * SVGA3D_V8U8 is unsupported; it has no corresponding
       * SVGA3D_DEVCAP_xxx. SVGA3D_BUMPU8V8 should be used instead.
       */
      "SVGA3D_V8U8",
      SVGA3D_V8U8, 0, 0, 0, 0, 0
   },
   {
      "SVGA3D_Q8W8V8U8",
      SVGA3D_Q8W8V8U8,
      SVGA3D_DEVCAP_SURFACEFMT_Q8W8V8U8,
      1, 1, 4,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE
   },
   {
      "SVGA3D_CxV8U8",
      SVGA3D_CxV8U8,
      SVGA3D_DEVCAP_SURFACEFMT_CxV8U8,
      1, 1, 2,
      SVGA3DFORMAT_OP_TEXTURE
   },
   {
      /*
       * SVGA3D_X8L8V8U8 is unsupported; it has no corresponding
       * SVGA3D_DEVCAP_xxx. SVGA3D_BUMPX8L8V8U8 should be used instead.
       */
      "SVGA3D_X8L8V8U8",
      SVGA3D_X8L8V8U8, 0, 0, 0, 0, 0
   },
   {
      "SVGA3D_A2W10V10U10",
      SVGA3D_A2W10V10U10,
      SVGA3D_DEVCAP_SURFACEFMT_A2W10V10U10,
      1, 1, 4,
      SVGA3DFORMAT_OP_TEXTURE
   },
   {
      "SVGA3D_ALPHA8",
      SVGA3D_ALPHA8,
      SVGA3D_DEVCAP_SURFACEFMT_ALPHA8,
      1, 1, 1,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE |
      SVGA3DFORMAT_OP_VOLUMETEXTURE
   },
   {
      "SVGA3D_R_S10E5",
      SVGA3D_R_S10E5,
      SVGA3D_DEVCAP_SURFACEFMT_R_S10E5,
      1, 1, 2,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_VOLUMETEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE |
      SVGA3DFORMAT_OP_OFFSCREEN_RENDERTARGET
   },
   {
      "SVGA3D_R_S23E8",
      SVGA3D_R_S23E8,
      SVGA3D_DEVCAP_SURFACEFMT_R_S23E8,
      1, 1, 4,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_VOLUMETEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE |
      SVGA3DFORMAT_OP_OFFSCREEN_RENDERTARGET
   },
   {
      "SVGA3D_RG_S10E5",
      SVGA3D_RG_S10E5,
      SVGA3D_DEVCAP_SURFACEFMT_RG_S10E5,
      1, 1, 2,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_VOLUMETEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE |
      SVGA3DFORMAT_OP_OFFSCREEN_RENDERTARGET
   },
   {
      "SVGA3D_RG_S23E8",
      SVGA3D_RG_S23E8,
      SVGA3D_DEVCAP_SURFACEFMT_RG_S23E8,
      1, 1, 4,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_VOLUMETEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE |
      SVGA3DFORMAT_OP_OFFSCREEN_RENDERTARGET
   },
   {
      /*
       * SVGA3D_BUFFER is a placeholder format for index/vertex buffers.
       */
      "SVGA3D_BUFFER",
      SVGA3D_BUFFER, 0, 1, 1, 1, 0
   },
   {
      "SVGA3D_Z_D24X8",
      SVGA3D_Z_D24X8,
      SVGA3D_DEVCAP_SURFACEFMT_Z_D24X8,
      1, 1, 4,
      SVGA3DFORMAT_OP_ZSTENCIL
   },
   {
      "SVGA3D_V16U16",
      SVGA3D_V16U16,
      SVGA3D_DEVCAP_SURFACEFMT_V16U16,
      1, 1, 4,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE |
      SVGA3DFORMAT_OP_VOLUMETEXTURE
   },
   {
      "SVGA3D_G16R16",
      SVGA3D_G16R16,
      SVGA3D_DEVCAP_SURFACEFMT_G16R16,
      1, 1, 4,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE |
      SVGA3DFORMAT_OP_VOLUMETEXTURE |
      SVGA3DFORMAT_OP_OFFSCREEN_RENDERTARGET
   },
   {
      "SVGA3D_A16B16G16R16",
      SVGA3D_A16B16G16R16,
      SVGA3D_DEVCAP_SURFACEFMT_A16B16G16R16,
      1, 1, 8,
      SVGA3DFORMAT_OP_TEXTURE |
      SVGA3DFORMAT_OP_CUBETEXTURE |
      SVGA3DFORMAT_OP_VOLUMETEXTURE |
      SVGA3DFORMAT_OP_OFFSCREEN_RENDERTARGET
   },
   {
      "SVGA3D_UYVY",
      SVGA3D_UYVY,
      SVGA3D_DEVCAP_SURFACEFMT_UYVY,
      0, 0, 0, 0
   },
   {
      "SVGA3D_YUY2",
      SVGA3D_YUY2,
      SVGA3D_DEVCAP_SURFACEFMT_YUY2,
      0, 0, 0, 0
   },
   {
      "SVGA3D_NV12",
      SVGA3D_NV12,
      SVGA3D_DEVCAP_SURFACEFMT_NV12,
      0, 0, 0, 0
   },
   {
      "SVGA3D_FORMAT_DEAD2",
      SVGA3D_FORMAT_DEAD2, 0, 0, 0, 0, 0
   },
   {
      "SVGA3D_R32G32B32A32_TYPELESS",
      SVGA3D_R32G32B32A32_TYPELESS,
      SVGA3D_DEVCAP_DXFMT_R32G32B32A32_TYPELESS,
      1, 1, 16, 0
   },
   {
      "SVGA3D_R32G32B32A32_UINT",
      SVGA3D_R32G32B32A32_UINT,
      SVGA3D_DEVCAP_DXFMT_R32G32B32A32_UINT,
      1, 1, 16, 0
   },
   {
      "SVGA3D_R32G32B32A32_SINT",
      SVGA3D_R32G32B32A32_SINT,
      SVGA3D_DEVCAP_DXFMT_R32G32B32A32_SINT,
      1, 1, 16, 0
   },
   {
      "SVGA3D_R32G32B32_TYPELESS",
      SVGA3D_R32G32B32_TYPELESS,
      SVGA3D_DEVCAP_DXFMT_R32G32B32_TYPELESS,
      1, 1, 12, 0
   },
   {
      "SVGA3D_R32G32B32_FLOAT",
      SVGA3D_R32G32B32_FLOAT,
      SVGA3D_DEVCAP_DXFMT_R32G32B32_FLOAT,
      1, 1, 12, 0
   },
   {
      "SVGA3D_R32G32B32_UINT",
      SVGA3D_R32G32B32_UINT,
      SVGA3D_DEVCAP_DXFMT_R32G32B32_UINT,
      1, 1, 12, 0
   },
   {
      "SVGA3D_R32G32B32_SINT",
      SVGA3D_R32G32B32_SINT,
      SVGA3D_DEVCAP_DXFMT_R32G32B32_SINT,
      1, 1, 12, 0
   },
   {
      "SVGA3D_R16G16B16A16_TYPELESS",
      SVGA3D_R16G16B16A16_TYPELESS,
      SVGA3D_DEVCAP_DXFMT_R16G16B16A16_TYPELESS,
      1, 1, 8, 0
   },
   {
      "SVGA3D_R16G16B16A16_UINT",
      SVGA3D_R16G16B16A16_UINT,
      SVGA3D_DEVCAP_DXFMT_R16G16B16A16_UINT,
      1, 1, 8, 0
   },
   {
      "SVGA3D_R16G16B16A16_SNORM",
      SVGA3D_R16G16B16A16_SNORM,
      SVGA3D_DEVCAP_DXFMT_R16G16B16A16_SNORM,
      1, 1, 8, 0
   },
   {
      "SVGA3D_R16G16B16A16_SINT",
      SVGA3D_R16G16B16A16_SINT,
      SVGA3D_DEVCAP_DXFMT_R16G16B16A16_SINT,
      1, 1, 8, 0
   },
   {
      "SVGA3D_R32G32_TYPELESS",
      SVGA3D_R32G32_TYPELESS,
      SVGA3D_DEVCAP_DXFMT_R32G32_TYPELESS,
      1, 1, 8, 0
   },
   {
      "SVGA3D_R32G32_UINT",
      SVGA3D_R32G32_UINT,
      SVGA3D_DEVCAP_DXFMT_R32G32_UINT,
      1, 1, 8, 0
   },
   {
      "SVGA3D_R32G32_SINT",
      SVGA3D_R32G32_SINT,
      SVGA3D_DEVCAP_DXFMT_R32G32_SINT,
      1, 1, 8,
      0
   },
   {
      "SVGA3D_R32G8X24_TYPELESS",
      SVGA3D_R32G8X24_TYPELESS,
      SVGA3D_DEVCAP_DXFMT_R32G8X24_TYPELESS,
      1, 1, 8, 0
   },
   {
      "SVGA3D_D32_FLOAT_S8X24_UINT",
      SVGA3D_D32_FLOAT_S8X24_UINT,
      SVGA3D_DEVCAP_DXFMT_D32_FLOAT_S8X24_UINT,
      1, 1, 8, 0
   },
   {
      "SVGA3D_R32_FLOAT_X8X24",
      SVGA3D_R32_FLOAT_X8X24,
      SVGA3D_DEVCAP_DXFMT_R32_FLOAT_X8X24,
      1, 1, 8, 0
   },
   {
      "SVGA3D_X32_G8X24_UINT",
      SVGA3D_X32_G8X24_UINT,
      SVGA3D_DEVCAP_DXFMT_X32_G8X24_UINT,
      1, 1, 4, 0
   },
   {
      "SVGA3D_R10G10B10A2_TYPELESS",
      SVGA3D_R10G10B10A2_TYPELESS,
      SVGA3D_DEVCAP_DXFMT_R10G10B10A2_TYPELESS,
      1, 1, 4, 0
   },
   {
      "SVGA3D_R10G10B10A2_UINT",
      SVGA3D_R10G10B10A2_UINT,
      SVGA3D_DEVCAP_DXFMT_R10G10B10A2_UINT,
      1, 1, 4, 0
   },
   {
      "SVGA3D_R11G11B10_FLOAT",
      SVGA3D_R11G11B10_FLOAT,
      SVGA3D_DEVCAP_DXFMT_R11G11B10_FLOAT,
      1, 1, 4, 0
   },
   {
      "SVGA3D_R8G8B8A8_TYPELESS",
      SVGA3D_R8G8B8A8_TYPELESS,
      SVGA3D_DEVCAP_DXFMT_R8G8B8A8_TYPELESS,
      1, 1, 4, 0
   },
   {
      "SVGA3D_R8G8B8A8_UNORM",
      SVGA3D_R8G8B8A8_UNORM,
      SVGA3D_DEVCAP_DXFMT_R8G8B8A8_UNORM,
      1, 1, 4, 0
   },
   {
      "SVGA3D_R8G8B8A8_UNORM_SRGB",
      SVGA3D_R8G8B8A8_UNORM_SRGB,
      SVGA3D_DEVCAP_DXFMT_R8G8B8A8_UNORM_SRGB,
      1, 1, 4, 0
   },
   {
      "SVGA3D_R8G8B8A8_UINT",
      SVGA3D_R8G8B8A8_UINT,
      SVGA3D_DEVCAP_DXFMT_R8G8B8A8_UINT,
      1, 1, 4, 0
      },
   {
      "SVGA3D_R8G8B8A8_SINT",
      SVGA3D_R8G8B8A8_SINT,
      SVGA3D_DEVCAP_DXFMT_R8G8B8A8_SINT,
      1, 1, 4, 0
   },
   {
      "SVGA3D_R16G16_TYPELESS",
      SVGA3D_R16G16_TYPELESS,
      SVGA3D_DEVCAP_DXFMT_R16G16_TYPELESS,
      1, 1, 4, 0
   },
   {
      "SVGA3D_R16G16_UINT",
      SVGA3D_R16G16_UINT,
      SVGA3D_DEVCAP_DXFMT_R16G16_UINT,
      1, 1, 4, 0
   },
   {
      "SVGA3D_R16G16_SINT",
      SVGA3D_R16G16_SINT,
      SVGA3D_DEVCAP_DXFMT_R16G16_SINT,
      1, 1, 4, 0
   },
   {
      "SVGA3D_R32_TYPELESS",
      SVGA3D_R32_TYPELESS,
      SVGA3D_DEVCAP_DXFMT_R32_TYPELESS,
      1, 1, 4, 0
   },
   {
      "SVGA3D_D32_FLOAT",
      SVGA3D_D32_FLOAT,
      SVGA3D_DEVCAP_DXFMT_D32_FLOAT,
      1, 1, 4, 0
   },
   {
      "SVGA3D_R32_UINT",
      SVGA3D_R32_UINT,
      SVGA3D_DEVCAP_DXFMT_R32_UINT,
      1, 1, 4, 0
   },
   {
      "SVGA3D_R32_SINT",
      SVGA3D_R32_SINT,
      SVGA3D_DEVCAP_DXFMT_R32_SINT,
      1, 1, 4, 0
   },
   {
      "SVGA3D_R24G8_TYPELESS",
      SVGA3D_R24G8_TYPELESS,
      SVGA3D_DEVCAP_DXFMT_R24G8_TYPELESS,
      1, 1, 4, 0
   },
   {
      "SVGA3D_D24_UNORM_S8_UINT",
      SVGA3D_D24_UNORM_S8_UINT,
      SVGA3D_DEVCAP_DXFMT_D24_UNORM_S8_UINT,
      1, 1, 4, 0
   },
   {
      "SVGA3D_R24_UNORM_X8",
      SVGA3D_R24_UNORM_X8,
      SVGA3D_DEVCAP_DXFMT_R24_UNORM_X8,
      1, 1, 4, 0
   },
   {
      "SVGA3D_X24_G8_UINT",
      SVGA3D_X24_G8_UINT,
      SVGA3D_DEVCAP_DXFMT_X24_G8_UINT,
      1, 1, 4, 0
   },
   {
      "SVGA3D_R8G8_TYPELESS",
      SVGA3D_R8G8_TYPELESS,
      SVGA3D_DEVCAP_DXFMT_R8G8_TYPELESS,
      1, 1, 2, 0
   },
   {
      "SVGA3D_R8G8_UNORM",
      SVGA3D_R8G8_UNORM,
      SVGA3D_DEVCAP_DXFMT_R8G8_UNORM,
      1, 1, 2, 0
   },
   {
      "SVGA3D_R8G8_UINT",
      SVGA3D_R8G8_UINT,
      SVGA3D_DEVCAP_DXFMT_R8G8_UINT,
      1, 1, 2, 0
   },
   {
      "SVGA3D_R8G8_SINT",
      SVGA3D_R8G8_SINT,
      SVGA3D_DEVCAP_DXFMT_R8G8_SINT,
      1, 1, 2, 0
   },
   {
      "SVGA3D_R16_TYPELESS",
      SVGA3D_R16_TYPELESS,
      SVGA3D_DEVCAP_DXFMT_R16_TYPELESS,
      1, 1, 2, 0
   },
   {
      "SVGA3D_R16_UNORM",
      SVGA3D_R16_UNORM,
      SVGA3D_DEVCAP_DXFMT_R16_UNORM,
      1, 1, 2, 0
   },
   {
      "SVGA3D_R16_UINT",
      SVGA3D_R16_UINT,
      SVGA3D_DEVCAP_DXFMT_R16_UINT,
      1, 1, 2, 0
   },
   {
      "SVGA3D_R16_SNORM",
      SVGA3D_R16_SNORM,
      SVGA3D_DEVCAP_DXFMT_R16_SNORM,
      1, 1, 2, 0
   },
   {
      "SVGA3D_R16_SINT",
      SVGA3D_R16_SINT,
      SVGA3D_DEVCAP_DXFMT_R16_SINT,
      1, 1, 2, 0
   },
   {
      "SVGA3D_R8_TYPELESS",
      SVGA3D_R8_TYPELESS,
      SVGA3D_DEVCAP_DXFMT_R8_TYPELESS,
      1, 1, 1, 0
   },
   {
      "SVGA3D_R8_UNORM",
      SVGA3D_R8_UNORM,
      SVGA3D_DEVCAP_DXFMT_R8_UNORM,
      1, 1, 1, 0
   },
   {
      "SVGA3D_R8_UINT",
      SVGA3D_R8_UINT,
      SVGA3D_DEVCAP_DXFMT_R8_UINT,
      1, 1, 1, 0
   },
   {
      "SVGA3D_R8_SNORM",
      SVGA3D_R8_SNORM,
      SVGA3D_DEVCAP_DXFMT_R8_SNORM,
      1, 1, 1, 0
   },
   {
      "SVGA3D_R8_SINT",
      SVGA3D_R8_SINT,
      SVGA3D_DEVCAP_DXFMT_R8_SINT,
      1, 1, 1, 0
   },
   {
      "SVGA3D_P8",
      SVGA3D_P8, 0, 0, 0, 0, 0
   },
   {
      "SVGA3D_R9G9B9E5_SHAREDEXP",
      SVGA3D_R9G9B9E5_SHAREDEXP,
      SVGA3D_DEVCAP_DXFMT_R9G9B9E5_SHAREDEXP,
      1, 1, 4, 0
   },
   {
      "SVGA3D_R8G8_B8G8_UNORM",
      SVGA3D_R8G8_B8G8_UNORM, 0, 0, 0, 0, 0
   },
   {
      "SVGA3D_G8R8_G8B8_UNORM",
      SVGA3D_G8R8_G8B8_UNORM, 0, 0, 0, 0, 0
   },
   {
      "SVGA3D_BC1_TYPELESS",
      SVGA3D_BC1_TYPELESS,
      SVGA3D_DEVCAP_DXFMT_BC1_TYPELESS,
      4, 4, 8, 0
   },
   {
      "SVGA3D_BC1_UNORM_SRGB",
      SVGA3D_BC1_UNORM_SRGB,
      SVGA3D_DEVCAP_DXFMT_BC1_UNORM_SRGB,
      4, 4, 8, 0
   },
   {
      "SVGA3D_BC2_TYPELESS",
      SVGA3D_BC2_TYPELESS,
      SVGA3D_DEVCAP_DXFMT_BC2_TYPELESS,
      4, 4, 16, 0
   },
   {
      "SVGA3D_BC2_UNORM_SRGB",
      SVGA3D_BC2_UNORM_SRGB,
      SVGA3D_DEVCAP_DXFMT_BC2_UNORM_SRGB,
      4, 4, 16, 0
   },
   {
      "SVGA3D_BC3_TYPELESS",
      SVGA3D_BC3_TYPELESS,
      SVGA3D_DEVCAP_DXFMT_BC3_TYPELESS,
      4, 4, 16, 0
   },
   {
      "SVGA3D_BC3_UNORM_SRGB",
      SVGA3D_BC3_UNORM_SRGB,
      SVGA3D_DEVCAP_DXFMT_BC3_UNORM_SRGB,
      4, 4, 16, 0
   },
   {
      "SVGA3D_BC4_TYPELESS",
      SVGA3D_BC4_TYPELESS,
      SVGA3D_DEVCAP_DXFMT_BC4_TYPELESS,
      4, 4, 8, 0
   },
   {
      "SVGA3D_ATI1",
      SVGA3D_ATI1, 0, 0, 0, 0, 0
   },
   {
      "SVGA3D_BC4_SNORM",
      SVGA3D_BC4_SNORM,
      SVGA3D_DEVCAP_DXFMT_BC4_SNORM,
      4, 4, 8, 0
   },
   {
      "SVGA3D_BC5_TYPELESS",
      SVGA3D_BC5_TYPELESS,
      SVGA3D_DEVCAP_DXFMT_BC5_TYPELESS,
      4, 4, 16, 0
   },
   {
      "SVGA3D_ATI2",
      SVGA3D_ATI2, 0, 0, 0, 0, 0
   },
   {
      "SVGA3D_BC5_SNORM",
      SVGA3D_BC5_SNORM,
      SVGA3D_DEVCAP_DXFMT_BC5_SNORM,
      4, 4, 16, 0
   },
   {
      "SVGA3D_R10G10B10_XR_BIAS_A2_UNORM",
      SVGA3D_R10G10B10_XR_BIAS_A2_UNORM, 0, 0, 0, 0, 0
   },
   {
      "SVGA3D_B8G8R8A8_TYPELESS",
      SVGA3D_B8G8R8A8_TYPELESS,
      SVGA3D_DEVCAP_DXFMT_B8G8R8A8_TYPELESS,
      1, 1, 4, 0
   },
   {
      "SVGA3D_B8G8R8A8_UNORM_SRGB",
      SVGA3D_B8G8R8A8_UNORM_SRGB,
      SVGA3D_DEVCAP_DXFMT_B8G8R8A8_UNORM_SRGB,
      1, 1, 4, 0
   },
   {
      "SVGA3D_B8G8R8X8_TYPELESS",
      SVGA3D_B8G8R8X8_TYPELESS,
      SVGA3D_DEVCAP_DXFMT_B8G8R8X8_TYPELESS,
      1, 1, 4, 0
   },
   {
      "SVGA3D_B8G8R8X8_UNORM_SRGB",
      SVGA3D_B8G8R8X8_UNORM_SRGB,
      SVGA3D_DEVCAP_DXFMT_B8G8R8X8_UNORM_SRGB,
      1, 1, 4, 0
   },
   {
      "SVGA3D_Z_DF16",
      SVGA3D_Z_DF16,
      SVGA3D_DEVCAP_SURFACEFMT_Z_DF16,
      1, 1, 2, 0
   },
   {
      "SVGA3D_Z_DF24",
      SVGA3D_Z_DF24,
      SVGA3D_DEVCAP_SURFACEFMT_Z_DF24,
      1, 1, 4, 0
   },
   {
      "SVGA3D_Z_D24S8_INT",
      SVGA3D_Z_D24S8_INT,
      SVGA3D_DEVCAP_SURFACEFMT_Z_D24S8_INT,
      1, 1, 4, 0
   },
   {
      "SVGA3D_YV12",
      SVGA3D_YV12, 0, 0, 0, 0, 0
   },
   {
      "SVGA3D_R32G32B32A32_FLOAT",
      SVGA3D_R32G32B32A32_FLOAT,
      SVGA3D_DEVCAP_DXFMT_R32G32B32A32_FLOAT,
      1, 1, 16, 0
   },
   {
      "SVGA3D_R16G16B16A16_FLOAT",
      SVGA3D_R16G16B16A16_FLOAT,
      SVGA3D_DEVCAP_DXFMT_R16G16B16A16_FLOAT,
      1, 1, 8, 0
   },
   {
      "SVGA3D_R16G16B16A16_UNORM",
      SVGA3D_R16G16B16A16_UNORM,
      SVGA3D_DEVCAP_DXFMT_R16G16B16A16_UNORM,
      1, 1, 8, 0
   },
   {
      "SVGA3D_R32G32_FLOAT",
      SVGA3D_R32G32_FLOAT,
      SVGA3D_DEVCAP_DXFMT_R32G32_FLOAT,
      1, 1, 8, 0
   },
   {
      "SVGA3D_R10G10B10A2_UNORM",
      SVGA3D_R10G10B10A2_UNORM,
      SVGA3D_DEVCAP_DXFMT_R10G10B10A2_UNORM,
      1, 1, 4, 0
   },
   {
      "SVGA3D_R8G8B8A8_SNORM",
      SVGA3D_R8G8B8A8_SNORM,
      SVGA3D_DEVCAP_DXFMT_R8G8B8A8_SNORM,
      1, 1, 4, 0
   },
   {
      "SVGA3D_R16G16_FLOAT",
      SVGA3D_R16G16_FLOAT,
      SVGA3D_DEVCAP_DXFMT_R16G16_FLOAT,
      1, 1, 4, 0
   },
   {
      "SVGA3D_R16G16_UNORM",
      SVGA3D_R16G16_UNORM,
      SVGA3D_DEVCAP_DXFMT_R16G16_UNORM,
      1, 1, 4, 0
   },
   {
      "SVGA3D_R16G16_SNORM",
      SVGA3D_R16G16_SNORM,
      SVGA3D_DEVCAP_DXFMT_R16G16_SNORM,
      1, 1, 4, 0
   },
   {
      "SVGA3D_R32_FLOAT",
      SVGA3D_R32_FLOAT,
      SVGA3D_DEVCAP_DXFMT_R32_FLOAT,
      1, 1, 4, 0
   },
   {
      "SVGA3D_R8G8_SNORM",
      SVGA3D_R8G8_SNORM,
      SVGA3D_DEVCAP_DXFMT_R8G8_SNORM,
      1, 1, 2, 0
   },
   {
      "SVGA3D_R16_FLOAT",
      SVGA3D_R16_FLOAT,
      SVGA3D_DEVCAP_DXFMT_R16_FLOAT,
      1, 1, 2, 0
   },
   {
      "SVGA3D_D16_UNORM",
      SVGA3D_D16_UNORM,
      SVGA3D_DEVCAP_DXFMT_D16_UNORM,
      1, 1, 2, 0
   },
   {
      "SVGA3D_A8_UNORM",
      SVGA3D_A8_UNORM,
      SVGA3D_DEVCAP_DXFMT_A8_UNORM,
      1, 1, 1, 0
   },
   {
      "SVGA3D_BC1_UNORM",
      SVGA3D_BC1_UNORM,
      SVGA3D_DEVCAP_DXFMT_BC1_UNORM,
      4, 4, 8, 0
   },
   {
      "SVGA3D_BC2_UNORM",
      SVGA3D_BC2_UNORM,
      SVGA3D_DEVCAP_DXFMT_BC2_UNORM,
      4, 4, 16, 0
   },
   {
      "SVGA3D_BC3_UNORM",
      SVGA3D_BC3_UNORM,
      SVGA3D_DEVCAP_DXFMT_BC3_UNORM,
      4, 4, 16, 0
   },
   {
      "SVGA3D_B5G6R5_UNORM",
      SVGA3D_B5G6R5_UNORM,
      SVGA3D_DEVCAP_DXFMT_B5G6R5_UNORM,
      1, 1, 2, 0
   },
   {
      "SVGA3D_B5G5R5A1_UNORM",
      SVGA3D_B5G5R5A1_UNORM,
      SVGA3D_DEVCAP_DXFMT_B5G5R5A1_UNORM,
      1, 1, 2, 0
   },
   {
      "SVGA3D_B8G8R8A8_UNORM",
      SVGA3D_B8G8R8A8_UNORM,
      SVGA3D_DEVCAP_DXFMT_B8G8R8A8_UNORM,
      1, 1, 4, 0
   },
   {
      "SVGA3D_B8G8R8X8_UNORM",
      SVGA3D_B8G8R8X8_UNORM,
      SVGA3D_DEVCAP_DXFMT_B8G8R8X8_UNORM,
      1, 1, 4, 0
   },
   {
      "SVGA3D_BC4_UNORM",
     SVGA3D_BC4_UNORM,
     SVGA3D_DEVCAP_DXFMT_BC4_UNORM,
     4, 4, 8, 0
   },
   {
      "SVGA3D_BC5_UNORM",
     SVGA3D_BC5_UNORM,
     SVGA3D_DEVCAP_DXFMT_BC5_UNORM,
     4, 4, 16, 0
   },
   {
      "SVGA3D_B4G4R4A4_UNORM",
	  SVGA3D_B4G4R4A4_UNORM,
      0, 0, 0, 0
   },
   {
      "SVGA3D_BC6H_TYPELESS",
     SVGA3D_BC6H_TYPELESS,
     SVGA3D_DEVCAP_DXFMT_BC6H_TYPELESS,
     4, 4, 16, 0
   },
   {
      "SVGA3D_BC6H_UF16",
     SVGA3D_BC6H_UF16,
     SVGA3D_DEVCAP_DXFMT_BC6H_UF16,
     4, 4, 16, 0
   },
   {
      "SVGA3D_BC6H_SF16",
     SVGA3D_BC6H_SF16,
     SVGA3D_DEVCAP_DXFMT_BC6H_SF16,
     4, 4, 16, 0
   },
   {
      "SVGA3D_BC7_TYPELESS",
     SVGA3D_BC7_TYPELESS,
     SVGA3D_DEVCAP_DXFMT_BC7_TYPELESS,
     4, 4, 16, 0
   },
   {
      "SVGA3D_BC7_UNORM",
     SVGA3D_BC7_UNORM,
     SVGA3D_DEVCAP_DXFMT_BC6H_TYPELESS,
     4, 4, 16, 0
   },
   {
      "SVGA3D_BC7_UNORM_SRGB",
     SVGA3D_BC7_UNORM_SRGB,
     SVGA3D_DEVCAP_DXFMT_BC6H_TYPELESS,
     4, 4, 16, 0
   },
   {
      "SVGA3D_AYUV",
     SVGA3D_AYUV,
     0,
     1, 1, 4, 0
   },
   {
      "SVGA3D_R11G11B10_TYPELESS",
     SVGA3D_R11G11B10_TYPELESS,
     SVGA3D_DEVCAP_DXFMT_R11G11B10_FLOAT,
     1, 1, 4, 0
   }
};

static const SVGA3dSurfaceFormat compat_x8r8g8b8[] = {
   SVGA3D_X8R8G8B8, SVGA3D_A8R8G8B8, SVGA3D_B8G8R8X8_UNORM,
   SVGA3D_B8G8R8A8_UNORM, SVGA3D_B8G8R8X8_TYPELESS, SVGA3D_B8G8R8A8_TYPELESS, 0
};
static const SVGA3dSurfaceFormat compat_r8g8b8a8[] = {
   SVGA3D_R8G8B8A8_UNORM, SVGA3D_R8G8B8A8_TYPELESS, 0
};
static const SVGA3dSurfaceFormat compat_r8[] = {
   SVGA3D_R8_UNORM, SVGA3D_NV12, SVGA3D_YV12, 0
};
static const SVGA3dSurfaceFormat compat_g8r8[] = {
   SVGA3D_R8G8_UNORM, SVGA3D_NV12, 0
};
static const SVGA3dSurfaceFormat compat_r5g6b5[] = {
   SVGA3D_R5G6B5, SVGA3D_B5G6R5_UNORM, 0
};

static const struct format_compat_entry format_compats[] = {
   {PIPE_FORMAT_B8G8R8X8_UNORM, compat_x8r8g8b8},
   {PIPE_FORMAT_B8G8R8A8_UNORM, compat_x8r8g8b8},
   {PIPE_FORMAT_R8G8B8A8_UNORM, compat_r8g8b8a8},
   {PIPE_FORMAT_R8_UNORM, compat_r8},
   {PIPE_FORMAT_R8G8_UNORM, compat_g8r8},
   {PIPE_FORMAT_B5G6R5_UNORM, compat_r5g6b5}
};

/**
 * Debug only:
 * 1. check that format_cap_table[i] matches the i-th SVGA3D format.
 * 2. check that format_conversion_table[i].pformat == i.
 */
static void
check_format_tables(void)
{
   static bool first_call = true;

   if (first_call) {
      unsigned i;

      STATIC_ASSERT(ARRAY_SIZE(format_cap_table) == SVGA3D_FORMAT_MAX);
      for (i = 0; i < ARRAY_SIZE(format_cap_table); i++) {
         assert(format_cap_table[i].format == i);
      }

      first_call = false;
   }
}


/**
 * Return string name of an SVGA3dDevCapIndex value.
 * For debugging.
 */
static const char *
svga_devcap_name(SVGA3dDevCapIndex cap)
{
   static const struct debug_named_value devcap_names[] = {
      /* Note, we only list the DXFMT devcaps so far */
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_X8R8G8B8),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_A8R8G8B8),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R5G6B5),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_X1R5G5B5),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_A1R5G5B5),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_A4R4G4B4),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_Z_D32),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_Z_D16),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_Z_D24S8),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_Z_D15S1),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_LUMINANCE8),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_LUMINANCE4_ALPHA4),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_LUMINANCE16),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_LUMINANCE8_ALPHA8),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_DXT1),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_DXT2),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_DXT3),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_DXT4),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_DXT5),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_BUMPU8V8),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_BUMPL6V5U5),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_BUMPX8L8V8U8),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_FORMAT_DEAD1),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_ARGB_S10E5),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_ARGB_S23E8),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_A2R10G10B10),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_V8U8),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_Q8W8V8U8),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_CxV8U8),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_X8L8V8U8),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_A2W10V10U10),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_ALPHA8),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R_S10E5),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R_S23E8),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_RG_S10E5),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_RG_S23E8),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_BUFFER),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_Z_D24X8),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_V16U16),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_G16R16),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_A16B16G16R16),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_UYVY),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_YUY2),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_NV12),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R32G32B32A32_TYPELESS),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R32G32B32A32_UINT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R32G32B32A32_SINT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R32G32B32_TYPELESS),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R32G32B32_FLOAT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R32G32B32_UINT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R32G32B32_SINT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R16G16B16A16_TYPELESS),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R16G16B16A16_UINT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R16G16B16A16_SNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R16G16B16A16_SINT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R32G32_TYPELESS),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R32G32_UINT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R32G32_SINT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R32G8X24_TYPELESS),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_D32_FLOAT_S8X24_UINT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R32_FLOAT_X8X24),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_X32_G8X24_UINT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R10G10B10A2_TYPELESS),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R10G10B10A2_UINT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R11G11B10_FLOAT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R8G8B8A8_TYPELESS),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R8G8B8A8_UNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R8G8B8A8_UNORM_SRGB),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R8G8B8A8_UINT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R8G8B8A8_SINT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R16G16_TYPELESS),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R16G16_UINT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R16G16_SINT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R32_TYPELESS),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_D32_FLOAT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R32_UINT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R32_SINT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R24G8_TYPELESS),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_D24_UNORM_S8_UINT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R24_UNORM_X8),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_X24_G8_UINT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R8G8_TYPELESS),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R8G8_UNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R8G8_UINT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R8G8_SINT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R16_TYPELESS),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R16_UNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R16_UINT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R16_SNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R16_SINT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R8_TYPELESS),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R8_UNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R8_UINT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R8_SNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R8_SINT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_P8),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R9G9B9E5_SHAREDEXP),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R8G8_B8G8_UNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_G8R8_G8B8_UNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_BC1_TYPELESS),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_BC1_UNORM_SRGB),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_BC2_TYPELESS),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_BC2_UNORM_SRGB),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_BC3_TYPELESS),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_BC3_UNORM_SRGB),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_BC4_TYPELESS),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_ATI1),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_BC4_SNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_BC5_TYPELESS),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_ATI2),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_BC5_SNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R10G10B10_XR_BIAS_A2_UNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_B8G8R8A8_TYPELESS),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_B8G8R8A8_UNORM_SRGB),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_B8G8R8X8_TYPELESS),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_B8G8R8X8_UNORM_SRGB),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_Z_DF16),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_Z_DF24),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_Z_D24S8_INT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_YV12),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R32G32B32A32_FLOAT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R16G16B16A16_FLOAT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R16G16B16A16_UNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R32G32_FLOAT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R10G10B10A2_UNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R8G8B8A8_SNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R16G16_FLOAT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R16G16_UNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R16G16_SNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R32_FLOAT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R8G8_SNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_R16_FLOAT),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_D16_UNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_A8_UNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_BC1_UNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_BC2_UNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_BC3_UNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_B5G6R5_UNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_B5G5R5A1_UNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_B8G8R8A8_UNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_B8G8R8X8_UNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_BC4_UNORM),
      DEBUG_NAMED_VALUE(SVGA3D_DEVCAP_DXFMT_BC5_UNORM),
      DEBUG_NAMED_VALUE_END,
   };
   return debug_dump_enum(devcap_names, cap);
}


/**
 * Return string for a bitmask of name of SVGA3D_DXFMT_x flags.
 * For debugging.
 */
static const char *
svga_devcap_format_flags(unsigned flags)
{
   static const struct debug_named_value devcap_flags[] = {
      DEBUG_NAMED_VALUE(SVGA3D_DXFMT_SUPPORTED),
      DEBUG_NAMED_VALUE(SVGA3D_DXFMT_SHADER_SAMPLE),
      DEBUG_NAMED_VALUE(SVGA3D_DXFMT_COLOR_RENDERTARGET),
      DEBUG_NAMED_VALUE(SVGA3D_DXFMT_DEPTH_RENDERTARGET),
      DEBUG_NAMED_VALUE(SVGA3D_DXFMT_BLENDABLE),
      DEBUG_NAMED_VALUE(SVGA3D_DXFMT_MIPS),
      DEBUG_NAMED_VALUE(SVGA3D_DXFMT_ARRAY),
      DEBUG_NAMED_VALUE(SVGA3D_DXFMT_VOLUME),
      DEBUG_NAMED_VALUE(SVGA3D_DXFMT_DX_VERTEX_BUFFER),
      DEBUG_NAMED_VALUE(SVGA3D_DXFMT_MULTISAMPLE),
      DEBUG_NAMED_VALUE_END
   };

   return debug_dump_flags(devcap_flags, flags);
}


/*
 * Get format capabilities from the host.  It takes in consideration
 * deprecated/unsupported formats, and formats which are implicitely assumed to
 * be supported when the host does not provide an explicit capability entry.
 */
void
svga_get_format_cap(struct svga_screen *ss,
                    SVGA3dSurfaceFormat format,
                    SVGA3dSurfaceFormatCaps *caps)
{
   struct svga_winsys_screen *sws = ss->sws;
   SVGA3dDevCapResult result;
   const struct format_cap *entry;

#ifdef DEBUG
   check_format_tables();
#else
   (void) check_format_tables;
#endif

   assert(format < ARRAY_SIZE(format_cap_table));
   entry = &format_cap_table[format];
   assert(entry->format == format);

   if (entry->devcap && sws->get_cap(sws, entry->devcap, &result)) {
      assert(format < SVGA3D_UYVY || entry->defaultOperations == 0);
      caps->value = result.u;
   } else {
      /* Implicitly advertised format -- use default caps */
      caps->value = entry->defaultOperations;
   }
}


/*
 * Get DX format capabilities from VGPU10 device.
 */
static void
svga_get_dx_format_cap(struct svga_screen *ss,
                       SVGA3dSurfaceFormat format,
                       SVGA3dDevCapResult *caps)
{
   struct svga_winsys_screen *sws = ss->sws;
   const struct format_cap *entry;

#ifdef DEBUG
   check_format_tables();
#else
   (void) check_format_tables;
#endif

   assert(sws->have_vgpu10);
   assert(format < ARRAY_SIZE(format_cap_table));
   entry = &format_cap_table[format];
   assert(entry->format == format);
   assert(entry->devcap > SVGA3D_DEVCAP_DXCONTEXT);

   caps->u = 0;
   if (entry->devcap) {
      sws->get_cap(sws, entry->devcap, caps);

      /* pre-SM41 capable svga device supports SHADER_SAMPLE capability for
       * these formats but does not advertise the devcap.
       * So enable this bit here.
       */
      if (!sws->have_sm4_1 &&
          (format == SVGA3D_R32_FLOAT_X8X24 ||
           format == SVGA3D_R24_UNORM_X8)) {
         caps->u |= SVGA3D_DXFMT_SHADER_SAMPLE;
      }
   }
   else {
      caps->u = entry->defaultOperations;
   }

   if (0) {
      debug_printf("Format %s, devcap %s = 0x%x (%s)\n",
                   svga_format_name(format),
                   svga_devcap_name(entry->devcap),
                   caps->u,
                   svga_devcap_format_flags(caps->u));
   }
}


void
svga_format_size(SVGA3dSurfaceFormat format,
                 unsigned *block_width,
                 unsigned *block_height,
                 unsigned *bytes_per_block)
{
   assert(format < ARRAY_SIZE(format_cap_table));
   *block_width = format_cap_table[format].block_width;
   *block_height = format_cap_table[format].block_height;
   *bytes_per_block = format_cap_table[format].block_bytes;
   /* Make sure the table entry was valid */
   if (*block_width == 0)
      debug_printf("Bad table entry for %s\n", svga_format_name(format));
   assert(*block_width);
   assert(*block_height);
   assert(*bytes_per_block);
}


const char *
svga_format_name(SVGA3dSurfaceFormat format)
{
   assert(format < ARRAY_SIZE(format_cap_table));
   return format_cap_table[format].name;
}


/**
 * Is the given SVGA3dSurfaceFormat a signed or unsigned integer color format?
 */
bool
svga_format_is_integer(SVGA3dSurfaceFormat format)
{
   switch (format) {
   case SVGA3D_R32G32B32A32_SINT:
   case SVGA3D_R32G32B32_SINT:
   case SVGA3D_R32G32_SINT:
   case SVGA3D_R32_SINT:
   case SVGA3D_R16G16B16A16_SINT:
   case SVGA3D_R16G16_SINT:
   case SVGA3D_R16_SINT:
   case SVGA3D_R8G8B8A8_SINT:
   case SVGA3D_R8G8_SINT:
   case SVGA3D_R8_SINT:
   case SVGA3D_R32G32B32A32_UINT:
   case SVGA3D_R32G32B32_UINT:
   case SVGA3D_R32G32_UINT:
   case SVGA3D_R32_UINT:
   case SVGA3D_R16G16B16A16_UINT:
   case SVGA3D_R16G16_UINT:
   case SVGA3D_R16_UINT:
   case SVGA3D_R8G8B8A8_UINT:
   case SVGA3D_R8G8_UINT:
   case SVGA3D_R8_UINT:
   case SVGA3D_R10G10B10A2_UINT:
      return true;
   default:
      return false;
   }
}

bool
svga_format_support_gen_mips(enum pipe_format format)
{
   const struct vgpu10_format_entry *entry = svga_format_entry(format);

   return (entry->flags & TF_GEN_MIPS) > 0;
}


/**
 * Given a texture format, return the expected data type returned from
 * the texture sampler.  For example, UNORM8 formats return floating point
 * values while SINT formats returned signed integer values.
 * Note: this function could be moved into the gallum u_format.[ch] code
 * if it's useful to anyone else.
 */
enum tgsi_return_type
svga_get_texture_datatype(enum pipe_format format)
{
   const struct util_format_description *desc = util_format_description(format);
   enum tgsi_return_type t;

   if (desc->layout == UTIL_FORMAT_LAYOUT_PLAIN ) {
      if (util_format_is_depth_or_stencil(format)) {
         t = TGSI_RETURN_TYPE_FLOAT; /* XXX revisit this */
      }
      else if (desc->channel[0].type == UTIL_FORMAT_TYPE_FLOAT) {
         t = TGSI_RETURN_TYPE_FLOAT;
      }
      else if (desc->channel[0].type == UTIL_FORMAT_TYPE_UNSIGNED) {
         t = desc->channel[0].normalized ? TGSI_RETURN_TYPE_UNORM : TGSI_RETURN_TYPE_UINT;
      }
      else if (desc->channel[0].type == UTIL_FORMAT_TYPE_SIGNED) {
         t = desc->channel[0].normalized ? TGSI_RETURN_TYPE_SNORM : TGSI_RETURN_TYPE_SINT;
      }
      else {
         assert(!"Unexpected channel type in svga_get_texture_datatype()");
         t = TGSI_RETURN_TYPE_FLOAT;
      }
   }
   else {
      /* compressed format, shared exponent format, etc. */
      switch (format) {
      case PIPE_FORMAT_DXT1_RGB:
      case PIPE_FORMAT_DXT1_RGBA:
      case PIPE_FORMAT_DXT3_RGBA:
      case PIPE_FORMAT_DXT5_RGBA:
      case PIPE_FORMAT_DXT1_SRGB:
      case PIPE_FORMAT_DXT1_SRGBA:
      case PIPE_FORMAT_DXT3_SRGBA:
      case PIPE_FORMAT_DXT5_SRGBA:
      case PIPE_FORMAT_RGTC1_UNORM:
      case PIPE_FORMAT_RGTC2_UNORM:
      case PIPE_FORMAT_LATC1_UNORM:
      case PIPE_FORMAT_LATC2_UNORM:
      case PIPE_FORMAT_ETC1_RGB8:
         t = TGSI_RETURN_TYPE_UNORM;
         break;
      case PIPE_FORMAT_RGTC1_SNORM:
      case PIPE_FORMAT_RGTC2_SNORM:
      case PIPE_FORMAT_LATC1_SNORM:
      case PIPE_FORMAT_LATC2_SNORM:
      case PIPE_FORMAT_R10G10B10X2_SNORM:
         t = TGSI_RETURN_TYPE_SNORM;
         break;
      case PIPE_FORMAT_R11G11B10_FLOAT:
      case PIPE_FORMAT_R9G9B9E5_FLOAT:
         t = TGSI_RETURN_TYPE_FLOAT;
         break;
      default:
         assert(!"Unexpected channel type in svga_get_texture_datatype()");
         t = TGSI_RETURN_TYPE_FLOAT;
      }
   }

   return t;
}


/**
 * Given an svga context, return true iff there are currently any integer color
 * buffers attached to the framebuffer.
 */
bool
svga_has_any_integer_cbufs(const struct svga_context *svga)
{
   unsigned i;
   for (i = 0; i < PIPE_MAX_COLOR_BUFS; ++i) {
      struct pipe_surface *cbuf = svga->curr.framebuffer.cbufs[i];

      if (cbuf && util_format_is_pure_integer(cbuf->format)) {
         return true;
      }
   }
   return false;
}


/**
 * Given an SVGA format, return the corresponding typeless format.
 * If there is no typeless format, return the format unchanged.
 */
SVGA3dSurfaceFormat
svga_typeless_format(SVGA3dSurfaceFormat format)
{
   switch (format) {
   case SVGA3D_R32G32B32A32_UINT:
   case SVGA3D_R32G32B32A32_SINT:
   case SVGA3D_R32G32B32A32_FLOAT:
   case SVGA3D_R32G32B32A32_TYPELESS:
      return SVGA3D_R32G32B32A32_TYPELESS;
   case SVGA3D_R32G32B32_FLOAT:
   case SVGA3D_R32G32B32_UINT:
   case SVGA3D_R32G32B32_SINT:
   case SVGA3D_R32G32B32_TYPELESS:
      return SVGA3D_R32G32B32_TYPELESS;
   case SVGA3D_R16G16B16A16_UINT:
   case SVGA3D_R16G16B16A16_UNORM:
   case SVGA3D_R16G16B16A16_SNORM:
   case SVGA3D_R16G16B16A16_SINT:
   case SVGA3D_R16G16B16A16_FLOAT:
   case SVGA3D_R16G16B16A16_TYPELESS:
      return SVGA3D_R16G16B16A16_TYPELESS;
   case SVGA3D_R32G32_UINT:
   case SVGA3D_R32G32_SINT:
   case SVGA3D_R32G32_FLOAT:
   case SVGA3D_R32G32_TYPELESS:
      return SVGA3D_R32G32_TYPELESS;
   case SVGA3D_D32_FLOAT_S8X24_UINT:
   case SVGA3D_X32_G8X24_UINT:
   case SVGA3D_R32G8X24_TYPELESS:
      return SVGA3D_R32G8X24_TYPELESS;
   case SVGA3D_R10G10B10A2_UINT:
   case SVGA3D_R10G10B10A2_UNORM:
   case SVGA3D_R10G10B10A2_TYPELESS:
      return SVGA3D_R10G10B10A2_TYPELESS;
   case SVGA3D_R8G8B8A8_UNORM:
   case SVGA3D_R8G8B8A8_SNORM:
   case SVGA3D_R8G8B8A8_UNORM_SRGB:
   case SVGA3D_R8G8B8A8_UINT:
   case SVGA3D_R8G8B8A8_SINT:
   case SVGA3D_R8G8B8A8_TYPELESS:
      return SVGA3D_R8G8B8A8_TYPELESS;
   case SVGA3D_R16G16_UINT:
   case SVGA3D_R16G16_SINT:
   case SVGA3D_R16G16_UNORM:
   case SVGA3D_R16G16_SNORM:
   case SVGA3D_R16G16_FLOAT:
   case SVGA3D_R16G16_TYPELESS:
      return SVGA3D_R16G16_TYPELESS;
   case SVGA3D_D32_FLOAT:
   case SVGA3D_R32_FLOAT:
   case SVGA3D_R32_UINT:
   case SVGA3D_R32_SINT:
   case SVGA3D_R32_TYPELESS:
      return SVGA3D_R32_TYPELESS;
   case SVGA3D_D24_UNORM_S8_UINT:
   case SVGA3D_R24G8_TYPELESS:
      return SVGA3D_R24G8_TYPELESS;
   case SVGA3D_X24_G8_UINT:
      return SVGA3D_R24_UNORM_X8;
   case SVGA3D_R8G8_UNORM:
   case SVGA3D_R8G8_SNORM:
   case SVGA3D_R8G8_UINT:
   case SVGA3D_R8G8_SINT:
   case SVGA3D_R8G8_TYPELESS:
      return SVGA3D_R8G8_TYPELESS;
   case SVGA3D_D16_UNORM:
   case SVGA3D_R16_UNORM:
   case SVGA3D_R16_UINT:
   case SVGA3D_R16_SNORM:
   case SVGA3D_R16_SINT:
   case SVGA3D_R16_FLOAT:
   case SVGA3D_R16_TYPELESS:
      return SVGA3D_R16_TYPELESS;
   case SVGA3D_R8_UNORM:
   case SVGA3D_R8_UINT:
   case SVGA3D_R8_SNORM:
   case SVGA3D_R8_SINT:
   case SVGA3D_R8_TYPELESS:
      return SVGA3D_R8_TYPELESS;
   case SVGA3D_B8G8R8A8_UNORM_SRGB:
   case SVGA3D_B8G8R8A8_UNORM:
   case SVGA3D_B8G8R8A8_TYPELESS:
      return SVGA3D_B8G8R8A8_TYPELESS;
   case SVGA3D_B8G8R8X8_UNORM_SRGB:
   case SVGA3D_B8G8R8X8_UNORM:
   case SVGA3D_B8G8R8X8_TYPELESS:
      return SVGA3D_B8G8R8X8_TYPELESS;
   case SVGA3D_BC1_UNORM:
   case SVGA3D_BC1_UNORM_SRGB:
   case SVGA3D_BC1_TYPELESS:
      return SVGA3D_BC1_TYPELESS;
   case SVGA3D_BC2_UNORM:
   case SVGA3D_BC2_UNORM_SRGB:
   case SVGA3D_BC2_TYPELESS:
      return SVGA3D_BC2_TYPELESS;
   case SVGA3D_BC3_UNORM:
   case SVGA3D_BC3_UNORM_SRGB:
   case SVGA3D_BC3_TYPELESS:
      return SVGA3D_BC3_TYPELESS;
   case SVGA3D_BC4_UNORM:
   case SVGA3D_BC4_SNORM:
   case SVGA3D_BC4_TYPELESS:
      return SVGA3D_BC4_TYPELESS;
   case SVGA3D_BC5_UNORM:
   case SVGA3D_BC5_SNORM:
   case SVGA3D_BC5_TYPELESS:
      return SVGA3D_BC5_TYPELESS;
   case SVGA3D_BC6H_UF16:
   case SVGA3D_BC6H_SF16:
   case SVGA3D_BC6H_TYPELESS:
      return SVGA3D_BC6H_TYPELESS;
   case SVGA3D_BC7_UNORM:
   case SVGA3D_BC7_UNORM_SRGB:
   case SVGA3D_BC7_TYPELESS:
      return SVGA3D_BC7_TYPELESS;
   case SVGA3D_R11G11B10_FLOAT:
   case SVGA3D_R11G11B10_TYPELESS:
      return SVGA3D_R11G11B10_TYPELESS;

   /* Special cases (no corresponding _TYPELESS formats) */
   case SVGA3D_A8_UNORM:
   case SVGA3D_B5G5R5A1_UNORM:
   case SVGA3D_B5G6R5_UNORM:
   case SVGA3D_R9G9B9E5_SHAREDEXP:
      return format;
   default:
      debug_printf("Unexpected format %s in %s\n",
                   svga_format_name(format), __func__);
      return format;
   }
}


/**
 * Given a surface format, return the corresponding format to use for
 * a texture sampler.  In most cases, it's the format unchanged, but there
 * are some special cases.
 */
SVGA3dSurfaceFormat
svga_sampler_format(SVGA3dSurfaceFormat format)
{
   switch (format) {
   case SVGA3D_D16_UNORM:
      return SVGA3D_R16_UNORM;
   case SVGA3D_D24_UNORM_S8_UINT:
      return SVGA3D_R24_UNORM_X8;
   case SVGA3D_D32_FLOAT:
      return SVGA3D_R32_FLOAT;
   case SVGA3D_D32_FLOAT_S8X24_UINT:
      return SVGA3D_R32_FLOAT_X8X24;
   default:
      return format;
   }
}


/**
 * Is the given format an uncompressed snorm format?
 */
bool
svga_format_is_uncompressed_snorm(SVGA3dSurfaceFormat format)
{
   switch (format) {
   case SVGA3D_R8G8B8A8_SNORM:
   case SVGA3D_R8G8_SNORM:
   case SVGA3D_R8_SNORM:
   case SVGA3D_R16G16B16A16_SNORM:
   case SVGA3D_R16G16_SNORM:
   case SVGA3D_R16_SNORM:
      return true;
   default:
      return false;
   }
}


bool
svga_format_is_typeless(SVGA3dSurfaceFormat format)
{
   switch (format) {
   case SVGA3D_R32G32B32A32_TYPELESS:
   case SVGA3D_R32G32B32_TYPELESS:
   case SVGA3D_R16G16B16A16_TYPELESS:
   case SVGA3D_R32G32_TYPELESS:
   case SVGA3D_R32G8X24_TYPELESS:
   case SVGA3D_R10G10B10A2_TYPELESS:
   case SVGA3D_R8G8B8A8_TYPELESS:
   case SVGA3D_R16G16_TYPELESS:
   case SVGA3D_R32_TYPELESS:
   case SVGA3D_R24G8_TYPELESS:
   case SVGA3D_R8G8_TYPELESS:
   case SVGA3D_R16_TYPELESS:
   case SVGA3D_R8_TYPELESS:
   case SVGA3D_BC1_TYPELESS:
   case SVGA3D_BC2_TYPELESS:
   case SVGA3D_BC3_TYPELESS:
   case SVGA3D_BC4_TYPELESS:
   case SVGA3D_BC5_TYPELESS:
   case SVGA3D_BC6H_TYPELESS:
   case SVGA3D_BC7_TYPELESS:
   case SVGA3D_B8G8R8A8_TYPELESS:
   case SVGA3D_B8G8R8X8_TYPELESS:
      return true;
   default:
      return false;
   }
}


/**
 * \brief Can we import a surface with a given SVGA3D format as a texture?
 *
 * \param ss[in]  pointer to the svga screen.
 * \param pformat[in]  pipe format of the local texture.
 * \param sformat[in]  svga3d format of the imported surface.
 * \param bind[in]  bind flags of the imported texture.
 * \param verbose[in]  Print out incompatibilities in debug mode.
 */
bool
svga_format_is_shareable(const struct svga_screen *ss,
                         enum pipe_format pformat,
                         SVGA3dSurfaceFormat sformat,
                         unsigned bind,
                         bool verbose)
{
   SVGA3dSurfaceFormat default_format =
      svga_translate_format(ss, pformat, bind);
   int i;

   if (default_format == SVGA3D_FORMAT_INVALID)
      return false;
   if (default_format == sformat)
      return true;

   for (i = 0; i < ARRAY_SIZE(format_compats); ++i) {
      if (format_compats[i].pformat == pformat) {
         const SVGA3dSurfaceFormat *compat_format =
            format_compats[i].compat_format;
         while (*compat_format != 0) {
            if (*compat_format == sformat)
               return true;
            compat_format++;
         }
      }
   }

   if (verbose) {
      debug_printf("Incompatible imported surface format.\n");
      debug_printf("Texture format: \"%s\". Imported format: \"%s\".\n",
                   svga_format_name(default_format),
                   svga_format_name(sformat));
   }

   return false;
}


/**
  * Return the sRGB format which corresponds to the given (linear) format.
  * If there's no such sRGB format, return the format as-is.
  */
SVGA3dSurfaceFormat
svga_linear_to_srgb(SVGA3dSurfaceFormat format)
{
   switch (format) {
   case SVGA3D_R8G8B8A8_UNORM:
      return SVGA3D_R8G8B8A8_UNORM_SRGB;
   case SVGA3D_BC1_UNORM:
      return SVGA3D_BC1_UNORM_SRGB;
   case SVGA3D_BC2_UNORM:
      return SVGA3D_BC2_UNORM_SRGB;
   case SVGA3D_BC3_UNORM:
      return SVGA3D_BC3_UNORM_SRGB;
   case SVGA3D_B8G8R8A8_UNORM:
      return SVGA3D_B8G8R8A8_UNORM_SRGB;
   case SVGA3D_B8G8R8X8_UNORM:
      return SVGA3D_B8G8R8X8_UNORM_SRGB;
   default:
      return format;
   }
}


/**
 * Implement pipe_screen::is_format_supported().
 * \param bindings  bitmask of PIPE_BIND_x flags
 */
bool
svga_is_format_supported(struct pipe_screen *screen,
                         enum pipe_format format,
                         enum pipe_texture_target target,
                         unsigned sample_count,
                         unsigned storage_sample_count,
                         unsigned bindings)
{
   struct svga_screen *ss = svga_screen(screen);
   SVGA3dSurfaceFormat svga_format;
   SVGA3dSurfaceFormatCaps caps;
   SVGA3dSurfaceFormatCaps mask;

   assert(bindings);
   assert(!ss->sws->have_vgpu10);

   /* Multisamples is not supported in VGPU9 device */
   if (sample_count > 1)
      return false;

   svga_format = svga_translate_format(ss, format, bindings);
   if (svga_format == SVGA3D_FORMAT_INVALID) {
      return false;
   }

   if (util_format_is_srgb(format) &&
       (bindings & (PIPE_BIND_DISPLAY_TARGET | PIPE_BIND_RENDER_TARGET))) {
       /* We only support sRGB rendering with vgpu10 */
      return false;
   }

   /*
    * Override host capabilities, so that we end up with the same
    * visuals for all virtual hardware implementations.
    */
   if (bindings & PIPE_BIND_DISPLAY_TARGET) {
      switch (svga_format) {
      case SVGA3D_A8R8G8B8:
      case SVGA3D_X8R8G8B8:
      case SVGA3D_R5G6B5:
         break;

      /* VGPU10 formats */
      case SVGA3D_B8G8R8A8_UNORM:
      case SVGA3D_B8G8R8X8_UNORM:
      case SVGA3D_B5G6R5_UNORM:
      case SVGA3D_B8G8R8X8_UNORM_SRGB:
      case SVGA3D_B8G8R8A8_UNORM_SRGB:
      case SVGA3D_R8G8B8A8_UNORM_SRGB:
         break;

      /* Often unsupported/problematic. This means we end up with the same
       * visuals for all virtual hardware implementations.
       */
      case SVGA3D_A4R4G4B4:
      case SVGA3D_A1R5G5B5:
         return false;

      default:
         return false;
      }
   }

   /*
    * Query the host capabilities.
    */
   svga_get_format_cap(ss, svga_format, &caps);

   if (bindings & PIPE_BIND_RENDER_TARGET) {
      /* Check that the color surface is blendable, unless it's an
       * integer format.
       */
      if (!svga_format_is_integer(svga_format) &&
          (caps.value & SVGA3DFORMAT_OP_NOALPHABLEND)) {
         return false;
      }
   }

   mask.value = 0;
   if (bindings & PIPE_BIND_RENDER_TARGET)
      mask.value |= SVGA3DFORMAT_OP_OFFSCREEN_RENDERTARGET;

   if (bindings & PIPE_BIND_DEPTH_STENCIL)
      mask.value |= SVGA3DFORMAT_OP_ZSTENCIL;

   if (bindings & PIPE_BIND_SAMPLER_VIEW)
      mask.value |= SVGA3DFORMAT_OP_TEXTURE;

   if (target == PIPE_TEXTURE_CUBE)
      mask.value |= SVGA3DFORMAT_OP_CUBETEXTURE;
   else if (target == PIPE_TEXTURE_3D)
      mask.value |= SVGA3DFORMAT_OP_VOLUMETEXTURE;

   return (caps.value & mask.value) == mask.value;
}


/**
 * Implement pipe_screen::is_format_supported() for VGPU10 device.
 * \param bindings  bitmask of PIPE_BIND_x flags
 */
bool
svga_is_dx_format_supported(struct pipe_screen *screen,
                            enum pipe_format format,
                            enum pipe_texture_target target,
                            unsigned sample_count,
                            unsigned storage_sample_count,
                            unsigned bindings)
{
   struct svga_screen *ss = svga_screen(screen);
   SVGA3dSurfaceFormat svga_format;
   SVGA3dDevCapResult caps;
   unsigned int mask = 0;

   assert(bindings);
   assert(ss->sws->have_vgpu10);

   /* To support framebuffer without attachments */
   if ((format == PIPE_FORMAT_NONE) && (bindings == PIPE_BIND_RENDER_TARGET))
      return (ss->sws->have_gl43 && (sample_count <= ss->forcedSampleCount));

   if (sample_count > 1) {

      /* No MSAA support for shader image */
      if (bindings & PIPE_BIND_SHADER_IMAGE)
         return false;

      /* In ms_samples, if bit N is set it means that we support
       * multisample with N+1 samples per pixel.
       */
      if ((ss->ms_samples & (1 << (sample_count - 1))) == 0) {
         return false;
      }
      mask |= SVGA3D_DXFMT_MULTISAMPLE;
   }

   /*
    * For VGPU10 vertex formats, skip querying host capabilities
    */

   if (bindings & PIPE_BIND_VERTEX_BUFFER) {
      unsigned flags;
      svga_translate_vertex_format_vgpu10(format, &svga_format, &flags);
      return svga_format != SVGA3D_FORMAT_INVALID;
   }

   if (bindings & PIPE_BIND_SAMPLER_VIEW && target == PIPE_BUFFER) {
      unsigned flags;
      svga_translate_texture_buffer_view_format(format, &svga_format, &flags);
      return svga_format != SVGA3D_FORMAT_INVALID;
   }

   svga_format = svga_translate_format(ss, format, bindings);
   if (svga_format == SVGA3D_FORMAT_INVALID) {
      return false;
   }

   /*
    * Override host capabilities, so that we end up with the same
    * visuals for all virtual hardware implementations.
    */
   if (bindings & PIPE_BIND_DISPLAY_TARGET) {
      switch (svga_format) {
      case SVGA3D_A8R8G8B8:
      case SVGA3D_X8R8G8B8:
      case SVGA3D_R5G6B5:
         break;

      /* VGPU10 formats */
      case SVGA3D_B8G8R8A8_UNORM:
      case SVGA3D_B8G8R8X8_UNORM:
      case SVGA3D_B5G6R5_UNORM:
      case SVGA3D_B8G8R8X8_UNORM_SRGB:
      case SVGA3D_B8G8R8A8_UNORM_SRGB:
      case SVGA3D_R8G8B8A8_UNORM_SRGB:
         break;

      /* Often unsupported/problematic. This means we end up with the same
       * visuals for all virtual hardware implementations.
       */
      case SVGA3D_A4R4G4B4:
      case SVGA3D_A1R5G5B5:
         return false;

      default:
         return false;
      }
   }

   /*
    * Query the host capabilities.
    */
   svga_get_dx_format_cap(ss, svga_format, &caps);

   if (bindings & PIPE_BIND_RENDER_TARGET) {
      /* Check that the color surface is blendable, unless it's an
       * integer format.
       */
      if (!(svga_format_is_integer(svga_format) ||
            (caps.u & SVGA3D_DXFMT_BLENDABLE))) {
         return false;
      }
      mask |= SVGA3D_DXFMT_COLOR_RENDERTARGET;
   }

   if (bindings & PIPE_BIND_DEPTH_STENCIL)
      mask |= SVGA3D_DXFMT_DEPTH_RENDERTARGET;

   switch (target) {
   case PIPE_TEXTURE_3D:
      mask |= SVGA3D_DXFMT_VOLUME;
      break;
   case PIPE_TEXTURE_1D_ARRAY:
   case PIPE_TEXTURE_2D_ARRAY:
   case PIPE_TEXTURE_CUBE_ARRAY:
      mask |= SVGA3D_DXFMT_ARRAY;
      break;
   default:
      break;
   }

   /* Is the format supported for rendering */
   if ((caps.u & mask) != mask)
      return false;

   if (bindings & PIPE_BIND_SAMPLER_VIEW) {
      SVGA3dSurfaceFormat sampler_format;

      /* Get the sampler view format */
      sampler_format = svga_sampler_format(svga_format);
      if (sampler_format != svga_format) {
         caps.u = 0;
         svga_get_dx_format_cap(ss, sampler_format, &caps);
         mask &= SVGA3D_DXFMT_VOLUME;
         mask |= SVGA3D_DXFMT_SHADER_SAMPLE;
         if ((caps.u & mask) != mask)
            return false;
      }
   }

   return true;
}
