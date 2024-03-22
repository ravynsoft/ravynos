/**************************************************************************
 *
 * Copyright 2013 Advanced Micro Devices, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/*
 * Authors:
 *      Christian König <christian.koenig@amd.com>
 *
 */

#include <unistd.h>

#include "util/u_memory.h"
#include "util/u_video.h"

#include "vl/vl_defines.h"
#include "vl/vl_video_buffer.h"

#include "r600_pipe_common.h"
#include "radeon_video.h"
#include "radeon_vce.h"

#define UVD_FW_1_66_16 ((1 << 24) | (66 << 16) | (16 << 8))

/* generate an stream handle */
unsigned rvid_alloc_stream_handle()
{
	static unsigned counter = 0;
	unsigned stream_handle = 0;
	unsigned pid = getpid();
	int i;

	for (i = 0; i < 32; ++i)
		stream_handle |= ((pid >> i) & 1) << (31 - i);

	stream_handle ^= ++counter;
	return stream_handle;
}

/* create a buffer in the winsys */
bool rvid_create_buffer(struct pipe_screen *screen, struct rvid_buffer *buffer,
			unsigned size, unsigned usage)
{
	memset(buffer, 0, sizeof(*buffer));
	buffer->usage = usage;

	/* Hardware buffer placement restrictions require the kernel to be
	 * able to move buffers around individually, so request a
	 * non-sub-allocated buffer.
	 */
	buffer->res = (struct r600_resource *)
		pipe_buffer_create(screen, PIPE_BIND_SHARED,
				   usage, size);

	return buffer->res != NULL;
}

/* destroy a buffer */
void rvid_destroy_buffer(struct rvid_buffer *buffer)
{
	r600_resource_reference(&buffer->res, NULL);
}

/* reallocate a buffer, preserving its content */
bool rvid_resize_buffer(struct pipe_screen *screen, struct radeon_cmdbuf *cs,
			struct rvid_buffer *new_buf, unsigned new_size)
{
	struct r600_common_screen *rscreen = (struct r600_common_screen *)screen;
	struct radeon_winsys* ws = rscreen->ws;
	unsigned bytes = MIN2(new_buf->res->buf->size, new_size);
	struct rvid_buffer old_buf = *new_buf;
	void *src = NULL, *dst = NULL;

	if (!rvid_create_buffer(screen, new_buf, new_size, new_buf->usage))
		goto error;

	src = ws->buffer_map(ws, old_buf.res->buf, cs,
			     PIPE_MAP_READ | RADEON_MAP_TEMPORARY);
	if (!src)
		goto error;

	dst = ws->buffer_map(ws, new_buf->res->buf, cs,
			     PIPE_MAP_WRITE | RADEON_MAP_TEMPORARY);
	if (!dst)
		goto error;

	memcpy(dst, src, bytes);
	if (new_size > bytes) {
		new_size -= bytes;
		dst += bytes;
		memset(dst, 0, new_size);
	}
	ws->buffer_unmap(ws, new_buf->res->buf);
	ws->buffer_unmap(ws, old_buf.res->buf);
	rvid_destroy_buffer(&old_buf);
	return true;

error:
	if (src)
		ws->buffer_unmap(ws, old_buf.res->buf);
	rvid_destroy_buffer(new_buf);
	*new_buf = old_buf;
	return false;
}

/* clear the buffer with zeros */
void rvid_clear_buffer(struct pipe_context *context, struct rvid_buffer* buffer)
{
	struct r600_common_context *rctx = (struct r600_common_context*)context;

	rctx->dma_clear_buffer(context, &buffer->res->b.b, 0,
			       buffer->res->buf->size, 0);
	context->flush(context, NULL, 0);
}

/**
 * join surfaces into the same buffer with identical tiling params
 * sumup their sizes and replace the backend buffers with a single bo
 */
void rvid_join_surfaces(struct r600_common_context *rctx,
			struct pb_buffer_lean** buffers[VL_NUM_COMPONENTS],
			struct radeon_surf *surfaces[VL_NUM_COMPONENTS])
{
	struct radeon_winsys* ws;
	unsigned best_tiling, best_wh, off;
	unsigned size, alignment;
	struct pb_buffer_lean *pb;
	unsigned i, j;

	ws = rctx->ws;

	for (i = 0, best_tiling = 0, best_wh = ~0; i < VL_NUM_COMPONENTS; ++i) {
		unsigned wh;

		if (!surfaces[i])
			continue;

		/* choose the smallest bank w/h for now */
		wh = surfaces[i]->u.legacy.bankw * surfaces[i]->u.legacy.bankh;
		if (wh < best_wh) {
			best_wh = wh;
			best_tiling = i;
		}
	}

	for (i = 0, off = 0; i < VL_NUM_COMPONENTS; ++i) {
		if (!surfaces[i])
			continue;

		/* adjust the texture layer offsets */
		off = align(off, 1 << surfaces[i]->surf_alignment_log2);

		/* copy the tiling parameters */
		surfaces[i]->u.legacy.bankw = surfaces[best_tiling]->u.legacy.bankw;
		surfaces[i]->u.legacy.bankh = surfaces[best_tiling]->u.legacy.bankh;
		surfaces[i]->u.legacy.mtilea = surfaces[best_tiling]->u.legacy.mtilea;
		surfaces[i]->u.legacy.tile_split = surfaces[best_tiling]->u.legacy.tile_split;

		for (j = 0; j < ARRAY_SIZE(surfaces[i]->u.legacy.level); ++j)
			surfaces[i]->u.legacy.level[j].offset_256B += off / 256;

		off += surfaces[i]->surf_size;
	}

