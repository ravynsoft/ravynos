/*
 * Copyright 2013 Advanced Micro Devices, Inc.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors: Marek Olšák <maraeo@gmail.com>
 *
 */

/**
 * This file contains common screen and context structures and functions
 * for r600g and radeonsi.
 */

#ifndef R600_PIPE_COMMON_H
#define R600_PIPE_COMMON_H

#include <stdio.h>

#include "winsys/radeon_winsys.h"

#include "util/disk_cache.h"
#include "util/u_blitter.h"
#include "util/list.h"
#include "util/u_range.h"
#include "util/slab.h"
#include "util/u_suballoc.h"
#include "util/u_transfer.h"
#include "util/u_threaded_context.h"

#include "compiler/nir/nir.h"

struct u_log_context;
#define ATI_VENDOR_ID 0x1002

#define R600_RESOURCE_FLAG_TRANSFER		(PIPE_RESOURCE_FLAG_DRV_PRIV << 0)
#define R600_RESOURCE_FLAG_FLUSHED_DEPTH	(PIPE_RESOURCE_FLAG_DRV_PRIV << 1)
#define R600_RESOURCE_FLAG_FORCE_TILING		(PIPE_RESOURCE_FLAG_DRV_PRIV << 2)
#define R600_RESOURCE_FLAG_UNMAPPABLE		(PIPE_RESOURCE_FLAG_DRV_PRIV << 4)

#define R600_CONTEXT_STREAMOUT_FLUSH		(1u << 0)
/* Pipeline & streamout query controls. */
#define R600_CONTEXT_START_PIPELINE_STATS	(1u << 1)
#define R600_CONTEXT_STOP_PIPELINE_STATS	(1u << 2)
#define R600_CONTEXT_FLUSH_FOR_RENDER_COND	(1u << 3)
#define R600_CONTEXT_PRIVATE_FLAG		(1u << 4)

/* special primitive types */
#define R600_PRIM_RECTANGLE_LIST	MESA_PRIM_COUNT

#define R600_NOT_QUERY		0xffffffff

/* Debug flags. */
#define DBG_VS			(1 << PIPE_SHADER_VERTEX)
#define DBG_PS			(1 << PIPE_SHADER_FRAGMENT)
#define DBG_GS			(1 << PIPE_SHADER_GEOMETRY)
#define DBG_TCS			(1 << PIPE_SHADER_TESS_CTRL)
#define DBG_TES			(1 << PIPE_SHADER_TESS_EVAL)
#define DBG_CS			(1 << PIPE_SHADER_COMPUTE)
#define DBG_ALL_SHADERS		(DBG_FS - 1)
#define DBG_FS			(1 << 6) /* fetch shader */
#define DBG_TEX			(1 << 7)
#define DBG_NIR			(1 << 8)
#define DBG_COMPUTE		(1 << 9)
/* gap */
#define DBG_VM			(1 << 11)
#define DBG_PREOPT_IR		(1 << 15)
#define DBG_CHECK_IR		(1 << 16)
/* gaps */
#define DBG_TEST_DMA		(1 << 20)
/* Bits 21-31 are reserved for the r600g driver. */
/* features */
#define DBG_NO_ASYNC_DMA	(1ull << 32)
#define DBG_NO_HYPERZ		(1ull << 33)
#define DBG_NO_DISCARD_RANGE	(1ull << 34)
#define DBG_NO_2D_TILING	(1ull << 35)
#define DBG_NO_TILING		(1ull << 36)
#define DBG_SWITCH_ON_EOP	(1ull << 37)
#define DBG_FORCE_DMA		(1ull << 38)
#define DBG_INFO		(1ull << 40)
#define DBG_NO_WC		(1ull << 41)
#define DBG_CHECK_VM		(1ull << 42)
/* gap */
#define DBG_TEST_VMFAULT_CP	(1ull << 51)
#define DBG_TEST_VMFAULT_SDMA	(1ull << 52)
#define DBG_TEST_VMFAULT_SHADER	(1ull << 53)

#define R600_MAP_BUFFER_ALIGNMENT 64
#define R600_MAX_VIEWPORTS        16

#define SI_MAX_VARIABLE_THREADS_PER_BLOCK 1024

enum r600_coherency {
	R600_COHERENCY_NONE, /* no cache flushes needed */
	R600_COHERENCY_SHADER,
	R600_COHERENCY_CB_META,
};

struct r600_common_context;
struct r600_perfcounters;
struct tgsi_shader_info;
struct r600_qbo_state;

