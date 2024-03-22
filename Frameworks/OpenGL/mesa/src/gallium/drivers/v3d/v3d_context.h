/*
 * Copyright © 2014-2017 Broadcom
 * Copyright (C) 2012 Rob Clark <robclark@freedesktop.org>
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

#ifndef V3D_CONTEXT_H
#define V3D_CONTEXT_H

#ifdef V3D_VERSION
#include "broadcom/common/v3d_macros.h"
#endif

#include <stdio.h>

#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include "util/bitset.h"
#include "util/slab.h"
#include "xf86drm.h"
#include "drm-uapi/v3d_drm.h"
#include "v3d_screen.h"
#include "broadcom/common/v3d_limits.h"

#include "broadcom/simulator/v3d_simulator.h"
#include "broadcom/compiler/v3d_compiler.h"

struct v3d_job;
struct v3d_bo;
void v3d_job_add_bo(struct v3d_job *job, struct v3d_bo *bo);

#include "v3d_bufmgr.h"
#include "v3d_resource.h"
#include "v3d_cl.h"

#ifdef USE_V3D_SIMULATOR
#define using_v3d_simulator true
#else
#define using_v3d_simulator false
#endif

#define V3D_DIRTY_BLEND               (1ull <<  0)
#define V3D_DIRTY_RASTERIZER          (1ull <<  1)
#define V3D_DIRTY_ZSA                 (1ull <<  2)
#define V3D_DIRTY_COMPTEX             (1ull <<  3)
#define V3D_DIRTY_VERTTEX             (1ull <<  4)
#define V3D_DIRTY_GEOMTEX             (1ull <<  5)
#define V3D_DIRTY_FRAGTEX             (1ull <<  6)

#define V3D_DIRTY_SHADER_IMAGE        (1ull <<  9)
#define V3D_DIRTY_BLEND_COLOR         (1ull << 10)
#define V3D_DIRTY_STENCIL_REF         (1ull << 11)
#define V3D_DIRTY_SAMPLE_STATE        (1ull << 12)
#define V3D_DIRTY_FRAMEBUFFER         (1ull << 13)
#define V3D_DIRTY_STIPPLE             (1ull << 14)
#define V3D_DIRTY_VIEWPORT            (1ull << 15)
#define V3D_DIRTY_CONSTBUF            (1ull << 16)
#define V3D_DIRTY_VTXSTATE            (1ull << 17)
#define V3D_DIRTY_VTXBUF              (1ull << 18)
#define V3D_DIRTY_SCISSOR             (1ull << 19)
#define V3D_DIRTY_FLAT_SHADE_FLAGS    (1ull << 20)
#define V3D_DIRTY_PRIM_MODE           (1ull << 21)
#define V3D_DIRTY_CLIP                (1ull << 22)
#define V3D_DIRTY_UNCOMPILED_CS       (1ull << 23)
#define V3D_DIRTY_UNCOMPILED_VS       (1ull << 24)
#define V3D_DIRTY_UNCOMPILED_GS       (1ull << 25)
#define V3D_DIRTY_UNCOMPILED_FS       (1ull << 26)

#define V3D_DIRTY_COMPILED_CS         (1ull << 29)
#define V3D_DIRTY_COMPILED_VS         (1ull << 30)
#define V3D_DIRTY_COMPILED_GS_BIN     (1ULL << 31)
#define V3D_DIRTY_COMPILED_GS         (1ULL << 32)
#define V3D_DIRTY_COMPILED_FS         (1ull << 33)

#define V3D_DIRTY_FS_INPUTS           (1ull << 38)
#define V3D_DIRTY_GS_INPUTS           (1ull << 39)
#define V3D_DIRTY_STREAMOUT           (1ull << 40)
#define V3D_DIRTY_OQ                  (1ull << 41)
#define V3D_DIRTY_CENTROID_FLAGS      (1ull << 42)
#define V3D_DIRTY_NOPERSPECTIVE_FLAGS (1ull << 43)
#define V3D_DIRTY_SSBO                (1ull << 44)

#define V3D_MAX_FS_INPUTS 64

#define MAX_JOB_SCISSORS 16

enum v3d_sampler_state_variant {
        V3D_SAMPLER_STATE_BORDER_0000,
        V3D_SAMPLER_STATE_BORDER_0001,
        V3D_SAMPLER_STATE_BORDER_1111,
        V3D_SAMPLER_STATE_F16,
        V3D_SAMPLER_STATE_F16_UNORM,
        V3D_SAMPLER_STATE_F16_SNORM,
        V3D_SAMPLER_STATE_F16_BGRA,
        V3D_SAMPLER_STATE_F16_BGRA_UNORM,
        V3D_SAMPLER_STATE_F16_BGRA_SNORM,
        V3D_SAMPLER_STATE_F16_A,
        V3D_SAMPLER_STATE_F16_A_SNORM,
        V3D_SAMPLER_STATE_F16_A_UNORM,
        V3D_SAMPLER_STATE_F16_LA,
        V3D_SAMPLER_STATE_F16_LA_UNORM,
        V3D_SAMPLER_STATE_F16_LA_SNORM,
        V3D_SAMPLER_STATE_32,
        V3D_SAMPLER_STATE_32_UNORM,
        V3D_SAMPLER_STATE_32_SNORM,
        V3D_SAMPLER_STATE_32_A,
        V3D_SAMPLER_STATE_32_A_UNORM,
        V3D_SAMPLER_STATE_32_A_SNORM,
        V3D_SAMPLER_STATE_1010102U,
        V3D_SAMPLER_STATE_16U,
        V3D_SAMPLER_STATE_16I,
        V3D_SAMPLER_STATE_8I,
        V3D_SAMPLER_STATE_8U,

        V3D_SAMPLER_STATE_VARIANT_COUNT,
};

enum v3d_flush_cond {
        /* Flush job unless we are flushing for transform feedback, where we
         * handle flushing in the driver via the 'Wait for TF' packet.
         */
        V3D_FLUSH_DEFAULT,
        /* Always flush the job, even for cases where we would normally not
         * do it, such as transform feedback.
         */
        V3D_FLUSH_ALWAYS,
        /* Flush job if it is not the current FBO job. This is intended to
         * skip automatic flushes of the current job for resources that we
         * expect to be externally synchronized by the application using
         * glMemoryBarrier(), such as SSBOs and shader images.
         */
        V3D_FLUSH_NOT_CURRENT_JOB,
};

