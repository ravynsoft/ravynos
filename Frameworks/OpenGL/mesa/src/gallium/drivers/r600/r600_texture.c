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
 *      Corbin Simpson
 */
#include "r600_pipe_common.h"
#include "r600_cs.h"
#include "r600_query.h"
#include "util/format/u_format.h"
#include "util/u_log.h"
#include "util/u_memory.h"
#include "util/u_pack_color.h"
#include "util/u_surface.h"
#include "util/os_time.h"
#include "frontend/winsys_handle.h"
#include <errno.h>
#include <inttypes.h>

static void r600_texture_discard_cmask(struct r600_common_screen *rscreen,
				       struct r600_texture *rtex);
static enum radeon_surf_mode
r600_choose_tiling(struct r600_common_screen *rscreen,
		   const struct pipe_resource *templ);


bool r600_prepare_for_dma_blit(struct r600_common_context *rctx,
			       struct r600_texture *rdst,
			       unsigned dst_level, unsigned dstx,
			       unsigned dsty, unsigned dstz,
			       struct r600_texture *rsrc,
			       unsigned src_level,
			       const struct pipe_box *src_box)
{
	if (!rctx->dma.cs.priv)
		return false;

	if (rdst->surface.bpe != rsrc->surface.bpe)
		return false;

	/* MSAA: Blits don't exist in the real world. */
	if (rsrc->resource.b.b.nr_samples > 1 ||
	    rdst->resource.b.b.nr_samples > 1)
		return false;

	/* Depth-stencil surfaces:
	 *   When dst is linear, the DB->CB copy preserves HTILE.
	 *   When dst is tiled, the 3D path must be used to update HTILE.
	 */
	if (rsrc->is_depth || rdst->is_depth)
		return false;

	/* CMASK as:
	 *   src: Both texture and SDMA paths need decompression. Use SDMA.
	 *   dst: If overwriting the whole texture, discard CMASK and use
	 *        SDMA. Otherwise, use the 3D path.
	 */
	if (rdst->cmask.size && rdst->dirty_level_mask & (1 << dst_level)) {
		/* The CMASK clear is only enabled for the first level. */
		assert(dst_level == 0);
		if (!util_texrange_covers_whole_level(&rdst->resource.b.b, dst_level,
						      dstx, dsty, dstz, src_box->width,
						      src_box->height, src_box->depth))
			return false;

		r600_texture_discard_cmask(rctx->screen, rdst);
	}

	/* All requirements are met. Prepare textures for SDMA. */
	if (rsrc->cmask.size && rsrc->dirty_level_mask & (1 << src_level))
		rctx->b.flush_resource(&rctx->b, &rsrc->resource.b.b);

	assert(!(rsrc->dirty_level_mask & (1 << src_level)));
	assert(!(rdst->dirty_level_mask & (1 << dst_level)));

	return true;
}

/* Same as resource_copy_region, except that both upsampling and downsampling are allowed. */
static void r600_copy_region_with_blit(struct pipe_context *pipe,
				       struct pipe_resource *dst,
                                       unsigned dst_level,
                                       unsigned dstx, unsigned dsty, unsigned dstz,
                                       struct pipe_resource *src,
                                       unsigned src_level,
                                       const struct pipe_box *src_box)
{
	struct pipe_blit_info blit;

	memset(&blit, 0, sizeof(blit));
	blit.src.resource = src;
	blit.src.format = src->format;
	blit.src.level = src_level;
	blit.src.box = *src_box;
	blit.dst.resource = dst;
	blit.dst.format = dst->format;
	blit.dst.level = dst_level;
	blit.dst.box.x = dstx;
	blit.dst.box.y = dsty;
	blit.dst.box.z = dstz;
	blit.dst.box.width = src_box->width;
	blit.dst.box.height = src_box->height;
	blit.dst.box.depth = src_box->depth;
	blit.mask = util_format_get_mask(src->format) &
		    util_format_get_mask(dst->format);
	blit.filter = PIPE_TEX_FILTER_NEAREST;

	if (blit.mask) {
		pipe->blit(pipe, &blit);
	}
}

/* Copy from a full GPU texture to a transfer's staging one. */
static void r600_copy_to_staging_texture(struct pipe_context *ctx, struct r600_transfer *rtransfer)
{
	struct r600_common_context *rctx = (struct r600_common_context*)ctx;
	struct pipe_transfer *transfer = (struct pipe_transfer*)rtransfer;
	struct pipe_resource *dst = &rtransfer->staging->b.b;
	struct pipe_resource *src = transfer->resource;

	if (src->nr_samples > 1) {
		r600_copy_region_with_blit(ctx, dst, 0, 0, 0, 0,
					   src, transfer->level, &transfer->box);
		return;
	}

	rctx->dma_copy(ctx, dst, 0, 0, 0, 0, src, transfer->level,
		       &transfer->box);
}

/* Copy from a transfer's staging texture to a full GPU one. */
static void r600_copy_from_staging_texture(struct pipe_context *ctx, struct r600_transfer *rtransfer)
{
	struct r600_common_context *rctx = (struct r600_common_context*)ctx;
	struct pipe_transfer *transfer = (struct pipe_transfer*)rtransfer;
	struct pipe_resource *dst = transfer->resource;
	struct pipe_resource *src = &rtransfer->staging->b.b;
	struct pipe_box sbox;

	u_box_3d(0, 0, 0, transfer->box.width, transfer->box.height, transfer->box.depth, &sbox);

	if (dst->nr_samples > 1) {
		r600_copy_region_with_blit(ctx, dst, transfer->level,
					   transfer->box.x, transfer->box.y, transfer->box.z,
					   src, 0, &sbox);
		return;
	}

	rctx->dma_copy(ctx, dst, transfer->level,
		       transfer->box.x, transfer->box.y, transfer->box.z,
		       src, 0, &sbox);
}

static unsigned r600_texture_get_offset(struct r600_common_screen *rscreen,
					struct r600_texture *rtex, unsigned level,
					const struct pipe_box *box,
					unsigned *stride,
					uintptr_t *layer_stride)
{
	*stride = rtex->surface.u.legacy.level[level].nblk_x *
		rtex->surface.bpe;
	assert((uint64_t)rtex->surface.u.legacy.level[level].slice_size_dw * 4 <= UINT_MAX);
	*layer_stride = (uint64_t)rtex->surface.u.legacy.level[level].slice_size_dw * 4;

	if (!box)
		return (uint64_t)rtex->surface.u.legacy.level[level].offset_256B * 256;

	/* Each texture is an array of mipmap levels. Each level is
	 * an array of slices. */
	return (uint64_t)rtex->surface.u.legacy.level[level].offset_256B * 256 +
		box->z * (uint64_t)rtex->surface.u.legacy.level[level].slice_size_dw * 4 +
		(box->y / rtex->surface.blk_h *
		 rtex->surface.u.legacy.level[level].nblk_x +
		 box->x / rtex->surface.blk_w) * rtex->surface.bpe;
}

static int r600_init_surface(struct r600_common_screen *rscreen,
			     struct radeon_surf *surface,
			     const struct pipe_resource *ptex,
			     enum radeon_surf_mode array_mode,
			     unsigned pitch_in_bytes_override,
			     unsigned offset,
			     bool is_imported,
			     bool is_scanout,
			     bool is_flushed_depth)
{
	const struct util_format_description *desc =
		util_format_description(ptex->format);
	bool is_depth, is_stencil;
	int r;
	unsigned i, bpe, flags = 0;

	is_depth = util_format_has_depth(desc);
	is_stencil = util_format_has_stencil(desc);

	if (rscreen->gfx_level >= EVERGREEN && !is_flushed_depth &&
	    ptex->format == PIPE_FORMAT_Z32_FLOAT_S8X24_UINT) {
		bpe = 4; /* stencil is allocated separately on evergreen */
	} else {
		bpe = util_format_get_blocksize(ptex->format);
		assert(util_is_power_of_two_or_zero(bpe));
	}

	if (!is_flushed_depth && is_depth) {
		flags |= RADEON_SURF_ZBUFFER;

		if (is_stencil)
			flags |= RADEON_SURF_SBUFFER;
	}

	if (ptex->bind & PIPE_BIND_SCANOUT || is_scanout) {
		/* This should catch bugs in gallium users setting incorrect flags. */
		assert(ptex->nr_samples <= 1 &&
		       ptex->array_size == 1 &&
		       ptex->depth0 == 1 &&
		       ptex->last_level == 0 &&
		       !(flags & RADEON_SURF_Z_OR_SBUFFER));

		flags |= RADEON_SURF_SCANOUT;
	}

	if (ptex->bind & PIPE_BIND_SHARED)
		flags |= RADEON_SURF_SHAREABLE;
	if (is_imported)
		flags |= RADEON_SURF_IMPORTED | RADEON_SURF_SHAREABLE;

	r = rscreen->ws->surface_init(rscreen->ws, &rscreen->info, ptex,
				      flags, bpe, array_mode, surface);
	if (r) {
		return r;
	}

	if (pitch_in_bytes_override &&
	    pitch_in_bytes_override != surface->u.legacy.level[0].nblk_x * bpe) {
		/* old ddx on evergreen over estimate alignment for 1d, only 1 level
		 * for those
		 */
		surface->u.legacy.level[0].nblk_x = pitch_in_bytes_override / bpe;
		surface->u.legacy.level[0].slice_size_dw =
			((uint64_t)pitch_in_bytes_override * surface->u.legacy.level[0].nblk_y) / 4;
	}

	if (offset) {
		for (i = 0; i < ARRAY_SIZE(surface->u.legacy.level); ++i)
			surface->u.legacy.level[i].offset_256B += offset / 256;
	}

	return 0;
}

