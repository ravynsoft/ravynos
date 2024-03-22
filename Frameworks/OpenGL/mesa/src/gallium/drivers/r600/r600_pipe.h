/*
 * Copyright 2010 Jerome Glisse <glisse@freedesktop.org>
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
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *      Jerome Glisse
 */
#ifndef R600_PIPE_H
#define R600_PIPE_H

#include "r600_pipe_common.h"
#include "r600_cs.h"
#include "r600_public.h"
#include "pipe/p_defines.h"

#include "util/u_suballoc.h"
#include "util/list.h"
#include "util/u_transfer.h"
#include "util/u_memory.h"

#include "tgsi/tgsi_scan.h"

#define R600_NUM_ATOMS 56

#define R600_MAX_IMAGES 8
/*
 * ranges reserved for images on evergreen
 * first set for the immediate buffers,
 * second for the actual resources for RESQ.
 */
#define R600_IMAGE_IMMED_RESOURCE_OFFSET 160
#define R600_IMAGE_REAL_RESOURCE_OFFSET 168

/* read caches */
#define R600_CONTEXT_INV_VERTEX_CACHE		(R600_CONTEXT_PRIVATE_FLAG << 0)
#define R600_CONTEXT_INV_TEX_CACHE		(R600_CONTEXT_PRIVATE_FLAG << 1)
#define R600_CONTEXT_INV_CONST_CACHE		(R600_CONTEXT_PRIVATE_FLAG << 2)
/* read-write caches */
#define R600_CONTEXT_FLUSH_AND_INV		(R600_CONTEXT_PRIVATE_FLAG << 3)
#define R600_CONTEXT_FLUSH_AND_INV_CB_META	(R600_CONTEXT_PRIVATE_FLAG << 4)
#define R600_CONTEXT_FLUSH_AND_INV_DB_META	(R600_CONTEXT_PRIVATE_FLAG << 5)
#define R600_CONTEXT_FLUSH_AND_INV_DB		(R600_CONTEXT_PRIVATE_FLAG << 6)
#define R600_CONTEXT_FLUSH_AND_INV_CB		(R600_CONTEXT_PRIVATE_FLAG << 7)
/* engine synchronization */
#define R600_CONTEXT_PS_PARTIAL_FLUSH		(R600_CONTEXT_PRIVATE_FLAG << 8)
#define R600_CONTEXT_WAIT_3D_IDLE		(R600_CONTEXT_PRIVATE_FLAG << 9)
#define R600_CONTEXT_WAIT_CP_DMA_IDLE		(R600_CONTEXT_PRIVATE_FLAG << 10)
#define R600_CONTEXT_CS_PARTIAL_FLUSH           (R600_CONTEXT_PRIVATE_FLAG << 11)

/* the number of CS dwords for flushing and drawing */
#define R600_MAX_FLUSH_CS_DWORDS	18
#define R600_MAX_DRAW_CS_DWORDS		58
#define R600_MAX_PFP_SYNC_ME_DWORDS	16

#define EG_MAX_ATOMIC_BUFFERS 8

#define R600_MAX_USER_CONST_BUFFERS 15
#define R600_MAX_DRIVER_CONST_BUFFERS 3
#define R600_MAX_CONST_BUFFERS (R600_MAX_USER_CONST_BUFFERS + R600_MAX_DRIVER_CONST_BUFFERS)

/* start driver buffers after user buffers */
#define R600_BUFFER_INFO_CONST_BUFFER (R600_MAX_USER_CONST_BUFFERS)
#define R600_UCP_SIZE (4*4*8)
#define R600_CS_BLOCK_GRID_SIZE (8 * 4)
#define R600_TCS_DEFAULT_LEVELS_SIZE (6 * 4)
#define R600_BUFFER_INFO_OFFSET (R600_UCP_SIZE)

/*
 * We only access this buffer through vtx clauses hence it's fine to exist
 * at index beyond 15.
 */
#define R600_LDS_INFO_CONST_BUFFER (R600_MAX_USER_CONST_BUFFERS + 1)
/*
 * Note GS doesn't use a constant buffer binding, just a resource index,
 * so it's fine to have it exist at index beyond 15. I.e. it's not actually
 * a const buffer, just a buffer resource.
 */
#define R600_GS_RING_CONST_BUFFER (R600_MAX_USER_CONST_BUFFERS + 2)
/* Currently R600_MAX_CONST_BUFFERS just fits on the hw, which has a limit
 * of 16 const buffers.
 * UCP/SAMPLE_POSITIONS are never accessed by same shader stage so they can use the same id.
 *
 * In order to support d3d 11 mandated minimum of 15 user const buffers
 * we'd have to squash all use cases into one driver buffer.
 */
#define R600_MAX_CONST_BUFFER_SIZE (4096 * sizeof(float[4]))

/* HW stages */
#define R600_HW_STAGE_PS 0
#define R600_HW_STAGE_VS 1
#define R600_HW_STAGE_GS 2
#define R600_HW_STAGE_ES 3
#define EG_HW_STAGE_LS 4
#define EG_HW_STAGE_HS 5

#define R600_NUM_HW_STAGES 4
#define EG_NUM_HW_STAGES 6

struct r600_context;
struct r600_bytecode;
union  r600_shader_key;

/* This is an atom containing GPU commands that never change.
 * This is supposed to be copied directly into the CS. */
struct r600_command_buffer {
	uint32_t *buf;
	unsigned num_dw;
	unsigned max_num_dw;
	unsigned pkt_flags;
};

struct r600_db_state {
	struct r600_atom		atom;
	struct r600_surface		*rsurf;
};

