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
#include "r600_pipe.h"
#include "r600d.h"
#include "util/u_memory.h"
#include <errno.h>
#include <unistd.h>


void r600_need_cs_space(struct r600_context *ctx, unsigned num_dw,
			bool count_draw_in, unsigned num_atomics)
{
	/* Flush the DMA IB if it's not empty. */
	if (radeon_emitted(&ctx->b.dma.cs, 0))
		ctx->b.dma.flush(ctx, PIPE_FLUSH_ASYNC, NULL);

	if (!radeon_cs_memory_below_limit(ctx->b.screen, &ctx->b.gfx.cs,
					  ctx->b.vram, ctx->b.gtt)) {
		ctx->b.gtt = 0;
		ctx->b.vram = 0;
		ctx->b.gfx.flush(ctx, PIPE_FLUSH_ASYNC, NULL);
		return;
	}
	/* all will be accounted once relocation are emitted */
	ctx->b.gtt = 0;
	ctx->b.vram = 0;

	/* Check available space in CS. */
	if (count_draw_in) {
		uint64_t mask;

		/* The number of dwords all the dirty states would take. */
		mask = ctx->dirty_atoms;
		while (mask != 0)
			num_dw += ctx->atoms[u_bit_scan64(&mask)]->num_dw;

		/* The upper-bound of how much space a draw command would take. */
		num_dw += R600_MAX_FLUSH_CS_DWORDS + R600_MAX_DRAW_CS_DWORDS;
	}

	/* add atomic counters, 8 pre + 8 post per counter + 16 post if any counters */
	num_dw += (num_atomics * 16) + (num_atomics ? 16 : 0);

	/* Count in r600_suspend_queries. */
	num_dw += ctx->b.num_cs_dw_queries_suspend;

	/* Count in streamout_end at the end of CS. */
	if (ctx->b.streamout.begin_emitted) {
		num_dw += ctx->b.streamout.num_dw_for_end;
	}

	/* SX_MISC */
	if (ctx->b.gfx_level == R600) {
		num_dw += 3;
	}

	/* Count in framebuffer cache flushes at the end of CS. */
	num_dw += R600_MAX_FLUSH_CS_DWORDS;

	/* The fence at the end of CS. */
	num_dw += 10;

	/* Flush if there's not enough space. */
	if (!ctx->b.ws->cs_check_space(&ctx->b.gfx.cs, num_dw)) {
		ctx->b.gfx.flush(ctx, PIPE_FLUSH_ASYNC, NULL);
	}
}