/* Only 32-bit buffer allocations are supported, gallium doesn't support more
 * at the moment.
 */
struct r600_resource {
	struct threaded_resource	b;

	/* Winsys objects. */
	struct pb_buffer_lean		*buf;
	uint64_t			gpu_address;
	/* Memory usage if the buffer placement is optimal. */
	uint64_t			vram_usage;
	uint64_t			gart_usage;

	/* Resource properties. */
	uint64_t			bo_size;
	unsigned			bo_alignment;
	enum radeon_bo_domain		domains;
	enum radeon_bo_flag		flags;
	unsigned			bind_history;

	/* The buffer range which is initialized (with a write transfer,
	 * streamout, DMA, or as a random access target). The rest of
	 * the buffer is considered invalid and can be mapped unsynchronized.
	 *
	 * This allows unsynchronized mapping of a buffer range which hasn't
	 * been used yet. It's for applications which forget to use
	 * the unsynchronized map flag and expect the driver to figure it out.
         */
	struct util_range		valid_buffer_range;

	/* Whether the resource has been exported via resource_get_handle. */
	unsigned			external_usage; /* PIPE_HANDLE_USAGE_* */

	/* Whether this resource is referenced by bindless handles. */
	bool				texture_handle_allocated;
	bool				image_handle_allocated;
	bool                            compute_global_bo;

	/*
	 * EG/Cayman only - for RAT operations hw need an immediate buffer
	 * to store results in.
	 */
	struct r600_resource            *immed_buffer;
};

struct r600_transfer {
	struct threaded_transfer	b;
	struct r600_resource		*staging;
};

struct r600_fmask_info {
	uint64_t offset;
	uint64_t size;
	unsigned alignment;
	unsigned pitch_in_pixels;
	unsigned bank_height;
	unsigned slice_tile_max;
	unsigned tile_mode_index;
	unsigned tile_swizzle;
};

struct r600_cmask_info {
	uint64_t offset;
	uint64_t size;
	unsigned alignment;
	unsigned slice_tile_max;
	uint64_t base_address_reg;
};

struct r600_texture {
	struct r600_resource		resource;

	uint64_t			size;
	unsigned			num_level0_transfers;
	enum pipe_format		db_render_format;
	bool				is_depth;
	bool				db_compatible;
	bool				can_sample_z;
	bool				can_sample_s;
	unsigned			dirty_level_mask; /* each bit says if that mipmap is compressed */
	unsigned			stencil_dirty_level_mask; /* each bit says if that mipmap is compressed */
	struct r600_texture		*flushed_depth_texture;
	struct radeon_surf		surface;

	/* Colorbuffer compression and fast clear. */
	struct r600_fmask_info		fmask;
	struct r600_cmask_info		cmask;
	struct r600_resource		*cmask_buffer;
	unsigned			cb_color_info; /* fast clear enable bit */
	unsigned			color_clear_value[2];
	unsigned			last_msaa_resolve_target_micro_mode;

	/* Depth buffer compression and fast clear. */
	uint64_t			htile_offset;
	bool				depth_cleared; /* if it was cleared at least once */
	float				depth_clear_value;
	bool				stencil_cleared; /* if it was cleared at least once */
	uint8_t				stencil_clear_value;

	bool				non_disp_tiling; /* R600-Cayman only */

	/* Counter that should be non-zero if the texture is bound to a
	 * framebuffer. Implemented in radeonsi only.
	 */
	uint32_t			framebuffers_bound;
};

struct r600_surface {
	struct pipe_surface		base;

	/* These can vary with block-compressed textures. */
	unsigned width0;
	unsigned height0;

	bool color_initialized;
	bool depth_initialized;

	/* Misc. color flags. */
	bool alphatest_bypass;
	bool export_16bpc;
	bool color_is_int8;
	bool color_is_int10;

	/* Color registers. */
	unsigned cb_color_info;
	unsigned cb_color_base;
	unsigned cb_color_view;
	unsigned cb_color_size;		/* R600 only */
	unsigned cb_color_dim;		/* EG only */
	unsigned cb_color_pitch;	/* EG and later */
	unsigned cb_color_slice;	/* EG and later */
	unsigned cb_color_attrib;	/* EG and later */
	unsigned cb_color_fmask;	/* CB_COLORn_FMASK (EG and later) or CB_COLORn_FRAG (r600) */
	unsigned cb_color_fmask_slice;	/* EG and later */
	unsigned cb_color_cmask;	/* CB_COLORn_TILE (r600 only) */
	unsigned cb_color_mask;		/* R600 only */
	struct r600_resource *cb_buffer_fmask; /* Used for FMASK relocations. R600 only */
	struct r600_resource *cb_buffer_cmask; /* Used for CMASK relocations. R600 only */