struct r600_db_misc_state {
	struct r600_atom		atom;
	bool				occlusion_queries_disabled;
	bool				flush_depthstencil_through_cb;
	bool				flush_depth_inplace;
	bool				flush_stencil_inplace;
	bool				copy_depth, copy_stencil;
	unsigned			copy_sample;
	unsigned			log_samples;
	unsigned			db_shader_control;
	bool				htile_clear;
	uint8_t				ps_conservative_z;
};

struct r600_cb_misc_state {
	struct r600_atom atom;
	unsigned cb_color_control; /* this comes from blend state */
	unsigned blend_colormask; /* 8*4 bits for 8 RGBA colorbuffers */
	unsigned nr_cbufs;
	unsigned bound_cbufs_target_mask;
	unsigned nr_ps_color_outputs;
	unsigned ps_color_export_mask;
	unsigned image_rat_enabled_mask;
	unsigned buffer_rat_enabled_mask;
	bool multiwrite;
	bool dual_src_blend;
};

struct r600_clip_misc_state {
	struct r600_atom atom;
	unsigned pa_cl_clip_cntl;   /* from rasterizer    */
	unsigned pa_cl_vs_out_cntl; /* from vertex shader */
	unsigned clip_plane_enable; /* from rasterizer    */
	unsigned cc_dist_mask;      /* from vertex shader */
	unsigned clip_dist_write;   /* from vertex shader */
	unsigned cull_dist_write;   /* from vertex shader */
	bool clip_disable;       /* from vertex shader */
	bool vs_out_viewport;    /* from vertex shader */
};

struct r600_alphatest_state {
	struct r600_atom atom;
	unsigned sx_alpha_test_control; /* this comes from dsa state */
	unsigned sx_alpha_ref; /* this comes from dsa state */
	bool bypass;
	bool cb0_export_16bpc; /* from set_framebuffer_state */
};

struct r600_vgt_state {
	struct r600_atom atom;
	uint32_t vgt_multi_prim_ib_reset_en;
	uint32_t vgt_multi_prim_ib_reset_indx;
	uint32_t vgt_indx_offset;
	bool last_draw_was_indirect;
};

struct r600_blend_color {
	struct r600_atom atom;
	struct pipe_blend_color state;
};

struct r600_clip_state {
	struct r600_atom atom;
	struct pipe_clip_state state;
};

struct r600_cs_shader_state {
	struct r600_atom atom;
	unsigned kernel_index;
	unsigned pc;
	struct r600_pipe_compute *shader;
};

struct r600_framebuffer {
	struct r600_atom atom;
	struct pipe_framebuffer_state state;
	unsigned compressed_cb_mask;
	unsigned nr_samples;
	bool export_16bpc;
	bool cb0_is_integer;
	bool is_msaa_resolve;
	bool dual_src_blend;
	bool do_update_surf_dirtiness;
};

struct r600_sample_mask {
	struct r600_atom atom;
	uint16_t sample_mask; /* there are only 8 bits on EG, 16 bits on Cayman */
};

struct r600_config_state {
	struct r600_atom atom;
	unsigned sq_gpr_resource_mgmt_1;
	unsigned sq_gpr_resource_mgmt_2;
	unsigned sq_gpr_resource_mgmt_3;
	bool dyn_gpr_enabled;
};

struct r600_stencil_ref
{
	uint8_t ref_value[2];
	uint8_t valuemask[2];
	uint8_t writemask[2];
};

struct r600_stencil_ref_state {
	struct r600_atom atom;
	struct r600_stencil_ref state;
	struct pipe_stencil_ref pipe_state;
};

struct r600_shader_stages_state {
	struct r600_atom atom;
	unsigned geom_enable;
};

struct r600_gs_rings_state {
	struct r600_atom atom;
	unsigned enable;
	struct pipe_constant_buffer esgs_ring;
	struct pipe_constant_buffer gsvs_ring;
};

/* This must start from 16. */
/* features */
#define DBG_NO_CP_DMA		(1 << 30)

struct r600_screen {
	struct r600_common_screen	b;
	bool				has_msaa;
	bool				has_compressed_msaa_texturing;
	bool				has_atomics;

	/*for compute global memory binding, we allocate stuff here, instead of
	 * buffers.
	 * XXX: Not sure if this is the best place for global_pool.  Also,
	 * it's not thread safe, so it won't work with multiple contexts. */
	struct compute_memory_pool *global_pool;
};

struct r600_pipe_sampler_view {
	struct pipe_sampler_view	base;
	struct list_head		list;
	struct r600_resource		*tex_resource;
	uint32_t			tex_resource_words[8];
	bool				skip_mip_address_reloc;
	bool				is_stencil_sampler;
};

struct r600_rasterizer_state {
	struct r600_command_buffer	buffer;
	bool				flatshade;
	bool				two_side;
	unsigned			sprite_coord_enable;
	unsigned                        clip_plane_enable;
	unsigned			pa_sc_line_stipple;
	unsigned			pa_cl_clip_cntl;
	unsigned			pa_su_sc_mode_cntl;
	float				offset_units;
	float				offset_scale;
	bool				offset_enable;
	bool				offset_units_unscaled;
	bool				scissor_enable;
	bool				multisample_enable;
	bool				clip_halfz;
	bool				rasterizer_discard;
};

struct r600_poly_offset_state {
	struct r600_atom		atom;
	enum pipe_format		zs_format;
	float				offset_units;
	float				offset_scale;
	bool				offset_units_unscaled;
};

