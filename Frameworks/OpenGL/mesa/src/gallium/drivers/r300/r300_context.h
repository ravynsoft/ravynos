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

#ifndef R300_CONTEXT_H
#define R300_CONTEXT_H

#define R300_BUFFER_ALIGNMENT 64

#include "draw/draw_vertex.h"

#include "util/u_blitter.h"

#include "pipe/p_context.h"
#include "util/u_inlines.h"
#include "util/u_transfer.h"

#include "r300_defines.h"
#include "r300_screen.h"
#include "compiler/radeon_regalloc.h"

struct u_upload_mgr;
struct r300_context;
struct r300_fragment_shader;
struct r300_vertex_shader;
struct r300_stencilref_context;

enum colormask_swizzle {
    COLORMASK_BGRA,
    COLORMASK_RGBA,
    COLORMASK_RRRR,
    COLORMASK_AAAA,
    COLORMASK_GRRG,
    COLORMASK_ARRA,
    COLORMASK_BGRX,
    COLORMASK_RGBX,
    COLORMASK_NUM_SWIZZLES
};

struct r300_atom {
    /* Name, for debugging. */
    const char* name;
    /* Opaque state. */
    void* state;
    /* Emit the state to the context. */
    void (*emit)(struct r300_context*, unsigned, void*);
    /* Upper bound on number of dwords to emit. */
    unsigned size;
    /* Whether this atom should be emitted. */
    bool dirty;
    /* Whether this atom may be emitted with state == NULL. */
    bool allow_null_state;
};

struct r300_aa_state {
    struct r300_surface *dest;

    uint32_t aa_config;
};

struct r300_blend_state {
    struct pipe_blend_state state;

    uint32_t cb_clamp[COLORMASK_NUM_SWIZZLES][8];
    uint32_t cb_noclamp[8];
    uint32_t cb_noclamp_noalpha[8];
    uint32_t cb_no_readwrite[8];
};

struct r300_blend_color_state {
    struct pipe_blend_color state;
    uint32_t cb[3];
};

struct r300_clip_state {
    uint32_t cb[29];
};

struct r300_dsa_state {
    struct pipe_depth_stencil_alpha_state dsa;

    /* This is actually a command buffer with named dwords. */
    uint32_t cb_begin;
    uint32_t z_buffer_control;  /* R300_ZB_CNTL: 0x4f00 */
    uint32_t z_stencil_control; /* R300_ZB_ZSTENCILCNTL: 0x4f04 */
    uint32_t stencil_ref_mask;  /* R300_ZB_STENCILREFMASK: 0x4f08 */
    uint32_t cb_reg;
    uint32_t stencil_ref_bf;    /* R500_ZB_STENCILREFMASK_BF: 0x4fd4 */
    uint32_t cb_reg1;
    uint32_t alpha_value;       /* R500_FG_ALPHA_VALUE: 0x4be0 */

    /* Same, but without ZB reads and writes. */
    uint32_t cb_zb_no_readwrite[8]; /* ZB not bound */

    /* Emitted separately: */
    uint32_t alpha_function;

    /* Whether a two-sided stencil is enabled. */
    bool two_sided;
    /* Whether a fallback should be used for a two-sided stencil ref value. */
    bool two_sided_stencil_ref;
};

struct r300_hyperz_state {
    int flush;
    /* This is actually a command buffer with named dwords. */
    uint32_t cb_flush_begin;
    uint32_t zb_zcache_ctlstat;     /* R300_ZB_CACHE_CNTL */
    uint32_t cb_begin;
    uint32_t zb_bw_cntl;            /* R300_ZB_BW_CNTL */
    uint32_t cb_reg1;
    uint32_t zb_depthclearvalue;    /* R300_ZB_DEPTHCLEARVALUE */
    uint32_t cb_reg2;
    uint32_t sc_hyperz;             /* R300_SC_HYPERZ */
    uint32_t cb_reg3;
    uint32_t gb_z_peq_config;       /* R300_GB_Z_PEQ_CONFIG: 0x4028 */
};

