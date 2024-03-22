/*
 * Copyright 2011 Joakim Sindholt <opensource@zhasha.com>
 * Copyright 2013 Christoph Bumiller
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "device9.h"
#include "nine_pipe.h"

#include "cso_cache/cso_context.h"

void
nine_convert_dsa_state(struct pipe_depth_stencil_alpha_state *dsa_state,
                       const DWORD *rs)
{
    struct pipe_depth_stencil_alpha_state dsa;

    memset(&dsa, 0, sizeof(dsa)); /* memcmp safety */

    if (rs[D3DRS_ZENABLE]) {
        dsa.depth_enabled = 1;
        dsa.depth_func = d3dcmpfunc_to_pipe_func(rs[D3DRS_ZFUNC]);
        /* Disable depth write if no change can occur */
        dsa.depth_writemask = !!rs[D3DRS_ZWRITEENABLE] &&
            dsa.depth_func != PIPE_FUNC_EQUAL &&
            dsa.depth_func != PIPE_FUNC_NEVER;
    }

    if (rs[D3DRS_STENCILENABLE]) {
        dsa.stencil[0].enabled = 1;
        dsa.stencil[0].func = d3dcmpfunc_to_pipe_func(rs[D3DRS_STENCILFUNC]);
        dsa.stencil[0].fail_op = d3dstencilop_to_pipe_stencil_op(rs[D3DRS_STENCILFAIL]);
        dsa.stencil[0].zpass_op = d3dstencilop_to_pipe_stencil_op(rs[D3DRS_STENCILPASS]);
        dsa.stencil[0].zfail_op = d3dstencilop_to_pipe_stencil_op(rs[D3DRS_STENCILZFAIL]);
        dsa.stencil[0].valuemask = rs[D3DRS_STENCILMASK];
        dsa.stencil[0].writemask = rs[D3DRS_STENCILWRITEMASK];

        if (rs[D3DRS_TWOSIDEDSTENCILMODE]) {
            dsa.stencil[1].enabled = 1;
            dsa.stencil[1].func = d3dcmpfunc_to_pipe_func(rs[D3DRS_CCW_STENCILFUNC]);
            dsa.stencil[1].fail_op = d3dstencilop_to_pipe_stencil_op(rs[D3DRS_CCW_STENCILFAIL]);
            dsa.stencil[1].zpass_op = d3dstencilop_to_pipe_stencil_op(rs[D3DRS_CCW_STENCILPASS]);
            dsa.stencil[1].zfail_op = d3dstencilop_to_pipe_stencil_op(rs[D3DRS_CCW_STENCILZFAIL]);
            dsa.stencil[1].valuemask = dsa.stencil[0].valuemask;
            dsa.stencil[1].writemask = dsa.stencil[0].writemask;
        }
    }

    if (rs[D3DRS_ALPHATESTENABLE] && rs[NINED3DRS_EMULATED_ALPHATEST] == 7 && rs[D3DRS_ALPHAFUNC] != D3DCMP_ALWAYS) {
        dsa.alpha_enabled = 1;
        dsa.alpha_func = d3dcmpfunc_to_pipe_func(rs[D3DRS_ALPHAFUNC]);
        dsa.alpha_ref_value = (float)rs[D3DRS_ALPHAREF] / 255.0f;
    }

    *dsa_state = dsa;
}

