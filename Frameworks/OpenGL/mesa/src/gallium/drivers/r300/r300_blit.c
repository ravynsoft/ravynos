/*
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
#include "r300_emit.h"
#include "r300_texture.h"
#include "r300_reg.h"

#include "util/format/u_format.h"
#include "util/half_float.h"
#include "util/u_pack_color.h"
#include "util/u_surface.h"

enum r300_blitter_op /* bitmask */
{
    R300_STOP_QUERY         = 1,
    R300_SAVE_TEXTURES      = 2,
    R300_SAVE_FRAMEBUFFER   = 4,
    R300_IGNORE_RENDER_COND = 8,

    R300_CLEAR         = R300_STOP_QUERY,

    R300_CLEAR_SURFACE = R300_STOP_QUERY | R300_SAVE_FRAMEBUFFER,

    R300_COPY          = R300_STOP_QUERY | R300_SAVE_FRAMEBUFFER |
                         R300_SAVE_TEXTURES | R300_IGNORE_RENDER_COND,

    R300_BLIT          = R300_STOP_QUERY | R300_SAVE_FRAMEBUFFER |
                         R300_SAVE_TEXTURES,

    R300_DECOMPRESS    = R300_STOP_QUERY | R300_IGNORE_RENDER_COND,
};

static void r300_blitter_begin(struct r300_context* r300, enum r300_blitter_op op)
{
    if ((op & R300_STOP_QUERY) && r300->query_current) {
        r300->blitter_saved_query = r300->query_current;
        r300_stop_query(r300);
    }

    /* Yeah we have to save all those states to ensure the blitter operation
     * is really transparent. The states will be restored by the blitter once
     * copying is done. */
    util_blitter_save_blend(r300->blitter, r300->blend_state.state);
    util_blitter_save_depth_stencil_alpha(r300->blitter, r300->dsa_state.state);
    util_blitter_save_stencil_ref(r300->blitter, &(r300->stencil_ref));
    util_blitter_save_rasterizer(r300->blitter, r300->rs_state.state);
    util_blitter_save_fragment_shader(r300->blitter, r300->fs.state);
    util_blitter_save_vertex_shader(r300->blitter, r300->vs_state.state);
    util_blitter_save_viewport(r300->blitter, &r300->viewport);
    util_blitter_save_scissor(r300->blitter, r300->scissor_state.state);
    util_blitter_save_sample_mask(r300->blitter, *(unsigned*)r300->sample_mask.state, 0);
    util_blitter_save_vertex_buffer_slot(r300->blitter, r300->vertex_buffer);
    util_blitter_save_vertex_elements(r300->blitter, r300->velems);

    struct pipe_constant_buffer cb = {
       /* r300 doesn't use the size for FS at all. The shader determines it.
        * Set something for blitter.
        */
       .buffer_size = 4,
       .user_buffer = ((struct r300_constant_buffer*)r300->fs_constants.state)->ptr,
    };
    util_blitter_save_fragment_constant_buffer_slot(r300->blitter, &cb);

    if (op & R300_SAVE_FRAMEBUFFER) {
        util_blitter_save_framebuffer(r300->blitter, r300->fb_state.state);
    }

    if (op & R300_SAVE_TEXTURES) {
        struct r300_textures_state* state =
            (struct r300_textures_state*)r300->textures_state.state;

        util_blitter_save_fragment_sampler_states(
            r300->blitter, state->sampler_state_count,
            (void**)state->sampler_states);

        util_blitter_save_fragment_sampler_views(
            r300->blitter, state->sampler_view_count,
            (struct pipe_sampler_view**)state->sampler_views);
    }

    if (op & R300_IGNORE_RENDER_COND) {
        /* Save the flag. */
        r300->blitter_saved_skip_rendering = r300->skip_rendering+1;
        r300->skip_rendering = false;
    } else {
        r300->blitter_saved_skip_rendering = 0;
    }
}

static void r300_blitter_end(struct r300_context *r300)
{
    if (r300->blitter_saved_query) {
        r300_resume_query(r300, r300->blitter_saved_query);
        r300->blitter_saved_query = NULL;
    }

    if (r300->blitter_saved_skip_rendering) {
        /* Restore the flag. */
        r300->skip_rendering = r300->blitter_saved_skip_rendering-1;
    }
}

static uint32_t r300_depth_clear_cb_value(enum pipe_format format,
                                          const float* rgba)
{
    union util_color uc;
    util_pack_color(rgba, format, &uc);

    if (util_format_get_blocksizebits(format) == 32)
        return uc.ui[0];
    else
        return uc.us | (uc.us << 16);
}

