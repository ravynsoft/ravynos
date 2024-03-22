/*
 * Copyright © 2014 Broadcom
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

#ifndef VC4_CONTEXT_H
#define VC4_CONTEXT_H

#include <stdio.h>

#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include "util/slab.h"
#include "util/u_debug_cb.h"
#include "xf86drm.h"

#define __user
#include "drm-uapi/vc4_drm.h"
#include "vc4_bufmgr.h"
#include "vc4_resource.h"
#include "vc4_cl.h"
#include "vc4_qir.h"

#ifndef DRM_VC4_PARAM_SUPPORTS_ETC1
#define DRM_VC4_PARAM_SUPPORTS_ETC1		4
#endif
#ifndef DRM_VC4_PARAM_SUPPORTS_THREADED_FS
#define DRM_VC4_PARAM_SUPPORTS_THREADED_FS	5
#endif

#ifdef USE_VC4_SIMULATOR
#define using_vc4_simulator true
#else
#define using_vc4_simulator false
#endif

#define VC4_DIRTY_BLEND         (1 <<  0)
#define VC4_DIRTY_RASTERIZER    (1 <<  1)
#define VC4_DIRTY_ZSA           (1 <<  2)
#define VC4_DIRTY_FRAGTEX       (1 <<  3)
#define VC4_DIRTY_VERTTEX       (1 <<  4)

#define VC4_DIRTY_BLEND_COLOR   (1 <<  7)
#define VC4_DIRTY_STENCIL_REF   (1 <<  8)
#define VC4_DIRTY_SAMPLE_MASK   (1 <<  9)
#define VC4_DIRTY_FRAMEBUFFER   (1 << 10)
#define VC4_DIRTY_STIPPLE       (1 << 11)
#define VC4_DIRTY_VIEWPORT      (1 << 12)
#define VC4_DIRTY_CONSTBUF      (1 << 13)
#define VC4_DIRTY_VTXSTATE      (1 << 14)
#define VC4_DIRTY_VTXBUF        (1 << 15)

#define VC4_DIRTY_SCISSOR       (1 << 17)
#define VC4_DIRTY_FLAT_SHADE_FLAGS (1 << 18)
#define VC4_DIRTY_PRIM_MODE     (1 << 19)
#define VC4_DIRTY_CLIP          (1 << 20)
#define VC4_DIRTY_UNCOMPILED_VS (1 << 21)
#define VC4_DIRTY_UNCOMPILED_FS (1 << 22)
#define VC4_DIRTY_COMPILED_CS   (1 << 23)
#define VC4_DIRTY_COMPILED_VS   (1 << 24)
#define VC4_DIRTY_COMPILED_FS   (1 << 25)
#define VC4_DIRTY_FS_INPUTS     (1 << 26)
#define VC4_DIRTY_UBO_1_SIZE    (1 << 27)

struct vc4_sampler_view {
        struct pipe_sampler_view base;
        uint32_t texture_p0;
        uint32_t texture_p1;
        bool force_first_level;
        /**
         * Resource containing the actual texture that will be sampled.
         *
         * We may need to rebase the .base.texture resource to work around the
         * lack of GL_TEXTURE_BASE_LEVEL, or to upload the texture as tiled.
         */
        struct pipe_resource *texture;
};

struct vc4_sampler_state {
        struct pipe_sampler_state base;
        uint32_t texture_p1;
};

struct vc4_texture_stateobj {
        struct pipe_sampler_view *textures[PIPE_MAX_SAMPLERS];
        unsigned num_textures;
        struct pipe_sampler_state *samplers[PIPE_MAX_SAMPLERS];
        unsigned num_samplers;
};

struct vc4_shader_uniform_info {
        enum quniform_contents *contents;
        uint32_t *data;
        uint32_t count;
        uint32_t num_texture_samples;
};

struct vc4_uncompiled_shader {
        /** A name for this program, so you can track it in shader-db output. */
        uint32_t program_id;
        /** How many variants of this program were compiled, for shader-db. */
        uint32_t compiled_variant_count;
        struct pipe_shader_state base;
};

struct vc4_fs_inputs {
        /**
         * Array of the meanings of the VPM inputs this shader needs.
         *
         * It doesn't include those that aren't part of the VPM, like
         * point/line coordinates.
         */
        struct vc4_varying_slot *input_slots;
        uint32_t num_inputs;
};