struct r300_gpu_flush {
    uint32_t cb_flush_clean[6];
};

#define RS_STATE_MAIN_SIZE 27

struct r300_rs_state {
    /* Original rasterizer state. */
    struct pipe_rasterizer_state rs;
    /* Draw-specific rasterizer state. */
    struct pipe_rasterizer_state rs_draw;

    /* Command buffers. */
    uint32_t cb_main[RS_STATE_MAIN_SIZE];
    uint32_t cb_poly_offset_zb16[5];
    uint32_t cb_poly_offset_zb24[5];

    /* The index to cb_main where the cull_mode register value resides. */
    unsigned cull_mode_index;

    /* Whether polygon offset is enabled. */
    bool polygon_offset_enable;

    /* This is emitted in the draw function. */
    uint32_t color_control;         /* R300_GA_COLOR_CONTROL: 0x4278 */
};

struct r300_rs_block {
    uint32_t vap_vtx_state_cntl;  /* R300_VAP_VTX_STATE_CNTL: 0x2180 */
    uint32_t vap_vsm_vtx_assm;    /* R300_VAP_VSM_VTX_ASSM: 0x2184 */
    uint32_t vap_out_vtx_fmt[2];  /* R300_VAP_OUTPUT_VTX_FMT_[0-1]: 0x2090 */
    uint32_t gb_enable;

    uint32_t ip[8]; /* R300_RS_IP_[0-7], R500_RS_IP_[0-7] */
    uint32_t count; /* R300_RS_COUNT */
    uint32_t inst_count; /* R300_RS_INST_COUNT */
    uint32_t inst[8]; /* R300_RS_INST_[0-7] */
};

struct r300_sampler_state {
    struct pipe_sampler_state state;

    uint32_t filter0;      /* R300_TX_FILTER0: 0x4400 */
    uint32_t filter1;      /* R300_TX_FILTER1: 0x4440 */

    /* Min/max LOD must be clamped to [0, last_level], thus
     * it's dependent on a currently bound texture */
    unsigned min_lod, max_lod;
};

struct r300_texture_format_state {
    uint32_t format0; /* R300_TX_FORMAT0: 0x4480 */
    uint32_t format1; /* R300_TX_FORMAT1: 0x44c0 */
    uint32_t format2; /* R300_TX_FORMAT2: 0x4500 */
    uint32_t tile_config; /* R300_TX_OFFSET (subset thereof) */
    uint32_t us_format0;   /* R500_US_FORMAT0_0: 0x4640 (through 15) */
};

struct r300_sampler_view {
    struct pipe_sampler_view base;

    /* For resource_copy_region. */
    unsigned width0_override;
    unsigned height0_override;

    /* Swizzles in the PIPE_SWIZZLE_* representation,
     * derived from base. */
    unsigned char swizzle[4];

    /* Copy of r300_texture::texture_format_state with format-specific bits
     * added. */
    struct r300_texture_format_state format;

    /* The texture cache region for this texture. */
    uint32_t texcache_region;
};

struct r300_texture_sampler_state {
    struct r300_texture_format_state format;
    uint32_t filter0;      /* R300_TX_FILTER0: 0x4400 */
    uint32_t filter1;      /* R300_TX_FILTER1: 0x4440 */
    uint32_t border_color; /* R300_TX_BORDER_COLOR: 0x45c0 */
};

struct r300_textures_state {
    /* Textures. */
    struct r300_sampler_view *sampler_views[16];
    int sampler_view_count;
    /* Sampler states. */
    struct r300_sampler_state *sampler_states[16];
    int sampler_state_count;

    /* This is the merge of the texture and sampler states. */
    unsigned count;
    uint32_t tx_enable;         /* R300_TX_ENABLE: 0x4101 */
    struct r300_texture_sampler_state regs[16];
};

struct r300_vertex_stream_state {
    /* R300_VAP_PROG_STREAK_CNTL_[0-7] */
    uint32_t vap_prog_stream_cntl[8];
    /* R300_VAP_PROG_STREAK_CNTL_EXT_[0-7] */
    uint32_t vap_prog_stream_cntl_ext[8];

