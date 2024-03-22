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

#ifndef D3D12_DEBUG_H
#define D3D12_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define D3D12_DEBUG_VERBOSE       (1 << 0)
#define D3D12_DEBUG_EXPERIMENTAL  (1 << 1)
#define D3D12_DEBUG_DXIL          (1 << 2)
#define D3D12_DEBUG_DISASS        (1 << 3)
#define D3D12_DEBUG_BLIT          (1 << 4)
#define D3D12_DEBUG_RESOURCE      (1 << 5)
#define D3D12_DEBUG_DEBUG_LAYER   (1 << 6)
#define D3D12_DEBUG_GPU_VALIDATOR (1 << 7)
#define D3D12_DEBUG_SINGLETON     (1 << 8)

extern uint32_t d3d12_debug;

#ifdef __cplusplus
}
#endif

#endif
