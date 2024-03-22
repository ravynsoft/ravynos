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

#ifndef D3D12_VIDEO_DEC_REFMGR_H
#define D3D12_VIDEO_DEC_REFMGR_H

#include "d3d12_video_types.h"
#include "d3d12_video_dpb_storage_manager.h"
#include "d3d12_util.h"
#include <algorithm>
#include <map>

struct d3d12_video_decoder_references_manager
{
   d3d12_video_decoder_references_manager(const struct d3d12_screen *       pD3D12Screen,
                                          uint32_t                          NodeMask,
                                          d3d12_video_decode_profile_type   DecodeProfileType,
                                          d3d12_video_decode_dpb_descriptor dpbDescriptor);

   bool is_reference_only()
   {
      return m_dpbDescriptor.fReferenceOnly;
   }
   bool is_array_of_textures()
   {
      return m_dpbDescriptor.fArrayOfTexture;
   }

   bool is_pipe_buffer_underlying_output_decode_allocation()
   {
      return (is_reference_only() || is_array_of_textures());
   }

   void mark_all_references_as_unused();
   void release_unused_references_texture_memory();

   template <typename T, size_t size>
   void mark_references_in_use(const T (&picEntries)[size]);
   template <typename T, size_t size>
   void mark_references_in_use_av1(const T (&picEntries)[size]);
   void mark_reference_in_use(uint16_t index);

   uint16_t store_future_reference(uint16_t index,
                                   _In_ ComPtr<ID3D12VideoDecoderHeap> &decoderHeap,
                                   ID3D12Resource *                     pTexture2D,
                                   uint32_t                             subresourceIndex);

   // Will clear() argument outNeededTransitions and fill it with the necessary transitions to perform by the caller
   // after the method returns
   template <typename T, size_t size>
   void update_entries(T (&picEntries)[size], std::vector<D3D12_RESOURCE_BARRIER> &outNeededTransitions);

   template <typename T, size_t size>
   void update_entries_av1(T (&picEntries)[size], std::vector<D3D12_RESOURCE_BARRIER> &outNeededTransitions);

   void get_reference_only_output(
      struct pipe_video_buffer *  pCurrentDecodeTarget,
      ID3D12Resource **ppOutputReference,     // out -> new reference slot assigned or nullptr
      uint32_t *       pOutputSubresource,    // out -> new reference slot assigned or nullptr
      bool &outNeedsTransitionToDecodeWrite   // out -> indicates if output resource argument has to be transitioned to
                                              // D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE by the caller
   );

   // Gets the output texture for the current frame to be decoded
   void get_current_frame_decode_output_texture(struct pipe_video_buffer *pCurrentDecodeTarget, ID3D12Resource **ppOutTexture2D, uint32_t *pOutSubresourceIndex);

   D3D12_VIDEO_DECODE_REFERENCE_FRAMES get_current_reference_frames();

   void print_dpb();

   uint8_t get_unused_index7bits()
   {
      for (uint32_t testIdx = 0; testIdx < 127; testIdx++) {
         auto it = std::find_if(m_DecodeTargetToOriginalIndex7Bits.begin(), m_DecodeTargetToOriginalIndex7Bits.end(),
            [&testIdx](const std::pair< struct pipe_video_buffer*, uint8_t > &p) {
               return p.second == testIdx;
            });

         if (it == m_DecodeTargetToOriginalIndex7Bits.end())
            return testIdx;
      }
      debug_printf(
         "[d3d12_video_decoder_references_manager] d3d12_video_decoder_references_manager - Decode - No available "
         "fresh indices left.\n");
      assert(false);
      return 0;
   }