static bool r300_cbzb_clear_allowed(struct r300_context *r300,
                                    unsigned clear_buffers)
{
    struct pipe_framebuffer_state *fb =
        (struct pipe_framebuffer_state*)r300->fb_state.state;

    /* Only color clear allowed, and only one colorbuffer. */
    if ((clear_buffers & ~PIPE_CLEAR_COLOR) != 0 || fb->nr_cbufs != 1 || !fb->cbufs[0])
        return false;

    return r300_surface(fb->cbufs[0])->cbzb_allowed;
}

static bool r300_fast_zclear_allowed(struct r300_context *r300,
                                     unsigned clear_buffers)
{
    struct pipe_framebuffer_state *fb =
        (struct pipe_framebuffer_state*)r300->fb_state.state;

    return r300_resource(fb->zsbuf->texture)->tex.zmask_dwords[fb->zsbuf->u.tex.level] != 0;
}

static bool r300_hiz_clear_allowed(struct r300_context *r300)
{
    struct pipe_framebuffer_state *fb =
        (struct pipe_framebuffer_state*)r300->fb_state.state;

    return r300_resource(fb->zsbuf->texture)->tex.hiz_dwords[fb->zsbuf->u.tex.level] != 0;
}

static uint32_t r300_depth_clear_value(enum pipe_format format,
                                       double depth, unsigned stencil)
{
    switch (format) {
        case PIPE_FORMAT_Z16_UNORM:
        case PIPE_FORMAT_X8Z24_UNORM:
            return util_pack_z(format, depth);

        case PIPE_FORMAT_S8_UINT_Z24_UNORM:
            return util_pack_z_stencil(format, depth, stencil);

        default:
            assert(0);
            return 0;
    }
}

static uint32_t r300_hiz_clear_value(double depth)
{
    uint32_t r = (uint32_t)(CLAMP(depth, 0, 1) * 255.5);
    assert(r <= 255);
    return r | (r << 8) | (r << 16) | (r << 24);
}

static void r300_set_clear_color(struct r300_context *r300,
                                 const union pipe_color_union *color)
{
    struct pipe_framebuffer_state *fb =
        (struct pipe_framebuffer_state*)r300->fb_state.state;
    union util_color uc;

    memset(&uc, 0, sizeof(uc));
    util_pack_color(color->f, fb->cbufs[0]->format, &uc);

    if (fb->cbufs[0]->format == PIPE_FORMAT_R16G16B16A16_FLOAT ||
        fb->cbufs[0]->format == PIPE_FORMAT_R16G16B16X16_FLOAT) {
        /* (0,1,2,3) maps to (B,G,R,A) */
        r300->color_clear_value_gb = uc.h[0] | ((uint32_t)uc.h[1] << 16);
        r300->color_clear_value_ar = uc.h[2] | ((uint32_t)uc.h[3] << 16);
    } else {
        r300->color_clear_value = uc.ui[0];
    }
}

DEBUG_GET_ONCE_BOOL_OPTION(hyperz, "RADEON_HYPERZ", false)

