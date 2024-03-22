
/*
 * Copyright © Microsoft Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef D3D12_VIDEO_ENC_HEVC_H
#define D3D12_VIDEO_ENC_HEVC_H
#include "d3d12_video_types.h"

bool
d3d12_video_encoder_update_current_encoder_config_state_hevc(struct d3d12_video_encoder *pD3D12Enc,
                                                             D3D12_VIDEO_SAMPLE srcTextureDesc,
                                                             struct pipe_picture_desc *  picture);
void
d3d12_video_encoder_update_current_rate_control_hevc(struct d3d12_video_encoder *pD3D12Enc,
                                                     pipe_h265_enc_picture_desc *picture);
bool
d3d12_video_encoder_negotiate_current_hevc_slices_configuration(struct d3d12_video_encoder *pD3D12Enc,
                                                             pipe_h265_enc_picture_desc *picture);
bool
d3d12_video_encoder_update_hevc_gop_configuration(struct d3d12_video_encoder *pD3D12Enc,
                                                  pipe_h265_enc_picture_desc *picture);
D3D12_VIDEO_ENCODER_MOTION_ESTIMATION_PRECISION_MODE
d3d12_video_encoder_convert_hevc_motion_configuration(struct d3d12_video_encoder *pD3D12Enc,
                                                      pipe_h265_enc_picture_desc *picture);
D3D12_VIDEO_ENCODER_LEVELS_HEVC
d3d12_video_encoder_convert_level_hevc(uint32_t hevcSpecLevel);
D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC
d3d12_video_encoder_convert_hevc_codec_configuration(struct d3d12_video_encoder *pD3D12Enc,
                                                     pipe_h265_enc_picture_desc *picture,
                                                     bool &is_supported);
void
d3d12_video_encoder_update_current_frame_pic_params_info_hevc(struct d3d12_video_encoder *pD3D12Enc,
                                                              struct pipe_video_buffer *  srcTexture,
                                                              struct pipe_picture_desc *  picture,
                                                              D3D12_VIDEO_ENCODER_PICTURE_CONTROL_CODEC_DATA &picParams,
                                                             bool &bUsedAsReference);
D3D12_VIDEO_ENCODER_FRAME_TYPE_HEVC
d3d12_video_encoder_convert_frame_type_hevc(enum pipe_h2645_enc_picture_type picType);
uint32_t
d3d12_video_encoder_build_codec_headers_hevc(struct d3d12_video_encoder *pD3D12Enc,
                                             std::vector<uint64_t> &pWrittenCodecUnitsSizes);
bool
d3d12_video_encoder_isequal_slice_config_hevc(
   D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE                   targetMode,
   D3D12_VIDEO_ENCODER_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA_SLICES targetConfig,
   D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE                   otherMode,
   D3D12_VIDEO_ENCODER_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA_SLICES otherConfig);

#endif