struct vc4_compiled_shader {
        uint64_t program_id;
        struct vc4_bo *bo;

        struct vc4_shader_uniform_info uniforms;

        /**
         * VC4_DIRTY_* flags that, when set in vc4->dirty, mean that the
         * uniforms have to be rewritten (and therefore the shader state
         * reemitted).
         */
        uint32_t uniform_dirty_bits;

        /** bitmask of which inputs are color inputs, for flat shade handling. */
        uint32_t color_inputs;

        bool disable_early_z;

        /* Set if the compile failed, likely due to register allocation
         * failure.  In this case, we have no shader to run and should not try
         * to do any draws.
         */
        bool failed;

        bool fs_threaded;

        uint8_t num_inputs;

        /* Byte offsets for the start of the vertex attributes 0-7, and the
         * total size as "attribute" 8.
         */
        uint8_t vattr_offsets[9];
        uint8_t vattrs_live;

        const struct vc4_fs_inputs *fs_inputs;
};

struct vc4_program_stateobj {
        struct vc4_uncompiled_shader *bind_vs, *bind_fs;
        struct vc4_compiled_shader *cs, *vs, *fs;
};

struct vc4_constbuf_stateobj {
        struct pipe_constant_buffer cb[PIPE_MAX_CONSTANT_BUFFERS];
        uint32_t enabled_mask;
        uint32_t dirty_mask;
};

struct vc4_vertexbuf_stateobj {
        struct pipe_vertex_buffer vb[PIPE_MAX_ATTRIBS];
        unsigned count;
        uint32_t enabled_mask;
        uint32_t dirty_mask;
};

struct vc4_vertex_stateobj {
        struct pipe_vertex_element pipe[PIPE_MAX_ATTRIBS];
        unsigned num_elements;
};

/* Hash table key for vc4->jobs */
struct vc4_job_key {
        struct pipe_surface *cbuf;
        struct pipe_surface *zsbuf;
};

struct vc4_hwperfmon {
        uint32_t id;
        uint64_t last_seqno;
        uint8_t events[DRM_VC4_MAX_PERF_COUNTERS];
        uint64_t counters[DRM_VC4_MAX_PERF_COUNTERS];
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
struct vc4_job {
        struct vc4_cl bcl;
        struct vc4_cl shader_rec;
        struct vc4_cl uniforms;
        struct vc4_cl bo_handles;
        struct vc4_cl bo_pointers;
        uint32_t shader_rec_count;
        /**
         * Amount of memory used by the BOs in bo_pointers.
         *
         * Used for checking when we should flush the job early so we don't
         * OOM.
         */
        uint32_t bo_space;

        /* Last BO hindex referenced from VC4_PACKET_GEM_HANDLES. */
        uint32_t last_gem_handle_hindex;

        /** @{ Surfaces to submit rendering for. */
        struct pipe_surface *color_read;
        struct pipe_surface *color_write;
        struct pipe_surface *zs_read;
        struct pipe_surface *zs_write;
        struct pipe_surface *msaa_color_write;
        struct pipe_surface *msaa_zs_write;
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
         * Width/height of the color framebuffer being rendered to,
         * for VC4_TILE_RENDERING_MODE_CONFIG.
        */
        uint32_t draw_width;
        uint32_t draw_height;
        /** @} */
        /** @{ Tile information, depending on MSAA and float color buffer. */
        uint32_t draw_tiles_x; /** @< Number of tiles wide for framebuffer. */
        uint32_t draw_tiles_y; /** @< Number of tiles high for framebuffer. */

        uint32_t tile_width; /** @< Width of a tile. */
        uint32_t tile_height; /** @< Height of a tile. */
        /** Whether the current rendering is in a 4X MSAA tile buffer. */
        bool msaa;
	/** @} */

        /* Bitmask of PIPE_CLEAR_* of buffers that were cleared before the
         * first rendering.
         */
        uint32_t cleared;
        /* Bitmask of PIPE_CLEAR_* of buffers that have been rendered to
         * (either clears or draws).
         */
        uint32_t resolve;
        uint32_t clear_color[2];
        uint32_t clear_depth; /**< 24-bit unorm depth */
        uint8_t clear_stencil;

