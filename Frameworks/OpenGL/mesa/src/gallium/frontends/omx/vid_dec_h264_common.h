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

#ifndef VID_DEC_H264_COMMON_H
#define VID_DEC_H264_COMMON_H

#include "vid_dec_common.h"

#define OMX_VID_DEC_AVC_DEFAULT_FRAME_WIDTH      176
#define OMX_VID_DEC_AVC_DEFAULT_FRAME_HEIGHT     144
#define OMX_VID_DEC_AVC_DEFAULT_FRAME_RATE 15<<16
#define OMX_VID_DEC_AVC_ROLE "video_decoder.avc"
/* With libtizonia, port indexes must start at index 0 */
#define OMX_VID_DEC_AVC_INPUT_PORT_INDEX               0
#define OMX_VID_DEC_AVC_OUTPUT_PORT_INDEX              1
#define OMX_VID_DEC_AVC_INPUT_PORT_MIN_BUF_COUNT       8
#define OMX_VID_DEC_AVC_OUTPUT_PORT_MIN_BUF_COUNT      4
/* 38016 = (width * height) + ((width * height)/2) */
#define OMX_VID_DEC_AVC_PORT_MIN_INPUT_BUF_SIZE  38016
#define OMX_VID_DEC_AVC_PORT_MIN_OUTPUT_BUF_SIZE 345600
#define OMX_VID_DEC_AVC_PORT_NONCONTIGUOUS       OMX_FALSE
#define OMX_VID_DEC_AVC_PORT_ALIGNMENT           0
#define OMX_VID_DEC_AVC_PORT_SUPPLIERPREF        OMX_BufferSupplyInput
#define OMX_VID_DEC_AVC_TIMESTAMP_INVALID ((OMX_TICKS) -1)

#define DPB_MAX_SIZE 5

struct dpb_list {
   struct list_head list;
   struct pipe_video_buffer *buffer;
   OMX_TICKS timestamp;
   int poc;
};

static const uint8_t Default_4x4_Intra[16] = {
    6, 13, 20, 28, 13, 20, 28, 32,
   20, 28, 32, 37, 28, 32, 37, 42
};

static const uint8_t Default_4x4_Inter[16] = {
   10, 14, 20, 24, 14, 20, 24, 27,
   20, 24, 27, 30, 24, 27, 30, 34
};

static const uint8_t Default_8x8_Intra[64] = {
    6, 10, 13, 16, 18, 23, 25, 27,
   10, 11, 16, 18, 23, 25, 27, 29,
   13, 16, 18, 23, 25, 27, 29, 31,
   16, 18, 23, 25, 27, 29, 31, 33,
   18, 23, 25, 27, 29, 31, 33, 36,
   23, 25, 27, 29, 31, 33, 36, 38,
   25, 27, 29, 31, 33, 36, 38, 40,
   27, 29, 31, 33, 36, 38, 40, 42
};

static const uint8_t Default_8x8_Inter[64] = {
    9, 13, 15, 17, 19, 21, 22, 24,
   13, 13, 17, 19, 21, 22, 24, 25,
   15, 17, 19, 21, 22, 24, 25, 27,
   17, 19, 21, 22, 24, 25, 27, 28,
   19, 21, 22, 24, 25, 27, 28, 30,
   21, 22, 24, 25, 27, 28, 30, 32,
   22, 24, 25, 27, 28, 30, 32, 33,
   24, 25, 27, 28, 30, 32, 33, 35
};

struct pipe_video_buffer *vid_dec_h264_Flush(vid_dec_PrivateType *priv,
                                             OMX_TICKS *timestamp);
void vid_dec_h264_EndFrame(vid_dec_PrivateType *priv);
void vid_dec_h264_Decode(vid_dec_PrivateType *priv, struct vl_vlc *vlc, unsigned min_bits_left);
void vid_dec_FreeInputPortPrivate(OMX_BUFFERHEADERTYPE *buf);
void vid_dec_FrameDecoded_common(vid_dec_PrivateType*priv, OMX_BUFFERHEADERTYPE* input,
                                 OMX_BUFFERHEADERTYPE* output);

#endif