    unsigned count;
};

struct r300_invariant_state {
    uint32_t cb[24];
};

struct r300_vap_invariant_state {
    uint32_t cb[11];
};

struct r300_viewport_state {
    float xscale;         /* R300_VAP_VPORT_XSCALE:  0x2098 */
    float xoffset;        /* R300_VAP_VPORT_XOFFSET: 0x209c */
    float yscale;         /* R300_VAP_VPORT_YSCALE:  0x20a0 */
    float yoffset;        /* R300_VAP_VPORT_YOFFSET: 0x20a4 */
    float zscale;         /* R300_VAP_VPORT_ZSCALE:  0x20a8 */
    float zoffset;        /* R300_VAP_VPORT_ZOFFSET: 0x20ac */
    uint32_t vte_control; /* R300_VAP_VTE_CNTL:      0x20b0 */
};

struct r300_ztop_state {
    uint32_t z_buffer_top;      /* R300_ZB_ZTOP: 0x4f14 */
};

/* The next several objects are not pure Radeon state; they inherit from
 * various Gallium classes. */

struct r300_constant_buffer {
    /* Buffer of constants */
    uint32_t *ptr;
    /* Remapping table. */
    unsigned *remap_table;
    /* const buffer base */
    uint32_t buffer_base;
};

/* Query object.
 *
 * This is not a subclass of pipe_query because pipe_query is never
 * actually fully defined. So, rather than have it as a member, and do
 * subclass-style casting, we treat pipe_query as an opaque, and just
 * trust that our gallium frontend does not ever mess up query objects.
 */
struct r300_query {
    /* The kind of query. Currently only OQ is supported. */
    unsigned type;
    /* The number of pipes where query results are stored. */
    unsigned num_pipes;
    /* How many results have been written, in dwords. It's incremented
     * after end_query and flush. */
    unsigned num_results;
    /* if begin has been emitted */
    bool begin_emitted;

    /* The buffer where query results are stored. */
    struct pb_buffer_lean *buf;
};

struct r300_surface {
    struct pipe_surface base;

    /* Winsys buffer backing the texture. */
    struct pb_buffer_lean *buf;

    enum radeon_bo_domain domain;

    uint32_t offset;    /* COLOROFFSET or DEPTHOFFSET. */
    uint32_t pitch;     /* COLORPITCH or DEPTHPITCH. */
    uint32_t pitch_zmask; /* ZMASK_PITCH */
    uint32_t pitch_hiz;   /* HIZ_PITCH */
    uint32_t pitch_cmask; /* CMASK_PITCH */
    uint32_t format;    /* US_OUT_FMT or ZB_FORMAT. */

    /* Parameters dedicated to the CBZB clear. */
    uint32_t cbzb_width;            /* Aligned width. */
    uint32_t cbzb_height;           /* Half of the height. */
    uint32_t cbzb_midpoint_offset;  /* DEPTHOFFSET. */
    uint32_t cbzb_pitch;            /* DEPTHPITCH. */
    uint32_t cbzb_format;           /* ZB_FORMAT. */

    /* Whether the CBZB clear is allowed on the surface. */
    bool cbzb_allowed;

    unsigned colormask_swizzle;
};

struct r300_texture_desc {
    /* Width, height, and depth.
     * Most of the time, these are equal to pipe_texture::width0, height0,
     * and depth0. However, NPOT 3D textures must have dimensions aligned
     * to POT, and this is the only case when these variables differ from
     * pipe_texture. */
    unsigned width0, height0, depth0;

    /* Buffer tiling.
     * Macrotiling is specified per-level because small mipmaps cannot
     * be macrotiled. */
    enum radeon_bo_layout microtile;
    enum radeon_bo_layout macrotile[R300_MAX_TEXTURE_LEVELS];

    /* Offsets into the buffer. */
    unsigned offset_in_bytes[R300_MAX_TEXTURE_LEVELS];

    /* Strides for each mip-level. */
    unsigned stride_in_bytes[R300_MAX_TEXTURE_LEVELS];

