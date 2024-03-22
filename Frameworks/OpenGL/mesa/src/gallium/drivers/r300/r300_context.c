/*
 * Copyright 2008 Corbin Simpson <MostAwesomeDude@gmail.com>
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

#include "draw/draw_context.h"

#include "util/u_memory.h"
#include "util/u_sampler.h"
#include "util/u_upload_mgr.h"
#include "util/u_debug_cb.h"
#include "util/os_time.h"
#include "vl/vl_decoder.h"
#include "vl/vl_video_buffer.h"

#include "r300_cb.h"
#include "r300_context.h"
#include "r300_emit.h"
#include "r300_screen.h"
#include "r300_screen_buffer.h"
#include "compiler/radeon_regalloc.h"

#include <inttypes.h>

static void r300_release_referenced_objects(struct r300_context *r300)
{
    struct pipe_framebuffer_state *fb =
            (struct pipe_framebuffer_state*)r300->fb_state.state;
    struct r300_textures_state *textures =
            (struct r300_textures_state*)r300->textures_state.state;
    unsigned i;

    /* Framebuffer state. */
    util_unreference_framebuffer_state(fb);

    /* Textures. */
    for (i = 0; i < textures->sampler_view_count; i++)
        pipe_sampler_view_reference(
                (struct pipe_sampler_view**)&textures->sampler_views[i], NULL);

    /* The special dummy texture for texkill. */
    if (r300->texkill_sampler) {
        pipe_sampler_view_reference(
                (struct pipe_sampler_view**)&r300->texkill_sampler,
                NULL);
    }

    /* Manually-created vertex buffers. */
    pipe_vertex_buffer_unreference(&r300->dummy_vb);
    radeon_bo_reference(r300->rws, &r300->vbo, NULL);

    r300->context.delete_depth_stencil_alpha_state(&r300->context,
                                                   r300->dsa_decompress_zmask);
}

static void r300_destroy_context(struct pipe_context* context)
{
    struct r300_context* r300 = r300_context(context);

    if (r300->cs.priv && r300->hyperz_enabled) {
        r300->rws->cs_request_feature(&r300->cs, RADEON_FID_R300_HYPERZ_ACCESS, false);
    }
    if (r300->cs.priv && r300->cmask_access) {
        r300->rws->cs_request_feature(&r300->cs, RADEON_FID_R300_CMASK_ACCESS, false);
    }

    if (r300->blitter)
        util_blitter_destroy(r300->blitter);
    if (r300->draw)
        draw_destroy(r300->draw);

    for (unsigned i = 0; i < r300->nr_vertex_buffers; i++)
       pipe_vertex_buffer_unreference(&r300->vertex_buffer[i]);

    if (r300->uploader)
        u_upload_destroy(r300->uploader);
    if (r300->context.stream_uploader)
        u_upload_destroy(r300->context.stream_uploader);
    if (r300->context.const_uploader)
       u_upload_destroy(r300->context.const_uploader);

    /* XXX: This function assumes r300->query_list was initialized */
    r300_release_referenced_objects(r300);

    r300->rws->cs_destroy(&r300->cs);
    if (r300->ctx)
        r300->rws->ctx_destroy(r300->ctx);

    rc_destroy_regalloc_state(&r300->fs_regalloc_state);
    rc_destroy_regalloc_state(&r300->vs_regalloc_state);

    /* XXX: No way to tell if this was initialized or not? */
    slab_destroy_child(&r300->pool_transfers);

    /* Free the structs allocated in r300_setup_atoms() */
    if (r300->aa_state.state) {
        FREE(r300->aa_state.state);
        FREE(r300->blend_color_state.state);
        FREE(r300->clip_state.state);
        FREE(r300->fb_state.state);
        FREE(r300->gpu_flush.state);
        FREE(r300->hyperz_state.state);
        FREE(r300->invariant_state.state);
        FREE(r300->rs_block_state.state);
        FREE(r300->sample_mask.state);
        FREE(r300->scissor_state.state);
        FREE(r300->textures_state.state);
        FREE(r300->vap_invariant_state.state);
        FREE(r300->viewport_state.state);
        FREE(r300->ztop_state.state);
        FREE(r300->fs_constants.state);
        FREE(r300->vs_constants.state);
        if (!r300->screen->caps.has_tcl) {
            FREE(r300->vertex_stream_state.state);
        }
    }

    FREE(r300->stencilref_fallback);

    FREE(r300);
}

static void r300_flush_callback(void *data, unsigned flags,
				struct pipe_fence_handle **fence)
{
    struct r300_context* const cs_context_copy = data;