struct v3d_sampler_view {
        struct pipe_sampler_view base;
        uint32_t p0;
        uint32_t p1;
        /* Precomputed swizzles to pass in to the shader key. */
        uint8_t swizzle[4];

        uint8_t texture_shader_state[32];
        /* V3D 4.x: Texture state struct. */
        struct v3d_bo *bo;

        enum v3d_sampler_state_variant sampler_variant;

        /* Actual texture to be read by this sampler view.  May be different
         * from base.texture in the case of having a shadow tiled copy of a
         * raster texture.
         */
        struct pipe_resource *texture;

        /* A serial ID used to identify cases where a new BO has been created
         * and we need to rebind a sampler view that was created against the
         * previous BO to to point to the new one.
         */
        uint32_t serial_id;
};

struct v3d_sampler_state {
        struct pipe_sampler_state base;
        uint32_t p0;
        uint32_t p1;

        /* V3D 3.x: Packed texture state. */
        uint8_t texture_shader_state[32];
        /* V3D 4.x: Sampler state struct. */
        struct pipe_resource *sampler_state;
        uint32_t sampler_state_offset[V3D_SAMPLER_STATE_VARIANT_COUNT];

        bool border_color_variants;
};

struct v3d_texture_stateobj {
        struct pipe_sampler_view *textures[V3D_MAX_TEXTURE_SAMPLERS];
        unsigned num_textures;
        struct pipe_sampler_state *samplers[V3D_MAX_TEXTURE_SAMPLERS];
        unsigned num_samplers;
        struct v3d_cl_reloc texture_state[V3D_MAX_TEXTURE_SAMPLERS];
};

