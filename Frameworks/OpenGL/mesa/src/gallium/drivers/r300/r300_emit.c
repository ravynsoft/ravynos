/*
 * Copyright 2008 Corbin Simpson <MostAwesomeDude@gmail.com>
 * Copyright 2009 Marek Olšák <maraeo@gmail.com>
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

/* r300_emit: Functions for emitting state. */

#include "util/format/u_format.h"
#include "util/u_math.h"

#include "r300_context.h"
#include "r300_cb.h"
#include "r300_cs.h"
#include "r300_emit.h"
#include "r300_fs.h"
#include "r300_screen.h"
#include "r300_screen_buffer.h"
#include "r300_vs.h"

void r300_emit_blend_state(struct r300_context* r300,
                           unsigned size, void* state)
{
    struct r300_blend_state* blend = (struct r300_blend_state*)state;
    struct pipe_framebuffer_state* fb =
        (struct pipe_framebuffer_state*)r300->fb_state.state;
    struct pipe_surface *cb;
    CS_LOCALS(r300);

    cb = fb->nr_cbufs ? r300_get_nonnull_cb(fb, 0) : NULL;

    if (cb) {
        if (cb->format == PIPE_FORMAT_R16G16B16A16_FLOAT) {
            WRITE_CS_TABLE(blend->cb_noclamp, size);
        } else if (cb->format == PIPE_FORMAT_R16G16B16X16_FLOAT) {
            WRITE_CS_TABLE(blend->cb_noclamp_noalpha, size);
        } else {
            unsigned swz = r300_surface(cb)->colormask_swizzle;
            WRITE_CS_TABLE(blend->cb_clamp[swz], size);
        }
    } else {
        WRITE_CS_TABLE(blend->cb_no_readwrite, size);
    }
}

void r300_emit_blend_color_state(struct r300_context* r300,
                                 unsigned size, void* state)
{
    struct r300_blend_color_state* bc = (struct r300_blend_color_state*)state;
    CS_LOCALS(r300);

    WRITE_CS_TABLE(bc->cb, size);
}

void r300_emit_clip_state(struct r300_context* r300,
                          unsigned size, void* state)
{
    struct r300_clip_state* clip = (struct r300_clip_state*)state;
    CS_LOCALS(r300);

    WRITE_CS_TABLE(clip->cb, size);
}

void r300_emit_dsa_state(struct r300_context* r300, unsigned size, void* state)
{
    struct r300_dsa_state* dsa = (struct r300_dsa_state*)state;
    struct pipe_framebuffer_state* fb =
        (struct pipe_framebuffer_state*)r300->fb_state.state;
    bool is_r500 = r300->screen->caps.is_r500;
    CS_LOCALS(r300);
    uint32_t alpha_func = dsa->alpha_function;

    /* Choose the alpha ref value between 8-bit (FG_ALPHA_FUNC.AM_VAL) and
     * 16-bit (FG_ALPHA_VALUE). */
    if (is_r500 && (alpha_func & R300_FG_ALPHA_FUNC_ENABLE)) {
        struct pipe_surface *cb = fb->nr_cbufs ? r300_get_nonnull_cb(fb, 0) : NULL;

        if (cb &&
            (cb->format == PIPE_FORMAT_R16G16B16A16_FLOAT ||
             cb->format == PIPE_FORMAT_R16G16B16X16_FLOAT)) {
            alpha_func |= R500_FG_ALPHA_FUNC_FP16_ENABLE;
        } else {
            alpha_func |= R500_FG_ALPHA_FUNC_8BIT;
        }
    }

    /* Setup alpha-to-coverage. */
    if (r300->alpha_to_coverage && r300->msaa_enable) {
        /* Always set 3/6, it improves precision even for 2x and 4x MSAA. */
        alpha_func |= R300_FG_ALPHA_FUNC_MASK_ENABLE |
                      R300_FG_ALPHA_FUNC_CFG_3_OF_6;
    }

    BEGIN_CS(size);
    OUT_CS_REG(R300_FG_ALPHA_FUNC, alpha_func);
    OUT_CS_TABLE(fb->zsbuf ? &dsa->cb_begin : dsa->cb_zb_no_readwrite, size-2);
    END_CS;
}

static void get_rc_constant_state(
    float vec[4],
    struct r300_context * r300,
    struct rc_constant * constant)
{
    struct r300_textures_state* texstate = r300->textures_state.state;
    struct r300_resource *tex;

    assert(constant->Type == RC_CONSTANT_STATE);

    /* vec should either be (0, 0, 0, 1), which should be a relatively safe
     * RGBA or STRQ value, or it could be one of the RC_CONSTANT_STATE
     * state factors. */

    switch (constant->u.State[0]) {
        /* Factor for converting rectangle coords to
         * normalized coords. Should only show up on non-r500. */
        case RC_STATE_R300_TEXRECT_FACTOR:
            tex = r300_resource(texstate->sampler_views[constant->u.State[1]]->base.texture);
            vec[0] = 1.0 / tex->tex.width0;
            vec[1] = 1.0 / tex->tex.height0;
            vec[2] = 0;
            vec[3] = 1;
            break;

        case RC_STATE_R300_TEXSCALE_FACTOR:
            tex = r300_resource(texstate->sampler_views[constant->u.State[1]]->base.texture);
            /* Add a small number to the texture size to work around rounding errors in hw. */
            vec[0] = tex->b.width0  / (tex->tex.width0  + 0.001f);
            vec[1] = tex->b.height0 / (tex->tex.height0 + 0.001f);
            vec[2] = tex->b.depth0  / (tex->tex.depth0  + 0.001f);
            vec[3] = 1;
            break;

        case RC_STATE_R300_VIEWPORT_SCALE:
            vec[0] = r300->viewport.scale[0];
            vec[1] = r300->viewport.scale[1];
            vec[2] = r300->viewport.scale[2];
            vec[3] = 1;
            break;

        case RC_STATE_R300_VIEWPORT_OFFSET:
            vec[0] = r300->viewport.translate[0];
            vec[1] = r300->viewport.translate[1];
            vec[2] = r300->viewport.translate[2];
            vec[3] = 1;
            break;

        default:
            fprintf(stderr, "r300: Implementation error: "
                "Unknown RC_CONSTANT type %d\n", constant->u.State[0]);
            vec[0] = 0;
            vec[1] = 0;
            vec[2] = 0;
            vec[3] = 1;
    }
}

/* Convert a normal single-precision float into the 7.16 format
 * used by the R300 fragment shader.
 */
uint32_t pack_float24(float f)
{
    union {
        float fl;
        uint32_t u;
    } u;
    float mantissa;
    int exponent;
    uint32_t float24 = 0;

    if (f == 0.0)
        return 0;

    u.fl = f;

    mantissa = frexpf(f, &exponent);

    /* Handle -ve */
    if (mantissa < 0) {
        float24 |= (1 << 23);
        mantissa = mantissa * -1.0;
    }
    /* Handle exponent, bias of 63 */
    exponent += 62;
    float24 |= (exponent << 16);
    /* Kill 7 LSB of mantissa */
    float24 |= (u.u & 0x7FFFFF) >> 7;

    return float24;
}

void r300_emit_fs(struct r300_context* r300, unsigned size, void *state)
{
    struct r300_fragment_shader *fs = r300_fs(r300);
    CS_LOCALS(r300);

    WRITE_CS_TABLE(fs->shader->cb_code, fs->shader->cb_code_size);
}