static void r600_texture_init_metadata(struct r600_common_screen *rscreen,
				       struct r600_texture *rtex,
				       struct radeon_bo_metadata *metadata)
{
	struct radeon_surf *surface = &rtex->surface;

	memset(metadata, 0, sizeof(*metadata));

	metadata->u.legacy.microtile = surface->u.legacy.level[0].mode >= RADEON_SURF_MODE_1D ?
		RADEON_LAYOUT_TILED : RADEON_LAYOUT_LINEAR;
	metadata->u.legacy.macrotile = surface->u.legacy.level[0].mode >= RADEON_SURF_MODE_2D ?
		RADEON_LAYOUT_TILED : RADEON_LAYOUT_LINEAR;
	metadata->u.legacy.pipe_config = surface->u.legacy.pipe_config;
	metadata->u.legacy.bankw = surface->u.legacy.bankw;
	metadata->u.legacy.bankh = surface->u.legacy.bankh;
	metadata->u.legacy.tile_split = surface->u.legacy.tile_split;
	metadata->u.legacy.mtilea = surface->u.legacy.mtilea;
	metadata->u.legacy.num_banks = surface->u.legacy.num_banks;
	metadata->u.legacy.stride = surface->u.legacy.level[0].nblk_x * surface->bpe;
	metadata->u.legacy.scanout = (surface->flags & RADEON_SURF_SCANOUT) != 0;
}

static void r600_surface_import_metadata(struct r600_common_screen *rscreen,
					 struct radeon_surf *surf,
					 struct radeon_bo_metadata *metadata,
					 enum radeon_surf_mode *array_mode,
					 bool *is_scanout)
{
	surf->u.legacy.pipe_config = metadata->u.legacy.pipe_config;
	surf->u.legacy.bankw = metadata->u.legacy.bankw;
	surf->u.legacy.bankh = metadata->u.legacy.bankh;
	surf->u.legacy.tile_split = metadata->u.legacy.tile_split;
	surf->u.legacy.mtilea = metadata->u.legacy.mtilea;
	surf->u.legacy.num_banks = metadata->u.legacy.num_banks;

	if (metadata->u.legacy.macrotile == RADEON_LAYOUT_TILED)
		*array_mode = RADEON_SURF_MODE_2D;
	else if (metadata->u.legacy.microtile == RADEON_LAYOUT_TILED)
		*array_mode = RADEON_SURF_MODE_1D;
	else
		*array_mode = RADEON_SURF_MODE_LINEAR_ALIGNED;

	*is_scanout = metadata->u.legacy.scanout;
}

static void r600_eliminate_fast_color_clear(struct r600_common_context *rctx,
					    struct r600_texture *rtex)
{
	struct r600_common_screen *rscreen = rctx->screen;
	struct pipe_context *ctx = &rctx->b;

	if (ctx == rscreen->aux_context)
		mtx_lock(&rscreen->aux_context_lock);

	ctx->flush_resource(ctx, &rtex->resource.b.b);
	ctx->flush(ctx, NULL, 0);

	if (ctx == rscreen->aux_context)
		mtx_unlock(&rscreen->aux_context_lock);
}

static void r600_texture_discard_cmask(struct r600_common_screen *rscreen,
				       struct r600_texture *rtex)
{
	if (!rtex->cmask.size)
		return;

	assert(rtex->resource.b.b.nr_samples <= 1);

	/* Disable CMASK. */
	memset(&rtex->cmask, 0, sizeof(rtex->cmask));
	rtex->cmask.base_address_reg = rtex->resource.gpu_address >> 8;
	rtex->dirty_level_mask = 0;

	rtex->cb_color_info &= ~EG_S_028C70_FAST_CLEAR(1);

	if (rtex->cmask_buffer != &rtex->resource)
	    r600_resource_reference(&rtex->cmask_buffer, NULL);

	/* Notify all contexts about the change. */
	p_atomic_inc(&rscreen->dirty_tex_counter);
	p_atomic_inc(&rscreen->compressed_colortex_counter);
}

static void r600_reallocate_texture_inplace(struct r600_common_context *rctx,
					    struct r600_texture *rtex,
					    unsigned new_bind_flag,
					    bool invalidate_storage)
{
	struct pipe_screen *screen = rctx->b.screen;
	struct r600_texture *new_tex;
	struct pipe_resource templ = rtex->resource.b.b;
	unsigned i;

	templ.bind |= new_bind_flag;

	/* r600g doesn't react to dirty_tex_descriptor_counter */
	if (rctx->gfx_level < GFX6)
		return;

	if (rtex->resource.b.is_shared)
		return;

	if (new_bind_flag == PIPE_BIND_LINEAR) {
		if (rtex->surface.is_linear)
			return;

		/* This fails with MSAA, depth, and compressed textures. */
		if (r600_choose_tiling(rctx->screen, &templ) !=
		    RADEON_SURF_MODE_LINEAR_ALIGNED)
			return;
	}

	new_tex = (struct r600_texture*)screen->resource_create(screen, &templ);
	if (!new_tex)
		return;

	/* Copy the pixels to the new texture. */
	if (!invalidate_storage) {
		for (i = 0; i <= templ.last_level; i++) {
			struct pipe_box box;

			u_box_3d(0, 0, 0,
				 u_minify(templ.width0, i), u_minify(templ.height0, i),
				 util_num_layers(&templ, i), &box);

			rctx->dma_copy(&rctx->b, &new_tex->resource.b.b, i, 0, 0, 0,
				       &rtex->resource.b.b, i, &box);
		}
	}

	if (new_bind_flag == PIPE_BIND_LINEAR) {
		r600_texture_discard_cmask(rctx->screen, rtex);
	}

	/* Replace the structure fields of rtex. */
	rtex->resource.b.b.bind = templ.bind;
	radeon_bo_reference(rctx->ws, &rtex->resource.buf, new_tex->resource.buf);
	rtex->resource.gpu_address = new_tex->resource.gpu_address;
	rtex->resource.vram_usage = new_tex->resource.vram_usage;
	rtex->resource.gart_usage = new_tex->resource.gart_usage;
	rtex->resource.bo_size = new_tex->resource.bo_size;
	rtex->resource.bo_alignment = new_tex->resource.bo_alignment;
	rtex->resource.domains = new_tex->resource.domains;
	rtex->resource.flags = new_tex->resource.flags;
	rtex->size = new_tex->size;
	rtex->db_render_format = new_tex->db_render_format;
	rtex->db_compatible = new_tex->db_compatible;
	rtex->can_sample_z = new_tex->can_sample_z;
	rtex->can_sample_s = new_tex->can_sample_s;
	rtex->surface = new_tex->surface;
	rtex->fmask = new_tex->fmask;
	rtex->cmask = new_tex->cmask;
	rtex->cb_color_info = new_tex->cb_color_info;
	rtex->last_msaa_resolve_target_micro_mode = new_tex->last_msaa_resolve_target_micro_mode;
	rtex->htile_offset = new_tex->htile_offset;
	rtex->depth_cleared = new_tex->depth_cleared;
	rtex->stencil_cleared = new_tex->stencil_cleared;
	rtex->non_disp_tiling = new_tex->non_disp_tiling;
	rtex->framebuffers_bound = new_tex->framebuffers_bound;

	if (new_bind_flag == PIPE_BIND_LINEAR) {
		assert(!rtex->htile_offset);
		assert(!rtex->cmask.size);
		assert(!rtex->fmask.size);
		assert(!rtex->is_depth);
	}

	r600_texture_reference(&new_tex, NULL);

	p_atomic_inc(&rctx->screen->dirty_tex_counter);
}

static void r600_texture_get_info(struct pipe_screen* screen,
				  struct pipe_resource *resource,
				  unsigned *pstride,
				  unsigned *poffset)
{
	struct r600_common_screen *rscreen = (struct r600_common_screen*)screen;
	struct r600_texture *rtex = (struct r600_texture*)resource;
	unsigned stride = 0;
	unsigned offset = 0;

	if (!rscreen || !rtex)
		return;

	if (resource->target != PIPE_BUFFER) {
		offset = (uint64_t)rtex->surface.u.legacy.level[0].offset_256B * 256;
		stride = rtex->surface.u.legacy.level[0].nblk_x *
			 rtex->surface.bpe;
	}

	if (pstride)
		*pstride = stride;

	if (poffset)
		*poffset = offset;
}