struct r600_blend_state {
	struct r600_command_buffer	buffer;
	struct r600_command_buffer	buffer_no_blend;
	unsigned			cb_target_mask;
	unsigned			cb_color_control;
	unsigned			cb_color_control_no_blend;
	bool				dual_src_blend;
	bool				alpha_to_one;
};

struct r600_dsa_state {
	struct r600_command_buffer	buffer;
	unsigned			alpha_ref;
	uint8_t				valuemask[2];
	uint8_t				writemask[2];
	unsigned			zwritemask;
	unsigned			sx_alpha_test_control;
};

struct r600_pipe_shader;

struct r600_pipe_shader_selector {
	struct r600_pipe_shader *current;

	struct tgsi_token       *tokens;
	struct nir_shader       *nir;

	size_t  nir_blob_size;
	void   *nir_blob;

	struct pipe_stream_output_info  so;
	struct tgsi_shader_info		info;

	unsigned	num_shaders;

	enum pipe_shader_type	type;
        enum pipe_shader_ir ir_type;

	/* geometry shader properties */
	enum mesa_prim	gs_output_prim;
	unsigned		gs_max_out_vertices;
	unsigned		gs_num_invocations;

	/* TCS/VS */
	uint64_t        lds_patch_outputs_written_mask;
	uint64_t        lds_outputs_written_mask;
};

struct r600_pipe_sampler_state {
	uint32_t			tex_sampler_words[3];
	union pipe_color_union		border_color;
	bool				border_color_use;
	bool				seamless_cube_map;
};

/* needed for blitter save */
#define NUM_TEX_UNITS 16

struct r600_seamless_cube_map {
	struct r600_atom		atom;
	bool				enabled;
};

struct r600_samplerview_state {
	struct r600_atom		atom;
	struct r600_pipe_sampler_view	*views[NUM_TEX_UNITS];
	uint32_t			enabled_mask;
	uint32_t			dirty_mask;
	uint32_t			compressed_depthtex_mask; /* which textures are depth */
	uint32_t			compressed_colortex_mask;
	bool				dirty_buffer_constants;
};

struct r600_sampler_states {
	struct r600_atom		atom;
	struct r600_pipe_sampler_state	*states[NUM_TEX_UNITS];
	uint32_t			enabled_mask;
	uint32_t			dirty_mask;
	uint32_t			has_bordercolor_mask; /* which states contain the border color */
};

struct r600_textures_info {
	struct r600_samplerview_state	views;
	struct r600_sampler_states	states;
	bool				is_array_sampler[NUM_TEX_UNITS];
};

struct r600_shader_driver_constants_info {
	/* currently 128 bytes for UCP/samplepos + sampler buffer constants */
	uint32_t			*constants;
	uint32_t			alloc_size;
	bool				texture_const_dirty;
	bool				vs_ucp_dirty;
	bool				ps_sample_pos_dirty;
	bool                            cs_block_grid_size_dirty;
	bool				tcs_default_levels_dirty;
};

struct r600_constbuf_state
{
	struct r600_atom		atom;
	struct pipe_constant_buffer	cb[PIPE_MAX_CONSTANT_BUFFERS];
	uint32_t			enabled_mask;
	uint32_t			dirty_mask;
};

struct r600_vertexbuf_state
{
	struct r600_atom		atom;
	struct pipe_vertex_buffer	vb[PIPE_MAX_ATTRIBS];
	uint32_t			enabled_mask; /* non-NULL buffers */
	uint32_t			dirty_mask;
};

/* CSO (constant state object, in other words, immutable state). */
struct r600_cso_state
{
	struct r600_atom atom;
	void *cso; /* e.g. r600_blend_state */
	struct r600_command_buffer *cb;
};

struct r600_fetch_shader {
	struct r600_resource		*buffer;
	unsigned			offset;
	uint32_t                        buffer_mask;
	unsigned                        strides[PIPE_MAX_ATTRIBS];
};

struct r600_shader_state {
	struct r600_atom		atom;
	struct r600_pipe_shader *shader;
};

struct r600_atomic_buffer_state {
	struct pipe_shader_buffer buffer[EG_MAX_ATOMIC_BUFFERS];
};

struct r600_image_view {
	struct pipe_image_view base;
	uint32_t cb_color_base;
	uint32_t cb_color_pitch;
	uint32_t cb_color_slice;
	uint32_t cb_color_view;
	uint32_t cb_color_info;
	uint32_t cb_color_attrib;
	uint32_t cb_color_dim;
	uint32_t cb_color_fmask;
	uint32_t cb_color_fmask_slice;
	uint32_t immed_resource_words[8];
	uint32_t resource_words[8];
	bool skip_mip_address_reloc;
	uint32_t buf_size;
};

struct r600_image_state {
	struct r600_atom atom;
	uint32_t                        enabled_mask;
	uint32_t                        dirty_mask;
	uint32_t			compressed_depthtex_mask;
	uint32_t			compressed_colortex_mask;
	bool				dirty_buffer_constants;
	struct r600_image_view views[R600_MAX_IMAGES];
};

/* Used to spill shader temps */
struct r600_scratch_buffer {
	struct r600_resource		*buffer;
	bool					dirty;
	unsigned				size;
	unsigned				item_size;
};

struct r600_context {
	struct r600_common_context	b;
	struct r600_screen		*screen;
	struct blitter_context		*blitter;
	struct u_suballocator		allocator_fetch_shader;

	/* Hardware info. */
	bool				has_vertex_cache;
	unsigned			default_gprs[EG_NUM_HW_STAGES];
	unsigned                        current_gprs[EG_NUM_HW_STAGES];
	unsigned			r6xx_num_clause_temp_gprs;

