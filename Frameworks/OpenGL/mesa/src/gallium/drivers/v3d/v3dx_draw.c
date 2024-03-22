/*
 * Copyright © 2014-2017 Broadcom
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "util/u_blitter.h"
#include "util/u_draw.h"
#include "util/u_prim.h"
#include "util/format/u_format.h"
#include "util/u_helpers.h"
#include "util/u_pack_color.h"
#include "util/u_prim_restart.h"
#include "util/u_upload_mgr.h"

#include "v3d_context.h"
#include "v3d_resource.h"
#include "v3d_cl.h"
#include "broadcom/compiler/v3d_compiler.h"
#include "broadcom/common/v3d_macros.h"
#include "broadcom/common/v3d_util.h"
#include "broadcom/common/v3d_csd.h"
#include "broadcom/cle/v3dx_pack.h"

void
v3dX(start_binning)(struct v3d_context *v3d, struct v3d_job *job)
{
        assert(job->needs_flush);

        /* Get space to emit our BCL state, using a branch to jump to a new BO
         * if necessary.
         */

        v3d_cl_ensure_space_with_branch(&job->bcl, 256 /* XXX */);

        job->submit.bcl_start = job->bcl.bo->offset;
        v3d_job_add_bo(job, job->bcl.bo);

        /* The PTB will request the tile alloc initial size per tile at start
         * of tile binning.
         */
        uint32_t tile_alloc_size =
                MAX2(job->num_layers, 1) * job->draw_tiles_x * job->draw_tiles_y * 64;

        /* The PTB allocates in aligned 4k chunks after the initial setup. */
        tile_alloc_size = align(tile_alloc_size, 4096);

        /* Include the first two chunk allocations that the PTB does so that
         * we definitely clear the OOM condition before triggering one (the HW
         * won't trigger OOM during the first allocations).
         */
        tile_alloc_size += 8192;

        /* For performance, allocate some extra initial memory after the PTB's
         * minimal allocations, so that we hopefully don't have to block the
         * GPU on the kernel handling an OOM signal.
         */
        tile_alloc_size += 512 * 1024;

        job->tile_alloc = v3d_bo_alloc(v3d->screen, tile_alloc_size,
                                       "tile_alloc");
        uint32_t tsda_per_tile_size = 256;
        job->tile_state = v3d_bo_alloc(v3d->screen,
                                       MAX2(job->num_layers, 1) *
                                       job->draw_tiles_y *
                                       job->draw_tiles_x *
                                       tsda_per_tile_size,
                                       "TSDA");

        /* This must go before the binning mode configuration. It is
         * required for layered framebuffers to work.
         */
        if (job->num_layers > 0) {
                cl_emit(&job->bcl, NUMBER_OF_LAYERS, config) {
                        config.number_of_layers = job->num_layers;
                }
        }

        assert(!job->msaa || !job->double_buffer);
#if V3D_VERSION >= 71
        cl_emit(&job->bcl, TILE_BINNING_MODE_CFG, config) {
                config.width_in_pixels = job->draw_width;
                config.height_in_pixels = job->draw_height;

                config.log2_tile_width = log2_tile_size(job->tile_width);
                config.log2_tile_height = log2_tile_size(job->tile_height);

                /* FIXME: ideallly we would like next assert on the packet header (as is
                 * general, so also applies to GL). We would need to expand
                 * gen_pack_header for that.
                 */
                assert(config.log2_tile_width == config.log2_tile_height ||
                       config.log2_tile_width == config.log2_tile_height + 1);
        }

#endif

#if V3D_VERSION == 42
        cl_emit(&job->bcl, TILE_BINNING_MODE_CFG, config) {
                config.width_in_pixels = job->draw_width;
                config.height_in_pixels = job->draw_height;
                config.number_of_render_targets =
                        MAX2(job->nr_cbufs, 1);

                config.multisample_mode_4x = job->msaa;
                config.double_buffer_in_non_ms_mode = job->double_buffer;

                config.maximum_bpp_of_all_render_targets = job->internal_bpp;
        }
#endif

        /* There's definitely nothing in the VCD cache we want. */
        cl_emit(&job->bcl, FLUSH_VCD_CACHE, bin);

        /* Disable any leftover OQ state from another job. */
        cl_emit(&job->bcl, OCCLUSION_QUERY_COUNTER, counter);

        /* "Binning mode lists must have a Start Tile Binning item (6) after
         *  any prefix state data before the binning list proper starts."
         */
        cl_emit(&job->bcl, START_TILE_BINNING, bin);
}
/**
 * Does the initial bining command list setup for drawing to a given FBO.
 */
static void
v3d_start_draw(struct v3d_context *v3d)
{
        struct v3d_job *job = v3d->job;

        if (job->needs_flush)
                return;

        job->needs_flush = true;
        job->draw_width = v3d->framebuffer.width;
        job->draw_height = v3d->framebuffer.height;
        job->num_layers = util_framebuffer_get_num_layers(&v3d->framebuffer);

        v3dX(start_binning)(v3d, job);
}

static void
v3d_predraw_check_stage_inputs(struct pipe_context *pctx,
                               enum pipe_shader_type s)
{
        struct v3d_context *v3d = v3d_context(pctx);

        /* Flush writes to textures we're sampling. */
        for (int i = 0; i < v3d->tex[s].num_textures; i++) {
                struct pipe_sampler_view *pview = v3d->tex[s].textures[i];
                if (!pview)
                        continue;
                struct v3d_sampler_view *view = v3d_sampler_view(pview);

                if (view->texture != view->base.texture &&
                    view->base.format != PIPE_FORMAT_X32_S8X24_UINT)
                        v3d_update_shadow_texture(pctx, &view->base);

                v3d_flush_jobs_writing_resource(v3d, view->texture,
                                                V3D_FLUSH_DEFAULT,
                                                s == PIPE_SHADER_COMPUTE);
        }

        /* Flush writes to UBOs. */
        u_foreach_bit(i, v3d->constbuf[s].enabled_mask) {
                struct pipe_constant_buffer *cb = &v3d->constbuf[s].cb[i];
                if (cb->buffer) {
                        v3d_flush_jobs_writing_resource(v3d, cb->buffer,
                                                        V3D_FLUSH_DEFAULT,
                                                        s == PIPE_SHADER_COMPUTE);
                }
        }

        /* Flush reads/writes to our SSBOs */
        u_foreach_bit(i, v3d->ssbo[s].enabled_mask) {
                struct pipe_shader_buffer *sb = &v3d->ssbo[s].sb[i];
                if (sb->buffer) {
                        v3d_flush_jobs_reading_resource(v3d, sb->buffer,
                                                        V3D_FLUSH_NOT_CURRENT_JOB,
                                                        s == PIPE_SHADER_COMPUTE);
                }
        }

        /* Flush reads/writes to our image views */
        u_foreach_bit(i, v3d->shaderimg[s].enabled_mask) {
                struct v3d_image_view *view = &v3d->shaderimg[s].si[i];

                v3d_flush_jobs_reading_resource(v3d, view->base.resource,
                                                V3D_FLUSH_NOT_CURRENT_JOB,
                                                s == PIPE_SHADER_COMPUTE);
        }

        /* Flush writes to our vertex buffers (i.e. from transform feedback) */
        if (s == PIPE_SHADER_VERTEX) {
                u_foreach_bit(i, v3d->vertexbuf.enabled_mask) {
                        struct pipe_vertex_buffer *vb = &v3d->vertexbuf.vb[i];

                        v3d_flush_jobs_writing_resource(v3d, vb->buffer.resource,
                                                        V3D_FLUSH_DEFAULT,
                                                        false);
                }
        }
}

static void
v3d_predraw_check_outputs(struct pipe_context *pctx)
{
        struct v3d_context *v3d = v3d_context(pctx);

        /* Flush jobs reading from TF buffers that we are about to write. */
        if (v3d_transform_feedback_enabled(v3d)) {
                struct v3d_streamout_stateobj *so = &v3d->streamout;

                for (int i = 0; i < so->num_targets; i++) {
                        if (!so->targets[i])
                                continue;

                        const struct pipe_stream_output_target *target =
                                so->targets[i];
                        v3d_flush_jobs_reading_resource(v3d, target->buffer,
                                                        V3D_FLUSH_DEFAULT,
                                                        false);
                }
        }
}

/**
 * Checks if the state for the current draw reads a particular resource in
 * in the given shader stage.
 */