static bool r600_texture_get_handle(struct pipe_screen* screen,
				    struct pipe_context *ctx,
				    struct pipe_resource *resource,
				    struct winsys_handle *whandle,
				    unsigned usage)
{
	struct r600_common_screen *rscreen = (struct r600_common_screen*)screen;
	struct r600_common_context *rctx;
	struct r600_resource *res = (struct r600_resource*)resource;
	struct r600_texture *rtex = (struct r600_texture*)resource;
	struct radeon_bo_metadata metadata;
	bool update_metadata = false;
	unsigned stride, offset, slice_size;

	ctx = threaded_context_unwrap_sync(ctx);
	rctx = (struct r600_common_context*)(ctx ? ctx : rscreen->aux_context);

	if (resource->target != PIPE_BUFFER) {
		/* This is not supported now, but it might be required for OpenCL
		 * interop in the future.
		 */
		if (resource->nr_samples > 1 || rtex->is_depth)
			return false;

		/* Move a suballocated texture into a non-suballocated allocation. */
		if (rscreen->ws->buffer_is_suballocated(res->buf) ||
		    rtex->surface.tile_swizzle) {
			assert(!res->b.is_shared);
			r600_reallocate_texture_inplace(rctx, rtex,
							PIPE_BIND_SHARED, false);
			rctx->b.flush(&rctx->b, NULL, 0);
			assert(res->b.b.bind & PIPE_BIND_SHARED);
			assert(res->flags & RADEON_FLAG_NO_SUBALLOC);
			assert(rtex->surface.tile_swizzle == 0);
		}

		if (!(usage & PIPE_HANDLE_USAGE_EXPLICIT_FLUSH) &&
		    rtex->cmask.size) {
			/* Eliminate fast clear (CMASK) */
			r600_eliminate_fast_color_clear(rctx, rtex);

			/* Disable CMASK if flush_resource isn't going
			 * to be called.
			 */
			if (rtex->cmask.size)
				r600_texture_discard_cmask(rscreen, rtex);
		}

		/* Set metadata. */
		if (!res->b.is_shared || update_metadata) {
			r600_texture_init_metadata(rscreen, rtex, &metadata);

			rscreen->ws->buffer_set_metadata(rscreen->ws, res->buf, &metadata, NULL);
		}

		slice_size = (uint64_t)rtex->surface.u.legacy.level[0].slice_size_dw * 4;
	} else {
		/* Move a suballocated buffer into a non-suballocated allocation. */
		if (rscreen->ws->buffer_is_suballocated(res->buf)) {
			assert(!res->b.is_shared);

			/* Allocate a new buffer with PIPE_BIND_SHARED. */
			struct pipe_resource templ = res->b.b;
			templ.bind |= PIPE_BIND_SHARED;

			struct pipe_resource *newb =
				screen->resource_create(screen, &templ);
			if (!newb)
				return false;

			/* Copy the old buffer contents to the new one. */
			struct pipe_box box;
			u_box_1d(0, newb->width0, &box);
			rctx->b.resource_copy_region(&rctx->b, newb, 0, 0, 0, 0,
						     &res->b.b, 0, &box);
			/* Move the new buffer storage to the old pipe_resource. */
			r600_replace_buffer_storage(&rctx->b, &res->b.b, newb);
			pipe_resource_reference(&newb, NULL);

			assert(res->b.b.bind & PIPE_BIND_SHARED);
			assert(res->flags & RADEON_FLAG_NO_SUBALLOC);
		}

		/* Buffers */
		slice_size = 0;
	}

	r600_texture_get_info(screen, resource, &stride, &offset);

	if (res->b.is_shared) {
		/* USAGE_EXPLICIT_FLUSH must be cleared if at least one user
		 * doesn't set it.
		 */
		res->external_usage |= usage & ~PIPE_HANDLE_USAGE_EXPLICIT_FLUSH;
		if (!(usage & PIPE_HANDLE_USAGE_EXPLICIT_FLUSH))
			res->external_usage &= ~PIPE_HANDLE_USAGE_EXPLICIT_FLUSH;
	} else {
		res->b.is_shared = true;
		res->external_usage = usage;
	}

	whandle->stride = stride;
	whandle->offset = offset + slice_size * whandle->layer;

	return rscreen->ws->buffer_get_handle(rscreen->ws, res->buf, whandle);
}

void r600_texture_destroy(struct pipe_screen *screen, struct pipe_resource *ptex)
{
	struct r600_common_screen *rscreen = (struct r600_common_screen*)screen;
	struct r600_texture *rtex = (struct r600_texture*)ptex;
	struct r600_resource *resource = &rtex->resource;

	r600_texture_reference(&rtex->flushed_depth_texture, NULL);
	pipe_resource_reference((struct pipe_resource**)&resource->immed_buffer, NULL);

	if (rtex->cmask_buffer != &rtex->resource) {
	    r600_resource_reference(&rtex->cmask_buffer, NULL);
	}
	radeon_bo_reference(rscreen->ws, &resource->buf, NULL);
	FREE(rtex);
}

/* The number of samples can be specified independently of the texture. */
void r600_texture_get_fmask_info(struct r600_common_screen *rscreen,
				 struct r600_texture *rtex,
				 unsigned nr_samples,
				 struct r600_fmask_info *out)
{
	/* FMASK is allocated like an ordinary texture. */
	struct pipe_resource templ = rtex->resource.b.b;
	struct radeon_surf fmask = {};
	unsigned flags, bpe;

	memset(out, 0, sizeof(*out));

	templ.nr_samples = 1;
	flags = rtex->surface.flags | RADEON_SURF_FMASK;

	/* Use the same parameters and tile mode. */
	fmask.u.legacy.bankw = rtex->surface.u.legacy.bankw;
	fmask.u.legacy.bankh = rtex->surface.u.legacy.bankh;
	fmask.u.legacy.mtilea = rtex->surface.u.legacy.mtilea;
	fmask.u.legacy.tile_split = rtex->surface.u.legacy.tile_split;

	if (nr_samples <= 4)
		fmask.u.legacy.bankh = 4;

	switch (nr_samples) {
	case 2:
	case 4:
		bpe = 1;
		break;
	case 8:
		bpe = 4;
		break;
	default:
		R600_ERR("Invalid sample count for FMASK allocation.\n");
		return;
	}

	/* Overallocate FMASK on R600-R700 to fix colorbuffer corruption.
	 * This can be fixed by writing a separate FMASK allocator specifically
	 * for R600-R700 asics. */
	if (rscreen->gfx_level <= R700) {
		bpe *= 2;
	}

	if (rscreen->ws->surface_init(rscreen->ws, &rscreen->info, &templ,
				      flags, bpe, RADEON_SURF_MODE_2D, &fmask)) {
		R600_ERR("Got error in surface_init while allocating FMASK.\n");
		return;
	}

	assert(fmask.u.legacy.level[0].mode == RADEON_SURF_MODE_2D);

	out->slice_tile_max = (fmask.u.legacy.level[0].nblk_x * fmask.u.legacy.level[0].nblk_y) / 64;
	if (out->slice_tile_max)
		out->slice_tile_max -= 1;

	out->tile_mode_index = fmask.u.legacy.tiling_index[0];
	out->pitch_in_pixels = fmask.u.legacy.level[0].nblk_x;
	out->bank_height = fmask.u.legacy.bankh;
	out->tile_swizzle = fmask.tile_swizzle;
	out->alignment = MAX2(256, 1 << fmask.surf_alignment_log2);
	out->size = fmask.surf_size;
}

static void r600_texture_allocate_fmask(struct r600_common_screen *rscreen,
					struct r600_texture *rtex)
{
	r600_texture_get_fmask_info(rscreen, rtex,
				    rtex->resource.b.b.nr_samples, &rtex->fmask);

	rtex->fmask.offset = align64(rtex->size, rtex->fmask.alignment);
	rtex->size = rtex->fmask.offset + rtex->fmask.size;
}

void r600_texture_get_cmask_info(struct r600_common_screen *rscreen,
				 struct r600_texture *rtex,
				 struct r600_cmask_info *out)
{
	unsigned cmask_tile_width = 8;
	unsigned cmask_tile_height = 8;
	unsigned cmask_tile_elements = cmask_tile_width * cmask_tile_height;
	unsigned element_bits = 4;
	unsigned cmask_cache_bits = 1024;
	unsigned num_pipes = rscreen->info.num_tile_pipes;
	unsigned pipe_interleave_bytes = rscreen->info.pipe_interleave_bytes;

	unsigned elements_per_macro_tile = (cmask_cache_bits / element_bits) * num_pipes;
	unsigned pixels_per_macro_tile = elements_per_macro_tile * cmask_tile_elements;
	unsigned sqrt_pixels_per_macro_tile = sqrt(pixels_per_macro_tile);
	unsigned macro_tile_width = util_next_power_of_two(sqrt_pixels_per_macro_tile);
	unsigned macro_tile_height = pixels_per_macro_tile / macro_tile_width;

	unsigned pitch_elements = align(rtex->resource.b.b.width0, macro_tile_width);
	unsigned height = align(rtex->resource.b.b.height0, macro_tile_height);

	unsigned base_align = num_pipes * pipe_interleave_bytes;
	unsigned slice_bytes =
		((pitch_elements * height * element_bits + 7) / 8) / cmask_tile_elements;

	assert(macro_tile_width % 128 == 0);
	assert(macro_tile_height % 128 == 0);

	out->slice_tile_max = ((pitch_elements * height) / (128*128)) - 1;
	out->alignment = MAX2(256, base_align);
	out->size = util_num_layers(&rtex->resource.b.b, 0) *
		    align(slice_bytes, base_align);
}

static void r600_texture_allocate_cmask(struct r600_common_screen *rscreen,
					struct r600_texture *rtex)
{
	r600_texture_get_cmask_info(rscreen, rtex, &rtex->cmask);

	rtex->cmask.offset = align64(rtex->size, rtex->cmask.alignment);
	rtex->size = rtex->cmask.offset + rtex->cmask.size;

	rtex->cb_color_info |= EG_S_028C70_FAST_CLEAR(1);
}

