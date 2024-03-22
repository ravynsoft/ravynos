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

#include <stdio.h>

#include "pipe/p_video_codec.h"

#include "util/u_video.h"
#include "util/u_memory.h"

#include "vl/vl_video_buffer.h"

#include "r600_pipe_common.h"
#include "radeon_video.h"
#include "radeon_vce.h"

#define FW_40_2_2 ((40 << 24) | (2 << 16) | (2 << 8))
#define FW_50_0_1 ((50 << 24) | (0 << 16) | (1 << 8))
#define FW_50_1_2 ((50 << 24) | (1 << 16) | (2 << 8))
#define FW_50_10_2 ((50 << 24) | (10 << 16) | (2 << 8))
#define FW_50_17_3 ((50 << 24) | (17 << 16) | (3 << 8))
#define FW_52_0_3 ((52 << 24) | (0 << 16) | (3 << 8))
#define FW_52_4_3 ((52 << 24) | (4 << 16) | (3 << 8))
#define FW_52_8_3 ((52 << 24) | (8 << 16) | (3 << 8))
#define FW_53 (53 << 24)

/* version specific function for getting parameters */
static void (*get_pic_param)(struct rvce_encoder *enc,
                             struct pipe_h264_enc_picture_desc *pic) = NULL;

/**
 * flush commands to the hardware
 */
static void flush(struct rvce_encoder *enc)
{
	enc->ws->cs_flush(&enc->cs, PIPE_FLUSH_ASYNC, NULL);
	enc->task_info_idx = 0;
	enc->bs_idx = 0;
}

#if 0
static void dump_feedback(struct rvce_encoder *enc, struct rvid_buffer *fb)
{
	uint32_t *ptr = enc->ws->buffer_map(fb->res->buf, &enc->cs, PIPE_MAP_READ_WRITE);
	unsigned i = 0;
	fprintf(stderr, "\n");
	fprintf(stderr, "encStatus:\t\t\t%08x\n", ptr[i++]);
	fprintf(stderr, "encHasBitstream:\t\t%08x\n", ptr[i++]);
	fprintf(stderr, "encHasAudioBitstream:\t\t%08x\n", ptr[i++]);
	fprintf(stderr, "encBitstreamOffset:\t\t%08x\n", ptr[i++]);
	fprintf(stderr, "encBitstreamSize:\t\t%08x\n", ptr[i++]);
	fprintf(stderr, "encAudioBitstreamOffset:\t%08x\n", ptr[i++]);
	fprintf(stderr, "encAudioBitstreamSize:\t\t%08x\n", ptr[i++]);
	fprintf(stderr, "encExtrabytes:\t\t\t%08x\n", ptr[i++]);
	fprintf(stderr, "encAudioExtrabytes:\t\t%08x\n", ptr[i++]);
	fprintf(stderr, "videoTimeStamp:\t\t\t%08x\n", ptr[i++]);
	fprintf(stderr, "audioTimeStamp:\t\t\t%08x\n", ptr[i++]);
	fprintf(stderr, "videoOutputType:\t\t%08x\n", ptr[i++]);
	fprintf(stderr, "attributeFlags:\t\t\t%08x\n", ptr[i++]);
	fprintf(stderr, "seiPrivatePackageOffset:\t%08x\n", ptr[i++]);
	fprintf(stderr, "seiPrivatePackageSize:\t\t%08x\n", ptr[i++]);
	fprintf(stderr, "\n");
	enc->ws->buffer_unmap(fb->res->buf);
}
#endif

/**
 * reset the CPB handling
 */
static void reset_cpb(struct rvce_encoder *enc)
{
	unsigned i;

	list_inithead(&enc->cpb_slots);
	for (i = 0; i < enc->cpb_num; ++i) {
		struct rvce_cpb_slot *slot = &enc->cpb_array[i];
		slot->index = i;
		slot->picture_type = PIPE_H2645_ENC_PICTURE_TYPE_SKIP;
		slot->frame_num = 0;
		slot->pic_order_cnt = 0;
		list_addtail(&slot->list, &enc->cpb_slots);
	}
}

/**
 * sort l0 and l1 to the top of the list
 */