    /* Size of one zslice or face or 2D image based on the texture target. */
    unsigned layer_size_in_bytes[R300_MAX_TEXTURE_LEVELS];

    /* Total size of this texture, in bytes,
     * derived from the texture properties. */
    unsigned size_in_bytes;

    /**
     * If non-zero, override the natural texture layout with
     * a custom stride (in bytes).
     *
     * \note Mipmapping fails for textures with a non-natural layout!
     *
     * \sa r300_texture_get_stride
     */
    unsigned stride_in_bytes_override;

    /* Whether this texture has non-power-of-two dimensions.
     * It can be either a regular texture or a rectangle one. */
    bool is_npot;

    /* This flag says that hardware must use the stride for addressing
     * instead of the width. */
    bool uses_stride_addressing;

    /* Whether CBZB fast color clear is allowed on the miplevel. */
    bool cbzb_allowed[R300_MAX_TEXTURE_LEVELS];

    /* Zbuffer compression info for each miplevel. */
    bool zcomp8x8[R300_MAX_TEXTURE_LEVELS];
    /* If zero, then disable Z compression/HiZ. */
    unsigned zmask_dwords[R300_MAX_TEXTURE_LEVELS];
    unsigned hiz_dwords[R300_MAX_TEXTURE_LEVELS];
    /* Zmask/HiZ strides for each miplevel. */
    unsigned zmask_stride_in_pixels[R300_MAX_TEXTURE_LEVELS];
    unsigned hiz_stride_in_pixels[R300_MAX_TEXTURE_LEVELS];

    /* CMASK info for AA buffers (no mipmapping). */
    unsigned cmask_dwords;
    unsigned cmask_stride_in_pixels;
};

struct r300_resource
{
    struct pipe_resource b;

    /* Winsys buffer backing this resource. */
    struct pb_buffer_lean *buf;
    enum radeon_bo_domain domain;

    /* Constant buffers and SWTCL vertex and index buffers are in user
     * memory. */
    uint8_t *malloced_buffer;

    /* Texture description (addressing, layout, special features). */
    struct r300_texture_desc tex;

    /* This is the level tiling flags were last time set for.
     * It's used to prevent redundant tiling-flags changes from happening.*/
    unsigned surface_level;
};

struct r300_vertex_element_state {
    unsigned count;
    struct pipe_vertex_element velem[PIPE_MAX_ATTRIBS];
    unsigned format_size[PIPE_MAX_ATTRIBS];

    /* The size of the vertex, in dwords. */
    unsigned vertex_size_dwords;

    struct r300_vertex_stream_state vertex_stream;
};

enum r300_hiz_func {
    HIZ_FUNC_NONE,

    /* The function, when determined, is set in stone
     * until the next HiZ clear. */

    /* MAX is written to the HiZ buffer.
     * Used for LESS, LEQUAL. */
    HIZ_FUNC_MAX,

    /* MIN is written to the HiZ buffer.
     * Used for GREATER, GEQUAL. */
    HIZ_FUNC_MIN,
};

/* For deferred fragment shader state validation. */
enum r300_fs_validity_status {
    FRAGMENT_SHADER_VALID,      /* No need to change/validate the FS. */
    FRAGMENT_SHADER_MAYBE_DIRTY,/* Validate the FS if external state was changed. */
    FRAGMENT_SHADER_DIRTY       /* Always validate the FS (if the FS was changed) */
};

struct r300_context {
    /* Parent class */
    struct pipe_context context;

    /* The interface to the windowing system, etc. */
    struct radeon_winsys *rws;
    /* The submission context. */
    struct radeon_winsys_ctx *ctx;
    /* The command stream. */
    struct radeon_cmdbuf cs;
    /* Screen. */
    struct r300_screen *screen;

    /* Draw module. Used mostly for SW TCL. */
    struct draw_context* draw;
    /* Vertex buffer for SW TCL. */
    struct pb_buffer_lean *vbo;
    /* Offset and size into the SW TCL VBO. */
    size_t draw_vbo_offset;

    /* Accelerated blit support. */
    struct blitter_context* blitter;
    /* Stencil two-sided reference value fallback. */
    struct r300_stencilref_context *stencilref_fallback;