void r600_flush_emit(struct r600_context *rctx)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;
	unsigned cp_coher_cntl = 0;
	unsigned wait_until = 0;

	if (!rctx->b.flags) {
		return;
	}

	/* Ensure coherency between streamout and shaders. */
	if (rctx->b.flags & R600_CONTEXT_STREAMOUT_FLUSH)
		rctx->b.flags |= r600_get_flush_flags(R600_COHERENCY_SHADER);

	if (rctx->b.flags & R600_CONTEXT_WAIT_3D_IDLE) {
		wait_until |= S_008040_WAIT_3D_IDLE(1);
	}
	if (rctx->b.flags & R600_CONTEXT_WAIT_CP_DMA_IDLE) {
		wait_until |= S_008040_WAIT_CP_DMA_IDLE(1);
	}

	if (wait_until) {
		/* Use of WAIT_UNTIL is deprecated on Cayman+ */
		if (rctx->b.family >= CHIP_CAYMAN) {
			/* emit a PS partial flush on Cayman/TN */
			rctx->b.flags |= R600_CONTEXT_PS_PARTIAL_FLUSH;
		}
	}

	/* Wait packets must be executed first, because SURFACE_SYNC doesn't
	 * wait for shaders if it's not flushing CB or DB.
	 */
	if (rctx->b.flags & R600_CONTEXT_PS_PARTIAL_FLUSH) {
		radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
		radeon_emit(cs, EVENT_TYPE(EVENT_TYPE_PS_PARTIAL_FLUSH) | EVENT_INDEX(4));
	}

	if (rctx->b.flags & R600_CONTEXT_CS_PARTIAL_FLUSH) {
		radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
		radeon_emit(cs, EVENT_TYPE(EVENT_TYPE_CS_PARTIAL_FLUSH) | EVENT_INDEX(4));
	}

	if (wait_until) {
		/* Use of WAIT_UNTIL is deprecated on Cayman+ */
		if (rctx->b.family < CHIP_CAYMAN) {
			/* wait for things to settle */
			radeon_set_config_reg(cs, R_008040_WAIT_UNTIL, wait_until);
		}
	}

	if (rctx->b.gfx_level >= R700 &&
	    (rctx->b.flags & R600_CONTEXT_FLUSH_AND_INV_CB_META)) {
		radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
		radeon_emit(cs, EVENT_TYPE(EVENT_TYPE_FLUSH_AND_INV_CB_META) | EVENT_INDEX(0));
	}

	if (rctx->b.gfx_level >= R700 &&
	    (rctx->b.flags & R600_CONTEXT_FLUSH_AND_INV_DB_META)) {
		radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
		radeon_emit(cs, EVENT_TYPE(EVENT_TYPE_FLUSH_AND_INV_DB_META) | EVENT_INDEX(0));

		/* Set FULL_CACHE_ENA for DB META flushes on r7xx and later.
		 *
		 * This hack predates use of FLUSH_AND_INV_DB_META, so it's
		 * unclear whether it's still needed or even whether it has
		 * any effect.
		 */
		cp_coher_cntl |= S_0085F0_FULL_CACHE_ENA(1);
	}

	if (rctx->b.flags & R600_CONTEXT_FLUSH_AND_INV ||
	    (rctx->b.gfx_level == R600 && rctx->b.flags & R600_CONTEXT_STREAMOUT_FLUSH)) {
		radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
		radeon_emit(cs, EVENT_TYPE(EVENT_TYPE_CACHE_FLUSH_AND_INV_EVENT) | EVENT_INDEX(0));
	}

	if (rctx->b.flags & R600_CONTEXT_INV_CONST_CACHE) {
		/* Direct constant addressing uses the shader cache.
		 * Indirect constant addressing uses the vertex cache. */
		cp_coher_cntl |= S_0085F0_SH_ACTION_ENA(1) |
				 (rctx->has_vertex_cache ? S_0085F0_VC_ACTION_ENA(1)
							 : S_0085F0_TC_ACTION_ENA(1));
	}
	if (rctx->b.flags & R600_CONTEXT_INV_VERTEX_CACHE) {
		cp_coher_cntl |= rctx->has_vertex_cache ? S_0085F0_VC_ACTION_ENA(1)
							: S_0085F0_TC_ACTION_ENA(1);
	}
	if (rctx->b.flags & R600_CONTEXT_INV_TEX_CACHE) {
		/* Textures use the texture cache.
		 * Texture buffer objects use the vertex cache. */
		cp_coher_cntl |= S_0085F0_TC_ACTION_ENA(1) |
				 (rctx->has_vertex_cache ? S_0085F0_VC_ACTION_ENA(1) : 0);
	}

	/* Don't use the DB CP COHER logic on r6xx.
	 * There are hw bugs.
	 */
	if (rctx->b.gfx_level >= R700 &&
	    (rctx->b.flags & R600_CONTEXT_FLUSH_AND_INV_DB)) {
		cp_coher_cntl |= S_0085F0_DB_ACTION_ENA(1) |
				S_0085F0_DB_DEST_BASE_ENA(1) |
				S_0085F0_SMX_ACTION_ENA(1);
	}

	/* Don't use the CB CP COHER logic on r6xx.
	 * There are hw bugs.
	 */
	if (rctx->b.gfx_level >= R700 &&
	    (rctx->b.flags & R600_CONTEXT_FLUSH_AND_INV_CB)) {
		cp_coher_cntl |= S_0085F0_CB_ACTION_ENA(1) |
				S_0085F0_CB0_DEST_BASE_ENA(1) |
				S_0085F0_CB1_DEST_BASE_ENA(1) |
				S_0085F0_CB2_DEST_BASE_ENA(1) |
				S_0085F0_CB3_DEST_BASE_ENA(1) |
				S_0085F0_CB4_DEST_BASE_ENA(1) |
				S_0085F0_CB5_DEST_BASE_ENA(1) |
				S_0085F0_CB6_DEST_BASE_ENA(1) |
				S_0085F0_CB7_DEST_BASE_ENA(1) |
				S_0085F0_SMX_ACTION_ENA(1);
		if (rctx->b.gfx_level >= EVERGREEN)
			cp_coher_cntl |= S_0085F0_CB8_DEST_BASE_ENA(1) |
					S_0085F0_CB9_DEST_BASE_ENA(1) |
					S_0085F0_CB10_DEST_BASE_ENA(1) |
					S_0085F0_CB11_DEST_BASE_ENA(1);
	}

	if (rctx->b.gfx_level >= R700 &&
	    rctx->b.flags & R600_CONTEXT_STREAMOUT_FLUSH) {
		cp_coher_cntl |= S_0085F0_SO0_DEST_BASE_ENA(1) |
				S_0085F0_SO1_DEST_BASE_ENA(1) |
				S_0085F0_SO2_DEST_BASE_ENA(1) |
				S_0085F0_SO3_DEST_BASE_ENA(1) |
				S_0085F0_SMX_ACTION_ENA(1);
	}

	/* Workaround for buggy flushing on some R6xx chipsets. */
	if ((rctx->b.flags & (R600_CONTEXT_FLUSH_AND_INV |
			      R600_CONTEXT_STREAMOUT_FLUSH)) &&
	    (rctx->b.family == CHIP_RV670 ||
	     rctx->b.family == CHIP_RS780 ||
	     rctx->b.family == CHIP_RS880)) {
		cp_coher_cntl |=  S_0085F0_CB1_DEST_BASE_ENA(1) |
				  S_0085F0_DEST_BASE_0_ENA(1);
	}

	if (cp_coher_cntl) {
		radeon_emit(cs, PKT3(PKT3_SURFACE_SYNC, 3, 0));
		radeon_emit(cs, cp_coher_cntl);   /* CP_COHER_CNTL */
		radeon_emit(cs, 0xffffffff);      /* CP_COHER_SIZE */
		radeon_emit(cs, 0);               /* CP_COHER_BASE */
		radeon_emit(cs, 0x0000000A);      /* POLL_INTERVAL */
	}

	if (rctx->b.flags & R600_CONTEXT_START_PIPELINE_STATS) {
		radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
		radeon_emit(cs, EVENT_TYPE(EVENT_TYPE_PIPELINESTAT_START) |
			        EVENT_INDEX(0));
	} else if (rctx->b.flags & R600_CONTEXT_STOP_PIPELINE_STATS) {
		radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
		radeon_emit(cs, EVENT_TYPE(EVENT_TYPE_PIPELINESTAT_STOP) |
			        EVENT_INDEX(0));
	}

	/* everything is properly flushed */
	rctx->b.flags = 0;
}