static bool
v3d_state_reads_resource(struct v3d_context *v3d,
                         struct pipe_resource *prsc,
                         enum pipe_shader_type s)
{
        struct v3d_resource *rsc = v3d_resource(prsc);

        /* Vertex buffers */
        if (s == PIPE_SHADER_VERTEX) {
                u_foreach_bit(i, v3d->vertexbuf.enabled_mask) {
                        struct pipe_vertex_buffer *vb = &v3d->vertexbuf.vb[i];
                        if (!vb->buffer.resource)
                                continue;

                        struct v3d_resource *vb_rsc =
                                v3d_resource(vb->buffer.resource);
                        if (rsc->bo == vb_rsc->bo)
                                return true;
                }
        }

        /* Constant buffers */
        u_foreach_bit(i, v3d->constbuf[s].enabled_mask) {
                struct pipe_constant_buffer *cb = &v3d->constbuf[s].cb[i];
                if (!cb->buffer)
                        continue;

                struct v3d_resource *cb_rsc = v3d_resource(cb->buffer);
                if (rsc->bo == cb_rsc->bo)
                        return true;
        }

        /* Shader storage buffers */
        u_foreach_bit(i, v3d->ssbo[s].enabled_mask) {
                struct pipe_shader_buffer *sb = &v3d->ssbo[s].sb[i];
                if (!sb->buffer)
                        continue;

                struct v3d_resource *sb_rsc = v3d_resource(sb->buffer);
                if (rsc->bo == sb_rsc->bo)
                        return true;
        }

        /* Textures  */
        for (int i = 0; i < v3d->tex[s].num_textures; i++) {
                struct pipe_sampler_view *pview = v3d->tex[s].textures[i];
                if (!pview)
                        continue;

                struct v3d_sampler_view *view = v3d_sampler_view(pview);
                struct v3d_resource *v_rsc = v3d_resource(view->texture);
                if (rsc->bo == v_rsc->bo)
                        return true;
        }

        return false;
}

static void
v3d_emit_wait_for_tf(struct v3d_job *job)
{
        /* XXX: we might be able to skip this in some cases, for now we
         * always emit it.
         */
        cl_emit(&job->bcl, FLUSH_TRANSFORM_FEEDBACK_DATA, flush);

        cl_emit(&job->bcl, WAIT_FOR_TRANSFORM_FEEDBACK, wait) {
                /* XXX: Wait for all outstanding writes... maybe we can do
                 * better in some cases.
                 */
                wait.block_count = 255;
        }

        /* We have just flushed all our outstanding TF work in this job so make
         * sure we don't emit TF flushes again for any of it again.
         */
        _mesa_set_clear(job->tf_write_prscs, NULL);
}

static void
v3d_emit_wait_for_tf_if_needed(struct v3d_context *v3d, struct v3d_job *job)
{
        if (!job->tf_enabled)
            return;

        set_foreach(job->tf_write_prscs, entry) {
                struct pipe_resource *prsc = (struct pipe_resource *)entry->key;
                for (int s = 0; s < PIPE_SHADER_COMPUTE; s++) {
                        /* Fragment shaders can only start executing after all
                         * binning (and thus TF) is complete.
                         *
                         * XXX: For VS/GS/TES, if the binning shader does not
                         * read the resource then we could also avoid emitting
                         * the wait.
                         */
                        if (s == PIPE_SHADER_FRAGMENT)
                            continue;

                        if (v3d_state_reads_resource(v3d, prsc, s)) {
                                v3d_emit_wait_for_tf(job);
                                return;
                        }
                }
        }
}

static void
v3d_emit_gs_state_record(struct v3d_job *job,
                         struct v3d_compiled_shader *gs_bin,
                         struct v3d_cl_reloc gs_bin_uniforms,
                         struct v3d_compiled_shader *gs,
                         struct v3d_cl_reloc gs_render_uniforms)
{
        cl_emit(&job->indirect, GEOMETRY_SHADER_STATE_RECORD, shader) {
                shader.geometry_bin_mode_shader_code_address =
                        cl_address(v3d_resource(gs_bin->resource)->bo,
                                   gs_bin->offset);
                shader.geometry_bin_mode_shader_4_way_threadable =
                        gs_bin->prog_data.gs->base.threads == 4;
                shader.geometry_bin_mode_shader_start_in_final_thread_section =
                        gs_bin->prog_data.gs->base.single_seg;
#if V3D_VERSION == 42
                shader.geometry_bin_mode_shader_propagate_nans = true;
#endif
                shader.geometry_bin_mode_shader_uniforms_address =
                        gs_bin_uniforms;

                shader.geometry_render_mode_shader_code_address =
                        cl_address(v3d_resource(gs->resource)->bo, gs->offset);
                shader.geometry_render_mode_shader_4_way_threadable =
                        gs->prog_data.gs->base.threads == 4;
                shader.geometry_render_mode_shader_start_in_final_thread_section =
                        gs->prog_data.gs->base.single_seg;
#if V3D_VERSION == 42
                shader.geometry_render_mode_shader_propagate_nans = true;
#endif
                shader.geometry_render_mode_shader_uniforms_address =
                        gs_render_uniforms;
        }
}

static uint8_t
v3d_gs_output_primitive(enum mesa_prim prim_type)
{
    switch (prim_type) {
    case MESA_PRIM_POINTS:
        return GEOMETRY_SHADER_POINTS;
    case MESA_PRIM_LINE_STRIP:
        return GEOMETRY_SHADER_LINE_STRIP;
    case MESA_PRIM_TRIANGLE_STRIP:
        return GEOMETRY_SHADER_TRI_STRIP;
    default:
        unreachable("Unsupported primitive type");
    }
}

static void
v3d_emit_tes_gs_common_params(struct v3d_job *job,
                              uint8_t gs_out_prim_type,
                              uint8_t gs_num_invocations)
{
        /* This, and v3d_emit_tes_gs_shader_params below, fill in default
         * values for tessellation fields even though we don't support
         * tessellation yet because our packing functions (and the simulator)
         * complain if we don't.
         */
        cl_emit(&job->indirect, TESSELLATION_GEOMETRY_COMMON_PARAMS, shader) {
                shader.tessellation_type = TESSELLATION_TYPE_TRIANGLE;
                shader.tessellation_point_mode = false;
                shader.tessellation_edge_spacing = TESSELLATION_EDGE_SPACING_EVEN;
                shader.tessellation_clockwise = true;
                shader.tessellation_invocations = 1;

                shader.geometry_shader_output_format =
                        v3d_gs_output_primitive(gs_out_prim_type);
                shader.geometry_shader_instances = gs_num_invocations & 0x1F;
        }
}

static uint8_t
simd_width_to_gs_pack_mode(uint32_t width)
{
    switch (width) {
    case 16:
        return V3D_PACK_MODE_16_WAY;
    case 8:
        return V3D_PACK_MODE_8_WAY;
    case 4:
        return V3D_PACK_MODE_4_WAY;
    case 1:
        return V3D_PACK_MODE_1_WAY;
    default:
        unreachable("Invalid SIMD width");
    };
}

static void
v3d_emit_tes_gs_shader_params(struct v3d_job *job,
                              uint32_t gs_simd,
                              uint32_t gs_vpm_output_size,
                              uint32_t gs_max_vpm_input_size_per_batch)
{
        cl_emit(&job->indirect, TESSELLATION_GEOMETRY_SHADER_PARAMS, shader) {
                shader.tcs_batch_flush_mode = V3D_TCS_FLUSH_MODE_FULLY_PACKED;
                shader.per_patch_data_column_depth = 1;
                shader.tcs_output_segment_size_in_sectors = 1;
                shader.tcs_output_segment_pack_mode = V3D_PACK_MODE_16_WAY;
                shader.tes_output_segment_size_in_sectors = 1;
                shader.tes_output_segment_pack_mode = V3D_PACK_MODE_16_WAY;
                shader.gs_output_segment_size_in_sectors = gs_vpm_output_size;
                shader.gs_output_segment_pack_mode =
                        simd_width_to_gs_pack_mode(gs_simd);
                shader.tbg_max_patches_per_tcs_batch = 1;
                shader.tbg_max_extra_vertex_segs_for_patches_after_first = 0;
                shader.tbg_min_tcs_output_segments_required_in_play = 1;
                shader.tbg_min_per_patch_data_segments_required_in_play = 1;
                shader.tpg_max_patches_per_tes_batch = 1;
                shader.tpg_max_vertex_segments_per_tes_batch = 0;
                shader.tpg_max_tcs_output_segments_per_tes_batch = 1;
                shader.tpg_min_tes_output_segments_required_in_play = 1;
                shader.gbg_max_tes_output_vertex_segments_per_gs_batch =
                        gs_max_vpm_input_size_per_batch;
                shader.gbg_min_gs_output_segments_required_in_play = 1;
        }
}

