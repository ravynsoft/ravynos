/*
 * Copyright 2013 Advanced Micro Devices, Inc.
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
 * Authors: Marek Olšák <maraeo@gmail.com>
 */

/**
 * This file contains helpers for writing commands to commands streams.
 */

#ifndef R600_CS_H
#define R600_CS_H

#include "r600_pipe_common.h"
#include "r600d_common.h"

/**
 * Return true if there is enough memory in VRAM and GTT for the buffers
 * added so far.
 *
 * \param vram      VRAM memory size not added to the buffer list yet
 * \param gtt       GTT memory size not added to the buffer list yet
 */
static inline bool
radeon_cs_memory_below_limit(struct r600_common_screen *screen,
			     struct radeon_cmdbuf *cs,
			     uint64_t vram, uint64_t gtt)
{
	vram += (uint64_t)cs->used_vram_kb * 1024;
	gtt += (uint64_t)cs->used_gart_kb * 1024;

	/* Anything that goes above the VRAM size should go to GTT. */
	if (vram > (uint64_t)screen->info.vram_size_kb * 1024)
		gtt += vram - (uint64_t)screen->info.vram_size_kb * 1024;

	/* Now we just need to check if we have enough GTT. */
	return gtt < (uint64_t)screen->info.gart_size_kb * 1024 * 0.7;
}

/**
 * Add a buffer to the buffer list for the given command stream (CS).
 *
 * All buffers used by a CS must be added to the list. This tells the kernel
 * driver which buffers are used by GPU commands. Other buffers can
 * be swapped out (not accessible) during execution.
 *
 * The buffer list becomes empty after every context flush and must be
 * rebuilt.
 */
static inline unsigned radeon_add_to_buffer_list(struct r600_common_context *rctx,
						 struct r600_ring *ring,
						 struct r600_resource *rbo,
						 unsigned usage)
{
	assert(usage);
	return rctx->ws->cs_add_buffer(
		&ring->cs, rbo->buf,
		usage | RADEON_USAGE_SYNCHRONIZED,
		rbo->domains) * 4;
}

/**
 * Same as above, but also checks memory usage and flushes the context
 * accordingly.
 *
 * When this SHOULD NOT be used:
 *
 * - if r600_context_add_resource_size has been called for the buffer
 *   followed by *_need_cs_space for checking the memory usage
 *
 * - if r600_need_dma_space has been called for the buffer
 *
 * - when emitting state packets and draw packets (because preceding packets
 *   can't be re-emitted at that point)
 *
 * - if shader resource "enabled_mask" is not up-to-date or there is
 *   a different constraint disallowing a context flush
 */
static inline unsigned
radeon_add_to_buffer_list_check_mem(struct r600_common_context *rctx,
				    struct r600_ring *ring,
				    struct r600_resource *rbo,
				    unsigned usage,
				    bool check_mem)
{
	if (check_mem &&
	    !radeon_cs_memory_below_limit(rctx->screen, &ring->cs,
					  rctx->vram + rbo->vram_usage,
					  rctx->gtt + rbo->gart_usage))
		ring->flush(rctx, PIPE_FLUSH_ASYNC, NULL);

	return radeon_add_to_buffer_list(rctx, ring, rbo, usage);
}

static inline void r600_emit_reloc(struct r600_common_context *rctx,
				   struct r600_ring *ring, struct r600_resource *rbo,
				   unsigned usage)
{
	struct radeon_cmdbuf *cs = &ring->cs;
	bool has_vm = ((struct r600_common_screen*)rctx->b.screen)->info.r600_has_virtual_memory;
	unsigned reloc = radeon_add_to_buffer_list(rctx, ring, rbo, usage);

	if (!has_vm) {
		radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
		radeon_emit(cs, reloc);
	}
}

