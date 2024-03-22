/*
 * Copyright © 2022 Intel Corporation
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

#ifndef GENX_RT_PACK_H
#define GENX_RT_PACK_H

#ifndef GFX_VERx10
#  error "The GFX_VERx10 macro must be defined"
#endif

#if (GFX_VERx10 == 40)
/* No RT support for this gfx ver */
#elif (GFX_VERx10 == 45)
/* No RT support for this gfx ver */
#elif (GFX_VERx10 == 50)
/* No RT support for this gfx ver */
#elif (GFX_VERx10 == 60)
/* No RT support for this gfx ver */
#elif (GFX_VERx10 == 70)
/* No RT support for this gfx ver */
#elif (GFX_VERx10 == 75)
/* No RT support for this gfx ver */
#elif (GFX_VERx10 == 80)
/* No RT support for this gfx ver */
#elif (GFX_VERx10 == 90)
/* No RT support for this gfx ver */
#elif (GFX_VERx10 == 110)
/* No RT support for this gfx ver */
#elif (GFX_VERx10 == 120)
/* No RT support for this gfx ver */
#elif (GFX_VERx10 == 125)
#  include "genxml/gen125_rt_pack.h"
#elif (GFX_VERx10 == 200)
#  include "genxml/gen20_rt_pack.h"
#else
#  error "Need to add a pack header include for this gen"
#endif

#endif /* GENX_RT_PACK_H */