static void r600_texture_alloc_cmask_separate(struct r600_common_screen *rscreen,
					      struct r600_texture *rtex)
{
	if (rtex->cmask_buffer)
                return;

	assert(rtex->cmask.size == 0);

	r600_texture_get_cmask_info(rscreen, rtex, &rtex->cmask);

	rtex->cmask_buffer = (struct r600_resource *)
		r600_aligned_buffer_create(&rscreen->b,
					   R600_RESOURCE_FLAG_UNMAPPABLE,
					   PIPE_USAGE_DEFAULT,
					   rtex->cmask.size,
					   rtex->cmask.alignment);
	if (rtex->cmask_buffer == NULL) {
		rtex->cmask.size = 0;
		return;
	}

	/* update colorbuffer state bits */
	rtex->cmask.base_address_reg = rtex->cmask_buffer->gpu_address >> 8;

	rtex->cb_color_info |= EG_S_028C70_FAST_CLEAR(1);

	p_atomic_inc(&rscreen->compressed_colortex_counter);
}

void eg_resource_alloc_immed(struct r600_common_screen *rscreen,
			     struct r600_resource *res,
			     unsigned immed_size)
{
	res->immed_buffer = (struct r600_resource *)
		pipe_buffer_create(&rscreen->b, PIPE_BIND_CUSTOM,
				   PIPE_USAGE_DEFAULT, immed_size);
}

static void r600_texture_get_htile_size(struct r600_common_screen *rscreen,
					struct r600_texture *rtex)
{
	unsigned cl_width, cl_height, width, height;
	unsigned slice_elements, slice_bytes, pipe_interleave_bytes, base_align;
	unsigned num_pipes = rscreen->info.num_tile_pipes;

	rtex->surface.meta_size = 0;

	/* HW bug on R6xx. */
	if (rscreen->gfx_level == R600 &&
	    (rtex->resource.b.b.width0 > 7680 ||
	     rtex->resource.b.b.height0 > 7680))
		return;

	switch (num_pipes) {
	case 1:
		cl_width = 32;
		cl_height = 16;
		break;
	case 2:
		cl_width = 32;
		cl_height = 32;
		break;
	case 4:
		cl_width = 64;
		cl_height = 32;
		break;
	case 8:
		cl_width = 64;
		cl_height = 64;
		break;
	case 16:
		cl_width = 128;
		cl_height = 64;
		break;
	default:
		assert(0);
		return;
	}

	width = align(rtex->surface.u.legacy.level[0].nblk_x, cl_width * 8);
	height = align(rtex->surface.u.legacy.level[0].nblk_y, cl_height * 8);

	slice_elements = (width * height) / (8 * 8);
	slice_bytes = slice_elements * 4;

	pipe_interleave_bytes = rscreen->info.pipe_interleave_bytes;
	base_align = num_pipes * pipe_interleave_bytes;

	rtex->surface.meta_alignment_log2 = util_logbase2(base_align);
	rtex->surface.meta_size =
		util_num_layers(&rtex->resource.b.b, 0) *
		align(slice_bytes, base_align);
}

static void r600_texture_allocate_htile(struct r600_common_screen *rscreen,
					struct r600_texture *rtex)
{
	r600_texture_get_htile_size(rscreen, rtex);

	if (!rtex->surface.meta_size)
		return;

	rtex->htile_offset = align(rtex->size, 1 << rtex->surface.meta_alignment_log2);
	rtex->size = rtex->htile_offset + rtex->surface.meta_size;
}

void r600_print_texture_info(struct r600_common_screen *rscreen,
			     struct r600_texture *rtex, struct u_log_context *log)
{
	int i;

	/* Common parameters. */
	u_log_printf(log, "  Info: npix_x=%u, npix_y=%u, npix_z=%u, blk_w=%u, "
		"blk_h=%u, array_size=%u, last_level=%u, "
		"bpe=%u, nsamples=%u, flags=0x%"PRIx64", %s\n",
		rtex->resource.b.b.width0, rtex->resource.b.b.height0,
		rtex->resource.b.b.depth0, rtex->surface.blk_w,
		rtex->surface.blk_h,
		rtex->resource.b.b.array_size, rtex->resource.b.b.last_level,
		rtex->surface.bpe, rtex->resource.b.b.nr_samples,
		rtex->surface.flags, util_format_short_name(rtex->resource.b.b.format));

	u_log_printf(log, "  Layout: size=%"PRIu64", alignment=%u, bankw=%u, "
		"bankh=%u, nbanks=%u, mtilea=%u, tilesplit=%u, pipeconfig=%u, scanout=%u\n",
		rtex->surface.surf_size, 1 << rtex->surface.surf_alignment_log2, rtex->surface.u.legacy.bankw,
		rtex->surface.u.legacy.bankh, rtex->surface.u.legacy.num_banks, rtex->surface.u.legacy.mtilea,
		rtex->surface.u.legacy.tile_split, rtex->surface.u.legacy.pipe_config,
		(rtex->surface.flags & RADEON_SURF_SCANOUT) != 0);

	if (rtex->fmask.size)
		u_log_printf(log, "  FMask: offset=%"PRIu64", size=%"PRIu64", alignment=%u, pitch_in_pixels=%u, "
			"bankh=%u, slice_tile_max=%u, tile_mode_index=%u\n",
			rtex->fmask.offset, rtex->fmask.size, rtex->fmask.alignment,
			rtex->fmask.pitch_in_pixels, rtex->fmask.bank_height,
			rtex->fmask.slice_tile_max, rtex->fmask.tile_mode_index);

	if (rtex->cmask.size)
		u_log_printf(log, "  CMask: offset=%"PRIu64", size=%"PRIu64", alignment=%u, "
			"slice_tile_max=%u\n",
			rtex->cmask.offset, rtex->cmask.size, rtex->cmask.alignment,
			rtex->cmask.slice_tile_max);

	if (rtex->htile_offset)
		u_log_printf(log, "  HTile: offset=%"PRIu64", size=%u "
			"alignment=%u\n",
			     rtex->htile_offset, rtex->surface.meta_size,
			     1 << rtex->surface.meta_alignment_log2);

	for (i = 0; i <= rtex->resource.b.b.last_level; i++)
		u_log_printf(log, "  Level[%i]: offset=%"PRIu64", slice_size=%"PRIu64", "
			"npix_x=%u, npix_y=%u, npix_z=%u, nblk_x=%u, nblk_y=%u, "
			"mode=%u, tiling_index = %u\n",
			i, (uint64_t)rtex->surface.u.legacy.level[i].offset_256B * 256,
			(uint64_t)rtex->surface.u.legacy.level[i].slice_size_dw * 4,
			u_minify(rtex->resource.b.b.width0, i),
			u_minify(rtex->resource.b.b.height0, i),
			u_minify(rtex->resource.b.b.depth0, i),
			rtex->surface.u.legacy.level[i].nblk_x,
			rtex->surface.u.legacy.level[i].nblk_y,
			rtex->surface.u.legacy.level[i].mode,
			rtex->surface.u.legacy.tiling_index[i]);

	if (rtex->surface.has_stencil) {
		u_log_printf(log, "  StencilLayout: tilesplit=%u\n",
			rtex->surface.u.legacy.stencil_tile_split);
		for (i = 0; i <= rtex->resource.b.b.last_level; i++) {
			u_log_printf(log, "  StencilLevel[%i]: offset=%"PRIu64", "
				"slice_size=%"PRIu64", npix_x=%u, "
				"npix_y=%u, npix_z=%u, nblk_x=%u, nblk_y=%u, "
				"mode=%u, tiling_index = %u\n",
				i, (uint64_t)rtex->surface.u.legacy.zs.stencil_level[i].offset_256B * 256,
				(uint64_t)rtex->surface.u.legacy.zs.stencil_level[i].slice_size_dw * 4,
				u_minify(rtex->resource.b.b.width0, i),
				u_minify(rtex->resource.b.b.height0, i),
				u_minify(rtex->resource.b.b.depth0, i),
				rtex->surface.u.legacy.zs.stencil_level[i].nblk_x,
				rtex->surface.u.legacy.zs.stencil_level[i].nblk_y,
				rtex->surface.u.legacy.zs.stencil_level[i].mode,
				rtex->surface.u.legacy.zs.stencil_tiling_index[i]);
		}
	}
}

/* Common processing for r600_texture_create and r600_texture_from_handle */
static struct r600_texture *
r600_texture_create_object(struct pipe_screen *screen,
			   const struct pipe_resource *base,
			   struct pb_buffer_lean *buf,
			   struct radeon_surf *surface)
{
	struct r600_texture *rtex;
	struct r600_resource *resource;
	struct r600_common_screen *rscreen = (struct r600_common_screen*)screen;

	rtex = CALLOC_STRUCT(r600_texture);
	if (!rtex)
		return NULL;

	resource = &rtex->resource;
	resource->b.b = *base;
	pipe_reference_init(&resource->b.b.reference, 1);
	resource->b.b.screen = screen;

	/* don't include stencil-only formats which we don't support for rendering */
	rtex->is_depth = util_format_has_depth(util_format_description(rtex->resource.b.b.format));

	rtex->surface = *surface;
	rtex->size = rtex->surface.surf_size;
	rtex->db_render_format = base->format;