	for (i = 0, size = 0, alignment = 0; i < VL_NUM_COMPONENTS; ++i) {
		if (!buffers[i] || !*buffers[i])
			continue;

		size = align(size, 1 << (*buffers[i])->alignment_log2);
		size += (*buffers[i])->size;
		alignment = MAX2(alignment, 1 << (*buffers[i])->alignment_log2);
	}

	if (!size)
		return;

	/* TODO: 2D tiling workaround */
	alignment *= 2;

	pb = ws->buffer_create(ws, size, alignment, RADEON_DOMAIN_VRAM,
			       RADEON_FLAG_GTT_WC);
	if (!pb)
		return;

	for (i = 0; i < VL_NUM_COMPONENTS; ++i) {
		if (!buffers[i] || !*buffers[i])
			continue;

		radeon_bo_reference(rctx->ws, buffers[i], pb);
	}

	radeon_bo_reference(rctx->ws, &pb, NULL);
}

int rvid_get_video_param(struct pipe_screen *screen,
			 enum pipe_video_profile profile,
			 enum pipe_video_entrypoint entrypoint,
			 enum pipe_video_cap param)
{
	struct r600_common_screen *rscreen = (struct r600_common_screen *)screen;
	enum pipe_video_format codec = u_reduce_video_profile(profile);
	struct radeon_info info;

	rscreen->ws->query_info(rscreen->ws, &info);

	if (entrypoint == PIPE_VIDEO_ENTRYPOINT_ENCODE) {
		switch (param) {
		case PIPE_VIDEO_CAP_SUPPORTED:
			return codec == PIPE_VIDEO_FORMAT_MPEG4_AVC &&
				rvce_is_fw_version_supported(rscreen);
		case PIPE_VIDEO_CAP_NPOT_TEXTURES:
			return 1;
		case PIPE_VIDEO_CAP_MAX_WIDTH:
			return 2048;
		case PIPE_VIDEO_CAP_MAX_HEIGHT:
			return 1152;
		case PIPE_VIDEO_CAP_PREFERED_FORMAT:
			return PIPE_FORMAT_NV12;
		case PIPE_VIDEO_CAP_PREFERS_INTERLACED:
			return false;
		case PIPE_VIDEO_CAP_SUPPORTS_INTERLACED:
			return false;
		case PIPE_VIDEO_CAP_SUPPORTS_PROGRESSIVE:
			return true;
		case PIPE_VIDEO_CAP_STACKED_FRAMES:
			return 1;
		default:
			return 0;
		}
	}

	switch (param) {
	case PIPE_VIDEO_CAP_SUPPORTED:
		switch (codec) {
		case PIPE_VIDEO_FORMAT_MPEG12:
			return profile != PIPE_VIDEO_PROFILE_MPEG1;
		case PIPE_VIDEO_FORMAT_MPEG4:
			/* no support for MPEG4 on older hw */
			return rscreen->family >= CHIP_PALM;
		case PIPE_VIDEO_FORMAT_MPEG4_AVC:
			return true;
		case PIPE_VIDEO_FORMAT_VC1:
			return true;
		case PIPE_VIDEO_FORMAT_HEVC:
			return false;
		case PIPE_VIDEO_FORMAT_JPEG:
			return false;
		default:
			return false;
		}
	case PIPE_VIDEO_CAP_NPOT_TEXTURES:
		return 1;
	case PIPE_VIDEO_CAP_MAX_WIDTH:
		return 2048;
	case PIPE_VIDEO_CAP_MAX_HEIGHT:
		return 1152;
	case PIPE_VIDEO_CAP_PREFERED_FORMAT:
		return PIPE_FORMAT_NV12;

	case PIPE_VIDEO_CAP_PREFERS_INTERLACED:
	case PIPE_VIDEO_CAP_SUPPORTS_INTERLACED:
		if (rscreen->family < CHIP_PALM) {
			/* MPEG2 only with shaders and no support for
			   interlacing on R6xx style UVD */
			return codec != PIPE_VIDEO_FORMAT_MPEG12 &&
			       rscreen->family > CHIP_RV770;
		} else {
			enum pipe_video_format format = u_reduce_video_profile(profile);

			if (format == PIPE_VIDEO_FORMAT_JPEG)
				return false;
			return true;
		}
	case PIPE_VIDEO_CAP_SUPPORTS_PROGRESSIVE:
		return true;
	case PIPE_VIDEO_CAP_MAX_LEVEL:
		switch (profile) {
		case PIPE_VIDEO_PROFILE_MPEG1:
			return 0;
		case PIPE_VIDEO_PROFILE_MPEG2_SIMPLE:
		case PIPE_VIDEO_PROFILE_MPEG2_MAIN:
			return 3;
		case PIPE_VIDEO_PROFILE_MPEG4_SIMPLE:
			return 3;
		case PIPE_VIDEO_PROFILE_MPEG4_ADVANCED_SIMPLE:
			return 5;
		case PIPE_VIDEO_PROFILE_VC1_SIMPLE:
			return 1;
		case PIPE_VIDEO_PROFILE_VC1_MAIN:
			return 2;
		case PIPE_VIDEO_PROFILE_VC1_ADVANCED:
			return 4;
		case PIPE_VIDEO_PROFILE_MPEG4_AVC_BASELINE:
		case PIPE_VIDEO_PROFILE_MPEG4_AVC_MAIN:
		case PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH:
			return 41;
		default:
			return 0;
		}
	default:
		return 0;
	}
}

bool rvid_is_format_supported(struct pipe_screen *screen,
			      enum pipe_format format,
			      enum pipe_video_profile profile,
			      enum pipe_video_entrypoint entrypoint)
{
	/* we can only handle this one with UVD */
	if (profile != PIPE_VIDEO_PROFILE_UNKNOWN)
		return format == PIPE_FORMAT_NV12;

	return vl_video_buffer_is_format_supported(screen, format, profile, entrypoint);
}