/* Clear currently bound buffers. */
static void r300_clear(struct pipe_context* pipe,
                       unsigned buffers,
                       const struct pipe_scissor_state *scissor_state,
                       const union pipe_color_union *color,
                       double depth,
                       unsigned stencil)
{
    /* My notes about Zbuffer compression:
     *
     * 1) The zbuffer must be micro-tiled and whole microtiles must be
     *    written if compression is enabled. If microtiling is disabled,
     *    it locks up.
     *
     * 2) There is ZMASK RAM which contains a compressed zbuffer.
     *    Each dword of the Z Mask contains compression information
     *    for 16 4x4 pixel tiles, that is 2 bits for each tile.
     *    On chips with 2 Z pipes, every other dword maps to a different
     *    pipe. On newer chipsets, there is a new compression mode
     *    with 8x8 pixel tiles per 2 bits.
     *
     * 3) The FASTFILL bit has nothing to do with filling. It only tells hw
     *    it should look in the ZMASK RAM first before fetching from a real
     *    zbuffer.
     *
     * 4) If a pixel is in a cleared state, ZB_DEPTHCLEARVALUE is returned
     *    during zbuffer reads instead of the value that is actually stored
     *    in the zbuffer memory. A pixel is in a cleared state when its ZMASK
     *    is equal to 0. Therefore, if you clear ZMASK with zeros, you may
     *    leave the zbuffer memory uninitialized, but then you must enable
     *    compression, so that the ZMASK RAM is actually used.
     *
     * 5) Each 4x4 (or 8x8) tile is automatically decompressed and recompressed
     *    during zbuffer updates. A special decompressing operation should be
     *    used to fully decompress a zbuffer, which basically just stores all
     *    compressed tiles in ZMASK to the zbuffer memory.
     *
     * 6) For a 16-bit zbuffer, compression causes a hung with one or
     *    two samples and should not be used.
     *
     * 7) FORCE_COMPRESSED_STENCIL_VALUE should be enabled for stencil clears
     *    to avoid needless decompression.
     *
     * 8) Fastfill must not be used if reading of compressed Z data is disabled
     *    and writing of compressed Z data is enabled (RD/WR_COMP_ENABLE),
     *    i.e. it cannot be used to compress the zbuffer.
     *
     * 9) ZB_CB_CLEAR does not interact with zbuffer compression in any way.
     *
     * - Marek
     */

    struct r300_context* r300 = r300_context(pipe);
    struct pipe_framebuffer_state *fb =
        (struct pipe_framebuffer_state*)r300->fb_state.state;
    struct r300_hyperz_state *hyperz =
        (struct r300_hyperz_state*)r300->hyperz_state.state;
    uint32_t width = fb->width;
    uint32_t height = fb->height;
    uint32_t hyperz_dcv = hyperz->zb_depthclearvalue;

    /* Use fast Z clear.
     * The zbuffer must be in micro-tiled mode, otherwise it locks up. */
    if (buffers & PIPE_CLEAR_DEPTHSTENCIL) {
        bool zmask_clear, hiz_clear;

        /* If both depth and stencil are present, they must be cleared together. */
        if (fb->zsbuf->texture->format == PIPE_FORMAT_S8_UINT_Z24_UNORM &&
            (buffers & PIPE_CLEAR_DEPTHSTENCIL) != PIPE_CLEAR_DEPTHSTENCIL) {
            zmask_clear = false;
            hiz_clear = false;
        } else {
            zmask_clear = r300_fast_zclear_allowed(r300, buffers);
            hiz_clear = r300_hiz_clear_allowed(r300);
        }

        /* If we need Hyper-Z. */
        if (zmask_clear || hiz_clear) {
            /* Try to obtain the access to Hyper-Z buffers if we don't have one. */
            if (!r300->hyperz_enabled &&
                (r300->screen->caps.is_r500 || debug_get_option_hyperz())) {
                r300->hyperz_enabled =
                    r300->rws->cs_request_feature(&r300->cs,
                                                RADEON_FID_R300_HYPERZ_ACCESS,
                                                true);
                if (r300->hyperz_enabled) {
                   /* Need to emit HyperZ buffer regs for the first time. */
                   r300_mark_fb_state_dirty(r300, R300_CHANGED_HYPERZ_FLAG);
                }
            }

            /* Setup Hyper-Z clears. */
            if (r300->hyperz_enabled) {
                if (zmask_clear) {
                    hyperz_dcv = hyperz->zb_depthclearvalue =
                        r300_depth_clear_value(fb->zsbuf->format, depth, stencil);

                    r300_mark_atom_dirty(r300, &r300->zmask_clear);
                    r300_mark_atom_dirty(r300, &r300->gpu_flush);
                    buffers &= ~PIPE_CLEAR_DEPTHSTENCIL;
                }

                if (hiz_clear) {
                    r300->hiz_clear_value = r300_hiz_clear_value(depth);
                    r300_mark_atom_dirty(r300, &r300->hiz_clear);
                    r300_mark_atom_dirty(r300, &r300->gpu_flush);
                }
                r300->num_z_clears++;
            }
        }
    }

    /* Use fast color clear for an AA colorbuffer.
     * The CMASK is shared between all colorbuffers, so we use it
     * if there is only one colorbuffer bound. */
    if ((buffers & PIPE_CLEAR_COLOR) && fb->nr_cbufs == 1 && fb->cbufs[0] &&
        r300_resource(fb->cbufs[0]->texture)->tex.cmask_dwords) {
        /* Try to obtain the access to the CMASK if we don't have one. */
        if (!r300->cmask_access) {
            r300->cmask_access =
                r300->rws->cs_request_feature(&r300->cs,
                                              RADEON_FID_R300_CMASK_ACCESS,
                                              true);
        }

        /* Setup the clear. */
        if (r300->cmask_access) {
            /* Pair the resource with the CMASK to avoid other resources
             * accessing it. */
            if (!r300->screen->cmask_resource) {
                mtx_lock(&r300->screen->cmask_mutex);
                /* Double checking (first unlocked, then locked). */
                if (!r300->screen->cmask_resource) {
                    /* Don't reference this, so that the texture can be
                     * destroyed while set in cmask_resource.
                     * Then in texture_destroy, we set cmask_resource to NULL. */
                    r300->screen->cmask_resource = fb->cbufs[0]->texture;
                }
                mtx_unlock(&r300->screen->cmask_mutex);
            }

            if (r300->screen->cmask_resource == fb->cbufs[0]->texture) {
                r300_set_clear_color(r300, color);
                r300_mark_atom_dirty(r300, &r300->cmask_clear);
                r300_mark_atom_dirty(r300, &r300->gpu_flush);
                buffers &= ~PIPE_CLEAR_COLOR;
            }
        }
    }
    /* Enable CBZB clear. */
    else if (r300_cbzb_clear_allowed(r300, buffers)) {
        struct r300_surface *surf = r300_surface(fb->cbufs[0]);

        hyperz->zb_depthclearvalue =
                r300_depth_clear_cb_value(surf->base.format, color->f);

        width = surf->cbzb_width;
        height = surf->cbzb_height;

        r300->cbzb_clear = true;
        r300_mark_fb_state_dirty(r300, R300_CHANGED_HYPERZ_FLAG);
    }

    /* Clear. */
    if (buffers) {
        /* Clear using the blitter. */
        r300_blitter_begin(r300, R300_CLEAR);
        util_blitter_clear(r300->blitter, width, height, 1,
                           buffers, color, depth, stencil,
                           util_framebuffer_get_num_samples(fb) > 1);
        r300_blitter_end(r300);
    } else if (r300->zmask_clear.dirty ||
               r300->hiz_clear.dirty ||
               r300->cmask_clear.dirty) {
        /* Just clear zmask and hiz now, this does not use the standard draw
         * procedure. */
        /* Calculate zmask_clear and hiz_clear atom sizes. */
        unsigned dwords =
            r300->gpu_flush.size +
            (r300->zmask_clear.dirty ? r300->zmask_clear.size : 0) +
            (r300->hiz_clear.dirty ? r300->hiz_clear.size : 0) +
            (r300->cmask_clear.dirty ? r300->cmask_clear.size : 0) +
            r300_get_num_cs_end_dwords(r300);

        /* Reserve CS space. */
        if (!r300->rws->cs_check_space(&r300->cs, dwords)) {
            r300_flush(&r300->context, PIPE_FLUSH_ASYNC, NULL);
        }

        /* Emit clear packets. */
        r300_emit_gpu_flush(r300, r300->gpu_flush.size, r300->gpu_flush.state);
        r300->gpu_flush.dirty = false;

        if (r300->zmask_clear.dirty) {
            r300_emit_zmask_clear(r300, r300->zmask_clear.size,
                                  r300->zmask_clear.state);
            r300->zmask_clear.dirty = false;
        }
        if (r300->hiz_clear.dirty) {
            r300_emit_hiz_clear(r300, r300->hiz_clear.size,
                                r300->hiz_clear.state);
            r300->hiz_clear.dirty = false;
        }
        if (r300->cmask_clear.dirty) {
            r300_emit_cmask_clear(r300, r300->cmask_clear.size,
                                  r300->cmask_clear.state);
            r300->cmask_clear.dirty = false;
        }
    } else {
        assert(0);
    }

    /* Disable CBZB clear. */
    if (r300->cbzb_clear) {
        r300->cbzb_clear = false;
        hyperz->zb_depthclearvalue = hyperz_dcv;
        r300_mark_fb_state_dirty(r300, R300_CHANGED_HYPERZ_FLAG);
    }

    /* Enable fastfill and/or hiz.
     *
     * If we cleared zmask/hiz, it's in use now. The Hyper-Z state update
     * looks if zmask/hiz is in use and programs hardware accordingly. */
    if (r300->zmask_in_use || r300->hiz_in_use) {
        r300_mark_atom_dirty(r300, &r300->hyperz_state);
    }
}