	/* Miscellaneous state objects. */
	void				*custom_dsa_flush;
	void				*custom_blend_resolve;
	void				*custom_blend_decompress;
	void                            *custom_blend_fastclear;
	/* With rasterizer discard, there doesn't have to be a pixel shader.
	 * In that case, we bind this one: */
	void				*dummy_pixel_shader;
	/* These dummy CMASK and FMASK buffers are used to get around the R6xx hardware
	 * bug where valid CMASK and FMASK are required to be present to avoid
	 * a hardlock in certain operations but aren't actually used
	 * for anything useful. */
	struct r600_resource		*dummy_fmask;
	struct r600_resource		*dummy_cmask;

	/* State binding slots are here. */
	struct r600_atom		*atoms[R600_NUM_ATOMS];
	/* Dirty atom bitmask for fast tests */
	uint64_t			dirty_atoms;
	/* States for CS initialization. */
	struct r600_command_buffer	start_cs_cmd; /* invariant state mostly */
	/** Compute specific registers initializations.  The start_cs_cmd atom
	 *  must be emitted before start_compute_cs_cmd. */
	struct r600_command_buffer      start_compute_cs_cmd;
	/* Register states. */
	struct r600_alphatest_state	alphatest_state;
	struct r600_cso_state		blend_state;
	struct r600_blend_color		blend_color;
	struct r600_cb_misc_state	cb_misc_state;
	struct r600_clip_misc_state	clip_misc_state;
	struct r600_clip_state		clip_state;
	struct r600_db_misc_state	db_misc_state;
	struct r600_db_state		db_state;
	struct r600_cso_state		dsa_state;
	struct r600_framebuffer		framebuffer;
	struct r600_poly_offset_state	poly_offset_state;
	struct r600_cso_state		rasterizer_state;
	struct r600_sample_mask		sample_mask;
	struct r600_seamless_cube_map	seamless_cube_map;
	struct r600_config_state	config_state;
	struct r600_stencil_ref_state	stencil_ref;
	struct r600_vgt_state		vgt_state;
	struct r600_atomic_buffer_state atomic_buffer_state;
	/* only have images on fragment shader */
	struct r600_image_state         fragment_images;
	struct r600_image_state         compute_images;
	struct r600_image_state         fragment_buffers;
	struct r600_image_state         compute_buffers;
	/* Shaders and shader resources. */
	struct r600_cso_state		vertex_fetch_shader;
	struct r600_shader_state        hw_shader_stages[EG_NUM_HW_STAGES];
	struct r600_cs_shader_state	cs_shader_state;
	struct r600_shader_stages_state shader_stages;
	struct r600_gs_rings_state	gs_rings;
	struct r600_constbuf_state	constbuf_state[PIPE_SHADER_TYPES];
	struct r600_textures_info	samplers[PIPE_SHADER_TYPES];

	struct r600_shader_driver_constants_info driver_consts[PIPE_SHADER_TYPES];

	/** Vertex buffers for fetch shaders */
	struct r600_vertexbuf_state	vertex_buffer_state;
	/** Vertex buffers for compute shaders */
	struct r600_vertexbuf_state	cs_vertex_buffer_state;

	/* Additional context states. */
	unsigned			compute_cb_target_mask;
	struct r600_pipe_shader_selector *ps_shader;
	struct r600_pipe_shader_selector *vs_shader;
	struct r600_pipe_shader_selector *gs_shader;

	struct r600_pipe_shader_selector *tcs_shader;
	struct r600_pipe_shader_selector *tes_shader;

	struct r600_pipe_shader_selector *fixed_func_tcs_shader;

	struct r600_rasterizer_state	*rasterizer;
	bool				alpha_to_one;
	bool				force_blend_disable;
	bool                            gs_tri_strip_adj_fix;
	bool				dual_src_blend;
	unsigned			zwritemask;
	unsigned			ps_iter_samples;

	/* The list of all texture buffer objects in this context.
	 * This list is walked when a buffer is invalidated/reallocated and
	 * the GPU addresses are updated. */
	struct list_head		texture_buffers;

	/* Last draw state (-1 = unset). */
	enum mesa_prim		last_primitive_type; /* Last primitive type used in draw_vbo. */
	enum mesa_prim		current_rast_prim; /* primitive type after TES, GS */
	enum mesa_prim		last_rast_prim;
	unsigned			last_start_instance;

	struct r600_isa		*isa;
	float sample_positions[4 * 16];
	float tess_state[8];
	uint32_t cs_block_grid_sizes[8]; /* 3 for grid + 1 pad, 3 for block  + 1 pad*/
	struct r600_pipe_shader_selector *last_ls;
	struct r600_pipe_shader_selector *last_tcs;
	unsigned last_num_tcs_input_cp;
	unsigned lds_alloc;

	struct r600_scratch_buffer scratch_buffers[MAX2(R600_NUM_HW_STAGES, EG_NUM_HW_STAGES)];

	/* Debug state. */
	bool			is_debug;
	struct radeon_saved_cs	last_gfx;
	struct r600_resource	*last_trace_buf;
	struct r600_resource	*trace_buf;
	unsigned		trace_id;

	uint8_t patch_vertices;
	bool cmd_buf_is_compute;
	struct pipe_resource *append_fence;
	uint32_t append_fence_id;
};

static inline void r600_emit_command_buffer(struct radeon_cmdbuf *cs,
					    struct r600_command_buffer *cb)
{
	assert(cs->current.cdw + cb->num_dw <= cs->current.max_dw);
	memcpy(cs->current.buf + cs->current.cdw, cb->buf, 4 * cb->num_dw);
	cs->current.cdw += cb->num_dw;
}

