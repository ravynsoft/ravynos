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

#include "r600_pipe_common.h"
#include "r600_cs.h"
#include "evergreen_compute.h"
#include "util/list.h"
#include "util/u_draw_quad.h"
#include "util/u_memory.h"
#include "util/format/u_format_s3tc.h"
#include "util/u_upload_mgr.h"
#include "util/os_time.h"
#include "util/hex.h"
#include "vl/vl_decoder.h"
#include "vl/vl_video_buffer.h"
#include "radeon_video.h"
#include "git_sha1.h"

#include <inttypes.h>
#include <sys/utsname.h>
#include <stdlib.h>

#if LLVM_AVAILABLE
#include <llvm-c/TargetMachine.h>
#endif

struct r600_multi_fence {
	struct pipe_reference reference;
	struct pipe_fence_handle *gfx;
	struct pipe_fence_handle *sdma;

	/* If the context wasn't flushed at fence creation, this is non-NULL. */
	struct {
		struct r600_common_context *ctx;
		unsigned ib_index;
	} gfx_unflushed;
};

/*
 * pipe_context
 */

/**
 * Write an EOP event.
 *
 * \param event		EVENT_TYPE_*
 * \param event_flags	Optional cache flush flags (TC)
 * \param data_sel	1 = fence, 3 = timestamp
 * \param buf		Buffer
 * \param va		GPU address
 * \param old_value	Previous fence value (for a bug workaround)
 * \param new_value	Fence value to write for this event.
 */
void r600_gfx_write_event_eop(struct r600_common_context *ctx,
			      unsigned event, unsigned event_flags,
			      unsigned data_sel,
			      struct r600_resource *buf, uint64_t va,
			      uint32_t new_fence, unsigned query_type)
{
	struct radeon_cmdbuf *cs = &ctx->gfx.cs;
	unsigned op = EVENT_TYPE(event) |
		      EVENT_INDEX(5) |
		      event_flags;
	unsigned sel = EOP_DATA_SEL(data_sel);

	radeon_emit(cs, PKT3(PKT3_EVENT_WRITE_EOP, 4, 0));
	radeon_emit(cs, op);
	radeon_emit(cs, va);
	radeon_emit(cs, ((va >> 32) & 0xffff) | sel);
	radeon_emit(cs, new_fence); /* immediate data */
	radeon_emit(cs, 0); /* unused */

	if (buf)
		r600_emit_reloc(ctx, &ctx->gfx, buf, RADEON_USAGE_WRITE |
				RADEON_PRIO_QUERY);
}

unsigned r600_gfx_write_fence_dwords(struct r600_common_screen *screen)
{
	unsigned dwords = 6;

	if (!screen->info.r600_has_virtual_memory)
		dwords += 2;

	return dwords;
}

void r600_gfx_wait_fence(struct r600_common_context *ctx,
			 struct r600_resource *buf,
			 uint64_t va, uint32_t ref, uint32_t mask)
{
	struct radeon_cmdbuf *cs = &ctx->gfx.cs;

	radeon_emit(cs, PKT3(PKT3_WAIT_REG_MEM, 5, 0));
	radeon_emit(cs, WAIT_REG_MEM_EQUAL | WAIT_REG_MEM_MEM_SPACE(1));
	radeon_emit(cs, va);
	radeon_emit(cs, va >> 32);
	radeon_emit(cs, ref); /* reference value */
	radeon_emit(cs, mask); /* mask */
	radeon_emit(cs, 4); /* poll interval */

	if (buf)
		r600_emit_reloc(ctx, &ctx->gfx, buf, RADEON_USAGE_READ |
				RADEON_PRIO_QUERY);
}

void r600_draw_rectangle(struct blitter_context *blitter,
			 void *vertex_elements_cso,
			 blitter_get_vs_func get_vs,
			 int x1, int y1, int x2, int y2,
			 float depth, unsigned num_instances,
			 enum blitter_attrib_type type,
			 const union blitter_attrib *attrib)
{
	struct r600_common_context *rctx =
		(struct r600_common_context*)util_blitter_get_pipe(blitter);
	struct pipe_viewport_state viewport;
	struct pipe_resource *buf = NULL;
	unsigned offset = 0;
	float *vb;

	rctx->b.bind_vertex_elements_state(&rctx->b, vertex_elements_cso);
	rctx->b.bind_vs_state(&rctx->b, get_vs(blitter));

	/* Some operations (like color resolve on r6xx) don't work
	 * with the conventional primitive types.
	 * One that works is PT_RECTLIST, which we use here. */

	/* setup viewport */
	viewport.scale[0] = 1.0f;
	viewport.scale[1] = 1.0f;
	viewport.scale[2] = 1.0f;
	viewport.translate[0] = 0.0f;
	viewport.translate[1] = 0.0f;
	viewport.translate[2] = 0.0f;
	rctx->b.set_viewport_states(&rctx->b, 0, 1, &viewport);

	/* Upload vertices. The hw rectangle has only 3 vertices,
	 * The 4th one is derived from the first 3.
	 * The vertex specification should match u_blitter's vertex element state. */
	u_upload_alloc(rctx->b.stream_uploader, 0, sizeof(float) * 24,
		       rctx->screen->info.tcc_cache_line_size,
                       &offset, &buf, (void**)&vb);
	if (!buf)
		return;

	vb[0] = x1;
	vb[1] = y1;
	vb[2] = depth;
	vb[3] = 1;

	vb[8] = x1;
	vb[9] = y2;
	vb[10] = depth;
	vb[11] = 1;

	vb[16] = x2;
	vb[17] = y1;
	vb[18] = depth;
	vb[19] = 1;

	switch (type) {
	case UTIL_BLITTER_ATTRIB_COLOR:
		memcpy(vb+4, attrib->color, sizeof(float)*4);
		memcpy(vb+12, attrib->color, sizeof(float)*4);
		memcpy(vb+20, attrib->color, sizeof(float)*4);
		break;
	case UTIL_BLITTER_ATTRIB_TEXCOORD_XYZW:
	case UTIL_BLITTER_ATTRIB_TEXCOORD_XY:
		vb[6] = vb[14] = vb[22] = attrib->texcoord.z;
		vb[7] = vb[15] = vb[23] = attrib->texcoord.w;
		/* fall through */
		vb[4] = attrib->texcoord.x1;
		vb[5] = attrib->texcoord.y1;
		vb[12] = attrib->texcoord.x1;
		vb[13] = attrib->texcoord.y2;
		vb[20] = attrib->texcoord.x2;
		vb[21] = attrib->texcoord.y1;
		break;
	default:; /* Nothing to do. */
	}

	/* draw */
	struct pipe_vertex_buffer vbuffer = {};
	vbuffer.buffer.resource = buf;
	vbuffer.buffer_offset = offset;

	rctx->b.set_vertex_buffers(&rctx->b, 1, 0, false, &vbuffer);
	util_draw_arrays_instanced(&rctx->b, R600_PRIM_RECTANGLE_LIST, 0, 3,
				   0, num_instances);
	pipe_resource_reference(&buf, NULL);
}

static void r600_dma_emit_wait_idle(struct r600_common_context *rctx)
{
	struct radeon_cmdbuf *cs = &rctx->dma.cs;

	if (rctx->gfx_level >= EVERGREEN)
		radeon_emit(cs, 0xf0000000); /* NOP */
	else {
		/* TODO: R600-R700 should use the FENCE packet.
		 * CS checker support is required. */
	}
}

