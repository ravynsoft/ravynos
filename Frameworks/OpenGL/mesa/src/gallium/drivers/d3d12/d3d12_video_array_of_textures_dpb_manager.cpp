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

#include "d3d12_video_array_of_textures_dpb_manager.h"
#include <algorithm>
///
/// d3d12_array_of_textures_dpb_manager
///
// Differences with d3d12_texture_array_dpb_manager
// Uses an std::vector with individual D3D resources as backing storage instead of an D3D12 Texture Array
// Supports dynamic pool capacity extension (by pushing back a new D3D12Resource) of the pool

#include "d3d12_common.h"

#include "d3d12_util.h"

void
d3d12_array_of_textures_dpb_manager::create_reconstructed_picture_allocations(ID3D12Resource **ppResource)
{
   D3D12_HEAP_PROPERTIES Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT, m_nodeMask, m_nodeMask);

   CD3DX12_RESOURCE_DESC reconstructedPictureResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(m_encodeFormat,
                                                                                         m_encodeResolution.Width,
                                                                                         m_encodeResolution.Height,
                                                                                         1,
                                                                                         1,
                                                                                         1,
                                                                                         0,
                                                                                         m_resourceAllocFlags);
   HRESULT hr = m_pDevice->CreateCommittedResource(&Properties,
                                                       D3D12_HEAP_FLAG_NONE,
                                                       &reconstructedPictureResourceDesc,
                                                       D3D12_RESOURCE_STATE_COMMON,
                                                       nullptr,
                                                       IID_PPV_ARGS(ppResource));
   if (FAILED(hr)) {
      debug_printf("CreateCommittedResource failed with HR %x\n", hr);
      assert(false);
   }
}

d3d12_array_of_textures_dpb_manager::d3d12_array_of_textures_dpb_manager(
   uint32_t                                    dpbInitialSize,
   ID3D12Device *                              pDevice,
   DXGI_FORMAT                                 encodeSessionFormat,
   D3D12_VIDEO_ENCODER_PICTURE_RESOLUTION_DESC encodeSessionResolution,
   D3D12_RESOURCE_FLAGS                        resourceAllocFlags,
   bool                                        setNullSubresourcesOnAllZero,
   uint32_t                                    nodeMask,
   bool                                        allocatePool)
   : m_dpbInitialSize(dpbInitialSize),
     m_pDevice(pDevice),
     m_encodeFormat(encodeSessionFormat),
     m_encodeResolution(encodeSessionResolution),
     m_resourceAllocFlags(resourceAllocFlags),
     m_NullSubresourcesOnAllZero(setNullSubresourcesOnAllZero),
     m_nodeMask(nodeMask)
{
   // Initialize D3D12 DPB exposed in this class implemented CRUD interface for a DPB
   clear_decode_picture_buffer();

   // Sometimes the client of this class can reuse allocations from an upper layer
   // and doesn't need to get fresh/tracked allocations
   if(allocatePool)
   {
      // Implement a reusable pool of D3D12 Resources as an array of textures
      m_ResourcesPool.resize(m_dpbInitialSize);

      // Build resource pool with commitedresources with a d3ddevice and the encoding session settings (eg. resolution) and
      // the reference_only flag
      for (auto &reusableRes : m_ResourcesPool) {
         reusableRes.isFree = true;
         create_reconstructed_picture_allocations(reusableRes.pResource.GetAddressOf());
      }
   }
}

uint32_t
d3d12_array_of_textures_dpb_manager::clear_decode_picture_buffer()
{
   uint32_t untrackCount = 0;
   // Mark resources used in DPB as re-usable in the resources pool
   for (auto &dpbResource : m_D3D12DPB.pResources) {
      // Don't assert the untracking result here in case the DPB contains resources not adquired using the pool methods
      // in this interface
      untrackCount += untrack_reconstructed_picture_allocation({ dpbResource, 0 }) ? 1 : 0;
   }

   // Clear DPB
   m_D3D12DPB.pResources.clear();
   m_D3D12DPB.pSubresources.clear();
   m_D3D12DPB.pHeaps.clear();
   m_D3D12DPB.pResources.reserve(m_dpbInitialSize);
   m_D3D12DPB.pSubresources.reserve(m_dpbInitialSize);
   m_D3D12DPB.pHeaps.reserve(m_dpbInitialSize);

   return untrackCount;
}

