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


#ifndef D3D12_VIDEO_ENCODE_REFERENCES_MANAGER_INTERFACE_H
#define D3D12_VIDEO_ENCODE_REFERENCES_MANAGER_INTERFACE_H

#include "d3d12_video_types.h"

class d3d12_video_encoder_references_manager_interface
{
 public:
   virtual void                                      begin_frame(D3D12_VIDEO_ENCODER_PICTURE_CONTROL_CODEC_DATA, bool bUsedAsReference, struct pipe_picture_desc* picture) = 0;
   virtual void                                      end_frame()                                                 = 0;
   virtual D3D12_VIDEO_ENCODER_RECONSTRUCTED_PICTURE get_current_frame_recon_pic_output_allocation()             = 0;
   virtual bool
                                               get_current_frame_picture_control_data(D3D12_VIDEO_ENCODER_PICTURE_CONTROL_CODEC_DATA &codecAllocation) = 0;
   virtual bool                                is_current_frame_used_as_reference() = 0;
   virtual D3D12_VIDEO_ENCODE_REFERENCE_FRAMES get_current_reference_frames()       = 0;
   virtual ~d3d12_video_encoder_references_manager_interface()
   { }
};

#endif