static void sort_cpb(struct rvce_encoder *enc)
{
	struct rvce_cpb_slot *i, *l0 = NULL, *l1 = NULL;

	LIST_FOR_EACH_ENTRY(i, &enc->cpb_slots, list) {
		if (i->frame_num == enc->pic.ref_idx_l0_list[0])
			l0 = i;

		if (i->frame_num == enc->pic.ref_idx_l1_list[0])
			l1 = i;

		if (enc->pic.picture_type == PIPE_H2645_ENC_PICTURE_TYPE_P && l0)
			break;

		if (enc->pic.picture_type == PIPE_H2645_ENC_PICTURE_TYPE_B &&
		    l0 && l1)
			break;
	}

	if (l1) {
		list_del(&l1->list);
		list_add(&l1->list, &enc->cpb_slots);
	}

	if (l0) {
		list_del(&l0->list);
		list_add(&l0->list, &enc->cpb_slots);
	}
}

/**
 * get number of cpbs based on dpb
 */
static unsigned get_cpb_num(struct rvce_encoder *enc)
{
	unsigned w = align(enc->base.width, 16) / 16;
	unsigned h = align(enc->base.height, 16) / 16;
	unsigned dpb;

	switch (enc->base.level) {
	case 10:
		dpb = 396;
		break;
	case 11:
		dpb = 900;
		break;
	case 12:
	case 13:
	case 20:
		dpb = 2376;
		break;
	case 21:
		dpb = 4752;
		break;
	case 22:
	case 30:
		dpb = 8100;
		break;
	case 31:
		dpb = 18000;
		break;
	case 32:
		dpb = 20480;
		break;
	case 40:
	case 41:
		dpb = 32768;
		break;
	case 42:
		dpb = 34816;
		break;
	case 50:
		dpb = 110400;
		break;
	default:
	case 51:
	case 52:
		dpb = 184320;
		break;
	}

	return MIN2(dpb / (w * h), 16);
}

/**
 * Get the slot for the currently encoded frame
 */
struct rvce_cpb_slot *current_slot(struct rvce_encoder *enc)
{
	return list_entry(enc->cpb_slots.prev, struct rvce_cpb_slot, list);
}

/**
 * Get the slot for L0
 */
struct rvce_cpb_slot *l0_slot(struct rvce_encoder *enc)
{
	return list_entry(enc->cpb_slots.next, struct rvce_cpb_slot, list);
}

/**
 * Get the slot for L1
 */
struct rvce_cpb_slot *l1_slot(struct rvce_encoder *enc)
{
	return list_entry(enc->cpb_slots.next->next, struct rvce_cpb_slot, list);
}

/**
 * Calculate the offsets into the CPB
 */
void rvce_frame_offset(struct rvce_encoder *enc, struct rvce_cpb_slot *slot,
		       signed *luma_offset, signed *chroma_offset)
{
	unsigned pitch, vpitch, fsize;

	pitch = align(enc->luma->u.legacy.level[0].nblk_x * enc->luma->bpe, 128);
	vpitch = align(enc->luma->u.legacy.level[0].nblk_y, 16);
	fsize = pitch * (vpitch + vpitch / 2);

	*luma_offset = slot->index * fsize;
	*chroma_offset = *luma_offset + pitch * vpitch;
}

/**
 * destroy this video encoder
 */
static void rvce_destroy(struct pipe_video_codec *encoder)
{
	struct rvce_encoder *enc = (struct rvce_encoder*)encoder;
	if (enc->stream_handle) {
		struct rvid_buffer fb;
		rvid_create_buffer(enc->screen, &fb, 512, PIPE_USAGE_STAGING);
		enc->fb = &fb;
		enc->session(enc);
		enc->feedback(enc);
		enc->destroy(enc);
		flush(enc);
		rvid_destroy_buffer(&fb);
	}
	rvid_destroy_buffer(&enc->cpb);
	enc->ws->cs_destroy(&enc->cs);
	FREE(enc->cpb_array);
	FREE(enc);
}

static void rvce_begin_frame(struct pipe_video_codec *encoder,
			     struct pipe_video_buffer *source,
			     struct pipe_picture_desc *picture)
{
	struct rvce_encoder *enc = (struct rvce_encoder*)encoder;
	struct vl_video_buffer *vid_buf = (struct vl_video_buffer *)source;
	struct pipe_h264_enc_picture_desc *pic = (struct pipe_h264_enc_picture_desc *)picture;

	bool need_rate_control =
		enc->pic.rate_ctrl[0].rate_ctrl_method != pic->rate_ctrl[0].rate_ctrl_method ||
		enc->pic.quant_i_frames != pic->quant_i_frames ||
		enc->pic.quant_p_frames != pic->quant_p_frames ||
		enc->pic.quant_b_frames != pic->quant_b_frames;

