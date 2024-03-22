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


#ifndef D3D12_VIDEO_TEXTURE_ARRAY_DPB_MANAGER_H
#define D3D12_VIDEO_TEXTURE_ARRAY_DPB_MANAGER_H

#include "d3d12_video_dpb_storage_manager.h"
#include "d3d12_video_types.h"

class d3d12_texture_array_dpb_manager : public d3d12_video_dpb_storage_manager_interface
{
   // d3d12_video_dpb_storage_manager_interface
 public:
   // Adds a new reference frame at a given position
   void insert_reference_frame(d3d12_video_reconstructed_picture pReconPicture, uint32_t dpbPosition);

   // Assigns a reference frame at a given position
   void assign_reference_frame(d3d12_video_reconstructed_picture pReconPicture, uint32_t dpbPosition);

   // Gets a reference frame at a given position
   d3d12_video_reconstructed_picture get_reference_frame(uint32_t dpbPosition);

   // Removes a new reference frame at a given position and returns operation success
   // pResourceUntracked is an optional output indicating if the removed resource was being tracked by the pool
   bool remove_reference_frame(uint32_t dpbPosition, bool *pResourceUntracked = nullptr);

   // Returns the resource allocation for a NEW picture
   d3d12_video_reconstructed_picture get_new_tracked_picture_allocation();

   // Returns true if the trackedItem was allocated (and is being tracked) by this class
   bool is_tracked_allocation(d3d12_video_reconstructed_picture trackedItem);

   // Returns whether it found the tracked resource on this instance pool tracking and was able to free it
   bool untrack_reconstructed_picture_allocation(d3d12_video_reconstructed_picture trackedItem);

   // Returns the number of pictures currently stored in the DPB
   uint32_t get_number_of_pics_in_dpb();

   // Returns all the current reference frames stored
   d3d12_video_reference_frames get_current_reference_frames();

   // Removes all pictures from DPB
   // returns the number of resources marked as reusable
   uint32_t clear_decode_picture_buffer();

   // number of resources in the pool that are marked as in use
   uint32_t get_number_of_in_use_allocations();

   uint32_t get_number_of_tracked_allocations();

   // d3d12_texture_array_dpb_manager
 public:
   d3d12_texture_array_dpb_manager(
      uint16_t dpbInitialSize,   // Maximum in use resources for a DPB of size x should be x+1 for cases when a P frame
                                 // is using the x references in the L0 list and also using an extra resource to output
                                 // it's own recon pic.
      ID3D12Device *                              pDevice,
      DXGI_FORMAT                                 encodeSessionFormat,
      D3D12_VIDEO_ENCODER_PICTURE_RESOLUTION_DESC encodeSessionResolution,
      D3D12_RESOURCE_FLAGS                        resourceAllocFlags = D3D12_RESOURCE_FLAG_NONE,
      uint32_t                                    nodeMask           = 0);
   ~d3d12_texture_array_dpb_manager();

   // d3d12_texture_array_dpb_manager
 private:
   void create_reconstructed_picture_allocations(ID3D12Resource **ppResource, uint16_t texArraySize);

   ID3D12Device *                              m_pDevice;
   DXGI_FORMAT                                 m_encodeFormat;
   D3D12_VIDEO_ENCODER_PICTURE_RESOLUTION_DESC m_encodeResolution;
   uint16_t                                    m_dpbTextureArraySize = 0;

   // DPB with array of resources backing storage

   struct d3d12_video_dpb
   {
      std::vector<ID3D12Resource *> pResources;
      std::vector<uint32_t>         pSubresources;
      std::vector<IUnknown *>       pHeaps;
   };

   d3d12_video_dpb m_D3D12DPB;

   // Flags used when creating the resource pool
   // Usually if reference only is needed for d3d12 video use
   // D3D12_RESOURCE_FLAG_VIDEO_DECODE_REFERENCE_ONLY | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE
   // D3D12_RESOURCE_FLAG_VIDEO_ENCODE_REFERENCE_ONLY | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE
   D3D12_RESOURCE_FLAGS m_resourceAllocFlags;

   // Pool of resources to be aliased by the DPB without giving memory ownership
   // This resources are allocated and released by this implementation
   struct d3d12_reusable_resource
   {
      ComPtr<ID3D12Resource> pResource;
      uint32_t               subresource;
      bool                   isFree;
   };

   ComPtr<ID3D12Resource>               m_baseTexArrayResource;
   std::vector<d3d12_reusable_resource> m_ResourcesPool;

   uint32_t m_nodeMask = 0u;
};

#endif