void r600_context_gfx_flush(void *context, unsigned flags,
			    struct pipe_fence_handle **fence)
{
	struct r600_context *ctx = context;
	struct radeon_cmdbuf *cs = &ctx->b.gfx.cs;
	struct radeon_winsys *ws = ctx->b.ws;

	if (!radeon_emitted(cs, ctx->b.initial_gfx_cs_size))
		return;

	if (r600_check_device_reset(&ctx->b))
		return;

	r600_preflush_suspend_features(&ctx->b);

	/* flush the framebuffer cache */
	ctx->b.flags |= R600_CONTEXT_FLUSH_AND_INV |
		      R600_CONTEXT_FLUSH_AND_INV_CB |
		      R600_CONTEXT_FLUSH_AND_INV_DB |
		      R600_CONTEXT_FLUSH_AND_INV_CB_META |
		      R600_CONTEXT_FLUSH_AND_INV_DB_META |
		      R600_CONTEXT_WAIT_3D_IDLE |
		      R600_CONTEXT_WAIT_CP_DMA_IDLE;

	r600_flush_emit(ctx);

	if (ctx->trace_buf)
		eg_trace_emit(ctx);
	/* old kernels and userspace don't set SX_MISC, so we must reset it to 0 here */
	if (ctx->b.gfx_level == R600) {
		radeon_set_context_reg(cs, R_028350_SX_MISC, 0);
	}