void r300_emit_fs_constants(struct r300_context* r300, unsigned size, void *state)
{
    struct r300_fragment_shader *fs = r300_fs(r300);
    struct r300_constant_buffer *buf = (struct r300_constant_buffer*)state;
    unsigned count = fs->shader->externals_count;
    unsigned i, j;
    CS_LOCALS(r300);

    if (count == 0)
        return;

    BEGIN_CS(size);
    OUT_CS_REG_SEQ(R300_PFS_PARAM_0_X, count * 4);
    if (buf->remap_table){
        for (i = 0; i < count; i++) {
            float *data = (float*)&buf->ptr[buf->remap_table[i]*4];
            for (j = 0; j < 4; j++)
                OUT_CS(pack_float24(data[j]));
        }
    } else {
        for (i = 0; i < count; i++)
            for (j = 0; j < 4; j++)
                OUT_CS(pack_float24(*(float*)&buf->ptr[i*4+j]));
    }

    END_CS;
}

void r300_emit_fs_rc_constant_state(struct r300_context* r300, unsigned size, void *state)
{
    struct r300_fragment_shader *fs = r300_fs(r300);
    struct rc_constant_list *constants = &fs->shader->code.constants;
    unsigned i;
    unsigned count = fs->shader->rc_state_count;
    unsigned first = fs->shader->externals_count;
    unsigned end = constants->Count;
    unsigned j;
    CS_LOCALS(r300);

    if (count == 0)
        return;

    BEGIN_CS(size);
    for(i = first; i < end; ++i) {
        if (constants->Constants[i].Type == RC_CONSTANT_STATE) {
            float data[4];

            get_rc_constant_state(data, r300, &constants->Constants[i]);

            OUT_CS_REG_SEQ(R300_PFS_PARAM_0_X + i * 16, 4);
            for (j = 0; j < 4; j++)
                OUT_CS(pack_float24(data[j]));
        }
    }
    END_CS;
}

void r500_emit_fs(struct r300_context* r300, unsigned size, void *state)
{
    struct r300_fragment_shader *fs = r300_fs(r300);
    CS_LOCALS(r300);

    WRITE_CS_TABLE(fs->shader->cb_code, fs->shader->cb_code_size);
}

void r500_emit_fs_constants(struct r300_context* r300, unsigned size, void *state)
{
    struct r300_fragment_shader *fs = r300_fs(r300);
    struct r300_constant_buffer *buf = (struct r300_constant_buffer*)state;
    unsigned count = fs->shader->externals_count;
    CS_LOCALS(r300);

    if (count == 0)
        return;

    BEGIN_CS(size);
    OUT_CS_REG(R500_GA_US_VECTOR_INDEX, R500_GA_US_VECTOR_INDEX_TYPE_CONST);
    OUT_CS_ONE_REG(R500_GA_US_VECTOR_DATA, count * 4);
    if (buf->remap_table){
        for (unsigned i = 0; i < count; i++) {
            uint32_t *data = &buf->ptr[buf->remap_table[i]*4];
            OUT_CS_TABLE(data, 4);
        }
    } else {
        OUT_CS_TABLE(buf->ptr, count * 4);
    }
    END_CS;
}

void r500_emit_fs_rc_constant_state(struct r300_context* r300, unsigned size, void *state)
{
    struct r300_fragment_shader *fs = r300_fs(r300);
    struct rc_constant_list *constants = &fs->shader->code.constants;
    unsigned i;
    unsigned count = fs->shader->rc_state_count;
    unsigned first = fs->shader->externals_count;
    unsigned end = constants->Count;
    CS_LOCALS(r300);

    if (count == 0)
        return;

    BEGIN_CS(size);
    for(i = first; i < end; ++i) {
        if (constants->Constants[i].Type == RC_CONSTANT_STATE) {
            float data[4];

            get_rc_constant_state(data, r300, &constants->Constants[i]);

            OUT_CS_REG(R500_GA_US_VECTOR_INDEX,
                       R500_GA_US_VECTOR_INDEX_TYPE_CONST |
                       (i & R500_GA_US_VECTOR_INDEX_MASK));
            OUT_CS_ONE_REG(R500_GA_US_VECTOR_DATA, 4);
            OUT_CS_TABLE(data, 4);
        }
    }
    END_CS;
}

void r300_emit_gpu_flush(struct r300_context *r300, unsigned size, void *state)
{
    struct r300_gpu_flush *gpuflush = (struct r300_gpu_flush*)state;
    struct pipe_framebuffer_state* fb =
            (struct pipe_framebuffer_state*)r300->fb_state.state;
    uint32_t height = fb->height;
    uint32_t width = fb->width;
    CS_LOCALS(r300);

    if (r300->cbzb_clear) {
        struct r300_surface *surf = r300_surface(fb->cbufs[0]);

        height = surf->cbzb_height;
        width = surf->cbzb_width;
    }

    DBG(r300, DBG_SCISSOR,
	"r300: Scissor width: %i, height: %i, CBZB clear: %s\n",
	width, height, r300->cbzb_clear ? "YES" : "NO");

    BEGIN_CS(size);

    /* Set up scissors.
     * By writing to the SC registers, SC & US assert idle. */
    OUT_CS_REG_SEQ(R300_SC_SCISSORS_TL, 2);
    if (r300->screen->caps.is_r500) {
        OUT_CS(0);
        OUT_CS(((width  - 1) << R300_SCISSORS_X_SHIFT) |
               ((height - 1) << R300_SCISSORS_Y_SHIFT));
    } else {
        OUT_CS((1440 << R300_SCISSORS_X_SHIFT) |
               (1440 << R300_SCISSORS_Y_SHIFT));
        OUT_CS(((width  + 1440-1) << R300_SCISSORS_X_SHIFT) |
               ((height + 1440-1) << R300_SCISSORS_Y_SHIFT));
    }

    /* Flush CB & ZB caches and wait until the 3D engine is idle and clean. */
    OUT_CS_TABLE(gpuflush->cb_flush_clean, 6);
    END_CS;
}

void r300_emit_aa_state(struct r300_context *r300, unsigned size, void *state)
{
    struct r300_aa_state *aa = (struct r300_aa_state*)state;
    CS_LOCALS(r300);

    BEGIN_CS(size);
    OUT_CS_REG(R300_GB_AA_CONFIG, aa->aa_config);

    if (aa->dest) {
        OUT_CS_REG_SEQ(R300_RB3D_AARESOLVE_OFFSET, 3);
        OUT_CS(aa->dest->offset);
        OUT_CS(aa->dest->pitch & R300_RB3D_AARESOLVE_PITCH_MASK);
        OUT_CS(R300_RB3D_AARESOLVE_CTL_AARESOLVE_MODE_RESOLVE |
               R300_RB3D_AARESOLVE_CTL_AARESOLVE_ALPHA_AVERAGE);
        OUT_CS_RELOC(aa->dest);
    } else {
        OUT_CS_REG(R300_RB3D_AARESOLVE_CTL, 0);
    }

    END_CS;
}

