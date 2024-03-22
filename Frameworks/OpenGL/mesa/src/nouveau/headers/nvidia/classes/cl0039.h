/*
 * Copyright (c) 2001-2001, NVIDIA CORPORATION. All rights reserved.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef _cl0039_h_
#define _cl0039_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "nvtypes.h"

#define  NV03_MEMORY_TO_MEMORY_FORMAT                              (0x00000039)
/* NvNotification[] elements */
#define NV039_NOTIFIERS_NOTIFY                                     (0)
#define NV039_NOTIFIERS_BUFFER_NOTIFY                              (1)
#define NV039_NOTIFIERS_MAXCOUNT                                   (2)
/* NvNotification[] fields and values */
#define NV039_NOTIFICATION_STATUS_IN_PROGRESS                      (0x8000)
#define NV039_NOTIFICATION_STATUS_ERROR_PROTECTION_FAULT           (0x4000)
#define NV039_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT               (0x2000)
#define NV039_NOTIFICATION_STATUS_ERROR_INVALID_STATE              (0x1000)
#define NV039_NOTIFICATION_STATUS_ERROR_STATE_IN_USE               (0x0800)
#define NV039_NOTIFICATION_STATUS_DONE_SUCCESS                     (0x0000)
/* pio method data structure */
typedef volatile struct _cl0039_tag0 {
 NvV32 NoOperation;             /* ignored                          0100-0103*/
 NvV32 Notify;                  /* NV039_NOTIFY_*                   0104-0107*/
 NvV32 Reserved00[0x01e];
 NvV32 SetContextDmaNotifies;   /* NV01_CONTEXT_DMA                 0180-0183*/
 NvV32 SetContextDmaBufferIn;   /* NV01_CONTEXT_DMA                 0184-0187*/
 NvV32 SetContextDmaBufferOut;  /* NV01_CONTEXT_DMA                 0188-018b*/
 NvV32 Reserved01[0x060];
#ifdef NV_LDDM
 NvU32 LDDMBuffer;              /* Buffer Number                    0300-0303*/
 NvU32 LDDMSetResource;         /* Set Resource                     0304-0307*/
 NvU32 LDDMSetResourceState;    /* Set Resource State               0308-030B*/
#endif
 NvU32 OffsetIn;                /* src offset in bytes              030c-030f*/
 NvU32 OffsetOut;               /* dst offset in bytes              0310-0313*/
 NvS32 PitchIn;                 /* delta in bytes, vert pixel delta 0314-0317*/
 NvS32 PitchOut;                /* delta in bytes, vert pixel delta 0318-031b*/
 NvU32 LineLengthIn;            /* in bytes                         031c-031f*/
 NvU32 LineCount;               /* in lines                         0320-0323*/
 NvV32 Format;                  /* out_in U24_U8                    0324-0327*/
 NvV32 BufferNotify;            /* NV039_BUFFER_NOTIFY_*            0328-032b*/
 NvV32 Reserved02[0x735];
} Nv039Typedef, Nv03MemoryToMemoryFormat;
#define NV039_TYPEDEF                                  Nv03MemoryToMemoryFormat
/* dma method offsets, fields, and values */
#define NV039_SET_OBJECT                                           (0x00000000)
#define NV039_NO_OPERATION                                         (0x00000100)
#define NV039_NOTIFY                                               (0x00000104)
#define NV039_NOTIFY_WRITE_ONLY                                    (0x00000000)
#define NV039_NOTIFY_WRITE_THEN_AWAKEN_1                           (0x00000001)
#define NV039_NOTIFY_WRITE_THEN_AWAKEN_2                           (0x00000002)
#define NV039_NOTIFY_WRITE_THEN_AWAKEN_3                           (0x00000003)
#define NV039_NOTIFY_WRITE_THEN_AWAKEN_4                           (0x00000004)
#define NV039_NOTIFY_WRITE_THEN_AWAKEN_5                           (0x00000005)
#define NV039_NOTIFY_WRITE_THEN_AWAKEN_6                           (0x00000006)
#define NV039_NOTIFY_WRITE_THEN_AWAKEN_7                           (0x00000007)
#define NV039_NOTIFY_WRITE_THEN_AWAKEN_8                           (0x00000008)
#define NV039_NOTIFY_WRITE_THEN_AWAKEN_9                           (0x00000009)
#define NV039_NOTIFY_WRITE_THEN_AWAKEN_A                           (0x0000000A)
#define NV039_NOTIFY_WRITE_THEN_AWAKEN_B                           (0x0000000B)
#define NV039_NOTIFY_WRITE_THEN_AWAKEN_C                           (0x0000000C)
#define NV039_NOTIFY_WRITE_THEN_AWAKEN_D                           (0x0000000D)
#define NV039_NOTIFY_WRITE_THEN_AWAKEN_E                           (0x0000000E)
#define NV039_NOTIFY_WRITE_THEN_AWAKEN_F                           (0x0000000F)
#define NV039_SET_CONTEXT_DMA_NOTIFIES                             (0x00000180)
#define NV039_SET_CONTEXT_DMA_BUFFER_IN                            (0x00000184)
#define NV039_SET_CONTEXT_DMA_BUFFER_OUT                           (0x00000188)