void
nine_convert_rasterizer_state(struct NineDevice9 *device,
                              struct pipe_rasterizer_state *rast_state,
                              const DWORD *rs)
{
    struct pipe_rasterizer_state rast;

    memset(&rast, 0, sizeof(rast));

    rast.flatshade = rs[D3DRS_SHADEMODE] == D3DSHADE_FLAT;
 /* rast.light_twoside = 0; */
 /* rast.clamp_fragment_color = 0; */
 /* rast.clamp_vertex_color = 0; */
 /* rast.front_ccw = 0; */
    rast.cull_face = d3dcull_to_pipe_face(rs[D3DRS_CULLMODE]);
    rast.fill_front = d3dfillmode_to_pipe_polygon_mode(rs[D3DRS_FILLMODE]);
    rast.fill_back = rast.fill_front;
    rast.offset_tri = !!(rs[D3DRS_DEPTHBIAS] | rs[D3DRS_SLOPESCALEDEPTHBIAS]);
    rast.offset_line = rast.offset_tri; /* triangles in wireframe mode */
    rast.offset_point = 0; /* XXX ? */
    rast.scissor = !!rs[D3DRS_SCISSORTESTENABLE];
 /* rast.poly_smooth = 0; */
 /* rast.poly_stipple_enable = 0; */
 /* rast.point_smooth = 0; */
    rast.sprite_coord_mode = PIPE_SPRITE_COORD_UPPER_LEFT;
    rast.point_quad_rasterization = 1;
    rast.point_size_per_vertex = rs[NINED3DRS_VSPOINTSIZE];
    rast.multisample = rs[NINED3DRS_MULTISAMPLE];
    rast.line_smooth = !!rs[D3DRS_ANTIALIASEDLINEENABLE];
 /* rast.line_stipple_enable = 0; */
    rast.line_last_pixel = !!rs[D3DRS_LASTPIXEL];
    rast.flatshade_first = 1;
 /* rast.half_pixel_center = 0; */
 /* rast.lower_left_origin = 0; */
 /* rast.bottom_edge_rule = 0; */
 /* rast.rasterizer_discard = 0; */
    if (rs[NINED3DRS_POSITIONT] &&
        !device->driver_caps.window_space_position_support &&
        device->driver_caps.disabling_depth_clipping_support) {
        /* rast.depth_clip_near = 0; */
        /* rast.depth_clip_far = 0; */
        rast.depth_clamp = 1;
    } else {
        rast.depth_clip_near = 1;
        rast.depth_clip_far = 1;
        /* rast.depth_clamp = 0; */
    }
    rast.clip_halfz = 1;
    rast.clip_plane_enable = rs[D3DRS_CLIPPLANEENABLE];
 /* rast.line_stipple_factor = 0; */
 /* rast.line_stipple_pattern = 0; */
    rast.sprite_coord_enable = rs[D3DRS_POINTSPRITEENABLE] ? 0xff : 0x00;
    rast.line_width = 1.0f;
    rast.line_rectangular = 0;
    if (rs[NINED3DRS_VSPOINTSIZE]) {
        rast.point_size = 1.0f;
    } else {
        rast.point_size = CLAMP(asfloat(rs[D3DRS_POINTSIZE]),
                asfloat(rs[D3DRS_POINTSIZE_MIN]),
                asfloat(rs[D3DRS_POINTSIZE_MAX]));
    }
    /* offset_units has the ogl/d3d11 meaning.
     * d3d9: offset = scale * dz + bias
     * ogl/d3d11: offset = scale * dz + r * bias
     * with r implementation dependent (+ different formula for float depth
     * buffers). r=2^-23 is often the right value for gallium drivers.
     * If possible, use offset_units_unscaled, which gives the d3d9
     * behaviour, else scale by 1 << 23 */
    rast.offset_units = asfloat(rs[D3DRS_DEPTHBIAS]) * (device->driver_caps.offset_units_unscaled ? 1.0f : (float)(1 << 23));
    rast.offset_units_unscaled = device->driver_caps.offset_units_unscaled;
    rast.offset_scale = asfloat(rs[D3DRS_SLOPESCALEDEPTHBIAS]);
 /* rast.offset_clamp = 0.0f; */

    *rast_state = rast;
}

