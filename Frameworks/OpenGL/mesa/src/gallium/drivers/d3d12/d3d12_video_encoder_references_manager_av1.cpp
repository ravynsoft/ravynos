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

#include "d3d12_video_enc.h"
#include "d3d12_video_enc_av1.h"
#include "d3d12_video_encoder_references_manager_av1.h"
#include <algorithm>
#include <string>
#include "d3d12_screen.h"

using namespace std;

d3d12_video_encoder_references_manager_av1::d3d12_video_encoder_references_manager_av1(
   bool gopHasInterFrames, d3d12_video_dpb_storage_manager_interface &rDpbStorageManager)
   : m_CurrentFrameReferencesData({}),
     m_PhysicalAllocationsStorage(rDpbStorageManager),
     m_gopHasInterFrames(gopHasInterFrames),
     m_isCurrentFrameUsedAsReference(false),
     m_CurrentFramePicParams({})
{
   assert((NUM_REF_FRAMES + 1 /*extra for cur frame output recon pic*/) ==
          m_PhysicalAllocationsStorage.get_number_of_tracked_allocations());

   debug_printf("[D3D12 Video Encoder Picture Manager AV1] Completed construction of "
                "d3d12_video_encoder_references_manager_AV1 instance.\n");
}

void
d3d12_video_encoder_references_manager_av1::clear_dpb()
{
   // Reset m_CurrentFrameReferencesData tracking
   m_CurrentFrameReferencesData.pVirtualDPBEntries.clear();
   m_CurrentFrameReferencesData.pVirtualDPBEntries.resize(NUM_REF_FRAMES);
   m_CurrentFrameReferencesData.ReconstructedPicTexture = { nullptr, 0 };

   // Initialize DPB slots as unused
   for (size_t i = 0; i < NUM_REF_FRAMES; i++)
      m_CurrentFrameReferencesData.pVirtualDPBEntries[i].ReconstructedPictureResourceIndex =
         UNUSED_VIRTUAL_DPB_SLOT_PHYSICAL_INDEX;

   // Reset physical DPB underlying storage
   ASSERTED uint32_t numPicsBeforeClearInDPB = m_PhysicalAllocationsStorage.get_number_of_pics_in_dpb();
   ASSERTED uint32_t cFreedResources = m_PhysicalAllocationsStorage.clear_decode_picture_buffer();
   assert(numPicsBeforeClearInDPB == cFreedResources);
}

D3D12_VIDEO_ENCODER_RECONSTRUCTED_PICTURE
d3d12_video_encoder_references_manager_av1::get_current_frame_recon_pic_output_allocation()
{
   return m_CurrentFrameReferencesData.ReconstructedPicTexture;
}

bool
d3d12_video_encoder_references_manager_av1::is_current_frame_used_as_reference()
{
   return m_isCurrentFrameUsedAsReference;
}

void
d3d12_video_encoder_references_manager_av1::begin_frame(D3D12_VIDEO_ENCODER_PICTURE_CONTROL_CODEC_DATA curFrameData,
                                                        bool bUsedAsReference,
                                                        struct pipe_picture_desc *picture)
{
   m_CurrentFramePicParams = *curFrameData.pAV1PicData;
   m_isCurrentFrameUsedAsReference = bUsedAsReference;

   if (m_CurrentFramePicParams.FrameType == D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_KEY_FRAME)
      clear_dpb();

   // Prepare current frame recon pic allocation
   m_CurrentFrameReferencesData.ReconstructedPicTexture = { nullptr, 0 };
   if (is_current_frame_used_as_reference() && m_gopHasInterFrames) {
      auto reconPic = m_PhysicalAllocationsStorage.get_new_tracked_picture_allocation();
      m_CurrentFrameReferencesData.ReconstructedPicTexture.pReconstructedPicture = reconPic.pReconstructedPicture;
      m_CurrentFrameReferencesData.ReconstructedPicTexture.ReconstructedPictureSubresource =
         reconPic.ReconstructedPictureSubresource;
   }

#ifdef DEBUG
   assert(m_PhysicalAllocationsStorage.get_number_of_tracked_allocations() <=
          (NUM_REF_FRAMES + 1));   // pool is not extended beyond maximum expected usage

