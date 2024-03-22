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


#ifndef D3D12_VIDEO_DPB_STORAGE_MANAGER_INTERFACE_H
#define D3D12_VIDEO_DPB_STORAGE_MANAGER_INTERFACE_H

#include "d3d12_video_types.h"

struct d3d12_video_reconstructed_picture
{
   ID3D12Resource *pReconstructedPicture;
   uint32_t        ReconstructedPictureSubresource;
   IUnknown *      pVideoHeap;
};

struct d3d12_video_reference_frames
{
   uint32_t         NumTexture2Ds;
   ID3D12Resource **ppTexture2Ds;
   uint32_t *       pSubresources;
   IUnknown **      ppHeaps;
};

// Defines interface for storing and retrieving the decoded picture buffer ID3D12Resources with
// the reconstructed pictures
// Implementors of this interface can decide how to do this, let Class1 and Class2 be implementors...
// for example Class1 can use a texture array and Class2 or an array of textures
class d3d12_video_dpb_storage_manager_interface
{
   // d3d12_video_dpb_storage_manager_interface
 public:
   // Adds a new reference frame at a given position
   virtual void insert_reference_frame(d3d12_video_reconstructed_picture pReconPicture, uint32_t dpbPosition) = 0;

   // Gets a reference frame at a given position
   virtual d3d12_video_reconstructed_picture get_reference_frame(uint32_t dpbPosition) = 0;

   // Assigns a reference frame at a given position
   virtual void assign_reference_frame(d3d12_video_reconstructed_picture pReconPicture, uint32_t dpbPosition) = 0;

   // Removes a new reference frame at a given position and returns operation success
   // pResourceUntracked is an optional output indicating if the removed resource was being tracked by the pool
   virtual bool remove_reference_frame(uint32_t dpbPosition, bool *pResourceUntracked) = 0;

   // Returns the resource allocation for a NEW reconstructed picture
   virtual d3d12_video_reconstructed_picture get_new_tracked_picture_allocation() = 0;

   // Returns whether it found the tracked resource on this instance pool tracking and was able to free it
   virtual bool untrack_reconstructed_picture_allocation(d3d12_video_reconstructed_picture trackedItem) = 0;

   // Returns true if the trackedItem was allocated (and is being tracked) by this class
   virtual bool is_tracked_allocation(d3d12_video_reconstructed_picture trackedItem) = 0;

   // resource pool size
   virtual uint32_t get_number_of_tracked_allocations() = 0;

   // number of resources in the pool that are marked as in use
   virtual uint32_t get_number_of_in_use_allocations() = 0;

   // Returns the number of pictures currently stored in the DPB
   virtual uint32_t get_number_of_pics_in_dpb() = 0;

   // Returns all the current reference frames stored in the storage manager
   virtual d3d12_video_reference_frames get_current_reference_frames() = 0;

   // Remove all pictures from DPB
   // returns the number of resources marked as reusable
   virtual uint32_t clear_decode_picture_buffer() = 0;

   virtual ~d3d12_video_dpb_storage_manager_interface()
   { }
};

#endif
