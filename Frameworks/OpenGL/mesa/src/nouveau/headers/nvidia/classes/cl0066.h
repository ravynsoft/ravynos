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

#ifndef _cl0066_h_
#define _cl0066_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "nvtypes.h"

#define  NV05_STRETCHED_IMAGE_FROM_CPU                             (0x00000066)
/* NvNotification[] elements */
#define NV066_NOTIFIERS_NOTIFY                                     (0)
#define NV066_NOTIFIERS_MAXCOUNT								   (1)
/* NvNotification[] fields and values */
#define NV066_NOTIFICATION_STATUS_IN_PROGRESS                      (0x8000)
#define NV066_NOTIFICATION_STATUS_ERROR_PROTECTION_FAULT           (0x4000)
#define NV066_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT               (0x2000)
#define NV066_NOTIFICATION_STATUS_ERROR_INVALID_STATE              (0x1000)
#define NV066_NOTIFICATION_STATUS_ERROR_STATE_IN_USE               (0x0800)
#define NV066_NOTIFICATION_STATUS_DONE_SUCCESS                     (0x0000)
/* pio method data structure */
/* MSVC71 bug: cannot compile this typedef as C++ without a struct tag */
/*             fixed in MSVC8 and later */
#define TAGHACK
#if defined(NV_WINDOWS)
#undef  TAGHACK
#define TAGHACK tagHack
#endif
typedef volatile struct TAGHACK {
#undef TAGHACK
 NvV32 NoOperation;             /* ignored                          0100-0103*/
 NvV32 Notify;                  /* NV066_NOTIFY_*                   0104-0107*/
 NvV32 Reserved00[0x01e];
 NvV32 SetContextDmaNotifies;   /* NV01_CONTEXT_DMA                 0180-0183*/
 NvV32 SetContextColorKey;      /* NV04_CONTEXT_COLOR_KEY           0184-0187*/
 NvV32 SetContextPattern;       /* NV04_CONTEXT_PATTERN             0188-018b*/
 NvV32 SetContextRop;           /* NV03_CONTEXT_ROP                 018c-018f*/
 NvV32 SetContextBeta1;         /* NV01_CONTEXT_BETA                0190-0193*/
 NvV32 SetContextBeta4;         /* NV04_CONTEXT_BETA                0194-0197*/
 NvV32 SetContextSurface;       /* NV04_CONTEXT_SURFACES_2D         0198-019b*/
 NvV32 Reserved01[0x057];
 NvV32 SetColorConversion;      /* NV066_SET_COLOR_CONVERSION_*     02f8-02fb*/
 NvV32 SetOperation;            /* NV066_SET_OPERATION_*            02fc-02ff*/
 NvV32 SetColorFormat;          /* NV066_SET_COLOR_FORMAT_*         0300-0303*/
 NvV32 SizeIn;                  /* height_width U16_U16 in texels   0304-0307*/
 NvV32 DxDs;                    /* S12d20 dx/ds                     0308-030b*/
 NvV32 DyDt;                    /* S12d20 dy/dt                     030c-030f*/
 NvV32 ClipPoint;               /* y_x S16_S16                      0310-0313*/
 NvV32 ClipSize;                /* height_width U16_U16             0314-0317*/
 NvV32 Point12d4;               /* y_x S12d4_S12d4 in pixels        0318-031b*/
 NvV32 Reserved02[0x039];
 NvV32 Color[1792];             /* source colors (packed texels)    0400-1fff*/
} Nv066Typedef, Nv05StretchedImageFromCpu;
#define NV066_TYPEDEF                                 Nv05StretchedImageFromCpu
/* dma method offsets, fields, and values */
#define NV066_SET_OBJECT                                           (0x00000000)
#define NV066_NO_OPERATION                                         (0x00000100)
#define NV066_NOTIFY                                               (0x00000104)
#define NV066_NOTIFY_WRITE_ONLY                                    (0x00000000)
#define NV066_NOTIFY_WRITE_THEN_AWAKEN                             (0x00000001)
#define NV066_SET_CONTEXT_DMA_NOTIFIES                             (0x00000180)
#define NV066_SET_CONTEXT_COLOR_KEY                                (0x00000184)
#define NV066_SET_CONTEXT_PATTERN                                  (0x00000188)
#define NV066_SET_CONTEXT_ROP                                      (0x0000018C)
#define NV066_SET_CONTEXT_BETA1                                    (0x00000190)
#define NV066_SET_CONTEXT_BETA4                                    (0x00000194)
#define NV066_SET_CONTEXT_SURFACE                                  (0x00000198)
#define NV066_SET_COLOR_CONVERSION                                 (0x000002F8)
#define NV066_SET_COLOR_CONVERSION_DITHER                          (0x00000000)
#define NV066_SET_COLOR_CONVERSION_TRUNCATE                        (0x00000001)
#define NV066_SET_COLOR_CONVERSION_SUBTRACT_TRUNCATE               (0x00000002)
#define NV066_SET_OPERATION                                        (0x000002FC)
#define NV066_SET_OPERATION_SRCCOPY_AND                            (0x00000000)
#define NV066_SET_OPERATION_ROP_AND                                (0x00000001)
#define NV066_SET_OPERATION_BLEND_AND                              (0x00000002)
#define NV066_SET_OPERATION_SRCCOPY                                (0x00000003)
#define NV066_SET_OPERATION_SRCCOPY_PREMULT                        (0x00000004)
#define NV066_SET_OPERATION_BLEND_PREMULT                          (0x00000005)
#define NV066_SET_COLOR_FORMAT                                     (0x00000300)
#define NV066_SET_COLOR_FORMAT_LE_R5G6B5                           (0x00000001)
#define NV066_SET_COLOR_FORMAT_LE_A1R5G5B5                         (0x00000002)
#define NV066_SET_COLOR_FORMAT_LE_X1R5G5B5                         (0x00000003)
#define NV066_SET_COLOR_FORMAT_LE_A8R8G8B8                         (0x00000004)
#define NV066_SET_COLOR_FORMAT_LE_X8R8G8B8                         (0x00000005)
#define NV066_SIZE_IN                                              (0x00000304)
#define NV066_SIZE_IN_WIDTH                                        15:0
#define NV066_SIZE_IN_HEIGHT                                       31:16
#define NV066_DX_DS                                                (0x00000308)
#define NV066_DY_DT                                                (0x0000030C)
#define NV066_CLIP_POINT                                           (0x00000310)
#define NV066_CLIP_POINT_X                                         15:0
#define NV066_CLIP_POINT_Y                                         31:16
#define NV066_CLIP_SIZE                                            (0x00000314)
#define NV066_CLIP_SIZE_WIDTH                                      15:0
#define NV066_CLIP_SIZE_HEIGHT                                     31:16
#define NV066_POINT_12D4                                           (0x00000318)
#define NV066_POINT_12D4_X                                         15:0
#define NV066_POINT_12D4_Y                                         31:16
#define NV066_COLOR(a)                                             (0x00000400\
                                                                   +(a)*0x0004)
#define NV066_COLOR__SIZE_1                                        1792

#ifdef __cplusplus
};     /* extern "C" */
#endif

#endif /* _cl0066_h_ */