/* Clear a region of a color surface to a constant value. */
static void r300_clear_render_target(struct pipe_context *pipe,
                                     struct pipe_surface *dst,
                                     const union pipe_color_union *color,
                                     unsigned dstx, unsigned dsty,
                                     unsigned width, unsigned height,
                                     bool render_condition_enabled)
{
    struct r300_context *r300 = r300_context(pipe);

    r300_blitter_begin(r300, R300_CLEAR_SURFACE |
                       (render_condition_enabled ? 0 : R300_IGNORE_RENDER_COND));
    util_blitter_clear_render_target(r300->blitter, dst, color,
                                     dstx, dsty, width, height);
    r300_blitter_end(r300);
}

/* Clear a region of a depth stencil surface. */
static void r300_clear_depth_stencil(struct pipe_context *pipe,
                                     struct pipe_surface *dst,
                                     unsigned clear_flags,
                                     double depth,
                                     unsigned stencil,
                                     unsigned dstx, unsigned dsty,
                                     unsigned width, unsigned height,
                                     bool render_condition_enabled)
{
    struct r300_context *r300 = r300_context(pipe);
    struct pipe_framebuffer_state *fb =
        (struct pipe_framebuffer_state*)r300->fb_state.state;

    if (r300->zmask_in_use && !r300->locked_zbuffer) {
        if (fb->zsbuf->texture == dst->texture) {
            r300_decompress_zmask(r300);
        }
    }

    /* XXX Do not decompress ZMask of the currently-set zbuffer. */
    r300_blitter_begin(r300, R300_CLEAR_SURFACE |
                       (render_condition_enabled ? 0 : R300_IGNORE_RENDER_COND));
    util_blitter_clear_depth_stencil(r300->blitter, dst, clear_flags, depth, stencil,
                                     dstx, dsty, width, height);
    r300_blitter_end(r300);
}