   if (m_CurrentFramePicParams.FrameType == D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_KEY_FRAME) {
      // After clearing the DPB, outstanding used allocations should be 1u only for the first allocation for the
      // reconstructed picture of the initial KEY_FRAME
      assert(m_PhysicalAllocationsStorage.get_number_of_in_use_allocations() ==
             ((is_current_frame_used_as_reference() && m_gopHasInterFrames) ? 1u : 0u));
   }
#endif
}

void
d3d12_video_encoder_references_manager_av1::end_frame()
{
   refresh_dpb_slots_with_current_frame_reconpic();
}

D3D12_VIDEO_ENCODE_REFERENCE_FRAMES
d3d12_video_encoder_references_manager_av1::get_current_reference_frames()
{
   D3D12_VIDEO_ENCODE_REFERENCE_FRAMES retVal = { 0,
                                                  // ppTexture2Ds
                                                  nullptr,
                                                  // pSubresources
                                                  nullptr };

   if ((m_CurrentFramePicParams.FrameType != D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_KEY_FRAME) && m_gopHasInterFrames) {
      auto curRef = m_PhysicalAllocationsStorage.get_current_reference_frames();
      retVal.NumTexture2Ds = curRef.NumTexture2Ds;
      retVal.ppTexture2Ds = curRef.ppTexture2Ds;
      retVal.pSubresources = curRef.pSubresources;
   }

   return retVal;
}

void
d3d12_video_encoder_references_manager_av1::print_ref_frame_idx()
{
   std::string refListContentsString;
   for (uint32_t idx = 0; idx < REFS_PER_FRAME; idx++) {
      uint32_t reference = 0;
      reference = m_CurrentFramePicParams.ReferenceIndices[idx];
      refListContentsString += "{ ref_frame_idx[" + std::to_string(idx) + "] - ";
      refListContentsString += " DPBidx: ";
      refListContentsString += std::to_string(reference);
      if (m_CurrentFrameReferencesData.pVirtualDPBEntries[reference].ReconstructedPictureResourceIndex !=
          UNUSED_VIRTUAL_DPB_SLOT_PHYSICAL_INDEX) {
         refListContentsString += " - OrderHint: ";
         refListContentsString += std::to_string(m_CurrentFrameReferencesData.pVirtualDPBEntries[reference].OrderHint);
         refListContentsString += " - PictureIndex: ";
         refListContentsString +=
            std::to_string(m_CurrentFrameReferencesData.pVirtualDPBEntries[reference].PictureIndex);
      } else {
         refListContentsString += " - Unused Virtual DPB slot ";
      }

      refListContentsString += " }\n";
   }

   debug_printf("[D3D12 Video Encoder Picture Manager AV1] ref_frame_idx[REFS_PER_FRAME] for frame with OrderHint %d "
                "(PictureIndex %d) is: \n%s \n",
                m_CurrentFramePicParams.OrderHint,
                m_CurrentFramePicParams.PictureIndex,
                refListContentsString.c_str());
   debug_printf("[D3D12 Video Encoder Picture Manager AV1] Requested PrimaryRefFrame: %d\n",
                m_CurrentFramePicParams.PrimaryRefFrame);
   debug_printf("[D3D12 Video Encoder Picture Manager AV1] Requested RefreshFrameFlags: %d\n",
                m_CurrentFramePicParams.RefreshFrameFlags);
}