   ///
   /// Get the Index7Bits associated with this decode target
   /// If there isn't one assigned yet, gives out a fresh/unused Index7Bits
   ///
   uint8_t get_index7bits(struct pipe_video_buffer * pDecodeTarget) {
      if(m_DecodeTargetToOriginalIndex7Bits.count(pDecodeTarget) == 0)
         m_DecodeTargetToOriginalIndex7Bits[pDecodeTarget] = get_unused_index7bits();
      return m_DecodeTargetToOriginalIndex7Bits[pDecodeTarget];
   }

 private:
   uint16_t update_entry(
      uint16_t         index,                // in
      ID3D12Resource *&pOutputReference,     // out -> new reference slot assigned or nullptr
      uint32_t &       OutputSubresource,    // out -> new reference slot assigned or 0
      bool &outNeedsTransitionToDecodeRead   // out -> indicates if output resource argument has to be transitioned to
                                             // D3D12_RESOURCE_STATE_VIDEO_DECODE_READ by the caller
   );

   uint16_t find_remapped_index(uint16_t originalIndex);

   struct ReferenceData
   {
      uint16_t originalIndex;
      bool     fUsed;
   };

   // Holds the DPB textures
   std::unique_ptr<d3d12_video_dpb_storage_manager_interface> m_upD3D12TexturesStorageManager;
   std::vector<ID3D12VideoDecoderHeap *>
      m_ppHeaps;   // Auxiliary allocation to QueryInterface the IUnknown's
                   // m_upD3D12TexturesStorageManager->get_current_reference_frames().ppHeaps
                   // containing the generic video encode/decode heap;

   // Holds the mapping between DXVA PicParams indices and the D3D12 indices
   std::vector<ReferenceData> m_referenceDXVAIndices;
   
   std::map<struct pipe_video_buffer *, uint8_t> m_DecodeTargetToOriginalIndex7Bits = { };
 
   const struct d3d12_screen *       m_pD3D12Screen;
   uint16_t                          m_invalidIndex;
   d3d12_video_decode_dpb_descriptor m_dpbDescriptor      = {};
   uint16_t                          m_currentOutputIndex = 0;
   uint16_t                          m_currentSubresourceIndex = 0;
   ID3D12Resource*                   m_currentResource = nullptr;
   D3D12_FEATURE_DATA_FORMAT_INFO    m_formatInfo         = { m_dpbDescriptor.Format };
};