void r300_decompress_zmask(struct r300_context *r300)
{
    struct pipe_framebuffer_state *fb =
        (struct pipe_framebuffer_state*)r300->fb_state.state;

    if (!r300->zmask_in_use || r300->locked_zbuffer)
        return;

    r300->zmask_decompress = true;
    r300_mark_atom_dirty(r300, &r300->hyperz_state);

    r300_blitter_begin(r300, R300_DECOMPRESS);
    util_blitter_custom_clear_depth(r300->blitter, fb->width, fb->height, 0,
                                    r300->dsa_decompress_zmask);
    r300_blitter_end(r300);

    r300->zmask_decompress = false;
    r300->zmask_in_use = false;
    r300_mark_atom_dirty(r300, &r300->hyperz_state);
}

void r300_decompress_zmask_locked_unsafe(struct r300_context *r300)
{
    struct pipe_framebuffer_state fb;

    memset(&fb, 0, sizeof(fb));
    fb.width = r300->locked_zbuffer->width;
    fb.height = r300->locked_zbuffer->height;
    fb.zsbuf = r300->locked_zbuffer;

    r300->context.set_framebuffer_state(&r300->context, &fb);
    r300_decompress_zmask(r300);
}

void r300_decompress_zmask_locked(struct r300_context *r300)
{
    struct pipe_framebuffer_state saved_fb;

    memset(&saved_fb, 0, sizeof(saved_fb));
    util_copy_framebuffer_state(&saved_fb, r300->fb_state.state);
    r300_decompress_zmask_locked_unsafe(r300);
    r300->context.set_framebuffer_state(&r300->context, &saved_fb);
    util_unreference_framebuffer_state(&saved_fb);

    pipe_surface_reference(&r300->locked_zbuffer, NULL);
}

bool r300_is_blit_supported(enum pipe_format format)
{
    const struct util_format_description *desc =
        util_format_description(format);

    return desc->layout == UTIL_FORMAT_LAYOUT_PLAIN ||
           desc->layout == UTIL_FORMAT_LAYOUT_S3TC ||
           desc->layout == UTIL_FORMAT_LAYOUT_RGTC;
}