        /**
         * Set if some drawing (triangles, blits, or just a glClear()) has
         * been done to the FBO, meaning that we need to
         * DRM_IOCTL_VC4_SUBMIT_CL.
         */
        bool needs_flush;

        /**
         * Number of draw calls (not counting full buffer clears) queued in
         * the current job.
         */
        uint32_t draw_calls_queued;

        /** Any flags to be passed in drm_vc4_submit_cl.flags. */
        uint32_t flags;

	/* Performance monitor attached to this job. */
	struct vc4_hwperfmon *perfmon;

        struct vc4_job_key key;
};

struct vc4_context {
        struct pipe_context base;

        int fd;
        struct vc4_screen *screen;

        /** The 3D rendering job for the currently bound FBO. */
        struct vc4_job *job;

        /* Map from struct vc4_job_key to the job for that FBO.
         */
        struct hash_table *jobs;

        /**
         * Map from vc4_resource to a job writing to that resource.
         *
         * Primarily for flushing jobs rendering to textures that are now
         * being read from.
         */
        struct hash_table *write_jobs;

        struct slab_child_pool transfer_pool;
        struct blitter_context *blitter;

        /** bitfield of VC4_DIRTY_* */
        uint32_t dirty;

        struct hash_table *fs_cache, *vs_cache;
        struct set *fs_inputs_set;
        uint32_t next_uncompiled_program_id;
        uint64_t next_compiled_program_id;

        struct ra_regs *regs;
        struct ra_class *reg_class_any[2];
        struct ra_class *reg_class_a_or_b[2];
        struct ra_class *reg_class_a_or_b_or_acc[2];
        struct ra_class *reg_class_r0_r3;
        struct ra_class *reg_class_r4_or_a[2];
        struct ra_class *reg_class_a[2];

        uint8_t prim_mode;

        /** Maximum index buffer valid for the current shader_rec. */
        uint32_t max_index;
        /** Last index bias baked into the current shader_rec. */
        uint32_t last_index_bias;

        /** Seqno of the last CL flush's job. */
        uint64_t last_emit_seqno;

        struct u_upload_mgr *uploader;

        struct pipe_shader_state *yuv_linear_blit_vs;
        struct pipe_shader_state *yuv_linear_blit_fs_8bit;
        struct pipe_shader_state *yuv_linear_blit_fs_16bit;

        /** @{ Current pipeline state objects */
        struct pipe_scissor_state scissor;
        struct pipe_blend_state *blend;
        struct vc4_rasterizer_state *rasterizer;
        struct vc4_depth_stencil_alpha_state *zsa;

        struct vc4_texture_stateobj verttex, fragtex;

        struct vc4_program_stateobj prog;

        struct vc4_vertex_stateobj *vtx;

        struct {
                struct pipe_blend_color f;
                uint8_t ub[4];
        } blend_color;
        struct pipe_stencil_ref stencil_ref;
        unsigned sample_mask;
        struct pipe_framebuffer_state framebuffer;
        struct pipe_poly_stipple stipple;
        struct pipe_clip_state clip;
        struct pipe_viewport_state viewport;
        struct vc4_constbuf_stateobj constbuf[PIPE_SHADER_TYPES];
        struct vc4_vertexbuf_stateobj vertexbuf;

        struct vc4_hwperfmon *perfmon;
        /** @} */

        /** Handle of syncobj containing the last submitted job fence. */
        uint32_t job_syncobj;

        int in_fence_fd;
        /** Handle of the syncobj that holds in_fence_fd for submission. */
        uint32_t in_syncobj;
};

struct vc4_rasterizer_state {
        struct pipe_rasterizer_state base;

        /* VC4_CONFIGURATION_BITS */
        uint8_t config_bits[V3D21_CONFIGURATION_BITS_length];

        struct PACKED {
                uint8_t depth_offset[V3D21_DEPTH_OFFSET_length];
                uint8_t point_size[V3D21_POINT_SIZE_length];
                uint8_t line_width[V3D21_LINE_WIDTH_length];
        } packed;

        /** Raster order flags to be passed in struct drm_vc4_submit_cl.flags. */
        uint32_t tile_raster_order_flags;
};

struct vc4_depth_stencil_alpha_state {
        struct pipe_depth_stencil_alpha_state base;

        /* VC4_CONFIGURATION_BITS */
        uint8_t config_bits[V3D21_CONFIGURATION_BITS_length];