    r300_flush(&cs_context_copy->context, flags, fence);
}

#define R300_INIT_ATOM(atomname, atomsize) \
 do { \
    r300->atomname.name = #atomname; \
    r300->atomname.state = NULL; \
    r300->atomname.size = atomsize; \
    r300->atomname.emit = r300_emit_##atomname; \
    r300->atomname.dirty = false; \
 } while (0)

#define R300_ALLOC_ATOM(atomname, statetype) \
do { \
    r300->atomname.state = CALLOC_STRUCT(statetype); \
    if (r300->atomname.state == NULL) \
        return false; \
} while (0)

static bool r300_setup_atoms(struct r300_context* r300)
{
    bool is_rv350 = r300->screen->caps.is_rv350;
    bool is_r500 = r300->screen->caps.is_r500;
    bool has_tcl = r300->screen->caps.has_tcl;

    /* Create the actual atom list.
     *
     * Some atoms never change size, others change every emit - those have
     * the size of 0 here.
     *
     * NOTE: The framebuffer state is split into these atoms:
     * - gpu_flush          (unpipelined regs)
     * - aa_state           (unpipelined regs)
     * - fb_state           (unpipelined regs)
     * - hyperz_state       (unpipelined regs followed by pipelined ones)
     * - fb_state_pipelined (pipelined regs)
     * The motivation behind this is to be able to emit a strict
     * subset of the regs, and to have reasonable register ordering. */
    /* SC, GB (unpipelined), RB3D (unpipelined), ZB (unpipelined). */
    R300_INIT_ATOM(gpu_flush, 9);
    R300_INIT_ATOM(aa_state, 4);
    R300_INIT_ATOM(fb_state, 0);
    R300_INIT_ATOM(hyperz_state, is_r500 || is_rv350 ? 10 : 8);
    /* ZB (unpipelined), SC. */
    R300_INIT_ATOM(ztop_state, 2);
    /* ZB, FG. */
    R300_INIT_ATOM(dsa_state, is_r500 ? 10 : 6);
    /* RB3D. */
    R300_INIT_ATOM(blend_state, 8);
    R300_INIT_ATOM(blend_color_state, is_r500 ? 3 : 2);
    /* SC. */
    R300_INIT_ATOM(sample_mask, 2);
    R300_INIT_ATOM(scissor_state, 3);
    /* GB, FG, GA, SU, SC, RB3D. */
    R300_INIT_ATOM(invariant_state, 14 + (is_rv350 ? 4 : 0) + (is_r500 ? 4 : 0));
    /* VAP. */
    R300_INIT_ATOM(viewport_state, 9);
    R300_INIT_ATOM(pvs_flush, 2);
    R300_INIT_ATOM(vap_invariant_state, is_r500 || !has_tcl ? 11 : 9);
    R300_INIT_ATOM(vertex_stream_state, 0);
    R300_INIT_ATOM(vs_state, 0);
    R300_INIT_ATOM(vs_constants, 0);
    R300_INIT_ATOM(clip_state, has_tcl ? 3 + (6 * 4) : 0);
    /* VAP, RS, GA, GB, SU, SC. */
    R300_INIT_ATOM(rs_block_state, 0);
    R300_INIT_ATOM(rs_state, 0);
    /* SC, US. */
    R300_INIT_ATOM(fb_state_pipelined, 8);
    /* US. */
    R300_INIT_ATOM(fs, 0);
    R300_INIT_ATOM(fs_rc_constant_state, 0);
    R300_INIT_ATOM(fs_constants, 0);
    /* TX. */
    R300_INIT_ATOM(texture_cache_inval, 2);
    R300_INIT_ATOM(textures_state, 0);
    /* Clear commands */
    R300_INIT_ATOM(hiz_clear, r300->screen->caps.hiz_ram > 0 ? 4 : 0);
    R300_INIT_ATOM(zmask_clear, r300->screen->caps.zmask_ram > 0 ? 4 : 0);
    R300_INIT_ATOM(cmask_clear, 4);
    /* ZB (unpipelined), SU. */
    R300_INIT_ATOM(query_start, 4);

    /* Replace emission functions for r500. */
    if (is_r500) {
        r300->fs.emit = r500_emit_fs;
        r300->fs_rc_constant_state.emit = r500_emit_fs_rc_constant_state;
        r300->fs_constants.emit = r500_emit_fs_constants;
    }

    /* Some non-CSO atoms need explicit space to store the state locally. */
    R300_ALLOC_ATOM(aa_state, r300_aa_state);
    R300_ALLOC_ATOM(blend_color_state, r300_blend_color_state);
    R300_ALLOC_ATOM(clip_state, r300_clip_state);
    R300_ALLOC_ATOM(hyperz_state, r300_hyperz_state);
    R300_ALLOC_ATOM(invariant_state, r300_invariant_state);
    R300_ALLOC_ATOM(textures_state, r300_textures_state);
    R300_ALLOC_ATOM(vap_invariant_state, r300_vap_invariant_state);
    R300_ALLOC_ATOM(viewport_state, r300_viewport_state);
    R300_ALLOC_ATOM(ztop_state, r300_ztop_state);
    R300_ALLOC_ATOM(fb_state, pipe_framebuffer_state);
    R300_ALLOC_ATOM(gpu_flush, pipe_framebuffer_state);
    r300->sample_mask.state = malloc(4);
    R300_ALLOC_ATOM(scissor_state, pipe_scissor_state);
    R300_ALLOC_ATOM(rs_block_state, r300_rs_block);
    R300_ALLOC_ATOM(fs_constants, r300_constant_buffer);
    R300_ALLOC_ATOM(vs_constants, r300_constant_buffer);
    if (!r300->screen->caps.has_tcl) {
        R300_ALLOC_ATOM(vertex_stream_state, r300_vertex_stream_state);
    }

    /* Some non-CSO atoms don't use the state pointer. */
    r300->fb_state_pipelined.allow_null_state = true;
    r300->fs_rc_constant_state.allow_null_state = true;
    r300->pvs_flush.allow_null_state = true;
    r300->query_start.allow_null_state = true;
    r300->texture_cache_inval.allow_null_state = true;

    /* Some states must be marked as dirty here to properly set up
     * hardware in the first command stream. */
    r300_mark_atom_dirty(r300, &r300->invariant_state);
    r300_mark_atom_dirty(r300, &r300->pvs_flush);
    r300_mark_atom_dirty(r300, &r300->vap_invariant_state);
    r300_mark_atom_dirty(r300, &r300->texture_cache_inval);
    r300_mark_atom_dirty(r300, &r300->textures_state);

    return true;
}

