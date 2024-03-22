/*
 * Copyright (C) 2022 Collabora, Ltd.
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

#ifndef __PAN_EARLYZS_H__
#define __PAN_EARLYZS_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Matches hardware Pixel Kill enum on Bifrost and Valhall */
enum pan_earlyzs {
   PAN_EARLYZS_FORCE_EARLY = 0,
   PAN_EARLYZS_WEAK_EARLY = 2,
   PAN_EARLYZS_FORCE_LATE = 3
};

/* Early-ZS pair. */
struct pan_earlyzs_state {
   /* Z/S test and update */
   enum pan_earlyzs update : 2;

   /* Pixel kill */
   enum pan_earlyzs kill : 2;

   /* So it fits in a byte */
   unsigned padding : 4;
};

/* Internal lookup table. Users should treat as an opaque structure and only
 * access through pan_earlyzs_get and pan_earlyzs_analyze. See pan_earlyzs_get
 * for definition of the arrays.
 */
struct pan_earlyzs_lut {
   struct pan_earlyzs_state states[2][2][2];
};

/*
 * Look up early-ZS state. This is in the draw hot path on Valhall, so this is
 * defined inline in the header.
 */
static inline struct pan_earlyzs_state
pan_earlyzs_get(struct pan_earlyzs_lut lut, bool writes_zs_or_oq,
                bool alpha_to_coverage, bool zs_always_passes)
{
   return lut.states[writes_zs_or_oq][alpha_to_coverage][zs_always_passes];
}

struct pan_shader_info;

struct pan_earlyzs_lut pan_earlyzs_analyze(const struct pan_shader_info *s);

#ifdef __cplusplus
} /* extern C */
#endif

#endif