struct v3d_shader_uniform_info {
        enum quniform_contents *contents;
        uint32_t *data;
        uint32_t count;
};

struct v3d_uncompiled_shader {
        /** A name for this program, so you can track it in shader-db output. */
        uint32_t program_id;
        /** How many variants of this program were compiled, for shader-db. */
        uint32_t compiled_variant_count;
        struct pipe_shader_state base;
        uint32_t num_tf_outputs;
        struct v3d_varying_slot *tf_outputs;
        uint16_t tf_specs[16];
        uint16_t tf_specs_psiz[16];
        uint32_t num_tf_specs;

        /* For caching */
        unsigned char sha1[20];
};

struct v3d_compiled_shader {
        struct pipe_resource *resource;
        uint32_t offset;

        union {
                struct v3d_prog_data *base;
                struct v3d_vs_prog_data *vs;
                struct v3d_gs_prog_data *gs;
                struct v3d_fs_prog_data *fs;
                struct v3d_compute_prog_data *compute;
        } prog_data;

        /**
         * V3D_DIRTY_* flags that, when set in v3d->dirty, mean that the
         * uniforms have to be rewritten (and therefore the shader state
         * reemitted).
         */
        uint64_t uniform_dirty_bits;
};

struct v3d_program_stateobj {
        struct v3d_uncompiled_shader *bind_vs, *bind_gs, *bind_fs, *bind_compute;
        struct v3d_compiled_shader *cs, *vs, *gs_bin, *gs, *fs, *compute;

        struct hash_table *cache[MESA_SHADER_STAGES];

        struct v3d_bo *spill_bo;
        int spill_size_per_thread;
};

struct v3d_constbuf_stateobj {
        struct pipe_constant_buffer cb[PIPE_MAX_CONSTANT_BUFFERS];
        uint32_t enabled_mask;
        uint32_t dirty_mask;
};

struct v3d_vertexbuf_stateobj {
        struct pipe_vertex_buffer vb[PIPE_MAX_ATTRIBS];
        unsigned count;
        uint32_t enabled_mask;
        uint32_t dirty_mask;
};

struct v3d_vertex_stateobj {
        struct pipe_vertex_element pipe[V3D_MAX_VS_INPUTS / 4];
        unsigned num_elements;

        uint8_t attrs[16 * (V3D_MAX_VS_INPUTS / 4)];
        /* defaults can be NULL for some hw generation */
        struct pipe_resource *defaults;
        uint32_t defaults_offset;
};

struct v3d_stream_output_target {
        struct pipe_stream_output_target base;
        /* Number of transform feedback vertices written to this target */
        uint32_t recorded_vertex_count;
        /* Number of vertices we've written into the buffer so far */
        uint32_t offset;
};

struct v3d_streamout_stateobj {
        struct pipe_stream_output_target *targets[PIPE_MAX_SO_BUFFERS];
        unsigned num_targets;
};

struct v3d_ssbo_stateobj {
        struct pipe_shader_buffer sb[PIPE_MAX_SHADER_BUFFERS];
        uint32_t enabled_mask;
};

/* Hash table key for v3d->jobs */
struct v3d_job_key {
        struct pipe_surface *cbufs[V3D_MAX_DRAW_BUFFERS];
        struct pipe_surface *zsbuf;
        struct pipe_surface *bbuf;
};

enum v3d_ez_state {
        V3D_EZ_UNDECIDED = 0,
        V3D_EZ_GT_GE,
        V3D_EZ_LT_LE,
        V3D_EZ_DISABLED,
};

struct v3d_image_view {
        struct pipe_image_view base;
        /* V3D 4.x texture shader state struct */
        struct pipe_resource *tex_state;
        uint32_t tex_state_offset;
};

struct v3d_shaderimg_stateobj {
        struct v3d_image_view si[PIPE_MAX_SHADER_IMAGES];
        uint32_t enabled_mask;
};