static inline void r600_set_atom_dirty(struct r600_context *rctx,
				       struct r600_atom *atom,
				       bool dirty)
{
	uint64_t mask;

	assert(atom->id != 0);
	assert(atom->id < sizeof(mask) * 8);
	mask = 1ull << atom->id;
	if (dirty)
		rctx->dirty_atoms |= mask;
	else
		rctx->dirty_atoms &= ~mask;
}

static inline void r600_mark_atom_dirty(struct r600_context *rctx,
					struct r600_atom *atom)
{
	r600_set_atom_dirty(rctx, atom, true);
}

static inline void r600_emit_atom(struct r600_context *rctx, struct r600_atom *atom)
{
	atom->emit(&rctx->b, atom);
	r600_set_atom_dirty(rctx, atom, false);
}

static inline void r600_set_cso_state(struct r600_context *rctx,
				      struct r600_cso_state *state, void *cso)
{
	state->cso = cso;
	r600_set_atom_dirty(rctx, &state->atom, cso != NULL);
}

static inline void r600_set_cso_state_with_cb(struct r600_context *rctx,
					      struct r600_cso_state *state, void *cso,
					      struct r600_command_buffer *cb)
{
	state->cb = cb;
	state->atom.num_dw = cb ? cb->num_dw : 0;
	r600_set_cso_state(rctx, state, cso);
}

/* compute_memory_pool.c */
struct compute_memory_pool;
void compute_memory_pool_delete(struct compute_memory_pool* pool);
struct compute_memory_pool* compute_memory_pool_new(
	struct r600_screen *rscreen);

/* evergreen_state.c */
struct pipe_sampler_view *
evergreen_create_sampler_view_custom(struct pipe_context *ctx,
				     struct pipe_resource *texture,
				     const struct pipe_sampler_view *state,
				     unsigned width0, unsigned height0,
				     unsigned force_level);
void evergreen_init_common_regs(struct r600_context *ctx,
				struct r600_command_buffer *cb,
				enum amd_gfx_level ctx_chip_class,
				enum radeon_family ctx_family,
				int ctx_drm_minor);
void cayman_init_common_regs(struct r600_command_buffer *cb,
			     enum amd_gfx_level ctx_chip_class,
			     enum radeon_family ctx_family,
			     int ctx_drm_minor);

void evergreen_init_state_functions(struct r600_context *rctx);
void evergreen_init_atom_start_cs(struct r600_context *rctx);
void evergreen_update_ps_state(struct pipe_context *ctx, struct r600_pipe_shader *shader);
void evergreen_update_es_state(struct pipe_context *ctx, struct r600_pipe_shader *shader);
void evergreen_update_gs_state(struct pipe_context *ctx, struct r600_pipe_shader *shader);
void evergreen_update_vs_state(struct pipe_context *ctx, struct r600_pipe_shader *shader);
void evergreen_update_ls_state(struct pipe_context *ctx, struct r600_pipe_shader *shader);
void evergreen_update_hs_state(struct pipe_context *ctx, struct r600_pipe_shader *shader);
void *evergreen_create_db_flush_dsa(struct r600_context *rctx);
void *evergreen_create_resolve_blend(struct r600_context *rctx);
void *evergreen_create_decompress_blend(struct r600_context *rctx);
void *evergreen_create_fastclear_blend(struct r600_context *rctx);
bool evergreen_is_format_supported(struct pipe_screen *screen,
				   enum pipe_format format,
				   enum pipe_texture_target target,
				   unsigned sample_count,
				   unsigned storage_sample_count,
				   unsigned usage);
void evergreen_init_color_surface(struct r600_context *rctx,
				  struct r600_surface *surf);
void evergreen_init_color_surface_rat(struct r600_context *rctx,
					struct r600_surface *surf);
void evergreen_update_db_shader_control(struct r600_context * rctx);
bool evergreen_adjust_gprs(struct r600_context *rctx);
void evergreen_setup_scratch_buffers(struct r600_context *rctx);
uint32_t evergreen_construct_rat_mask(struct r600_context *rctx, struct r600_cb_misc_state *a,
				      unsigned nr_cbufs);
/* r600_blit.c */
void r600_init_blit_functions(struct r600_context *rctx);
void r600_decompress_depth_textures(struct r600_context *rctx,
				    struct r600_samplerview_state *textures);
void r600_decompress_depth_images(struct r600_context *rctx,
				  struct r600_image_state *images);
void r600_decompress_color_textures(struct r600_context *rctx,
				    struct r600_samplerview_state *textures);
void r600_decompress_color_images(struct r600_context *rctx,
				  struct r600_image_state *images);
void r600_resource_copy_region(struct pipe_context *ctx,
			       struct pipe_resource *dst,
			       unsigned dst_level,
			       unsigned dstx, unsigned dsty, unsigned dstz,
			       struct pipe_resource *src,
			       unsigned src_level,
			       const struct pipe_box *src_box);

/* r600_shader.c */
int r600_pipe_shader_create(struct pipe_context *ctx,
			    struct r600_pipe_shader *shader,
			    union r600_shader_key key);

void r600_pipe_shader_destroy(struct pipe_context *ctx, struct r600_pipe_shader *shader);

/* r600_state.c */
struct pipe_sampler_view *
r600_create_sampler_view_custom(struct pipe_context *ctx,
				struct pipe_resource *texture,
				const struct pipe_sampler_view *state,
				unsigned width_first_level, unsigned height_first_level);
