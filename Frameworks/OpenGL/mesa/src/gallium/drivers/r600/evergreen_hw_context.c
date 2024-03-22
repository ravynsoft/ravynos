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
#include "evergreend.h"
#include "util/u_memory.h"
#include "util/u_math.h"

void evergreen_dma_copy_buffer(struct r600_context *rctx,
			       struct pipe_resource *dst,
			       struct pipe_resource *src,
			       uint64_t dst_offset,
			       uint64_t src_offset,
			       uint64_t size)
{
	struct radeon_cmdbuf *cs = &rctx->b.dma.cs;
	unsigned i, ncopy, csize, sub_cmd, shift;
	struct r600_resource *rdst = (struct r600_resource*)dst;
	struct r600_resource *rsrc = (struct r600_resource*)src;

	/* Mark the buffer range of destination as valid (initialized),
	 * so that transfer_map knows it should wait for the GPU when mapping
	 * that range. */
	util_range_add(&rdst->b.b, &rdst->valid_buffer_range, dst_offset,
		       dst_offset + size);

	dst_offset += rdst->gpu_address;
	src_offset += rsrc->gpu_address;

	/* see if we use dword or byte copy */
	if (!(dst_offset % 4) && !(src_offset % 4) && !(size % 4)) {
		size >>= 2;
		sub_cmd = EG_DMA_COPY_DWORD_ALIGNED;
		shift = 2;
	} else {
		sub_cmd = EG_DMA_COPY_BYTE_ALIGNED;
		shift = 0;
	}
	ncopy = (size / EG_DMA_COPY_MAX_SIZE) + !!(size % EG_DMA_COPY_MAX_SIZE);

	r600_need_dma_space(&rctx->b, ncopy * 5, rdst, rsrc);
	for (i = 0; i < ncopy; i++) {
		csize = size < EG_DMA_COPY_MAX_SIZE ? size : EG_DMA_COPY_MAX_SIZE;
		/* emit reloc before writing cs so that cs is always in consistent state */
		radeon_add_to_buffer_list(&rctx->b, &rctx->b.dma, rsrc, RADEON_USAGE_READ);
		radeon_add_to_buffer_list(&rctx->b, &rctx->b.dma, rdst, RADEON_USAGE_WRITE);
		radeon_emit(cs, DMA_PACKET(DMA_PACKET_COPY, sub_cmd, csize));
		radeon_emit(cs, dst_offset & 0xffffffff);
		radeon_emit(cs, src_offset & 0xffffffff);
		radeon_emit(cs, (dst_offset >> 32UL) & 0xff);
		radeon_emit(cs, (src_offset >> 32UL) & 0xff);
		dst_offset += csize << shift;
		src_offset += csize << shift;
		size -= csize;
	}
}

/* The max number of bytes to copy per packet. */
#define CP_DMA_MAX_BYTE_COUNT ((1 << 21) - 8)

void evergreen_cp_dma_clear_buffer(struct r600_context *rctx,
				   struct pipe_resource *dst, uint64_t offset,
				   unsigned size, uint32_t clear_value,
				   enum r600_coherency coher)
{
	struct radeon_cmdbuf *cs = &rctx->b.gfx.cs;

	assert(size);
	assert(rctx->screen->b.has_cp_dma);

	/* Mark the buffer range of destination as valid (initialized),
	 * so that transfer_map knows it should wait for the GPU when mapping
	 * that range. */
	util_range_add(dst, &r600_resource(dst)->valid_buffer_range, offset,
		       offset + size);

	offset += r600_resource(dst)->gpu_address;

	/* Flush the cache where the resource is bound. */
	rctx->b.flags |= r600_get_flush_flags(coher) |
			 R600_CONTEXT_WAIT_3D_IDLE;

	while (size) {
		unsigned sync = 0;
		unsigned byte_count = MIN2(size, CP_DMA_MAX_BYTE_COUNT);
		unsigned reloc;

		r600_need_cs_space(rctx,
				   10 + (rctx->b.flags ? R600_MAX_FLUSH_CS_DWORDS : 0) +
				   R600_MAX_PFP_SYNC_ME_DWORDS, false, 0);

		/* Flush the caches for the first copy only. */
		if (rctx->b.flags) {
			r600_flush_emit(rctx);
		}

		/* Do the synchronization after the last copy, so that all data is written to memory. */
		if (size == byte_count) {
			sync = PKT3_CP_DMA_CP_SYNC;
		}

		/* This must be done after r600_need_cs_space. */
		reloc = radeon_add_to_buffer_list(&rctx->b, &rctx->b.gfx,
					      (struct r600_resource*)dst, RADEON_USAGE_WRITE |
					      RADEON_PRIO_CP_DMA);

		radeon_emit(cs, PKT3(PKT3_CP_DMA, 4, 0));
		radeon_emit(cs, clear_value);	/* DATA [31:0] */
		radeon_emit(cs, sync | PKT3_CP_DMA_SRC_SEL(2));	/* CP_SYNC [31] | SRC_SEL[30:29] */
		radeon_emit(cs, offset);	/* DST_ADDR_LO [31:0] */
		radeon_emit(cs, (offset >> 32) & 0xff);		/* DST_ADDR_HI [7:0] */
		radeon_emit(cs, byte_count);	/* COMMAND [29:22] | BYTE_COUNT [20:0] */

		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
		radeon_emit(cs, reloc);

		size -= byte_count;
		offset += byte_count;
	}

	/* CP DMA is executed in ME, but index buffers are read by PFP.
	 * This ensures that ME (CP DMA) is idle before PFP starts fetching
	 * indices. If we wanted to execute CP DMA in PFP, this packet
	 * should precede it.
	 */
	if (coher == R600_COHERENCY_SHADER)
		r600_emit_pfp_sync_me(rctx);
}