static inline void
nine_convert_blend_state_fixup(struct pipe_blend_state *blend, const DWORD *rs)
{
    if (unlikely(rs[D3DRS_SRCBLEND] == D3DBLEND_BOTHSRCALPHA ||
                 rs[D3DRS_SRCBLEND] == D3DBLEND_BOTHINVSRCALPHA)) {
        blend->rt[0].rgb_dst_factor = (rs[D3DRS_SRCBLEND] == D3DBLEND_BOTHSRCALPHA) ?
            PIPE_BLENDFACTOR_INV_SRC_ALPHA : PIPE_BLENDFACTOR_SRC_ALPHA;
        if (!rs[D3DRS_SEPARATEALPHABLENDENABLE])
            blend->rt[0].alpha_dst_factor = blend->rt[0].rgb_dst_factor;
    } else
    if (unlikely(rs[D3DRS_SEPARATEALPHABLENDENABLE] &&
                 (rs[D3DRS_SRCBLENDALPHA] == D3DBLEND_BOTHSRCALPHA ||
                  rs[D3DRS_SRCBLENDALPHA] == D3DBLEND_BOTHINVSRCALPHA))) {
        blend->rt[0].alpha_dst_factor = (rs[D3DRS_SRCBLENDALPHA] == D3DBLEND_BOTHSRCALPHA) ?
            PIPE_BLENDFACTOR_INV_SRC_ALPHA : PIPE_BLENDFACTOR_SRC_ALPHA;
    }
}

void
nine_convert_blend_state(struct pipe_blend_state *blend_state, const DWORD *rs)
{
    struct pipe_blend_state blend;

    memset(&blend, 0, sizeof(blend)); /* memcmp safety */

    blend.dither = !!rs[D3DRS_DITHERENABLE];

 /* blend.alpha_to_one = 0; */
    blend.alpha_to_coverage = !!(rs[NINED3DRS_ALPHACOVERAGE] & 5);

    blend.rt[0].blend_enable = !!rs[D3DRS_ALPHABLENDENABLE];
    if (blend.rt[0].blend_enable) {
        blend.rt[0].rgb_func = d3dblendop_to_pipe_blend(rs[D3DRS_BLENDOP]);
        blend.rt[0].rgb_src_factor = d3dblend_color_to_pipe_blendfactor(rs[D3DRS_SRCBLEND]);
        blend.rt[0].rgb_dst_factor = d3dblend_color_to_pipe_blendfactor(rs[D3DRS_DESTBLEND]);
        if (rs[D3DRS_SEPARATEALPHABLENDENABLE]) {
            blend.rt[0].alpha_func = d3dblendop_to_pipe_blend(rs[D3DRS_BLENDOPALPHA]);
            blend.rt[0].alpha_src_factor = d3dblend_alpha_to_pipe_blendfactor(rs[D3DRS_SRCBLENDALPHA]);
            blend.rt[0].alpha_dst_factor = d3dblend_alpha_to_pipe_blendfactor(rs[D3DRS_DESTBLENDALPHA]);
        } else {
            /* TODO: Just copy the rgb values ? SRC1_x may differ ... */
            blend.rt[0].alpha_func = blend.rt[0].rgb_func;
            blend.rt[0].alpha_src_factor = d3dblend_alpha_to_pipe_blendfactor(rs[D3DRS_SRCBLEND]);
            blend.rt[0].alpha_dst_factor = d3dblend_alpha_to_pipe_blendfactor(rs[D3DRS_DESTBLEND]);
        }
        nine_convert_blend_state_fixup(&blend, rs); /* for BOTH[INV]SRCALPHA */
    }

    blend.max_rt = 3; /* Upper bound. Could be optimized to fb->nr_cbufs for example */
    blend.rt[0].colormask = rs[D3DRS_COLORWRITEENABLE];

    if (rs[D3DRS_COLORWRITEENABLE1] != rs[D3DRS_COLORWRITEENABLE] ||
        rs[D3DRS_COLORWRITEENABLE2] != rs[D3DRS_COLORWRITEENABLE] ||
        rs[D3DRS_COLORWRITEENABLE3] != rs[D3DRS_COLORWRITEENABLE]) {
        unsigned i;
        blend.independent_blend_enable = true;
        for (i = 1; i < 4; ++i)
            blend.rt[i] = blend.rt[0];
        blend.rt[1].colormask = rs[D3DRS_COLORWRITEENABLE1];
        blend.rt[2].colormask = rs[D3DRS_COLORWRITEENABLE2];
        blend.rt[3].colormask = rs[D3DRS_COLORWRITEENABLE3];
    }

    /* blend.force_srgb = !!rs[D3DRS_SRGBWRITEENABLE]; */

    *blend_state = blend;
}

