/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef PVR_TYPES_H
#define PVR_TYPES_H

#include <inttypes.h>
#include <stdint.h>

/**
 * \brief Convert dwords to bytes.
 *
 * Throughout the code base we keep sizes in dwords (e.g. code_size and
 * data_size returned by the pds api) and in some cases we need to convert those
 * to be in bytes.
 *
 * This macro makes the conversion more obvious.
 */
/* clang-format off */
#define PVR_DW_TO_BYTES(_value) ((_value) * 4)
/* clang-format on */

/*****************************************************************************
   Device virtual addresses
*****************************************************************************/

typedef struct pvr_dev_addr {
   uint64_t addr;
} pvr_dev_addr_t;

#define PVR_DEV_ADDR(addr_) ((pvr_dev_addr_t){ .addr = (addr_) })
#define PVR_DEV_ADDR_OFFSET(base, offset) PVR_DEV_ADDR((base).addr + (offset))
#define PVR_DEV_ADDR_INVALID PVR_DEV_ADDR(0)

/* All currently supported devices use a 40-bit virtual address space. */
#define PVR_DEV_ADDR_FMT "0x%010" PRIx64

#endif /* PVR_TYPES_H */