void r600_need_dma_space(struct r600_common_context *ctx, unsigned num_dw,
                         struct r600_resource *dst, struct r600_resource *src)
{
	uint64_t vram = (uint64_t)ctx->dma.cs.used_vram_kb * 1024;
	uint64_t gtt = (uint64_t)ctx->dma.cs.used_gart_kb * 1024;

	if (dst) {
		vram += dst->vram_usage;
		gtt += dst->gart_usage;
	}
	if (src) {
		vram += src->vram_usage;
		gtt += src->gart_usage;
	}

	/* Flush the GFX IB if DMA depends on it. */
	if (radeon_emitted(&ctx->gfx.cs, ctx->initial_gfx_cs_size) &&
	    ((dst &&
	      ctx->ws->cs_is_buffer_referenced(&ctx->gfx.cs, dst->buf,
					       RADEON_USAGE_READWRITE)) ||
	     (src &&
	      ctx->ws->cs_is_buffer_referenced(&ctx->gfx.cs, src->buf,
					       RADEON_USAGE_WRITE))))
		ctx->gfx.flush(ctx, PIPE_FLUSH_ASYNC, NULL);

	/* Flush if there's not enough space, or if the memory usage per IB
	 * is too large.
	 *
	 * IBs using too little memory are limited by the IB submission overhead.
	 * IBs using too much memory are limited by the kernel/TTM overhead.
	 * Too long IBs create CPU-GPU pipeline bubbles and add latency.
	 *
	 * This heuristic makes sure that DMA requests are executed
	 * very soon after the call is made and lowers memory usage.
	 * It improves texture upload performance by keeping the DMA
	 * engine busy while uploads are being submitted.
	 */
	num_dw++; /* for emit_wait_idle below */
	if (!ctx->ws->cs_check_space(&ctx->dma.cs, num_dw) ||
	    ctx->dma.cs.used_vram_kb + ctx->dma.cs.used_gart_kb > 64 * 1024 ||
	    !radeon_cs_memory_below_limit(ctx->screen, &ctx->dma.cs, vram, gtt)) {
		ctx->dma.flush(ctx, PIPE_FLUSH_ASYNC, NULL);
		assert((num_dw + ctx->dma.cs.current.cdw) <= ctx->dma.cs.current.max_dw);
	}

	/* Wait for idle if either buffer has been used in the IB before to
	 * prevent read-after-write hazards.
	 */
	if ((dst &&
	     ctx->ws->cs_is_buffer_referenced(&ctx->dma.cs, dst->buf,
					      RADEON_USAGE_READWRITE)) ||
	    (src &&
	     ctx->ws->cs_is_buffer_referenced(&ctx->dma.cs, src->buf,
					      RADEON_USAGE_WRITE)))
		r600_dma_emit_wait_idle(ctx);

	/* If GPUVM is not supported, the CS checker needs 2 entries
	 * in the buffer list per packet, which has to be done manually.
	 */
	if (ctx->screen->info.r600_has_virtual_memory) {
		if (dst)
			radeon_add_to_buffer_list(ctx, &ctx->dma, dst,
						  RADEON_USAGE_WRITE);
		if (src)
			radeon_add_to_buffer_list(ctx, &ctx->dma, src,
						  RADEON_USAGE_READ);
	}

	/* this function is called before all DMA calls, so increment this. */
	ctx->num_dma_calls++;
}

void r600_preflush_suspend_features(struct r600_common_context *ctx)
{
	/* suspend queries */
	if (!list_is_empty(&ctx->active_queries))
		r600_suspend_queries(ctx);

	ctx->streamout.suspended = false;
	if (ctx->streamout.begin_emitted) {
		r600_emit_streamout_end(ctx);
		ctx->streamout.suspended = true;
	}
}

void r600_postflush_resume_features(struct r600_common_context *ctx)
{
	if (ctx->streamout.suspended) {
		ctx->streamout.append_bitmask = ctx->streamout.enabled_mask;
		r600_streamout_buffers_dirty(ctx);
	}

	/* resume queries */
	if (!list_is_empty(&ctx->active_queries))
		r600_resume_queries(ctx);
}

static void r600_fence_server_sync(struct pipe_context *ctx,
				   struct pipe_fence_handle *fence)
{
	/* radeon synchronizes all rings by default and will not implement
	 * fence imports.
	 */
}

static void r600_flush_from_st(struct pipe_context *ctx,
			       struct pipe_fence_handle **fence,
			       unsigned flags)
{
	struct pipe_screen *screen = ctx->screen;
	struct r600_common_context *rctx = (struct r600_common_context *)ctx;
	struct radeon_winsys *ws = rctx->ws;
	struct pipe_fence_handle *gfx_fence = NULL;
	struct pipe_fence_handle *sdma_fence = NULL;
	bool deferred_fence = false;
	unsigned rflags = PIPE_FLUSH_ASYNC;

	if (flags & PIPE_FLUSH_END_OF_FRAME)
		rflags |= PIPE_FLUSH_END_OF_FRAME;

	/* DMA IBs are preambles to gfx IBs, therefore must be flushed first. */
	if (rctx->dma.cs.priv)
		rctx->dma.flush(rctx, rflags, fence ? &sdma_fence : NULL);

	if (!radeon_emitted(&rctx->gfx.cs, rctx->initial_gfx_cs_size)) {
		if (fence)
			ws->fence_reference(ws, &gfx_fence, rctx->last_gfx_fence);
		if (!(flags & PIPE_FLUSH_DEFERRED))
			ws->cs_sync_flush(&rctx->gfx.cs);
	} else {
		/* Instead of flushing, create a deferred fence. Constraints:
		 * - the gallium frontend must allow a deferred flush.
		 * - the gallium frontend must request a fence.
		 * Thread safety in fence_finish must be ensured by the gallium frontend.
		 */
		if (flags & PIPE_FLUSH_DEFERRED && fence) {
			gfx_fence = rctx->ws->cs_get_next_fence(&rctx->gfx.cs);
			deferred_fence = true;
		} else {
			rctx->gfx.flush(rctx, rflags, fence ? &gfx_fence : NULL);
		}
	}

	/* Both engines can signal out of order, so we need to keep both fences. */
	if (fence) {
		struct r600_multi_fence *multi_fence =
			CALLOC_STRUCT(r600_multi_fence);
		if (!multi_fence) {
			ws->fence_reference(ws, &sdma_fence, NULL);
			ws->fence_reference(ws, &gfx_fence, NULL);
			goto finish;
		}

		multi_fence->reference.count = 1;
		/* If both fences are NULL, fence_finish will always return true. */
		multi_fence->gfx = gfx_fence;
		multi_fence->sdma = sdma_fence;

		if (deferred_fence) {
			multi_fence->gfx_unflushed.ctx = rctx;
			multi_fence->gfx_unflushed.ib_index = rctx->num_gfx_cs_flushes;
		}

		screen->fence_reference(screen, fence, NULL);
		*fence = (struct pipe_fence_handle*)multi_fence;
	}
finish:
	if (!(flags & PIPE_FLUSH_DEFERRED)) {
		if (rctx->dma.cs.priv)
			ws->cs_sync_flush(&rctx->dma.cs);
		ws->cs_sync_flush(&rctx->gfx.cs);
	}
}

static void r600_flush_dma_ring(void *ctx, unsigned flags,
				struct pipe_fence_handle **fence)
{
	struct r600_common_context *rctx = (struct r600_common_context *)ctx;
	struct radeon_cmdbuf *cs = &rctx->dma.cs;
	struct radeon_saved_cs saved;
	bool check_vm =
		(rctx->screen->debug_flags & DBG_CHECK_VM) &&
		rctx->check_vm_faults;