        /** Uniforms for stencil state.
         *
         * Index 0 is either the front config, or the front-and-back config.
         * Index 1 is the back config if doing separate back stencil.
         * Index 2 is the writemask config if it's not a common mask value.
         */
        uint32_t stencil_uniforms[3];
};

#define perf_debug(...) do {                            \
        if (VC4_DBG(PERF))                            \
                fprintf(stderr, __VA_ARGS__);           \
        if (unlikely(vc4->base.debug.debug_message))         \
                util_debug_message(&vc4->base.debug, PERF_INFO, __VA_ARGS__); \
} while (0)

static inline struct vc4_context *
vc4_context(struct pipe_context *pcontext)
{
        return (struct vc4_context *)pcontext;
}

static inline struct vc4_sampler_view *
vc4_sampler_view(struct pipe_sampler_view *psview)
{
        return (struct vc4_sampler_view *)psview;
}

static inline struct vc4_sampler_state *
vc4_sampler_state(struct pipe_sampler_state *psampler)
{
        return (struct vc4_sampler_state *)psampler;
}

int vc4_get_driver_query_group_info(struct pipe_screen *pscreen,
                                    unsigned index,
                                    struct pipe_driver_query_group_info *info);
int vc4_get_driver_query_info(struct pipe_screen *pscreen, unsigned index,
                              struct pipe_driver_query_info *info);

struct pipe_context *vc4_context_create(struct pipe_screen *pscreen,
                                        void *priv, unsigned flags);
void vc4_draw_init(struct pipe_context *pctx);
void vc4_state_init(struct pipe_context *pctx);
void vc4_program_init(struct pipe_context *pctx);
void vc4_program_fini(struct pipe_context *pctx);
void vc4_query_init(struct pipe_context *pctx);
void vc4_simulator_init(struct vc4_screen *screen);
void vc4_simulator_destroy(struct vc4_screen *screen);
int vc4_simulator_ioctl(int fd, unsigned long request, void *arg);
void vc4_simulator_open_from_handle(int fd, int handle, uint32_t size);

static inline int
vc4_ioctl(int fd, unsigned long request, void *arg)
{
        if (using_vc4_simulator)
                return vc4_simulator_ioctl(fd, request, arg);
        else
                return drmIoctl(fd, request, arg);
}

void vc4_set_shader_uniform_dirty_flags(struct vc4_compiled_shader *shader);
void vc4_write_uniforms(struct vc4_context *vc4,
                        struct vc4_compiled_shader *shader,
                        struct vc4_constbuf_stateobj *cb,
                        struct vc4_texture_stateobj *texstate);

void vc4_flush(struct pipe_context *pctx);
int vc4_job_init(struct vc4_context *vc4);
int vc4_fence_context_init(struct vc4_context *vc4);
struct vc4_job *vc4_get_job(struct vc4_context *vc4,
                            struct pipe_surface *cbuf,
                            struct pipe_surface *zsbuf);
struct vc4_job *vc4_get_job_for_fbo(struct vc4_context *vc4);

void vc4_job_submit(struct vc4_context *vc4, struct vc4_job *job);
void vc4_flush_jobs_writing_resource(struct vc4_context *vc4,
                                     struct pipe_resource *prsc);
void vc4_flush_jobs_reading_resource(struct vc4_context *vc4,
                                     struct pipe_resource *prsc);
void vc4_emit_state(struct pipe_context *pctx);
void vc4_generate_code(struct vc4_context *vc4, struct vc4_compile *c);
struct qpu_reg *vc4_register_allocate(struct vc4_context *vc4, struct vc4_compile *c);
bool vc4_update_compiled_shaders(struct vc4_context *vc4, uint8_t prim_mode);

bool vc4_rt_format_supported(enum pipe_format f);
bool vc4_rt_format_is_565(enum pipe_format f);
bool vc4_tex_format_supported(enum pipe_format f);
uint8_t vc4_get_tex_format(enum pipe_format f);
const uint8_t *vc4_get_format_swizzle(enum pipe_format f);
void vc4_init_query_functions(struct vc4_context *vc4);
void vc4_blit(struct pipe_context *pctx, const struct pipe_blit_info *blit_info);
void vc4_blitter_save(struct vc4_context *vc4);
#endif /* VC4_CONTEXT_H */