static void
v3d_emit_gl_shader_state(struct v3d_context *v3d,
                         const struct pipe_draw_info *info)
{
        struct v3d_job *job = v3d->job;
        /* V3D_DIRTY_VTXSTATE */
        struct v3d_vertex_stateobj *vtx = v3d->vtx;
        /* V3D_DIRTY_VTXBUF */
        struct v3d_vertexbuf_stateobj *vertexbuf = &v3d->vertexbuf;

        /* Upload the uniforms to the indirect CL first */
        struct v3d_cl_reloc fs_uniforms =
                v3d_write_uniforms(v3d, job, v3d->prog.fs,
                                   PIPE_SHADER_FRAGMENT);

        struct v3d_cl_reloc gs_uniforms = { NULL, 0 };
        struct v3d_cl_reloc gs_bin_uniforms = { NULL, 0 };
        if (v3d->prog.gs) {
                gs_uniforms = v3d_write_uniforms(v3d, job, v3d->prog.gs,
                                                 PIPE_SHADER_GEOMETRY);
        }
        if (v3d->prog.gs_bin) {
                gs_bin_uniforms = v3d_write_uniforms(v3d, job, v3d->prog.gs_bin,
                                                     PIPE_SHADER_GEOMETRY);
        }

        struct v3d_cl_reloc vs_uniforms =
                v3d_write_uniforms(v3d, job, v3d->prog.vs,
                                   PIPE_SHADER_VERTEX);
        struct v3d_cl_reloc cs_uniforms =
                v3d_write_uniforms(v3d, job, v3d->prog.cs,
                                   PIPE_SHADER_VERTEX);

        /* Update the cache dirty flag based on the shader progs data */
        job->tmu_dirty_rcl |= v3d->prog.cs->prog_data.vs->base.tmu_dirty_rcl;
        job->tmu_dirty_rcl |= v3d->prog.vs->prog_data.vs->base.tmu_dirty_rcl;
        if (v3d->prog.gs_bin) {
                job->tmu_dirty_rcl |=
                        v3d->prog.gs_bin->prog_data.gs->base.tmu_dirty_rcl;
        }
        if (v3d->prog.gs) {
                job->tmu_dirty_rcl |=
                        v3d->prog.gs->prog_data.gs->base.tmu_dirty_rcl;
        }
        job->tmu_dirty_rcl |= v3d->prog.fs->prog_data.fs->base.tmu_dirty_rcl;

        uint32_t num_elements_to_emit = 0;
        for (int i = 0; i < vtx->num_elements; i++) {
                struct pipe_vertex_element *elem = &vtx->pipe[i];
                struct pipe_vertex_buffer *vb =
                        &vertexbuf->vb[elem->vertex_buffer_index];
                if (vb->buffer.resource)
                        num_elements_to_emit++;
        }

        uint32_t shader_state_record_length =
                cl_packet_length(GL_SHADER_STATE_RECORD);
        if (v3d->prog.gs) {
                shader_state_record_length +=
                        cl_packet_length(GEOMETRY_SHADER_STATE_RECORD) +
                        cl_packet_length(TESSELLATION_GEOMETRY_COMMON_PARAMS) +
                        2 * cl_packet_length(TESSELLATION_GEOMETRY_SHADER_PARAMS);
        }

        /* See GFXH-930 workaround below */
        uint32_t shader_rec_offset =
                    v3d_cl_ensure_space(&job->indirect,
                                    shader_state_record_length +
                                    MAX2(num_elements_to_emit, 1) *
                                    cl_packet_length(GL_SHADER_STATE_ATTRIBUTE_RECORD),
                                    32);

        /* XXX perf: We should move most of the SHADER_STATE_RECORD setup to
         * compile time, so that we mostly just have to OR the VS and FS
         * records together at draw time.
         */

        struct vpm_config vpm_cfg_bin, vpm_cfg;
        v3d_compute_vpm_config(&v3d->screen->devinfo,
                               v3d->prog.cs->prog_data.vs,
                               v3d->prog.vs->prog_data.vs,
                               v3d->prog.gs ? v3d->prog.gs_bin->prog_data.gs : NULL,
                               v3d->prog.gs ? v3d->prog.gs->prog_data.gs : NULL,
                               &vpm_cfg_bin,
                               &vpm_cfg);

        if (v3d->prog.gs) {
                v3d_emit_gs_state_record(v3d->job,
                                         v3d->prog.gs_bin, gs_bin_uniforms,
                                         v3d->prog.gs, gs_uniforms);

                struct v3d_gs_prog_data *gs = v3d->prog.gs->prog_data.gs;
                v3d_emit_tes_gs_common_params(v3d->job,
                                              gs->out_prim_type,
                                              gs->num_invocations);

                /* Bin Tes/Gs params */
                v3d_emit_tes_gs_shader_params(v3d->job,
                                              vpm_cfg_bin.gs_width,
                                              vpm_cfg_bin.Gd,
                                              vpm_cfg_bin.Gv);

                /* Render Tes/Gs params */
                v3d_emit_tes_gs_shader_params(v3d->job,
                                              vpm_cfg.gs_width,
                                              vpm_cfg.Gd,
                                              vpm_cfg.Gv);
        }

        cl_emit(&job->indirect, GL_SHADER_STATE_RECORD, shader) {
                shader.enable_clipping = true;
                /* V3D_DIRTY_PRIM_MODE | V3D_DIRTY_RASTERIZER */
                shader.point_size_in_shaded_vertex_data =
                        (info->mode == MESA_PRIM_POINTS &&
                         v3d->rasterizer->base.point_size_per_vertex);

                /* Must be set if the shader modifies Z, discards, or modifies
                 * the sample mask.  For any of these cases, the fragment
                 * shader needs to write the Z value (even just discards).
                 */
                shader.fragment_shader_does_z_writes =
                        v3d->prog.fs->prog_data.fs->writes_z;

                /* Set if the EZ test must be disabled (due to shader side
                 * effects and the early_z flag not being present in the
                 * shader).
                 */
                shader.turn_off_early_z_test =
                        v3d->prog.fs->prog_data.fs->disable_ez;

                shader.fragment_shader_uses_real_pixel_centre_w_in_addition_to_centroid_w2 =
                        v3d->prog.fs->prog_data.fs->uses_center_w;

                shader.any_shader_reads_hardware_written_primitive_id =
                        (v3d->prog.gs && v3d->prog.gs->prog_data.gs->uses_pid) ||
                        v3d->prog.fs->prog_data.fs->uses_pid;
                shader.insert_primitive_id_as_first_varying_to_fragment_shader =
                        !v3d->prog.gs && v3d->prog.fs->prog_data.fs->uses_pid;

                shader.do_scoreboard_wait_on_first_thread_switch =
                        v3d->prog.fs->prog_data.fs->lock_scoreboard_on_first_thrsw;
                shader.disable_implicit_point_line_varyings =
                        !v3d->prog.fs->prog_data.fs->uses_implicit_point_line_varyings;

                shader.number_of_varyings_in_fragment_shader =
                        v3d->prog.fs->prog_data.fs->num_inputs;

                shader.coordinate_shader_code_address =
                        cl_address(v3d_resource(v3d->prog.cs->resource)->bo,
                                   v3d->prog.cs->offset);
                shader.vertex_shader_code_address =
                        cl_address(v3d_resource(v3d->prog.vs->resource)->bo,
                                   v3d->prog.vs->offset);
                shader.fragment_shader_code_address =
                        cl_address(v3d_resource(v3d->prog.fs->resource)->bo,
                                   v3d->prog.fs->offset);

#if V3D_VERSION == 42
                shader.coordinate_shader_propagate_nans = true;
                shader.vertex_shader_propagate_nans = true;
                shader.fragment_shader_propagate_nans = true;

                /* XXX: Use combined input/output size flag in the common
                 * case.
                 */
                shader.coordinate_shader_has_separate_input_and_output_vpm_blocks =
                        v3d->prog.cs->prog_data.vs->separate_segments;
                shader.vertex_shader_has_separate_input_and_output_vpm_blocks =
                        v3d->prog.vs->prog_data.vs->separate_segments;
                shader.coordinate_shader_input_vpm_segment_size =
                        v3d->prog.cs->prog_data.vs->separate_segments ?
                        v3d->prog.cs->prog_data.vs->vpm_input_size : 1;
                shader.vertex_shader_input_vpm_segment_size =
                        v3d->prog.vs->prog_data.vs->separate_segments ?
                        v3d->prog.vs->prog_data.vs->vpm_input_size : 1;
#endif
                /* On V3D 7.1 there isn't a specific flag to set if we are using
                 * shared/separate segments or not. We just set the value of
                 * vpm_input_size to 0, and set output to the max needed. That should be
                 * already properly set on prog_data_vs_bin
                 */
#if V3D_VERSION == 71
                shader.coordinate_shader_input_vpm_segment_size =
                        v3d->prog.cs->prog_data.vs->vpm_input_size;
                shader.vertex_shader_input_vpm_segment_size =
                        v3d->prog.vs->prog_data.vs->vpm_input_size;
#endif

                shader.coordinate_shader_output_vpm_segment_size =
                        v3d->prog.cs->prog_data.vs->vpm_output_size;
                shader.vertex_shader_output_vpm_segment_size =
                        v3d->prog.vs->prog_data.vs->vpm_output_size;

                shader.coordinate_shader_uniforms_address = cs_uniforms;
                shader.vertex_shader_uniforms_address = vs_uniforms;
                shader.fragment_shader_uniforms_address = fs_uniforms;

                shader.min_coord_shader_input_segments_required_in_play =
                        vpm_cfg_bin.As;
                shader.min_vertex_shader_input_segments_required_in_play =
                        vpm_cfg.As;

                shader.min_coord_shader_output_segments_required_in_play_in_addition_to_vcm_cache_size =
                        vpm_cfg_bin.Ve;
                shader.min_vertex_shader_output_segments_required_in_play_in_addition_to_vcm_cache_size =
                        vpm_cfg.Ve;

                shader.coordinate_shader_4_way_threadable =
                        v3d->prog.cs->prog_data.vs->base.threads == 4;
                shader.vertex_shader_4_way_threadable =
                        v3d->prog.vs->prog_data.vs->base.threads == 4;
                shader.fragment_shader_4_way_threadable =
                        v3d->prog.fs->prog_data.fs->base.threads == 4;

                shader.coordinate_shader_start_in_final_thread_section =
                        v3d->prog.cs->prog_data.vs->base.single_seg;
                shader.vertex_shader_start_in_final_thread_section =
                        v3d->prog.vs->prog_data.vs->base.single_seg;
                shader.fragment_shader_start_in_final_thread_section =
                        v3d->prog.fs->prog_data.fs->base.single_seg;

                shader.vertex_id_read_by_coordinate_shader =
                        v3d->prog.cs->prog_data.vs->uses_vid;
                shader.instance_id_read_by_coordinate_shader =
                        v3d->prog.cs->prog_data.vs->uses_iid;
                shader.vertex_id_read_by_vertex_shader =
                        v3d->prog.vs->prog_data.vs->uses_vid;
                shader.instance_id_read_by_vertex_shader =
                        v3d->prog.vs->prog_data.vs->uses_iid;

#if V3D_VERSION == 42
                shader.address_of_default_attribute_values =
                        cl_address(v3d_resource(vtx->defaults)->bo,
                                   vtx->defaults_offset);
#endif
        }

        bool cs_loaded_any = false;
        for (int i = 0; i < vtx->num_elements; i++) {
                struct pipe_vertex_element *elem = &vtx->pipe[i];
                struct pipe_vertex_buffer *vb =
                        &vertexbuf->vb[elem->vertex_buffer_index];
                struct v3d_resource *rsc = v3d_resource(vb->buffer.resource);

                if (!rsc)
                        continue;

                enum { size = cl_packet_length(GL_SHADER_STATE_ATTRIBUTE_RECORD) };
                cl_emit_with_prepacked(&job->indirect,
                                       GL_SHADER_STATE_ATTRIBUTE_RECORD,
                                       &vtx->attrs[i * size], attr) {
                        attr.stride = elem->src_stride;
                        attr.address = cl_address(rsc->bo,
                                                  vb->buffer_offset +
                                                  elem->src_offset);
                        attr.number_of_values_read_by_coordinate_shader =
                                v3d->prog.cs->prog_data.vs->vattr_sizes[i];
                        attr.number_of_values_read_by_vertex_shader =
                                v3d->prog.vs->prog_data.vs->vattr_sizes[i];

                        /* GFXH-930: At least one attribute must be enabled
                         * and read by CS and VS.  If we have attributes being
                         * consumed by the VS but not the CS, then set up a
                         * dummy load of the last attribute into the CS's VPM
                         * inputs.  (Since CS is just dead-code-elimination
                         * compared to VS, we can't have CS loading but not
                         * VS).
                         */
                        if (v3d->prog.cs->prog_data.vs->vattr_sizes[i])
                                cs_loaded_any = true;
                        if (i == vtx->num_elements - 1 && !cs_loaded_any) {
                                attr.number_of_values_read_by_coordinate_shader = 1;
                        }
                        attr.maximum_index = 0xffffff;
                }
                STATIC_ASSERT(sizeof(vtx->attrs) >= V3D_MAX_VS_INPUTS / 4 * size);
        }

        if (num_elements_to_emit == 0) {
                /* GFXH-930: At least one attribute must be enabled and read
                 * by CS and VS.  If we have no attributes being consumed by
                 * the shader, set up a dummy to be loaded into the VPM.
                 */
                cl_emit(&job->indirect, GL_SHADER_STATE_ATTRIBUTE_RECORD, attr) {
                        /* Valid address of data whose value will be unused. */
                        attr.address = cl_address(job->indirect.bo, 0);

                        attr.type = ATTRIBUTE_FLOAT;
                        attr.stride = 0;
                        attr.vec_size = 1;

                        attr.number_of_values_read_by_coordinate_shader = 1;
                        attr.number_of_values_read_by_vertex_shader = 1;
                }
                num_elements_to_emit = 1;
        }

        cl_emit(&job->bcl, VCM_CACHE_SIZE, vcm) {
                vcm.number_of_16_vertex_batches_for_binning = vpm_cfg_bin.Vc;
                vcm.number_of_16_vertex_batches_for_rendering = vpm_cfg.Vc;
        }

        if (v3d->prog.gs) {
                cl_emit(&job->bcl, GL_SHADER_STATE_INCLUDING_GS, state) {
                        state.address = cl_address(job->indirect.bo,
                                                   shader_rec_offset);
                        state.number_of_attribute_arrays = num_elements_to_emit;
                }
        } else {
                cl_emit(&job->bcl, GL_SHADER_STATE, state) {
                        state.address = cl_address(job->indirect.bo,
                                                   shader_rec_offset);
                        state.number_of_attribute_arrays = num_elements_to_emit;
                }
        }

        v3d_bo_unreference(&cs_uniforms.bo);
        v3d_bo_unreference(&vs_uniforms.bo);
        if (gs_uniforms.bo)
                v3d_bo_unreference(&gs_uniforms.bo);
        if (gs_bin_uniforms.bo)
                v3d_bo_unreference(&gs_bin_uniforms.bo);
        v3d_bo_unreference(&fs_uniforms.bo);
}