//----------------------------------------------------------------------------------------------------------------------------------
template <typename T, size_t size>
void
d3d12_video_decoder_references_manager::update_entries(T (&picEntries)[size],
                                                       std::vector<D3D12_RESOURCE_BARRIER> &outNeededTransitions)
{
   outNeededTransitions.clear();

   for (auto &picEntry : picEntries) {
      // uint16_t update_entry(
      //     uint16_t index, // in
      //     ID3D12Resource*& pOutputReference, // out -> new reference slot assigned or nullptr
      //     uint32_t& OutputSubresource, // out -> new reference slot assigned or 0
      //     bool& outNeedsTransitionToDecodeRead // out -> indicates if output resource argument has to be transitioned
      //     to D3D12_RESOURCE_STATE_VIDEO_DECODE_READ by the caller
      // );

      ID3D12Resource *pOutputReference               = {};
      uint32_t        OutputSubresource              = 0u;
      bool            outNeedsTransitionToDecodeRead = false;

      picEntry.Index7Bits =
         update_entry(picEntry.Index7Bits, pOutputReference, OutputSubresource, outNeedsTransitionToDecodeRead);

      if (outNeedsTransitionToDecodeRead) {
         ///
         /// The subresource indexing in D3D12 Video within the DPB doesn't take into account the Y, UV planes (ie.
         /// subresource 0, 1, 2, 3..., N are different full NV12 references in the DPB) but when using the subresources
         /// in other areas of D3D12 we need to convert it to the D3D12CalcSubresource format, explained in
         /// https://docs.microsoft.com/en-us/windows/win32/direct3d12/subresources
         ///
         CD3DX12_RESOURCE_DESC refDesc(GetDesc(pOutputReference));
         uint32_t              MipLevel, PlaneSlice, ArraySlice;
         D3D12DecomposeSubresource(OutputSubresource,
                                   refDesc.MipLevels,
                                   refDesc.ArraySize(),
                                   MipLevel,
                                   ArraySlice,
                                   PlaneSlice);

         for (PlaneSlice = 0; PlaneSlice < m_formatInfo.PlaneCount; PlaneSlice++) {
            uint planeOutputSubresource = refDesc.CalcSubresource(MipLevel, ArraySlice, PlaneSlice);
            outNeededTransitions.push_back(CD3DX12_RESOURCE_BARRIER::Transition(pOutputReference,
                                                                                D3D12_RESOURCE_STATE_COMMON,
                                                                                D3D12_RESOURCE_STATE_VIDEO_DECODE_READ,
                                                                                planeOutputSubresource));
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------------------
template <typename T, size_t size>
void
d3d12_video_decoder_references_manager::mark_references_in_use(const T (&picEntries)[size])
{
   for (auto &picEntry : picEntries) {
      mark_reference_in_use(picEntry.Index7Bits);
   }
}

//----------------------------------------------------------------------------------------------------------------------------------
template <typename T, size_t size>
void
d3d12_video_decoder_references_manager::update_entries_av1(T (&picEntries)[size],
                                                       std::vector<D3D12_RESOURCE_BARRIER> &outNeededTransitions)
{
   outNeededTransitions.clear();

   for (auto &picEntry : picEntries) {
      // uint16_t update_entry(
      //     uint16_t index, // in
      //     ID3D12Resource*& pOutputReference, // out -> new reference slot assigned or nullptr
      //     uint32_t& OutputSubresource, // out -> new reference slot assigned or 0
      //     bool& outNeedsTransitionToDecodeRead // out -> indicates if output resource argument has to be transitioned
      //     to D3D12_RESOURCE_STATE_VIDEO_DECODE_READ by the caller
      // );

      ID3D12Resource *pOutputReference               = {};
      uint32_t        OutputSubresource              = 0u;
      bool            outNeedsTransitionToDecodeRead = false;

      picEntry =
         update_entry(picEntry, pOutputReference, OutputSubresource, outNeedsTransitionToDecodeRead);

      if (outNeedsTransitionToDecodeRead) {
         ///
         /// The subresource indexing in D3D12 Video within the DPB doesn't take into account the Y, UV planes (ie.
         /// subresource 0, 1, 2, 3..., N are different full NV12 references in the DPB) but when using the subresources
         /// in other areas of D3D12 we need to convert it to the D3D12CalcSubresource format, explained in
         /// https://docs.microsoft.com/en-us/windows/win32/direct3d12/subresources
         ///
         CD3DX12_RESOURCE_DESC refDesc(GetDesc(pOutputReference));
         uint32_t              MipLevel, PlaneSlice, ArraySlice;
         D3D12DecomposeSubresource(OutputSubresource,
                                   refDesc.MipLevels,
                                   refDesc.ArraySize(),
                                   MipLevel,
                                   ArraySlice,
                                   PlaneSlice);

         for (PlaneSlice = 0; PlaneSlice < m_formatInfo.PlaneCount; PlaneSlice++) {
            uint planeOutputSubresource = refDesc.CalcSubresource(MipLevel, ArraySlice, PlaneSlice);
            outNeededTransitions.push_back(CD3DX12_RESOURCE_BARRIER::Transition(pOutputReference,
                                                                                D3D12_RESOURCE_STATE_COMMON,
                                                                                D3D12_RESOURCE_STATE_VIDEO_DECODE_READ,
                                                                                planeOutputSubresource));
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------------------
template <typename T, size_t size>
void
d3d12_video_decoder_references_manager::mark_references_in_use_av1(const T (&picEntries)[size])
{
   for (auto &picEntry : picEntries) {
      mark_reference_in_use(picEntry);
   }
}

#endif
