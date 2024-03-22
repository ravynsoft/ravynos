
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

#ifndef D3D12_VIDEO_ENC_AV1_H
#define D3D12_VIDEO_ENC_AV1_H
#include "d3d12_video_types.h"

const uint32_t UNUSED_VIRTUAL_DPB_SLOT_PHYSICAL_INDEX = 0xFF;   // As per D3D12 spec

size_t
d3d12_video_encoder_calculate_metadata_resolved_buffer_size_av1(uint32_t maxSliceNumber);

bool
d3d12_video_encoder_update_current_encoder_config_state_av1(struct d3d12_video_encoder *pD3D12Enc,
                                                            D3D12_VIDEO_SAMPLE srcTextureDesc,
                                                            struct pipe_picture_desc *picture);

void
d3d12_video_encoder_update_current_frame_pic_params_info_av1(struct d3d12_video_encoder *pD3D12Enc,
                                                             struct pipe_video_buffer *srcTexture,
                                                             struct pipe_picture_desc *picture,
                                                             D3D12_VIDEO_ENCODER_PICTURE_CONTROL_CODEC_DATA &picParams,
                                                             bool &bUsedAsReference);

unsigned
d3d12_video_encoder_build_post_encode_codec_bitstream_av1(struct d3d12_video_encoder *pD3D12Enc,
                                                          uint64_t associated_fence_value,
                                                          EncodedBitstreamResolvedMetadata &associatedMetadata);

D3D12_VIDEO_ENCODER_AV1_RESTORATION_TILESIZE
d3d12_video_encoder_looprestorationsize_uint_to_d3d12_av1(uint32_t pixel_size);

unsigned
d3d12_video_encoder_looprestorationsize_d3d12_to_uint_av1(D3D12_VIDEO_ENCODER_AV1_RESTORATION_TILESIZE d3d12_type);

void
upload_tile_group_obu(struct d3d12_video_encoder *pD3D12Enc,
                      size_t tile_group_obu_size,
                      size_t decode_tile_elements_size,
                      std::vector<uint8_t> &staging_bitstream_buffer,
                      size_t staging_bitstream_buffer_offset,
                      pipe_resource *src_driver_bitstream,
                      pipe_resource *comp_bit_destination,
                      size_t comp_bit_destination_offset,
                      const D3D12_VIDEO_ENCODER_FRAME_SUBREGION_METADATA *pFrameSubregionMetadata,
                      size_t TileSizeBytes,   // Pass already +1'd from TileSizeBytesMinus1
                      const D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA_TILES &TilesPartition,
                      const av1_tile_group_t &tileGroup,
                      size_t &written_bytes_to_staging_bitstream_buffer,
                      std::vector<uint64_t> &pWrittenCodecUnitsSizes);


void
d3d12_video_encoder_store_current_picture_references_av1(d3d12_video_encoder *pD3D12Enc,
                                                         uint64_t current_metadata_slot);

#endif
