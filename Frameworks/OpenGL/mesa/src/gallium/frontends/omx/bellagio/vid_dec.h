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

#ifndef OMX_VID_DEC_H
#define OMX_VID_DEC_H

#include <string.h>

#include <OMX_Types.h>
#include <OMX_Component.h>

#include "util/u_thread.h"

#include "vid_dec_common.h"

#define OMX_VID_DEC_BASE_NAME "OMX.mesa.video_decoder"

#define OMX_VID_DEC_MPEG2_NAME "OMX.mesa.video_decoder.mpeg2"
#define OMX_VID_DEC_MPEG2_ROLE "video_decoder.mpeg2"

#define OMX_VID_DEC_AVC_NAME "OMX.mesa.video_decoder.avc"
#define OMX_VID_DEC_AVC_ROLE "video_decoder.avc"

#define OMX_VID_DEC_HEVC_NAME "OMX.mesa.video_decoder.hevc"
#define OMX_VID_DEC_HEVC_ROLE "video_decoder.hevc"

#define OMX_VID_DEC_AV1_NAME "OMX.mesa.video_decoder.av1"
#define OMX_VID_DEC_AV1_ROLE "video_decoder.av1"

#define OMX_VID_DEC_TIMESTAMP_INVALID ((OMX_TICKS) -1)

OMX_ERRORTYPE vid_dec_LoaderComponent(stLoaderComponentType *comp);

/* vid_dec_mpeg12.c */
void vid_dec_mpeg12_Init(vid_dec_PrivateType *priv);

/* vid_dec_h264.c */
void vid_dec_h264_Init(vid_dec_PrivateType *priv);

/* vid_dec_h265.c */
void vid_dec_h265_Init(vid_dec_PrivateType *priv);

/* vid_dec_av1.c */
void vid_dec_av1_Init(vid_dec_PrivateType *priv);

OMX_ERRORTYPE vid_dec_av1_AllocateInBuffer(omx_base_PortType *port,
                                           OMX_INOUT OMX_BUFFERHEADERTYPE **buf,
                                           OMX_IN OMX_U32 idx, OMX_IN OMX_PTR private,
                                           OMX_IN OMX_U32 size);

OMX_ERRORTYPE vid_dec_av1_UseInBuffer(omx_base_PortType *port, OMX_BUFFERHEADERTYPE **buf,
                                      OMX_U32 idx, OMX_PTR private, OMX_U32 size, OMX_U8 *mem);

void vid_dec_av1_FrameDecoded(OMX_COMPONENTTYPE *comp, OMX_BUFFERHEADERTYPE* input,
                              OMX_BUFFERHEADERTYPE* output);

void vid_dec_av1_ReleaseTasks(vid_dec_PrivateType *priv);

void vid_dec_av1_FreeInputPortPrivate(vid_dec_PrivateType *priv, OMX_BUFFERHEADERTYPE *buf);

#endif