	if (ctx->is_debug) {
		/* Save the IB for debug contexts. */
		radeon_clear_saved_cs(&ctx->last_gfx);
		radeon_save_cs(ws, cs, &ctx->last_gfx, true);
		r600_resource_reference(&ctx->last_trace_buf, ctx->trace_buf);
		r600_resource_reference(&ctx->trace_buf, NULL);
	}
	/* Flush the CS. */
	ws->cs_flush(cs, flags, &ctx->b.last_gfx_fence);
	if (fence)
		ws->fence_reference(ws, fence, ctx->b.last_gfx_fence);
	ctx->b.num_gfx_cs_flushes++;

	if (ctx->is_debug) {
		if (!ws->fence_wait(ws, ctx->b.last_gfx_fence, 10000000)) {
			const char *fname = getenv("R600_TRACE");
			if (!fname)
				exit(-1);
			FILE *fl = fopen(fname, "w+");
			if (fl) {
				eg_dump_debug_state(&ctx->b.b, fl, 0);
				fclose(fl);
			} else
				perror(fname);
			exit(-1);
		}
	}
	r600_begin_new_cs(ctx);
}

void r600_begin_new_cs(struct r600_context *ctx)
{
	unsigned shader;

	if (ctx->is_debug) {
		uint32_t zero = 0;

		/* Create a buffer used for writing trace IDs and initialize it to 0. */
		assert(!ctx->trace_buf);
		ctx->trace_buf = (struct r600_resource*)
			pipe_buffer_create(ctx->b.b.screen, 0,
					   PIPE_USAGE_STAGING, 4);
		if (ctx->trace_buf)
			pipe_buffer_write_nooverlap(&ctx->b.b, &ctx->trace_buf->b.b,
						    0, sizeof(zero), &zero);
		ctx->trace_id = 0;
	}

	if (ctx->trace_buf)
		eg_trace_emit(ctx);

	ctx->b.flags = 0;
	ctx->b.gtt = 0;
	ctx->b.vram = 0;

	/* Begin a new CS. */
	r600_emit_command_buffer(&ctx->b.gfx.cs, &ctx->start_cs_cmd);

	/* Re-emit states. */
	r600_mark_atom_dirty(ctx, &ctx->alphatest_state.atom);
	r600_mark_atom_dirty(ctx, &ctx->blend_color.atom);
	r600_mark_atom_dirty(ctx, &ctx->cb_misc_state.atom);
	r600_mark_atom_dirty(ctx, &ctx->clip_misc_state.atom);
	r600_mark_atom_dirty(ctx, &ctx->clip_state.atom);
	r600_mark_atom_dirty(ctx, &ctx->db_misc_state.atom);
	r600_mark_atom_dirty(ctx, &ctx->db_state.atom);
	r600_mark_atom_dirty(ctx, &ctx->framebuffer.atom);
	if (ctx->b.gfx_level >= EVERGREEN) {
		r600_mark_atom_dirty(ctx, &ctx->fragment_images.atom);
		r600_mark_atom_dirty(ctx, &ctx->fragment_buffers.atom);
		r600_mark_atom_dirty(ctx, &ctx->compute_images.atom);
		r600_mark_atom_dirty(ctx, &ctx->compute_buffers.atom);
	}
	r600_mark_atom_dirty(ctx, &ctx->hw_shader_stages[R600_HW_STAGE_PS].atom);
	r600_mark_atom_dirty(ctx, &ctx->poly_offset_state.atom);
	r600_mark_atom_dirty(ctx, &ctx->vgt_state.atom);
	r600_mark_atom_dirty(ctx, &ctx->sample_mask.atom);
	ctx->b.scissors.dirty_mask = (1 << R600_MAX_VIEWPORTS) - 1;
	r600_mark_atom_dirty(ctx, &ctx->b.scissors.atom);
	ctx->b.viewports.dirty_mask = (1 << R600_MAX_VIEWPORTS) - 1;
	ctx->b.viewports.depth_range_dirty_mask = (1 << R600_MAX_VIEWPORTS) - 1;
	r600_mark_atom_dirty(ctx, &ctx->b.viewports.atom);
	if (ctx->b.gfx_level <= EVERGREEN) {
		r600_mark_atom_dirty(ctx, &ctx->config_state.atom);
	}
	r600_mark_atom_dirty(ctx, &ctx->stencil_ref.atom);
	r600_mark_atom_dirty(ctx, &ctx->vertex_fetch_shader.atom);
	r600_mark_atom_dirty(ctx, &ctx->hw_shader_stages[R600_HW_STAGE_ES].atom);
	r600_mark_atom_dirty(ctx, &ctx->shader_stages.atom);
	if (ctx->gs_shader) {
		r600_mark_atom_dirty(ctx, &ctx->hw_shader_stages[R600_HW_STAGE_GS].atom);
		r600_mark_atom_dirty(ctx, &ctx->gs_rings.atom);
	}
	if (ctx->tes_shader) {
		r600_mark_atom_dirty(ctx, &ctx->hw_shader_stages[EG_HW_STAGE_HS].atom);
		r600_mark_atom_dirty(ctx, &ctx->hw_shader_stages[EG_HW_STAGE_LS].atom);
	}
	r600_mark_atom_dirty(ctx, &ctx->hw_shader_stages[R600_HW_STAGE_VS].atom);
	r600_mark_atom_dirty(ctx, &ctx->b.streamout.enable_atom);
	r600_mark_atom_dirty(ctx, &ctx->b.render_cond_atom);

	if (ctx->blend_state.cso)
		r600_mark_atom_dirty(ctx, &ctx->blend_state.atom);
	if (ctx->dsa_state.cso)
		r600_mark_atom_dirty(ctx, &ctx->dsa_state.atom);
	if (ctx->rasterizer_state.cso)
		r600_mark_atom_dirty(ctx, &ctx->rasterizer_state.atom);

	if (ctx->b.gfx_level <= R700) {
		r600_mark_atom_dirty(ctx, &ctx->seamless_cube_map.atom);
	}

	ctx->vertex_buffer_state.dirty_mask = ctx->vertex_buffer_state.enabled_mask;
	r600_vertex_buffers_dirty(ctx);

	/* Re-emit shader resources. */
	for (shader = 0; shader < PIPE_SHADER_TYPES; shader++) {
		struct r600_constbuf_state *constbuf = &ctx->constbuf_state[shader];
		struct r600_textures_info *samplers = &ctx->samplers[shader];

		constbuf->dirty_mask = constbuf->enabled_mask;
		samplers->views.dirty_mask = samplers->views.enabled_mask;
		samplers->states.dirty_mask = samplers->states.enabled_mask;

		r600_constant_buffers_dirty(ctx, constbuf);
		r600_sampler_views_dirty(ctx, &samplers->views);
		r600_sampler_states_dirty(ctx, &samplers->states);
	}

	for (shader = 0; shader < ARRAY_SIZE(ctx->scratch_buffers); shader++) {
		ctx->scratch_buffers[shader].dirty = true;
	}

	r600_postflush_resume_features(&ctx->b);

	/* Re-emit the draw state. */
	ctx->last_primitive_type = -1;
	ctx->last_start_instance = -1;
	ctx->last_rast_prim      = -1;
	ctx->current_rast_prim   = -1;

	assert(!ctx->b.gfx.cs.prev_dw);
	ctx->b.initial_gfx_cs_size = ctx->b.gfx.cs.current.cdw;
}