	/* Tiled depth textures utilize the non-displayable tile order.
	 * This must be done after r600_setup_surface.
	 * Applies to R600-Cayman. */
	rtex->non_disp_tiling = rtex->is_depth && rtex->surface.u.legacy.level[0].mode >= RADEON_SURF_MODE_1D;
	/* Applies to GCN. */
	rtex->last_msaa_resolve_target_micro_mode = rtex->surface.micro_tile_mode;

	if (rtex->is_depth) {
		if (base->flags & (R600_RESOURCE_FLAG_TRANSFER |
				   R600_RESOURCE_FLAG_FLUSHED_DEPTH) ||
		    rscreen->gfx_level >= EVERGREEN) {
			rtex->can_sample_z = !rtex->surface.u.legacy.depth_adjusted;
			rtex->can_sample_s = !rtex->surface.u.legacy.stencil_adjusted;
		} else {
			if (rtex->resource.b.b.nr_samples <= 1 &&
			    (rtex->resource.b.b.format == PIPE_FORMAT_Z16_UNORM ||
			     rtex->resource.b.b.format == PIPE_FORMAT_Z32_FLOAT))
				rtex->can_sample_z = true;
		}

		if (!(base->flags & (R600_RESOURCE_FLAG_TRANSFER |
				     R600_RESOURCE_FLAG_FLUSHED_DEPTH))) {
			rtex->db_compatible = true;

			if (!(rscreen->debug_flags & DBG_NO_HYPERZ))
				r600_texture_allocate_htile(rscreen, rtex);
		}
	} else {
		if (base->nr_samples > 1) {
			if (!buf) {
				r600_texture_allocate_fmask(rscreen, rtex);
				r600_texture_allocate_cmask(rscreen, rtex);
				rtex->cmask_buffer = &rtex->resource;
			}
			if (!rtex->fmask.size || !rtex->cmask.size) {
				FREE(rtex);
				return NULL;
			}
		}
	}

	/* Now create the backing buffer. */
	if (!buf) {
		r600_init_resource_fields(rscreen, resource, rtex->size,
					  1 << rtex->surface.surf_alignment_log2);

		if (!r600_alloc_resource(rscreen, resource)) {
			FREE(rtex);
			return NULL;
		}
	} else {
		resource->buf = buf;
		resource->gpu_address = rscreen->ws->buffer_get_virtual_address(resource->buf);
		resource->bo_size = buf->size;
		resource->bo_alignment = 1 << buf->alignment_log2;
		resource->domains = rscreen->ws->buffer_get_initial_domain(resource->buf);
		if (resource->domains & RADEON_DOMAIN_VRAM)
			resource->vram_usage = buf->size;
		else if (resource->domains & RADEON_DOMAIN_GTT)
			resource->gart_usage = buf->size;
	}

	if (rtex->cmask.size) {
		/* Initialize the cmask to 0xCC (= compressed state). */
		r600_screen_clear_buffer(rscreen, &rtex->cmask_buffer->b.b,
					 rtex->cmask.offset, rtex->cmask.size,
					 0xCCCCCCCC);
	}
	if (rtex->htile_offset) {
		uint32_t clear_value = 0;

		r600_screen_clear_buffer(rscreen, &rtex->resource.b.b,
					 rtex->htile_offset,
					 rtex->surface.meta_size,
					 clear_value);
	}

	/* Initialize the CMASK base register value. */
	rtex->cmask.base_address_reg =
		(rtex->resource.gpu_address + rtex->cmask.offset) >> 8;

	if (rscreen->debug_flags & DBG_VM) {
		fprintf(stderr, "VM start=0x%"PRIX64"  end=0x%"PRIX64" | Texture %ix%ix%i, %i levels, %i samples, %s\n",
			rtex->resource.gpu_address,
			rtex->resource.gpu_address + rtex->resource.buf->size,
			base->width0, base->height0, util_num_layers(base, 0), base->last_level+1,
			base->nr_samples ? base->nr_samples : 1, util_format_short_name(base->format));
	}

	if (rscreen->debug_flags & DBG_TEX) {
		puts("Texture:");
		struct u_log_context log;
		u_log_context_init(&log);
		r600_print_texture_info(rscreen, rtex, &log);
		u_log_new_page_print(&log, stdout);
		fflush(stdout);
		u_log_context_destroy(&log);
	}

	return rtex;
}

static enum radeon_surf_mode
r600_choose_tiling(struct r600_common_screen *rscreen,
		   const struct pipe_resource *templ)
{
	const struct util_format_description *desc = util_format_description(templ->format);
	bool force_tiling = templ->flags & R600_RESOURCE_FLAG_FORCE_TILING;
	bool is_depth_stencil = util_format_is_depth_or_stencil(templ->format) &&
				!(templ->flags & R600_RESOURCE_FLAG_FLUSHED_DEPTH);

	/* MSAA resources must be 2D tiled. */
	if (templ->nr_samples > 1)
		return RADEON_SURF_MODE_2D;

	/* Transfer resources should be linear. */
	if (templ->flags & R600_RESOURCE_FLAG_TRANSFER)
		return RADEON_SURF_MODE_LINEAR_ALIGNED;

	/* r600g: force tiling on TEXTURE_2D and TEXTURE_3D compute resources. */
	if (rscreen->gfx_level >= R600 && rscreen->gfx_level <= CAYMAN &&
	    (templ->bind & PIPE_BIND_COMPUTE_RESOURCE) &&
	    (templ->target == PIPE_TEXTURE_2D ||
	     templ->target == PIPE_TEXTURE_3D))
		force_tiling = true;

	/* Handle common candidates for the linear mode.
	 * Compressed textures and DB surfaces must always be tiled.
	 */
	if (!force_tiling &&
	    !is_depth_stencil &&
	    !util_format_is_compressed(templ->format)) {
		if (rscreen->debug_flags & DBG_NO_TILING)
			return RADEON_SURF_MODE_LINEAR_ALIGNED;

		/* Tiling doesn't work with the 422 (SUBSAMPLED) formats on R600+. */
		if (desc->layout == UTIL_FORMAT_LAYOUT_SUBSAMPLED)
			return RADEON_SURF_MODE_LINEAR_ALIGNED;

		if (templ->bind & PIPE_BIND_LINEAR)
			return RADEON_SURF_MODE_LINEAR_ALIGNED;

		/* 1D textures should be linear - fixes image operations on 1d */
		if (templ->target == PIPE_TEXTURE_1D ||
		    templ->target == PIPE_TEXTURE_1D_ARRAY)
			return RADEON_SURF_MODE_LINEAR_ALIGNED;

		/* Textures likely to be mapped often. */
		if (templ->usage == PIPE_USAGE_STAGING ||
		    templ->usage == PIPE_USAGE_STREAM)
			return RADEON_SURF_MODE_LINEAR_ALIGNED;
	}

	/* Make small textures 1D tiled. */
	if (templ->width0 <= 16 || templ->height0 <= 16 ||
	    (rscreen->debug_flags & DBG_NO_2D_TILING))
		return RADEON_SURF_MODE_1D;

	/* The allocator will switch to 1D if needed. */
	return RADEON_SURF_MODE_2D;
}

struct pipe_resource *r600_texture_create(struct pipe_screen *screen,
					  const struct pipe_resource *templ)
{
	struct r600_common_screen *rscreen = (struct r600_common_screen*)screen;
	struct radeon_surf surface = {0};
	bool is_flushed_depth = templ->flags & R600_RESOURCE_FLAG_FLUSHED_DEPTH;
	int r;

	r = r600_init_surface(rscreen, &surface, templ,
			      r600_choose_tiling(rscreen, templ), 0, 0,
			      false, false, is_flushed_depth);
	if (r) {
		return NULL;
	}

	return (struct pipe_resource *)
	       r600_texture_create_object(screen, templ, NULL, &surface);
}

static struct pipe_resource *r600_texture_from_handle(struct pipe_screen *screen,
						      const struct pipe_resource *templ,
						      struct winsys_handle *whandle,
                                                      unsigned usage)
{
	struct r600_common_screen *rscreen = (struct r600_common_screen*)screen;
	struct pb_buffer_lean *buf = NULL;
	enum radeon_surf_mode array_mode;
	struct radeon_surf surface = {};
	int r;
	struct radeon_bo_metadata metadata = {};
	struct r600_texture *rtex;
	bool is_scanout;

	/* Support only 2D textures without mipmaps */
	if ((templ->target != PIPE_TEXTURE_2D && templ->target != PIPE_TEXTURE_RECT) ||
	      templ->depth0 != 1 || templ->last_level != 0)
		return NULL;

	buf = rscreen->ws->buffer_from_handle(rscreen->ws, whandle,
					      rscreen->info.max_alignment, false);
	if (!buf)
		return NULL;

	rscreen->ws->buffer_get_metadata(rscreen->ws, buf, &metadata, NULL);
	r600_surface_import_metadata(rscreen, &surface, &metadata,
				     &array_mode, &is_scanout);

	r = r600_init_surface(rscreen, &surface, templ, array_mode,
			      whandle->stride, whandle->offset,
			      true, is_scanout, false);
	if (r) {
		return NULL;
	}

	rtex = r600_texture_create_object(screen, templ, buf, &surface);
	if (!rtex)
		return NULL;

	rtex->resource.b.is_shared = true;
	rtex->resource.external_usage = usage;

	assert(rtex->surface.tile_swizzle == 0);
	return &rtex->resource.b.b;
}