struct v3d_perfmon_state {
        /* The kernel perfmon id */
        uint32_t kperfmon_id;
        /* True if at least one job was submitted with this perfmon. */
        bool job_submitted;
        /* Fence to be signaled when the last job submitted with this perfmon
         * is executed by the GPU.
         */
        struct v3d_fence *last_job_fence;
        uint8_t counters[DRM_V3D_MAX_PERF_COUNTERS];
        uint64_t values[DRM_V3D_MAX_PERF_COUNTERS];
};

/**
 * A complete bin/render job.
 *
 * This is all of the state necessary to submit a bin/render to the kernel.
 * We want to be able to have multiple in progress at a time, so that we don't
 * need to flush an existing CL just to switch to rendering to a new render
 * target (which would mean reading back from the old render target when
 * starting to render to it again).
 */
struct v3d_job {
        struct v3d_context *v3d;
        struct v3d_cl bcl;
        struct v3d_cl rcl;
        struct v3d_cl indirect;
        struct v3d_bo *tile_alloc;
        struct v3d_bo *tile_state;

        struct drm_v3d_submit_cl submit;

        /**
         * Set of all BOs referenced by the job.  This will be used for making
         * the list of BOs that the kernel will need to have paged in to
         * execute our job.
         */
        struct set *bos;

        /** Sum of the sizes of the BOs referenced by the job. */
        uint32_t referenced_size;

        struct set *write_prscs;
        struct set *tf_write_prscs;

        /* Size of the submit.bo_handles array. */
        uint32_t bo_handles_size;

        /** @{
         * Surfaces to submit rendering for.
         * For blit operations, bbuf is the source surface, and cbufs[0] is
         * the destination surface.
         */
        uint32_t nr_cbufs;
        struct pipe_surface *cbufs[V3D_MAX_DRAW_BUFFERS];
        struct pipe_surface *zsbuf;
        struct pipe_surface *bbuf;
        /** @} */
        /** @{
         * Bounding box of the scissor across all queued drawing.
         *
         * Note that the max values are exclusive.
         */
        uint32_t draw_min_x;
        uint32_t draw_min_y;
        uint32_t draw_max_x;
        uint32_t draw_max_y;

        /** @} */
        /** @{
         * List of scissor rects used for all queued drawing. All scissor
         * rects will be contained in the draw_{min/max}_{x/y} bounding box.
         *
         * This is used as an optimization when all drawing is scissored to
         * limit tile flushing only to tiles that intersect a scissor rect.
         * If scissor is used together with non-scissored drawing, then
         * the optimization is disabled.
         */
        struct {
                bool disabled;
                uint32_t count;
                struct {
                        uint32_t min_x, min_y;
                        uint32_t max_x, max_y;
                } rects[MAX_JOB_SCISSORS];
        } scissor;

        /** @} */
        /** @{
         * Width/height of the color framebuffer being rendered to,
         * for V3D_TILE_RENDERING_MODE_CONFIG.
         */
        uint32_t draw_width;
        uint32_t draw_height;
        uint32_t num_layers;

        /** @} */
        /** @{ Tile information, depending on MSAA and float color buffer. */
        uint32_t draw_tiles_x; /** @< Number of tiles wide for framebuffer. */
        uint32_t draw_tiles_y; /** @< Number of tiles high for framebuffer. */

        uint32_t tile_width; /** @< Width of a tile. */
        uint32_t tile_height; /** @< Height of a tile. */
        /** maximum internal_bpp of all color render targets. */
        uint32_t internal_bpp;

        /** Whether the current rendering is in a 4X MSAA tile buffer. */
        bool msaa;
        /** @} */

        /* Bitmask of PIPE_CLEAR_* of buffers that were cleared before the
         * first rendering.
         */
        uint32_t clear;
        /* Bitmask of PIPE_CLEAR_* of buffers that have been read by a draw
         * call without having been cleared first.
         */
        uint32_t load;
        /* Bitmask of PIPE_CLEAR_* of buffers that have been rendered to
         * (either clears or draws) and should be stored.
         */
        uint32_t store;
        uint32_t clear_color[V3D_MAX_DRAW_BUFFERS][4];
        float clear_z;
        uint8_t clear_s;