    /* The KIL opcode needs the first texture unit to be enabled
     * on r3xx-r4xx. In order to calm down the CS checker, we bind this
     * dummy texture there. */
    struct r300_sampler_view *texkill_sampler;

    /* When no vertex buffer is set, this one is used instead to prevent
     * hardlocks. */
    struct pipe_vertex_buffer dummy_vb;

    /* The currently active query. */
    struct r300_query *query_current;
    /* The saved query for blitter operations. */
    struct r300_query *blitter_saved_query;
    /* Query list. */
    struct r300_query query_list;

    /* Various CSO state objects. */

    /* Each atom is emitted in the order it appears here, which can affect
     * performance and stability if not handled with care. */
    /* GPU flush. */
    struct r300_atom gpu_flush;
    /* Clears must be emitted immediately after the flush. */
    /* HiZ clear */
    struct r300_atom hiz_clear;
    /* zmask clear */
    struct r300_atom zmask_clear;
    /* cmask clear */
    struct r300_atom cmask_clear;
    /* Anti-aliasing (MSAA) state. */
    struct r300_atom aa_state;
    /* Framebuffer state. */
    struct r300_atom fb_state;
    /* HyperZ state (various SC/ZB bits). */
    struct r300_atom hyperz_state;
    /* ZTOP state. */
    struct r300_atom ztop_state;
    /* Depth, stencil, and alpha state. */
    struct r300_atom dsa_state;
    /* Blend state. */
    struct r300_atom blend_state;
    /* Blend color state. */
    struct r300_atom blend_color_state;
    /* Scissor state. */
    struct r300_atom scissor_state;
    /* Sample mask. */
    struct r300_atom sample_mask;
    /* Invariant state. This must be emitted to get the engine started. */
    struct r300_atom invariant_state;
    /* Viewport state. */
    struct r300_atom viewport_state;
    /* PVS flush. */
    struct r300_atom pvs_flush;
    /* VAP invariant state. */
    struct r300_atom vap_invariant_state;
    /* Vertex stream formatting state. */
    struct r300_atom vertex_stream_state;
    /* Vertex shader. */
    struct r300_atom vs_state;
    /* User clip planes. */
    struct r300_atom clip_state;
    /* RS block state + VAP (vertex shader) output mapping state. */
    struct r300_atom rs_block_state;
    /* Rasterizer state. */
    struct r300_atom rs_state;
    /* Framebuffer state (pipelined regs). */
    struct r300_atom fb_state_pipelined;
    /* Fragment shader. */
    struct r300_atom fs;
    /* Fragment shader RC_CONSTANT_STATE variables. */
    struct r300_atom fs_rc_constant_state;
    /* Fragment shader constant buffer. */
    struct r300_atom fs_constants;
    /* Vertex shader constant buffer. */
    struct r300_atom vs_constants;
    /* Texture cache invalidate. */
    struct r300_atom texture_cache_inval;
    /* Textures state. */
    struct r300_atom textures_state;
    /* Occlusion query. */
    struct r300_atom query_start;

    /* The pointers to the first and the last atom. */
    struct r300_atom *first_dirty, *last_dirty;

    /* Vertex elements for Gallium. */
    struct r300_vertex_element_state *velems;

    /* Vertex info for Draw. */
    struct vertex_info vertex_info;

    struct pipe_stencil_ref stencil_ref;
    struct pipe_viewport_state viewport;

    /* Stream locations for SWTCL. */
    int stream_loc_notcl[16];

