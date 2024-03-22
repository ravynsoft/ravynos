/*
 * Copyright (c) 2003-2004, NVIDIA CORPORATION. All rights reserved.
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

#ifndef _cl_nv50_memory_to_memory_format_h_
#define _cl_nv50_memory_to_memory_format_h_

/* This file is generated - do not edit. */

#include "nvtypes.h"

#define NV50_MEMORY_TO_MEMORY_FORMAT    0x5039

#define NV5039_SET_OBJECT                                                                                  0x0000
#define NV5039_SET_OBJECT_POINTER                                                                            15:0

#define NV5039_NO_OPERATION                                                                                0x0100
#define NV5039_NO_OPERATION_V                                                                                31:0

#define NV5039_NOTIFY                                                                                      0x0104
#define NV5039_NOTIFY_TYPE                                                                                   31:0
#define NV5039_NOTIFY_TYPE_WRITE_ONLY                                                                  0x00000000
#define NV5039_NOTIFY_TYPE_WRITE_THEN_AWAKEN                                                           0x00000001

#define NV5039_WAIT_FOR_IDLE                                                                               0x0110
#define NV5039_WAIT_FOR_IDLE_V                                                                               31:0

#define NV5039_PM_TRIGGER                                                                                  0x0140
#define NV5039_PM_TRIGGER_V                                                                                  31:0

#define NV5039_SET_CONTEXT_DMA_NOTIFY                                                                      0x0180
#define NV5039_SET_CONTEXT_DMA_NOTIFY_HANDLE                                                                 31:0

#define NV5039_SET_CONTEXT_DMA_BUFFER_IN                                                                   0x0184
#define NV5039_SET_CONTEXT_DMA_BUFFER_IN_HANDLE                                                              31:0

#define NV5039_SET_CONTEXT_DMA_BUFFER_OUT                                                                  0x0188
#define NV5039_SET_CONTEXT_DMA_BUFFER_OUT_HANDLE                                                             31:0

#define NV5039_SET_SRC_MEMORY_LAYOUT                                                                       0x0200
#define NV5039_SET_SRC_MEMORY_LAYOUT_V                                                                        0:0
#define NV5039_SET_SRC_MEMORY_LAYOUT_V_BLOCKLINEAR                                                     0x00000000
#define NV5039_SET_SRC_MEMORY_LAYOUT_V_PITCH                                                           0x00000001

#define NV5039_SET_SRC_BLOCK_SIZE                                                                          0x0204
#define NV5039_SET_SRC_BLOCK_SIZE_WIDTH                                                                       3:0
#define NV5039_SET_SRC_BLOCK_SIZE_WIDTH_ONE_GOB                                                        0x00000000
#define NV5039_SET_SRC_BLOCK_SIZE_HEIGHT                                                                      7:4
#define NV5039_SET_SRC_BLOCK_SIZE_HEIGHT_ONE_GOB                                                       0x00000000
#define NV5039_SET_SRC_BLOCK_SIZE_HEIGHT_TWO_GOBS                                                      0x00000001
#define NV5039_SET_SRC_BLOCK_SIZE_HEIGHT_FOUR_GOBS                                                     0x00000002
#define NV5039_SET_SRC_BLOCK_SIZE_HEIGHT_EIGHT_GOBS                                                    0x00000003
#define NV5039_SET_SRC_BLOCK_SIZE_HEIGHT_SIXTEEN_GOBS                                                  0x00000004
#define NV5039_SET_SRC_BLOCK_SIZE_HEIGHT_THIRTYTWO_GOBS                                                0x00000005
#define NV5039_SET_SRC_BLOCK_SIZE_DEPTH                                                                      11:8
#define NV5039_SET_SRC_BLOCK_SIZE_DEPTH_ONE_GOB                                                        0x00000000
#define NV5039_SET_SRC_BLOCK_SIZE_DEPTH_TWO_GOBS                                                       0x00000001
#define NV5039_SET_SRC_BLOCK_SIZE_DEPTH_FOUR_GOBS                                                      0x00000002
#define NV5039_SET_SRC_BLOCK_SIZE_DEPTH_EIGHT_GOBS                                                     0x00000003
#define NV5039_SET_SRC_BLOCK_SIZE_DEPTH_SIXTEEN_GOBS                                                   0x00000004
#define NV5039_SET_SRC_BLOCK_SIZE_DEPTH_THIRTYTWO_GOBS                                                 0x00000005

#define NV5039_SET_SRC_WIDTH                                                                               0x0208
#define NV5039_SET_SRC_WIDTH_V                                                                               31:0

#define NV5039_SET_SRC_HEIGHT                                                                              0x020c
#define NV5039_SET_SRC_HEIGHT_V                                                                              31:0

#define NV5039_SET_SRC_DEPTH                                                                               0x0210
#define NV5039_SET_SRC_DEPTH_V                                                                               31:0

#define NV5039_SET_SRC_LAYER                                                                               0x0214
#define NV5039_SET_SRC_LAYER_V                                                                               31:0

#define NV5039_SET_SRC_ORIGIN                                                                              0x0218
#define NV5039_SET_SRC_ORIGIN_X                                                                              15:0
#define NV5039_SET_SRC_ORIGIN_Y                                                                             31:16

#define NV5039_SET_DST_MEMORY_LAYOUT                                                                       0x021c
#define NV5039_SET_DST_MEMORY_LAYOUT_V                                                                        0:0
#define NV5039_SET_DST_MEMORY_LAYOUT_V_BLOCKLINEAR                                                     0x00000000
#define NV5039_SET_DST_MEMORY_LAYOUT_V_PITCH                                                           0x00000001