void r300_emit_fb_state(struct r300_context* r300, unsigned size, void* state)
{
    struct pipe_framebuffer_state* fb = (struct pipe_framebuffer_state*)state;
    struct r300_surface* surf;
    unsigned i;
    uint32_t rb3d_cctl = 0;

    CS_LOCALS(r300);

    BEGIN_CS(size);

    if (r300->screen->caps.is_r500) {
        rb3d_cctl = R300_RB3D_CCTL_INDEPENDENT_COLORFORMAT_ENABLE_ENABLE;
    }
    /* NUM_MULTIWRITES replicates COLOR[0] to all colorbuffers. */
    if (fb->nr_cbufs && r300->fb_multiwrite) {
        rb3d_cctl |= R300_RB3D_CCTL_NUM_MULTIWRITES(fb->nr_cbufs);
    }
    if (r300->cmask_in_use) {
        rb3d_cctl |= R300_RB3D_CCTL_AA_COMPRESSION_ENABLE |
                     R300_RB3D_CCTL_CMASK_ENABLE;
    }

    OUT_CS_REG(R300_RB3D_CCTL, rb3d_cctl);

    /* Set up colorbuffers. */
    for (i = 0; i < fb->nr_cbufs; i++) {
        surf = r300_surface(r300_get_nonnull_cb(fb, i));

        OUT_CS_REG(R300_RB3D_COLOROFFSET0 + (4 * i), surf->offset);
        OUT_CS_RELOC(surf);

        OUT_CS_REG(R300_RB3D_COLORPITCH0 + (4 * i), surf->pitch);
        OUT_CS_RELOC(surf);

        if (r300->cmask_in_use && i == 0) {
            OUT_CS_REG(R300_RB3D_CMASK_OFFSET0, 0);
            OUT_CS_REG(R300_RB3D_CMASK_PITCH0, surf->pitch_cmask);
            OUT_CS_REG(R300_RB3D_COLOR_CLEAR_VALUE, r300->color_clear_value);
            if (r300->screen->caps.is_r500) {
                OUT_CS_REG_SEQ(R500_RB3D_COLOR_CLEAR_VALUE_AR, 2);
                OUT_CS(r300->color_clear_value_ar);
                OUT_CS(r300->color_clear_value_gb);
            }
        }
    }

    /* Set up the ZB part of the CBZB clear. */
    if (r300->cbzb_clear) {
        surf = r300_surface(fb->cbufs[0]);

        OUT_CS_REG(R300_ZB_FORMAT, surf->cbzb_format);

        OUT_CS_REG(R300_ZB_DEPTHOFFSET, surf->cbzb_midpoint_offset);
        OUT_CS_RELOC(surf);

        OUT_CS_REG(R300_ZB_DEPTHPITCH, surf->cbzb_pitch);
        OUT_CS_RELOC(surf);

        DBG(r300, DBG_CBZB,
            "CBZB clearing cbuf %08x %08x\n", surf->cbzb_format,
            surf->cbzb_pitch);
    }
    /* Set up a zbuffer. */
    else if (fb->zsbuf) {
        surf = r300_surface(fb->zsbuf);

        OUT_CS_REG(R300_ZB_FORMAT, surf->format);

        OUT_CS_REG(R300_ZB_DEPTHOFFSET, surf->offset);
        OUT_CS_RELOC(surf);

        OUT_CS_REG(R300_ZB_DEPTHPITCH, surf->pitch);
        OUT_CS_RELOC(surf);

        if (r300->hyperz_enabled) {
            /* HiZ RAM. */
            OUT_CS_REG(R300_ZB_HIZ_OFFSET, 0);
            OUT_CS_REG(R300_ZB_HIZ_PITCH, surf->pitch_hiz);
            /* Z Mask RAM. (compressed zbuffer) */
            OUT_CS_REG(R300_ZB_ZMASK_OFFSET, 0);
            OUT_CS_REG(R300_ZB_ZMASK_PITCH, surf->pitch_zmask);
        }
    }

    END_CS;
}

void r300_emit_hyperz_state(struct r300_context *r300,
                            unsigned size, void *state)
{
    struct r300_hyperz_state *z = state;
    CS_LOCALS(r300);

    if (z->flush)
        WRITE_CS_TABLE(&z->cb_flush_begin, size);
    else
        WRITE_CS_TABLE(&z->cb_begin, size - 2);
}

void r300_emit_hyperz_end(struct r300_context *r300)
{
    struct r300_hyperz_state z =
            *(struct r300_hyperz_state*)r300->hyperz_state.state;

    z.flush = 1;
    z.zb_bw_cntl = 0;
    z.zb_depthclearvalue = 0;
    z.sc_hyperz = R300_SC_HYPERZ_ADJ_2;
    z.gb_z_peq_config = 0;

    r300_emit_hyperz_state(r300, r300->hyperz_state.size, &z);
}

#define R300_NIBBLES(x0, y0, x1, y1, x2, y2, d0y, d0x)  \
    (((x0) & 0xf) | (((y0) & 0xf) << 4) |		   \
    (((x1) & 0xf) << 8) | (((y1) & 0xf) << 12) |	   \
    (((x2) & 0xf) << 16) | (((y2) & 0xf) << 20) |	   \
    (((d0y) & 0xf) << 24) | (((d0x) & 0xf) << 28))

static unsigned r300_get_mspos(int index, unsigned *p)
{
    unsigned reg, i, distx, disty, dist;

    if (index == 0) {
        /* MSPOS0 contains positions for samples 0,1,2 as (X,Y) pairs of nibbles,
         * followed by a (Y,X) pair containing the minimum distance from the pixel
         * edge:
         *     X0, Y0, X1, Y1, X2, Y2, D0_Y, D0_X
         *
         * There is a quirk when setting D0_X. The value represents the distance
         * from the left edge of the pixel quad to the first sample in subpixels.
         * All values less than eight should use the actual value, but „7‟ should
         * be used for the distance „8‟. The hardware will convert 7 into 8 internally.
         */
        distx = 11;
        for (i = 0; i < 12; i += 2) {
            if (p[i] < distx)
                distx = p[i];
        }

        disty = 11;
        for (i = 1; i < 12; i += 2) {
            if (p[i] < disty)
                disty = p[i];
        }

        if (distx == 8)
            distx = 7;

        reg = R300_NIBBLES(p[0], p[1], p[2], p[3], p[4], p[5], disty, distx);
    } else {
        /* MSPOS1 contains positions for samples 3,4,5 as (X,Y) pairs of nibbles,
         * followed by the minimum distance from the pixel edge (not sure if X or Y):
         *     X3, Y3, X4, Y4, X5, Y5, D1
         */
        dist = 11;
        for (i = 0; i < 12; i++) {
            if (p[i] < dist)
                dist = p[i];
        }

        reg = R300_NIBBLES(p[6], p[7], p[8], p[9], p[10], p[11], dist, 0);
    }
    return reg;
}