	/* DB registers. */
	uint64_t db_depth_base;		/* DB_Z_READ/WRITE_BASE (EG and later) or DB_DEPTH_BASE (r600) */
	uint64_t db_stencil_base;	/* EG and later */
	uint64_t db_htile_data_base;
	unsigned db_depth_info;		/* R600 only, then SI and later */
	unsigned db_z_info;		/* EG and later */
	unsigned db_depth_view;
	unsigned db_depth_size;
	unsigned db_depth_slice;	/* EG and later */
	unsigned db_stencil_info;	/* EG and later */
	unsigned db_prefetch_limit;	/* R600 only */
	unsigned db_htile_surface;
	unsigned db_preload_control;	/* EG and later */
};

struct r600_mmio_counter {
	unsigned busy;
	unsigned idle;
};

union r600_mmio_counters {
	struct r600_mmio_counters_named {
		/* For global GPU load including SDMA. */
		struct r600_mmio_counter gpu;

		/* GRBM_STATUS */
		struct r600_mmio_counter spi;
		struct r600_mmio_counter gui;
		struct r600_mmio_counter ta;
		struct r600_mmio_counter gds;
		struct r600_mmio_counter vgt;
		struct r600_mmio_counter ia;
		struct r600_mmio_counter sx;
		struct r600_mmio_counter wd;
		struct r600_mmio_counter bci;
		struct r600_mmio_counter sc;
		struct r600_mmio_counter pa;
		struct r600_mmio_counter db;
		struct r600_mmio_counter cp;
		struct r600_mmio_counter cb;

		/* SRBM_STATUS2 */
		struct r600_mmio_counter sdma;

		/* CP_STAT */
		struct r600_mmio_counter pfp;
		struct r600_mmio_counter meq;
		struct r600_mmio_counter me;
		struct r600_mmio_counter surf_sync;
		struct r600_mmio_counter cp_dma;
		struct r600_mmio_counter scratch_ram;
	} named;
	unsigned array[sizeof(struct r600_mmio_counters_named) / sizeof(unsigned)];
};

struct r600_memory_object {
	struct pipe_memory_object	b;
	struct pb_buffer_lean		*buf;
	uint32_t			stride;
	uint32_t			offset;
};

struct r600_common_screen {
	struct pipe_screen		b;
	struct radeon_winsys		*ws;
	enum radeon_family		family;
	enum amd_gfx_level			gfx_level;
	struct radeon_info		info;
	uint64_t			debug_flags;
	bool				has_cp_dma;
	bool				has_streamout;

	struct disk_cache		*disk_shader_cache;

	struct slab_parent_pool		pool_transfers;

	/* Texture filter settings. */
	int				force_aniso; /* -1 = disabled */

	/* Auxiliary context. Mainly used to initialize resources.
	 * It must be locked prior to using and flushed before unlocking. */
	struct pipe_context		*aux_context;
	mtx_t				aux_context_lock;

	/* This must be in the screen, because UE4 uses one context for
	 * compilation and another one for rendering.
	 */
	unsigned			num_compilations;
	/* Along with ST_DEBUG=precompile, this should show if applications
	 * are loading shaders on demand. This is a monotonic counter.
	 */
	unsigned			num_shaders_created;
	unsigned			num_shader_cache_hits;

	/* GPU load thread. */
	mtx_t				gpu_load_mutex;
	thrd_t				gpu_load_thread;
	bool				gpu_load_thread_created;
	union r600_mmio_counters	mmio_counters;
	volatile unsigned		gpu_load_stop_thread; /* bool */

	char				renderer_string[100];

	/* Performance counters. */
	struct r600_perfcounters	*perfcounters;

	/* If pipe_screen wants to recompute and re-emit the framebuffer,
	 * sampler, and image states of all contexts, it should atomically
	 * increment this.
	 *
	 * Each context will compare this with its own last known value of
	 * the counter before drawing and re-emit the states accordingly.
	 */
	unsigned			dirty_tex_counter;

	/* Atomically increment this counter when an existing texture's
	 * metadata is enabled or disabled in a way that requires changing
	 * contexts' compressed texture binding masks.
	 */
	unsigned			compressed_colortex_counter;