void r600_emit_pfp_sync_me(struct r600_context *rctx)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;

	if (rctx->b.gfx_level >= EVERGREEN) {
		radeon_emit(cs, PKT3(PKT3_PFP_SYNC_ME, 0, 0));
		radeon_emit(cs, 0);
	} else {
		/* Emulate PFP_SYNC_ME by writing a value to memory in ME and
		 * waiting for it in PFP.
		 */
		struct r600_resource *buf = NULL;
		unsigned offset, reloc;
		uint64_t va;

		/* 16-byte address alignment is required by WAIT_REG_MEM. */
		u_suballocator_alloc(&rctx->b.allocator_zeroed_memory, 4, 16,
				     &offset, (struct pipe_resource**)&buf);
		if (!buf) {
			/* This is too heavyweight, but will work. */
			rctx->b.gfx.flush(rctx, PIPE_FLUSH_ASYNC, NULL);
			return;
		}

		reloc = radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx, buf,
						  RADEON_USAGE_READWRITE |
						  RADEON_PRIO_FENCE_TRACE);

		va = buf->gpu_address + offset;
		assert(va % 16 == 0);

		/* Write 1 to memory in ME. */
		radeon_emit(cs, PKT3(PKT3_MEM_WRITE, 3, 0));
		radeon_emit(cs, va);
		radeon_emit(cs, ((va >> 32) & 0xff) | MEM_WRITE_32_BITS);
		radeon_emit(cs, 1);
		radeon_emit(cs, 0);

		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
		radeon_emit(cs, reloc);

		/* Wait in PFP (PFP can only do GEQUAL against memory). */
		radeon_emit(cs, PKT3(PKT3_WAIT_REG_MEM, 5, 0));
		radeon_emit(cs, WAIT_REG_MEM_GEQUAL |
			        WAIT_REG_MEM_MEMORY |
			        WAIT_REG_MEM_PFP);
		radeon_emit(cs, va);
		radeon_emit(cs, va >> 32);
		radeon_emit(cs, 1); /* reference value */
		radeon_emit(cs, 0xffffffff); /* mask */
		radeon_emit(cs, 4); /* poll interval */

		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
		radeon_emit(cs, reloc);

		r600_resource_reference(&buf, NULL);
	}
}