void r300_emit_fb_state_pipelined(struct r300_context *r300,
                                  unsigned size, void *state)
{
    /* The sample coordinates are in the range [0,11], because
     * GB_TILE_CONFIG.SUBPIXEL is set to the 1/12 subpixel precision.
     *
     * Some sample coordinates reach to neighboring pixels and should not be used.
     * (e.g. Y=11)
     *
     * The unused samples must be set to the positions of other valid samples. */
    static unsigned sample_locs_1x[12] = {
        6,6,  6,6,  6,6,  6,6,  6,6,  6,6
    };
    static unsigned sample_locs_2x[12] = {
        3,9,  9,3,  9,3,  9,3,  9,3,  9,3
    };
    static unsigned sample_locs_4x[12] = {
        4,4,  8,8,  2,10,  10,2,  10,2,  10,2
    };
    static unsigned sample_locs_6x[12] = {
        3,1,  7,3,  11,5,  1,7,  5,9,  9,10
    };

    struct pipe_framebuffer_state* fb =
            (struct pipe_framebuffer_state*)r300->fb_state.state;
    unsigned i, num_cbufs = fb->nr_cbufs;
    unsigned mspos0, mspos1;
    CS_LOCALS(r300);

    /* If we use the multiwrite feature, the colorbuffers 2,3,4 must be
     * marked as UNUSED in the US block. */
    if (r300->fb_multiwrite) {
        num_cbufs = MIN2(num_cbufs, 1);
    }

    BEGIN_CS(size);

    /* Colorbuffer format in the US block.
     * (must be written after unpipelined regs) */
    OUT_CS_REG_SEQ(R300_US_OUT_FMT_0, 4);
    for (i = 0; i < num_cbufs; i++) {
        OUT_CS(r300_surface(r300_get_nonnull_cb(fb, i))->format);
    }
    for (; i < 1; i++) {
        OUT_CS(R300_US_OUT_FMT_C4_8 |
               R300_C0_SEL_B | R300_C1_SEL_G |
               R300_C2_SEL_R | R300_C3_SEL_A);
    }
    for (; i < 4; i++) {
        OUT_CS(R300_US_OUT_FMT_UNUSED);
    }

    /* Set sample positions. It depends on the framebuffer sample count.
     * These are pipelined regs and as such cannot be moved to the AA state.
     */
    switch (r300->num_samples) {
    default:
        mspos0 = r300_get_mspos(0, sample_locs_1x);
        mspos1 = r300_get_mspos(1, sample_locs_1x);
        break;
    case 2:
        mspos0 = r300_get_mspos(0, sample_locs_2x);
        mspos1 = r300_get_mspos(1, sample_locs_2x);
        break;
    case 4:
        mspos0 = r300_get_mspos(0, sample_locs_4x);
        mspos1 = r300_get_mspos(1, sample_locs_4x);
        break;
    case 6:
        mspos0 = r300_get_mspos(0, sample_locs_6x);
        mspos1 = r300_get_mspos(1, sample_locs_6x);
        break;
    }

    OUT_CS_REG_SEQ(R300_GB_MSPOS0, 2);
    OUT_CS(mspos0);
    OUT_CS(mspos1);
    END_CS;
}

void r300_emit_query_start(struct r300_context *r300, unsigned size, void*state)
{
    struct r300_query *query = r300->query_current;
    CS_LOCALS(r300);

    if (!query)
	return;

    BEGIN_CS(size);
    if (r300->screen->caps.family == CHIP_RV530) {
        OUT_CS_REG(RV530_FG_ZBREG_DEST, RV530_FG_ZBREG_DEST_PIPE_SELECT_ALL);
    } else {
        OUT_CS_REG(R300_SU_REG_DEST, R300_RASTER_PIPE_SELECT_ALL);
    }
    OUT_CS_REG(R300_ZB_ZPASS_DATA, 0);
    END_CS;
    query->begin_emitted = true;
}

static void r300_emit_query_end_frag_pipes(struct r300_context *r300,
                                           struct r300_query *query)
{
    struct r300_capabilities* caps = &r300->screen->caps;
    uint32_t gb_pipes = r300->screen->info.r300_num_gb_pipes;
    CS_LOCALS(r300);

    assert(gb_pipes);

    BEGIN_CS(6 * gb_pipes + 2);
    /* I'm not so sure I like this switch, but it's hard to be elegant
     * when there's so many special cases...
     *
     * So here's the basic idea. For each pipe, enable writes to it only,
     * then put out the relocation for ZPASS_ADDR, taking into account a
     * 4-byte offset for each pipe. RV380 and older are special; they have
     * only two pipes, and the second pipe's enable is on bit 3, not bit 1,
     * so there's a chipset cap for that. */
    switch (gb_pipes) {
        case 4:
            /* pipe 3 only */
            OUT_CS_REG(R300_SU_REG_DEST, 1 << 3);
            OUT_CS_REG(R300_ZB_ZPASS_ADDR, (query->num_results + 3) * 4);
            OUT_CS_RELOC(r300->query_current);
            FALLTHROUGH;
        case 3:
            /* pipe 2 only */
            OUT_CS_REG(R300_SU_REG_DEST, 1 << 2);
            OUT_CS_REG(R300_ZB_ZPASS_ADDR, (query->num_results + 2) * 4);
            OUT_CS_RELOC(r300->query_current);
            FALLTHROUGH;
        case 2:
            /* pipe 1 only */
            /* As mentioned above, accommodate RV380 and older. */
            OUT_CS_REG(R300_SU_REG_DEST,
                    1 << (caps->high_second_pipe ? 3 : 1));
            OUT_CS_REG(R300_ZB_ZPASS_ADDR, (query->num_results + 1) * 4);
            OUT_CS_RELOC(r300->query_current);
            FALLTHROUGH;
        case 1:
            /* pipe 0 only */
            OUT_CS_REG(R300_SU_REG_DEST, 1 << 0);
            OUT_CS_REG(R300_ZB_ZPASS_ADDR, (query->num_results + 0) * 4);
            OUT_CS_RELOC(r300->query_current);
            break;
        default:
            fprintf(stderr, "r300: Implementation error: Chipset reports %d"
                    " pixel pipes!\n", gb_pipes);
            abort();
    }

    /* And, finally, reset it to normal... */
    OUT_CS_REG(R300_SU_REG_DEST, 0xF);
    END_CS;
}

static void rv530_emit_query_end_single_z(struct r300_context *r300,
                                          struct r300_query *query)
{
    CS_LOCALS(r300);

    BEGIN_CS(8);
    OUT_CS_REG(RV530_FG_ZBREG_DEST, RV530_FG_ZBREG_DEST_PIPE_SELECT_0);
    OUT_CS_REG(R300_ZB_ZPASS_ADDR, query->num_results * 4);
    OUT_CS_RELOC(r300->query_current);
    OUT_CS_REG(RV530_FG_ZBREG_DEST, RV530_FG_ZBREG_DEST_PIPE_SELECT_ALL);
    END_CS;
}

static void rv530_emit_query_end_double_z(struct r300_context *r300,
                                          struct r300_query *query)
{
    CS_LOCALS(r300);

    BEGIN_CS(14);
    OUT_CS_REG(RV530_FG_ZBREG_DEST, RV530_FG_ZBREG_DEST_PIPE_SELECT_0);
    OUT_CS_REG(R300_ZB_ZPASS_ADDR, (query->num_results + 0) * 4);
    OUT_CS_RELOC(r300->query_current);
    OUT_CS_REG(RV530_FG_ZBREG_DEST, RV530_FG_ZBREG_DEST_PIPE_SELECT_1);
    OUT_CS_REG(R300_ZB_ZPASS_ADDR, (query->num_results + 1) * 4);
    OUT_CS_RELOC(r300->query_current);
    OUT_CS_REG(RV530_FG_ZBREG_DEST, RV530_FG_ZBREG_DEST_PIPE_SELECT_ALL);
    END_CS;
}

void r300_emit_query_end(struct r300_context* r300)
{
    struct r300_capabilities *caps = &r300->screen->caps;
    struct r300_query *query = r300->query_current;

    if (!query)
	return;

    if (query->begin_emitted == false)
        return;

    if (caps->family == CHIP_RV530) {
        if (r300->screen->info.r300_num_z_pipes == 2)
            rv530_emit_query_end_double_z(r300, query);
        else
            rv530_emit_query_end_single_z(r300, query);
    } else 
        r300_emit_query_end_frag_pipes(r300, query);

    query->begin_emitted = false;
    query->num_results += query->num_pipes;

    /* XXX grab all the results and reset the counter. */
    if (query->num_results >= query->buf->size / 4 - 4) {
        query->num_results = (query->buf->size / 4) / 2;
        fprintf(stderr, "r300: Rewinding OQBO...\n");
    }
}