	struct {
		/* Context flags to set so that all writes from earlier jobs
		 * in the CP are seen by L2 clients.
		 */
		unsigned cp_to_L2;

		/* Context flags to set so that all writes from earlier jobs
		 * that end in L2 are seen by CP.
		 */
		unsigned L2_to_cp;

		/* Context flags to set so that all writes from earlier
		 * compute jobs are seen by L2 clients.
		 */
		unsigned compute_to_L2;
	} barrier_flags;

	struct nir_shader_compiler_options nir_options;
	struct nir_shader_compiler_options nir_options_fs;
};

/* This encapsulates a state or an operation which can emitted into the GPU
 * command stream. */
struct r600_atom {
	void (*emit)(struct r600_common_context *ctx, struct r600_atom *state);
	unsigned		num_dw;
	unsigned short		id;
};

struct r600_so_target {
	struct pipe_stream_output_target b;

	/* The buffer where BUFFER_FILLED_SIZE is stored. */
	struct r600_resource	*buf_filled_size;
	unsigned		buf_filled_size_offset;
	bool			buf_filled_size_valid;

	unsigned		stride_in_dw;
};

struct r600_streamout {
	struct r600_atom		begin_atom;
	bool				begin_emitted;
	unsigned			num_dw_for_end;

	unsigned			enabled_mask;
	unsigned			num_targets;
	struct r600_so_target		*targets[PIPE_MAX_SO_BUFFERS];

	unsigned			append_bitmask;
	bool				suspended;

	/* External state which comes from the vertex shader,
	 * it must be set explicitly when binding a shader. */
	uint16_t			*stride_in_dw;
	unsigned			enabled_stream_buffers_mask; /* stream0 buffers0-3 in 4 LSB */

	/* The state of VGT_STRMOUT_BUFFER_(CONFIG|EN). */
	unsigned			hw_enabled_mask;

	/* The state of VGT_STRMOUT_(CONFIG|EN). */
	struct r600_atom		enable_atom;
	bool				streamout_enabled;
	bool				prims_gen_query_enabled;
	int				num_prims_gen_queries;
};

struct r600_signed_scissor {
	int minx;
	int miny;
	int maxx;
	int maxy;
};

struct r600_scissors {
	struct r600_atom		atom;
	unsigned			dirty_mask;
	struct pipe_scissor_state	states[R600_MAX_VIEWPORTS];
};

struct r600_viewports {
	struct r600_atom		atom;
	unsigned			dirty_mask;
	unsigned			depth_range_dirty_mask;
	struct pipe_viewport_state	states[R600_MAX_VIEWPORTS];
	struct r600_signed_scissor	as_scissor[R600_MAX_VIEWPORTS];
};

struct r600_ring {
	struct radeon_cmdbuf		cs;
	void (*flush)(void *ctx, unsigned flags,
		      struct pipe_fence_handle **fence);
};

/* Saved CS data for debugging features. */
struct radeon_saved_cs {
	uint32_t			*ib;
	unsigned			num_dw;

	struct radeon_bo_list_item	*bo_list;
	unsigned			bo_count;
};

struct r600_common_context {
	struct pipe_context b; /* base class */

	struct r600_common_screen	*screen;
	struct radeon_winsys		*ws;
	struct radeon_winsys_ctx	*ctx;
	enum radeon_family		family;
	enum amd_gfx_level			gfx_level;
	struct r600_ring		gfx;
	struct r600_ring		dma;
	struct pipe_fence_handle	*last_gfx_fence;
	struct pipe_fence_handle	*last_sdma_fence;
	struct r600_resource		*eop_bug_scratch;
	unsigned			num_gfx_cs_flushes;
	unsigned			initial_gfx_cs_size;
	unsigned			last_dirty_tex_counter;
	unsigned			last_compressed_colortex_counter;
	unsigned			last_num_draw_calls;

	struct threaded_context		*tc;
	struct u_suballocator		allocator_zeroed_memory;
	struct slab_child_pool		pool_transfers;
	struct slab_child_pool		pool_transfers_unsync; /* for threaded_context */

	/* Current unaccounted memory usage. */
	uint64_t			vram;
	uint64_t			gtt;

	/* States. */
	struct r600_streamout		streamout;
	struct r600_scissors		scissors;
	struct r600_viewports		viewports;
	bool				scissor_enabled;
	bool				clip_halfz;
	bool				vs_writes_viewport_index;
	bool				vs_disables_clipping_viewport;