bool r600_init_flushed_depth_texture(struct pipe_context *ctx,
				     struct pipe_resource *texture,
				     struct r600_texture **staging)
{
	struct r600_texture *rtex = (struct r600_texture*)texture;
	struct pipe_resource resource;
	struct r600_texture **flushed_depth_texture = staging ?
			staging : &rtex->flushed_depth_texture;
	enum pipe_format pipe_format = texture->format;

	if (!staging) {
		if (rtex->flushed_depth_texture)
			return true; /* it's ready */

		if (!rtex->can_sample_z && rtex->can_sample_s) {
			switch (pipe_format) {
			case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
				/* Save memory by not allocating the S plane. */
				pipe_format = PIPE_FORMAT_Z32_FLOAT;
				break;
			case PIPE_FORMAT_Z24_UNORM_S8_UINT:
			case PIPE_FORMAT_S8_UINT_Z24_UNORM:
				/* Save memory bandwidth by not copying the
				 * stencil part during flush.
				 *
				 * This potentially increases memory bandwidth
				 * if an application uses both Z and S texturing
				 * simultaneously (a flushed Z24S8 texture
				 * would be stored compactly), but how often
				 * does that really happen?
				 */
				pipe_format = PIPE_FORMAT_Z24X8_UNORM;
				break;
			default:;
			}
		} else if (!rtex->can_sample_s && rtex->can_sample_z) {
			assert(util_format_has_stencil(util_format_description(pipe_format)));

			/* DB->CB copies to an 8bpp surface don't work. */
			pipe_format = PIPE_FORMAT_X24S8_UINT;
		}
	}

	memset(&resource, 0, sizeof(resource));
	resource.target = texture->target;
	resource.format = pipe_format;
	resource.width0 = texture->width0;
	resource.height0 = texture->height0;
	resource.depth0 = texture->depth0;
	resource.array_size = texture->array_size;
	resource.last_level = texture->last_level;
	resource.nr_samples = texture->nr_samples;
	resource.usage = staging ? PIPE_USAGE_STAGING : PIPE_USAGE_DEFAULT;
	resource.bind = texture->bind & ~PIPE_BIND_DEPTH_STENCIL;
	resource.flags = texture->flags | R600_RESOURCE_FLAG_FLUSHED_DEPTH;

	if (staging)
		resource.flags |= R600_RESOURCE_FLAG_TRANSFER;

	*flushed_depth_texture = (struct r600_texture *)ctx->screen->resource_create(ctx->screen, &resource);
	if (*flushed_depth_texture == NULL) {
		R600_ERR("failed to create temporary texture to hold flushed depth\n");
		return false;
	}

	(*flushed_depth_texture)->non_disp_tiling = false;
	return true;
}

/**
 * Initialize the pipe_resource descriptor to be of the same size as the box,
 * which is supposed to hold a subregion of the texture "orig" at the given
 * mipmap level.
 */
static void r600_init_temp_resource_from_box(struct pipe_resource *res,
					     struct pipe_resource *orig,
					     const struct pipe_box *box,
					     unsigned level, unsigned flags)
{
	memset(res, 0, sizeof(*res));
	res->format = orig->format;
	res->width0 = box->width;
	res->height0 = box->height;
	res->depth0 = 1;
	res->array_size = 1;
	res->usage = flags & R600_RESOURCE_FLAG_TRANSFER ? PIPE_USAGE_STAGING : PIPE_USAGE_DEFAULT;
	res->flags = flags;

	/* We must set the correct texture target and dimensions for a 3D box. */
	if (box->depth > 1 && util_max_layer(orig, level) > 0) {
		res->target = PIPE_TEXTURE_2D_ARRAY;
		res->array_size = box->depth;
	} else {
		res->target = PIPE_TEXTURE_2D;
	}
}

static bool r600_can_invalidate_texture(struct r600_common_screen *rscreen,
					struct r600_texture *rtex,
					unsigned transfer_usage,
					const struct pipe_box *box)
{
	/* r600g doesn't react to dirty_tex_descriptor_counter */
	return rscreen->gfx_level >= GFX6 &&
		!rtex->resource.b.is_shared &&
		!(transfer_usage & PIPE_MAP_READ) &&
		rtex->resource.b.b.last_level == 0 &&
		util_texrange_covers_whole_level(&rtex->resource.b.b, 0,
						 box->x, box->y, box->z,
						 box->width, box->height,
						 box->depth);
}

static void r600_texture_invalidate_storage(struct r600_common_context *rctx,
					    struct r600_texture *rtex)
{
	struct r600_common_screen *rscreen = rctx->screen;

	/* There is no point in discarding depth and tiled buffers. */
	assert(!rtex->is_depth);
	assert(rtex->surface.is_linear);

	/* Reallocate the buffer in the same pipe_resource. */
	r600_alloc_resource(rscreen, &rtex->resource);

	/* Initialize the CMASK base address (needed even without CMASK). */
	rtex->cmask.base_address_reg =
		(rtex->resource.gpu_address + rtex->cmask.offset) >> 8;

	p_atomic_inc(&rscreen->dirty_tex_counter);

	rctx->num_alloc_tex_transfer_bytes += rtex->size;
}

void *r600_texture_transfer_map(struct pipe_context *ctx,
			       struct pipe_resource *texture,
			       unsigned level,
			       unsigned usage,
			       const struct pipe_box *box,
			       struct pipe_transfer **ptransfer)
{
	struct r600_common_context *rctx = (struct r600_common_context*)ctx;
	struct r600_texture *rtex = (struct r600_texture*)texture;
	struct r600_transfer *trans;
	struct r600_resource *buf;
	unsigned offset = 0;
	char *map;
	bool use_staging_texture = false;

	assert(!(texture->flags & R600_RESOURCE_FLAG_TRANSFER));
	assert(box->width && box->height && box->depth);

	/* Depth textures use staging unconditionally. */
	if (!rtex->is_depth) {
		/* Degrade the tile mode if we get too many transfers on APUs.
		 * On dGPUs, the staging texture is always faster.
		 * Only count uploads that are at least 4x4 pixels large.
		 */
		if (!rctx->screen->info.has_dedicated_vram &&
		    level == 0 &&
		    box->width >= 4 && box->height >= 4 &&
		    p_atomic_inc_return(&rtex->num_level0_transfers) == 10) {
			bool can_invalidate =
				r600_can_invalidate_texture(rctx->screen, rtex,
							    usage, box);

			r600_reallocate_texture_inplace(rctx, rtex,
							PIPE_BIND_LINEAR,
							can_invalidate);
		}

		/* Tiled textures need to be converted into a linear texture for CPU
		 * access. The staging texture is always linear and is placed in GART.
		 *
		 * Reading from VRAM or GTT WC is slow, always use the staging
		 * texture in this case.
		 *
		 * Use the staging texture for uploads if the underlying BO
		 * is busy.
		 */
		if (!rtex->surface.is_linear)
			use_staging_texture = true;
		else if (usage & PIPE_MAP_READ)
			use_staging_texture =
				rtex->resource.domains & RADEON_DOMAIN_VRAM ||
				rtex->resource.flags & RADEON_FLAG_GTT_WC;
		/* Write & linear only: */
		else if (r600_rings_is_buffer_referenced(rctx, rtex->resource.buf,
							 RADEON_USAGE_READWRITE) ||
			 !rctx->ws->buffer_wait(rctx->ws, rtex->resource.buf, 0,
						RADEON_USAGE_READWRITE)) {
			/* It's busy. */
			if (r600_can_invalidate_texture(rctx->screen, rtex,
							usage, box))
				r600_texture_invalidate_storage(rctx, rtex);
			else
				use_staging_texture = true;
		}
	}

	trans = CALLOC_STRUCT(r600_transfer);
	if (!trans)
		return NULL;
	pipe_resource_reference(&trans->b.b.resource, texture);
	trans->b.b.level = level;
	trans->b.b.usage = usage;
	trans->b.b.box = *box;