/* Copy a block of pixels from one surface to another. */
static void r300_resource_copy_region(struct pipe_context *pipe,
                                      struct pipe_resource *dst,
                                      unsigned dst_level,
                                      unsigned dstx, unsigned dsty, unsigned dstz,
                                      struct pipe_resource *src,
                                      unsigned src_level,
                                      const struct pipe_box *src_box)
{
    struct pipe_screen *screen = pipe->screen;
    struct r300_context *r300 = r300_context(pipe);
    struct pipe_framebuffer_state *fb =
        (struct pipe_framebuffer_state*)r300->fb_state.state;
    unsigned src_width0 = r300_resource(src)->tex.width0;
    unsigned src_height0 = r300_resource(src)->tex.height0;
    unsigned dst_width0 = r300_resource(dst)->tex.width0;
    unsigned dst_height0 = r300_resource(dst)->tex.height0;
    unsigned layout;
    struct pipe_box box, dstbox;
    struct pipe_sampler_view src_templ, *src_view;
    struct pipe_surface dst_templ, *dst_view;

    /* Fallback for buffers. */
    if ((dst->target == PIPE_BUFFER && src->target == PIPE_BUFFER) ||
        !r300_is_blit_supported(dst->format)) {
        util_resource_copy_region(pipe, dst, dst_level, dstx, dsty, dstz,
                                  src, src_level, src_box);
        return;
    }

    /* Can't read MSAA textures. */
    if (src->nr_samples > 1 || dst->nr_samples > 1) {
        return;
    }

    /* The code below changes the texture format so that the copy can be done
     * on hardware. E.g. depth-stencil surfaces are copied as RGBA
     * colorbuffers. */

    util_blitter_default_dst_texture(&dst_templ, dst, dst_level, dstz);
    util_blitter_default_src_texture(r300->blitter, &src_templ, src, src_level);

    layout = util_format_description(dst_templ.format)->layout;

    /* Handle non-renderable plain formats. */
    if (layout == UTIL_FORMAT_LAYOUT_PLAIN &&
        (!screen->is_format_supported(screen, src_templ.format, src->target,
                                      src->nr_samples, src->nr_storage_samples,
                                      PIPE_BIND_SAMPLER_VIEW) ||
         !screen->is_format_supported(screen, dst_templ.format, dst->target,
                                      dst->nr_samples, dst->nr_storage_samples,
                                      PIPE_BIND_RENDER_TARGET))) {
        switch (util_format_get_blocksize(dst_templ.format)) {
            case 1:
                dst_templ.format = PIPE_FORMAT_I8_UNORM;
                break;
            case 2:
                dst_templ.format = PIPE_FORMAT_B4G4R4A4_UNORM;
                break;
            case 4:
                dst_templ.format = PIPE_FORMAT_B8G8R8A8_UNORM;
                break;
            case 8:
                dst_templ.format = PIPE_FORMAT_R16G16B16A16_UNORM;
                break;
            default:
                debug_printf("r300: copy_region: Unhandled format: %s. Falling back to software.\n"
                             "r300: copy_region: Software fallback doesn't work for tiled textures.\n",
                             util_format_short_name(dst_templ.format));
        }
        src_templ.format = dst_templ.format;
    }

    /* Handle compressed formats. */
    if (layout == UTIL_FORMAT_LAYOUT_S3TC ||
        layout == UTIL_FORMAT_LAYOUT_RGTC) {
        assert(src_templ.format == dst_templ.format);

        box = *src_box;
        src_box = &box;

        dst_width0 = align(dst_width0, 4);
        dst_height0 = align(dst_height0, 4);
        src_width0 = align(src_width0, 4);
        src_height0 = align(src_height0, 4);
        box.width = align(box.width, 4);
        box.height = align(box.height, 4);

        switch (util_format_get_blocksize(dst_templ.format)) {
        case 8:
            /* one 4x4 pixel block has 8 bytes.
             * we set 1 pixel = 4 bytes ===> 1 block corresponds to 2 pixels. */
            dst_templ.format = PIPE_FORMAT_R8G8B8A8_UNORM;
            dst_width0 = dst_width0 / 2;
            src_width0 = src_width0 / 2;
            dstx /= 2;
            box.x /= 2;
            box.width /= 2;
            break;
        case 16:
            /* one 4x4 pixel block has 16 bytes.
             * we set 1 pixel = 4 bytes ===> 1 block corresponds to 4 pixels. */
            dst_templ.format = PIPE_FORMAT_R8G8B8A8_UNORM;
            break;
        }
        src_templ.format = dst_templ.format;

        dst_height0 = dst_height0 / 4;
        src_height0 = src_height0 / 4;
        dsty /= 4;
        box.y /= 4;
        box.height /= 4;
    }

    /* Fallback for textures. */
    if (!screen->is_format_supported(screen, dst_templ.format,
                                     dst->target, dst->nr_samples,
                                     dst->nr_storage_samples,
                                     PIPE_BIND_RENDER_TARGET) ||
	!screen->is_format_supported(screen, src_templ.format,
                                     src->target, src->nr_samples,
                                     src->nr_storage_samples,
                                     PIPE_BIND_SAMPLER_VIEW)) {
        assert(0 && "this shouldn't happen, update r300_is_blit_supported");
        util_resource_copy_region(pipe, dst, dst_level, dstx, dsty, dstz,
                                  src, src_level, src_box);
        return;
    }

    /* Decompress ZMASK. */
    if (r300->zmask_in_use && !r300->locked_zbuffer) {
        if (fb->zsbuf->texture == src ||
            fb->zsbuf->texture == dst) {
            r300_decompress_zmask(r300);
        }
    }

    dst_view = r300_create_surface_custom(pipe, dst, &dst_templ, dst_width0, dst_height0);
    src_view = r300_create_sampler_view_custom(pipe, src, &src_templ, src_width0, src_height0);

    u_box_3d(dstx, dsty, dstz, abs(src_box->width), abs(src_box->height),
             abs(src_box->depth), &dstbox);

    r300_blitter_begin(r300, R300_COPY);
    util_blitter_blit_generic(r300->blitter, dst_view, &dstbox,
                              src_view, src_box, src_width0, src_height0,
                              PIPE_MASK_RGBAZS, PIPE_TEX_FILTER_NEAREST, NULL,
                              false, false, 0);
    r300_blitter_end(r300);

    pipe_surface_reference(&dst_view, NULL);
    pipe_sampler_view_reference(&src_view, NULL);
}