// Assigns a reference frame at a given position
void
d3d12_array_of_textures_dpb_manager::assign_reference_frame(d3d12_video_reconstructed_picture pReconPicture,
                                                            uint32_t                          dpbPosition)
{
   assert(m_D3D12DPB.pResources.size() == m_D3D12DPB.pSubresources.size());
   assert(m_D3D12DPB.pResources.size() == m_D3D12DPB.pHeaps.size());

   assert (dpbPosition < m_D3D12DPB.pResources.size());

   m_D3D12DPB.pResources[dpbPosition]    = pReconPicture.pReconstructedPicture;
   m_D3D12DPB.pSubresources[dpbPosition] = pReconPicture.ReconstructedPictureSubresource;
   m_D3D12DPB.pHeaps[dpbPosition]        = pReconPicture.pVideoHeap;
}

// Adds a new reference frame at a given position
void
d3d12_array_of_textures_dpb_manager::insert_reference_frame(d3d12_video_reconstructed_picture pReconPicture,
                                                            uint32_t                          dpbPosition)
{
   assert(m_D3D12DPB.pResources.size() == m_D3D12DPB.pSubresources.size());
   assert(m_D3D12DPB.pResources.size() == m_D3D12DPB.pHeaps.size());

   if (dpbPosition > m_D3D12DPB.pResources.size()) {
      // extend capacity
      m_D3D12DPB.pResources.resize(dpbPosition);
      m_D3D12DPB.pSubresources.resize(dpbPosition);
      m_D3D12DPB.pHeaps.resize(dpbPosition);
   }

   m_D3D12DPB.pResources.insert(m_D3D12DPB.pResources.begin() + dpbPosition, pReconPicture.pReconstructedPicture);
   m_D3D12DPB.pSubresources.insert(m_D3D12DPB.pSubresources.begin() + dpbPosition,
                                   pReconPicture.ReconstructedPictureSubresource);
   m_D3D12DPB.pHeaps.insert(m_D3D12DPB.pHeaps.begin() + dpbPosition, pReconPicture.pVideoHeap);
}

// Gets a reference frame at a given position
d3d12_video_reconstructed_picture
d3d12_array_of_textures_dpb_manager::get_reference_frame(uint32_t dpbPosition)
{
   assert(dpbPosition < m_D3D12DPB.pResources.size());

   d3d12_video_reconstructed_picture retVal = { m_D3D12DPB.pResources[dpbPosition],
                                                m_D3D12DPB.pSubresources[dpbPosition],
                                                m_D3D12DPB.pHeaps[dpbPosition] };

   return retVal;
}

// Removes a new reference frame at a given position and returns operation success
bool
d3d12_array_of_textures_dpb_manager::remove_reference_frame(uint32_t dpbPosition, bool *pResourceUntracked)
{
   assert(m_D3D12DPB.pResources.size() == m_D3D12DPB.pSubresources.size());
   assert(m_D3D12DPB.pResources.size() == m_D3D12DPB.pHeaps.size());

   assert(dpbPosition < m_D3D12DPB.pResources.size());

   // If removed resource came from resource pool, mark it as free
   // to free it for a new usage
   // Don't assert the untracking result here in case the DPB contains resources not adquired using the pool methods in
   // this interface
   bool resUntracked = untrack_reconstructed_picture_allocation({ m_D3D12DPB.pResources[dpbPosition], 0 });

   if (pResourceUntracked != nullptr) {
      *pResourceUntracked = resUntracked;
   }

   // Remove from DPB tables
   m_D3D12DPB.pResources.erase(m_D3D12DPB.pResources.begin() + dpbPosition);
   m_D3D12DPB.pSubresources.erase(m_D3D12DPB.pSubresources.begin() + dpbPosition);
   m_D3D12DPB.pHeaps.erase(m_D3D12DPB.pHeaps.begin() + dpbPosition);

   return true;
}

// Returns true if the trackedItem was allocated (and is being tracked) by this class
bool
d3d12_array_of_textures_dpb_manager::is_tracked_allocation(d3d12_video_reconstructed_picture trackedItem)
{
   for (auto &reusableRes : m_ResourcesPool) {
      if (trackedItem.pReconstructedPicture == reusableRes.pResource.Get() && !reusableRes.isFree) {
         return true;
      }
   }
   return false;
}

