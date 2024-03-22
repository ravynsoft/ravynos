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

/* The purpose of this file is to abstract the differences between the Windows
 * C ABI for D3D12 and the Linux ABI. Essentially, for class methods that return
 * structures, the MSVC C++ ABI specifies that they are always called with the return
 * structure allocated by the caller, and passed as a hidden second parameter,
 * after "this". But the C compiler doesn't apply that automatically to the C
 * equivalent definition of the method, and so that ABI needs to be explicitly
 * embedded in the C function signature. For Linux, no such ABI difference between
 * C and C++ exists, and so C callers should use the same signature as C++.
 */

#ifndef DZN_ABI_HELPER_H
#define DZN_ABI_HELPER_H

static inline D3D12_HEAP_PROPERTIES
dzn_ID3D12Device4_GetCustomHeapProperties(ID3D12Device4 *dev, UINT node_mask, D3D12_HEAP_TYPE type)
{
    D3D12_HEAP_PROPERTIES ret;
#ifdef _WIN32
    ID3D12Device4_GetCustomHeapProperties(dev, &ret, node_mask, type);
#else
    ret = ID3D12Device4_GetCustomHeapProperties(dev, node_mask, type);
#endif
    return ret;
}

static inline D3D12_RESOURCE_ALLOCATION_INFO
dzn_ID3D12Device4_GetResourceAllocationInfo(ID3D12Device4 *dev, UINT visible_mask, UINT num_resource_descs, const D3D12_RESOURCE_DESC *resource_descs)
{
    D3D12_RESOURCE_ALLOCATION_INFO ret;
#ifdef _WIN32
    ID3D12Device4_GetResourceAllocationInfo(dev, &ret, visible_mask, num_resource_descs, resource_descs);
#else
    ret = ID3D12Device4_GetResourceAllocationInfo(dev, visible_mask, num_resource_descs, resource_descs);
#endif
    return ret;
}

static inline D3D12_RESOURCE_ALLOCATION_INFO
dzn_ID3D12Device12_GetResourceAllocationInfo3(ID3D12Device12 *dev, UINT visible_mask, UINT num_resource_descs, const D3D12_RESOURCE_DESC1 *resource_descs,
                                              const UINT *num_castable_formats, const DXGI_FORMAT *const *castable_formats,
                                              D3D12_RESOURCE_ALLOCATION_INFO1 *allocation_info1)
{
   D3D12_RESOURCE_ALLOCATION_INFO ret;
#ifdef _WIN32
   ID3D12Device12_GetResourceAllocationInfo3(dev, &ret, visible_mask, num_resource_descs, resource_descs, num_castable_formats, castable_formats, allocation_info1);
#else
   ret = ID3D12Device12_GetResourceAllocationInfo3(dev, visible_mask, num_resource_descs, resource_descs, num_castable_formats, castable_formats, allocation_info1);
#endif
   return ret;
}

static inline D3D12_RESOURCE_DESC
dzn_ID3D12Resource_GetDesc(ID3D12Resource *res)
{
    D3D12_RESOURCE_DESC ret;
#ifdef _WIN32
    ID3D12Resource_GetDesc(res, &ret);
#else
    ret = ID3D12Resource_GetDesc(res);
#endif
    return ret;
}

static inline D3D12_HEAP_DESC
dzn_ID3D12Heap_GetDesc(ID3D12Heap *heap)
{
   D3D12_HEAP_DESC ret;
#ifdef _WIN32
    ID3D12Heap_GetDesc(heap, &ret);
#else
    ret = ID3D12Heap_GetDesc(heap);
#endif
    return ret;
}

static inline D3D12_CPU_DESCRIPTOR_HANDLE
dzn_ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart(ID3D12DescriptorHeap *heap)
{
    D3D12_CPU_DESCRIPTOR_HANDLE ret;
#ifdef _WIN32
    ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart(heap, &ret);
#else
    ret = ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart(heap);
#endif
    return ret;
}

static inline D3D12_GPU_DESCRIPTOR_HANDLE
dzn_ID3D12DescriptorHeap_GetGPUDescriptorHandleForHeapStart(ID3D12DescriptorHeap *heap)
{
    D3D12_GPU_DESCRIPTOR_HANDLE ret;
#ifdef _WIN32
    ID3D12DescriptorHeap_GetGPUDescriptorHandleForHeapStart(heap, &ret);
#else
    ret = ID3D12DescriptorHeap_GetGPUDescriptorHandleForHeapStart(heap);
#endif
    return ret;
}

#endif /*DZN_ABI_HELPER_H*/
