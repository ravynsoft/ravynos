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

#include "r300_context.h"
#include "r300_reg.h"
#include "r300_fs.h"

#include "util/format/u_format.h"

/*
  HiZ rules - taken from various docs 
   1. HiZ only works on depth values
   2. Cannot HiZ if stencil fail or zfail is !KEEP
   3. on R300/400, HiZ is disabled if depth test is EQUAL
   4. comparison changes without clears usually mean disabling HiZ
*/
/*****************************************************************************/
/* The HyperZ setup                                                          */
/*****************************************************************************/

static enum r300_hiz_func r300_get_hiz_func(struct r300_context *r300)
{
    struct r300_dsa_state *dsa = r300->dsa_state.state;

    switch (dsa->dsa.depth_func) {
    case PIPE_FUNC_NEVER:
    case PIPE_FUNC_EQUAL:
    case PIPE_FUNC_NOTEQUAL:
    case PIPE_FUNC_ALWAYS:
    default:
        /* Guess MAX for uncertain cases. */
    case PIPE_FUNC_LESS:
    case PIPE_FUNC_LEQUAL:
        return HIZ_FUNC_MAX;

    case PIPE_FUNC_GREATER:
    case PIPE_FUNC_GEQUAL:
        return HIZ_FUNC_MIN;
    }
}

/* Return what's used for the depth test (either minimum or maximum). */
static unsigned r300_get_sc_hz_max(struct r300_context *r300)
{
    struct r300_dsa_state *dsa = r300->dsa_state.state;
    unsigned func = dsa->dsa.depth_func;

    return func >= PIPE_FUNC_GREATER ? R300_SC_HYPERZ_MAX : R300_SC_HYPERZ_MIN;
}

static bool r300_is_hiz_func_valid(struct r300_context *r300)
{
    struct r300_dsa_state *dsa = r300->dsa_state.state;
    unsigned func = dsa->dsa.depth_func;

    if (r300->hiz_func == HIZ_FUNC_NONE)
        return true;

    /* func1 is less/lessthan */
    if (r300->hiz_func == HIZ_FUNC_MAX &&
        (func == PIPE_FUNC_GEQUAL || func == PIPE_FUNC_GREATER))
        return false;

    /* func1 is greater/greaterthan */
    if (r300->hiz_func == HIZ_FUNC_MIN &&
        (func == PIPE_FUNC_LESS   || func == PIPE_FUNC_LEQUAL))
        return false;

    return true;
}

static bool r300_dsa_stencil_op_not_keep(struct pipe_stencil_state *s)
{
    return s->enabled && (s->fail_op != PIPE_STENCIL_OP_KEEP ||
                          s->zfail_op != PIPE_STENCIL_OP_KEEP);
}

static bool r300_hiz_allowed(struct r300_context *r300)
{
    struct r300_dsa_state *dsa = r300->dsa_state.state;
    struct r300_screen *r300screen = r300->screen;

    if (r300_fragment_shader_writes_depth(r300_fs(r300)))
        return false;

    if (r300->query_current)
        return false;

    /* If the depth function is inverted, HiZ must be disabled. */
    if (!r300_is_hiz_func_valid(r300))
        return false;

    /* if stencil fail/zfail op is not KEEP */
    if (r300_dsa_stencil_op_not_keep(&dsa->dsa.stencil[0]) ||
        r300_dsa_stencil_op_not_keep(&dsa->dsa.stencil[1]))
        return false;

    if (dsa->dsa.depth_enabled) {
        /* if depth func is EQUAL pre-r500 */
        if (dsa->dsa.depth_func == PIPE_FUNC_EQUAL && !r300screen->caps.is_r500)
            return false;

        /* if depth func is NOTEQUAL */
        if (dsa->dsa.depth_func == PIPE_FUNC_NOTEQUAL)
            return false;
    }
    return true;
}