void r600_init_state_functions(struct r600_context *rctx);
void r600_init_atom_start_cs(struct r600_context *rctx);
void r600_update_ps_state(struct pipe_context *ctx, struct r600_pipe_shader *shader);
void r600_update_es_state(struct pipe_context *ctx, struct r600_pipe_shader *shader);
void r600_update_gs_state(struct pipe_context *ctx, struct r600_pipe_shader *shader);
void r600_update_vs_state(struct pipe_context *ctx, struct r600_pipe_shader *shader);
void *r600_create_db_flush_dsa(struct r600_context *rctx);
void *r600_create_resolve_blend(struct r600_context *rctx);
void *r700_create_resolve_blend(struct r600_context *rctx);
void *r600_create_decompress_blend(struct r600_context *rctx);
bool r600_adjust_gprs(struct r600_context *rctx);
bool r600_is_format_supported(struct pipe_screen *screen,
			      enum pipe_format format,
			      enum pipe_texture_target target,
			      unsigned sample_count,
			      unsigned storage_sample_count,
			      unsigned usage);
void r600_update_db_shader_control(struct r600_context * rctx);
void r600_setup_scratch_buffers(struct r600_context *rctx);

/* r600_hw_context.c */
void r600_context_gfx_flush(void *context, unsigned flags,
			    struct pipe_fence_handle **fence);
void r600_begin_new_cs(struct r600_context *ctx);
void r600_flush_emit(struct r600_context *ctx);
void r600_need_cs_space(struct r600_context *ctx, unsigned num_dw, bool count_draw_in, unsigned num_atomics);
void r600_emit_pfp_sync_me(struct r600_context *rctx);
void r600_cp_dma_copy_buffer(struct r600_context *rctx,
			     struct pipe_resource *dst, uint64_t dst_offset,
			     struct pipe_resource *src, uint64_t src_offset,
			     unsigned size);
void evergreen_cp_dma_clear_buffer(struct r600_context *rctx,
				   struct pipe_resource *dst, uint64_t offset,
				   unsigned size, uint32_t clear_value,
				   enum r600_coherency coher);
void r600_dma_copy_buffer(struct r600_context *rctx,
			  struct pipe_resource *dst,
			  struct pipe_resource *src,
			  uint64_t dst_offset,
			  uint64_t src_offset,
			  uint64_t size);

/*
 * evergreen_hw_context.c
 */
void evergreen_dma_copy_buffer(struct r600_context *rctx,
			       struct pipe_resource *dst,
			       struct pipe_resource *src,
			       uint64_t dst_offset,
			       uint64_t src_offset,
			       uint64_t size);
void evergreen_setup_tess_constants(struct r600_context *rctx,
				    const struct pipe_draw_info *info,
				    unsigned *num_patches);
uint32_t evergreen_get_ls_hs_config(struct r600_context *rctx,
				    const struct pipe_draw_info *info,
				    unsigned num_patches);
void evergreen_set_ls_hs_config(struct r600_context *rctx,
				struct radeon_cmdbuf *cs,
				uint32_t ls_hs_config);
void evergreen_set_lds_alloc(struct r600_context *rctx,
			     struct radeon_cmdbuf *cs,
			     uint32_t lds_alloc);

/* r600_state_common.c */
void r600_init_common_state_functions(struct r600_context *rctx);
void r600_emit_cso_state(struct r600_context *rctx, struct r600_atom *atom);
void r600_emit_alphatest_state(struct r600_context *rctx, struct r600_atom *atom);
void r600_emit_blend_color(struct r600_context *rctx, struct r600_atom *atom);
void r600_emit_vgt_state(struct r600_context *rctx, struct r600_atom *atom);
void r600_emit_clip_misc_state(struct r600_context *rctx, struct r600_atom *atom);
void r600_emit_stencil_ref(struct r600_context *rctx, struct r600_atom *atom);
void r600_emit_shader(struct r600_context *rctx, struct r600_atom *a);
void r600_add_atom(struct r600_context *rctx, struct r600_atom *atom, unsigned id);
void r600_init_atom(struct r600_context *rctx, struct r600_atom *atom, unsigned id,
		    void (*emit)(struct r600_context *ctx, struct r600_atom *state),
		    unsigned num_dw);
void r600_vertex_buffers_dirty(struct r600_context *rctx);
void r600_sampler_views_dirty(struct r600_context *rctx,
			      struct r600_samplerview_state *state);
void r600_sampler_states_dirty(struct r600_context *rctx,
			       struct r600_sampler_states *state);
void r600_constant_buffers_dirty(struct r600_context *rctx, struct r600_constbuf_state *state);
void r600_set_sample_locations_constant_buffer(struct r600_context *rctx);
void r600_setup_scratch_area_for_shader(struct r600_context *rctx,
	struct r600_pipe_shader *shader, struct r600_scratch_buffer *scratch,
	unsigned ring_base_reg, unsigned item_size_reg, unsigned ring_size_reg);
uint32_t r600_translate_stencil_op(int s_op);
uint32_t r600_translate_fill(uint32_t func);
unsigned r600_tex_wrap(unsigned wrap);
unsigned r600_tex_mipfilter(unsigned filter);
unsigned r600_tex_compare(unsigned compare);
bool sampler_state_needs_border_color(const struct pipe_sampler_state *state);
unsigned r600_get_swizzle_combined(const unsigned char *swizzle_format,
				   const unsigned char *swizzle_view,
				   bool vtx);
uint32_t r600_translate_texformat(struct pipe_screen *screen, enum pipe_format format,
				  const unsigned char *swizzle_view,
				  uint32_t *word4_p, uint32_t *yuv_format_p,
				  bool do_endian_swap);
