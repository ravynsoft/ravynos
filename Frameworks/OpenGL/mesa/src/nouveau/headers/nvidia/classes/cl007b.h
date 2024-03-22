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

#ifndef _cl007b_h_
#define _cl007b_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "nvtypes.h"

#define  NV10_TEXTURE_FROM_CPU                                     (0x0000007B)
/* NvNotification[] elements */
#define NV07B_NOTIFIERS_NOTIFY                                     (0)
#define NV07B_NOTIFIERS_MAXCOUNT								   (1)
/* NvNotification[] fields and values */
#define NV07B_NOTIFICATION_STATUS_IN_PROGRESS                      (0x8000)
#define NV07B_NOTIFICATION_STATUS_ERROR_PROTECTION_FAULT           (0x4000)
#define NV07B_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT               (0x2000)
#define NV07B_NOTIFICATION_STATUS_ERROR_INVALID_STATE              (0x1000)
#define NV07B_NOTIFICATION_STATUS_ERROR_STATE_IN_USE               (0x0800)
#define NV07B_NOTIFICATION_STATUS_DONE_SUCCESS                     (0x0000)
/* pio method data structure */
typedef volatile struct _cl007b_tag0 {
 NvV32 NoOperation;             /* ignored                          0100-0103*/
 NvV32 Notify;                  /* NV089_NOTIFY_*                   0104-0107*/
 NvV32 Reserved00[0x01e];
 NvV32 SetContextDmaNotifies;   /* NV01_CONTEXT_DMA                 0180-0183*/
 NvV32 SetContextSurface;       /* NV04_CONTEXT_SURFACES_2D         0184-0187*/
 NvV32 Reserved01[0x05e];
 NvV32 SetColorFormat;          /* NV07B_SET_COLOR_FORMAT_*         0300-0303*/
 NvV32 Point;                   /* y_x S16_S16 in pixels            0304-0307*/
 NvV32 Size;                    /* height_width U16_U16, pixels     0308-030b*/
 NvV32 ClipHorizontal;          /* width_x U16_U16                  030c-030f*/
 NvV32 ClipVertical;            /* height_y U16_U16                 0310-0313*/
 NvV32 Reserved02[0x03b];
 struct {                       /* start aliased methods in array   0400-    */
  NvV32 a;                      /* source colors (packed texels)       0-   3*/
  NvV32 b;                      /* source colors (packed texels)       4-   7*/
 } Color[896];                  /* end of aliased methods in array      -1fff*/
} Nv07bTypedef, Nv10TextureFromCpu;
#define NV07B_TYPEDEF                                        Nv10TextureFromCpu
/* dma method offsets, fields, and values */
#define NV07B_SET_OBJECT                                           (0x00000000)
#define NV07B_NO_OPERATION                                         (0x00000100)
#define NV07B_NOTIFY                                               (0x00000104)
#define NV07B_NOTIFY_WRITE_ONLY                                    (0x00000000)
#define NV07B_NOTIFY_WRITE_THEN_AWAKEN                             (0x00000001)
#define NV07B_SET_CONTEXT_DMA_NOTIFIES                             (0x00000180)
#define NV07B_SET_CONTEXT_SURFACE                                  (0x00000184)
#define NV07B_SET_COLOR_FORMAT                                     (0x00000300)
#define NV07B_SET_COLOR_FORMAT_LE_R5G6B5                           (0x00000001)
#define NV07B_SET_COLOR_FORMAT_LE_A1R5G5B5                         (0x00000002)
#define NV07B_SET_COLOR_FORMAT_LE_X1R5G5B5                         (0x00000003)
#define NV07B_SET_COLOR_FORMAT_LE_A8R8G8B8                         (0x00000004)
#define NV07B_SET_COLOR_FORMAT_LE_X8R8G8B8                         (0x00000005)
#define NV07B_POINT                                                (0x00000304)
#define NV07B_POINT_X                                              15:0
#define NV07B_POINT_Y                                              31:16
#define NV07B_SIZE                                                 (0x00000308)
#define NV07B_SIZE_WIDTH                                           15:0
#define NV07B_SIZE_HEIGHT                                          31:16
#define NV07B_CLIP_HORIZONTAL                                      (0x0000030C)
#define NV07B_CLIP_HORIZONTAL_X                                    15:0
#define NV07B_CLIP_HORIZONTAL_WIDTH                                31:16
#define NV07B_CLIP_VERTICAL                                        (0x00000310)
#define NV07B_CLIP_VERTICAL_Y                                      15:0
#define NV07B_CLIP_VERTICAL_HEIGHT                                 31:16
#define NV07B_COLOR(a)                                             (0x00000400\
                                                                   +(a)*0x0008)
#define NV07B_COLOR_A(a)                                           (0x00000400\
                                                                   +(a)*0x0008)
#define NV07B_COLOR_B(a)                                           (0x00000404\
                                                                   +(a)*0x0008)
#define NV07B_COLOR__SIZE_1                                        896

#ifdef __cplusplus
};     /* extern "C" */
#endif

#endif /* _cl007b_h_ */