static inline void radeon_set_config_reg_seq(struct radeon_cmdbuf *cs, unsigned reg, unsigned num)
{
	assert(reg < R600_CONTEXT_REG_OFFSET);
	assert(cs->current.cdw + 2 + num <= cs->current.max_dw);
	radeon_emit(cs, PKT3(PKT3_SET_CONFIG_REG, num, 0));
	radeon_emit(cs, (reg - R600_CONFIG_REG_OFFSET) >> 2);
}

static inline void radeon_set_config_reg(struct radeon_cmdbuf *cs, unsigned reg, unsigned value)
{
	radeon_set_config_reg_seq(cs, reg, 1);
	radeon_emit(cs, value);
}

static inline void radeon_set_context_reg_seq(struct radeon_cmdbuf *cs, unsigned reg, unsigned num)
{
	assert(reg >= R600_CONTEXT_REG_OFFSET);
	assert(cs->current.cdw + 2 + num <= cs->current.max_dw);
	radeon_emit(cs, PKT3(PKT3_SET_CONTEXT_REG, num, 0));
	radeon_emit(cs, (reg - R600_CONTEXT_REG_OFFSET) >> 2);
}

static inline void radeon_set_context_reg(struct radeon_cmdbuf *cs, unsigned reg, unsigned value)
{
	radeon_set_context_reg_seq(cs, reg, 1);
	radeon_emit(cs, value);
}

static inline void radeon_set_context_reg_idx(struct radeon_cmdbuf *cs,
					      unsigned reg, unsigned idx,
					      unsigned value)
{
	assert(reg >= R600_CONTEXT_REG_OFFSET);
	assert(cs->current.cdw + 3 <= cs->current.max_dw);
	radeon_emit(cs, PKT3(PKT3_SET_CONTEXT_REG, 1, 0));
	radeon_emit(cs, (reg - R600_CONTEXT_REG_OFFSET) >> 2 | (idx << 28));
	radeon_emit(cs, value);
}

static inline void radeon_set_sh_reg_seq(struct radeon_cmdbuf *cs, unsigned reg, unsigned num)
{
	assert(reg >= SI_SH_REG_OFFSET && reg < SI_SH_REG_END);
	assert(cs->current.cdw + 2 + num <= cs->current.max_dw);
	radeon_emit(cs, PKT3(PKT3_SET_SH_REG, num, 0));
	radeon_emit(cs, (reg - SI_SH_REG_OFFSET) >> 2);
}

static inline void radeon_set_sh_reg(struct radeon_cmdbuf *cs, unsigned reg, unsigned value)
{
	radeon_set_sh_reg_seq(cs, reg, 1);
	radeon_emit(cs, value);
}

static inline void radeon_set_uconfig_reg_seq(struct radeon_cmdbuf *cs, unsigned reg, unsigned num)
{
	assert(reg >= CIK_UCONFIG_REG_OFFSET && reg < CIK_UCONFIG_REG_END);
	assert(cs->current.cdw + 2 + num <= cs->current.max_dw);
	radeon_emit(cs, PKT3(PKT3_SET_UCONFIG_REG, num, 0));
	radeon_emit(cs, (reg - CIK_UCONFIG_REG_OFFSET) >> 2);
}

static inline void radeon_set_uconfig_reg(struct radeon_cmdbuf *cs, unsigned reg, unsigned value)
{
	radeon_set_uconfig_reg_seq(cs, reg, 1);
	radeon_emit(cs, value);
}

static inline void radeon_set_uconfig_reg_idx(struct radeon_cmdbuf *cs,
					      unsigned reg, unsigned idx,
					      unsigned value)
{
	assert(reg >= CIK_UCONFIG_REG_OFFSET && reg < CIK_UCONFIG_REG_END);
	assert(cs->current.cdw + 3 <= cs->current.max_dw);
	radeon_emit(cs, PKT3(PKT3_SET_UCONFIG_REG, 1, 0));
	radeon_emit(cs, (reg - CIK_UCONFIG_REG_OFFSET) >> 2 | (idx << 28));
	radeon_emit(cs, value);
}

#endif
