/*
 * Copyright 2011-2013 Maarten Lankhorst
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

#include <nouveau.h>

#include "pipe/p_defines.h"
#include "vl/vl_video_buffer.h"
#include "util/u_video.h"

struct nouveau_vp3_video_buffer {
   struct pipe_video_buffer base;
   unsigned num_planes, valid_ref;
   struct pipe_resource *resources[VL_NUM_COMPONENTS];
   struct pipe_sampler_view *sampler_view_planes[VL_NUM_COMPONENTS];
   struct pipe_sampler_view *sampler_view_components[VL_NUM_COMPONENTS];
   struct pipe_surface *surfaces[VL_NUM_COMPONENTS * 2];
};

#define SLICE_SIZE 0x200
#define VP_OFFSET 0x200
#define COMM_OFFSET 0x500

#define NOUVEAU_VP3_BSP_RESERVED_SIZE 0x700

#define NOUVEAU_VP3_DEBUG_FENCE 0

#if NOUVEAU_VP3_DEBUG_FENCE
# define NOUVEAU_VP3_VIDEO_QDEPTH 1
#else
# define NOUVEAU_VP3_VIDEO_QDEPTH 2
#endif

#define SUBC_BSP(m) dec->bsp_idx, (m)
#define SUBC_VP(m) dec->vp_idx, (m)
#define SUBC_PPP(m) dec->ppp_idx, (m)

union pipe_desc {
   struct pipe_picture_desc *base;
   struct pipe_mpeg12_picture_desc *mpeg12;
   struct pipe_mpeg4_picture_desc *mpeg4;
   struct pipe_vc1_picture_desc *vc1;
   struct pipe_h264_picture_desc *h264;
};

struct nouveau_vp3_decoder {
   struct pipe_video_codec base;
   struct nouveau_client *client;
   struct nouveau_object *channel[3], *bsp, *vp, *ppp;
   struct nouveau_pushbuf *pushbuf[3];

#if NOUVEAU_VP3_DEBUG_FENCE
   /* dump fence and comm, as needed.. */
   unsigned *fence_map;
   struct comm *comm;

   struct nouveau_bo *fence_bo;
#endif

   struct nouveau_bo *fw_bo, *bitplane_bo;

   // array size max_references + 2, contains unpostprocessed images
   // added at the end of ref_bo is a tmp array
   // tmp is an array for h264, with each member being used for a ref frame or current
   // target.. size = (((mb(w)*((mb(h)+1)&~1))+3)>>2)<<8 * (max_references+1)
   // for other codecs, it simply seems that size = w*h is enough
   // unsure what it's supposed to contain..
   struct nouveau_bo *ref_bo;

   struct nouveau_bo *inter_bo[2];

   struct nouveau_bo *bsp_bo[NOUVEAU_VP3_VIDEO_QDEPTH];

   // bo's used by each cycle:

   // bsp_bo: contains raw bitstream data and parameters for BSP and VP.
   // inter_bo: contains data shared between BSP and VP
   // ref_bo: reference image data, used by PPP and VP
   // bitplane_bo: contain bitplane data (similar to ref_bo), used by BSP only
   // fw_bo: used by VP only.

   // Needed amount of copies in optimal case:
   // 2 copies of inter_bo, VP would process the last inter_bo, while BSP is
   // writing out a new set.
   // NOUVEAU_VP3_VIDEO_QDEPTH copies of bsp_bo. We don't want to block the
   // pipeline ever, and give shaders a chance to run as well.

   struct {
      struct nouveau_vp3_video_buffer *vidbuf;
      unsigned last_used;
      unsigned field_pic_flag : 1;
      unsigned decoded_top : 1;
      unsigned decoded_bottom : 1;
      unsigned decoded_first : 1;
   } refs[17];
   unsigned fence_seq, fw_sizes, last_frame_num, tmp_stride, ref_stride;

   unsigned bsp_idx, vp_idx, ppp_idx;

   /* End of the bsp bo where new data should be appended between one begin/end
    * frame.
    */
   char *bsp_ptr;
};

struct comm {
   uint32_t bsp_cur_index; // 000
   uint32_t byte_ofs; // 004
   uint32_t status[0x10]; // 008
   uint32_t pos[0x10]; // 048
   uint8_t pad[0x100 - 0x88]; // 0a0 bool comm_encrypted

