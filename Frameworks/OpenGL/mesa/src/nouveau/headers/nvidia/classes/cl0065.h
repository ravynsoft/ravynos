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

#ifndef _cl0065_h_
#define _cl0065_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "nvtypes.h"

#define  NV05_IMAGE_FROM_CPU                                       (0x00000065)
/* NvNotification[] elements */
#define NV065_NOTIFIERS_NOTIFY                                     (0)
#define NV065_NOTIFIERS_MAXCOUNT								   (1)

/* NvNotification[] fields and values */
#define NV065_NOTIFICATION_STATUS_IN_PROGRESS                      (0x8000)
#define NV065_NOTIFICATION_STATUS_ERROR_PROTECTION_FAULT           (0x4000)
#define NV065_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT               (0x2000)
#define NV065_NOTIFICATION_STATUS_ERROR_INVALID_STATE              (0x1000)
#define NV065_NOTIFICATION_STATUS_ERROR_STATE_IN_USE               (0x0800)
#define NV065_NOTIFICATION_STATUS_DONE_SUCCESS                     (0x0000)
/* pio method data structure */
typedef volatile struct _cl0065_tag0 {
 NvV32 NoOperation;             /* ignored                          0100-0103*/
 NvV32 Notify;                  /* NV065_NOTIFY_*                   0104-0107*/
 NvV32 Reserved00[0x01e];
 NvV32 SetContextDmaNotifies;   /* NV01_CONTEXT_DMA                 0180-0183*/
 NvV32 SetContextColorKey;      /* NV04_CONTEXT_COLOR_KEY           0184-0187*/
 NvV32 SetContextClipRectangle; /* NV01_CONTEXT_CLIP_RECTANGLE      0188-018b*/
 NvV32 SetContextPattern;       /* NV04_CONTEXT_PATTERN             018c-018f*/
 NvV32 SetContextRop;           /* NV03_CONTEXT_ROP                 0190-0193*/
 NvV32 SetContextBeta1;         /* NV01_CONTEXT_BETA                0194-0197*/
 NvV32 SetContextBeta4;         /* NV04_CONTEXT_BETA                0198-019b*/
 NvV32 SetContextSurface;       /* NV04_CONTEXT_SURFACES_2D         019c-019f*/
 NvV32 Reserved01[0x056];
 NvV32 SetColorConversion;      /* NV065_SET_COLOR_CONVERSION_*     02f8-02fb*/
 NvV32 SetOperation;            /* NV065_SET_OPERATION_*            02fc-02ff*/
 NvV32 SetColorFormat;          /* NV065_SET_COLOR_FORMAT_*         0300-0303*/
 NvV32 Point;                   /* y_x S16_S16 in pixels            0304-0307*/
 NvV32 SizeOut;                 /* height_width U16_U16, pixels     0308-030b*/
 NvV32 SizeIn;                  /* height_width U16_U16, pixels     030c-030f*/
 NvV32 Reserved02[0x03c];
 NvV32 Color[1792];             /* source colors (packed texels)    0400-1fff*/
} Nv065Typedef, Nv05ImageFromCpu;
#define NV065_TYPEDEF                                          Nv05ImageFromCpu
/* dma method offsets, fields, and values */
#define NV065_SET_OBJECT                                           (0x00000000)
#define NV065_NO_OPERATION                                         (0x00000100)
#define NV065_NOTIFY                                               (0x00000104)
#define NV065_NOTIFY_WRITE_ONLY                                    (0x00000000)
#define NV065_NOTIFY_WRITE_THEN_AWAKEN                             (0x00000001)
#define NV065_SET_CONTEXT_DMA_NOTIFIES                             (0x00000180)
#define NV065_SET_CONTEXT_COLOR_KEY                                (0x00000184)
#define NV065_SET_CONTEXT_CLIP_RECTANGLE                           (0x00000188)
#define NV065_SET_CONTEXT_PATTERN                                  (0x0000018C)
#define NV065_SET_CONTEXT_ROP                                      (0x00000190)
#define NV065_SET_CONTEXT_BETA1                                    (0x00000194)
#define NV065_SET_CONTEXT_BETA4                                    (0x00000198)
#define NV065_SET_CONTEXT_SURFACE                                  (0x0000019C)
#define NV065_SET_COLOR_CONVERSION                                 (0x000002F8)
#define NV065_SET_COLOR_CONVERSION_DITHER                          (0x00000000)
#define NV065_SET_COLOR_CONVERSION_TRUNCATE                        (0x00000001)
#define NV065_SET_COLOR_CONVERSION_SUBTRACT_TRUNCATE               (0x00000002)
#define NV065_SET_OPERATION                                        (0x000002FC)
#define NV065_SET_OPERATION_SRCCOPY_AND                            (0x00000000)
#define NV065_SET_OPERATION_ROP_AND                                (0x00000001)
#define NV065_SET_OPERATION_BLEND_AND                              (0x00000002)
#define NV065_SET_OPERATION_SRCCOPY                                (0x00000003)
#define NV065_SET_OPERATION_SRCCOPY_PREMULT                        (0x00000004)
#define NV065_SET_OPERATION_BLEND_PREMULT                          (0x00000005)
#define NV065_SET_COLOR_FORMAT                                     (0x00000300)
#define NV065_SET_COLOR_FORMAT_LE_R5G6B5                           (0x00000001)
#define NV065_SET_COLOR_FORMAT_LE_A1R5G5B5                         (0x00000002)
#define NV065_SET_COLOR_FORMAT_LE_X1R5G5B5                         (0x00000003)
#define NV065_SET_COLOR_FORMAT_LE_A8R8G8B8                         (0x00000004)
#define NV065_SET_COLOR_FORMAT_LE_X8R8G8B8                         (0x00000005)
#define NV065_POINT                                                (0x00000304)
#define NV065_POINT_X                                              15:0
#define NV065_POINT_Y                                              31:16
#define NV065_SIZE_OUT                                             (0x00000308)
#define NV065_SIZE_OUT_WIDTH                                       15:0
#define NV065_SIZE_OUT_HEIGHT                                      31:16
#define NV065_SIZE_IN                                              (0x0000030C)
#define NV065_SIZE_IN_WIDTH                                        15:0
#define NV065_SIZE_IN_HEIGHT                                       31:16
#define NV065_COLOR(a)                                             (0x00000400\
                                                                   +(a)*0x0004)
#define NV065_COLOR__SIZE_1                                        1792

#ifdef __cplusplus
};     /* extern "C" */
#endif

#endif /* _cl0065_h_ */