void r300_emit_invariant_state(struct r300_context *r300,
                               unsigned size, void *state)
{
    CS_LOCALS(r300);
    WRITE_CS_TABLE(state, size);
}

void r300_emit_rs_state(struct r300_context* r300, unsigned size, void* state)
{
    struct r300_rs_state* rs = state;
    CS_LOCALS(r300);

    BEGIN_CS(size);
    OUT_CS_TABLE(rs->cb_main, RS_STATE_MAIN_SIZE);
    if (rs->polygon_offset_enable) {
        if (r300->zbuffer_bpp == 16) {
            OUT_CS_TABLE(rs->cb_poly_offset_zb16, 5);
        } else {
            OUT_CS_TABLE(rs->cb_poly_offset_zb24, 5);
        }
    }
    END_CS;
}

void r300_emit_rs_block_state(struct r300_context* r300,
                              unsigned size, void* state)
{
    struct r300_rs_block* rs = (struct r300_rs_block*)state;
    unsigned i;
    /* It's the same for both INST and IP tables */
    unsigned count = (rs->inst_count & R300_RS_INST_COUNT_MASK) + 1;
    CS_LOCALS(r300);

    if (DBG_ON(r300, DBG_RS_BLOCK)) {
        r500_dump_rs_block(rs);

        fprintf(stderr, "r300: RS emit:\n");

        for (i = 0; i < count; i++)
            fprintf(stderr, "    : ip %d: 0x%08x\n", i, rs->ip[i]);

        for (i = 0; i < count; i++)
            fprintf(stderr, "    : inst %d: 0x%08x\n", i, rs->inst[i]);

        fprintf(stderr, "    : count: 0x%08x inst_count: 0x%08x\n",
            rs->count, rs->inst_count);
    }

    BEGIN_CS(size);
    OUT_CS_REG_SEQ(R300_VAP_VTX_STATE_CNTL, 2);
    OUT_CS(rs->vap_vtx_state_cntl);
    OUT_CS(rs->vap_vsm_vtx_assm);
    OUT_CS_REG_SEQ(R300_VAP_OUTPUT_VTX_FMT_0, 2);
    OUT_CS(rs->vap_out_vtx_fmt[0]);
    OUT_CS(rs->vap_out_vtx_fmt[1]);
    OUT_CS_REG_SEQ(R300_GB_ENABLE, 1);
    OUT_CS(rs->gb_enable);

    if (r300->screen->caps.is_r500) {
        OUT_CS_REG_SEQ(R500_RS_IP_0, count);
    } else {
        OUT_CS_REG_SEQ(R300_RS_IP_0, count);
    }
    OUT_CS_TABLE(rs->ip, count);

    OUT_CS_REG_SEQ(R300_RS_COUNT, 2);
    OUT_CS(rs->count);
    OUT_CS(rs->inst_count);

    if (r300->screen->caps.is_r500) {
        OUT_CS_REG_SEQ(R500_RS_INST_0, count);
    } else {
        OUT_CS_REG_SEQ(R300_RS_INST_0, count);
    }
    OUT_CS_TABLE(rs->inst, count);
    END_CS;
}

void r300_emit_sample_mask(struct r300_context *r300,
                           unsigned size, void *state)
{
    unsigned mask = (*(unsigned*)state) & ((1 << 6)-1);
    CS_LOCALS(r300);

    BEGIN_CS(size);
    OUT_CS_REG(R300_SC_SCREENDOOR,
               mask | (mask << 6) | (mask << 12) | (mask << 18));
    END_CS;
}

void r300_emit_scissor_state(struct r300_context* r300,
                             unsigned size, void* state)
{
    struct pipe_scissor_state* scissor = (struct pipe_scissor_state*)state;
    CS_LOCALS(r300);

    BEGIN_CS(size);
    OUT_CS_REG_SEQ(R300_SC_CLIPRECT_TL_0, 2);
    if (r300->screen->caps.is_r500) {
        OUT_CS((scissor->minx << R300_CLIPRECT_X_SHIFT) |
               (scissor->miny << R300_CLIPRECT_Y_SHIFT));
        OUT_CS(((scissor->maxx - 1) << R300_CLIPRECT_X_SHIFT) |
               ((scissor->maxy - 1) << R300_CLIPRECT_Y_SHIFT));
    } else {
        OUT_CS(((scissor->minx + 1440) << R300_CLIPRECT_X_SHIFT) |
               ((scissor->miny + 1440) << R300_CLIPRECT_Y_SHIFT));
        OUT_CS(((scissor->maxx + 1440-1) << R300_CLIPRECT_X_SHIFT) |
               ((scissor->maxy + 1440-1) << R300_CLIPRECT_Y_SHIFT));
    }
    END_CS;
}

void r300_emit_textures_state(struct r300_context *r300,
                              unsigned size, void *state)
{
    struct r300_textures_state *allstate = (struct r300_textures_state*)state;
    struct r300_texture_sampler_state *texstate;
    struct r300_resource *tex;
    unsigned i;
    bool has_us_format = r300->screen->caps.has_us_format;
    CS_LOCALS(r300);

    BEGIN_CS(size);
    OUT_CS_REG(R300_TX_ENABLE, allstate->tx_enable);

    for (i = 0; i < allstate->count; i++) {
        if ((1 << i) & allstate->tx_enable) {
            texstate = &allstate->regs[i];
            tex = r300_resource(allstate->sampler_views[i]->base.texture);

            OUT_CS_REG(R300_TX_FILTER0_0 + (i * 4), texstate->filter0);
            OUT_CS_REG(R300_TX_FILTER1_0 + (i * 4), texstate->filter1);
            OUT_CS_REG(R300_TX_BORDER_COLOR_0 + (i * 4),
                       texstate->border_color);

            OUT_CS_REG(R300_TX_FORMAT0_0 + (i * 4), texstate->format.format0);
            OUT_CS_REG(R300_TX_FORMAT1_0 + (i * 4), texstate->format.format1);
            OUT_CS_REG(R300_TX_FORMAT2_0 + (i * 4), texstate->format.format2);

            OUT_CS_REG(R300_TX_OFFSET_0 + (i * 4), texstate->format.tile_config);
            OUT_CS_RELOC(tex);

            if (has_us_format) {
                OUT_CS_REG(R500_US_FORMAT0_0 + (i * 4),
                           texstate->format.us_format0);
            }
        }
    }
    END_CS;
}