   uint32_t pvp_cur_index; // 100
   uint32_t acked_byte_ofs; // 104
   uint32_t status_vp[0x10]; // 108
   uint16_t mb_y[0x10]; //148
   uint32_t pvp_stage; // 168 0xeeXX
   uint16_t parse_endpos_index; // 16c
   uint16_t irq_index; // 16e
   uint8_t  irq_470[0x10]; // 170
   uint32_t irq_pos[0x10]; // 180
   uint32_t parse_endpos[0x10]; // 1c0
};

static inline uint32_t nouveau_vp3_video_align(uint32_t h)
{
   return ((h+0x3f)&~0x3f);
};

static inline uint32_t mb(uint32_t coord)
{
   return (coord + 0xf)>>4;
}

static inline uint32_t mb_half(uint32_t coord)
{
   return (coord + 0x1f)>>5;
}

static inline uint64_t
nouveau_vp3_video_addr(struct nouveau_vp3_decoder *dec, struct nouveau_vp3_video_buffer *target)
{
   uint64_t ret;
   if (target)
      ret = dec->ref_stride * target->valid_ref;
   else
      ret = dec->ref_stride * (dec->base.max_references+1);
   return dec->ref_bo->offset + ret;
}

static inline void
nouveau_vp3_ycbcr_offsets(struct nouveau_vp3_decoder *dec, uint32_t *y2,
                          uint32_t *cbcr, uint32_t *cbcr2)
{
   uint32_t w = mb(dec->base.width), size;
   *y2 = mb_half(dec->base.height)*w;
   *cbcr = *y2 * 2;
   *cbcr2 = *cbcr + w * (nouveau_vp3_video_align(dec->base.height)>>6);

   /* The check here should never fail because it means a bug
    * in the code rather than a bug in hardware..
    */
   size = (2 * (*cbcr2 - *cbcr) + *cbcr) << 8;
   if (size > dec->ref_stride) {
      debug_printf("Overshot ref_stride (%u) with size %u and ofs (%u,%u,%u)\n",
                   dec->ref_stride, size, *y2<<8, *cbcr<<8, *cbcr2<<8);
      *y2 = *cbcr = *cbcr2 = 0;
      assert(size <= dec->ref_stride);
   }
}

static inline void
nouveau_vp3_inter_sizes(struct nouveau_vp3_decoder *dec, uint32_t slice_count,
                        uint32_t *slice_size, uint32_t *bucket_size,
                        uint32_t *ring_size)
{
   *slice_size = (SLICE_SIZE * slice_count)>>8;
   if (u_reduce_video_profile(dec->base.profile) == PIPE_VIDEO_FORMAT_MPEG12)
      *bucket_size = 0;
   else
      *bucket_size = mb(dec->base.width) * 3;
   *ring_size = (dec->inter_bo[0]->size >> 8) - *bucket_size - *slice_size;
}

struct pipe_video_buffer *
nouveau_vp3_video_buffer_create(struct pipe_context *pipe,
                                const struct pipe_video_buffer *templat,
                                int flags);

void
nouveau_vp3_decoder_init_common(struct pipe_video_codec *decoder);

int
nouveau_vp3_load_firmware(struct nouveau_vp3_decoder *dec,
                          enum pipe_video_profile profile,
                          unsigned chipset);

void
nouveau_vp3_bsp_begin(struct nouveau_vp3_decoder *dec);

void
nouveau_vp3_bsp_next(struct nouveau_vp3_decoder *dec, unsigned num_buffers,
                     const void *const *data, const unsigned *num_bytes);

uint32_t
nouveau_vp3_bsp_end(struct nouveau_vp3_decoder *dec, union pipe_desc desc);

void
nouveau_vp3_vp_caps(struct nouveau_vp3_decoder *dec, union pipe_desc desc,
                    struct nouveau_vp3_video_buffer *target, unsigned comm_seq,
                    unsigned *caps, unsigned *is_ref,
                    struct nouveau_vp3_video_buffer *refs[16]);

int
nouveau_vp3_screen_get_video_param(struct pipe_screen *pscreen,
                                   enum pipe_video_profile profile,
                                   enum pipe_video_entrypoint entrypoint,
                                   enum pipe_video_cap param);

bool
nouveau_vp3_screen_video_supported(struct pipe_screen *screen,
                                   enum pipe_format format,
                                   enum pipe_video_profile profile,
                                   enum pipe_video_entrypoint entrypoint);
