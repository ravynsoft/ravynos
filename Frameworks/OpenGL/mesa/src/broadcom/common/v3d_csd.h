/*
 * Copyright © 2023 Raspberry Pi Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef V3D_CSD_H
#define V3D_CSD_H

#define V3D_CSD_CFG012_WG_COUNT_SHIFT 16
#define V3D_CSD_CFG012_WG_OFFSET_SHIFT 0
/* Allow this dispatch to start while the last one is still running. */
#define V3D_CSD_CFG3_OVERLAP_WITH_PREV (1 << 26)
/* Maximum supergroup ID.  6 bits. */
#define V3D_CSD_CFG3_MAX_SG_ID_SHIFT 20
/* Batches per supergroup minus 1.  8 bits. */
#define V3D_CSD_CFG3_BATCHES_PER_SG_M1_SHIFT 12
/* Workgroups per supergroup, 0 means 16 */
#define V3D_CSD_CFG3_WGS_PER_SG_SHIFT 8
#define V3D_CSD_CFG3_WG_SIZE_SHIFT 0

#define V3D_CSD_CFG5_PROPAGATE_NANS (1 << 2)
#define V3D_CSD_CFG5_SINGLE_SEG (1 << 1)
#define V3D_CSD_CFG5_THREADING (1 << 0)

#endif