	if (rtex->is_depth) {
		struct r600_texture *staging_depth;

		if (rtex->resource.b.b.nr_samples > 1) {
			/* MSAA depth buffers need to be converted to single sample buffers.
			 *
			 * Mapping MSAA depth buffers can occur if ReadPixels is called
			 * with a multisample GLX visual.
			 *
			 * First downsample the depth buffer to a temporary texture,
			 * then decompress the temporary one to staging.
			 *
			 * Only the region being mapped is transferred.
			 */
			struct pipe_resource resource;

			r600_init_temp_resource_from_box(&resource, texture, box, level, 0);

			if (!r600_init_flushed_depth_texture(ctx, &resource, &staging_depth)) {
				R600_ERR("failed to create temporary texture to hold untiled copy\n");
				FREE(trans);
				return NULL;
			}

			if (usage & PIPE_MAP_READ) {
				struct pipe_resource *temp = ctx->screen->resource_create(ctx->screen, &resource);
				if (!temp) {
					R600_ERR("failed to create a temporary depth texture\n");
					FREE(trans);
					return NULL;
				}

				r600_copy_region_with_blit(ctx, temp, 0, 0, 0, 0, texture, level, box);
				rctx->blit_decompress_depth(ctx, (struct r600_texture*)temp, staging_depth,
							    0, 0, 0, box->depth, 0, 0);
				pipe_resource_reference(&temp, NULL);
			}

			/* Just get the strides. */
			r600_texture_get_offset(rctx->screen, staging_depth, level, NULL,
						&trans->b.b.stride,
						&trans->b.b.layer_stride);
		} else {
			/* XXX: only readback the rectangle which is being mapped? */
			/* XXX: when discard is true, no need to read back from depth texture */
			if (!r600_init_flushed_depth_texture(ctx, texture, &staging_depth)) {
				R600_ERR("failed to create temporary texture to hold untiled copy\n");
				FREE(trans);
				return NULL;
			}

			rctx->blit_decompress_depth(ctx, rtex, staging_depth,
						    level, level,
						    box->z, box->z + box->depth - 1,
						    0, 0);

			offset = r600_texture_get_offset(rctx->screen, staging_depth,
							 level, box,
							 &trans->b.b.stride,
							 &trans->b.b.layer_stride);
		}

		trans->staging = (struct r600_resource*)staging_depth;
		buf = trans->staging;
	} else if (use_staging_texture) {
		struct pipe_resource resource;
		struct r600_texture *staging;

		r600_init_temp_resource_from_box(&resource, texture, box, level,
						 R600_RESOURCE_FLAG_TRANSFER);
		resource.usage = (usage & PIPE_MAP_READ) ?
			PIPE_USAGE_STAGING : PIPE_USAGE_STREAM;

		/* Create the temporary texture. */
		staging = (struct r600_texture*)ctx->screen->resource_create(ctx->screen, &resource);
		if (!staging) {
			R600_ERR("failed to create temporary texture to hold untiled copy\n");
			FREE(trans);
			return NULL;
		}
		trans->staging = &staging->resource;

		/* Just get the strides. */
		r600_texture_get_offset(rctx->screen, staging, 0, NULL,
					&trans->b.b.stride,
					&trans->b.b.layer_stride);

		if (usage & PIPE_MAP_READ)
			r600_copy_to_staging_texture(ctx, trans);
		else
			usage |= PIPE_MAP_UNSYNCHRONIZED;

		buf = trans->staging;
	} else {
		/* the resource is mapped directly */
		offset = r600_texture_get_offset(rctx->screen, rtex, level, box,
						 &trans->b.b.stride,
						 &trans->b.b.layer_stride);
		buf = &rtex->resource;
	}

	if (!(map = r600_buffer_map_sync_with_rings(rctx, buf, usage))) {
		r600_resource_reference(&trans->staging, NULL);
		FREE(trans);
		return NULL;
	}

	*ptransfer = &trans->b.b;
	return map + offset;
}

void r600_texture_transfer_unmap(struct pipe_context *ctx,
				struct pipe_transfer* transfer)
{
	struct r600_common_context *rctx = (struct r600_common_context*)ctx;
	struct r600_transfer *rtransfer = (struct r600_transfer*)transfer;
	struct pipe_resource *texture = transfer->resource;
	struct r600_texture *rtex = (struct r600_texture*)texture;

	if ((transfer->usage & PIPE_MAP_WRITE) && rtransfer->staging) {
		if (rtex->is_depth && rtex->resource.b.b.nr_samples <= 1) {
			ctx->resource_copy_region(ctx, texture, transfer->level,
						  transfer->box.x, transfer->box.y, transfer->box.z,
						  &rtransfer->staging->b.b, transfer->level,
						  &transfer->box);
		} else {
			r600_copy_from_staging_texture(ctx, rtransfer);
		}
	}

	if (rtransfer->staging) {
		rctx->num_alloc_tex_transfer_bytes += rtransfer->staging->buf->size;
		r600_resource_reference(&rtransfer->staging, NULL);
	}

	/* Heuristic for {upload, draw, upload, draw, ..}:
	 *
	 * Flush the gfx IB if we've allocated too much texture storage.
	 *
	 * The idea is that we don't want to build IBs that use too much
	 * memory and put pressure on the kernel memory manager and we also
	 * want to make temporary and invalidated buffers go idle ASAP to
	 * decrease the total memory usage or make them reusable. The memory
	 * usage will be slightly higher than given here because of the buffer
	 * cache in the winsys.
	 *
	 * The result is that the kernel memory manager is never a bottleneck.
	 */
	if (rctx->num_alloc_tex_transfer_bytes > (uint64_t)rctx->screen->info.gart_size_kb * 1024 / 4) {
		rctx->gfx.flush(rctx, PIPE_FLUSH_ASYNC, NULL);
		rctx->num_alloc_tex_transfer_bytes = 0;
	}

	pipe_resource_reference(&transfer->resource, NULL);
	FREE(transfer);
}

struct pipe_surface *r600_create_surface_custom(struct pipe_context *pipe,
						struct pipe_resource *texture,
						const struct pipe_surface *templ,
						unsigned width0, unsigned height0,
						unsigned width, unsigned height)
{
	struct r600_surface *surface = CALLOC_STRUCT(r600_surface);

	if (!surface)
		return NULL;

	assert(templ->u.tex.first_layer <= util_max_layer(texture, templ->u.tex.level));
	assert(templ->u.tex.last_layer <= util_max_layer(texture, templ->u.tex.level));

	pipe_reference_init(&surface->base.reference, 1);
	pipe_resource_reference(&surface->base.texture, texture);
	surface->base.context = pipe;
	surface->base.format = templ->format;
	surface->base.width = width;
	surface->base.height = height;
	surface->base.u = templ->u;

	surface->width0 = width0;
	surface->height0 = height0;

	return &surface->base;
}

static struct pipe_surface *r600_create_surface(struct pipe_context *pipe,
						struct pipe_resource *tex,
						const struct pipe_surface *templ)
{
	unsigned level = templ->u.tex.level;
	unsigned width = u_minify(tex->width0, level);
	unsigned height = u_minify(tex->height0, level);
	unsigned width0 = tex->width0;
	unsigned height0 = tex->height0;

	if (tex->target != PIPE_BUFFER && templ->format != tex->format) {
		const struct util_format_description *tex_desc
			= util_format_description(tex->format);
		const struct util_format_description *templ_desc
			= util_format_description(templ->format);

		assert(tex_desc->block.bits == templ_desc->block.bits);

		/* Adjust size of surface if and only if the block width or
		 * height is changed. */
		if (tex_desc->block.width != templ_desc->block.width ||
		    tex_desc->block.height != templ_desc->block.height) {
			unsigned nblks_x = util_format_get_nblocksx(tex->format, width);
			unsigned nblks_y = util_format_get_nblocksy(tex->format, height);

			width = nblks_x * templ_desc->block.width;
			height = nblks_y * templ_desc->block.height;

			width0 = util_format_get_nblocksx(tex->format, width0);
			height0 = util_format_get_nblocksy(tex->format, height0);
		}
	}

	return r600_create_surface_custom(pipe, tex, templ,
					  width0, height0,
					  width, height);
}

static void r600_surface_destroy(struct pipe_context *pipe,
				 struct pipe_surface *surface)
{
	struct r600_surface *surf = (struct r600_surface*)surface;
	r600_resource_reference(&surf->cb_buffer_fmask, NULL);
	r600_resource_reference(&surf->cb_buffer_cmask, NULL);
	pipe_resource_reference(&surface->texture, NULL);
	FREE(surface);
}

unsigned r600_translate_colorswap(enum pipe_format format, bool do_endian_swap)
{
	const struct util_format_description *desc = util_format_description(format);

#define HAS_SWIZZLE(chan,swz) (desc->swizzle[chan] == PIPE_SWIZZLE_##swz)

	if (format == PIPE_FORMAT_R11G11B10_FLOAT) /* isn't plain */
		return V_0280A0_SWAP_STD;

	if (desc->layout != UTIL_FORMAT_LAYOUT_PLAIN)
		return ~0U;

	switch (desc->nr_channels) {
	case 1:
		if (HAS_SWIZZLE(0,X))
			return V_0280A0_SWAP_STD; /* X___ */
		else if (HAS_SWIZZLE(3,X))
			return V_0280A0_SWAP_ALT_REV; /* ___X */
		break;
	case 2:
		if ((HAS_SWIZZLE(0,X) && HAS_SWIZZLE(1,Y)) ||
		    (HAS_SWIZZLE(0,X) && HAS_SWIZZLE(1,NONE)) ||
		    (HAS_SWIZZLE(0,NONE) && HAS_SWIZZLE(1,Y)))
			return V_0280A0_SWAP_STD; /* XY__ */
		else if ((HAS_SWIZZLE(0,Y) && HAS_SWIZZLE(1,X)) ||
			 (HAS_SWIZZLE(0,Y) && HAS_SWIZZLE(1,NONE)) ||
		         (HAS_SWIZZLE(0,NONE) && HAS_SWIZZLE(1,X)))
			/* YX__ */
			return (do_endian_swap ? V_0280A0_SWAP_STD : V_0280A0_SWAP_STD_REV);
		else if (HAS_SWIZZLE(0,X) && HAS_SWIZZLE(3,Y))
			return V_0280A0_SWAP_ALT; /* X__Y */
		else if (HAS_SWIZZLE(0,Y) && HAS_SWIZZLE(3,X))
			return V_0280A0_SWAP_ALT_REV; /* Y__X */
		break;
	case 3:
		if (HAS_SWIZZLE(0,X))
			return (do_endian_swap ? V_0280A0_SWAP_STD_REV : V_0280A0_SWAP_STD);
		else if (HAS_SWIZZLE(0,Z))
			return V_0280A0_SWAP_STD_REV; /* ZYX */
		break;
	case 4:
		/* check the middle channels, the 1st and 4th channel can be NONE */
		if (HAS_SWIZZLE(1,Y) && HAS_SWIZZLE(2,Z)) {
			return V_0280A0_SWAP_STD; /* XYZW */
		} else if (HAS_SWIZZLE(1,Z) && HAS_SWIZZLE(2,Y)) {
			return V_0280A0_SWAP_STD_REV; /* WZYX */
		} else if (HAS_SWIZZLE(1,Y) && HAS_SWIZZLE(2,X)) {
			return V_0280A0_SWAP_ALT; /* ZYXW */
		} else if (HAS_SWIZZLE(1,Z) && HAS_SWIZZLE(2,W)) {
			/* YZWX */
			if (desc->is_array)
				return V_0280A0_SWAP_ALT_REV;
			else
				return (do_endian_swap ? V_0280A0_SWAP_ALT : V_0280A0_SWAP_ALT_REV);
		}
		break;
	}
	return ~0U;
}