void
nine_convert_sampler_state(struct cso_context *ctx, int idx, const DWORD *ss)
{
    struct pipe_sampler_state samp;

    assert(idx >= 0 &&
           (idx < NINE_MAX_SAMPLERS_PS || idx >= NINE_SAMPLER_VS(0)) &&
           (idx < NINE_MAX_SAMPLERS));

    if (ss[D3DSAMP_MIPFILTER] != D3DTEXF_NONE) {
        samp.lod_bias = asfloat(ss[D3DSAMP_MIPMAPLODBIAS]);
        samp.min_lod = ss[NINED3DSAMP_MINLOD];
        samp.min_mip_filter = (ss[D3DSAMP_MIPFILTER] == D3DTEXF_POINT) ? PIPE_TEX_FILTER_NEAREST : PIPE_TEX_FILTER_LINEAR;
    } else {
        samp.min_lod = 0.0;
        samp.lod_bias = 0.0;
        samp.min_mip_filter = PIPE_TEX_MIPFILTER_NONE;
    }
    samp.max_lod = 15.0f;

    if (ss[NINED3DSAMP_CUBETEX]) {
        /* Cube textures are always clamped to edge on D3D */
        samp.wrap_s = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
        samp.wrap_t = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
        samp.wrap_r = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
    } else {
        samp.wrap_s = d3dtextureaddress_to_pipe_tex_wrap(ss[D3DSAMP_ADDRESSU]);
        samp.wrap_t = d3dtextureaddress_to_pipe_tex_wrap(ss[D3DSAMP_ADDRESSV]);
        samp.wrap_r = d3dtextureaddress_to_pipe_tex_wrap(ss[D3DSAMP_ADDRESSW]);
    }
    samp.min_img_filter = (ss[D3DSAMP_MINFILTER] == D3DTEXF_POINT && !ss[NINED3DSAMP_SHADOW]) ? PIPE_TEX_FILTER_NEAREST : PIPE_TEX_FILTER_LINEAR;
    samp.mag_img_filter = (ss[D3DSAMP_MAGFILTER] == D3DTEXF_POINT && !ss[NINED3DSAMP_SHADOW]) ? PIPE_TEX_FILTER_NEAREST : PIPE_TEX_FILTER_LINEAR;
    if (ss[D3DSAMP_MINFILTER] == D3DTEXF_ANISOTROPIC ||
        ss[D3DSAMP_MAGFILTER] == D3DTEXF_ANISOTROPIC)
        samp.max_anisotropy = MIN2(16, ss[D3DSAMP_MAXANISOTROPY]);
    else
        samp.max_anisotropy = 0;
    samp.compare_mode = ss[NINED3DSAMP_SHADOW] ? PIPE_TEX_COMPARE_R_TO_TEXTURE : PIPE_TEX_COMPARE_NONE;
    samp.compare_func = PIPE_FUNC_LEQUAL;
    samp.unnormalized_coords = 0;
    samp.seamless_cube_map = 0;
    samp.border_color_is_integer = 0;
    samp.reduction_mode = 0;
    samp.pad = 0;
    samp.border_color_format = PIPE_FORMAT_NONE;
    d3dcolor_to_pipe_color_union(&samp.border_color, ss[D3DSAMP_BORDERCOLOR]);

    /* see nine_state.h */
    if (idx < NINE_MAX_SAMPLERS_PS)
        cso_single_sampler(ctx, PIPE_SHADER_FRAGMENT, idx - NINE_SAMPLER_PS(0), &samp);
    else
        cso_single_sampler(ctx, PIPE_SHADER_VERTEX, idx - NINE_SAMPLER_VS(0), &samp);
}