void r300_emit_vertex_arrays(struct r300_context* r300, int offset,
                             bool indexed, int instance_id)
{
    struct pipe_vertex_buffer *vbuf = r300->vertex_buffer;
    struct pipe_vertex_element *velem = r300->velems->velem;
    struct r300_resource *buf;
    int i;
    unsigned vertex_array_count = r300->velems->count;
    unsigned packet_size = (vertex_array_count * 3 + 1) / 2;
    struct pipe_vertex_buffer *vb1, *vb2;
    unsigned *hw_format_size = r300->velems->format_size;
    unsigned size1, size2, offset1, offset2, stride1, stride2;
    CS_LOCALS(r300);

    BEGIN_CS(2 + packet_size + vertex_array_count * 2);
    OUT_CS_PKT3(R300_PACKET3_3D_LOAD_VBPNTR, packet_size);
    OUT_CS(vertex_array_count | (!indexed ? R300_VC_FORCE_PREFETCH : 0));

    if (instance_id == -1) {
        /* Non-instanced arrays. This ignores instance_divisor and instance_id. */
        for (i = 0; i < vertex_array_count - 1; i += 2) {
            vb1 = &vbuf[velem[i].vertex_buffer_index];
            vb2 = &vbuf[velem[i+1].vertex_buffer_index];
            size1 = hw_format_size[i];
            size2 = hw_format_size[i+1];

            OUT_CS(R300_VBPNTR_SIZE0(size1) | R300_VBPNTR_STRIDE0(velem[i].src_stride) |
                   R300_VBPNTR_SIZE1(size2) | R300_VBPNTR_STRIDE1(velem[i+1].src_stride));
            OUT_CS(vb1->buffer_offset + velem[i].src_offset   + offset * velem[i].src_stride);
            OUT_CS(vb2->buffer_offset + velem[i+1].src_offset + offset * velem[i+1].src_stride);
        }

        if (vertex_array_count & 1) {
            vb1 = &vbuf[velem[i].vertex_buffer_index];
            size1 = hw_format_size[i];

            OUT_CS(R300_VBPNTR_SIZE0(size1) | R300_VBPNTR_STRIDE0(velem[i].src_stride));
            OUT_CS(vb1->buffer_offset + velem[i].src_offset + offset * velem[i].src_stride);
        }

        for (i = 0; i < vertex_array_count; i++) {
            buf = r300_resource(vbuf[velem[i].vertex_buffer_index].buffer.resource);
            OUT_CS_RELOC(buf);
        }
    } else {
        /* Instanced arrays. */
        for (i = 0; i < vertex_array_count - 1; i += 2) {
            vb1 = &vbuf[velem[i].vertex_buffer_index];
            vb2 = &vbuf[velem[i+1].vertex_buffer_index];
            size1 = hw_format_size[i];
            size2 = hw_format_size[i+1];

            if (velem[i].instance_divisor) {
                stride1 = 0;
                offset1 = vb1->buffer_offset + velem[i].src_offset +
                          (instance_id / velem[i].instance_divisor) * velem[i].src_stride;
            } else {
                stride1 = velem[i].src_stride;
                offset1 = vb1->buffer_offset + velem[i].src_offset + offset * velem[i].src_stride;
            }
            if (velem[i+1].instance_divisor) {
                stride2 = 0;
                offset2 = vb2->buffer_offset + velem[i+1].src_offset +
                          (instance_id / velem[i+1].instance_divisor) * velem[i+1].src_stride;
            } else {
                stride2 = velem[i+1].src_stride;
                offset2 = vb2->buffer_offset + velem[i+1].src_offset + offset * velem[i+1].src_stride;
            }

            OUT_CS(R300_VBPNTR_SIZE0(size1) | R300_VBPNTR_STRIDE0(stride1) |
                   R300_VBPNTR_SIZE1(size2) | R300_VBPNTR_STRIDE1(stride2));
            OUT_CS(offset1);
            OUT_CS(offset2);
        }

        if (vertex_array_count & 1) {
            vb1 = &vbuf[velem[i].vertex_buffer_index];
            size1 = hw_format_size[i];

            if (velem[i].instance_divisor) {
                stride1 = 0;
                offset1 = vb1->buffer_offset + velem[i].src_offset +
                          (instance_id / velem[i].instance_divisor) * velem[i].src_stride;
            } else {
                stride1 = velem[i].src_stride;
                offset1 = vb1->buffer_offset + velem[i].src_offset + offset * velem[i].src_stride;
            }

            OUT_CS(R300_VBPNTR_SIZE0(size1) | R300_VBPNTR_STRIDE0(stride1));
            OUT_CS(offset1);
        }

        for (i = 0; i < vertex_array_count; i++) {
            buf = r300_resource(vbuf[velem[i].vertex_buffer_index].buffer.resource);
            OUT_CS_RELOC(buf);
        }
    }
    END_CS;
}

void r300_emit_vertex_arrays_swtcl(struct r300_context *r300, bool indexed)
{
    CS_LOCALS(r300);

    DBG(r300, DBG_SWTCL, "r300: Preparing vertex buffer %p for render, "
            "vertex size %d\n", r300->vbo,
            r300->vertex_info.size);
    /* Set the pointer to our vertex buffer. The emitted values are this:
     * PACKET3 [3D_LOAD_VBPNTR]
     * COUNT   [1]
     * FORMAT  [size | stride << 8]
     * OFFSET  [offset into BO]
     * VBPNTR  [relocated BO]
     */
    BEGIN_CS(7);
    OUT_CS_PKT3(R300_PACKET3_3D_LOAD_VBPNTR, 3);
    OUT_CS(1 | (!indexed ? R300_VC_FORCE_PREFETCH : 0));
    OUT_CS(r300->vertex_info.size |
            (r300->vertex_info.size << 8));
    OUT_CS(r300->draw_vbo_offset);
    OUT_CS(0);

    assert(r300->vbo);
    OUT_CS(0xc0001000); /* PKT3_NOP */
    OUT_CS(r300->rws->cs_lookup_buffer(&r300->cs, r300->vbo) * 4);
    END_CS;
}

void r300_emit_vertex_stream_state(struct r300_context* r300,
                                   unsigned size, void* state)
{
    struct r300_vertex_stream_state *streams =
        (struct r300_vertex_stream_state*)state;
    unsigned i;
    CS_LOCALS(r300);

    if (DBG_ON(r300, DBG_PSC)) {
        fprintf(stderr, "r300: PSC emit:\n");

        for (i = 0; i < streams->count; i++) {
            fprintf(stderr, "    : prog_stream_cntl%d: 0x%08x\n", i,
                   streams->vap_prog_stream_cntl[i]);
        }

        for (i = 0; i < streams->count; i++) {
            fprintf(stderr, "    : prog_stream_cntl_ext%d: 0x%08x\n", i,
                   streams->vap_prog_stream_cntl_ext[i]);
        }
    }

    BEGIN_CS(size);
    OUT_CS_REG_SEQ(R300_VAP_PROG_STREAM_CNTL_0, streams->count);
    OUT_CS_TABLE(streams->vap_prog_stream_cntl, streams->count);
    OUT_CS_REG_SEQ(R300_VAP_PROG_STREAM_CNTL_EXT_0, streams->count);
    OUT_CS_TABLE(streams->vap_prog_stream_cntl_ext, streams->count);
    END_CS;
}

void r300_emit_pvs_flush(struct r300_context* r300, unsigned size, void* state)
{
    CS_LOCALS(r300);

    BEGIN_CS(size);
    OUT_CS_REG(R300_VAP_PVS_STATE_FLUSH_REG, 0x0);
    END_CS;
}

void r300_emit_vap_invariant_state(struct r300_context *r300,
                                   unsigned size, void *state)
{
    CS_LOCALS(r300);
    WRITE_CS_TABLE(state, size);
}