void
d3d12_video_encoder_references_manager_av1::print_virtual_dpb_entries()
{
   std::string dpbContents;
   for (uint32_t dpbResIdx = 0; dpbResIdx < m_CurrentFrameReferencesData.pVirtualDPBEntries.size(); dpbResIdx++) {
      auto &dpbDesc = m_CurrentFrameReferencesData.pVirtualDPBEntries[dpbResIdx];

      if (dpbDesc.ReconstructedPictureResourceIndex != UNUSED_VIRTUAL_DPB_SLOT_PHYSICAL_INDEX) {
         d3d12_video_reconstructed_picture dpbEntry =
            m_PhysicalAllocationsStorage.get_reference_frame(dpbDesc.ReconstructedPictureResourceIndex);
         dpbContents += "{ DPBidx: ";
         dpbContents += std::to_string(dpbResIdx);
         dpbContents += " - OrderHint: ";
         dpbContents += std::to_string(dpbDesc.OrderHint);
         dpbContents += " - PictureIndex: ";
         dpbContents += std::to_string(dpbDesc.PictureIndex);
         dpbContents += " - DPBStorageIdx: ";
         dpbContents += std::to_string(dpbDesc.ReconstructedPictureResourceIndex);
         dpbContents += " - DPBStorageResourcePtr: ";
         char strBuf[256];
         memset(&strBuf, '\0', 256);
         sprintf(strBuf, "%p", dpbEntry.pReconstructedPicture);
         dpbContents += std::string(strBuf);
         dpbContents += " - DPBStorageSubresource: ";
         dpbContents += std::to_string(dpbEntry.ReconstructedPictureSubresource);
         dpbContents += " - RefCount (from any ref_frame_idx[0..6]): ";
         dpbContents += std::to_string(get_dpb_virtual_slot_refcount_from_ref_frame_idx(dpbResIdx));
         dpbContents += " }\n";
      } else {
         dpbContents += "{ DPBidx: ";
         dpbContents += std::to_string(dpbResIdx);
         dpbContents += " < Unused Virtual DPB slot > }\n";
      }
   }

   debug_printf(
      "[D3D12 Video Encoder Picture Manager AV1] DPB Current output reconstructed picture %p subresource %d\n",
      m_CurrentFrameReferencesData.ReconstructedPicTexture.pReconstructedPicture,
      m_CurrentFrameReferencesData.ReconstructedPicTexture.ReconstructedPictureSubresource);

   debug_printf("[D3D12 Video Encoder Picture Manager AV1] NumTexture2Ds is %d frames - "
                "Number of DPB virtual entries is %" PRIu64 " entries for frame with OrderHint "
                "%d (PictureIndex %d) are: \n%s \n",
                m_PhysicalAllocationsStorage.get_number_of_pics_in_dpb(),
                static_cast<uint64_t>(m_CurrentFrameReferencesData.pVirtualDPBEntries.size()),
                m_CurrentFramePicParams.OrderHint,
                m_CurrentFramePicParams.PictureIndex,
                dpbContents.c_str());
}

void
d3d12_video_encoder_references_manager_av1::print_physical_resource_references()
{
   debug_printf("[D3D12 Video Encoder Picture Manager AV1] %d physical resources IN USE out of a total of %d physical "
                "resources ALLOCATED "
                "resources at end_frame for frame with OrderHint %d (PictureIndex %d)\n",
                m_PhysicalAllocationsStorage.get_number_of_in_use_allocations(),
                m_PhysicalAllocationsStorage.get_number_of_tracked_allocations(),
                m_CurrentFramePicParams.OrderHint,
                m_CurrentFramePicParams.PictureIndex);

   D3D12_VIDEO_ENCODE_REFERENCE_FRAMES curRefs = get_current_reference_frames();
   std::string descContents;
   for (uint32_t index = 0; index < curRefs.NumTexture2Ds; index++) {
      assert(curRefs.ppTexture2Ds[index]);   // These must be contiguous when sending down to D3D12 API
      descContents += "{ REFERENCE_FRAMES Index: ";
      descContents += std::to_string(index);
      descContents += " - ppTextures ptr: ";
      char strBuf[256];
      memset(&strBuf, '\0', 256);
      sprintf(strBuf, "%p", curRefs.ppTexture2Ds[index]);
      descContents += std::string(strBuf);
      descContents += " - ppSubresources index: ";
      if (curRefs.pSubresources != nullptr)
         descContents += std::to_string(curRefs.pSubresources[index]);
      else
         descContents += "(nil)";
      descContents += " - RefCount (from any virtual dpb slot [0..REFS_PER_FRAME]): ";
      descContents += std::to_string(get_dpb_physical_slot_refcount_from_virtual_dpb(index));
      descContents += " }\n";
   }

   debug_printf("[D3D12 Video Encoder Picture Manager AV1] D3D12_VIDEO_ENCODE_REFERENCE_FRAMES has %d physical "
                "resources in ppTexture2Ds for OrderHint "
                "%d (PictureIndex %d) are: \n%s \n",
                curRefs.NumTexture2Ds,
                m_CurrentFramePicParams.OrderHint,
                m_CurrentFramePicParams.PictureIndex,
                descContents.c_str());
}

