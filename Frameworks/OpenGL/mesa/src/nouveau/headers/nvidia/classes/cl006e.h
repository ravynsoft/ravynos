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
#ifndef _cl006e_h_
#define _cl006e_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "nvtypes.h"

/* class NV10_CHANNEL_DMA */
#define  NV10_CHANNEL_DMA                                          (0x0000006E)
#define NV06E_NOTIFIERS_MAXCOUNT                                   1
/* NvNotification[] fields and values */
#define NV06E_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT               (0x2000)
#define NV06E_NOTIFICATION_STATUS_ERROR_PROTECTION_FAULT           (0x4000)
/* pio method data structure */
typedef volatile struct _cl006e_tag0 {
 NvV32 Reserved00[0x7c0];
} Nv06eTypedef, Nv10ChannelDma;
#define NV06E_TYPEDEF                                            Nv10ChannelDma
/* pio flow control data structure */
typedef volatile struct _cl006e_tag1 {
 NvV32 Ignored00[0x010];
 NvU32 Put;                     /* put offset, write only           0040-0043*/
 NvU32 Get;                     /* get offset, read only            0044-0047*/
 NvU32 Reference;               /* reference value, read only       0048-004b*/
 NvV32 Ignored01[0x1];
 NvU32 SetReference;            /* reference value, write only      0050-0053*/
 NvV32 Ignored02[0x3eb];
} Nv06eControl, Nv10ControlDma;
/* fields and values */
#define NV06E_NUMBER_OF_SUBCHANNELS                                (8)
#define NV06E_SET_OBJECT                                           (0x00000000)
#define NV06E_SET_REFERENCE                                        (0x00000050)

/* dma method descriptor format */
#define NV06E_DMA_METHOD_ADDRESS                                   12:2
#define NV06E_DMA_METHOD_SUBCHANNEL                                15:13
#define NV06E_DMA_METHOD_COUNT                                     28:18
#define NV06E_DMA_OPCODE                                           31:29
#define NV06E_DMA_OPCODE_METHOD                                    (0x00000000)
#define NV06E_DMA_OPCODE_NONINC_METHOD                             (0x00000002)
/* dma data format */
#define NV06E_DMA_DATA                                             31:0
/* dma jump format */
#define NV06E_DMA_OPCODE_JUMP                                      (0x00000001)
#define NV06E_DMA_JUMP_OFFSET                                      28:2

#ifdef __cplusplus
};     /* extern "C" */
#endif

#endif /* _cl006e_h_ */
