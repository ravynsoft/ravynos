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

#ifndef PVR_DEBUG_H
#define PVR_DEBUG_H

#include <stdint.h>

#include "util/macros.h"

extern uint32_t PVR_DEBUG;

/* clang-format off */
#define PVR_IS_DEBUG_SET(x) unlikely(PVR_DEBUG & PVR_DEBUG_##x)
/* clang-format on */

#define PVR_DEBUG_DUMP_CONTROL_STREAM BITFIELD_BIT(0)
#define PVR_DEBUG_TRACK_BOS BITFIELD_BIT(1)
#define PVR_DEBUG_VK_DUMP_DESCRIPTOR_SET_LAYOUT BITFIELD_BIT(2)
#define PVR_DEBUG_INFO BITFIELD_BIT(3)

void pvr_process_debug_variable(void);

#endif /* PVR_DEBUG_H */