        /* If TLB double-buffering is enabled for this job */
        bool double_buffer;

        /**
         * Set if some drawing (triangles, blits, or just a glClear()) has
         * been done to the FBO, meaning that we need to
         * DRM_IOCTL_V3D_SUBMIT_CL.
         */
        bool needs_flush;

        /* Set if any shader has dirtied cachelines in the TMU that need to be
         * flushed before job end.
         */
        bool tmu_dirty_rcl;

        /**
         * Set if a packet enabling TF has been emitted in the job (V3D 4.x).
         */
        bool tf_enabled;

        bool needs_primitives_generated;

        /**
         * Current EZ state for drawing. Updated at the start of draw after
         * we've decided on the shader being rendered.
         */
        enum v3d_ez_state ez_state;
        /**
         * The first EZ state that was used for drawing with a decided EZ
         * direction (so either UNDECIDED, GT, or LT).
         */
        enum v3d_ez_state first_ez_state;

        /**
         * If we have already decided if we need to disable early Z/S
         * completely for this job.
         */
        bool decided_global_ez_enable;

        /**
         * If this job has been configured to use early Z/S clear.
         */
        bool early_zs_clear;

        /**
         * Number of draw calls (not counting full buffer clears) queued in
         * the current job.
         */
        uint32_t draw_calls_queued;

        /**
         * Number of draw calls (not counting full buffer clears) queued in
         * the current job during active transform feedback.
         */
        uint32_t tf_draw_calls_queued;

        struct v3d_job_key key;
};

struct v3d_context {
        struct pipe_context base;

        int fd;
        struct v3d_screen *screen;

        /** The 3D rendering job for the currently bound FBO. */
        struct v3d_job *job;

        /* Map from struct v3d_job_key to the job for that FBO.
         */
        struct hash_table *jobs;

        /**
         * Map from v3d_resource to a job writing to that resource.
         *
         * Primarily for flushing jobs rendering to textures that are now
         * being read from.
         */
        struct hash_table *write_jobs;

        struct slab_child_pool transfer_pool;
        struct blitter_context *blitter;

        /** bitfield of V3D_DIRTY_* */
        uint64_t dirty;

        uint32_t next_uncompiled_program_id;
        uint64_t next_compiled_program_id;

        struct v3d_compiler_state *compiler_state;

        uint8_t prim_mode;

        /** Maximum index buffer valid for the current shader_rec. */
        uint32_t max_index;

        /** Sync object that our RCL or TFU job will update as its out_sync. */
        uint32_t out_sync;

        /* Stream uploader used by gallium internals.  This could also be used
         * by driver internals, but we tend to use the v3d_cl.h interfaces
         * instead.
         */
        struct u_upload_mgr *uploader;
        /* State uploader used inside the driver.  This is for packing bits of
         * long-term state inside buffers, since the kernel interfaces
         * allocate a page at a time.
         */
        struct u_upload_mgr *state_uploader;

        struct pipe_shader_state *sand8_blit_vs;
        struct pipe_shader_state *sand8_blit_fs_luma;
        struct pipe_shader_state *sand8_blit_fs_chroma;
        struct pipe_shader_state *sand30_blit_vs;
        struct pipe_shader_state *sand30_blit_fs;

        /** @{ Current pipeline state objects */
        struct pipe_scissor_state scissor;
        struct v3d_blend_state *blend;
        struct v3d_rasterizer_state *rasterizer;
        struct v3d_depth_stencil_alpha_state *zsa;

        struct v3d_program_stateobj prog;
        uint32_t compute_num_workgroups[3];
        struct v3d_bo *compute_shared_memory;

        struct v3d_vertex_stateobj *vtx;