static void r300_update_hyperz(struct r300_context* r300)
{
    struct r300_hyperz_state *z =
        (struct r300_hyperz_state*)r300->hyperz_state.state;
    struct pipe_framebuffer_state *fb =
        (struct pipe_framebuffer_state*)r300->fb_state.state;
    struct r300_dsa_state *dsa = r300->dsa_state.state;
    struct r300_resource *zstex =
            fb->zsbuf ? r300_resource(fb->zsbuf->texture) : NULL;

    z->gb_z_peq_config = 0;
    z->zb_bw_cntl = 0;
    z->sc_hyperz = R300_SC_HYPERZ_ADJ_2;
    z->flush = 0;

    if (r300->cbzb_clear) {
        z->zb_bw_cntl |= R300_ZB_CB_CLEAR_CACHE_LINE_WRITE_ONLY;
        return;
    }

    if (!zstex || !r300->hyperz_enabled)
        return;

    /* Set the size of ZMASK tiles. */
    if (zstex->tex.zcomp8x8[fb->zsbuf->u.tex.level]) {
        z->gb_z_peq_config |= R300_GB_Z_PEQ_CONFIG_Z_PEQ_SIZE_8_8;
    }

    /* R500-specific features and optimizations. */
    if (r300->screen->caps.is_r500) {
        z->zb_bw_cntl |= R500_PEQ_PACKING_ENABLE |
                         R500_COVERED_PTR_MASKING_ENABLE;
    }

    /* Setup decompression if needed. No other HyperZ setting is required. */
    if (r300->zmask_decompress) {
        z->zb_bw_cntl |= R300_FAST_FILL_ENABLE |
                         R300_RD_COMP_ENABLE;
        return;
    }

    /* Do not set anything if depth and stencil tests are off. */
    if (!dsa->dsa.depth_enabled &&
        !dsa->dsa.stencil[0].enabled &&
        !dsa->dsa.stencil[1].enabled) {
        assert(!dsa->dsa.depth_writemask);
        return;
    }

    /* Zbuffer compression. */
    if (r300->zmask_in_use && !r300->locked_zbuffer) {
        z->zb_bw_cntl |= R300_FAST_FILL_ENABLE |
                         R300_RD_COMP_ENABLE |
                         R300_WR_COMP_ENABLE;
    }

    /* HiZ. */
    if (r300->hiz_in_use && !r300->locked_zbuffer) {
        /* HiZ cannot be used under some circumstances. */
        if (!r300_hiz_allowed(r300)) {
            /* If writemask is disabled, the HiZ memory will not be changed,
             * so we can keep its content for later. */
            if (dsa->dsa.depth_writemask) {
                r300->hiz_in_use = false;
            }
            return;
        }
        DBG(r300, DBG_HYPERZ, "r300: Z-func: %i\n", dsa->dsa.depth_func);

        /* Set the HiZ function if needed. */
        if (r300->hiz_func == HIZ_FUNC_NONE) {
            r300->hiz_func = r300_get_hiz_func(r300);
        }

        /* Setup the HiZ bits. */
        z->zb_bw_cntl |= R300_HIZ_ENABLE |
                (r300->hiz_func == HIZ_FUNC_MIN ? R300_HIZ_MIN : R300_HIZ_MAX);

        z->sc_hyperz |= R300_SC_HYPERZ_ENABLE |
                        r300_get_sc_hz_max(r300);

        if (r300->screen->caps.is_r500) {
            z->zb_bw_cntl |= R500_HIZ_EQUAL_REJECT_ENABLE;
        }
    }
}

/*****************************************************************************/
/* The ZTOP state                                                            */
/*****************************************************************************/

static bool r300_dsa_alpha_test_enabled(
        struct pipe_depth_stencil_alpha_state *dsa)
{
    /* We are interested only in the cases when alpha testing can kill
     * a fragment. */

    return dsa->alpha_enabled && dsa->alpha_func != PIPE_FUNC_ALWAYS;
}

static void r300_update_ztop(struct r300_context* r300)
{
    struct r300_ztop_state* ztop_state =
        (struct r300_ztop_state*)r300->ztop_state.state;
    uint32_t old_ztop = ztop_state->z_buffer_top;

    /* This is important enough that I felt it warranted a comment.
     *
     * According to the docs, these are the conditions where ZTOP must be
     * disabled:
     * 1) Alpha testing enabled
     * 2) Texture kill instructions in fragment shader
     * 3) Chroma key culling enabled
     * 4) W-buffering enabled
     *
     * The docs claim that for the first three cases, if no ZS writes happen,
     * then ZTOP can be used.
     *
     * (3) will never apply since we do not support chroma-keyed operations.
     * (4) will need to be re-examined (and this comment updated) if/when
     * Hyper-Z becomes supported.
     *
     * Additionally, the following conditions require disabled ZTOP:
     * 5) Depth writes in fragment shader
     * 6) Outstanding occlusion queries
     *
     * This register causes stalls all the way from SC to CB when changed,
     * but it is buffered on-chip so it does not hurt to write it if it has
     * not changed.
     *
     * ~C.
     */

    /* ZS writes */
    if (util_writes_depth_stencil(r300->dsa_state.state) &&
           (r300_dsa_alpha_test_enabled(r300->dsa_state.state) ||  /* (1) */
            r300_fs(r300)->shader->info.uses_kill)) {              /* (2) */
        ztop_state->z_buffer_top = R300_ZTOP_DISABLE;
    } else if (r300_fragment_shader_writes_depth(r300_fs(r300))) { /* (5) */
        ztop_state->z_buffer_top = R300_ZTOP_DISABLE;
    } else if (r300->query_current) {                              /* (6) */
        ztop_state->z_buffer_top = R300_ZTOP_DISABLE;
    } else {
        ztop_state->z_buffer_top = R300_ZTOP_ENABLE;
    }
    if (ztop_state->z_buffer_top != old_ztop)
        r300_mark_atom_dirty(r300, &r300->ztop_state);
}

void r300_update_hyperz_state(struct r300_context* r300)
{
    r300_update_ztop(r300);

    if (r300->hyperz_state.dirty) {
        r300_update_hyperz(r300);
    }
}