uint32_t r600_translate_colorformat(enum amd_gfx_level chip, enum pipe_format format,
				  bool do_endian_swap);
uint32_t r600_colorformat_endian_swap(uint32_t colorformat, bool do_endian_swap);

/* r600_uvd.c */
struct pipe_video_codec *r600_uvd_create_decoder(struct pipe_context *context,
						   const struct pipe_video_codec *decoder);

struct pipe_video_buffer *r600_video_buffer_create(struct pipe_context *pipe,
						   const struct pipe_video_buffer *tmpl);

/*
 * Helpers for building command buffers
 */

#define PKT3_SET_CONFIG_REG	0x68
#define PKT3_SET_CONTEXT_REG	0x69
#define PKT3_SET_CTL_CONST      0x6F
#define PKT3_SET_LOOP_CONST                    0x6C

#define R600_CONFIG_REG_OFFSET	0x08000
#define R600_CONTEXT_REG_OFFSET 0x28000
#define R600_CTL_CONST_OFFSET   0x3CFF0
#define R600_LOOP_CONST_OFFSET                 0X0003E200
#define EG_LOOP_CONST_OFFSET               0x0003A200

#define PKT_TYPE_S(x)                   (((unsigned)(x) & 0x3) << 30)
#define PKT_COUNT_S(x)                  (((unsigned)(x) & 0x3FFF) << 16)
#define PKT3_IT_OPCODE_S(x)             (((unsigned)(x) & 0xFF) << 8)
#define PKT3_PREDICATE(x)               (((x) >> 0) & 0x1)
#define PKT3(op, count, predicate) (PKT_TYPE_S(3) | PKT_COUNT_S(count) | PKT3_IT_OPCODE_S(op) | PKT3_PREDICATE(predicate))

#define RADEON_CP_PACKET3_COMPUTE_MODE 0x00000002

/*Evergreen Compute packet3*/
#define PKT3C(op, count, predicate) (PKT_TYPE_S(3) | PKT3_IT_OPCODE_S(op) | PKT_COUNT_S(count) | PKT3_PREDICATE(predicate) | RADEON_CP_PACKET3_COMPUTE_MODE)

static inline void r600_store_value(struct r600_command_buffer *cb, unsigned value)
{
	cb->buf[cb->num_dw++] = value;
}

static inline void r600_store_array(struct r600_command_buffer *cb, unsigned num, unsigned *ptr)
{
	assert(cb->num_dw+num <= cb->max_num_dw);
	memcpy(&cb->buf[cb->num_dw], ptr, num * sizeof(ptr[0]));
	cb->num_dw += num;
}

static inline void r600_store_config_reg_seq(struct r600_command_buffer *cb, unsigned reg, unsigned num)
{
	assert(reg < R600_CONTEXT_REG_OFFSET);
	assert(cb->num_dw+2+num <= cb->max_num_dw);
	cb->buf[cb->num_dw++] = PKT3(PKT3_SET_CONFIG_REG, num, 0);
	cb->buf[cb->num_dw++] = (reg - R600_CONFIG_REG_OFFSET) >> 2;
}

/**
 * Needs cb->pkt_flags set to  RADEON_CP_PACKET3_COMPUTE_MODE for compute
 * shaders.
 */
static inline void r600_store_context_reg_seq(struct r600_command_buffer *cb, unsigned reg, unsigned num)
{
	assert(reg >= R600_CONTEXT_REG_OFFSET && reg < R600_CTL_CONST_OFFSET);
	assert(cb->num_dw+2+num <= cb->max_num_dw);
	cb->buf[cb->num_dw++] = PKT3(PKT3_SET_CONTEXT_REG, num, 0) | cb->pkt_flags;
	cb->buf[cb->num_dw++] = (reg - R600_CONTEXT_REG_OFFSET) >> 2;
}

/**
 * Needs cb->pkt_flags set to  RADEON_CP_PACKET3_COMPUTE_MODE for compute
 * shaders.
 */
static inline void r600_store_ctl_const_seq(struct r600_command_buffer *cb, unsigned reg, unsigned num)
{
	assert(reg >= R600_CTL_CONST_OFFSET);
	assert(cb->num_dw+2+num <= cb->max_num_dw);
	cb->buf[cb->num_dw++] = PKT3(PKT3_SET_CTL_CONST, num, 0) | cb->pkt_flags;
	cb->buf[cb->num_dw++] = (reg - R600_CTL_CONST_OFFSET) >> 2;
}

static inline void r600_store_loop_const_seq(struct r600_command_buffer *cb, unsigned reg, unsigned num)
{
	assert(reg >= R600_LOOP_CONST_OFFSET);
	assert(cb->num_dw+2+num <= cb->max_num_dw);
	cb->buf[cb->num_dw++] = PKT3(PKT3_SET_LOOP_CONST, num, 0);
	cb->buf[cb->num_dw++] = (reg - R600_LOOP_CONST_OFFSET) >> 2;
}

/**
 * Needs cb->pkt_flags set to  RADEON_CP_PACKET3_COMPUTE_MODE for compute
 * shaders.
 */
static inline void eg_store_loop_const_seq(struct r600_command_buffer *cb, unsigned reg, unsigned num)
{
	assert(reg >= EG_LOOP_CONST_OFFSET);
	assert(cb->num_dw+2+num <= cb->max_num_dw);
	cb->buf[cb->num_dw++] = PKT3(PKT3_SET_LOOP_CONST, num, 0) | cb->pkt_flags;
	cb->buf[cb->num_dw++] = (reg - EG_LOOP_CONST_OFFSET) >> 2;
}