        struct {
                struct pipe_blend_color f;
                uint16_t hf[4];
        } blend_color;
        struct pipe_stencil_ref stencil_ref;
        unsigned sample_mask;
        struct pipe_framebuffer_state framebuffer;

        /* Per render target, whether we should swap the R and B fields in the
         * shader's color output and in blending.  If render targets disagree
         * on the R/B swap and use the constant color, then we would need to
         * fall back to in-shader blending.
         */
        uint8_t swap_color_rb;

        /* Per render target, whether we should treat the dst alpha values as
         * one in blending.
         *
         * For RGBX formats, the tile buffer's alpha channel will be
         * undefined.
         */
        uint8_t blend_dst_alpha_one;

        bool active_queries;

        /**
         * If a compute job writes a resource read by a non-compute stage we
         * should sync on the last compute job.
         */
        bool sync_on_last_compute_job;

        uint32_t tf_prims_generated;
        uint32_t prims_generated;
        bool prim_restart;

        uint32_t n_primitives_generated_queries_in_flight;

        struct pipe_poly_stipple stipple;
        struct pipe_clip_state clip;
        struct pipe_viewport_state viewport;
        struct v3d_ssbo_stateobj ssbo[PIPE_SHADER_TYPES];
        struct v3d_shaderimg_stateobj shaderimg[PIPE_SHADER_TYPES];
        struct v3d_constbuf_stateobj constbuf[PIPE_SHADER_TYPES];
        struct v3d_texture_stateobj tex[PIPE_SHADER_TYPES];
        struct v3d_vertexbuf_stateobj vertexbuf;
        struct v3d_streamout_stateobj streamout;
        struct v3d_bo *current_oq;
        struct pipe_resource *prim_counts;
        uint32_t prim_counts_offset;
        struct v3d_perfmon_state *active_perfmon;
        struct v3d_perfmon_state *last_perfmon;

        struct pipe_query *cond_query;
        bool cond_cond;
        enum pipe_render_cond_flag cond_mode;

        int in_fence_fd;
        /** Handle of the syncobj that holds in_fence_fd for submission. */
        uint32_t in_syncobj;
        /** @} */
};

struct v3d_rasterizer_state {
        struct pipe_rasterizer_state base;

        float point_size;

        uint8_t depth_offset[9];
        uint8_t depth_offset_z16[9];
};

struct v3d_depth_stencil_alpha_state {
        struct pipe_depth_stencil_alpha_state base;

        enum v3d_ez_state ez_state;

        uint8_t stencil_front[6];
        uint8_t stencil_back[6];
};

struct v3d_blend_state {
        struct pipe_blend_state base;

        /* Per-RT mask of whether blending is enabled. */
        uint8_t blend_enables;
};

#define perf_debug(...) do {                            \
        if (V3D_DBG(PERF))                            \
                fprintf(stderr, __VA_ARGS__);           \
        if (unlikely(v3d->base.debug.debug_message))         \
                util_debug_message(&v3d->base.debug, PERF_INFO, __VA_ARGS__); \
} while (0)

static inline struct v3d_context *
v3d_context(struct pipe_context *pcontext)
{
        return (struct v3d_context *)pcontext;
}

static inline struct v3d_sampler_view *
v3d_sampler_view(struct pipe_sampler_view *psview)
{
        return (struct v3d_sampler_view *)psview;
}

static inline struct v3d_sampler_state *
v3d_sampler_state(struct pipe_sampler_state *psampler)
{
        return (struct v3d_sampler_state *)psampler;
}

static inline struct v3d_stream_output_target *
v3d_stream_output_target(struct pipe_stream_output_target *ptarget)
{
        return (struct v3d_stream_output_target *)ptarget;
}

static inline uint32_t
v3d_stream_output_target_get_vertex_count(struct pipe_stream_output_target *ptarget)
{
    return v3d_stream_output_target(ptarget)->recorded_vertex_count;
}

int v3d_get_driver_query_group_info(struct pipe_screen *pscreen,
                                    unsigned index,
                                    struct pipe_driver_query_group_info *info);