	if (!radeon_emitted(cs, 0)) {
		if (fence)
			rctx->ws->fence_reference(rctx->ws, fence, rctx->last_sdma_fence);
		return;
	}

	if (check_vm)
		radeon_save_cs(rctx->ws, cs, &saved, true);

	rctx->ws->cs_flush(cs, flags, &rctx->last_sdma_fence);
	if (fence)
		rctx->ws->fence_reference(rctx->ws, fence, rctx->last_sdma_fence);

	if (check_vm) {
		/* Use conservative timeout 800ms, after which we won't wait any
		 * longer and assume the GPU is hung.
		 */
		rctx->ws->fence_wait(rctx->ws, rctx->last_sdma_fence, 800*1000*1000);

		rctx->check_vm_faults(rctx, &saved, AMD_IP_SDMA);
		radeon_clear_saved_cs(&saved);
	}
}

/**
 * Store a linearized copy of all chunks of \p cs together with the buffer
 * list in \p saved.
 */
void radeon_save_cs(struct radeon_winsys *ws, struct radeon_cmdbuf *cs,
		    struct radeon_saved_cs *saved, bool get_buffer_list)
{
	uint32_t *buf;
	unsigned i;

	/* Save the IB chunks. */
	saved->num_dw = cs->prev_dw + cs->current.cdw;
	saved->ib = MALLOC(4 * saved->num_dw);
	if (!saved->ib)
		goto oom;

	buf = saved->ib;
	for (i = 0; i < cs->num_prev; ++i) {
		memcpy(buf, cs->prev[i].buf, cs->prev[i].cdw * 4);
		buf += cs->prev[i].cdw;
	}
	memcpy(buf, cs->current.buf, cs->current.cdw * 4);

	if (!get_buffer_list)
		return;

	/* Save the buffer list. */
	saved->bo_count = ws->cs_get_buffer_list(cs, NULL);
	saved->bo_list = CALLOC(saved->bo_count,
				sizeof(saved->bo_list[0]));
	if (!saved->bo_list) {
		FREE(saved->ib);
		goto oom;
	}
	ws->cs_get_buffer_list(cs, saved->bo_list);

	return;

oom:
	fprintf(stderr, "%s: out of memory\n", __func__);
	memset(saved, 0, sizeof(*saved));
}

void radeon_clear_saved_cs(struct radeon_saved_cs *saved)
{
	FREE(saved->ib);
	FREE(saved->bo_list);

	memset(saved, 0, sizeof(*saved));
}

static enum pipe_reset_status r600_get_reset_status(struct pipe_context *ctx)
{
	struct r600_common_context *rctx = (struct r600_common_context *)ctx;

	return rctx->ws->ctx_query_reset_status(rctx->ctx, false, NULL, NULL);
}

static void r600_set_debug_callback(struct pipe_context *ctx,
				    const struct util_debug_callback *cb)
{
	struct r600_common_context *rctx = (struct r600_common_context *)ctx;

	if (cb)
		rctx->debug = *cb;
	else
		memset(&rctx->debug, 0, sizeof(rctx->debug));
}

static void r600_set_device_reset_callback(struct pipe_context *ctx,
					   const struct pipe_device_reset_callback *cb)
{
	struct r600_common_context *rctx = (struct r600_common_context *)ctx;

	if (cb)
		rctx->device_reset_callback = *cb;
	else
		memset(&rctx->device_reset_callback, 0,
		       sizeof(rctx->device_reset_callback));
}

bool r600_check_device_reset(struct r600_common_context *rctx)
{
	enum pipe_reset_status status;

	if (!rctx->device_reset_callback.reset)
		return false;

	if (!rctx->b.get_device_reset_status)
		return false;

	status = rctx->b.get_device_reset_status(&rctx->b);
	if (status == PIPE_NO_RESET)
		return false;

	rctx->device_reset_callback.reset(rctx->device_reset_callback.data, status);
	return true;
}

static void r600_dma_clear_buffer_fallback(struct pipe_context *ctx,
					   struct pipe_resource *dst,
					   uint64_t offset, uint64_t size,
					   unsigned value)
{
	struct r600_common_context *rctx = (struct r600_common_context *)ctx;

	rctx->clear_buffer(ctx, dst, offset, size, value, R600_COHERENCY_NONE);
}

static bool r600_resource_commit(struct pipe_context *pctx,
				 struct pipe_resource *resource,
				 unsigned level, struct pipe_box *box,
				 bool commit)
{
	struct r600_common_context *ctx = (struct r600_common_context *)pctx;
	struct r600_resource *res = r600_resource(resource);

	/*
	 * Since buffer commitment changes cannot be pipelined, we need to
	 * (a) flush any pending commands that refer to the buffer we're about
	 *     to change, and
	 * (b) wait for threaded submit to finish, including those that were
	 *     triggered by some other, earlier operation.
	 */
	if (radeon_emitted(&ctx->gfx.cs, ctx->initial_gfx_cs_size) &&
	    ctx->ws->cs_is_buffer_referenced(&ctx->gfx.cs,
					     res->buf, RADEON_USAGE_READWRITE)) {
		ctx->gfx.flush(ctx, PIPE_FLUSH_ASYNC, NULL);
	}
	if (radeon_emitted(&ctx->dma.cs, 0) &&
	    ctx->ws->cs_is_buffer_referenced(&ctx->dma.cs,
					     res->buf, RADEON_USAGE_READWRITE)) {
		ctx->dma.flush(ctx, PIPE_FLUSH_ASYNC, NULL);
	}

	ctx->ws->cs_sync_flush(&ctx->dma.cs);
	ctx->ws->cs_sync_flush(&ctx->gfx.cs);

	assert(resource->target == PIPE_BUFFER);

	return ctx->ws->buffer_commit(ctx->ws, res->buf, box->x, box->width, commit);
}

bool r600_common_context_init(struct r600_common_context *rctx,
			      struct r600_common_screen *rscreen,
			      unsigned context_flags)
{
	slab_create_child(&rctx->pool_transfers, &rscreen->pool_transfers);
	slab_create_child(&rctx->pool_transfers_unsync, &rscreen->pool_transfers);

	rctx->screen = rscreen;
	rctx->ws = rscreen->ws;
	rctx->family = rscreen->family;
	rctx->gfx_level = rscreen->gfx_level;

	rctx->b.clear_buffer = u_default_clear_buffer;
	rctx->b.invalidate_resource = r600_invalidate_resource;
	rctx->b.resource_commit = r600_resource_commit;
	rctx->b.buffer_map = r600_buffer_transfer_map;
        rctx->b.texture_map = r600_texture_transfer_map;
	rctx->b.transfer_flush_region = r600_buffer_flush_region;
	rctx->b.buffer_unmap = r600_buffer_transfer_unmap;
        rctx->b.texture_unmap = r600_texture_transfer_unmap;
	rctx->b.texture_subdata = u_default_texture_subdata;
	rctx->b.flush = r600_flush_from_st;
	rctx->b.set_debug_callback = r600_set_debug_callback;
	rctx->b.fence_server_sync = r600_fence_server_sync;
	rctx->dma_clear_buffer = r600_dma_clear_buffer_fallback;

	/* evergreen_compute.c has a special codepath for global buffers.
	 * Everything else can use the direct path.
	 */
	if ((rscreen->gfx_level == EVERGREEN || rscreen->gfx_level == CAYMAN) &&
	    (context_flags & PIPE_CONTEXT_COMPUTE_ONLY))
		rctx->b.buffer_subdata = u_default_buffer_subdata;
	else
		rctx->b.buffer_subdata = r600_buffer_subdata;

	rctx->b.get_device_reset_status = r600_get_reset_status;
	rctx->b.set_device_reset_callback = r600_set_device_reset_callback;

	r600_init_context_texture_functions(rctx);
	r600_init_viewport_functions(rctx);
	r600_streamout_init(rctx);
	r600_query_init(rctx);
	cayman_init_msaa(&rctx->b);

	u_suballocator_init(&rctx->allocator_zeroed_memory, &rctx->b, rscreen->info.gart_page_size,
			    0, PIPE_USAGE_DEFAULT, 0, true);

	rctx->b.stream_uploader = u_upload_create(&rctx->b, 1024 * 1024,
						  0, PIPE_USAGE_STREAM, 0);
	if (!rctx->b.stream_uploader)
		return false;

	rctx->b.const_uploader = u_upload_create(&rctx->b, 128 * 1024,
						 0, PIPE_USAGE_DEFAULT, 0);
	if (!rctx->b.const_uploader)
		return false;

	rctx->ctx = rctx->ws->ctx_create(rctx->ws, RADEON_CTX_PRIORITY_MEDIUM, false);
	if (!rctx->ctx)
		return false;

	if (rscreen->info.ip[AMD_IP_SDMA].num_queues && !(rscreen->debug_flags & DBG_NO_ASYNC_DMA)) {
		rctx->ws->cs_create(&rctx->dma.cs, rctx->ctx, AMD_IP_SDMA,
                                    r600_flush_dma_ring, rctx);
		rctx->dma.flush = r600_flush_dma_ring;
	}

	return true;
}