	enc->pic = *pic;
	get_pic_param(enc, pic);

	enc->get_buffer(vid_buf->resources[0], &enc->handle, &enc->luma);
	enc->get_buffer(vid_buf->resources[1], NULL, &enc->chroma);

	if (pic->picture_type == PIPE_H2645_ENC_PICTURE_TYPE_IDR)
		reset_cpb(enc);
	else if (pic->picture_type == PIPE_H2645_ENC_PICTURE_TYPE_P ||
	         pic->picture_type == PIPE_H2645_ENC_PICTURE_TYPE_B)
		sort_cpb(enc);
	
	if (!enc->stream_handle) {
		struct rvid_buffer fb;
		enc->stream_handle = rvid_alloc_stream_handle();
		rvid_create_buffer(enc->screen, &fb, 512, PIPE_USAGE_STAGING);
		enc->fb = &fb;
		enc->session(enc);
		enc->create(enc);
		enc->config(enc);
		enc->feedback(enc);
		flush(enc);
		//dump_feedback(enc, &fb);
		rvid_destroy_buffer(&fb);
		need_rate_control = false;
	}

	if (need_rate_control) {
		enc->session(enc);
		enc->config(enc);
		flush(enc);
	}
}

static void rvce_encode_bitstream(struct pipe_video_codec *encoder,
				  struct pipe_video_buffer *source,
				  struct pipe_resource *destination,
				  void **fb)
{
	struct rvce_encoder *enc = (struct rvce_encoder*)encoder;
	enc->get_buffer(destination, &enc->bs_handle, NULL);
	enc->bs_size = destination->width0;

	*fb = enc->fb = CALLOC_STRUCT(rvid_buffer);
	if (!rvid_create_buffer(enc->screen, enc->fb, 512, PIPE_USAGE_STAGING)) {
		RVID_ERR("Can't create feedback buffer.\n");
		return;
	}
	if (!radeon_emitted(&enc->cs, 0))
		enc->session(enc);
	enc->encode(enc);
	enc->feedback(enc);
}

static void rvce_end_frame(struct pipe_video_codec *encoder,
			   struct pipe_video_buffer *source,
			   struct pipe_picture_desc *picture)
{
	struct rvce_encoder *enc = (struct rvce_encoder*)encoder;
	struct rvce_cpb_slot *slot = list_entry(enc->cpb_slots.prev,
	                                        struct rvce_cpb_slot,
	                                        list);

	if (!enc->dual_inst || enc->bs_idx > 1)
		flush(enc);

	/* update the CPB backtrack with the just encoded frame */
	slot->picture_type = enc->pic.picture_type;
	slot->frame_num = enc->pic.frame_num;
	slot->pic_order_cnt = enc->pic.pic_order_cnt;
	if (!enc->pic.not_referenced) {
		list_del(&slot->list);
		list_add(&slot->list, &enc->cpb_slots);
	}
}

static void rvce_get_feedback(struct pipe_video_codec *encoder,
			      void *feedback, unsigned *size,
				  struct pipe_enc_feedback_metadata* metadata)
{
	struct rvce_encoder *enc = (struct rvce_encoder*)encoder;
	struct rvid_buffer *fb = feedback;

	if (size) {
		uint32_t *ptr = enc->ws->buffer_map(enc->ws,
			fb->res->buf, &enc->cs,
			PIPE_MAP_READ_WRITE | RADEON_MAP_TEMPORARY);

		if (ptr[1]) {
			*size = ptr[4] - ptr[9];
		} else {
			*size = 0;
		}

		enc->ws->buffer_unmap(enc->ws, fb->res->buf);
	}
	//dump_feedback(enc, fb);
	rvid_destroy_buffer(fb);
	FREE(fb);
}

/**
 * flush any outstanding command buffers to the hardware
 */
static void rvce_flush(struct pipe_video_codec *encoder)
{
	struct rvce_encoder *enc = (struct rvce_encoder*)encoder;

	flush(enc);
}

static void rvce_cs_flush(void *ctx, unsigned flags,
			  struct pipe_fence_handle **fence)
{
	// just ignored
}