/* The max number of bytes to copy per packet. */
#define CP_DMA_MAX_BYTE_COUNT ((1 << 21) - 8)

void r600_cp_dma_copy_buffer(struct r600_context *rctx,
			     struct pipe_resource *dst, uint64_t dst_offset,
			     struct pipe_resource *src, uint64_t src_offset,
			     unsigned size)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;

	assert(size);
	assert(rctx->screen->b.has_cp_dma);

	/* Mark the buffer range of destination as valid (initialized),
	 * so that transfer_map knows it should wait for the GPU when mapping
	 * that range. */
	util_range_add(dst, &r600_resource(dst)->valid_buffer_range, dst_offset,
		       dst_offset + size);

	dst_offset += r600_resource(dst)->gpu_address;
	src_offset += r600_resource(src)->gpu_address;

	/* Flush the caches where the resources are bound. */
	rctx->b.flags |= r600_get_flush_flags(R600_COHERENCY_SHADER) |
			 R600_CONTEXT_WAIT_3D_IDLE;

	/* There are differences between R700 and EG in CP DMA,
	 * but we only use the common bits here. */
	while (size) {
		unsigned sync = 0;
		unsigned byte_count = MIN2(size, CP_DMA_MAX_BYTE_COUNT);
		unsigned src_reloc, dst_reloc;

		r600_need_cs_space(rctx,
				   10 + (rctx->b.flags ? R600_MAX_FLUSH_CS_DWORDS : 0) +
				   3 + R600_MAX_PFP_SYNC_ME_DWORDS, false, 0);

		/* Flush the caches for the first copy only. */
		if (rctx->b.flags) {
			r600_flush_emit(rctx);
		}

		/* Do the synchronization after the last copy, so that all data is written to memory. */
		if (size == byte_count) {
			sync = PKT3_CP_DMA_CP_SYNC;
		}

		/* This must be done after r600_need_cs_space. */
		src_reloc = radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx, (struct r600_resource*)src,
						  RADEON_USAGE_READ | RADEON_PRIO_CP_DMA);
		dst_reloc = radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx, (struct r600_resource*)dst,
						  RADEON_USAGE_WRITE | RADEON_PRIO_CP_DMA);

		radeon_emit(cs, PKT3(PKT3_CP_DMA, 4, 0));
		radeon_emit(cs, src_offset);	/* SRC_ADDR_LO [31:0] */
		radeon_emit(cs, sync | ((src_offset >> 32) & 0xff));		/* CP_SYNC [31] | SRC_ADDR_HI [7:0] */
		radeon_emit(cs, dst_offset);	/* DST_ADDR_LO [31:0] */
		radeon_emit(cs, (dst_offset >> 32) & 0xff);		/* DST_ADDR_HI [7:0] */
		radeon_emit(cs, byte_count);	/* COMMAND [29:22] | BYTE_COUNT [20:0] */

		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
		radeon_emit(cs, src_reloc);
		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
		radeon_emit(cs, dst_reloc);

		size -= byte_count;
		src_offset += byte_count;
		dst_offset += byte_count;
	}

	/* CP_DMA_CP_SYNC doesn't wait for idle on R6xx, but this does. */
	if (rctx->b.gfx_level == R600)
		radeon_set_config_reg(cs, R_008040_WAIT_UNTIL,
				      S_008040_WAIT_CP_DMA_IDLE(1));

	/* CP DMA is executed in ME, but index buffers are read by PFP.
	 * This ensures that ME (CP DMA) is idle before PFP starts fetching
	 * indices. If we wanted to execute CP DMA in PFP, this packet
	 * should precede it.
	 */
	r600_emit_pfp_sync_me(rctx);
}