//
// Returns the number of Reference<XXX> (ie. LAST, LAST2, ..., ALTREF, etc)
// pointing to dpbSlotIndex
//
uint32_t
d3d12_video_encoder_references_manager_av1::get_dpb_virtual_slot_refcount_from_ref_frame_idx(uint32_t dpbSlotIndex)
{
   uint32_t refCount = 0;
   for (uint8_t i = 0; i < ARRAY_SIZE(m_CurrentFramePicParams.ReferenceIndices); i++) {
      if (m_CurrentFramePicParams.ReferenceIndices[i] == dpbSlotIndex) {
         refCount++;
      }
   }

   return refCount;
}

//
// Returns the number of entries in the virtual DPB descriptors
// that point to physicalSlotIndex
//
uint32_t
d3d12_video_encoder_references_manager_av1::get_dpb_physical_slot_refcount_from_virtual_dpb(uint32_t physicalSlotIndex)
{
   uint32_t refCount = 0;
   for (unsigned dpbSlotIndex = 0; dpbSlotIndex < m_CurrentFrameReferencesData.pVirtualDPBEntries.size();
        dpbSlotIndex++) {
      if (m_CurrentFrameReferencesData.pVirtualDPBEntries[dpbSlotIndex].ReconstructedPictureResourceIndex ==
          physicalSlotIndex)
         refCount++;
   }
   return refCount;
}

bool
d3d12_video_encoder_references_manager_av1::get_current_frame_picture_control_data(
   D3D12_VIDEO_ENCODER_PICTURE_CONTROL_CODEC_DATA &codecAllocation)
{
   assert(m_CurrentFrameReferencesData.pVirtualDPBEntries.size() == NUM_REF_FRAMES);
   assert(codecAllocation.DataSize == sizeof(D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_CODEC_DATA));

   // Some apps don't clean this up for INTRA/KEY frames
   if ((m_CurrentFramePicParams.FrameType != D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_INTER_FRAME)
   && (m_CurrentFramePicParams.FrameType != D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_SWITCH_FRAME))
   {
      for (uint8_t i = 0; i < ARRAY_SIZE(m_CurrentFramePicParams.ReferenceIndices); i++) {
         m_CurrentFramePicParams.ReferenceIndices[i] = 0;
      }
   }

   for (uint8_t i = 0; i < NUM_REF_FRAMES; i++)
      m_CurrentFramePicParams.ReferenceFramesReconPictureDescriptors[i] =
         m_CurrentFrameReferencesData.pVirtualDPBEntries[i];

#ifdef DEBUG   // Otherwise may iterate over structures and do no-op debug_printf
   print_ref_frame_idx();
   print_virtual_dpb_entries();
   print_physical_resource_references();
#endif

   *codecAllocation.pAV1PicData = m_CurrentFramePicParams;
   return true;
}