// Returns whether it found the tracked resource on this instance pool tracking and was able to free it
bool
d3d12_array_of_textures_dpb_manager::untrack_reconstructed_picture_allocation(
   d3d12_video_reconstructed_picture trackedItem)
{
   for (auto &reusableRes : m_ResourcesPool) {
      if (trackedItem.pReconstructedPicture == reusableRes.pResource.Get()) {
         reusableRes.isFree = true;
         return true;
      }
   }
   return false;
}

// Returns a fresh resource for a new reconstructed picture to be written to
// this class implements the dpb allocations as an array of textures
d3d12_video_reconstructed_picture
d3d12_array_of_textures_dpb_manager::get_new_tracked_picture_allocation()
{
   d3d12_video_reconstructed_picture freshAllocation = { // pResource
                                                         nullptr,
                                                         // subresource
                                                         0
   };

   // Find first (if any) available resource to (re-)use
   bool bAvailableResourceInPool = false;
   for (auto &reusableRes : m_ResourcesPool) {
      if (reusableRes.isFree) {
         bAvailableResourceInPool              = true;
         freshAllocation.pReconstructedPicture = reusableRes.pResource.Get();
         reusableRes.isFree                    = false;
         break;
      }
   }

   if (!bAvailableResourceInPool) {
      // Expand resources pool by one
      assert(m_ResourcesPool.size() < UINT32_MAX);
      debug_printf(
         "[d3d12_array_of_textures_dpb_manager] ID3D12Resource Pool capacity (%" PRIu32 ") exceeded - extending capacity "
         "and appending new allocation at the end",
         static_cast<uint32_t>(m_ResourcesPool.size()));
      d3d12_reusable_resource newPoolEntry = {};
      newPoolEntry.isFree                  = false;
      create_reconstructed_picture_allocations(newPoolEntry.pResource.GetAddressOf());
      m_ResourcesPool.push_back(newPoolEntry);

      // Assign it to current ask
      freshAllocation.pReconstructedPicture = newPoolEntry.pResource.Get();
   }

   return freshAllocation;
}

uint32_t
d3d12_array_of_textures_dpb_manager::get_number_of_pics_in_dpb()
{
   assert(m_D3D12DPB.pResources.size() == m_D3D12DPB.pSubresources.size());
   assert(m_D3D12DPB.pResources.size() == m_D3D12DPB.pHeaps.size());

   assert(m_D3D12DPB.pResources.size() < UINT32_MAX);
   return static_cast<uint32_t>(m_D3D12DPB.pResources.size());
}

d3d12_video_reference_frames
d3d12_array_of_textures_dpb_manager::get_current_reference_frames()
{
   // If all subresources are 0, the DPB is loaded with an array of individual textures, the D3D Encode API expects
   // pSubresources to be null in this case The D3D Decode API expects it to be non-null even with all zeroes.
   uint32_t *pSubresources = m_D3D12DPB.pSubresources.data();
   if ((std::all_of(m_D3D12DPB.pSubresources.cbegin(), m_D3D12DPB.pSubresources.cend(), [](int i) { return i == 0; })) &&
       m_NullSubresourcesOnAllZero) {
      pSubresources = nullptr;
   }

   d3d12_video_reference_frames retVal = { get_number_of_pics_in_dpb(),
                                           m_D3D12DPB.pResources.data(),
                                           pSubresources,
                                           m_D3D12DPB.pHeaps.data() };

   return retVal;
}

// number of resources in the pool that are marked as in use
uint32_t
d3d12_array_of_textures_dpb_manager::get_number_of_in_use_allocations()
{
   uint32_t countOfInUseResourcesInPool = 0;
   for (auto &reusableRes : m_ResourcesPool) {
      if (!reusableRes.isFree) {
         countOfInUseResourcesInPool++;
      }
   }
   return countOfInUseResourcesInPool;
}

// Returns the number of pictures currently stored in the DPB
uint32_t
d3d12_array_of_textures_dpb_manager::get_number_of_tracked_allocations()
{
   assert(m_ResourcesPool.size() < UINT32_MAX);
   return static_cast<uint32_t>(m_ResourcesPool.size());
}