void r600_common_context_cleanup(struct r600_common_context *rctx)
{
	if (rctx->query_result_shader)
		rctx->b.delete_compute_state(&rctx->b, rctx->query_result_shader);

	rctx->ws->cs_destroy(&rctx->gfx.cs);
	rctx->ws->cs_destroy(&rctx->dma.cs);
	if (rctx->ctx)
		rctx->ws->ctx_destroy(rctx->ctx);

	if (rctx->b.stream_uploader)
		u_upload_destroy(rctx->b.stream_uploader);
	if (rctx->b.const_uploader)
		u_upload_destroy(rctx->b.const_uploader);

	slab_destroy_child(&rctx->pool_transfers);
	slab_destroy_child(&rctx->pool_transfers_unsync);

	u_suballocator_destroy(&rctx->allocator_zeroed_memory);
	rctx->ws->fence_reference(rctx->ws, &rctx->last_gfx_fence, NULL);
	rctx->ws->fence_reference(rctx->ws, &rctx->last_sdma_fence, NULL);
	r600_resource_reference(&rctx->eop_bug_scratch, NULL);
}

/*
 * pipe_screen
 */

static const struct debug_named_value common_debug_options[] = {
	/* logging */
	{ "tex", DBG_TEX, "Print texture info" },
	{ "nir", DBG_NIR, "Enable experimental NIR shaders" },
	{ "compute", DBG_COMPUTE, "Print compute info" },
	{ "vm", DBG_VM, "Print virtual addresses when creating resources" },
	{ "info", DBG_INFO, "Print driver information" },

	/* shaders */
	{ "fs", DBG_FS, "Print fetch shaders" },
	{ "vs", DBG_VS, "Print vertex shaders" },
	{ "gs", DBG_GS, "Print geometry shaders" },
	{ "ps", DBG_PS, "Print pixel shaders" },
	{ "cs", DBG_CS, "Print compute shaders" },
	{ "tcs", DBG_TCS, "Print tessellation control shaders" },
	{ "tes", DBG_TES, "Print tessellation evaluation shaders" },
	{ "preoptir", DBG_PREOPT_IR, "Print the LLVM IR before initial optimizations" },
	{ "checkir", DBG_CHECK_IR, "Enable additional sanity checks on shader IR" },

	{ "testdma", DBG_TEST_DMA, "Invoke SDMA tests and exit." },
	{ "testvmfaultcp", DBG_TEST_VMFAULT_CP, "Invoke a CP VM fault test and exit." },
	{ "testvmfaultsdma", DBG_TEST_VMFAULT_SDMA, "Invoke a SDMA VM fault test and exit." },
	{ "testvmfaultshader", DBG_TEST_VMFAULT_SHADER, "Invoke a shader VM fault test and exit." },

	/* features */
	{ "nodma", DBG_NO_ASYNC_DMA, "Disable asynchronous DMA" },
	{ "nohyperz", DBG_NO_HYPERZ, "Disable Hyper-Z" },
	/* GL uses the word INVALIDATE, gallium uses the word DISCARD */
	{ "noinvalrange", DBG_NO_DISCARD_RANGE, "Disable handling of INVALIDATE_RANGE map flags" },
	{ "no2d", DBG_NO_2D_TILING, "Disable 2D tiling" },
	{ "notiling", DBG_NO_TILING, "Disable tiling" },
	{ "switch_on_eop", DBG_SWITCH_ON_EOP, "Program WD/IA to switch on end-of-packet." },
	{ "forcedma", DBG_FORCE_DMA, "Use asynchronous DMA for all operations when possible." },
	{ "nowc", DBG_NO_WC, "Disable GTT write combining" },
	{ "check_vm", DBG_CHECK_VM, "Check VM faults and dump debug info." },

	DEBUG_NAMED_VALUE_END /* must be last */
};

static const char* r600_get_vendor(struct pipe_screen* pscreen)
{
	return "Mesa";
}

static const char* r600_get_device_vendor(struct pipe_screen* pscreen)
{
	return "AMD";
}

static const char *r600_get_family_name(const struct r600_common_screen *rscreen)
{
	switch (rscreen->info.family) {
	case CHIP_R600: return "AMD R600";
	case CHIP_RV610: return "AMD RV610";
	case CHIP_RV630: return "AMD RV630";
	case CHIP_RV670: return "AMD RV670";
	case CHIP_RV620: return "AMD RV620";
	case CHIP_RV635: return "AMD RV635";
	case CHIP_RS780: return "AMD RS780";
	case CHIP_RS880: return "AMD RS880";
	case CHIP_RV770: return "AMD RV770";
	case CHIP_RV730: return "AMD RV730";
	case CHIP_RV710: return "AMD RV710";
	case CHIP_RV740: return "AMD RV740";
	case CHIP_CEDAR: return "AMD CEDAR";
	case CHIP_REDWOOD: return "AMD REDWOOD";
	case CHIP_JUNIPER: return "AMD JUNIPER";
	case CHIP_CYPRESS: return "AMD CYPRESS";
	case CHIP_HEMLOCK: return "AMD HEMLOCK";
	case CHIP_PALM: return "AMD PALM";
	case CHIP_SUMO: return "AMD SUMO";
	case CHIP_SUMO2: return "AMD SUMO2";
	case CHIP_BARTS: return "AMD BARTS";
	case CHIP_TURKS: return "AMD TURKS";
	case CHIP_CAICOS: return "AMD CAICOS";
	case CHIP_CAYMAN: return "AMD CAYMAN";
	case CHIP_ARUBA: return "AMD ARUBA";
	default: return "AMD unknown";
	}
}

static void r600_disk_cache_create(struct r600_common_screen *rscreen)
{
	/* Don't use the cache if shader dumping is enabled. */
	if (rscreen->debug_flags & DBG_ALL_SHADERS)
		return;

	struct mesa_sha1 ctx;
	unsigned char sha1[20];
	char cache_id[20 * 2 + 1];

	_mesa_sha1_init(&ctx);
	if (!disk_cache_get_function_identifier(r600_disk_cache_create,
						&ctx))
		return;

	_mesa_sha1_final(&ctx, sha1);
	mesa_bytes_to_hex(cache_id, sha1, 20);

	/* These flags affect shader compilation. */
	rscreen->disk_shader_cache =
		disk_cache_create(r600_get_family_name(rscreen),
				  cache_id, 0);
}