static inline void r600_store_config_reg(struct r600_command_buffer *cb, unsigned reg, unsigned value)
{
	r600_store_config_reg_seq(cb, reg, 1);
	r600_store_value(cb, value);
}

static inline void r600_store_context_reg(struct r600_command_buffer *cb, unsigned reg, unsigned value)
{
	r600_store_context_reg_seq(cb, reg, 1);
	r600_store_value(cb, value);
}

static inline void r600_store_ctl_const(struct r600_command_buffer *cb, unsigned reg, unsigned value)
{
	r600_store_ctl_const_seq(cb, reg, 1);
	r600_store_value(cb, value);
}

static inline void r600_store_loop_const(struct r600_command_buffer *cb, unsigned reg, unsigned value)
{
	r600_store_loop_const_seq(cb, reg, 1);
	r600_store_value(cb, value);
}

static inline void eg_store_loop_const(struct r600_command_buffer *cb, unsigned reg, unsigned value)
{
	eg_store_loop_const_seq(cb, reg, 1);
	r600_store_value(cb, value);
}

void r600_init_command_buffer(struct r600_command_buffer *cb, unsigned num_dw);
void r600_release_command_buffer(struct r600_command_buffer *cb);

static inline void radeon_compute_set_context_reg_seq(struct radeon_cmdbuf *cs, unsigned reg, unsigned num)
{
	radeon_set_context_reg_seq(cs, reg, num);
	/* Set the compute bit on the packet header */
	cs->current.buf[cs->current.cdw - 2] |= RADEON_CP_PACKET3_COMPUTE_MODE;
}

static inline void radeon_set_ctl_const_seq(struct radeon_cmdbuf *cs, unsigned reg, unsigned num)
{
	assert(reg >= R600_CTL_CONST_OFFSET);
	assert(cs->current.cdw + 2 + num <= cs->current.max_dw);
	radeon_emit(cs, PKT3(PKT3_SET_CTL_CONST, num, 0));
	radeon_emit(cs, (reg - R600_CTL_CONST_OFFSET) >> 2);
}

static inline void radeon_compute_set_context_reg(struct radeon_cmdbuf *cs, unsigned reg, unsigned value)
{
	radeon_compute_set_context_reg_seq(cs, reg, 1);
	radeon_emit(cs, value);
}

static inline void radeon_set_context_reg_flag(struct radeon_cmdbuf *cs, unsigned reg, unsigned value, unsigned flag)
{
	if (flag & RADEON_CP_PACKET3_COMPUTE_MODE) {
		radeon_compute_set_context_reg(cs, reg, value);
	} else {
		radeon_set_context_reg(cs, reg, value);
	}
}

static inline void radeon_set_ctl_const(struct radeon_cmdbuf *cs, unsigned reg, unsigned value)
{
	radeon_set_ctl_const_seq(cs, reg, 1);
	radeon_emit(cs, value);
}

/*
 * common helpers
 */

/* 12.4 fixed-point */
static inline unsigned r600_pack_float_12p4(float x)
{
	return x <= 0    ? 0 :
	       x >= 4096 ? 0xffff : x * 16;
}

static inline unsigned r600_get_flush_flags(enum r600_coherency coher)
{
	switch (coher) {
	default:
	case R600_COHERENCY_NONE:
		return 0;
	case R600_COHERENCY_SHADER:
		return R600_CONTEXT_INV_CONST_CACHE |
		       R600_CONTEXT_INV_VERTEX_CACHE |
		       R600_CONTEXT_INV_TEX_CACHE |
		       R600_CONTEXT_STREAMOUT_FLUSH;
	case R600_COHERENCY_CB_META:
		return R600_CONTEXT_FLUSH_AND_INV_CB |
		       R600_CONTEXT_FLUSH_AND_INV_CB_META;
	}
}

#define     V_028A6C_OUTPRIM_TYPE_POINTLIST            0
#define     V_028A6C_OUTPRIM_TYPE_LINESTRIP            1
#define     V_028A6C_OUTPRIM_TYPE_TRISTRIP             2

unsigned r600_conv_prim_to_gs_out(unsigned mode);

void eg_trace_emit(struct r600_context *rctx);
void eg_dump_debug_state(struct pipe_context *ctx, FILE *f,
			 unsigned flags);

struct r600_pipe_shader_selector *r600_create_shader_state_tokens(struct pipe_context *ctx,
								  const void *tokens,
								  enum pipe_shader_ir,
								  unsigned pipe_shader_type);
int r600_shader_select(struct pipe_context *ctx,
		       struct r600_pipe_shader_selector* sel,
		       bool *dirty, bool precompile);

void r600_delete_shader_selector(struct pipe_context *ctx,
				 struct r600_pipe_shader_selector *sel);

struct r600_shader_atomic;
void evergreen_emit_atomic_buffer_setup_count(struct r600_context *rctx,
					      struct r600_pipe_shader *cs_shader,
					      struct r600_shader_atomic *combined_atomics,
					      uint8_t *atomic_used_mask_p);
void evergreen_emit_atomic_buffer_setup(struct r600_context *rctx,
					bool is_compute,
					struct r600_shader_atomic *combined_atomics,
					uint8_t atomic_used_mask);
void evergreen_emit_atomic_buffer_save(struct r600_context *rctx,
				       bool is_compute,
				       struct r600_shader_atomic *combined_atomics,
				       uint8_t *atomic_used_mask_p);
void r600_update_compressed_resource_state(struct r600_context *rctx, bool compute_only);

void eg_setup_buffer_constants(struct r600_context *rctx, int shader_type);
void r600_update_driver_const_buffers(struct r600_context *rctx, bool compute_only);
#endif
