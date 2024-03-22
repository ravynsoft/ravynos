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

#ifndef D3D12_VIDEO_ENCODE_FIFO_REFERENCES_MANAGER_AV1_H
#define D3D12_VIDEO_ENCODE_FIFO_REFERENCES_MANAGER_AV1_H

#include "d3d12_video_types.h"
#include "d3d12_video_encoder_references_manager.h"
#include "d3d12_video_dpb_storage_manager.h"

class d3d12_video_encoder_references_manager_av1 : public d3d12_video_encoder_references_manager_interface
{
 public:
   void end_frame();
   void begin_frame(D3D12_VIDEO_ENCODER_PICTURE_CONTROL_CODEC_DATA curFrameData,
                    bool bUsedAsReference,
                    struct pipe_picture_desc *picture);
   D3D12_VIDEO_ENCODER_RECONSTRUCTED_PICTURE get_current_frame_recon_pic_output_allocation();
   bool get_current_frame_picture_control_data(D3D12_VIDEO_ENCODER_PICTURE_CONTROL_CODEC_DATA &codecAllocation);
   bool is_current_frame_used_as_reference();
   D3D12_VIDEO_ENCODE_REFERENCE_FRAMES get_current_reference_frames();

   d3d12_video_encoder_references_manager_av1(bool gopHasInterCodedFrames,
                                              d3d12_video_dpb_storage_manager_interface &rDpbStorageManager);

   ~d3d12_video_encoder_references_manager_av1()
   { }

 private:
   // Class helpers

   void clear_dpb();
   void refresh_dpb_slots_with_current_frame_reconpic();

   uint32_t get_dpb_virtual_slot_refcount_from_ref_frame_idx(uint32_t dpbSlotIndex);
   uint32_t get_dpb_physical_slot_refcount_from_virtual_dpb(uint32_t ReconstructedPictureResourceIndex);

   void print_virtual_dpb_entries();
   void print_physical_resource_references();
   void print_ref_frame_idx();

   // Class members

   struct current_frame_references_data
   {
      std::vector<D3D12_VIDEO_ENCODER_AV1_REFERENCE_PICTURE_DESCRIPTOR> pVirtualDPBEntries;
      D3D12_VIDEO_ENCODER_RECONSTRUCTED_PICTURE ReconstructedPicTexture;
   } m_CurrentFrameReferencesData = {};
   d3d12_video_dpb_storage_manager_interface &m_PhysicalAllocationsStorage;
   bool m_gopHasInterFrames = false;
   bool m_isCurrentFrameUsedAsReference = false;
   D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_CODEC_DATA m_CurrentFramePicParams = {};

   // Constants
   const uint32_t NUM_REF_FRAMES = 8;
   const uint32_t REFS_PER_FRAME = 7;
};

#endif