void r300_emit_vs_state(struct r300_context* r300, unsigned size, void* state)
{
    struct r300_vertex_shader_code* vs = ((struct r300_vertex_shader*)state)->shader;
    struct r300_vertex_program_code* code = &vs->code;
    struct r300_screen* r300screen = r300->screen;
    unsigned instruction_count = code->length / 4;

    unsigned vtx_mem_size = r300screen->caps.is_r500 ? 128 : 72;
    unsigned input_count = MAX2(util_bitcount(code->InputsRead), 1);
    unsigned output_count = MAX2(util_bitcount(code->OutputsWritten), 1);
    unsigned temp_count = MAX2(code->num_temporaries, 1);

    unsigned pvs_num_slots = MIN3(vtx_mem_size / input_count,
                                  vtx_mem_size / output_count, 10);
    unsigned pvs_num_controllers = MIN2(vtx_mem_size / temp_count, 5);

    CS_LOCALS(r300);

    BEGIN_CS(size);

    /* R300_VAP_PVS_CODE_CNTL_0
     * R300_VAP_PVS_CONST_CNTL
     * R300_VAP_PVS_CODE_CNTL_1
     * See the r5xx docs for instructions on how to use these. */
    OUT_CS_REG(R300_VAP_PVS_CODE_CNTL_0, R300_PVS_FIRST_INST(0) |
	       R300_PVS_XYZW_VALID_INST(code->last_pos_write) |
	       R300_PVS_LAST_INST(instruction_count - 1));
    OUT_CS_REG(R300_VAP_PVS_CODE_CNTL_1, code->last_input_read);

    OUT_CS_REG(R300_VAP_PVS_VECTOR_INDX_REG, 0);
    OUT_CS_ONE_REG(R300_VAP_PVS_UPLOAD_DATA, code->length);
    OUT_CS_TABLE(code->body.d, code->length);

    OUT_CS_REG(R300_VAP_CNTL, R300_PVS_NUM_SLOTS(pvs_num_slots) |
            R300_PVS_NUM_CNTLRS(pvs_num_controllers) |
            R300_PVS_NUM_FPUS(r300screen->caps.num_vert_fpus) |
            R300_PVS_VF_MAX_VTX_NUM(12) |
            (r300->clip_halfz ? R300_DX_CLIP_SPACE_DEF : 0) |
            (r300screen->caps.is_r500 ? R500_TCL_STATE_OPTIMIZATION : 0));

    /* Emit flow control instructions.  Even if there are no fc instructions,
     * we still need to write the registers to make sure they are cleared. */
    OUT_CS_REG(R300_VAP_PVS_FLOW_CNTL_OPC, code->fc_ops);
    if (r300screen->caps.is_r500) {
        OUT_CS_REG_SEQ(R500_VAP_PVS_FLOW_CNTL_ADDRS_LW_0, R300_VS_MAX_FC_OPS * 2);
        OUT_CS_TABLE(code->fc_op_addrs.r500, R300_VS_MAX_FC_OPS * 2);
    } else {
        OUT_CS_REG_SEQ(R300_VAP_PVS_FLOW_CNTL_ADDRS_0, R300_VS_MAX_FC_OPS);
        OUT_CS_TABLE(code->fc_op_addrs.r300, R300_VS_MAX_FC_OPS);
    }
    OUT_CS_REG_SEQ(R300_VAP_PVS_FLOW_CNTL_LOOP_INDEX_0, R300_VS_MAX_FC_OPS);
    OUT_CS_TABLE(code->fc_loop_index, R300_VS_MAX_FC_OPS);

    END_CS;
}

void r300_emit_vs_constants(struct r300_context* r300,
                            unsigned size, void *state)
{
    unsigned count = r300_vs(r300)->shader->externals_count;
    struct r300_constant_buffer *buf = (struct r300_constant_buffer*)state;
    struct r300_vertex_shader_code *vs = r300_vs(r300)->shader;
    unsigned i;
    int imm_first = vs->externals_count;
    int imm_end = vs->code.constants.Count;
    int imm_count = vs->immediates_count;
    CS_LOCALS(r300);

    BEGIN_CS(size);
    OUT_CS_REG(R300_VAP_PVS_CONST_CNTL,
               R300_PVS_CONST_BASE_OFFSET(buf->buffer_base) |
               R300_PVS_MAX_CONST_ADDR(MAX2(imm_end - 1, 0)));
    if (vs->externals_count) {
        OUT_CS_REG(R300_VAP_PVS_VECTOR_INDX_REG,
                   (r300->screen->caps.is_r500 ?
                   R500_PVS_CONST_START : R300_PVS_CONST_START) + buf->buffer_base);
        OUT_CS_ONE_REG(R300_VAP_PVS_UPLOAD_DATA, count * 4);
        if (buf->remap_table){
            for (i = 0; i < count; i++) {
                uint32_t *data = &buf->ptr[buf->remap_table[i]*4];
                OUT_CS_TABLE(data, 4);
            }
        } else {
            OUT_CS_TABLE(buf->ptr, count * 4);
        }
    }

    /* Emit immediates. */
    if (imm_count) {
        OUT_CS_REG(R300_VAP_PVS_VECTOR_INDX_REG,
                   (r300->screen->caps.is_r500 ?
                   R500_PVS_CONST_START : R300_PVS_CONST_START) +
                   buf->buffer_base + imm_first);
        OUT_CS_ONE_REG(R300_VAP_PVS_UPLOAD_DATA, imm_count * 4);
        for (i = imm_first; i < imm_end; i++) {
            const float *data = vs->code.constants.Constants[i].u.Immediate;
            OUT_CS_TABLE(data, 4);
        }
    }
    END_CS;
}

void r300_emit_viewport_state(struct r300_context* r300,
                              unsigned size, void* state)
{
    struct r300_viewport_state* viewport = (struct r300_viewport_state*)state;
    CS_LOCALS(r300);

    BEGIN_CS(size);
    OUT_CS_REG_SEQ(R300_SE_VPORT_XSCALE, 6);
    OUT_CS_TABLE(&viewport->xscale, 6);
    OUT_CS_REG(R300_VAP_VTE_CNTL, viewport->vte_control);
    END_CS;
}

void r300_emit_hiz_clear(struct r300_context *r300, unsigned size, void *state)
{
    struct pipe_framebuffer_state *fb =
        (struct pipe_framebuffer_state*)r300->fb_state.state;
    struct r300_resource* tex;
    CS_LOCALS(r300);

    tex = r300_resource(fb->zsbuf->texture);

    BEGIN_CS(size);
    OUT_CS_PKT3(R300_PACKET3_3D_CLEAR_HIZ, 2);
    OUT_CS(0);
    OUT_CS(tex->tex.hiz_dwords[fb->zsbuf->u.tex.level]);
    OUT_CS(r300->hiz_clear_value);
    END_CS;

    /* Mark the current zbuffer's hiz ram as in use. */
    r300->hiz_in_use = true;
    r300->hiz_func = HIZ_FUNC_NONE;
    r300_mark_atom_dirty(r300, &r300->hyperz_state);
}

void r300_emit_zmask_clear(struct r300_context *r300, unsigned size, void *state)
{
    struct pipe_framebuffer_state *fb =
        (struct pipe_framebuffer_state*)r300->fb_state.state;
    struct r300_resource *tex;
    CS_LOCALS(r300);

    tex = r300_resource(fb->zsbuf->texture);

    BEGIN_CS(size);
    OUT_CS_PKT3(R300_PACKET3_3D_CLEAR_ZMASK, 2);
    OUT_CS(0);
    OUT_CS(tex->tex.zmask_dwords[fb->zsbuf->u.tex.level]);
    OUT_CS(0);
    END_CS;

    /* Mark the current zbuffer's zmask as in use. */
    r300->zmask_in_use = true;
    r300_mark_atom_dirty(r300, &r300->hyperz_state);
}