const enum pipe_format nine_d3d9_to_pipe_format_map[120] =
{
   [D3DFMT_UNKNOWN]       = PIPE_FORMAT_NONE,
   [D3DFMT_R8G8B8]        = PIPE_FORMAT_R8G8B8_UNORM,
   [D3DFMT_A8R8G8B8]      = PIPE_FORMAT_B8G8R8A8_UNORM,
   [D3DFMT_X8R8G8B8]      = PIPE_FORMAT_B8G8R8X8_UNORM,
   [D3DFMT_R5G6B5]        = PIPE_FORMAT_B5G6R5_UNORM,
   [D3DFMT_X1R5G5B5]      = PIPE_FORMAT_B5G5R5X1_UNORM,
   [D3DFMT_A1R5G5B5]      = PIPE_FORMAT_B5G5R5A1_UNORM,
   [D3DFMT_A4R4G4B4]      = PIPE_FORMAT_B4G4R4A4_UNORM,
   [D3DFMT_R3G3B2]        = PIPE_FORMAT_B2G3R3_UNORM,
   [D3DFMT_A8]            = PIPE_FORMAT_A8_UNORM,
   [D3DFMT_A8R3G3B2]      = PIPE_FORMAT_NONE,
   [D3DFMT_X4R4G4B4]      = PIPE_FORMAT_B4G4R4X4_UNORM,
   [D3DFMT_A2B10G10R10]   = PIPE_FORMAT_R10G10B10A2_UNORM,
   [D3DFMT_A8B8G8R8]      = PIPE_FORMAT_R8G8B8A8_UNORM,
   [D3DFMT_X8B8G8R8]      = PIPE_FORMAT_R8G8B8X8_UNORM,
   [D3DFMT_G16R16]        = PIPE_FORMAT_R16G16_UNORM,
   [D3DFMT_A2R10G10B10]   = PIPE_FORMAT_B10G10R10A2_UNORM,
   [D3DFMT_A16B16G16R16]  = PIPE_FORMAT_R16G16B16A16_UNORM,
   [D3DFMT_A8P8]          = PIPE_FORMAT_NONE,
   [D3DFMT_P8]            = PIPE_FORMAT_NONE,
   [D3DFMT_L8]            = PIPE_FORMAT_L8_UNORM,
   [D3DFMT_A8L8]          = PIPE_FORMAT_L8A8_UNORM,
   [D3DFMT_A4L4]          = PIPE_FORMAT_L4A4_UNORM,
   [D3DFMT_V8U8]          = PIPE_FORMAT_R8G8_SNORM,
   [D3DFMT_L6V5U5]        = PIPE_FORMAT_NONE, /* Should be PIPE_FORMAT_R5SG5SB6U_NORM, but interpretation of the data differs a bit. */
   [D3DFMT_X8L8V8U8]      = PIPE_FORMAT_R8SG8SB8UX8U_NORM,
   [D3DFMT_Q8W8V8U8]      = PIPE_FORMAT_R8G8B8A8_SNORM,
   [D3DFMT_V16U16]        = PIPE_FORMAT_R16G16_SNORM,
   [D3DFMT_A2W10V10U10]   = PIPE_FORMAT_R10SG10SB10SA2U_NORM,
   [D3DFMT_D16_LOCKABLE]  = PIPE_FORMAT_Z16_UNORM,
   [D3DFMT_D32]           = PIPE_FORMAT_Z32_UNORM,
   [D3DFMT_D15S1]         = PIPE_FORMAT_NONE,
   [D3DFMT_D24S8]         = PIPE_FORMAT_S8_UINT_Z24_UNORM,
   [D3DFMT_D24X8]         = PIPE_FORMAT_X8Z24_UNORM,
   [D3DFMT_D24X4S4]       = PIPE_FORMAT_NONE,
   [D3DFMT_D16]           = PIPE_FORMAT_Z16_UNORM,
   [D3DFMT_D32F_LOCKABLE] = PIPE_FORMAT_Z32_FLOAT,
   [D3DFMT_D24FS8]        = PIPE_FORMAT_Z32_FLOAT_S8X24_UINT,
   [D3DFMT_D32_LOCKABLE]  = PIPE_FORMAT_NONE,
   [D3DFMT_S8_LOCKABLE]   = PIPE_FORMAT_NONE,
   [D3DFMT_L16]           = PIPE_FORMAT_L16_UNORM,
   [D3DFMT_VERTEXDATA]    = PIPE_FORMAT_NONE,
   [D3DFMT_INDEX16]       = PIPE_FORMAT_R16_UINT,
   [D3DFMT_INDEX32]       = PIPE_FORMAT_R32_UINT,
   [D3DFMT_Q16W16V16U16]  = PIPE_FORMAT_R16G16B16A16_SNORM,
   [D3DFMT_R16F]          = PIPE_FORMAT_R16_FLOAT,
   [D3DFMT_G16R16F]       = PIPE_FORMAT_R16G16_FLOAT,
   [D3DFMT_A16B16G16R16F] = PIPE_FORMAT_R16G16B16A16_FLOAT,
   [D3DFMT_R32F]          = PIPE_FORMAT_R32_FLOAT,
   [D3DFMT_G32R32F]       = PIPE_FORMAT_R32G32_FLOAT,
   [D3DFMT_A32B32G32R32F] = PIPE_FORMAT_R32G32B32A32_FLOAT,
   [D3DFMT_CxV8U8]        = PIPE_FORMAT_NONE,
   [D3DFMT_A1]            = PIPE_FORMAT_NONE,
   [D3DFMT_A2B10G10R10_XR_BIAS] = PIPE_FORMAT_NONE,
};

