/* Copyright 2022 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: AMD
 *
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define VPELIB_API_VERSION_MAJOR 0
#define VPELIB_API_VERSION_MINOR 3

#define VPELIB_API_VERSION_MAJOR_SHIFT 16
#define VPELIB_API_VERSION_MINOR_SHIFT 0
#define VPELIB_API_VERSION_MAJOR_MASK  0xFFFF0000
#define VPELIB_API_VERSION_MINOR_MASK  0x0000FFFF

#define VPELIB_GET_API_MAJOR(version)                                                              \
    ((version & VPELIB_API_VERSION_MAJOR_MASK) >> VPELIB_API_VERSION_MAJOR_SHIFT)

#define VPELIB_GET_API_MINOR(version)                                                              \
    ((version & VPELIB_API_VERSION_MINOR_MASK) >> VPELIB_API_VERSION_MINOR_SHIFT)


#define VPE_VERSION(mj, mn, rv)     (((mj) << 16) | ((mn) << 8) | (rv))
#define VPE_VERSION_MAJ(ver)        ((ver) >> 16)
#define VPE_VERSION_MIN(ver)        (((ver) >> 8) & 0xFF)
#define VPE_VERSION_REV(ver)        ((ver) & 0xFF)
#define VPE_VERSION_6_1_0(ver)      ((ver) == VPE_VERSION(6, 1, 0))
#define VPE_VERSION_6_1_1(ver)      ((ver) == VPE_VERSION(6, 1, 1))

#ifdef __cplusplus
}
#endif