#ifdef NV_LDDM
// For 32 Bit Handles
#define NV039_LDDM_BIND_RESOURCE                                   (0x00000200)
#define NV039_LDDM_UNBIND_RESOURCE                                 (0x00000204)
#define NV039_LDDM_ENABLE_RESOURCE                                 (0x00000208)
#define NV039_LDDM_UPDATE_OFFSET                                   (0x0000020C)
#define NV039_LDDM_UPDATE_OFFSET_LOCATION                          1:0
#define NV039_LDDM_UPDATE_OFFSET_LOCATION_VIDMEM                   0x00000000
#define NV039_LDDM_UPDATE_OFFSET_LOCATION_PCI                      0x00000001
#define NV039_LDDM_UPDATE_OFFSET_OFFSET                            31:2

#define NV039_LDDM_DISABLE_RESOURCE                                (0x00000210)
#define NV039_LDDM_ENABLE_RESOURCE_IMMEDIATE                       (0x00000214)


#endif

#define NV039_SET_HRESOURCE                                        (0x00000218)
#define NV039_SET_HDMA                                             (0x0000021C)
#define NV039_SET_HDMA_CONTEXT                                     (0x00000220)
#define NV039_SET_START_PAGE                                       (0x00000224)
//#define NV039_SET_COUNT                                            (0x00000228)
#define NV039_SET_HIGH_PTE_DATA                                    (0x0000022C)
#define NV039_SET_LOW_PTE_DATA                                     (0x00000230)

#define NV039_OFFSET_IN                                            (0x0000030C)
#define NV039_OFFSET_OUT                                           (0x00000310)
#define NV039_PITCH_IN                                             (0x00000314)
#define NV039_PITCH_OUT                                            (0x00000318)
#define NV039_LINE_LENGTH_IN                                       (0x0000031C)
#define NV039_LINE_COUNT                                           (0x00000320)
#define NV039_FORMAT                                               (0x00000324)
#define NV039_FORMAT_IN                                            7:0
#define NV039_FORMAT_OUT                                           31:8
#define NV039_BUFFER_NOTIFY                                        (0x00000328)
#define NV039_BUFFER_NOTIFY_WRITE_ONLY                             (0x00000000)
#define NV039_BUFFER_NOTIFY_WRITE_THEN_AWAKEN                      (0x00000001)
/* obsolete stuff */
#define NV3_MEMORY_TO_MEMORY_FORMAT                                (0x00000039)
#define Nv3MemoryToMemoryFormat                        Nv03MemoryToMemoryFormat
#define nv3MemoryToMemoryFormat                        Nv03MemoryToMemoryFormat
#define nv03MemoryToMemoryFormat                       Nv03MemoryToMemoryFormat

#ifdef __cplusplus
};     /* extern "C" */
#endif

#endif /* _cl0039_h_ */
