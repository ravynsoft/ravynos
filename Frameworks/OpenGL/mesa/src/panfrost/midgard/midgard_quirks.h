/*
 * Copyright (C) 2019 Collabora, Ltd.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __MDG_QUIRKS_H
#define __MDG_QUIRKS_H

/* Model-specific quirks requiring compiler workarounds/etc. Quirks
 * may be errata requiring a workaround, or features. We're trying to be
 * quirk-positive here; quirky is the best! */

/* Typed loads are broken on this Midgard GPU due to issue #10607 and #10632 and
 * should use software unpacks instead.
 */
#define MIDGARD_BROKEN_BLEND_LOADS (1 << 0)

/* Whether output texture registers (normally r28/r29) overlap with work
 * registers r0/r1 and input texture registers (also normally r28/r29) overlap
 * with load/store registers r26/r27. This constrains register allocation
 * considerably but is a space-saving measure on small Midgards. It's worth
 * noting if you try to access r28/r29, it may still work, but you'll mess up
 * the interference. Corresponds to BASE_HW_FEATURE_INTERPIPE_REG_ALIASING in
 * kbase. */

#define MIDGARD_INTERPIPE_REG_ALIASING (1 << 1)

/* Whether we should use old-style blend opcodes */

#define MIDGARD_OLD_BLEND (1 << 2)

/* Errata causing the LOD clamps and bias in the sampler descriptor to be
 * ignored. This errata affects the command stream but uses a compiler
 * workaround (applying the clamps/bias manually in the shader. Corresponds in
 * BASE_HW_ISSUE_10471 in kbase, described as "TEXGRD doesn't honor Sampler
 * Descriptor LOD clamps nor bias". (I'm assuming TEXGRD is what we call
 * textureLod) */

#define MIDGARD_BROKEN_LOD (1 << 3)

/* Don't use upper ALU tags for writeout (if you do, you'll get a
 * INSTR_INVALID_ENC). It's not clear to me what these tags are for. */

#define MIDGARD_NO_UPPER_ALU (1 << 4)

/* Whether (texture) out-of-order execution support is missing on early
 * Midgards. For these just set the OoO bits to 0. */

#define MIDGARD_NO_OOO (1 << 5)

static inline unsigned
midgard_get_quirks(unsigned gpu_id)
{
   switch (gpu_id) {
   case 0x600:
   case 0x620:
      return MIDGARD_OLD_BLEND | MIDGARD_BROKEN_BLEND_LOADS |
             MIDGARD_BROKEN_LOD | MIDGARD_NO_UPPER_ALU | MIDGARD_NO_OOO;

   case 0x720:
      return MIDGARD_INTERPIPE_REG_ALIASING | MIDGARD_OLD_BLEND |
             MIDGARD_BROKEN_LOD | MIDGARD_NO_UPPER_ALU | MIDGARD_NO_OOO;

   case 0x820:
   case 0x830:
      return MIDGARD_INTERPIPE_REG_ALIASING;

   case 0x750:
      return MIDGARD_NO_UPPER_ALU;

   case 0x860:
   case 0x880:
      return 0;

   default:
      unreachable("Invalid Midgard GPU ID");
   }
}

#endif