	/* Additional context states. */
	unsigned flags; /* flush flags */

	/* Queries. */
	/* Maintain the list of active queries for pausing between IBs. */
	int				num_occlusion_queries;
	int				num_perfect_occlusion_queries;
	struct list_head		active_queries;
	unsigned			num_cs_dw_queries_suspend;
	/* Misc stats. */
	unsigned			num_draw_calls;
	unsigned			num_decompress_calls;
	unsigned			num_mrt_draw_calls;
	unsigned			num_prim_restart_calls;
	unsigned			num_spill_draw_calls;
	unsigned			num_compute_calls;
	unsigned			num_spill_compute_calls;
	unsigned			num_dma_calls;
	unsigned			num_cp_dma_calls;
	unsigned			num_vs_flushes;
	unsigned			num_ps_flushes;
	unsigned			num_cs_flushes;
	unsigned			num_cb_cache_flushes;
	unsigned			num_db_cache_flushes;
	unsigned			num_resident_handles;
	uint64_t			num_alloc_tex_transfer_bytes;

	/* Render condition. */
	struct r600_atom		render_cond_atom;
	struct pipe_query		*render_cond;
	unsigned			render_cond_mode;
	bool				render_cond_invert;
	bool				render_cond_force_off; /* for u_blitter */

	/* MSAA sample locations.
	 * The first index is the sample index.
	 * The second index is the coordinate: X, Y. */
	float				sample_locations_1x[1][2];
	float				sample_locations_2x[2][2];
	float				sample_locations_4x[4][2];
	float				sample_locations_8x[8][2];
	float				sample_locations_16x[16][2];

	struct util_debug_callback	debug;
	struct pipe_device_reset_callback device_reset_callback;
	struct u_log_context		*log;

	void				*query_result_shader;

	/* Copy one resource to another using async DMA. */
	void (*dma_copy)(struct pipe_context *ctx,
			 struct pipe_resource *dst,
			 unsigned dst_level,
			 unsigned dst_x, unsigned dst_y, unsigned dst_z,
			 struct pipe_resource *src,
			 unsigned src_level,
			 const struct pipe_box *src_box);

	void (*dma_clear_buffer)(struct pipe_context *ctx, struct pipe_resource *dst,
				 uint64_t offset, uint64_t size, unsigned value);

	void (*clear_buffer)(struct pipe_context *ctx, struct pipe_resource *dst,
			     uint64_t offset, uint64_t size, unsigned value,
			     enum r600_coherency coher);

	void (*blit_decompress_depth)(struct pipe_context *ctx,
				      struct r600_texture *texture,
				      struct r600_texture *staging,
				      unsigned first_level, unsigned last_level,
				      unsigned first_layer, unsigned last_layer,
				      unsigned first_sample, unsigned last_sample);

	/* Reallocate the buffer and update all resource bindings where
	 * the buffer is bound, including all resource descriptors. */
	void (*invalidate_buffer)(struct pipe_context *ctx, struct pipe_resource *buf);

	/* Update all resource bindings where the buffer is bound, including
	 * all resource descriptors. This is invalidate_buffer without
	 * the invalidation. */
	void (*rebind_buffer)(struct pipe_context *ctx, struct pipe_resource *buf,
			      uint64_t old_gpu_address);

	void (*save_qbo_state)(struct pipe_context *ctx, struct r600_qbo_state *st);

	/* This ensures there is enough space in the command stream. */
	void (*need_gfx_cs_space)(struct pipe_context *ctx, unsigned num_dw,
				  bool include_draw_vbo);

	void (*set_atom_dirty)(struct r600_common_context *ctx,
			       struct r600_atom *atom, bool dirty);

	void (*check_vm_faults)(struct r600_common_context *ctx,
				struct radeon_saved_cs *saved,
				enum amd_ip_type ring);
};

/* r600_buffer_common.c */
bool r600_rings_is_buffer_referenced(struct r600_common_context *ctx,
				     struct pb_buffer_lean *buf,
				     unsigned usage);
void *r600_buffer_map_sync_with_rings(struct r600_common_context *ctx,
                                      struct r600_resource *resource,
                                      unsigned usage);
void r600_buffer_subdata(struct pipe_context *ctx,
			 struct pipe_resource *buffer,
			 unsigned usage, unsigned offset,
			 unsigned size, const void *data);
void r600_init_resource_fields(struct r600_common_screen *rscreen,
			       struct r600_resource *res,
			       uint64_t size, unsigned alignment);