    /* Flag indicating whether or not the HW is dirty. */
    uint32_t dirty_hw;
    /* Whether polygon offset is enabled. */
    bool polygon_offset_enabled;
    /* Z buffer bit depth. */
    uint32_t zbuffer_bpp;
    /* Whether rendering is conditional and should be skipped. */
    bool skip_rendering;
    /* The flag above saved by blitter. */
    unsigned char blitter_saved_skip_rendering;
    /* Point sprites texcoord index,  1 bit per texcoord */
    int sprite_coord_enable;
    /* Whether we are drawing points, to disable sprite coord if not */
    bool is_point;
    /* Whether two-sided color selection is enabled (AKA light_twoside). */
    bool two_sided_color;
    bool flatshade;
    bool clip_halfz;
    /* Whether fast color clear is enabled. */
    bool cbzb_clear;
    /* Whether fragment shader needs to be validated. */
    enum r300_fs_validity_status fs_status;
    /* Framebuffer multi-write. */
    bool fb_multiwrite;
    unsigned num_samples;
    bool msaa_enable;
    bool alpha_to_one;
    bool alpha_to_coverage;

    void *dsa_decompress_zmask;

    struct pipe_vertex_buffer vertex_buffer[PIPE_MAX_ATTRIBS];
    unsigned nr_vertex_buffers;
    struct u_upload_mgr *uploader;

    struct slab_child_pool pool_transfers;

    /* Stat counter. */
    uint64_t flush_counter;

    /* const tracking for VS */
    int vs_const_base;

    /* Vertex array state info */
    bool vertex_arrays_dirty;
    bool vertex_arrays_indexed;
    int vertex_arrays_offset;
    int vertex_arrays_instance_id;
    bool instancing_enabled;

    /* Hyper-Z stats. */
    bool hyperz_enabled;     /* Whether it owns Hyper-Z access. */
    int64_t hyperz_time_of_last_flush; /* Time of the last flush with Z clear. */
    unsigned num_z_clears;      /* Since the last flush. */

    /* ZMask state. */
    bool zmask_in_use;       /* Whether ZMASK is enabled. */
    bool zmask_decompress;   /* Whether ZMASK is being decompressed. */
    struct pipe_surface *locked_zbuffer; /* Unbound zbuffer which still has data in ZMASK. */

    /* HiZ state. */
    bool hiz_in_use;         /* Whether HIZ is enabled. */
    enum r300_hiz_func hiz_func; /* HiZ function. Can be either MIN or MAX. */
    uint32_t hiz_clear_value;   /* HiZ clear value. */

    /* CMASK state. */
    bool cmask_access;
    bool cmask_in_use;
    uint32_t color_clear_value; /* RGBA8 or RGBA1010102 */
    uint32_t color_clear_value_ar; /* RGBA16F */
    uint32_t color_clear_value_gb; /* RGBA16F */

    /* Compiler state. */
    struct rc_regalloc_state fs_regalloc_state; /* Register allocator info for
                                                 * fragment shaders. */
    struct rc_regalloc_state vs_regalloc_state; /* Register allocator info for
                                                 * vertex shaders. */
};

#define foreach_atom(r300, atom) \
    for (atom = &r300->gpu_flush; atom != (&r300->query_start)+1; atom++)

#define foreach_dirty_atom(r300, atom) \
    for (atom = r300->first_dirty; atom != r300->last_dirty; atom++)

/* Convenience cast wrappers. */
static inline struct r300_query* r300_query(struct pipe_query* q)
{
    return (struct r300_query*)q;
}

static inline struct r300_surface* r300_surface(struct pipe_surface* surf)
{
    return (struct r300_surface*)surf;
}

static inline struct r300_resource* r300_resource(struct pipe_resource* tex)
{
    return (struct r300_resource*)tex;
}

static inline struct r300_context* r300_context(struct pipe_context* context)
{
    return (struct r300_context*)context;
}

static inline struct r300_fragment_shader *r300_fs(struct r300_context *r300)
{
    return (struct r300_fragment_shader*)r300->fs.state;
}

static inline struct r300_vertex_shader *r300_vs(struct r300_context *r300)
{
    return (struct r300_vertex_shader*)r300->vs_state.state;
}

static inline void r300_mark_atom_dirty(struct r300_context *r300,
                                        struct r300_atom *atom)
{
    atom->dirty = true;

    if (!r300->first_dirty) {
        r300->first_dirty = atom;
        r300->last_dirty = atom+1;
    } else {
        if (atom < r300->first_dirty)
            r300->first_dirty = atom;
        else if (atom+1 > r300->last_dirty)
            r300->last_dirty = atom+1;
    }
}

