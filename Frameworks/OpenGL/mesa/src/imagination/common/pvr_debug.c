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

#include <stdint.h>

#include "util/u_debug.h"
#include "pvr_debug.h"

uint32_t PVR_DEBUG = 0;

/* clang-format off */
static const struct debug_named_value debug_control[] = {
   { "cs", PVR_DEBUG_DUMP_CONTROL_STREAM,
     "Dump the contents of the control stream buffer on every job submit." },
   { "bo_track", PVR_DEBUG_TRACK_BOS,
     "Track all buffer objects with at least one reference." },
   { "vk_desc", PVR_DEBUG_VK_DUMP_DESCRIPTOR_SET_LAYOUT,
     "Dump descriptor set and pipeline layouts." },
   { "info", PVR_DEBUG_INFO,
     "Display information about the driver and device." },
   DEBUG_NAMED_VALUE_END
};
/* clang-format on */

DEBUG_GET_ONCE_FLAGS_OPTION(pvr_debug, "PVR_DEBUG", debug_control, 0)

#define PVR_DEBUG_SET(x) PVR_DEBUG |= (PVR_DEBUG_##x)

void pvr_process_debug_variable(void)
{
   PVR_DEBUG = debug_get_option_pvr_debug();

   /* Perform any automatic selections. For example, if one debug option
    * implies another it should be set here.
    */

   if (PVR_IS_DEBUG_SET(DUMP_CONTROL_STREAM))
      PVR_DEBUG_SET(TRACK_BOS);
}