bool r600_alloc_resource(struct r600_common_screen *rscreen,
			 struct r600_resource *res);
void r600_buffer_destroy(struct pipe_screen *screen, struct pipe_resource *buf);
void r600_buffer_flush_region(struct pipe_context *ctx,
			      struct pipe_transfer *transfer,
			      const struct pipe_box *rel_box);
struct pipe_resource *r600_buffer_create(struct pipe_screen *screen,
					 const struct pipe_resource *templ,
					 unsigned alignment);
struct pipe_resource * r600_aligned_buffer_create(struct pipe_screen *screen,
						  unsigned flags,
						  unsigned usage,
						  unsigned size,
						  unsigned alignment);
struct pipe_resource *
r600_buffer_from_user_memory(struct pipe_screen *screen,
			     const struct pipe_resource *templ,
			     void *user_memory);
void
r600_invalidate_resource(struct pipe_context *ctx,
			 struct pipe_resource *resource);
void r600_replace_buffer_storage(struct pipe_context *ctx,
				 struct pipe_resource *dst,
				 struct pipe_resource *src);
void *r600_buffer_transfer_map(struct pipe_context *ctx,
                               struct pipe_resource *resource,
                               unsigned level,
                               unsigned usage,
                               const struct pipe_box *box,
                               struct pipe_transfer **ptransfer);
void r600_buffer_transfer_unmap(struct pipe_context *ctx,
				struct pipe_transfer *transfer);

/* r600_common_pipe.c */
void r600_gfx_write_event_eop(struct r600_common_context *ctx,
			      unsigned event, unsigned event_flags,
			      unsigned data_sel,
			      struct r600_resource *buf, uint64_t va,
			      uint32_t new_fence, unsigned query_type);
unsigned r600_gfx_write_fence_dwords(struct r600_common_screen *screen);
void r600_gfx_wait_fence(struct r600_common_context *ctx,
			 struct r600_resource *buf,
			 uint64_t va, uint32_t ref, uint32_t mask);
void r600_draw_rectangle(struct blitter_context *blitter,
			 void *vertex_elements_cso,
			 blitter_get_vs_func get_vs,
			 int x1, int y1, int x2, int y2,
			 float depth, unsigned num_instances,
			 enum blitter_attrib_type type,
			 const union blitter_attrib *attrib);
bool r600_common_screen_init(struct r600_common_screen *rscreen,
			     struct radeon_winsys *ws);
void r600_destroy_common_screen(struct r600_common_screen *rscreen);
void r600_preflush_suspend_features(struct r600_common_context *ctx);
void r600_postflush_resume_features(struct r600_common_context *ctx);
bool r600_common_context_init(struct r600_common_context *rctx,
			      struct r600_common_screen *rscreen,
			      unsigned context_flags);
void r600_common_context_cleanup(struct r600_common_context *rctx);
bool r600_can_dump_shader(struct r600_common_screen *rscreen,
			  unsigned processor);
bool r600_extra_shader_checks(struct r600_common_screen *rscreen,
			      unsigned processor);
void r600_screen_clear_buffer(struct r600_common_screen *rscreen, struct pipe_resource *dst,
			      uint64_t offset, uint64_t size, unsigned value);
struct pipe_resource *r600_resource_create_common(struct pipe_screen *screen,
						  const struct pipe_resource *templ);
const char *r600_get_llvm_processor_name(enum radeon_family family);
void r600_need_dma_space(struct r600_common_context *ctx, unsigned num_dw,
			 struct r600_resource *dst, struct r600_resource *src);
void radeon_save_cs(struct radeon_winsys *ws, struct radeon_cmdbuf *cs,
		    struct radeon_saved_cs *saved, bool get_buffer_list);
void radeon_clear_saved_cs(struct radeon_saved_cs *saved);
bool r600_check_device_reset(struct r600_common_context *rctx);

/* r600_gpu_load.c */
void r600_gpu_load_kill_thread(struct r600_common_screen *rscreen);
uint64_t r600_begin_counter(struct r600_common_screen *rscreen, unsigned type);
unsigned r600_end_counter(struct r600_common_screen *rscreen, unsigned type,
			  uint64_t begin);

/* r600_perfcounters.c */
void r600_perfcounters_destroy(struct r600_common_screen *rscreen);

