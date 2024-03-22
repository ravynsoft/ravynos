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
#ifndef _cl006c_h_
#define _cl006c_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "nvtypes.h"

/* class NV04_CHANNEL_DMA */
#define  NV04_CHANNEL_DMA                                          (0x0000006C)
#define NV06C_NOTIFIERS_MAXCOUNT                                   1
/* NvNotification[] fields and values */
#define NV06C_NOTIFICATION_STATUS_ERROR_BAD_ARGUMENT               (0x2000)
#define NV06C_NOTIFICATION_STATUS_ERROR_PROTECTION_FAULT           (0x4000)
/* pio method data structure */
typedef volatile struct _cl006c_tag0 {
 NvV32 Reserved00[0x7c0];
} Nv06cTypedef, Nv04ChannelDma;
#define NV06C_TYPEDEF                                            Nv04ChannelDma
/* pio flow control data structure */
typedef volatile struct _cl006c_tag1 {
 NvV32 Ignored00[0x010];
 NvU32 Put;                     /* put offset, write only           0040-0043*/
 NvU32 Get;                     /* get offset, read only            0044-0047*/
 NvV32 Ignored01[0x002];
 NvU32 StallNotifier;           /* Set stall notifier               0050-0053*/
 NvU32 StallChannel;            /* Stall the channel                0054-0057*/
 NvV32 Ignored02[0x3EA];
} Nv04ControlDma;
/* obsolete stuff */
#define NV4_CHANNEL_DMA                                            (0x0000006C)
#define Nv4ChannelDma                                            Nv04ChannelDma
#define nv4ChannelDma                                            Nv04ChannelDma
#define Nv4ControlDma                                            Nv04ControlDma

/* dma method descriptor format */
#define NV06C_METHOD_ADDRESS                                       12:2
#define NV06C_METHOD_SUBCHANNEL                                    15:13
#define NV06C_METHOD_COUNT                                         28:18
#define NV06C_OPCODE                                               31:29
#define NV06C_OPCODE_METHOD                                        (0x00000000)
#define NV06C_OPCODE_NONINC_METHOD                                 (0x00000002)
/* dma data format */
#define NV06C_DATA                                                 31:0
/* dma jump format */
#define NV06C_OPCODE_JUMP                                          (0x00000001)
#define NV06C_JUMP_OFFSET                                          28:2

#ifdef __cplusplus
};     /* extern "C" */
#endif

#endif /* _cl006c_h_ */