static struct disk_cache *r600_get_disk_shader_cache(struct pipe_screen *pscreen)
{
	struct r600_common_screen *rscreen = (struct r600_common_screen*)pscreen;
	return rscreen->disk_shader_cache;
}

static const char* r600_get_name(struct pipe_screen* pscreen)
{
	struct r600_common_screen *rscreen = (struct r600_common_screen*)pscreen;

	return rscreen->renderer_string;
}

static float r600_get_paramf(struct pipe_screen* pscreen,
			     enum pipe_capf param)
{
	switch (param) {
	case PIPE_CAPF_MIN_LINE_WIDTH:
	case PIPE_CAPF_MIN_LINE_WIDTH_AA:
	case PIPE_CAPF_MIN_POINT_SIZE:
	case PIPE_CAPF_MIN_POINT_SIZE_AA:
		return 1;

	case PIPE_CAPF_POINT_SIZE_GRANULARITY:
	case PIPE_CAPF_LINE_WIDTH_GRANULARITY:
		return 0.1;

	case PIPE_CAPF_MAX_LINE_WIDTH:
	case PIPE_CAPF_MAX_LINE_WIDTH_AA:
	case PIPE_CAPF_MAX_POINT_SIZE:
	case PIPE_CAPF_MAX_POINT_SIZE_AA:
         return 8191.0f;
	case PIPE_CAPF_MAX_TEXTURE_ANISOTROPY:
		return 16.0f;
	case PIPE_CAPF_MAX_TEXTURE_LOD_BIAS:
		return 16.0f;
    case PIPE_CAPF_MIN_CONSERVATIVE_RASTER_DILATE:
    case PIPE_CAPF_MAX_CONSERVATIVE_RASTER_DILATE:
    case PIPE_CAPF_CONSERVATIVE_RASTER_DILATE_GRANULARITY:
        return 0.0f;
	}
	return 0.0f;
}

static int r600_get_video_param(struct pipe_screen *screen,
				enum pipe_video_profile profile,
				enum pipe_video_entrypoint entrypoint,
				enum pipe_video_cap param)
{
	switch (param) {
	case PIPE_VIDEO_CAP_SUPPORTED:
		return vl_profile_supported(screen, profile, entrypoint);
	case PIPE_VIDEO_CAP_NPOT_TEXTURES:
		return 1;
	case PIPE_VIDEO_CAP_MAX_WIDTH:
	case PIPE_VIDEO_CAP_MAX_HEIGHT:
		return vl_video_buffer_max_size(screen);
	case PIPE_VIDEO_CAP_PREFERED_FORMAT:
		return PIPE_FORMAT_NV12;
	case PIPE_VIDEO_CAP_PREFERS_INTERLACED:
		return false;
	case PIPE_VIDEO_CAP_SUPPORTS_INTERLACED:
		return false;
	case PIPE_VIDEO_CAP_SUPPORTS_PROGRESSIVE:
		return true;
	case PIPE_VIDEO_CAP_MAX_LEVEL:
		return vl_level_supported(screen, profile);
	default:
		return 0;
	}
}

const char *r600_get_llvm_processor_name(enum radeon_family family)
{
	switch (family) {
	case CHIP_R600:
	case CHIP_RV630:
	case CHIP_RV635:
	case CHIP_RV670:
		return "r600";
	case CHIP_RV610:
	case CHIP_RV620:
	case CHIP_RS780:
	case CHIP_RS880:
		return "rs880";
	case CHIP_RV710:
		return "rv710";
	case CHIP_RV730:
		return "rv730";
	case CHIP_RV740:
	case CHIP_RV770:
		return "rv770";
	case CHIP_PALM:
	case CHIP_CEDAR:
		return "cedar";
	case CHIP_SUMO:
	case CHIP_SUMO2:
		return "sumo";
	case CHIP_REDWOOD:
		return "redwood";
	case CHIP_JUNIPER:
		return "juniper";
	case CHIP_HEMLOCK:
	case CHIP_CYPRESS:
		return "cypress";
	case CHIP_BARTS:
		return "barts";
	case CHIP_TURKS:
		return "turks";
	case CHIP_CAICOS:
		return "caicos";
	case CHIP_CAYMAN:
        case CHIP_ARUBA:
		return "cayman";

	default:
		return "";
	}
}

static unsigned get_max_threads_per_block(struct r600_common_screen *screen,
					  enum pipe_shader_ir ir_type)
{
	if (ir_type != PIPE_SHADER_IR_TGSI &&
	    ir_type != PIPE_SHADER_IR_NIR)
		return 256;
	if (screen->gfx_level >= EVERGREEN)
		return 1024;
	return 256;
}