/* r600_query.c */
void r600_init_screen_query_functions(struct r600_common_screen *rscreen);
void r600_query_init(struct r600_common_context *rctx);
void r600_suspend_queries(struct r600_common_context *ctx);
void r600_resume_queries(struct r600_common_context *ctx);
void r600_query_fix_enabled_rb_mask(struct r600_common_screen *rscreen);

/* r600_streamout.c */
void r600_streamout_buffers_dirty(struct r600_common_context *rctx);
void r600_set_streamout_targets(struct pipe_context *ctx,
				unsigned num_targets,
				struct pipe_stream_output_target **targets,
				const unsigned *offset);
void r600_emit_streamout_end(struct r600_common_context *rctx);
void r600_update_prims_generated_query_state(struct r600_common_context *rctx,
					     unsigned type, int diff);
void r600_streamout_init(struct r600_common_context *rctx);

/* r600_test_dma.c */
void r600_test_dma(struct r600_common_screen *rscreen);

/* r600_texture.c */
bool r600_prepare_for_dma_blit(struct r600_common_context *rctx,
				struct r600_texture *rdst,
				unsigned dst_level, unsigned dstx,
				unsigned dsty, unsigned dstz,
				struct r600_texture *rsrc,
				unsigned src_level,
				const struct pipe_box *src_box);
void r600_texture_destroy(struct pipe_screen *screen, struct pipe_resource *ptex);
void r600_texture_get_fmask_info(struct r600_common_screen *rscreen,
				 struct r600_texture *rtex,
				 unsigned nr_samples,
				 struct r600_fmask_info *out);
void r600_texture_get_cmask_info(struct r600_common_screen *rscreen,
				 struct r600_texture *rtex,
				 struct r600_cmask_info *out);
bool r600_init_flushed_depth_texture(struct pipe_context *ctx,
				     struct pipe_resource *texture,
				     struct r600_texture **staging);
void r600_print_texture_info(struct r600_common_screen *rscreen,
			     struct r600_texture *rtex, struct u_log_context *log);
struct pipe_resource *r600_texture_create(struct pipe_screen *screen,
					const struct pipe_resource *templ);
struct pipe_surface *r600_create_surface_custom(struct pipe_context *pipe,
						struct pipe_resource *texture,
						const struct pipe_surface *templ,
						unsigned width0, unsigned height0,
						unsigned width, unsigned height);
unsigned r600_translate_colorswap(enum pipe_format format, bool do_endian_swap);
void evergreen_do_fast_color_clear(struct r600_common_context *rctx,
				   struct pipe_framebuffer_state *fb,
				   struct r600_atom *fb_state,
				   unsigned *buffers, uint8_t *dirty_cbufs,
				   const union pipe_color_union *color);
void r600_init_screen_texture_functions(struct r600_common_screen *rscreen);
void r600_init_context_texture_functions(struct r600_common_context *rctx);
void eg_resource_alloc_immed(struct r600_common_screen *rscreen,
			     struct r600_resource *res,
			     unsigned immed_size);
void *r600_texture_transfer_map(struct pipe_context *ctx,
			       struct pipe_resource *texture,
			       unsigned level,
			       unsigned usage,
			       const struct pipe_box *box,
			       struct pipe_transfer **ptransfer);
void r600_texture_transfer_unmap(struct pipe_context *ctx,
				struct pipe_transfer* transfer);

/* r600_viewport.c */
void evergreen_apply_scissor_bug_workaround(struct r600_common_context *rctx,
					    struct pipe_scissor_state *scissor);
void r600_viewport_set_rast_deps(struct r600_common_context *rctx,
				 bool scissor_enable, bool clip_halfz);
void r600_update_vs_writes_viewport_index(struct r600_common_context *rctx,
					  struct tgsi_shader_info *info);
void r600_init_viewport_functions(struct r600_common_context *rctx);

/* cayman_msaa.c */
extern const uint32_t eg_sample_locs_2x[4];
extern const unsigned eg_max_dist_2x;
extern const uint32_t eg_sample_locs_4x[4];
extern const unsigned eg_max_dist_4x;
void cayman_get_sample_position(struct pipe_context *ctx, unsigned sample_count,
				unsigned sample_index, float *out_value);
void cayman_init_msaa(struct pipe_context *ctx);
void cayman_emit_msaa_state(struct radeon_cmdbuf *cs, int nr_samples,
			    int ps_iter_samples, int overrast_samples);


/* Inline helpers. */

static inline struct r600_resource *r600_resource(struct pipe_resource *r)
{
	return (struct r600_resource*)r;
}