struct pipe_video_codec *rvce_create_encoder(struct pipe_context *context,
					     const struct pipe_video_codec *templ,
					     struct radeon_winsys* ws,
					     rvce_get_buffer get_buffer)
{
	struct r600_common_screen *rscreen = (struct r600_common_screen *)context->screen;
	struct r600_common_context *rctx = (struct r600_common_context*)context;
	struct rvce_encoder *enc;
	struct pipe_video_buffer *tmp_buf, templat = {};
	struct radeon_surf *tmp_surf;
	unsigned cpb_size;

	if (!rscreen->info.vce_fw_version) {
		RVID_ERR("Kernel doesn't supports VCE!\n");
		return NULL;

	} else if (!rvce_is_fw_version_supported(rscreen)) {
		RVID_ERR("Unsupported VCE fw version loaded!\n");
		return NULL;
	}

	enc = CALLOC_STRUCT(rvce_encoder);
	if (!enc)
		return NULL;

	enc->use_vui = true;

	enc->base = *templ;
	enc->base.context = context;

	enc->base.destroy = rvce_destroy;
	enc->base.begin_frame = rvce_begin_frame;
	enc->base.encode_bitstream = rvce_encode_bitstream;
	enc->base.end_frame = rvce_end_frame;
	enc->base.flush = rvce_flush;
	enc->base.get_feedback = rvce_get_feedback;
	enc->get_buffer = get_buffer;

	enc->screen = context->screen;
	enc->ws = ws;

	if (!ws->cs_create(&enc->cs, rctx->ctx, AMD_IP_VCE, rvce_cs_flush, enc)) {
		RVID_ERR("Can't get command submission context.\n");
		goto error;
	}

	templat.buffer_format = PIPE_FORMAT_NV12;
	templat.width = enc->base.width;
	templat.height = enc->base.height;
	templat.interlaced = false;
	if (!(tmp_buf = context->create_video_buffer(context, &templat))) {
		RVID_ERR("Can't create video buffer.\n");
		goto error;
	}

	enc->cpb_num = get_cpb_num(enc);
	if (!enc->cpb_num)
		goto error;

	get_buffer(((struct vl_video_buffer *)tmp_buf)->resources[0], NULL, &tmp_surf);

	cpb_size = align(tmp_surf->u.legacy.level[0].nblk_x * tmp_surf->bpe, 128) *
	  align(tmp_surf->u.legacy.level[0].nblk_y, 32);

	cpb_size = cpb_size * 3 / 2;
	cpb_size = cpb_size * enc->cpb_num;
	if (enc->dual_pipe)
		cpb_size +=  RVCE_MAX_AUX_BUFFER_NUM *
			RVCE_MAX_BITSTREAM_OUTPUT_ROW_SIZE * 2;
	tmp_buf->destroy(tmp_buf);
	if (!rvid_create_buffer(enc->screen, &enc->cpb, cpb_size, PIPE_USAGE_DEFAULT)) {
		RVID_ERR("Can't create CPB buffer.\n");
		goto error;
	}

	enc->cpb_array = CALLOC(enc->cpb_num, sizeof(struct rvce_cpb_slot));
	if (!enc->cpb_array)
		goto error;

	reset_cpb(enc);

	goto error;

	return &enc->base;

error:
	enc->ws->cs_destroy(&enc->cs);

	rvid_destroy_buffer(&enc->cpb);

	FREE(enc->cpb_array);
	FREE(enc);
	return NULL;
}

/**
 * check if kernel has the right fw version loaded
 */
bool rvce_is_fw_version_supported(struct r600_common_screen *rscreen)
{
	switch (rscreen->info.vce_fw_version) {
	case FW_40_2_2:
	case FW_50_0_1:
	case FW_50_1_2:
	case FW_50_10_2:
	case FW_50_17_3:
	case FW_52_0_3:
	case FW_52_4_3:
	case FW_52_8_3:
		return true;
	default:
		if ((rscreen->info.vce_fw_version & (0xff << 24)) == FW_53)
			return true;
		else
			return false;
	}
}

/**
 * Add the buffer as relocation to the current command submission
 */
void rvce_add_buffer(struct rvce_encoder *enc, struct pb_buffer_lean *buf,
                     unsigned usage, enum radeon_bo_domain domain,
                     signed offset)
{
	int reloc_idx;

	reloc_idx = enc->ws->cs_add_buffer(&enc->cs, buf, usage | RADEON_USAGE_SYNCHRONIZED,
					   domain);
	if (enc->use_vm) {
		uint64_t addr;
		addr = enc->ws->buffer_get_virtual_address(buf);
		addr = addr + offset;
		RVCE_CS(addr >> 32);
		RVCE_CS(addr);
	} else {
		offset += enc->ws->buffer_get_reloc_offset(buf);
		RVCE_CS(reloc_idx * 4);
		RVCE_CS(offset);
	}
}