/* Not every gallium frontend calls every driver function before the first draw
 * call and we must initialize the command buffers somehow. */
static void r300_init_states(struct pipe_context *pipe)
{
    struct r300_context *r300 = r300_context(pipe);
    struct pipe_blend_color bc = {{0}};
    struct pipe_clip_state cs = {{{0}}};
    struct pipe_scissor_state ss = {0};
    struct r300_gpu_flush *gpuflush =
            (struct r300_gpu_flush*)r300->gpu_flush.state;
    struct r300_vap_invariant_state *vap_invariant =
            (struct r300_vap_invariant_state*)r300->vap_invariant_state.state;
    struct r300_invariant_state *invariant =
            (struct r300_invariant_state*)r300->invariant_state.state;

    CB_LOCALS;

    pipe->set_blend_color(pipe, &bc);
    pipe->set_clip_state(pipe, &cs);
    pipe->set_scissor_states(pipe, 0, 1, &ss);
    pipe->set_sample_mask(pipe, ~0);

    /* Initialize the GPU flush. */
    {
        BEGIN_CB(gpuflush->cb_flush_clean, 6);

        /* Flush and free renderbuffer caches. */
        OUT_CB_REG(R300_RB3D_DSTCACHE_CTLSTAT,
            R300_RB3D_DSTCACHE_CTLSTAT_DC_FREE_FREE_3D_TAGS |
            R300_RB3D_DSTCACHE_CTLSTAT_DC_FLUSH_FLUSH_DIRTY_3D);
        OUT_CB_REG(R300_ZB_ZCACHE_CTLSTAT,
            R300_ZB_ZCACHE_CTLSTAT_ZC_FLUSH_FLUSH_AND_FREE |
            R300_ZB_ZCACHE_CTLSTAT_ZC_FREE_FREE);

        /* Wait until the GPU is idle.
         * This fixes random pixels sometimes appearing probably caused
         * by incomplete rendering. */
        OUT_CB_REG(RADEON_WAIT_UNTIL, RADEON_WAIT_3D_IDLECLEAN);
        END_CB;
    }

    /* Initialize the VAP invariant state. */
    {
        BEGIN_CB(vap_invariant->cb, r300->vap_invariant_state.size);
        OUT_CB_REG(VAP_PVS_VTX_TIMEOUT_REG, 0xffff);
        OUT_CB_REG_SEQ(R300_VAP_GB_VERT_CLIP_ADJ, 4);
        OUT_CB_32F(1.0);
        OUT_CB_32F(1.0);
        OUT_CB_32F(1.0);
        OUT_CB_32F(1.0);
        OUT_CB_REG(R300_VAP_PSC_SGN_NORM_CNTL, R300_SGN_NORM_NO_ZERO);

        if (r300->screen->caps.is_r500) {
            OUT_CB_REG(R500_VAP_TEX_TO_COLOR_CNTL, 0);
        } else if (!r300->screen->caps.has_tcl) {
            /* RSxxx:
             * Static VAP setup since r300_emit_vs_state() is never called.
             */
            OUT_CB_REG(R300_VAP_CNTL, R300_PVS_NUM_SLOTS(10) |
                                      R300_PVS_NUM_CNTLRS(5) |
                                      R300_PVS_NUM_FPUS(2) |
                                      R300_PVS_VF_MAX_VTX_NUM(5));
        }
        END_CB;
    }

    /* Initialize the invariant state. */
    {
        BEGIN_CB(invariant->cb, r300->invariant_state.size);
        OUT_CB_REG(R300_GB_SELECT, 0);
        OUT_CB_REG(R300_FG_FOG_BLEND, 0);
        OUT_CB_REG(R300_GA_OFFSET, 0);
        OUT_CB_REG(R300_SU_TEX_WRAP, 0);
        OUT_CB_REG(R300_SU_DEPTH_SCALE, 0x4B7FFFFF);
        OUT_CB_REG(R300_SU_DEPTH_OFFSET, 0);
        OUT_CB_REG(R300_SC_EDGERULE, 0x2DA49525);

        if (r300->screen->caps.is_rv350) {
            OUT_CB_REG(R500_RB3D_DISCARD_SRC_PIXEL_LTE_THRESHOLD, 0x01010101);
            OUT_CB_REG(R500_RB3D_DISCARD_SRC_PIXEL_GTE_THRESHOLD, 0xFEFEFEFE);
        }

        if (r300->screen->caps.is_r500) {
            OUT_CB_REG(R500_GA_COLOR_CONTROL_PS3, 0);
            OUT_CB_REG(R500_SU_TEX_WRAP_PS3, 0);
        }
        END_CB;
    }

    /* Initialize the hyperz state. */
    {
        struct r300_hyperz_state *hyperz =
            (struct r300_hyperz_state*)r300->hyperz_state.state;
        BEGIN_CB(&hyperz->cb_flush_begin, r300->hyperz_state.size);
        OUT_CB_REG(R300_ZB_ZCACHE_CTLSTAT,
                   R300_ZB_ZCACHE_CTLSTAT_ZC_FLUSH_FLUSH_AND_FREE);
        OUT_CB_REG(R300_ZB_BW_CNTL, 0);
        OUT_CB_REG(R300_ZB_DEPTHCLEARVALUE, 0);
        OUT_CB_REG(R300_SC_HYPERZ, R300_SC_HYPERZ_ADJ_2);

        if (r300->screen->caps.is_r500 || r300->screen->caps.is_rv350) {
            OUT_CB_REG(R300_GB_Z_PEQ_CONFIG, 0);
        }
        END_CB;
    }
}

