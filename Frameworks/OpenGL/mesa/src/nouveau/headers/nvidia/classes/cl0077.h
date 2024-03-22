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

#ifndef _cl0077_h_
#define _cl0077_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "nvtypes.h"

#define  NV04_SCALED_IMAGE_FROM_MEMORY                             (0x00000077)
/* NvNotification[] elements */
#define NV077_NOTIFIERS_NOTIFY                                     (0)
#define NV077_NOTIFIERS_MAXCOUNT								   (1)
/* NvNotification[] fields and values */
#define NV077_NOTIFICATION_STATUS_IN_PROGRESS                      (0x8000)
#define NV077_NOTIFICATION_STATUS_ERROR_PROTECTION_FAULT           (0x4000)
#define NV077_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT               (0x2000)
#define NV077_NOTIFICATION_STATUS_ERROR_INVALID_STATE              (0x1000)
#define NV077_NOTIFICATION_STATUS_ERROR_STATE_IN_USE               (0x0800)
#define NV077_NOTIFICATION_STATUS_DONE_SUCCESS                     (0x0000)
/* pio method data structure */
typedef volatile struct _cl0077_tag0 {
 NvV32 NoOperation;             /* ignored                          0100-0103*/
 NvV32 Notify;                  /* NV077_NOTIFY_*                   0104-0107*/
 NvV32 Reserved00[0x01e];
 NvV32 SetContextDmaNotifies;   /* NV01_CONTEXT_DMA                 0180-0183*/
 NvV32 SetContextDmaImage;      /* NV01_CONTEXT_DMA                 0184-0187*/
 NvV32 SetContextPattern;       /* NV04_CONTEXT_PATTERN             0188-018b*/
 NvV32 SetContextRop;           /* NV03_CONTEXT_ROP                 018c-018f*/
 NvV32 SetContextBeta1;         /* NV01_CONTEXT_BETA                0190-0193*/
 NvV32 SetContextBeta4;         /* NV04_CONTEXT_BETA                0194-0197*/
 NvV32 SetContextSurface;       /* NV04_CONTEXT_SURFACES_2D,SWIZZLE 0198-019b*/
 NvV32 Reserved01[0x059];
 NvV32 SetColorFormat;          /* NV077_SET_COLOR_FORMAT_*         0300-0303*/
 NvV32 SetOperation;            /* NV077_SET_OPERATION_*            0304-0307*/
 NvV32 ClipPoint;               /* y_x S16_S16                      0308-030b*/
 NvV32 ClipSize;                /* height_width U16_U16             030c-030f*/
 NvV32 ImageOutPoint;           /* y_x S16_S16                      0310-0313*/
 NvV32 ImageOutSize;            /* height_width U16_U16             0314-0317*/
 NvV32 DeltaDuDx;               /* S12d20 ratio du/dx               0318-031b*/
 NvV32 DeltaDvDy;               /* S12d20 ratio dv/dy               031c-031f*/
 NvV32 Reserved02[0x038];
 NvV32 ImageInSize;             /* height_width U16_U16             0400-0403*/
 NvU32 ImageInFormat;           /* interpolator_origin_pitch        0404-0407*/
 NvU32 ImageInOffset;           /* bytes                            0408-040b*/
 NvV32 ImageInPoint;            /* v_u U12d4_U12d4                  040c-040f*/
 NvV32 Reserved03[0x6fc];
} Nv077Typedef, Nv04ScaledImageFromMemory;
#define NV077_TYPEDEF                                 Nv04ScaledImageFromMemory
/* dma method offsets, fields, and values */
#define NV077_SET_OBJECT                                           (0x00000000)
#define NV077_NO_OPERATION                                         (0x00000100)
#define NV077_NOTIFY                                               (0x00000104)
#define NV077_NOTIFY_WRITE_ONLY                                    (0x00000000)
#define NV077_NOTIFY_WRITE_THEN_AWAKEN                             (0x00000001)
#define NV077_SET_CONTEXT_DMA_NOTIFIES                             (0x00000180)
#define NV077_SET_CONTEXT_DMA_IMAGE                                (0x00000184)
#define NV077_SET_CONTEXT_PATTERN                                  (0x00000188)
#define NV077_SET_CONTEXT_ROP                                      (0x0000018C)
#define NV077_SET_CONTEXT_BETA1                                    (0x00000190)
#define NV077_SET_CONTEXT_BETA4                                    (0x00000194)
#define NV077_SET_CONTEXT_SURFACE                                  (0x00000198)
#define NV077_SET_COLOR_FORMAT                                     (0x00000300)
#define NV077_SET_COLOR_FORMAT_LE_A1R5G5B5                         (0x00000001)
#define NV077_SET_COLOR_FORMAT_LE_X1R5G5B5                         (0x00000002)
#define NV077_SET_COLOR_FORMAT_LE_A8R8G8B8                         (0x00000003)
#define NV077_SET_COLOR_FORMAT_LE_X8R8G8B8                         (0x00000004)
#define NV077_SET_COLOR_FORMAT_LE_V8YB8U8YA8                       (0x00000005)
#define NV077_SET_COLOR_FORMAT_LE_YB8V8YA8U8                       (0x00000006)
#define NV077_SET_COLOR_FORMAT_LE_R5G6B5                           (0x00000007)
#define NV077_SET_OPERATION                                        (0x00000304)
#define NV077_SET_OPERATION_SRCCOPY_AND                            (0x00000000)
#define NV077_SET_OPERATION_ROP_AND                                (0x00000001)
#define NV077_SET_OPERATION_BLEND_AND                              (0x00000002)
#define NV077_SET_OPERATION_SRCCOPY                                (0x00000003)
#define NV077_SET_OPERATION_SRCCOPY_PREMULT                        (0x00000004)
#define NV077_SET_OPERATION_BLEND_PREMULT                          (0x00000005)
#define NV077_CLIP_POINT                                           (0x00000308)
#define NV077_CLIP_POINT_X                                         15:0
#define NV077_CLIP_POINT_Y                                         31:16
#define NV077_CLIP_SIZE                                            (0x0000030C)
#define NV077_CLIP_SIZE_WIDTH                                      15:0
#define NV077_CLIP_SIZE_HEIGHT                                     31:16
#define NV077_IMAGE_OUT_POINT                                      (0x00000310)
#define NV077_IMAGE_OUT_POINT_X                                    15:0
#define NV077_IMAGE_OUT_POINT_Y                                    31:16
#define NV077_IMAGE_OUT_SIZE                                       (0x00000314)
#define NV077_IMAGE_OUT_SIZE_WIDTH                                 15:0
#define NV077_IMAGE_OUT_SIZE_HEIGHT                                31:16
#define NV077_DELTA_DU_DX                                          (0x00000318)
#define NV077_DELTA_DV_DY                                          (0x0000031C)
#define NV077_IMAGE_IN_SIZE                                        (0x00000400)
#define NV077_IMAGE_IN_SIZE_WIDTH                                  15:0
#define NV077_IMAGE_IN_SIZE_HEIGHT                                 31:16
#define NV077_IMAGE_IN_FORMAT                                      (0x00000404)
#define NV077_IMAGE_IN_FORMAT_PITCH                                15:0
#define NV077_IMAGE_IN_FORMAT_ORIGIN                               23:16
#define NV077_IMAGE_IN_FORMAT_ORIGIN_CENTER                        (0x00000001)
#define NV077_IMAGE_IN_FORMAT_ORIGIN_CORNER                        (0x00000002)
#define NV077_IMAGE_IN_FORMAT_INTERPOLATOR                         31:24
#define NV077_IMAGE_IN_FORMAT_INTERPOLATOR_ZOH                     (0x00000000)
#define NV077_IMAGE_IN_FORMAT_INTERPOLATOR_FOH                     (0x00000001)
#define NV077_IMAGE_IN_OFFSET                                      (0x00000408)
#define NV077_IMAGE_IN                                             (0x0000040C)
#define NV077_IMAGE_IN_POINT_U                                     15:0
#define NV077_IMAGE_IN_POINT_V                                     31:16
/* obsolete stuff */
#define NV4_SCALED_IMAGE_FROM_MEMORY                               (0x00000077)
#define Nv4ScaledImageFromMemory                      Nv04ScaledImageFromMemory
#define nv4ScaledImageFromMemory                      Nv04ScaledImageFromMemory
#define nv04ScaledImageFromMemory                     Nv04ScaledImageFromMemory

#ifdef __cplusplus
};     /* extern "C" */
#endif

#endif /* _cl0077_h_ */