const D3DFORMAT nine_pipe_to_d3d9_format_map[PIPE_FORMAT_COUNT] =
{
   [PIPE_FORMAT_NONE]               = D3DFMT_UNKNOWN,
   /* TODO: rename PIPE_FORMAT_R8G8B8_UNORM to PIPE_FORMAT_B8G8R8_UNORM */
   [PIPE_FORMAT_R8G8B8_UNORM]       = D3DFMT_R8G8B8,
   [PIPE_FORMAT_B8G8R8A8_UNORM]     = D3DFMT_A8R8G8B8,
   [PIPE_FORMAT_B8G8R8X8_UNORM]     = D3DFMT_X8R8G8B8,
   [PIPE_FORMAT_B5G6R5_UNORM]       = D3DFMT_R5G6B5,
   [PIPE_FORMAT_B5G5R5X1_UNORM]     = D3DFMT_X1R5G5B5,
   [PIPE_FORMAT_B5G5R5A1_UNORM]     = D3DFMT_A1R5G5B5,
   [PIPE_FORMAT_B4G4R4A4_UNORM]     = D3DFMT_A4R4G4B4,
   [PIPE_FORMAT_B2G3R3_UNORM]       = D3DFMT_R3G3B2,
   [PIPE_FORMAT_A8_UNORM]           = D3DFMT_A8,
/* [PIPE_FORMAT_B2G3R3A8_UNORM]     = D3DFMT_A8R3G3B2, */
   [PIPE_FORMAT_B4G4R4X4_UNORM]     = D3DFMT_X4R4G4B4,
   [PIPE_FORMAT_R10G10B10A2_UNORM]  = D3DFMT_A2B10G10R10,
   [PIPE_FORMAT_R8G8B8A8_UNORM]     = D3DFMT_A8B8G8R8,
   [PIPE_FORMAT_R8G8B8X8_UNORM]     = D3DFMT_X8B8G8R8,
   [PIPE_FORMAT_R16G16_UNORM]       = D3DFMT_G16R16,
   [PIPE_FORMAT_B10G10R10A2_UNORM]  = D3DFMT_A2R10G10B10,
   [PIPE_FORMAT_R16G16B16A16_UNORM] = D3DFMT_A16B16G16R16,

   [PIPE_FORMAT_R8_UINT]            = D3DFMT_P8,
   [PIPE_FORMAT_R8A8_UINT]          = D3DFMT_A8P8,

   [PIPE_FORMAT_L8_UNORM]           = D3DFMT_L8,
   [PIPE_FORMAT_L8A8_UNORM]         = D3DFMT_A8L8,
   [PIPE_FORMAT_L4A4_UNORM]         = D3DFMT_A4L4,

   [PIPE_FORMAT_R8G8_SNORM]           = D3DFMT_V8U8,
/* [PIPE_FORMAT_?]                    = D3DFMT_L6V5U5, */
/* [PIPE_FORMAT_?]                    = D3DFMT_X8L8V8U8, */
   [PIPE_FORMAT_R8G8B8A8_SNORM]       = D3DFMT_Q8W8V8U8,
   [PIPE_FORMAT_R16G16_SNORM]         = D3DFMT_V16U16,
   [PIPE_FORMAT_R10SG10SB10SA2U_NORM] = D3DFMT_A2W10V10U10,

   [PIPE_FORMAT_YUYV]               = D3DFMT_UYVY,
/* [PIPE_FORMAT_YUY2]               = D3DFMT_YUY2, */
   [PIPE_FORMAT_DXT1_RGBA]          = D3DFMT_DXT1,
/* [PIPE_FORMAT_DXT2_RGBA]          = D3DFMT_DXT2, */
   [PIPE_FORMAT_DXT3_RGBA]          = D3DFMT_DXT3,
/* [PIPE_FORMAT_DXT4_RGBA]          = D3DFMT_DXT4, */
   [PIPE_FORMAT_DXT5_RGBA]          = D3DFMT_DXT5,
/* [PIPE_FORMAT_?]                  = D3DFMT_MULTI2_ARGB8, (MET) */
   [PIPE_FORMAT_R8G8_B8G8_UNORM]    = D3DFMT_R8G8_B8G8, /* XXX: order */
   [PIPE_FORMAT_G8R8_G8B8_UNORM]    = D3DFMT_G8R8_G8B8,

   [PIPE_FORMAT_Z16_UNORM]          = D3DFMT_D16_LOCKABLE,
   [PIPE_FORMAT_Z32_UNORM]          = D3DFMT_D32,
/* [PIPE_FORMAT_Z15_UNORM_S1_UINT]  = D3DFMT_D15S1, */
   [PIPE_FORMAT_S8_UINT_Z24_UNORM]  = D3DFMT_D24S8,
   [PIPE_FORMAT_X8Z24_UNORM]        = D3DFMT_D24X8,
   [PIPE_FORMAT_L16_UNORM]          = D3DFMT_L16,
   [PIPE_FORMAT_Z32_FLOAT]          = D3DFMT_D32F_LOCKABLE,
/* [PIPE_FORMAT_Z24_FLOAT_S8_UINT]  = D3DFMT_D24FS8, */

   [PIPE_FORMAT_R16_UINT]           = D3DFMT_INDEX16,
   [PIPE_FORMAT_R32_UINT]           = D3DFMT_INDEX32,
   [PIPE_FORMAT_R16G16B16A16_SNORM] = D3DFMT_Q16W16V16U16,

   [PIPE_FORMAT_R16_FLOAT]          = D3DFMT_R16F,
   [PIPE_FORMAT_R32_FLOAT]          = D3DFMT_R32F,
   [PIPE_FORMAT_R16G16_FLOAT]       = D3DFMT_G16R16F,
   [PIPE_FORMAT_R32G32_FLOAT]       = D3DFMT_G32R32F,
   [PIPE_FORMAT_R16G16B16A16_FLOAT] = D3DFMT_A16B16G16R16F,
   [PIPE_FORMAT_R32G32B32A32_FLOAT] = D3DFMT_A32B32G32R32F,

/* [PIPE_FORMAT_?]                  = D3DFMT_CxV8U8, */
};