#define NV5039_SET_DST_BLOCK_SIZE                                                                          0x0220
#define NV5039_SET_DST_BLOCK_SIZE_WIDTH                                                                       3:0
#define NV5039_SET_DST_BLOCK_SIZE_WIDTH_ONE_GOB                                                        0x00000000
#define NV5039_SET_DST_BLOCK_SIZE_HEIGHT                                                                      7:4
#define NV5039_SET_DST_BLOCK_SIZE_HEIGHT_ONE_GOB                                                       0x00000000
#define NV5039_SET_DST_BLOCK_SIZE_HEIGHT_TWO_GOBS                                                      0x00000001
#define NV5039_SET_DST_BLOCK_SIZE_HEIGHT_FOUR_GOBS                                                     0x00000002
#define NV5039_SET_DST_BLOCK_SIZE_HEIGHT_EIGHT_GOBS                                                    0x00000003
#define NV5039_SET_DST_BLOCK_SIZE_HEIGHT_SIXTEEN_GOBS                                                  0x00000004
#define NV5039_SET_DST_BLOCK_SIZE_HEIGHT_THIRTYTWO_GOBS                                                0x00000005
#define NV5039_SET_DST_BLOCK_SIZE_DEPTH                                                                      11:8
#define NV5039_SET_DST_BLOCK_SIZE_DEPTH_ONE_GOB                                                        0x00000000
#define NV5039_SET_DST_BLOCK_SIZE_DEPTH_TWO_GOBS                                                       0x00000001
#define NV5039_SET_DST_BLOCK_SIZE_DEPTH_FOUR_GOBS                                                      0x00000002
#define NV5039_SET_DST_BLOCK_SIZE_DEPTH_EIGHT_GOBS                                                     0x00000003
#define NV5039_SET_DST_BLOCK_SIZE_DEPTH_SIXTEEN_GOBS                                                   0x00000004
#define NV5039_SET_DST_BLOCK_SIZE_DEPTH_THIRTYTWO_GOBS                                                 0x00000005

#define NV5039_SET_DST_WIDTH                                                                               0x0224
#define NV5039_SET_DST_WIDTH_V                                                                               31:0

#define NV5039_SET_DST_HEIGHT                                                                              0x0228
#define NV5039_SET_DST_HEIGHT_V                                                                              31:0

#define NV5039_SET_DST_DEPTH                                                                               0x022c
#define NV5039_SET_DST_DEPTH_V                                                                               31:0

#define NV5039_SET_DST_LAYER                                                                               0x0230
#define NV5039_SET_DST_LAYER_V                                                                               31:0

#define NV5039_SET_DST_ORIGIN                                                                              0x0234
#define NV5039_SET_DST_ORIGIN_X                                                                              15:0
#define NV5039_SET_DST_ORIGIN_Y                                                                             31:16

#define NV5039_OFFSET_IN_UPPER                                                                             0x0238
#define NV5039_OFFSET_IN_UPPER_VALUE                                                                          7:0

#define NV5039_OFFSET_OUT_UPPER                                                                            0x023c
#define NV5039_OFFSET_OUT_UPPER_VALUE                                                                         7:0

#define NV5039_OFFSET_IN                                                                                   0x030c
#define NV5039_OFFSET_IN_VALUE                                                                               31:0

#define NV5039_OFFSET_OUT                                                                                  0x0310
#define NV5039_OFFSET_OUT_VALUE                                                                              31:0

#define NV5039_PITCH_IN                                                                                    0x0314
#define NV5039_PITCH_IN_VALUE                                                                                31:0

#define NV5039_PITCH_OUT                                                                                   0x0318
#define NV5039_PITCH_OUT_VALUE                                                                               31:0

#define NV5039_LINE_LENGTH_IN                                                                              0x031c
#define NV5039_LINE_LENGTH_IN_VALUE                                                                          31:0

#define NV5039_LINE_COUNT                                                                                  0x0320
#define NV5039_LINE_COUNT_VALUE                                                                              31:0

#define NV5039_FORMAT                                                                                      0x0324
#define NV5039_FORMAT_IN                                                                                      7:0
#define NV5039_FORMAT_IN_ONE                                                                           0x00000001
// removing options greater than _ONE due to hardware bug 294038.
// it is POSSIBLE to use them, but doing so will require some nasty workarounds,
// such as handling timeouts on notifiers that the HW will fail to write.
// as of july 2007, no driver needs them. hopefully they never will.
// #define NV5039_FORMAT_IN_TWO                                                                           0x00000002
// #define NV5039_FORMAT_IN_FOUR                                                                          0x00000004
// #define NV5039_FORMAT_IN_EIGHT                                                                         0x00000008
// #define NV5039_FORMAT_IN_SIXTEEN                                                                       0x00000010
#define NV5039_FORMAT_OUT                                                                                    15:8
#define NV5039_FORMAT_OUT_ONE                                                                          0x00000001
// #define NV5039_FORMAT_OUT_TWO                                                                          0x00000002
// #define NV5039_FORMAT_OUT_FOUR                                                                         0x00000004
// #define NV5039_FORMAT_OUT_EIGHT                                                                        0x00000008
// #define NV5039_FORMAT_OUT_SIXTEEN                                                                      0x00000010

#define NV5039_BUFFER_NOTIFY                                                                               0x0328
#define NV5039_BUFFER_NOTIFY_TYPE                                                                            31:0
#define NV5039_BUFFER_NOTIFY_TYPE_WRITE_ONLY                                                           0x00000000
#define NV5039_BUFFER_NOTIFY_TYPE_WRITE_THEN_AWAKEN                                                    0x00000001

#endif /* _cl_nv50_memory_to_memory_format_h_ */