static int r600_get_compute_param(struct pipe_screen *screen,
        enum pipe_shader_ir ir_type,
        enum pipe_compute_cap param,
        void *ret)
{
	struct r600_common_screen *rscreen = (struct r600_common_screen *)screen;

	//TODO: select these params by asic
	switch (param) {
	case PIPE_COMPUTE_CAP_IR_TARGET: {
		const char *gpu;
		const char *triple = "r600--";
		gpu = r600_get_llvm_processor_name(rscreen->family);
		if (ret) {
			sprintf(ret, "%s-%s", gpu, triple);
		}
		/* +2 for dash and terminating NIL byte */
		return (strlen(triple) + strlen(gpu) + 2) * sizeof(char);
	}
	case PIPE_COMPUTE_CAP_GRID_DIMENSION:
		if (ret) {
			uint64_t *grid_dimension = ret;
			grid_dimension[0] = 3;
		}
		return 1 * sizeof(uint64_t);

	case PIPE_COMPUTE_CAP_MAX_GRID_SIZE:
		if (ret) {
			uint64_t *grid_size = ret;
			grid_size[0] = 65535;
			grid_size[1] = 65535;
			grid_size[2] = 65535;
		}
		return 3 * sizeof(uint64_t) ;

	case PIPE_COMPUTE_CAP_MAX_BLOCK_SIZE:
		if (ret) {
			uint64_t *block_size = ret;
			unsigned threads_per_block = get_max_threads_per_block(rscreen, ir_type);
			block_size[0] = threads_per_block;
			block_size[1] = threads_per_block;
			block_size[2] = threads_per_block;
		}
		return 3 * sizeof(uint64_t);

	case PIPE_COMPUTE_CAP_MAX_THREADS_PER_BLOCK:
		if (ret) {
			uint64_t *max_threads_per_block = ret;
			*max_threads_per_block = get_max_threads_per_block(rscreen, ir_type);
		}
		return sizeof(uint64_t);
	case PIPE_COMPUTE_CAP_ADDRESS_BITS:
		if (ret) {
			uint32_t *address_bits = ret;
			address_bits[0] = 32;
		}
		return 1 * sizeof(uint32_t);

	case PIPE_COMPUTE_CAP_MAX_GLOBAL_SIZE:
		if (ret) {
			uint64_t *max_global_size = ret;
			uint64_t max_mem_alloc_size;

			r600_get_compute_param(screen, ir_type,
				PIPE_COMPUTE_CAP_MAX_MEM_ALLOC_SIZE,
				&max_mem_alloc_size);

			/* In OpenCL, the MAX_MEM_ALLOC_SIZE must be at least
			 * 1/4 of the MAX_GLOBAL_SIZE.  Since the
			 * MAX_MEM_ALLOC_SIZE is fixed for older kernels,
			 * make sure we never report more than
			 * 4 * MAX_MEM_ALLOC_SIZE.
			 */
			*max_global_size = MIN2(4 * max_mem_alloc_size,
						rscreen->info.max_heap_size_kb * 1024ull);
		}
		return sizeof(uint64_t);

	case PIPE_COMPUTE_CAP_MAX_LOCAL_SIZE:
		if (ret) {
			uint64_t *max_local_size = ret;
			/* Value reported by the closed source driver. */
			*max_local_size = 32768;
		}
		return sizeof(uint64_t);

	case PIPE_COMPUTE_CAP_MAX_INPUT_SIZE:
		if (ret) {
			uint64_t *max_input_size = ret;
			/* Value reported by the closed source driver. */
			*max_input_size = 1024;
		}
		return sizeof(uint64_t);

	case PIPE_COMPUTE_CAP_MAX_MEM_ALLOC_SIZE:
		if (ret) {
			uint64_t *max_mem_alloc_size = ret;

			*max_mem_alloc_size = (rscreen->info.max_heap_size_kb / 4) * 1024ull;
		}
		return sizeof(uint64_t);

	case PIPE_COMPUTE_CAP_MAX_CLOCK_FREQUENCY:
		if (ret) {
			uint32_t *max_clock_frequency = ret;
			*max_clock_frequency = rscreen->info.max_gpu_freq_mhz;
		}
		return sizeof(uint32_t);

	case PIPE_COMPUTE_CAP_MAX_COMPUTE_UNITS:
		if (ret) {
			uint32_t *max_compute_units = ret;
			*max_compute_units = rscreen->info.num_cu;
		}
		return sizeof(uint32_t);

	case PIPE_COMPUTE_CAP_IMAGES_SUPPORTED:
		if (ret) {
			uint32_t *images_supported = ret;
			*images_supported = 0;
		}
		return sizeof(uint32_t);
	case PIPE_COMPUTE_CAP_MAX_PRIVATE_SIZE:
		break; /* unused */
	case PIPE_COMPUTE_CAP_SUBGROUP_SIZES:
		if (ret) {
			uint32_t *subgroup_size = ret;
			*subgroup_size = r600_wavefront_size(rscreen->family);
		}
		return sizeof(uint32_t);
	case PIPE_COMPUTE_CAP_MAX_VARIABLE_THREADS_PER_BLOCK:
		if (ret) {
			uint64_t *max_variable_threads_per_block = ret;
			*max_variable_threads_per_block = 0;
		}
		return sizeof(uint64_t);
        case PIPE_COMPUTE_CAP_MAX_SUBGROUPS:
           return 0;
	}

        fprintf(stderr, "unknown PIPE_COMPUTE_CAP %d\n", param);
        return 0;
}

static uint64_t r600_get_timestamp(struct pipe_screen *screen)
{
	struct r600_common_screen *rscreen = (struct r600_common_screen*)screen;

	return 1000000 * rscreen->ws->query_value(rscreen->ws, RADEON_TIMESTAMP) /
			rscreen->info.clock_crystal_freq;
}

static void r600_fence_reference(struct pipe_screen *screen,
				 struct pipe_fence_handle **dst,
				 struct pipe_fence_handle *src)
{
	struct radeon_winsys *ws = ((struct r600_common_screen*)screen)->ws;
	struct r600_multi_fence **rdst = (struct r600_multi_fence **)dst;
	struct r600_multi_fence *rsrc = (struct r600_multi_fence *)src;

	if (pipe_reference(&(*rdst)->reference, &rsrc->reference)) {
		ws->fence_reference(ws, &(*rdst)->gfx, NULL);
		ws->fence_reference(ws, &(*rdst)->sdma, NULL);
		FREE(*rdst);
	}
        *rdst = rsrc;
}

static bool r600_fence_finish(struct pipe_screen *screen,
			      struct pipe_context *ctx,
			      struct pipe_fence_handle *fence,
			      uint64_t timeout)
{
	struct radeon_winsys *rws = ((struct r600_common_screen*)screen)->ws;
	struct r600_multi_fence *rfence = (struct r600_multi_fence *)fence;
	struct r600_common_context *rctx;
	int64_t abs_timeout = os_time_get_absolute_timeout(timeout);

	ctx = threaded_context_unwrap_sync(ctx);
	rctx = ctx ? (struct r600_common_context*)ctx : NULL;

	if (rfence->sdma) {
		if (!rws->fence_wait(rws, rfence->sdma, timeout))
			return false;

		/* Recompute the timeout after waiting. */
		if (timeout && timeout != OS_TIMEOUT_INFINITE) {
			int64_t time = os_time_get_nano();
			timeout = abs_timeout > time ? abs_timeout - time : 0;
		}
	}

	if (!rfence->gfx)
		return true;

	/* Flush the gfx IB if it hasn't been flushed yet. */
	if (rctx &&
	    rfence->gfx_unflushed.ctx == rctx &&
	    rfence->gfx_unflushed.ib_index == rctx->num_gfx_cs_flushes) {
		rctx->gfx.flush(rctx, timeout ? 0 : PIPE_FLUSH_ASYNC, NULL);
		rfence->gfx_unflushed.ctx = NULL;

		if (!timeout)
			return false;

		/* Recompute the timeout after all that. */
		if (timeout && timeout != OS_TIMEOUT_INFINITE) {
			int64_t time = os_time_get_nano();
			timeout = abs_timeout > time ? abs_timeout - time : 0;
		}
	}

	return rws->fence_wait(rws, rfence->gfx, timeout);
}

static void r600_query_memory_info(struct pipe_screen *screen,
				   struct pipe_memory_info *info)
{
	struct r600_common_screen *rscreen = (struct r600_common_screen*)screen;
	struct radeon_winsys *ws = rscreen->ws;
	unsigned vram_usage, gtt_usage;

	info->total_device_memory = rscreen->info.vram_size_kb;
	info->total_staging_memory = rscreen->info.gart_size_kb;

	/* The real TTM memory usage is somewhat random, because:
	 *
	 * 1) TTM delays freeing memory, because it can only free it after
	 *    fences expire.
	 *
	 * 2) The memory usage can be really low if big VRAM evictions are
	 *    taking place, but the real usage is well above the size of VRAM.
	 *
	 * Instead, return statistics of this process.
	 */
	vram_usage = ws->query_value(ws, RADEON_REQUESTED_VRAM_MEMORY) / 1024;
	gtt_usage =  ws->query_value(ws, RADEON_REQUESTED_GTT_MEMORY) / 1024;

	info->avail_device_memory =
		vram_usage <= info->total_device_memory ?
				info->total_device_memory - vram_usage : 0;
	info->avail_staging_memory =
		gtt_usage <= info->total_staging_memory ?
				info->total_staging_memory - gtt_usage : 0;

	info->device_memory_evicted =
		ws->query_value(ws, RADEON_NUM_BYTES_MOVED) / 1024;

	/* Just return the number of evicted 64KB pages. */
	info->nr_device_memory_evictions = info->device_memory_evicted / 64;
}

struct pipe_resource *r600_resource_create_common(struct pipe_screen *screen,
						  const struct pipe_resource *templ)
{
	if (templ->target == PIPE_BUFFER) {
		return r600_buffer_create(screen, templ, 256);
	} else {
		return r600_texture_create(screen, templ);
	}
}