int v3d_get_driver_query_info(struct pipe_screen *pscreen, unsigned index,
                              struct pipe_driver_query_info *info);

struct pipe_context *v3d_context_create(struct pipe_screen *pscreen,
                                        void *priv, unsigned flags);
void v3d_program_init(struct pipe_context *pctx);
void v3d_program_fini(struct pipe_context *pctx);
void v3d_query_init(struct pipe_context *pctx);

static inline int
v3d_ioctl(int fd, unsigned long request, void *arg)
{
        if (using_v3d_simulator)
                return v3d_simulator_ioctl(fd, request, arg);
        else
                return drmIoctl(fd, request, arg);
}

static inline bool
v3d_transform_feedback_enabled(struct v3d_context *v3d)
{
        return (v3d->prog.bind_vs->num_tf_specs != 0 ||
                (v3d->prog.bind_gs && v3d->prog.bind_gs->num_tf_specs != 0)) &&
               v3d->active_queries;
}

void v3d_set_shader_uniform_dirty_flags(struct v3d_compiled_shader *shader);
struct v3d_cl_reloc v3d_write_uniforms(struct v3d_context *v3d,
                                       struct v3d_job *job,
                                       struct v3d_compiled_shader *shader,
                                       enum pipe_shader_type stage);

void v3d_flush(struct pipe_context *pctx);
void v3d_job_init(struct v3d_context *v3d);
struct v3d_job *v3d_job_create(struct v3d_context *v3d);
void v3d_job_free(struct v3d_context *v3d, struct v3d_job *job);
struct v3d_job *v3d_get_job(struct v3d_context *v3d,
                            uint32_t nr_cbufs,
                            struct pipe_surface **cbufs,
                            struct pipe_surface *zsbuf,
                            struct pipe_surface *bbuf);
struct v3d_job *v3d_get_job_for_fbo(struct v3d_context *v3d);
void v3d_job_add_bo(struct v3d_job *job, struct v3d_bo *bo);
void v3d_job_add_write_resource(struct v3d_job *job, struct pipe_resource *prsc);
void v3d_job_add_tf_write_resource(struct v3d_job *job, struct pipe_resource *prsc);
void v3d_job_submit(struct v3d_context *v3d, struct v3d_job *job);
void v3d_flush_jobs_using_bo(struct v3d_context *v3d, struct v3d_bo *bo);
void v3d_flush_jobs_writing_resource(struct v3d_context *v3d,
                                     struct pipe_resource *prsc,
                                     enum v3d_flush_cond flush_cond,
                                     bool is_compute_pipeline);
void v3d_flush_jobs_reading_resource(struct v3d_context *v3d,
                                     struct pipe_resource *prsc,
                                     enum v3d_flush_cond flush_cond,
                                     bool is_compute_pipeline);
void v3d_update_compiled_shaders(struct v3d_context *v3d, uint8_t prim_mode);
void v3d_update_compiled_cs(struct v3d_context *v3d);

bool v3d_rt_format_supported(const struct v3d_device_info *devinfo,
                             enum pipe_format f);
bool v3d_tex_format_supported(const struct v3d_device_info *devinfo,
                              enum pipe_format f);
uint8_t v3d_get_rt_format(const struct v3d_device_info *devinfo, enum pipe_format f);
uint8_t v3d_get_tex_format(const struct v3d_device_info *devinfo, enum pipe_format f);
uint8_t v3d_get_tex_return_size(const struct v3d_device_info *devinfo,
                                enum pipe_format f);
uint8_t v3d_get_tex_return_channels(const struct v3d_device_info *devinfo,
                                    enum pipe_format f);
const uint8_t *v3d_get_format_swizzle(const struct v3d_device_info *devinfo,
                                      enum pipe_format f);
bool v3d_format_supports_tlb_msaa_resolve(const struct v3d_device_info *devinfo,
                                          enum pipe_format f);