/* FAST COLOR CLEAR */

static void evergreen_set_clear_color(struct r600_texture *rtex,
				      enum pipe_format surface_format,
				      const union pipe_color_union *color)
{
	union util_color uc;

	memset(&uc, 0, sizeof(uc));

	if (rtex->surface.bpe == 16) {
		/* DCC fast clear only:
		 *   CLEAR_WORD0 = R = G = B
		 *   CLEAR_WORD1 = A
		 */
		assert(color->ui[0] == color->ui[1] &&
		       color->ui[0] == color->ui[2]);
		uc.ui[0] = color->ui[0];
		uc.ui[1] = color->ui[3];
	} else {
		util_pack_color_union(surface_format, &uc, color);
	}

	memcpy(rtex->color_clear_value, &uc, 2 * sizeof(uint32_t));
}

void evergreen_do_fast_color_clear(struct r600_common_context *rctx,
				   struct pipe_framebuffer_state *fb,
				   struct r600_atom *fb_state,
				   unsigned *buffers, uint8_t *dirty_cbufs,
				   const union pipe_color_union *color)
{
	int i;

	/* This function is broken in BE, so just disable this path for now */
#if UTIL_ARCH_BIG_ENDIAN
	return;
#endif

	if (rctx->render_cond)
		return;

	for (i = 0; i < fb->nr_cbufs; i++) {
		struct r600_texture *tex;
		unsigned clear_bit = PIPE_CLEAR_COLOR0 << i;

		if (!fb->cbufs[i])
			continue;

		/* if this colorbuffer is not being cleared */
		if (!(*buffers & clear_bit))
			continue;

		tex = (struct r600_texture *)fb->cbufs[i]->texture;

		/* the clear is allowed if all layers are bound */
		if (fb->cbufs[i]->u.tex.first_layer != 0 ||
		    fb->cbufs[i]->u.tex.last_layer != util_max_layer(&tex->resource.b.b, 0)) {
			continue;
		}

		/* cannot clear mipmapped textures */
		if (fb->cbufs[i]->texture->last_level != 0) {
			continue;
		}

		/* only supported on tiled surfaces */
		if (tex->surface.is_linear) {
			continue;
		}

		/* shared textures can't use fast clear without an explicit flush,
		 * because there is no way to communicate the clear color among
		 * all clients
		 */
		if (tex->resource.b.is_shared &&
		    !(tex->resource.external_usage & PIPE_HANDLE_USAGE_EXPLICIT_FLUSH))
			continue;

		/* Use a slow clear for small surfaces where the cost of
		 * the eliminate pass can be higher than the benefit of fast
		 * clear. AMDGPU-pro does this, but the numbers may differ.
		 *
		 * This helps on both dGPUs and APUs, even small ones.
		 */
		if (tex->resource.b.b.nr_samples <= 1 &&
		    tex->resource.b.b.width0 * tex->resource.b.b.height0 <= 300 * 300)
			continue;

		{
			/* 128-bit formats are unusupported */
			if (tex->surface.bpe > 8) {
				continue;
			}

			/* ensure CMASK is enabled */
			r600_texture_alloc_cmask_separate(rctx->screen, tex);
			if (tex->cmask.size == 0) {
				continue;
			}

			/* Do the fast clear. */
			rctx->clear_buffer(&rctx->b, &tex->cmask_buffer->b.b,
					   tex->cmask.offset, tex->cmask.size, 0,
					   R600_COHERENCY_CB_META);

			bool need_compressed_update = !tex->dirty_level_mask;

			tex->dirty_level_mask |= 1 << fb->cbufs[i]->u.tex.level;

			if (need_compressed_update)
				p_atomic_inc(&rctx->screen->compressed_colortex_counter);
		}

		evergreen_set_clear_color(tex, fb->cbufs[i]->format, color);

		if (dirty_cbufs)
			*dirty_cbufs |= 1 << i;
		rctx->set_atom_dirty(rctx, fb_state, true);
		*buffers &= ~clear_bit;
	}
}

static struct pipe_memory_object *
r600_memobj_from_handle(struct pipe_screen *screen,
			struct winsys_handle *whandle,
			bool dedicated)
{
	struct r600_common_screen *rscreen = (struct r600_common_screen*)screen;
	struct r600_memory_object *memobj = CALLOC_STRUCT(r600_memory_object);
	struct pb_buffer_lean *buf = NULL;

	if (!memobj)
		return NULL;

	buf = rscreen->ws->buffer_from_handle(rscreen->ws, whandle,
					      rscreen->info.max_alignment, false);
	if (!buf) {
		free(memobj);
		return NULL;
	}

	memobj->b.dedicated = dedicated;
	memobj->buf = buf;
	memobj->stride = whandle->stride;
	memobj->offset = whandle->offset;

	return (struct pipe_memory_object *)memobj;

}

static void
r600_memobj_destroy(struct pipe_screen *screen,
		    struct pipe_memory_object *_memobj)
{
	struct r600_common_screen *rscreen = (struct r600_common_screen*)screen;
	struct r600_memory_object *memobj = (struct r600_memory_object *)_memobj;

	radeon_bo_reference(rscreen->ws, &memobj->buf, NULL);
	free(memobj);
}

static struct pipe_resource *
r600_texture_from_memobj(struct pipe_screen *screen,
			 const struct pipe_resource *templ,
			 struct pipe_memory_object *_memobj,
			 uint64_t offset)
{
	int r;
	struct r600_common_screen *rscreen = (struct r600_common_screen*)screen;
	struct r600_memory_object *memobj = (struct r600_memory_object *)_memobj;
	struct r600_texture *rtex;
	struct radeon_surf surface = {};
	struct radeon_bo_metadata metadata = {};
	enum radeon_surf_mode array_mode;
	bool is_scanout;
	struct pb_buffer_lean *buf = NULL;

	if (memobj->b.dedicated) {
		rscreen->ws->buffer_get_metadata(rscreen->ws, memobj->buf, &metadata, NULL);
		r600_surface_import_metadata(rscreen, &surface, &metadata,
				     &array_mode, &is_scanout);
	} else {
		/**
		 * The bo metadata is unset for un-dedicated images. So we fall
		 * back to linear. See answer to question 5 of the
		 * VK_KHX_external_memory spec for some details.
		 *
		 * It is possible that this case isn't going to work if the
		 * surface pitch isn't correctly aligned by default.
		 *
		 * In order to support it correctly we require multi-image
		 * metadata to be synchronized between radv and radeonsi. The
		 * semantics of associating multiple image metadata to a memory
		 * object on the vulkan export side are not concretely defined
		 * either.
		 *
		 * All the use cases we are aware of at the moment for memory
		 * objects use dedicated allocations. So lets keep the initial
		 * implementation simple.
		 *
		 * A possible alternative is to attempt to reconstruct the
		 * tiling information when the TexParameter TEXTURE_TILING_EXT
		 * is set.
		 */
		array_mode = RADEON_SURF_MODE_LINEAR_ALIGNED;
		is_scanout = false;

	}

	r = r600_init_surface(rscreen, &surface, templ,
			      array_mode, memobj->stride,
			      offset, true, is_scanout,
			      false);
	if (r)
		return NULL;

	rtex = r600_texture_create_object(screen, templ, memobj->buf, &surface);
	if (!rtex)
		return NULL;

	/* r600_texture_create_object doesn't increment refcount of
	 * memobj->buf, so increment it here.
	 */
	radeon_bo_reference(rscreen->ws, &buf, memobj->buf);

	rtex->resource.b.is_shared = true;
	rtex->resource.external_usage = PIPE_HANDLE_USAGE_FRAMEBUFFER_WRITE;

	return &rtex->resource.b.b;
}

void r600_init_screen_texture_functions(struct r600_common_screen *rscreen)
{
	rscreen->b.resource_from_handle = r600_texture_from_handle;
	rscreen->b.resource_get_handle = r600_texture_get_handle;
	rscreen->b.resource_get_info = r600_texture_get_info;
	rscreen->b.resource_from_memobj = r600_texture_from_memobj;
	rscreen->b.memobj_create_from_handle = r600_memobj_from_handle;
	rscreen->b.memobj_destroy = r600_memobj_destroy;
}

void r600_init_context_texture_functions(struct r600_common_context *rctx)
{
	rctx->b.create_surface = r600_create_surface;
	rctx->b.surface_destroy = r600_surface_destroy;
	rctx->b.clear_texture = u_default_clear_texture;
}