struct pipe_context* r300_create_context(struct pipe_screen* screen,
                                         void *priv, unsigned flags)
{
    struct r300_context* r300 = CALLOC_STRUCT(r300_context);
    struct r300_screen* r300screen = r300_screen(screen);
    struct radeon_winsys *rws = r300screen->rws;

    if (!r300)
        return NULL;

    r300->rws = rws;
    r300->screen = r300screen;

    r300->context.screen = screen;
    r300->context.priv = priv;
    r300->context.set_debug_callback = u_default_set_debug_callback;

    r300->context.destroy = r300_destroy_context;

    slab_create_child(&r300->pool_transfers, &r300screen->pool_transfers);

    r300->ctx = rws->ctx_create(rws, RADEON_CTX_PRIORITY_MEDIUM, false);
    if (!r300->ctx)
        goto fail;


    if (!rws->cs_create(&r300->cs, r300->ctx, AMD_IP_GFX, r300_flush_callback, r300))
        goto fail;

    if (!r300screen->caps.has_tcl) {
        /* Create a Draw. This is used for SW TCL. */
        r300->draw = draw_create(&r300->context);
        if (r300->draw == NULL)
            goto fail;
        /* Enable our renderer. */
        draw_set_rasterize_stage(r300->draw, r300_draw_stage(r300));
        /* Disable converting points/lines to triangles. */
        draw_wide_line_threshold(r300->draw, 10000000.f);
        draw_wide_point_threshold(r300->draw, 10000000.f);
        draw_wide_point_sprites(r300->draw, false);
        draw_enable_line_stipple(r300->draw, true);
        draw_enable_point_sprites(r300->draw, false);
    }

    if (!r300_setup_atoms(r300))
        goto fail;

    r300_init_blit_functions(r300);
    r300_init_flush_functions(r300);
    r300_init_query_functions(r300);
    r300_init_state_functions(r300);
    r300_init_resource_functions(r300);
    r300_init_render_functions(r300);
    r300_init_states(&r300->context);

    r300->context.create_video_codec = vl_create_decoder;
    r300->context.create_video_buffer = vl_video_buffer_create;

    r300->uploader = u_upload_create(&r300->context, 128 * 1024,
                                     PIPE_BIND_CUSTOM, PIPE_USAGE_STREAM, 0);
    r300->context.stream_uploader = u_upload_create(&r300->context, 1024 * 1024,
                                                    0, PIPE_USAGE_STREAM, 0);
    r300->context.const_uploader = u_upload_create(&r300->context, 1024 * 1024,
                                                   PIPE_BIND_CONSTANT_BUFFER,
                                                   PIPE_USAGE_STREAM, 0);

    r300->blitter = util_blitter_create(&r300->context);
    if (r300->blitter == NULL)
        goto fail;
    r300->blitter->draw_rectangle = r300_blitter_draw_rectangle;

    /* The KIL opcode needs the first texture unit to be enabled
     * on r3xx-r4xx. In order to calm down the CS checker, we bind this
     * dummy texture there. */
    if (!r300->screen->caps.is_r500) {
        struct pipe_resource *tex;
        struct pipe_resource rtempl = {0};
        struct pipe_sampler_view vtempl = {0};

        rtempl.target = PIPE_TEXTURE_2D;
        rtempl.format = PIPE_FORMAT_I8_UNORM;
        rtempl.usage = PIPE_USAGE_IMMUTABLE;
        rtempl.width0 = 1;
        rtempl.height0 = 1;
        rtempl.depth0 = 1;
        tex = screen->resource_create(screen, &rtempl);

        u_sampler_view_default_template(&vtempl, tex, tex->format);

        r300->texkill_sampler = (struct r300_sampler_view*)
            r300->context.create_sampler_view(&r300->context, tex, &vtempl);

        pipe_resource_reference(&tex, NULL);
    }

    if (r300screen->caps.has_tcl) {
        struct pipe_resource vb;
        memset(&vb, 0, sizeof(vb));
        vb.target = PIPE_BUFFER;
        vb.format = PIPE_FORMAT_R8_UNORM;
        vb.usage = PIPE_USAGE_DEFAULT;
        vb.width0 = sizeof(float) * 16;
        vb.height0 = 1;
        vb.depth0 = 1;

        r300->dummy_vb.buffer.resource = screen->resource_create(screen, &vb);
        r300->context.set_vertex_buffers(&r300->context, 1, 0, false, &r300->dummy_vb);
    }

    {
        struct pipe_depth_stencil_alpha_state dsa;
        memset(&dsa, 0, sizeof(dsa));
        dsa.depth_writemask = 1;

        r300->dsa_decompress_zmask =
            r300->context.create_depth_stencil_alpha_state(&r300->context,
                                                           &dsa);
    }

    r300->hyperz_time_of_last_flush = os_time_get();

    /* Register allocator state */
    rc_init_regalloc_state(&r300->fs_regalloc_state, RC_FRAGMENT_PROGRAM);
    rc_init_regalloc_state(&r300->vs_regalloc_state, RC_VERTEX_PROGRAM);

    /* Print driver info. */
#ifdef DEBUG
    {
#else
    if (DBG_ON(r300, DBG_INFO)) {
#endif
        fprintf(stderr,
                "r300: DRM version: %d.%d.%d, Name: %s, ID: 0x%04x, GB: %d, Z: %d\n"
                "r300: GART size: %u MB, VRAM size: %u MB\n"
                "r300: AA compression RAM: %s, Z compression RAM: %s, HiZ RAM: %s\n",
                r300->screen->info.drm_major,
                r300->screen->info.drm_minor,
                r300->screen->info.drm_patchlevel,
                screen->get_name(screen),
                r300->screen->info.pci_id,
                r300->screen->info.r300_num_gb_pipes,
                r300->screen->info.r300_num_z_pipes,
                r300->screen->info.gart_size_kb >> 10,
                r300->screen->info.vram_size_kb >> 10,
                "YES", /* XXX really? */
                r300->screen->caps.zmask_ram ? "YES" : "NO",
                r300->screen->caps.hiz_ram ? "YES" : "NO");
    }

    return &r300->context;

fail:
    r300_destroy_context(&r300->context);
    return NULL;
}