void r300_emit_cmask_clear(struct r300_context *r300, unsigned size, void *state)
{
    struct pipe_framebuffer_state *fb =
        (struct pipe_framebuffer_state*)r300->fb_state.state;
    struct r300_resource *tex;
    CS_LOCALS(r300);

    tex = r300_resource(fb->cbufs[0]->texture);

    BEGIN_CS(size);
    OUT_CS_PKT3(R300_PACKET3_3D_CLEAR_CMASK, 2);
    OUT_CS(0);
    OUT_CS(tex->tex.cmask_dwords);
    OUT_CS(0);
    END_CS;

    /* Mark the current zbuffer's zmask as in use. */
    r300->cmask_in_use = true;
    r300_mark_fb_state_dirty(r300, R300_CHANGED_CMASK_ENABLE);
}

void r300_emit_ztop_state(struct r300_context* r300,
                          unsigned size, void* state)
{
    struct r300_ztop_state* ztop = (struct r300_ztop_state*)state;
    CS_LOCALS(r300);

    BEGIN_CS(size);
    OUT_CS_REG(R300_ZB_ZTOP, ztop->z_buffer_top);
    END_CS;
}

void r300_emit_texture_cache_inval(struct r300_context* r300, unsigned size, void* state)
{
    CS_LOCALS(r300);

    BEGIN_CS(size);
    OUT_CS_REG(R300_TX_INVALTAGS, 0);
    END_CS;
}

bool r300_emit_buffer_validate(struct r300_context *r300,
                               bool do_validate_vertex_buffers,
                               struct pipe_resource *index_buffer)
{
    struct pipe_framebuffer_state *fb =
        (struct pipe_framebuffer_state*)r300->fb_state.state;
    struct r300_aa_state *aa = (struct r300_aa_state*)r300->aa_state.state;
    struct r300_textures_state *texstate =
        (struct r300_textures_state*)r300->textures_state.state;
    struct r300_resource *tex;
    unsigned i;
    bool flushed = false;

validate:
    if (r300->fb_state.dirty) {
        /* Color buffers... */
        for (i = 0; i < fb->nr_cbufs; i++) {
            if (!fb->cbufs[i])
                continue;
            tex = r300_resource(fb->cbufs[i]->texture);
            assert(tex && tex->buf && "cbuf is marked, but NULL!");
            r300->rws->cs_add_buffer(&r300->cs, tex->buf,
                                    RADEON_USAGE_READWRITE | RADEON_USAGE_SYNCHRONIZED |
                                    (tex->b.nr_samples > 1 ?
                                        RADEON_PRIO_COLOR_BUFFER_MSAA :
                                        RADEON_PRIO_COLOR_BUFFER),
                                    r300_surface(fb->cbufs[i])->domain);
        }
        /* ...depth buffer... */
        if (fb->zsbuf) {
            tex = r300_resource(fb->zsbuf->texture);
            assert(tex && tex->buf && "zsbuf is marked, but NULL!");
            r300->rws->cs_add_buffer(&r300->cs, tex->buf,
                                    RADEON_USAGE_READWRITE | RADEON_USAGE_SYNCHRONIZED |
                                    (tex->b.nr_samples > 1 ?
                                        RADEON_PRIO_DEPTH_BUFFER_MSAA :
                                        RADEON_PRIO_DEPTH_BUFFER),
                                    r300_surface(fb->zsbuf)->domain);
        }
    }
    /* The AA resolve buffer. */
    if (r300->aa_state.dirty) {
        if (aa->dest) {
            r300->rws->cs_add_buffer(&r300->cs, aa->dest->buf,
                                    RADEON_USAGE_WRITE | RADEON_USAGE_SYNCHRONIZED |
                                    RADEON_PRIO_COLOR_BUFFER,
                                    aa->dest->domain);
        }
    }
    if (r300->textures_state.dirty) {
        /* ...textures... */
        for (i = 0; i < texstate->count; i++) {
            if (!(texstate->tx_enable & (1U << i))) {
                continue;
            }

            tex = r300_resource(texstate->sampler_views[i]->base.texture);
            r300->rws->cs_add_buffer(&r300->cs, tex->buf,
                                     RADEON_USAGE_READ | RADEON_USAGE_SYNCHRONIZED |
                                     RADEON_PRIO_SAMPLER_TEXTURE,
                                    tex->domain);
        }
    }
    /* ...occlusion query buffer... */
    if (r300->query_current)
        r300->rws->cs_add_buffer(&r300->cs, r300->query_current->buf,
                                 RADEON_USAGE_WRITE | RADEON_USAGE_SYNCHRONIZED |
                                 RADEON_PRIO_QUERY,
                                 RADEON_DOMAIN_GTT);
    /* ...vertex buffer for SWTCL path... */
    if (r300->vbo)
        r300->rws->cs_add_buffer(&r300->cs, r300->vbo,
                                 RADEON_USAGE_READ | RADEON_USAGE_SYNCHRONIZED |
                                 RADEON_PRIO_VERTEX_BUFFER,
                                 RADEON_DOMAIN_GTT);
    /* ...vertex buffers for HWTCL path... */
    if (do_validate_vertex_buffers && r300->vertex_arrays_dirty) {
        struct pipe_vertex_buffer *vbuf = r300->vertex_buffer;
        struct pipe_vertex_buffer *last = r300->vertex_buffer +
                                      r300->nr_vertex_buffers;
        struct pipe_resource *buf;

        for (; vbuf != last; vbuf++) {
            buf = vbuf->buffer.resource;
            if (!buf)
                continue;

            r300->rws->cs_add_buffer(&r300->cs, r300_resource(buf)->buf,
                                    RADEON_USAGE_READ | RADEON_USAGE_SYNCHRONIZED |
                                    RADEON_PRIO_SAMPLER_BUFFER,
                                    r300_resource(buf)->domain);
        }
    }
    /* ...and index buffer for HWTCL path. */
    if (index_buffer)
        r300->rws->cs_add_buffer(&r300->cs, r300_resource(index_buffer)->buf,
                                RADEON_USAGE_READ | RADEON_USAGE_SYNCHRONIZED |
                                RADEON_PRIO_INDEX_BUFFER,
                                r300_resource(index_buffer)->domain);

    /* Now do the validation (flush is called inside cs_validate on failure). */
    if (!r300->rws->cs_validate(&r300->cs)) {
        /* Ooops, an infinite loop, give up. */
        if (flushed)
            return false;

        flushed = true;
        goto validate;
    }

    return true;
}

unsigned r300_get_num_dirty_dwords(struct r300_context *r300)
{
    struct r300_atom* atom;
    unsigned dwords = 0;

    foreach_dirty_atom(r300, atom) {
        if (atom->dirty) {
            dwords += atom->size;
        }
    }

    /* let's reserve some more, just in case */
    dwords += 32;

    return dwords;
}

unsigned r300_get_num_cs_end_dwords(struct r300_context *r300)
{
    unsigned dwords = 0;

    /* Emitted in flush. */
    dwords += 26; /* emit_query_end */
    dwords += r300->hyperz_state.size + 2; /* emit_hyperz_end + zcache flush */
    if (r300->screen->caps.is_r500)
        dwords += 2; /* emit_index_bias */
    dwords += 3; /* MSPOS */

    return dwords;
}

/* Emit all dirty state. */
void r300_emit_dirty_state(struct r300_context* r300)
{
    struct r300_atom *atom;

    foreach_dirty_atom(r300, atom) {
        if (atom->dirty) {
            atom->emit(r300, atom->size, atom->state);
            atom->dirty = false;
        }
    }

    r300->first_dirty = NULL;
    r300->last_dirty = NULL;
    r300->dirty_hw++;
}