void r600_dma_copy_buffer(struct r600_context *rctx,
			  struct pipe_resource *dst,
			  struct pipe_resource *src,
			  uint64_t dst_offset,
			  uint64_t src_offset,
			  uint64_t size)
{
	struct radeon_cmdbuf *cs = &rctx->b.dma.cs;
	unsigned i, ncopy, csize;
	struct r600_resource *rdst = (struct r600_resource*)dst;
	struct r600_resource *rsrc = (struct r600_resource*)src;

	/* Mark the buffer range of destination as valid (initialized),
	 * so that transfer_map knows it should wait for the GPU when mapping
	 * that range. */
	util_range_add(&rdst->b.b, &rdst->valid_buffer_range, dst_offset,
		       dst_offset + size);

	size >>= 2; /* convert to dwords */
	ncopy = (size / R600_DMA_COPY_MAX_SIZE_DW) + !!(size % R600_DMA_COPY_MAX_SIZE_DW);

	r600_need_dma_space(&rctx->b, ncopy * 5, rdst, rsrc);
	for (i = 0; i < ncopy; i++) {
		csize = size < R600_DMA_COPY_MAX_SIZE_DW ? size : R600_DMA_COPY_MAX_SIZE_DW;
		/* emit reloc before writing cs so that cs is always in consistent state */
		radeon_add_to_buffer_list(&rctx->b, &rctx->b.dma, rsrc, RADEON_USAGE_READ);
		radeon_add_to_buffer_list(&rctx->b, &rctx->b.dma, rdst, RADEON_USAGE_WRITE);
		radeon_emit(cs, DMA_PACKET(DMA_PACKET_COPY, 0, 0, csize));
		radeon_emit(cs, dst_offset & 0xfffffffc);
		radeon_emit(cs, src_offset & 0xfffffffc);
		radeon_emit(cs, (dst_offset >> 32UL) & 0xff);
		radeon_emit(cs, (src_offset >> 32UL) & 0xff);
		dst_offset += csize << 2;
		src_offset += csize << 2;
		size -= csize;
	}
}