static const void *
r600_get_compiler_options(struct pipe_screen *screen,
			  enum pipe_shader_ir ir,
			  enum pipe_shader_type shader)
{
       assert(ir == PIPE_SHADER_IR_NIR);

       struct r600_common_screen *rscreen = (struct r600_common_screen *)screen;

       if (shader != PIPE_SHADER_FRAGMENT)
          return &rscreen->nir_options;
       else
          return &rscreen->nir_options_fs;
}

extern bool r600_lower_to_scalar_instr_filter(const nir_instr *instr, const void *);

static void r600_resource_destroy(struct pipe_screen *screen,
				  struct pipe_resource *res)
{
	if (res->target == PIPE_BUFFER) {
		if (r600_resource(res)->compute_global_bo)
			r600_compute_global_buffer_destroy(screen, res);
		else
			r600_buffer_destroy(screen, res);
	} else {
		r600_texture_destroy(screen, res);
	}
}

static int r600_get_screen_fd(struct pipe_screen *screen)
{
	struct radeon_winsys *ws = ((struct r600_common_screen*)screen)->ws;

	return ws->get_fd(ws);
}

static void r600_get_driver_uuid(UNUSED struct pipe_screen *screen, char *uuid)
{
	const char *driver_id = PACKAGE_VERSION MESA_GIT_SHA1 "r600";

	/* The driver UUID is used for determining sharability of images and
	 * memory between two Vulkan instances in separate processes, but also
	 * to determining memory objects and sharability between Vulkan and
	 * OpenGL driver. People who want to share memory need to also check
	 * the device UUID.
	 */
	struct mesa_sha1 sha1_ctx;
	_mesa_sha1_init(&sha1_ctx);

	_mesa_sha1_update(&sha1_ctx, driver_id, strlen(driver_id));

	uint8_t sha1[SHA1_DIGEST_LENGTH];
	_mesa_sha1_final(&sha1_ctx, sha1);

	assert(SHA1_DIGEST_LENGTH >= PIPE_UUID_SIZE);
	memcpy(uuid, sha1, PIPE_UUID_SIZE);
}

static void r600_get_device_uuid(struct pipe_screen *screen, char *uuid)
{
	uint32_t *uint_uuid = (uint32_t *)uuid;
	struct r600_common_screen* rs = (struct r600_common_screen*)screen;

	assert(PIPE_UUID_SIZE >= sizeof(uint32_t) * 4);

	/* Copied from ac_device_info
	 * Use the device info directly instead of using a sha1. GL/VK UUIDs
	 * are 16 byte vs 20 byte for sha1, and the truncation that would be
	 * required would get rid of part of the little entropy we have.
	 */
	memset(uuid, 0, PIPE_UUID_SIZE);
	if (!rs->info.pci.valid)
		fprintf(stderr,
		"r600 device_uuid output is based on invalid pci bus info.\n");
	uint_uuid[0] = rs->info.pci.domain;
	uint_uuid[1] = rs->info.pci.bus;
	uint_uuid[2] = rs->info.pci.dev;
	uint_uuid[3] = rs->info.pci.func;
}

bool r600_common_screen_init(struct r600_common_screen *rscreen,
			     struct radeon_winsys *ws)
{
	char family_name[32] = {}, kernel_version[128] = {};
	struct utsname uname_data;
	const char *chip_name;

	ws->query_info(ws, &rscreen->info);
	rscreen->ws = ws;

	chip_name = r600_get_family_name(rscreen);

	if (uname(&uname_data) == 0)
		snprintf(kernel_version, sizeof(kernel_version),
			 " / %s", uname_data.release);

	snprintf(rscreen->renderer_string, sizeof(rscreen->renderer_string),
		 "%s (%sDRM %i.%i.%i%s"
#if LLVM_AVAILABLE
		 ", LLVM " MESA_LLVM_VERSION_STRING
#endif
		 ")",
		 chip_name, family_name, rscreen->info.drm_major,
		 rscreen->info.drm_minor, rscreen->info.drm_patchlevel,
		 kernel_version);

	rscreen->b.get_name = r600_get_name;
	rscreen->b.get_vendor = r600_get_vendor;
	rscreen->b.get_device_vendor = r600_get_device_vendor;
	rscreen->b.get_disk_shader_cache = r600_get_disk_shader_cache;
	rscreen->b.get_compute_param = r600_get_compute_param;
	rscreen->b.get_screen_fd = r600_get_screen_fd;
	rscreen->b.get_paramf = r600_get_paramf;
	rscreen->b.get_timestamp = r600_get_timestamp;
	rscreen->b.get_compiler_options = r600_get_compiler_options;
	rscreen->b.fence_finish = r600_fence_finish;
	rscreen->b.fence_reference = r600_fence_reference;
	rscreen->b.resource_destroy = r600_resource_destroy;
	rscreen->b.resource_from_user_memory = r600_buffer_from_user_memory;
	rscreen->b.query_memory_info = r600_query_memory_info;
	rscreen->b.get_device_uuid = r600_get_device_uuid;
	rscreen->b.get_driver_uuid = r600_get_driver_uuid;

	if (rscreen->info.ip[AMD_IP_UVD].num_queues) {
		rscreen->b.get_video_param = rvid_get_video_param;
		rscreen->b.is_video_format_supported = rvid_is_format_supported;
	} else {
		rscreen->b.get_video_param = r600_get_video_param;
		rscreen->b.is_video_format_supported = vl_video_buffer_is_format_supported;
	}

	r600_init_screen_texture_functions(rscreen);
	r600_init_screen_query_functions(rscreen);

	rscreen->family = rscreen->info.family;
	rscreen->gfx_level = rscreen->info.gfx_level;
	rscreen->debug_flags |= debug_get_flags_option("R600_DEBUG", common_debug_options, 0);

	r600_disk_cache_create(rscreen);

	slab_create_parent(&rscreen->pool_transfers, sizeof(struct r600_transfer), 64);

	rscreen->force_aniso = MIN2(16, debug_get_num_option("R600_TEX_ANISO", -1));
	if (rscreen->force_aniso >= 0) {
		printf("radeon: Forcing anisotropy filter to %ix\n",
		       /* round down to a power of two */
		       1 << util_logbase2(rscreen->force_aniso));
	}

	(void) mtx_init(&rscreen->aux_context_lock, mtx_plain);
	(void) mtx_init(&rscreen->gpu_load_mutex, mtx_plain);