static bool r300_is_simple_msaa_resolve(const struct pipe_blit_info *info)
{
    unsigned dst_width = u_minify(info->dst.resource->width0, info->dst.level);
    unsigned dst_height = u_minify(info->dst.resource->height0, info->dst.level);

    return info->src.resource->nr_samples > 1 &&
           info->dst.resource->nr_samples <= 1 &&
           info->dst.resource->format == info->src.resource->format &&
           info->dst.resource->format == info->dst.format &&
           info->src.resource->format == info->src.format &&
           !info->scissor_enable &&
           info->mask == PIPE_MASK_RGBA &&
           dst_width == info->src.resource->width0 &&
           dst_height == info->src.resource->height0 &&
           info->dst.box.x == 0 &&
           info->dst.box.y == 0 &&
           info->dst.box.width == dst_width &&
           info->dst.box.height == dst_height &&
           info->src.box.x == 0 &&
           info->src.box.y == 0 &&
           info->src.box.width == dst_width &&
           info->src.box.height == dst_height &&
           (r300_resource(info->dst.resource)->tex.microtile != RADEON_LAYOUT_LINEAR ||
            r300_resource(info->dst.resource)->tex.macrotile[info->dst.level] != RADEON_LAYOUT_LINEAR);
}

static void r300_simple_msaa_resolve(struct pipe_context *pipe,
                                     struct pipe_resource *dst,
                                     unsigned dst_level,
                                     unsigned dst_layer,
                                     struct pipe_resource *src,
                                     enum pipe_format format)
{
    struct r300_context *r300 = r300_context(pipe);
    struct r300_surface *srcsurf, *dstsurf;
    struct pipe_surface surf_tmpl;
    struct r300_aa_state *aa = (struct r300_aa_state*)r300->aa_state.state;

    memset(&surf_tmpl, 0, sizeof(surf_tmpl));
    surf_tmpl.format = format;
    srcsurf = r300_surface(pipe->create_surface(pipe, src, &surf_tmpl));

    surf_tmpl.format = format;
    surf_tmpl.u.tex.level = dst_level;
    surf_tmpl.u.tex.first_layer =
    surf_tmpl.u.tex.last_layer = dst_layer;
    dstsurf = r300_surface(pipe->create_surface(pipe, dst, &surf_tmpl));

    /* COLORPITCH should contain the tiling info of the resolve buffer.
     * The tiling of the AA buffer isn't programmable anyway. */
    srcsurf->pitch &= ~(R300_COLOR_TILE(1) | R300_COLOR_MICROTILE(3));
    srcsurf->pitch |= dstsurf->pitch & (R300_COLOR_TILE(1) | R300_COLOR_MICROTILE(3));

    /* Enable AA resolve. */
    aa->dest = dstsurf;
    r300->aa_state.size = 8;
    r300_mark_atom_dirty(r300, &r300->aa_state);

    /* Resolve the surface. */
    r300_blitter_begin(r300, R300_CLEAR_SURFACE);
    util_blitter_custom_color(r300->blitter, &srcsurf->base, NULL);
    r300_blitter_end(r300);

    /* Disable AA resolve. */
    aa->dest = NULL;
    r300->aa_state.size = 4;
    r300_mark_atom_dirty(r300, &r300->aa_state);

    pipe_surface_reference((struct pipe_surface**)&srcsurf, NULL);
    pipe_surface_reference((struct pipe_surface**)&dstsurf, NULL);
}