/**
 * Updates the number of primitives generated from the number of vertices
 * to draw. This only works when no GS is present, since otherwise the number
 * of primitives generated cannot be determined in advance and we need to
 * use the PRIMITIVE_COUNTS_FEEDBACK command instead, however, that requires
 * a sync wait for the draw to complete, so we only use that when GS is present.
 */
static void
v3d_update_primitives_generated_counter(struct v3d_context *v3d,
                                        const struct pipe_draw_info *info,
                                        const struct pipe_draw_start_count_bias *draw)
{
        assert(!v3d->prog.gs);

        if (!v3d->active_queries)
                return;

        uint32_t prims = u_prims_for_vertices(info->mode, draw->count);
        v3d->prims_generated += prims;
}

static void
v3d_update_job_ez(struct v3d_context *v3d, struct v3d_job *job)
{
        /* If first_ez_state is V3D_EZ_DISABLED it means that we have already
         * determined that we should disable EZ completely for all draw calls
         * in this job. This will cause us to disable EZ for the entire job in
         * the Tile Rendering Mode RCL packet and when we do that we need to
         * make sure we never emit a draw call in the job with EZ enabled in
         * the CFG_BITS packet, so ez_state must also be V3D_EZ_DISABLED.
         */
        if (job->first_ez_state == V3D_EZ_DISABLED) {
                assert(job->ez_state == V3D_EZ_DISABLED);
                return;
        }

        /* If this is the first time we update EZ state for this job we first
         * check if there is anything that requires disabling it completely
         * for the entire job (based on state that is not related to the
         * current draw call and pipeline state).
         */
        if (!job->decided_global_ez_enable) {
                job->decided_global_ez_enable = true;

                if (!job->zsbuf) {
                        job->first_ez_state = V3D_EZ_DISABLED;
                        job->ez_state = V3D_EZ_DISABLED;
                        return;
                }

                /* GFXH-1918: the early-Z buffer may load incorrect depth
                 * values if the frame has odd width or height. Disable early-Z
                 * in this case.
                 */
                bool needs_depth_load = v3d->zsa && job->zsbuf &&
                        v3d->zsa->base.depth_enabled &&
                        (PIPE_CLEAR_DEPTH & ~job->clear);
                if (needs_depth_load &&
                     ((job->draw_width % 2 != 0) || (job->draw_height % 2 != 0))) {
                        perf_debug("Loading depth buffer for framebuffer with odd width "
                                   "or height disables early-Z tests\n");
                        job->first_ez_state = V3D_EZ_DISABLED;
                        job->ez_state = V3D_EZ_DISABLED;
                        return;
                }
        }

        switch (v3d->zsa->ez_state) {
        case V3D_EZ_UNDECIDED:
                /* If the Z/S state didn't pick a direction but didn't
                 * disable, then go along with the current EZ state.  This
                 * allows EZ optimization for Z func == EQUAL or NEVER.
                 */
                break;

        case V3D_EZ_LT_LE:
        case V3D_EZ_GT_GE:
                /* If the Z/S state picked a direction, then it needs to match
                 * the current direction if we've decided on one.
                 */
                if (job->ez_state == V3D_EZ_UNDECIDED)
                        job->ez_state = v3d->zsa->ez_state;
                else if (job->ez_state != v3d->zsa->ez_state)
                        job->ez_state = V3D_EZ_DISABLED;
                break;

        case V3D_EZ_DISABLED:
                /* If the current Z/S state disables EZ because of a bad Z
                 * func or stencil operation, then we can't do any more EZ in
                 * this frame.
                 */
                job->ez_state = V3D_EZ_DISABLED;
                break;
        }

        /* If the FS affects the Z of the pixels, then it may update against
         * the chosen EZ direction (though we could use
         * ARB_conservative_depth's hints to avoid this)
         */
        if (v3d->prog.fs->prog_data.fs->writes_z &&
            !v3d->prog.fs->prog_data.fs->writes_z_from_fep) {
                job->ez_state = V3D_EZ_DISABLED;
        }

        if (job->first_ez_state == V3D_EZ_UNDECIDED &&
            (job->ez_state != V3D_EZ_DISABLED || job->draw_calls_queued == 0))
                job->first_ez_state = job->ez_state;
}