	if (rscreen->debug_flags & DBG_INFO) {
		printf("pci (domain:bus:dev.func): %04x:%02x:%02x.%x\n",
		       rscreen->info.pci.domain, rscreen->info.pci.bus,
		       rscreen->info.pci.dev, rscreen->info.pci.func);
		printf("pci_id = 0x%x\n", rscreen->info.pci_id);
		printf("family = %i (%s)\n", rscreen->info.family,
		       r600_get_family_name(rscreen));
		printf("gfx_level = %i\n", rscreen->info.gfx_level);
		printf("pte_fragment_size = %u\n", rscreen->info.pte_fragment_size);
		printf("gart_page_size = %u\n", rscreen->info.gart_page_size);
		printf("gart_size = %i MB\n", (int)DIV_ROUND_UP(rscreen->info.gart_size_kb, 1024));
		printf("vram_size = %i MB\n", (int)DIV_ROUND_UP(rscreen->info.vram_size_kb, 1024));
		printf("vram_vis_size = %i MB\n", (int)DIV_ROUND_UP(rscreen->info.vram_vis_size_kb, 1024));
		printf("max_heap_size = %i MB\n",
		       (int)DIV_ROUND_UP(rscreen->info.max_heap_size_kb, 1024));
		printf("min_alloc_size = %u\n", rscreen->info.min_alloc_size);
		printf("has_dedicated_vram = %u\n", rscreen->info.has_dedicated_vram);
		printf("r600_has_virtual_memory = %i\n", rscreen->info.r600_has_virtual_memory);
		printf("gfx_ib_pad_with_type2 = %i\n", rscreen->info.gfx_ib_pad_with_type2);
		printf("ip[AMD_IP_UVD] = %u\n", rscreen->info.ip[AMD_IP_UVD].num_queues);
		printf("ip[AMD_IP_SDMA] = %i\n", rscreen->info.ip[AMD_IP_SDMA].num_queues);
		printf("ip[AMD_IP_COMPUTE] = %u\n", rscreen->info.ip[AMD_IP_COMPUTE].num_queues);
		printf("uvd_fw_version = %u\n", rscreen->info.uvd_fw_version);
		printf("vce_fw_version = %u\n", rscreen->info.vce_fw_version);
		printf("me_fw_version = %i\n", rscreen->info.me_fw_version);
		printf("pfp_fw_version = %i\n", rscreen->info.pfp_fw_version);
		printf("vce_harvest_config = %i\n", rscreen->info.vce_harvest_config);
		printf("clock_crystal_freq = %i\n", rscreen->info.clock_crystal_freq);
		printf("tcc_cache_line_size = %u\n", rscreen->info.tcc_cache_line_size);
		printf("drm = %i.%i.%i\n", rscreen->info.drm_major,
		       rscreen->info.drm_minor, rscreen->info.drm_patchlevel);
		printf("has_userptr = %i\n", rscreen->info.has_userptr);
		printf("has_syncobj = %u\n", rscreen->info.has_syncobj);

		printf("r600_max_quad_pipes = %i\n", rscreen->info.r600_max_quad_pipes);
		printf("max_gpu_freq_mhz = %i\n", rscreen->info.max_gpu_freq_mhz);
		printf("num_cu = %i\n", rscreen->info.num_cu);
		printf("max_se = %i\n", rscreen->info.max_se);
		printf("max_sh_per_se = %i\n", rscreen->info.max_sa_per_se);

		printf("r600_gb_backend_map = %i\n", rscreen->info.r600_gb_backend_map);
		printf("r600_gb_backend_map_valid = %i\n", rscreen->info.r600_gb_backend_map_valid);
		printf("r600_num_banks = %i\n", rscreen->info.r600_num_banks);
		printf("num_render_backends = %i\n", rscreen->info.max_render_backends);
		printf("num_tile_pipes = %i\n", rscreen->info.num_tile_pipes);
		printf("pipe_interleave_bytes = %i\n", rscreen->info.pipe_interleave_bytes);
		printf("enabled_rb_mask = 0x%" PRIx64 "\n", rscreen->info.enabled_rb_mask);
		printf("max_alignment = %u\n", (unsigned)rscreen->info.max_alignment);
	}

	const struct nir_shader_compiler_options nir_options = {
		.fuse_ffma16 = true,
		.fuse_ffma32 = true,
		.fuse_ffma64 = true,
		.lower_flrp32 = true,
		.lower_flrp64 = true,
		.lower_fdiv = true,
		.lower_isign = true,
		.lower_fsign = true,
		.lower_fmod = true,
		.lower_uadd_carry = true,
		.lower_usub_borrow = true,
		.lower_bitfield_extract = true,
		.lower_bitfield_insert = true,
		.lower_extract_byte = true,
		.lower_extract_word = true,
		.lower_insert_byte = true,
		.lower_insert_word = true,
		.lower_ldexp = true,
		/* due to a bug in the shader compiler, some loops hang
		 * if they are not unrolled, see:
		 *    https://bugs.freedesktop.org/show_bug.cgi?id=86720
		 */
		.max_unroll_iterations = 255,
		.lower_interpolate_at = true,
		.vectorize_io = true,
		.has_umad24 = true,
		.has_umul24 = true,
		.has_fmulz = true,
		.use_interpolated_input_intrinsics = true,
		.has_fsub = true,
		.has_isub = true,
		.has_find_msb_rev = true,
		.lower_iabs = true,
		.lower_uadd_sat = true,
		.lower_usub_sat = true,
		.has_fused_comp_and_csel = true,
		.lower_ifind_msb = true,
		.lower_ufind_msb = true,
		.lower_to_scalar = true,
		.lower_to_scalar_filter = r600_lower_to_scalar_instr_filter,
		.linker_ignore_precision = true,
		.lower_fpow = true,
		.lower_int64_options = ~0,
		.lower_cs_local_index_to_id = true,
		.lower_uniforms_to_ubo = true,
		.lower_image_offset_to_range_base = 1,
		.vectorize_tess_levels = 1,
	};

	rscreen->nir_options = nir_options;

	if (rscreen->info.family < CHIP_CEDAR)
		rscreen->nir_options.force_indirect_unrolling_sampler = true;

	if (rscreen->info.gfx_level >= EVERGREEN) {
		rscreen->nir_options.has_bfe = true;
		rscreen->nir_options.has_bfm = true;
		rscreen->nir_options.has_bitfield_select = true;
	}

	if (rscreen->info.gfx_level < EVERGREEN) {
		/* Pre-EG doesn't have these ALU ops */
		rscreen->nir_options.lower_bit_count = true;
		rscreen->nir_options.lower_bitfield_reverse = true;
	}

	if (rscreen->info.gfx_level < CAYMAN) {
		rscreen->nir_options.lower_doubles_options = nir_lower_fp64_full_software;
		rscreen->nir_options.lower_atomic_offset_to_range_base = true;
	} else {
		rscreen->nir_options.lower_doubles_options =
			nir_lower_ddiv |
			nir_lower_dfloor |
			nir_lower_dceil |
			nir_lower_dmod |
			nir_lower_dsub |
			nir_lower_dtrunc |
			nir_lower_dround_even;
	}

        rscreen->nir_options_fs = rscreen->nir_options;
	rscreen->nir_options_fs.lower_all_io_to_temps = true;

	return true;
}

void r600_destroy_common_screen(struct r600_common_screen *rscreen)
{
	r600_perfcounters_destroy(rscreen);
	r600_gpu_load_kill_thread(rscreen);

	mtx_destroy(&rscreen->gpu_load_mutex);
	mtx_destroy(&rscreen->aux_context_lock);
	rscreen->aux_context->destroy(rscreen->aux_context);

	slab_destroy_parent(&rscreen->pool_transfers);

	disk_cache_destroy(rscreen->disk_shader_cache);
	rscreen->ws->destroy(rscreen->ws);
	FREE(rscreen);
}

bool r600_can_dump_shader(struct r600_common_screen *rscreen,
			  unsigned processor)
{
	return rscreen->debug_flags & (1 << processor);
}

bool r600_extra_shader_checks(struct r600_common_screen *rscreen, unsigned processor)
{
	return (rscreen->debug_flags & DBG_CHECK_IR) ||
	       r600_can_dump_shader(rscreen, processor);
}

void r600_screen_clear_buffer(struct r600_common_screen *rscreen, struct pipe_resource *dst,
			      uint64_t offset, uint64_t size, unsigned value)
{
	struct r600_common_context *rctx = (struct r600_common_context*)rscreen->aux_context;

	mtx_lock(&rscreen->aux_context_lock);
	rctx->dma_clear_buffer(&rctx->b, dst, offset, size, value);
	rscreen->aux_context->flush(rscreen->aux_context, NULL, 0);
	mtx_unlock(&rscreen->aux_context_lock);
}