static void r300_msaa_resolve(struct pipe_context *pipe,
                              const struct pipe_blit_info *info)
{
    struct r300_context *r300 = r300_context(pipe);
    struct pipe_screen *screen = pipe->screen;
    struct pipe_resource *tmp, templ;
    struct pipe_blit_info blit;

    assert(info->src.level == 0);
    assert(info->src.box.z == 0);
    assert(info->src.box.depth == 1);
    assert(info->dst.box.depth == 1);

    if (r300_is_simple_msaa_resolve(info)) {
        r300_simple_msaa_resolve(pipe, info->dst.resource, info->dst.level,
                                 info->dst.box.z, info->src.resource,
                                 info->src.format);
        return;
    }

    /* resolve into a temporary texture, then blit */
    memset(&templ, 0, sizeof(templ));
    templ.target = PIPE_TEXTURE_2D;
    templ.format = info->src.resource->format;
    templ.width0 = info->src.resource->width0;
    templ.height0 = info->src.resource->height0;
    templ.depth0 = 1;
    templ.array_size = 1;
    templ.usage = PIPE_USAGE_DEFAULT;
    templ.flags = R300_RESOURCE_FORCE_MICROTILING;

    tmp = screen->resource_create(screen, &templ);

    /* resolve */
    r300_simple_msaa_resolve(pipe, tmp, 0, 0, info->src.resource,
                             info->src.format);

    /* blit */
    blit = *info;
    blit.src.resource = tmp;
    blit.src.box.z = 0;

    r300_blitter_begin(r300, R300_BLIT | R300_IGNORE_RENDER_COND);
    util_blitter_blit(r300->blitter, &blit);
    r300_blitter_end(r300);

    pipe_resource_reference(&tmp, NULL);
}

static void r300_blit(struct pipe_context *pipe,
                      const struct pipe_blit_info *blit)
{
    struct r300_context *r300 = r300_context(pipe);
    struct pipe_framebuffer_state *fb =
        (struct pipe_framebuffer_state*)r300->fb_state.state;
    struct pipe_blit_info info = *blit;

    /* The driver supports sRGB textures but not framebuffers. Blitting
     * from sRGB to sRGB should be the same as blitting from linear
     * to linear, so use that, This avoids incorrect linearization.
     */
    if (util_format_is_srgb(info.src.format)) {
      info.src.format = util_format_linear(info.src.format);
      info.dst.format = util_format_linear(info.dst.format);
    }

    /* MSAA resolve. */
    if (info.src.resource->nr_samples > 1 &&
        !util_format_is_depth_or_stencil(info.src.resource->format)) {
        r300_msaa_resolve(pipe, &info);
        return;
    }

    /* Can't read MSAA textures. */
    if (info.src.resource->nr_samples > 1) {
        return;
    }

    /* Blit a combined depth-stencil resource as color.
     * S8Z24 is the only supported stencil format. */
    if ((info.mask & PIPE_MASK_S) &&
        info.src.format == PIPE_FORMAT_S8_UINT_Z24_UNORM &&
        info.dst.format == PIPE_FORMAT_S8_UINT_Z24_UNORM) {
        if (info.dst.resource->nr_samples > 1) {
            /* Cannot do that with MSAA buffers. */
            info.mask &= ~PIPE_MASK_S;
            if (!(info.mask & PIPE_MASK_Z)) {
                return;
            }
        } else {
            /* Single-sample buffer. */
            info.src.format = PIPE_FORMAT_B8G8R8A8_UNORM;
            info.dst.format = PIPE_FORMAT_B8G8R8A8_UNORM;
            if (info.mask & PIPE_MASK_Z) {
                info.mask = PIPE_MASK_RGBA; /* depth+stencil */
            } else {
                info.mask = PIPE_MASK_B; /* stencil only */
            }
        }
    }

    /* Decompress ZMASK. */
    if (r300->zmask_in_use && !r300->locked_zbuffer) {
        if (fb->zsbuf->texture == info.src.resource ||
            fb->zsbuf->texture == info.dst.resource) {
            r300_decompress_zmask(r300);
        }
    }

    r300_blitter_begin(r300, R300_BLIT |
		       (info.render_condition_enable ? 0 : R300_IGNORE_RENDER_COND));
    util_blitter_blit(r300->blitter, &info);
    r300_blitter_end(r300);
}

static void r300_flush_resource(struct pipe_context *ctx,
				struct pipe_resource *resource)
{
}

void r300_init_blit_functions(struct r300_context *r300)
{
    r300->context.clear = r300_clear;
    r300->context.clear_render_target = r300_clear_render_target;
    r300->context.clear_depth_stencil = r300_clear_depth_stencil;
    r300->context.resource_copy_region = r300_resource_copy_region;
    r300->context.blit = r300_blit;
    r300->context.flush_resource = r300_flush_resource;
}