static inline struct pipe_surface *
r300_get_nonnull_cb(struct pipe_framebuffer_state *fb, unsigned i)
{
    if (fb->cbufs[i])
        return fb->cbufs[i];

    /* The i-th framebuffer is NULL, return any non-NULL one. */
    for (i = 0; i < fb->nr_cbufs; i++)
        if (fb->cbufs[i])
            return fb->cbufs[i];

    return NULL;
}

struct pipe_context* r300_create_context(struct pipe_screen* screen,
                                         void *priv, unsigned flags);

/* Context initialization. */
struct draw_stage* r300_draw_stage(struct r300_context* r300);
void r300_init_blit_functions(struct r300_context *r300);
void r300_init_flush_functions(struct r300_context* r300);
void r300_init_query_functions(struct r300_context* r300);
void r300_init_render_functions(struct r300_context *r300);
void r300_init_state_functions(struct r300_context* r300);
void r300_init_resource_functions(struct r300_context* r300);

/* r300_blit.c */
void r300_decompress_zmask(struct r300_context *r300);
void r300_decompress_zmask_locked_unsafe(struct r300_context *r300);
void r300_decompress_zmask_locked(struct r300_context *r300);
bool r300_is_blit_supported(enum pipe_format format);

/* r300_flush.c */
void r300_flush(struct pipe_context *pipe,
                unsigned flags,
                struct pipe_fence_handle **fence);

/* r300_hyperz.c */
void r300_update_hyperz_state(struct r300_context* r300);

/* r300_query.c */
void r300_resume_query(struct r300_context *r300,
                       struct r300_query *query);
void r300_stop_query(struct r300_context *r300);

/* r300_render_translate.c */
void r300_translate_index_buffer(struct r300_context *r300,
                                 const struct pipe_draw_info *info,
                                 struct pipe_resource **out_index_buffer,
                                 unsigned *index_size, unsigned index_offset,
                                 unsigned *start, unsigned count);

/* r300_render_stencilref.c */
void r300_plug_in_stencil_ref_fallback(struct r300_context *r300);

/* r300_render.c */
void r500_emit_index_bias(struct r300_context *r300, int index_bias);
void r300_blitter_draw_rectangle(struct blitter_context *blitter,
                                 void *vertex_elements_cso,
                                 blitter_get_vs_func get_vs,
                                 int x1, int y1, int x2, int y2,
                                 float depth, unsigned num_instances,
                                 enum blitter_attrib_type type,
                                 const union blitter_attrib *attrib);

/* r300_state.c */
enum r300_fb_state_change {
    R300_CHANGED_FB_STATE = 0,
    R300_CHANGED_HYPERZ_FLAG,
    R300_CHANGED_MULTIWRITE,
    R300_CHANGED_CMASK_ENABLE,
};

void r300_mark_fb_state_dirty(struct r300_context *r300,
                              enum r300_fb_state_change change);
void r300_mark_fs_code_dirty(struct r300_context *r300);

struct pipe_sampler_view *
r300_create_sampler_view_custom(struct pipe_context *pipe,
                         struct pipe_resource *texture,
                         const struct pipe_sampler_view *templ,
                         unsigned width0_override,
                         unsigned height0_override);

/* r300_state_derived.c */
void r300_update_derived_state(struct r300_context* r300);

/* r300_debug.c */
void r500_dump_rs_block(struct r300_rs_block *rs);


static inline bool CTX_DBG_ON(struct r300_context * ctx, unsigned flags)
{
    return SCREEN_DBG_ON(ctx->screen, flags);
}

static inline void CTX_DBG(struct r300_context * ctx, unsigned flags,
                       const char * fmt, ...)
{
    if (CTX_DBG_ON(ctx, flags)) {
        va_list va;
        va_start(va, fmt);
        vfprintf(stderr, fmt, va);
        va_end(va);
    }
}

#define DBG_ON  CTX_DBG_ON
#define DBG     CTX_DBG

#endif /* R300_CONTEXT_H */
