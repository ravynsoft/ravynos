/**************************************************************************
 *
 * Copyright 2009 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/**
 * @file
 * SDK/DDK compatability.
 *
 * Different headers/defines on different Windows SDKs / DDKs, so define
 * all used status here to keep the code portable.
 *
 * @author <jfonseca@vmware.com>
 */

#ifndef VMW_WDDM_COMPAT_H_
#define VMW_WDDM_COMPAT_H_

#ifndef __in
#define __in /**/
#endif

#ifndef __out
#define __out /**/
#endif

#ifndef __inout
#define __inout /**/
#endif

#ifndef __in_opt
#define __in_opt /**/
#endif

#ifndef __inout_opt
#define __inout_opt /**/
#endif

#ifndef __ecount
#define __ecount(x) /**/
#endif

#ifndef __in_ecount
#define __in_ecount(x) /**/
#endif

#ifndef __deref_ecount
#define __deref_ecount(x) /**/
#endif

#ifndef __in_bcount
#define __in_bcount(x) /**/
#endif

#ifndef __out_bcount
#define __out_bcount(x) /**/
#endif

#ifndef __out_ecount_opt
#define __out_ecount_opt(x) /**/
#endif

#ifndef __deref_out
#define __deref_out /**/
#endif

#ifndef __in_range
#define __in_range(x,y) /**/
#endif

#ifndef __field_bcount
#define __field_bcount(x) /**/
#endif

#ifndef __out_bcount
#define __out_bcount(x) /**/
#endif

#ifndef __out_bcount_full_opt
#define __out_bcount_full_opt(x) /**/
#endif

#ifndef __out_ecount_part_z_opt
#define __out_ecount_part_z_opt(x, y) /**/
#endif

#ifndef __out_ecount_part_opt
#define __out_ecount_part_opt(x, y) /**/
#endif

#ifndef __field_ecount
#define __field_ecount(x) /**/
#endif

#ifndef __field_ecount_full
#define __field_ecount_full(x) /**/
#endif

#ifndef __checkReturn
#define __checkReturn /**/
#endif

#ifndef __drv_requiresIRQL
#define __drv_requiresIRQL(x) /**/
#endif

#ifndef __drv_minIRQL
#define __drv_minIRQL(x) /**/
#endif

#ifndef __drv_maxIRQL
#define __drv_maxIRQL(x) /**/
#endif

#ifdef __MINGW32__
#define __inline static __inline__
#endif

#ifndef EXTERN_C
#define EXTERN_C /**/
#endif

#ifdef __MINGW32__
typedef unsigned char UINT8;
#endif


#ifndef NTSTATUS
#define NTSTATUS LONG
#endif

typedef LARGE_INTEGER PHYSICAL_ADDRESS;

#ifndef NT_SUCCESS
#define NT_SUCCESS(_status) ((_status) >= 0)
#endif

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS                                  ((NTSTATUS)0x00000000L)
#endif

#ifndef STATUS_UNSUCCESSFUL
#define STATUS_UNSUCCESSFUL                             ((NTSTATUS)0xC0000001L)
#endif

#ifndef STATUS_INVALID_PARAMETER
#define STATUS_INVALID_PARAMETER                        ((NTSTATUS)0xC000000DL)
#endif

#ifndef STATUS_NO_MEMORY
#define STATUS_NO_MEMORY                                ((NTSTATUS)0xC0000017L)
#endif

#ifndef STATUS_ILLEGAL_INSTRUCTION
#define STATUS_ILLEGAL_INSTRUCTION                      ((NTSTATUS)0xC000001DL)
#endif

#ifndef STATUS_BUFFER_TOO_SMALL
#define STATUS_BUFFER_TOO_SMALL                         ((NTSTATUS)0xC0000023L)
#endif

#ifndef STATUS_PRIVILEGED_INSTRUCTION
#define STATUS_PRIVILEGED_INSTRUCTION                   ((NTSTATUS)0xC0000096L)
#endif

#ifndef STATUS_NOT_SUPPORTED
#define STATUS_NOT_SUPPORTED                            ((NTSTATUS)0xC00000BBL)
#endif

#ifndef STATUS_DEVICE_REMOVED
#define STATUS_DEVICE_REMOVED                           ((NTSTATUS)0xC00002B6L)
#endif

#ifndef STATUS_INVALID_USER_BUFFER
#define STATUS_INVALID_USER_BUFFER                      ((NTSTATUS)0xC00000E8L)
#endif

#ifndef STATUS_GRAPHICS_NOT_EXCLUSIVE_MODE_OWNER
#define STATUS_GRAPHICS_NOT_EXCLUSIVE_MODE_OWNER        ((NTSTATUS)0xC01E0000L)
#endif

#ifndef STATUS_NO_VIDEO_MEMORY
#define STATUS_NO_VIDEO_MEMORY                          ((NTSTATUS)0xC01E0100L)
#endif

#ifndef STATUS_GRAPHICS_ALLOCATION_BUSY
#define STATUS_GRAPHICS_ALLOCATION_BUSY                 ((NTSTATUS)0xC01E0102L)
#endif

#ifndef STATUS_GRAPHICS_TOO_MANY_REFERENCES
#define STATUS_GRAPHICS_TOO_MANY_REFERENCES             ((NTSTATUS)0xC01E0103L)
#endif

#ifndef STATUS_GRAPHICS_ALLOCATION_INVALID
#define STATUS_GRAPHICS_ALLOCATION_INVALID              ((NTSTATUS)0xC01E0106L)
#endif

#ifndef STATUS_GRAPHICS_CANT_EVICT_PINNED_ALLOCATION
#define STATUS_GRAPHICS_CANT_EVICT_PINNED_ALLOCATION    ((NTSTATUS)0xC01E0109L)
#endif

#ifndef STATUS_GRAPHICS_CANT_RENDER_LOCKED_ALLOCATION
#define STATUS_GRAPHICS_CANT_RENDER_LOCKED_ALLOCATION   ((NTSTATUS)0xC01E0111L)
#endif

#ifndef STATUS_GRAPHICS_GPU_EXCEPTION_ON_DEVICE
#define STATUS_GRAPHICS_GPU_EXCEPTION_ON_DEVICE         ((NTSTATUS)0xC01E0200L)
#endif

#ifndef STATUS_GRAPHICS_NO_AVAILABLE_VIDPN_TARGET
#define STATUS_GRAPHICS_NO_AVAILABLE_VIDPN_TARGET       ((NTSTATUS)0xC01E0333L)
#endif

#endif /* VMW_WDDM_COMPAT_H_ */
