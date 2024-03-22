/*******************************************************************************
    Copyright (c) 2020, NVIDIA CORPORATION. All rights reserved.

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.

*******************************************************************************/

#ifndef _cl0042_h_
#define _cl0042_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "nvtypes.h"

#define  NV04_CONTEXT_SURFACES_2D                                  (0x00000042)
/* NvNotification[] elements */
#define NV042_NOTIFIERS_NOTIFY                                     (0)
#define NV042_NOTIFIERS_MAXCOUNT								   (1)
/* NvNotification[] fields and values */
#define NV042_NOTIFICATION_STATUS_IN_PROGRESS                      (0x8000)
#define NV042_NOTIFICATION_STATUS_ERROR_PROTECTION_FAULT           (0x4000)
#define NV042_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT               (0x2000)
#define NV042_NOTIFICATION_STATUS_ERROR_INVALID_STATE              (0x1000)
#define NV042_NOTIFICATION_STATUS_ERROR_STATE_IN_USE               (0x0800)
#define NV042_NOTIFICATION_STATUS_DONE_SUCCESS                     (0x0000)
/* pio method data structure */
typedef volatile struct _cl0042_tag0 {
 NvV32 NoOperation;             /* ignored                          0100-0103*/
 NvV32 Notify;                  /* NV042_NOTIFY_*                   0104-0107*/
 NvV32 Reserved00[0x01e];
 NvV32 SetContextDmaNotifies;   /* NV01_CONTEXT_DMA                 0180-0183*/
 NvV32 SetContextDmaImageSource;/* NV01_CONTEXT_DMA                 0184-0187*/
 NvV32 SetContextDmaImageDestin;/* NV01_CONTEXT_DMA                 0188-018b*/
 NvV32 Reserved01[0x05d];
 NvV32 SetColorFormat;          /* NV042_SET_COLOR_FORMAT_*         0300-0303*/
 NvU32 SetPitch;                /* destin_source U16_U16            0304-0307*/
 NvU32 SetOffsetSource;         /* byte offset of top-left pixel    0308-030b*/
 NvU32 SetOffsetDestin;         /* byte offset of top-left pixel    030c-030f*/
 NvV32 Reserved02[0x73c];
} Nv042Typedef, Nv04ContextSurfaces2d;
#define NV042_TYPEDEF                                     Nv04ContextSurfaces2d
/* dma method offsets, fields, and values */
#define NV042_SET_OBJECT                                           (0x00000000)
#define NV042_NO_OPERATION                                         (0x00000100)
#define NV042_NOTIFY                                               (0x00000104)
#define NV042_NOTIFY_WRITE_ONLY                                    (0x00000000)
#define NV042_NOTIFY_WRITE_THEN_AWAKEN                             (0x00000001)
#define NV042_SET_CONTEXT_DMA_NOTIFIES                             (0x00000180)
#define NV042_SET_CONTEXT_DMA_IMAGE_SOURCE                         (0x00000184)
#define NV042_SET_CONTEXT_DMA_IMAGE_DESTIN                         (0x00000188)
#define NV042_SET_COLOR_FORMAT                                     (0x00000300)
#define NV042_SET_COLOR_FORMAT_LE_Y8                               (0x00000001)
#define NV042_SET_COLOR_FORMAT_LE_X1R5G5B5_Z1R5G5B5                (0x00000002)
#define NV042_SET_COLOR_FORMAT_LE_X1R5G5B5_O1R5G5B5                (0x00000003)
#define NV042_SET_COLOR_FORMAT_LE_R5G6B5                           (0x00000004)
#define NV042_SET_COLOR_FORMAT_LE_Y16                              (0x00000005)
#define NV042_SET_COLOR_FORMAT_LE_X8R8G8B8_Z8R8G8B8                (0x00000006)
#define NV042_SET_COLOR_FORMAT_LE_X8R8G8B8_O8R8G8B8                (0x00000007)
#define NV042_SET_COLOR_FORMAT_LE_X1A7R8G8B8_Z1A7R8G8B8            (0x00000008)
#define NV042_SET_COLOR_FORMAT_LE_X1A7R8G8B8_O1A7R8G8B8            (0x00000009)
#define NV042_SET_COLOR_FORMAT_LE_A8R8G8B8                         (0x0000000A)
#define NV042_SET_COLOR_FORMAT_LE_Y32                              (0x0000000B)
#define NV042_SET_PITCH                                            (0x00000304)
#define NV042_SET_PITCH_SOURCE                                     15:0
#define NV042_SET_PITCH_DESTIN                                     31:16
#define NV042_SET_OFFSET_SOURCE                                    (0x00000308)
#define NV042_SET_OFFSET_DESTIN                                    (0x0000030C)
/* obsolete stuff */
#define NV4_CONTEXT_SURFACES_2D                                    (0x00000042)
#define Nv4ContextSurfaces2d                              Nv04ContextSurfaces2d
#define nv4ContextSurfaces2d                              Nv04ContextSurfaces2d
#define nv4ContextSurfaces2D                              Nv04ContextSurfaces2d
#define nv04ContextSurfaces2d                             Nv04ContextSurfaces2d

#ifdef __cplusplus
};     /* extern "C" */
#endif

#endif /* _cl0042_h_ */
