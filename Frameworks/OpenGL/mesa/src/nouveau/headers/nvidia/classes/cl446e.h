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
#ifndef _cl446e_h_
#define _cl446e_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "nvtypes.h"

/* class NV44_CHANNEL_DMA */
#define  NV44_CHANNEL_DMA                                          (0x0000446E)
/* NvNotification[] fields and values */
#define NV446E_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT              (0x2000)
#define NV446E_NOTIFICATION_STATUS_ERROR_PROTECTION_FAULT          (0x4000)
/* pio method data structure */
typedef volatile struct _cl446e_tag0 {
 NvV32 Reserved00[0x7c0];
} Nv446eTypedef, Nv44ChannelDma;
#define NV446E_TYPEDEF                                           Nv44ChannelDma
/* pio flow control data structure */
typedef volatile struct _cl446e_tag1 {
 NvV32 Ignored00[0x010];
 NvU32 Put;                     /* put offset, write only           0040-0043*/
 NvU32 Get;                     /* get offset, read only            0044-0047*/
 NvU32 Reference;               /* reference value, read only       0048-004b*/
 NvU32 Ignored01[0x1];
 NvU32 SetReference;            /* set reference value              0050-0053*/
 NvU32 TopLevelGet;             /* top level get offset, read only  0054-0057*/
 NvU32 Ignored02[0x2];
 NvU32 SetContextDmaSemaphore;  /* set sema ctxdma, write only      0060-0063*/
 NvU32 SetSemaphoreOffset;      /* set sema offset, write only      0064-0067*/
 NvU32 SetSemaphoreAcquire;     /* set sema acquire, write only     0068-006b*/
 NvU32 SetSemaphoreRelease;     /* set sema release, write only     006c-006f*/
 NvV32 Ignored03[0x4];
 NvU32 Yield;                   /* engine yield, write only         0080-0083*/
 NvV32 Ignored04[0x3df];
} Nv446eControl, Nv44ControlDma;
/* fields and values */
#define NV446E_NUMBER_OF_SUBCHANNELS                               (8)
#define NV446E_SET_OBJECT                                          (0x00000000)
#define NV446E_SET_REFERENCE                                       (0x00000050)
#define NV446E_SET_CONTEXT_DMA_SEMAPHORE                           (0x00000060)
#define NV446E_SEMAPHORE_OFFSET                                    (0x00000064)
#define NV446E_SEMAPHORE_ACQUIRE                                   (0x00000068)
#define NV446E_SEMAPHORE_RELEASE                                   (0x0000006c)
#define NV446E_YIELD                                               (0x00000080)

/* dma method descriptor format */
#define NV446E_DMA_METHOD_ADDRESS                                  12:2
#define NV446E_DMA_METHOD_SUBCHANNEL                               15:13
#define NV446E_DMA_METHOD_COUNT                                    28:18

/* dma opcode format */
#define NV446E_DMA_OPCODE                                          31:29
#define NV446E_DMA_OPCODE_METHOD                                   (0x00000000)
#define NV446E_DMA_OPCODE_NONINC_METHOD                            (0x00000002)
/* dma jump format */
#define NV446E_DMA_OPCODE_JUMP                                     (0x00000001)
#define NV446E_DMA_JUMP_OFFSET                                     28:2

/* dma opcode2 format */
#define NV446E_DMA_OPCODE2                                         1:0
#define NV446E_DMA_OPCODE2_NONE                                    (0x00000000)
/* dma jump_long format */
#define NV446E_DMA_OPCODE2_JUMP_LONG                               (0x00000001)
#define NV446E_DMA_JUMP_LONG_OFFSET                                31:2
/* dma call format */
#define NV446E_DMA_OPCODE2_CALL                                    (0x00000002)
#define NV446E_DMA_CALL_OFFSET                                     31:2

/* dma opcode3 format */
#define NV446E_DMA_OPCODE3                                         17:16
#define NV446E_DMA_OPCODE3_NONE                                    (0x00000000)
/* dma return format */
#define NV446E_DMA_RETURN                                          (0x00020000)
#define NV446E_DMA_OPCODE3_RETURN                                  (0x00000002)

/* dma data format */
#define NV446E_DMA_DATA                                            31:0

/* dma nop format */
#define NV446E_DMA_NOP                                             (0x00000000)

/* dma set subdevice mask format */
#define NV446E_DMA_SET_SUBDEVICE_MASK                              (0x00010000)
#define NV446E_DMA_SET_SUBDEVICE_MASK_VALUE                        15:4
#define NV446E_DMA_OPCODE3_SET_SUBDEVICE_MASK                      (0x00000001)

#ifdef __cplusplus
};     /* extern "C" */
#endif

#endif /* _cl446e_h_ */
