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

#include "pipe/p_video_codec.h"
#include "util/u_memory.h"
#include "util/u_video.h"
#include "util/vl_rbsp.h"
#include "vl/vl_zscan.h"

#include "entrypoint.h"
#include "vid_dec.h"
#include "vid_dec_h264_common.h"

void vid_dec_h264_Init(vid_dec_PrivateType *priv)
{
   priv->picture.base.profile = PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH;

   priv->Decode = vid_dec_h264_Decode;
   priv->EndFrame = vid_dec_h264_EndFrame;
   priv->Flush = vid_dec_h264_Flush;

   list_inithead(&priv->codec_data.h264.dpb_list);
   priv->picture.h264.field_order_cnt[0] = priv->picture.h264.field_order_cnt[1] = INT_MAX;
   priv->first_buf_in_frame = true;
}