static bool
v3d_check_compiled_shaders(struct v3d_context *v3d)
{
        static bool warned[5] = { 0 };

        uint32_t failed_stage = MESA_SHADER_NONE;
        if (!v3d->prog.vs->resource || !v3d->prog.cs->resource) {
                failed_stage = MESA_SHADER_VERTEX;
        } else if ((v3d->prog.gs_bin && !v3d->prog.gs_bin->resource) ||
                   (v3d->prog.gs && !v3d->prog.gs->resource)) {
                failed_stage = MESA_SHADER_GEOMETRY;
        } else if (v3d->prog.fs && !v3d->prog.fs->resource) {
                failed_stage = MESA_SHADER_FRAGMENT;
        }

        if (likely(failed_stage == MESA_SHADER_NONE))
                return true;

        if (!warned[failed_stage]) {
                fprintf(stderr,
                        "%s shader failed to compile. Expect corruption.\n",
                        _mesa_shader_stage_to_string(failed_stage));
                warned[failed_stage] = true;
        }
        return false;
}

static void
v3d_draw_vbo(struct pipe_context *pctx, const struct pipe_draw_info *info,
             unsigned drawid_offset,
             const struct pipe_draw_indirect_info *indirect,
             const struct pipe_draw_start_count_bias *draws,
             unsigned num_draws)
{
        if (num_draws > 1) {
                util_draw_multi(pctx, info, drawid_offset, indirect, draws, num_draws);
                return;
        }

        if (!indirect && (!draws[0].count || !info->instance_count))
           return;

        struct v3d_context *v3d = v3d_context(pctx);

        if (!indirect &&
            !info->primitive_restart &&
            !u_trim_pipe_prim(info->mode, (unsigned*)&draws[0].count))
                return;

        if (!v3d_render_condition_check(v3d))
                return;

        /* Fall back for weird desktop GL primitive restart values. */
        if (info->primitive_restart &&
            info->index_size) {
                uint32_t mask = util_prim_restart_index_from_size(info->index_size);
                if (info->restart_index != mask) {
                        util_draw_vbo_without_prim_restart(pctx, info, drawid_offset, indirect, &draws[0]);
                        return;
                }
        }

        /* Before setting up the draw, flush anything writing to the resources
         * that we read from or reading from resources we write to.
         */
        for (int s = 0; s < PIPE_SHADER_COMPUTE; s++)
                v3d_predraw_check_stage_inputs(pctx, s);

        if (indirect && indirect->buffer) {
                v3d_flush_jobs_writing_resource(v3d, indirect->buffer,
                                                V3D_FLUSH_DEFAULT, false);
        }

        v3d_predraw_check_outputs(pctx);

        /* If transform feedback is active and we are switching primitive type
         * we need to submit the job before drawing and update the vertex count
         * written to TF based on the primitive type since we will need to
         * know the exact vertex count if the application decides to call
         * glDrawTransformFeedback() later.
         */
        if (v3d->streamout.num_targets > 0 &&
            u_base_prim_type(info->mode) != u_base_prim_type(v3d->prim_mode)) {
                v3d_update_primitive_counters(v3d);
        }

        struct v3d_job *job = v3d_get_job_for_fbo(v3d);

        /* If vertex texturing depends on the output of rendering, we need to
         * ensure that that rendering is complete before we run a coordinate
         * shader that depends on it.
         *
         * Given that doing that is unusual, for now we just block the binner
         * on the last submitted render, rather than tracking the last
         * rendering to each texture's BO.
         */
        if (v3d->tex[PIPE_SHADER_VERTEX].num_textures || (indirect && indirect->buffer)) {
                static bool warned = false;
                if (!warned) {
                        perf_debug("Blocking binner on last render due to "
                                   "vertex texturing or indirect drawing.\n");
                        warned = true;
                }
                job->submit.in_sync_bcl = v3d->out_sync;
        }

        /* We also need to ensure that compute is complete when render depends
         * on resources written by it.
         */
        if (v3d->sync_on_last_compute_job) {
                job->submit.in_sync_bcl = v3d->out_sync;
                v3d->sync_on_last_compute_job = false;
        }

        /* Mark SSBOs and images as being written.  We don't actually know
         * which ones are read vs written, so just assume the worst.
         */
        for (int s = 0; s < PIPE_SHADER_COMPUTE; s++) {
                u_foreach_bit(i, v3d->ssbo[s].enabled_mask) {
                        v3d_job_add_write_resource(job,
                                                   v3d->ssbo[s].sb[i].buffer);
                        job->tmu_dirty_rcl = true;
                }

                u_foreach_bit(i, v3d->shaderimg[s].enabled_mask) {
                        v3d_job_add_write_resource(job,
                                                   v3d->shaderimg[s].si[i].base.resource);
                        job->tmu_dirty_rcl = true;
                }
        }

        /* Get space to emit our draw call into the BCL, using a branch to
         * jump to a new BO if necessary.
         */
        v3d_cl_ensure_space_with_branch(&job->bcl, 256 /* XXX */);

        if (v3d->prim_mode != info->mode) {
                v3d->prim_mode = info->mode;
                v3d->dirty |= V3D_DIRTY_PRIM_MODE;
        }

        v3d_start_draw(v3d);
        v3d_update_compiled_shaders(v3d, info->mode);
        if (!v3d_check_compiled_shaders(v3d))
                return;
        v3d_update_job_ez(v3d, job);

        /* If this job was writing to transform feedback buffers before this
         * draw and we are reading from them here, then we need to wait for TF
         * to complete before we emit this draw.
         *
         * Notice this check needs to happen before we emit state for the
         * current draw call, where we update job->tf_enabled, so we can ensure
         * that we only check TF writes for prior draws.
         */
        v3d_emit_wait_for_tf_if_needed(v3d, job);

        v3dX(emit_state)(pctx);

        if (v3d->dirty & (V3D_DIRTY_VTXBUF |
                          V3D_DIRTY_VTXSTATE |
                          V3D_DIRTY_PRIM_MODE |
                          V3D_DIRTY_RASTERIZER |
                          V3D_DIRTY_COMPILED_CS |
                          V3D_DIRTY_COMPILED_VS |
                          V3D_DIRTY_COMPILED_GS_BIN |
                          V3D_DIRTY_COMPILED_GS |
                          V3D_DIRTY_COMPILED_FS |
                          v3d->prog.cs->uniform_dirty_bits |
                          v3d->prog.vs->uniform_dirty_bits |
                          (v3d->prog.gs_bin ?
                                    v3d->prog.gs_bin->uniform_dirty_bits : 0) |
                          (v3d->prog.gs ?
                                    v3d->prog.gs->uniform_dirty_bits : 0) |
                          v3d->prog.fs->uniform_dirty_bits)) {
                v3d_emit_gl_shader_state(v3d, info);
        }

        v3d->dirty = 0;

        /* The Base Vertex/Base Instance packet sets those values to nonzero
         * for the next draw call only.
         */
        if ((info->index_size && draws->index_bias) || info->start_instance) {
                cl_emit(&job->bcl, BASE_VERTEX_BASE_INSTANCE, base) {
                        base.base_instance = info->start_instance;
                        base.base_vertex = info->index_size ? draws->index_bias : 0;
                }
        }

        uint32_t prim_tf_enable = 0;

        v3d->prim_restart = info->primitive_restart;

        if (!v3d->prog.gs && !v3d->prim_restart)
                v3d_update_primitives_generated_counter(v3d, info, &draws[0]);

        uint32_t hw_prim_type = v3d_hw_prim_type(info->mode);
        if (info->index_size) {
                uint32_t index_size = info->index_size;
                uint32_t offset = draws[0].start * index_size;
                struct pipe_resource *prsc;
                if (info->has_user_indices) {
                        unsigned start_offset = draws[0].start * info->index_size;
                        prsc = NULL;
                        u_upload_data(v3d->uploader, start_offset,
                                      draws[0].count * info->index_size, 4,
                                      (char*)info->index.user + start_offset,
                                      &offset, &prsc);
                } else {
                        prsc = info->index.resource;
                }
                struct v3d_resource *rsc = v3d_resource(prsc);

                cl_emit(&job->bcl, INDEX_BUFFER_SETUP, ib) {
                        ib.address = cl_address(rsc->bo, 0);
                        ib.size = rsc->bo->size;
                }

                if (indirect && indirect->buffer) {
                        cl_emit(&job->bcl, INDIRECT_INDEXED_INSTANCED_PRIM_LIST, prim) {
                                prim.index_type = ffs(info->index_size) - 1;
                                prim.mode = hw_prim_type | prim_tf_enable;
                                prim.enable_primitive_restarts = info->primitive_restart;

                                prim.number_of_draw_indirect_indexed_records = indirect->draw_count;

                                prim.stride_in_multiples_of_4_bytes = indirect->stride >> 2;
                                prim.address = cl_address(v3d_resource(indirect->buffer)->bo,
                                                          indirect->offset);
                        }
                } else if (info->instance_count > 1) {
                        cl_emit(&job->bcl, INDEXED_INSTANCED_PRIM_LIST, prim) {
                                prim.index_type = ffs(info->index_size) - 1;
                                prim.index_offset = offset;
                                prim.mode = hw_prim_type | prim_tf_enable;
                                prim.enable_primitive_restarts = info->primitive_restart;

                                prim.number_of_instances = info->instance_count;
                                prim.instance_length = draws[0].count;
                        }
                } else {
                        cl_emit(&job->bcl, INDEXED_PRIM_LIST, prim) {
                                prim.index_type = ffs(info->index_size) - 1;
                                prim.length = draws[0].count;
                                prim.index_offset = offset;
                                prim.mode = hw_prim_type | prim_tf_enable;
                                prim.enable_primitive_restarts = info->primitive_restart;
                        }
                }

                if (info->has_user_indices)
                        pipe_resource_reference(&prsc, NULL);
        } else {
                if (indirect && indirect->buffer) {
                        cl_emit(&job->bcl, INDIRECT_VERTEX_ARRAY_INSTANCED_PRIMS, prim) {
                                prim.mode = hw_prim_type | prim_tf_enable;
                                prim.number_of_draw_indirect_array_records = indirect->draw_count;

                                prim.stride_in_multiples_of_4_bytes = indirect->stride >> 2;
                                prim.address = cl_address(v3d_resource(indirect->buffer)->bo,
                                                          indirect->offset);
                        }
                } else if (info->instance_count > 1) {
                        struct pipe_stream_output_target *so =
                                indirect && indirect->count_from_stream_output ?
                                        indirect->count_from_stream_output : NULL;
                        uint32_t vert_count = so ?
                                v3d_stream_output_target_get_vertex_count(so) :
                                draws[0].count;
                        cl_emit(&job->bcl, VERTEX_ARRAY_INSTANCED_PRIMS, prim) {
                                prim.mode = hw_prim_type | prim_tf_enable;
                                prim.index_of_first_vertex = draws[0].start;
                                prim.number_of_instances = info->instance_count;
                                prim.instance_length = vert_count;
                        }
                } else {
                        struct pipe_stream_output_target *so =
                                indirect && indirect->count_from_stream_output ?
                                        indirect->count_from_stream_output : NULL;
                        uint32_t vert_count = so ?
                                v3d_stream_output_target_get_vertex_count(so) :
                                draws[0].count;
                        cl_emit(&job->bcl, VERTEX_ARRAY_PRIMS, prim) {
                                prim.mode = hw_prim_type | prim_tf_enable;
                                prim.length = vert_count;
                                prim.index_of_first_vertex = draws[0].start;
                        }
                }
        }

        /* A flush is required in between a TF draw and any following TF specs
         * packet, or the GPU may hang.  Just flush each time for now.
         */
        if (v3d->streamout.num_targets)
                cl_emit(&job->bcl, TRANSFORM_FEEDBACK_FLUSH_AND_COUNT, flush);

        job->draw_calls_queued++;
        if (v3d->streamout.num_targets)
           job->tf_draw_calls_queued++;

        /* Increment the TF offsets by how many verts we wrote.  XXX: This
         * needs some clamping to the buffer size.
         *
         * If primitive restart is enabled or we have a geometry shader, we
         * update it later, when we can query the device to know how many
         * vertices were written.
         */
        if (!v3d->prog.gs && !v3d->prim_restart) {
                for (int i = 0; i < v3d->streamout.num_targets; i++)
                        v3d_stream_output_target(v3d->streamout.targets[i])->offset +=
                                u_stream_outputs_for_vertices(info->mode, draws[0].count);
        }

        if (v3d->zsa && job->zsbuf && v3d->zsa->base.depth_enabled) {
                struct v3d_resource *rsc = v3d_resource(job->zsbuf->texture);
                v3d_job_add_bo(job, rsc->bo);

                job->load |= PIPE_CLEAR_DEPTH & ~job->clear;
                if (v3d->zsa->base.depth_writemask)
                        job->store |= PIPE_CLEAR_DEPTH;
                rsc->initialized_buffers = PIPE_CLEAR_DEPTH;
        }

        if (v3d->zsa && job->zsbuf && v3d->zsa->base.stencil[0].enabled) {
                struct v3d_resource *rsc = v3d_resource(job->zsbuf->texture);
                if (rsc->separate_stencil)
                        rsc = rsc->separate_stencil;

                v3d_job_add_bo(job, rsc->bo);

                job->load |= PIPE_CLEAR_STENCIL & ~job->clear;
                if (v3d->zsa->base.stencil[0].writemask ||
                    v3d->zsa->base.stencil[1].writemask) {
                        job->store |= PIPE_CLEAR_STENCIL;
                }
                rsc->initialized_buffers |= PIPE_CLEAR_STENCIL;
        }

        for (int i = 0; i < job->nr_cbufs; i++) {
                uint32_t bit = PIPE_CLEAR_COLOR0 << i;
                int blend_rt = v3d->blend->base.independent_blend_enable ? i : 0;

                if (job->store & bit || !job->cbufs[i])
                        continue;
                struct v3d_resource *rsc = v3d_resource(job->cbufs[i]->texture);

                job->load |= bit & ~job->clear;
                if (v3d->blend->base.rt[blend_rt].colormask)
                        job->store |= bit;
                v3d_job_add_bo(job, rsc->bo);
        }

        if (job->referenced_size > 768 * 1024 * 1024) {
                perf_debug("Flushing job with %dkb to try to free up memory\n",
                        job->referenced_size / 1024);
                v3d_flush(pctx);
        }

        if (V3D_DBG(ALWAYS_FLUSH))
                v3d_flush(pctx);
}