void v3d_init_query_functions(struct v3d_context *v3d);
void v3d_blit(struct pipe_context *pctx, const struct pipe_blit_info *blit_info);
void v3d_blitter_save(struct v3d_context *v3d, bool op_blit,  bool render_cond);
bool v3d_generate_mipmap(struct pipe_context *pctx,
                         struct pipe_resource *prsc,
                         enum pipe_format format,
                         unsigned int base_level,
                         unsigned int last_level,
                         unsigned int first_layer,
                         unsigned int last_layer);

void
v3d_fence_unreference(struct v3d_fence **fence);

struct v3d_fence *v3d_fence_create(struct v3d_context *v3d, int fd);

bool v3d_fence_wait(struct v3d_screen *screen,
                    struct v3d_fence *fence,
                    uint64_t timeout_ns);

int v3d_fence_context_init(struct v3d_context *v3d);
void v3d_fence_context_finish(struct v3d_context *v3d);

void v3d_update_primitive_counters(struct v3d_context *v3d);

bool v3d_line_smoothing_enabled(struct v3d_context *v3d);

float v3d_get_real_line_width(struct v3d_context *v3d);

void v3d_ensure_prim_counts_allocated(struct v3d_context *ctx);

void v3d_flag_dirty_sampler_state(struct v3d_context *v3d,
                                  enum pipe_shader_type shader);

void v3d_get_tile_buffer_size(const struct v3d_device_info *devinfo,
                              bool is_msaa,
                              bool double_buffer,
                              uint32_t nr_cbufs,
                              struct pipe_surface **cbufs,
                              struct pipe_surface *bbuf,
                              uint32_t *tile_width,
                              uint32_t *tile_height,
                              uint32_t *max_bpp);

bool v3d_render_condition_check(struct v3d_context *v3d);

#ifdef ENABLE_SHADER_CACHE
struct v3d_compiled_shader *v3d_disk_cache_retrieve(struct v3d_context *v3d,
                                                    const struct v3d_key *key,
                                                    const struct v3d_uncompiled_shader *uncompiled);

void v3d_disk_cache_store(struct v3d_context *v3d,
                          const struct v3d_key *key,
                          const struct v3d_uncompiled_shader *uncompiled,
                          const struct v3d_compiled_shader *shader,
                          uint64_t *qpu_insts,
                          uint32_t qpu_size);
#endif /* ENABLE_SHADER_CACHE */

/* Helper to call hw ver specific functions */
#define v3d_X(devinfo, thing) ({                                \
        __typeof(&v3d42_##thing) v3d_X_thing;                   \
        switch (devinfo->ver) {                                 \
        case 42:                                                \
                v3d_X_thing = &v3d42_##thing;                   \
                break;                                          \
        case 71:                                                \
                v3d_X_thing = &v3d71_##thing;                   \
                break;                                          \
        default:                                                \
                unreachable("Unsupported hardware generation"); \
        }                                                       \
        v3d_X_thing;                                            \
})

/* FIXME: The same for vulkan/opengl. Common place? define it at the
 * v3d_packet files?
 */
#define V3D42_CLIPPER_XY_GRANULARITY 256.0f
#define V3D71_CLIPPER_XY_GRANULARITY 64.0f

/* Helper to get hw-specific macro values */
#define V3DV_X(devinfo, thing) ({                               \
   __typeof(V3D42_##thing) V3D_X_THING;                         \
   switch (devinfo->ver) {                                      \
   case 42:                                                     \
      V3D_X_THING = V3D42_##thing;                              \
      break;                                                    \
   case 71:                                                     \
      V3D_X_THING = V3D71_##thing;                              \
      break;                                                    \
   default:                                                     \
      unreachable("Unsupported hardware generation");           \
   }                                                            \
   V3D_X_THING;                                                 \
})

#ifdef v3dX
#  include "v3dx_context.h"
#else
#  define v3dX(x) v3d42_##x
#  include "v3dx_context.h"
#  undef v3dX

#  define v3dX(x) v3d71_##x
#  include "v3dx_context.h"
#  undef v3dX
#endif

#endif /* V3D_CONTEXT_H */
