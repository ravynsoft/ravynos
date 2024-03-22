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

#ifndef H264E_H
#define H264E_H

#include <OMX_Core.h>
#include <OMX_Types.h>
#include <OMX_Video.h>

#define OMX_VID_ENC_AVC_ROLE "video_encoder.avc"

/* With libtizonia, port indexes must start at index 0 */
#define OMX_VID_ENC_AVC_INPUT_PORT_INDEX 0
#define OMX_VID_ENC_AVC_OUTPUT_PORT_INDEX 1
#define OMX_VID_ENC_AVC_DEFAULT_FRAME_WIDTH 176
#define OMX_VID_ENC_AVC_DEFAULT_FRAME_HEIGHT 144
#define OMX_VID_ENC_AVC_INPUT_PORT_MIN_BUF_COUNT 8
#define OMX_VID_ENC_AVC_OUTPUT_PORT_MIN_BUF_COUNT 2
#define OMX_VID_ENC_AVC_PORT_MIN_INPUT_BUF_SIZE 4 * 1024
#define OMX_VID_ENC_AVC_PORT_MIN_OUTPUT_BUF_SIZE 345600
#define OMX_VID_ENC_AVC_PORT_NONCONTIGUOUS OMX_FALSE
#define OMX_VID_ENC_AVC_PORT_ALIGNMENT 0
#define OMX_VID_ENC_AVC_PORT_SUPPLIERPREF OMX_BufferSupplyInput

OMX_PTR instantiate_h264e_config_port(OMX_HANDLETYPE ap_hdl);
OMX_PTR instantiate_h264e_input_port(OMX_HANDLETYPE ap_hdl);
OMX_PTR instantiate_h264e_output_port(OMX_HANDLETYPE ap_hdl);
OMX_PTR instantiate_h264e_processor(OMX_HANDLETYPE ap_hdl);

#endif                          /* H264E_H */