static void
v3d_launch_grid(struct pipe_context *pctx, const struct pipe_grid_info *info)
{
        struct v3d_context *v3d = v3d_context(pctx);
        struct v3d_screen *screen = v3d->screen;

        v3d_predraw_check_stage_inputs(pctx, PIPE_SHADER_COMPUTE);

        v3d_update_compiled_cs(v3d);

        if (!v3d->prog.compute->resource) {
                static bool warned = false;
                if (!warned) {
                        fprintf(stderr,
                                "Compute shader failed to compile.  "
                                "Expect corruption.\n");
                        warned = true;
                }
                return;
        }

        /* Some of the units of scale:
         *
         * - Batches of 16 work items (shader invocations) that will be queued
         *   to the run on a QPU at once.
         *
         * - Workgroups composed of work items based on the shader's layout
         *   declaration.
         *
         * - Supergroups of 1-16 workgroups.  There can only be 16 supergroups
         *   running at a time on the core, so we want to keep them large to
         *   keep the QPUs busy, but a whole supergroup will sync at a barrier
         *   so we want to keep them small if one is present.
         */
        struct drm_v3d_submit_csd submit = { 0 };
        struct v3d_job *job = v3d_job_create(v3d);

        /* Set up the actual number of workgroups, synchronously mapping the
         * indirect buffer if necessary to get the dimensions.
         */
        if (info->indirect) {
                struct pipe_transfer *transfer;
                uint32_t *map = pipe_buffer_map_range(pctx, info->indirect,
                                                      info->indirect_offset,
                                                      3 * sizeof(uint32_t),
                                                      PIPE_MAP_READ,
                                                      &transfer);
                memcpy(v3d->compute_num_workgroups, map, 3 * sizeof(uint32_t));
                pipe_buffer_unmap(pctx, transfer);

                if (v3d->compute_num_workgroups[0] == 0 ||
                    v3d->compute_num_workgroups[1] == 0 ||
                    v3d->compute_num_workgroups[2] == 0) {
                        /* Nothing to dispatch, so skip the draw (CSD can't
                         * handle 0 workgroups).
                         */
                        return;
                }
        } else {
                v3d->compute_num_workgroups[0] = info->grid[0];
                v3d->compute_num_workgroups[1] = info->grid[1];
                v3d->compute_num_workgroups[2] = info->grid[2];
        }

        uint32_t num_wgs = 1;
        for (int i = 0; i < 3; i++) {
                num_wgs *= v3d->compute_num_workgroups[i];
                submit.cfg[i] |= (v3d->compute_num_workgroups[i] <<
                                  V3D_CSD_CFG012_WG_COUNT_SHIFT);
        }

        uint32_t wg_size = info->block[0] * info->block[1] * info->block[2];

        struct v3d_compute_prog_data *compute =
                v3d->prog.compute->prog_data.compute;
        uint32_t wgs_per_sg =
                v3d_csd_choose_workgroups_per_supergroup(
                        &v3d->screen->devinfo,
                        compute->has_subgroups,
                        compute->base.has_control_barrier,
                        compute->base.threads,
                        num_wgs, wg_size);

        uint32_t batches_per_sg = DIV_ROUND_UP(wgs_per_sg * wg_size, 16);
        uint32_t whole_sgs = num_wgs / wgs_per_sg;
        uint32_t rem_wgs = num_wgs - whole_sgs * wgs_per_sg;
        uint32_t num_batches = batches_per_sg * whole_sgs +
                               DIV_ROUND_UP(rem_wgs * wg_size, 16);

        submit.cfg[3] |= (wgs_per_sg & 0xf) << V3D_CSD_CFG3_WGS_PER_SG_SHIFT;
        submit.cfg[3] |=
                (batches_per_sg - 1) << V3D_CSD_CFG3_BATCHES_PER_SG_M1_SHIFT;
        submit.cfg[3] |= (wg_size & 0xff) << V3D_CSD_CFG3_WG_SIZE_SHIFT;


        /* Number of batches the dispatch will invoke.
         * V3D 7.1.6 and later don't subtract 1 from the number of batches
         */
        if (v3d->screen->devinfo.ver < 71 ||
            (v3d->screen->devinfo.ver == 71 && v3d->screen->devinfo.rev < 6)) {
                submit.cfg[4] = num_batches - 1;
        } else {
                submit.cfg[4] = num_batches;
        }

        /* Make sure we didn't accidentally underflow. */
        assert(submit.cfg[4] != ~0);

        v3d_job_add_bo(job, v3d_resource(v3d->prog.compute->resource)->bo);
        submit.cfg[5] = (v3d_resource(v3d->prog.compute->resource)->bo->offset +
                         v3d->prog.compute->offset);
        if (v3d->screen->devinfo.ver < 71)
                submit.cfg[5] |= V3D_CSD_CFG5_PROPAGATE_NANS;
        if (v3d->prog.compute->prog_data.base->single_seg)
                submit.cfg[5] |= V3D_CSD_CFG5_SINGLE_SEG;
        if (v3d->prog.compute->prog_data.base->threads == 4)
                submit.cfg[5] |= V3D_CSD_CFG5_THREADING;

        if (v3d->prog.compute->prog_data.compute->shared_size) {
                v3d->compute_shared_memory =
                        v3d_bo_alloc(v3d->screen,
                                     v3d->prog.compute->prog_data.compute->shared_size *
                                     num_wgs,
                                     "shared_vars");
        }

        struct v3d_cl_reloc uniforms = v3d_write_uniforms(v3d, job,
                                                          v3d->prog.compute,
                                                          PIPE_SHADER_COMPUTE);
        v3d_job_add_bo(job, uniforms.bo);
        submit.cfg[6] = uniforms.bo->offset + uniforms.offset;

        /* Pull some job state that was stored in a SUBMIT_CL struct out to
         * our SUBMIT_CSD struct
         */
        submit.bo_handles = job->submit.bo_handles;
        submit.bo_handle_count = job->submit.bo_handle_count;

        /* Serialize this in the rest of our command stream. */
        submit.in_sync = v3d->out_sync;
        submit.out_sync = v3d->out_sync;

        if (v3d->active_perfmon) {
                assert(screen->has_perfmon);
                submit.perfmon_id = v3d->active_perfmon->kperfmon_id;
        }

        v3d->last_perfmon = v3d->active_perfmon;

        if (!V3D_DBG(NORAST)) {
                int ret = v3d_ioctl(screen->fd, DRM_IOCTL_V3D_SUBMIT_CSD,
                                    &submit);
                static bool warned = false;
                if (ret && !warned) {
                        fprintf(stderr, "CSD submit call returned %s.  "
                                "Expect corruption.\n", strerror(errno));
                        warned = true;
                } else if (!ret) {
                        if (v3d->active_perfmon)
                                v3d->active_perfmon->job_submitted = true;
                }
        }

        v3d_job_free(v3d, job);

        /* Mark SSBOs as being written.. we don't actually know which ones are
         * read vs written, so just assume the worst
         */
        u_foreach_bit(i, v3d->ssbo[PIPE_SHADER_COMPUTE].enabled_mask) {
                struct v3d_resource *rsc = v3d_resource(
                        v3d->ssbo[PIPE_SHADER_COMPUTE].sb[i].buffer);
                rsc->writes++;
                rsc->compute_written = true;
        }

        u_foreach_bit(i, v3d->shaderimg[PIPE_SHADER_COMPUTE].enabled_mask) {
                struct v3d_resource *rsc = v3d_resource(
                        v3d->shaderimg[PIPE_SHADER_COMPUTE].si[i].base.resource);
                rsc->writes++;
                rsc->compute_written = true;
        }

        v3d_bo_unreference(&uniforms.bo);
        v3d_bo_unreference(&v3d->compute_shared_memory);
}