static inline void
r600_resource_reference(struct r600_resource **ptr, struct r600_resource *res)
{
	pipe_resource_reference((struct pipe_resource **)ptr,
				(struct pipe_resource *)res);
}

static inline void
r600_texture_reference(struct r600_texture **ptr, struct r600_texture *res)
{
	pipe_resource_reference((struct pipe_resource **)ptr, &res->resource.b.b);
}

static inline void
r600_context_add_resource_size(struct pipe_context *ctx, struct pipe_resource *r)
{
	struct r600_common_context *rctx = (struct r600_common_context *)ctx;
	struct r600_resource *res = (struct r600_resource *)r;

	if (res) {
		/* Add memory usage for need_gfx_cs_space */
		rctx->vram += res->vram_usage;
		rctx->gtt += res->gart_usage;
	}
}

static inline bool r600_get_strmout_en(struct r600_common_context *rctx)
{
	return rctx->streamout.streamout_enabled ||
	       rctx->streamout.prims_gen_query_enabled;
}

#define     SQ_TEX_XY_FILTER_POINT                         0x00
#define     SQ_TEX_XY_FILTER_BILINEAR                      0x01
#define     SQ_TEX_XY_FILTER_ANISO_POINT                   0x02
#define     SQ_TEX_XY_FILTER_ANISO_BILINEAR                0x03

static inline unsigned eg_tex_filter(unsigned filter, unsigned max_aniso)
{
	if (filter == PIPE_TEX_FILTER_LINEAR)
		return max_aniso > 1 ? SQ_TEX_XY_FILTER_ANISO_BILINEAR
				     : SQ_TEX_XY_FILTER_BILINEAR;
	else
		return max_aniso > 1 ? SQ_TEX_XY_FILTER_ANISO_POINT
				     : SQ_TEX_XY_FILTER_POINT;
}

static inline unsigned r600_tex_aniso_filter(unsigned filter)
{
	if (filter < 2)
		return 0;
	if (filter < 4)
		return 1;
	if (filter < 8)
		return 2;
	if (filter < 16)
		return 3;
	return 4;
}

static inline unsigned r600_wavefront_size(enum radeon_family family)
{
	switch (family) {
	case CHIP_RV610:
	case CHIP_RS780:
	case CHIP_RV620:
	case CHIP_RS880:
		return 16;
	case CHIP_RV630:
	case CHIP_RV635:
	case CHIP_RV730:
	case CHIP_RV710:
	case CHIP_PALM:
	case CHIP_CEDAR:
		return 32;
	default:
		return 64;
	}
}

static inline unsigned
r600_get_sampler_view_priority(struct r600_resource *res)
{
	if (res->b.b.target == PIPE_BUFFER)
		return RADEON_PRIO_SAMPLER_BUFFER;

	if (res->b.b.nr_samples > 1)
		return RADEON_PRIO_SAMPLER_TEXTURE_MSAA;

	return RADEON_PRIO_SAMPLER_TEXTURE;
}

static inline bool
r600_can_sample_zs(struct r600_texture *tex, bool stencil_sampler)
{
	return (stencil_sampler && tex->can_sample_s) ||
	       (!stencil_sampler && tex->can_sample_z);
}

static inline bool
r600_htile_enabled(struct r600_texture *tex, unsigned level)
{
	return tex->htile_offset && level == 0;
}

#define COMPUTE_DBG(rscreen, fmt, args...) \
	do { \
		if ((rscreen->b.debug_flags & DBG_COMPUTE)) fprintf(stderr, fmt, ##args); \
	} while (0);

#define R600_ERR(fmt, args...) \
	fprintf(stderr, "EE %s:%d %s - " fmt, __FILE__, __LINE__, __func__, ##args)

/* For MSAA sample positions. */
#define FILL_SREG(s0x, s0y, s1x, s1y, s2x, s2y, s3x, s3y)  \
	(((s0x) & 0xf) | (((unsigned)(s0y) & 0xf) << 4) |		   \
	(((unsigned)(s1x) & 0xf) << 8) | (((unsigned)(s1y) & 0xf) << 12) |	   \
	(((unsigned)(s2x) & 0xf) << 16) | (((unsigned)(s2y) & 0xf) << 20) |	   \
	 (((unsigned)(s3x) & 0xf) << 24) | (((unsigned)(s3y) & 0xf) << 28))

static inline int S_FIXED(float value, unsigned frac_bits)
{
	return value * (1 << frac_bits);
}

#endif
