/*
 * Copyright 2013 Ilia Mirkin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef NV84_VIDEO_H_
#define NV84_VIDEO_H_

#include "vl/vl_decoder.h"
#include "vl/vl_video_buffer.h"
#include "vl/vl_types.h"

#include "vl/vl_mpeg12_bitstream.h"

#include "util/u_video.h"

#include "nv50/nv50_context.h"

/* These are expected to be on their own pushbufs */
#define SUBC_BSP(m) 2, (m)
#define SUBC_VP(m) 2, (m)

union pipe_desc {
   struct pipe_picture_desc *base;
   struct pipe_mpeg12_picture_desc *mpeg12;
   struct pipe_mpeg4_picture_desc *mpeg4;
   struct pipe_vc1_picture_desc *vc1;
   struct pipe_h264_picture_desc *h264;
};

struct nv84_video_buffer {
   struct pipe_video_buffer base;
   struct pipe_resource *resources[VL_NUM_COMPONENTS];
   struct pipe_sampler_view *sampler_view_planes[VL_NUM_COMPONENTS];
   struct pipe_sampler_view *sampler_view_components[VL_NUM_COMPONENTS];
   struct pipe_surface *surfaces[VL_NUM_COMPONENTS * 2];

   struct nouveau_bo *interlaced, *full;
   int mvidx;
   unsigned frame_num, frame_num_max;
};

struct nv84_decoder {
   struct pipe_video_codec base;
   struct nouveau_client *client;
   struct nouveau_object *bsp_channel, *vp_channel, *bsp, *vp;
   struct nouveau_pushbuf *bsp_pushbuf, *vp_pushbuf;
   struct nouveau_bufctx *bsp_bufctx, *vp_bufctx;

   struct nouveau_bo *bsp_fw, *bsp_data;
   struct nouveau_bo *vp_fw, *vp_data;
   struct nouveau_bo *mbring, *vpring;

   /*
    * states:
    *  0: init
    *  1: vpring/mbring cleared, bsp is ready
    *  2: bsp is done, vp is ready
    * and then vp it back to 1
    */
   struct nouveau_bo *fence;

   struct nouveau_bo *bitstream;
   struct nouveau_bo *vp_params;

   size_t vp_fw2_offset;

   unsigned frame_mbs, frame_size;
   /* VPRING layout:
        RESIDUAL
        CTRL
        DEBLOCK
        0x1000
   */
   unsigned vpring_deblock, vpring_residual, vpring_ctrl;


   struct vl_mpg12_bs *mpeg12_bs;

   struct nouveau_bo *mpeg12_bo;
   void *mpeg12_mb_info;
   uint16_t *mpeg12_data;
   const int *zscan;
   uint8_t mpeg12_intra_matrix[64];
   uint8_t mpeg12_non_intra_matrix[64];
};

static inline uint32_t mb(uint32_t coord)
{
   return (coord + 0xf)>>4;
}

static inline uint32_t mb_half(uint32_t coord)
{
   return (coord + 0x1f)>>5;
}

int
nv84_decoder_bsp(struct nv84_decoder *dec,
                 struct pipe_h264_picture_desc *desc,
                 unsigned num_buffers,
                 const void *const *data,
                 const unsigned *num_bytes,
                 struct nv84_video_buffer *dest);

void
nv84_decoder_vp_h264(struct nv84_decoder *dec,
                     struct pipe_h264_picture_desc *desc,
                     struct nv84_video_buffer *dest);

void
nv84_decoder_vp_mpeg12_mb(struct nv84_decoder *dec,
                          struct pipe_mpeg12_picture_desc *desc,
                          const struct pipe_mpeg12_macroblock *mb);

void
nv84_decoder_vp_mpeg12(struct nv84_decoder *dec,
                       struct pipe_mpeg12_picture_desc *desc,
                       struct nv84_video_buffer *dest);

#endif