/**
 * Implements gallium's clear() hook (glClear()) by drawing a pair of triangles.
 */
static void
v3d_draw_clear(struct v3d_context *v3d,
               unsigned buffers,
               const union pipe_color_union *color,
               double depth, unsigned stencil)
{
        v3d_blitter_save(v3d, false, true);
        util_blitter_clear(v3d->blitter,
                           v3d->framebuffer.width,
                           v3d->framebuffer.height,
                           util_framebuffer_get_num_layers(&v3d->framebuffer),
                           buffers, color, depth, stencil,
                           util_framebuffer_get_num_samples(&v3d->framebuffer) > 1);
}

/**
 * Attempts to perform the GL clear by using the TLB's fast clear at the start
 * of the frame.
 */
static unsigned
v3d_tlb_clear(struct v3d_job *job, unsigned buffers,
              const union pipe_color_union *color,
              double depth, unsigned stencil)
{
        struct v3d_context *v3d = job->v3d;

        if (job->draw_calls_queued) {
                /* If anything in the CL has drawn using the buffer, then the
                 * TLB clear we're trying to add now would happen before that
                 * drawing.
                 */
                buffers &= ~(job->load | job->store);
        }

        /* GFXH-1461: If we were to emit a load of just depth or just stencil,
         * then the clear for the other may get lost.  We need to decide now
         * if it would be possible to need to emit a load of just one after
         * we've set up our TLB clears. This issue is fixed since V3D 4.3.18.
         */
        if (v3d->screen->devinfo.ver == 42 &&
            buffers & PIPE_CLEAR_DEPTHSTENCIL &&
            (buffers & PIPE_CLEAR_DEPTHSTENCIL) != PIPE_CLEAR_DEPTHSTENCIL &&
            job->zsbuf &&
            util_format_is_depth_and_stencil(job->zsbuf->texture->format)) {
                buffers &= ~PIPE_CLEAR_DEPTHSTENCIL;
        }

        for (int i = 0; i < job->nr_cbufs; i++) {
                uint32_t bit = PIPE_CLEAR_COLOR0 << i;
                if (!(buffers & bit))
                        continue;

                struct pipe_surface *psurf = v3d->framebuffer.cbufs[i];
                struct v3d_surface *surf = v3d_surface(psurf);
                struct v3d_resource *rsc = v3d_resource(psurf->texture);

                union util_color uc;
                uint32_t internal_size = 4 << surf->internal_bpp;

                /*  While hardware supports clamping, this is not applied on
                 *  the clear values, so we need to do it manually.
                 *
                 *  "Clamping is performed on color values immediately as they
                 *   enter the TLB and after blending. Clamping is not
                 *   performed on the clear color."
                 */
                union pipe_color_union clamped_color =
                        util_clamp_color(psurf->format, color);

                if (v3d->swap_color_rb & (1 << i)) {
                        union pipe_color_union orig_color = clamped_color;
                        clamped_color.f[0] = orig_color.f[2];
                        clamped_color.f[1] = orig_color.f[1];
                        clamped_color.f[2] = orig_color.f[0];
                        clamped_color.f[3] = orig_color.f[3];
                }

                if (util_format_is_alpha(psurf->format))
                        clamped_color.f[0] = clamped_color.f[3];

                switch (surf->internal_type) {
                case V3D_INTERNAL_TYPE_8:
                        util_pack_color(clamped_color.f, PIPE_FORMAT_R8G8B8A8_UNORM,
                                        &uc);
                        memcpy(job->clear_color[i], uc.ui, internal_size);
                        break;
                case V3D_INTERNAL_TYPE_8I:
                case V3D_INTERNAL_TYPE_8UI:
                        job->clear_color[i][0] = ((clamped_color.ui[0] & 0xff) |
                                                  (clamped_color.ui[1] & 0xff) << 8 |
                                                  (clamped_color.ui[2] & 0xff) << 16 |
                                                  (clamped_color.ui[3] & 0xff) << 24);
                        break;
                case V3D_INTERNAL_TYPE_16F:
                        util_pack_color(clamped_color.f, PIPE_FORMAT_R16G16B16A16_FLOAT,
                                        &uc);
                        memcpy(job->clear_color[i], uc.ui, internal_size);
                        break;
                case V3D_INTERNAL_TYPE_16I:
                case V3D_INTERNAL_TYPE_16UI:
                        job->clear_color[i][0] = ((clamped_color.ui[0] & 0xffff) |
                                                  clamped_color.ui[1] << 16);
                        job->clear_color[i][1] = ((clamped_color.ui[2] & 0xffff) |
                                                  clamped_color.ui[3] << 16);
                        break;
                case V3D_INTERNAL_TYPE_32F:
                case V3D_INTERNAL_TYPE_32I:
                case V3D_INTERNAL_TYPE_32UI:
                        memcpy(job->clear_color[i], clamped_color.ui, internal_size);
                        break;
                }

                rsc->initialized_buffers |= bit;
        }

        unsigned zsclear = buffers & PIPE_CLEAR_DEPTHSTENCIL;
        if (zsclear) {
                struct v3d_resource *rsc =
                        v3d_resource(v3d->framebuffer.zsbuf->texture);

                if (zsclear & PIPE_CLEAR_DEPTH)
                        job->clear_z = depth;
                if (zsclear & PIPE_CLEAR_STENCIL)
                        job->clear_s = stencil;

                rsc->initialized_buffers |= zsclear;
        }

        job->draw_min_x = 0;
        job->draw_min_y = 0;
        job->draw_max_x = v3d->framebuffer.width;
        job->draw_max_y = v3d->framebuffer.height;
        job->clear |= buffers;
        job->store |= buffers;
        job->scissor.disabled = true;

        v3d_start_draw(v3d);

        return buffers;
}

