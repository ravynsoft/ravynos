/**************************************************************************
 *
 * Copyright © 2010 Jakob Bornecrantz
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include "i915_context.h"
#include "i915_reg.h"
#include "i915_state.h"

/***********************************************************************
 */
static void
update_hw_constants(struct i915_context *i915)
{
   i915->hardware_dirty |= I915_HW_CONSTANTS;
}

struct i915_tracked_state i915_hw_constants = {
   "hw_constants", update_hw_constants, I915_NEW_FS_CONSTANTS | I915_NEW_FS};

/***********************************************************************
 */
static void
update_fs(struct i915_context *i915)
{
   i915->hardware_dirty |= I915_HW_PROGRAM;
}

struct i915_tracked_state i915_hw_fs = {"fs", update_fs,
                                        I915_NEW_FS | I915_NEW_COLOR_SWIZZLE};