void
d3d12_video_encoder_references_manager_av1::refresh_dpb_slots_with_current_frame_reconpic()
{
   UINT refresh_frame_flags = m_CurrentFramePicParams.RefreshFrameFlags;
   debug_printf("[D3D12 Video Encoder Picture Manager AV1] refresh_frame_flags 0x%x for frame with OrderHint %d "
                "(PictureIndex %d)\n",
                refresh_frame_flags,
                m_CurrentFramePicParams.OrderHint,
                m_CurrentFramePicParams.PictureIndex);

   //
   // Put current frame in all slots of DPB indicated by refresh_frame_flags
   //
   if (is_current_frame_used_as_reference() && m_gopHasInterFrames && (refresh_frame_flags != 0)) {

      //
      // First do a eviction pass and update virtual DPB physical indices in case the physical array shifted with an
      // eviction (erasing an ppTexture2Ds entry)
      //

      for (unsigned dpbSlotIdx = 0; dpbSlotIdx < NUM_REF_FRAMES; dpbSlotIdx++) {
         if (((refresh_frame_flags >> dpbSlotIdx) & 0x1) != 0) {

            //
            // Check if the slot this reconpic will take in the virtual DPB will leave an unreferenced
            // physical allocation, that may need to be evicted (if no other virtual dpb slot references it)
            //

            if (m_CurrentFrameReferencesData.pVirtualDPBEntries[dpbSlotIdx].ReconstructedPictureResourceIndex !=
                UNUSED_VIRTUAL_DPB_SLOT_PHYSICAL_INDEX) {

               // If this is a virtual dpb valid entry, there has to be a valid underlying physical allocation
               assert(m_CurrentFrameReferencesData.pVirtualDPBEntries[dpbSlotIdx].ReconstructedPictureResourceIndex <
                      m_PhysicalAllocationsStorage.get_number_of_pics_in_dpb());

               // Get the number of entries in the virtual DPB descriptors that point to
               // ReconstructedPictureResourceIndex of the current virtual dpb slot (counting the current dpbSlotIdx we
               // didn't clear yet)
               uint32_t numRefs = get_dpb_physical_slot_refcount_from_virtual_dpb(
                  m_CurrentFrameReferencesData.pVirtualDPBEntries[dpbSlotIdx].ReconstructedPictureResourceIndex);
               if (numRefs == 1) {
                  // When refreshing this dpbSlotIdx, we will leave an unreferenced physical allocation
                  // so we can just remove it (and release the ID3D12Resource allocation back to the unused pool)

                  bool wasTracked = false;
                  m_PhysicalAllocationsStorage.remove_reference_frame(
                     m_CurrentFrameReferencesData.pVirtualDPBEntries[dpbSlotIdx].ReconstructedPictureResourceIndex,
                     &wasTracked);
                  assert(wasTracked);

                  // Indices in the virtual dpb higher or equal than
                  // m_CurrentFrameReferencesData.pVirtualDPBEntries[dpbSlotIdx].ReconstructedPictureResourceIndex
                  // must be shifted back one place as we erased the entry in the physical allocations array (ppTexture2Ds)
                  for (auto &dpbVirtualEntry : m_CurrentFrameReferencesData.pVirtualDPBEntries) {
                     if (
                        // Check for slot being used
                        (dpbVirtualEntry.ReconstructedPictureResourceIndex != UNUSED_VIRTUAL_DPB_SLOT_PHYSICAL_INDEX) &&
                        // Check for slot to be affected by removing the entry in ppTexture2Ds above
                        (dpbVirtualEntry.ReconstructedPictureResourceIndex >
                         m_CurrentFrameReferencesData.pVirtualDPBEntries[dpbSlotIdx].ReconstructedPictureResourceIndex)) {

                        // Decrease the index to compensate for the removed ppTexture2Ds entry
                        dpbVirtualEntry.ReconstructedPictureResourceIndex--;
                     }
                  }
               }

               // Clear this virtual dpb entry (so next iterations will decrease refcount)
               m_CurrentFrameReferencesData.pVirtualDPBEntries[dpbSlotIdx].ReconstructedPictureResourceIndex =
                  UNUSED_VIRTUAL_DPB_SLOT_PHYSICAL_INDEX;
            }
         }
      }

      //
      // Find a new free physical index for the current recon pic; always put new physical entry at the end to avoid
      // having to shift existing indices in virtual DPB
      //
      UINT allocationSlotIdx = m_PhysicalAllocationsStorage.get_number_of_pics_in_dpb();
      assert(static_cast<uint32_t>(allocationSlotIdx) < NUM_REF_FRAMES);
      D3D12_VIDEO_ENCODER_RECONSTRUCTED_PICTURE recAlloc = get_current_frame_recon_pic_output_allocation();
      d3d12_video_reconstructed_picture refFrameDesc = {};
      refFrameDesc.pReconstructedPicture = recAlloc.pReconstructedPicture;
      refFrameDesc.ReconstructedPictureSubresource = recAlloc.ReconstructedPictureSubresource;
      m_PhysicalAllocationsStorage.insert_reference_frame(refFrameDesc, allocationSlotIdx);

      //
      // Update virtual DPB entries with the current frame recon picture
      //
      for (unsigned dpbSlotIdx = 0; dpbSlotIdx < NUM_REF_FRAMES; dpbSlotIdx++) {
         if (((refresh_frame_flags >> dpbSlotIdx) & 0x1) != 0) {
            m_CurrentFrameReferencesData.pVirtualDPBEntries[dpbSlotIdx] = { allocationSlotIdx,
                                                                            0,   // NO temporal scalability support
                                                                            0,   // NO spatial scalability support
                                                                            m_CurrentFramePicParams.FrameType,
                                                                            {},   // No global_motion support
                                                                            m_CurrentFramePicParams.OrderHint,
                                                                            m_CurrentFramePicParams.PictureIndex };
         }
      }
   }

   // Number of allocations, disregarding if they are used or not, should not exceed this limit due to reuse policies
   // on DPB items removal.
   assert(m_PhysicalAllocationsStorage.get_number_of_tracked_allocations() <= (NUM_REF_FRAMES + 1));
}