static void
v3d_clear(struct pipe_context *pctx, unsigned buffers, const struct pipe_scissor_state *scissor_state,
          const union pipe_color_union *color, double depth, unsigned stencil)
{
        struct v3d_context *v3d = v3d_context(pctx);
        struct v3d_job *job = v3d_get_job_for_fbo(v3d);

        buffers &= ~v3d_tlb_clear(job, buffers, color, depth, stencil);

        if (!buffers || !v3d_render_condition_check(v3d))
                return;

        v3d_draw_clear(v3d, buffers, color, depth, stencil);
}

static void
v3d_clear_render_target(struct pipe_context *pctx, struct pipe_surface *ps,
                        const union pipe_color_union *color,
                        unsigned x, unsigned y, unsigned w, unsigned h,
                        bool render_condition_enabled)
{
        struct v3d_context *v3d = v3d_context(pctx);

        if (render_condition_enabled && !v3d_render_condition_check(v3d))
                return;

        v3d_blitter_save(v3d, false, render_condition_enabled);
        util_blitter_clear_render_target(v3d->blitter, ps, color, x, y, w, h);
}

static void
v3d_clear_depth_stencil(struct pipe_context *pctx, struct pipe_surface *ps,
                        unsigned buffers, double depth, unsigned stencil,
                        unsigned x, unsigned y, unsigned w, unsigned h,
                        bool render_condition_enabled)
{
        struct v3d_context *v3d = v3d_context(pctx);

        if (render_condition_enabled && !v3d_render_condition_check(v3d))
                return;

        v3d_blitter_save(v3d, false, render_condition_enabled);
        util_blitter_clear_depth_stencil(v3d->blitter, ps, buffers, depth,
                                         stencil, x, y, w, h);
}

void
v3dX(draw_init)(struct pipe_context *pctx)
{
        pctx->draw_vbo = v3d_draw_vbo;
        pctx->clear = v3d_clear;
        pctx->clear_render_target = v3d_clear_render_target;
        pctx->clear_depth_stencil = v3d_clear_depth_stencil;
        if (v3d_context(pctx)->screen->has_csd)
                pctx->launch_grid = v3d_launch_grid;
}
