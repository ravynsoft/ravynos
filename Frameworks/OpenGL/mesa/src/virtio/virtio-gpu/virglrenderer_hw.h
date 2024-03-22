/*
 * Copyright 2020 Chromium
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef VIRGLRENDERER_HW_H
#define VIRGLRENDERER_HW_H

#include "venus_hw.h"
#include "virgl_hw.h"
#include "drm_hw.h"

#ifdef VIRGL_RENDERER_UNSTABLE_APIS
enum virgl_renderer_capset {
   VIRGL_RENDERER_CAPSET_VIRGL                   = 1,
   VIRGL_RENDERER_CAPSET_VIRGL2                  = 2,
   /* 3 is reserved for gfxstream */
   VIRGL_RENDERER_CAPSET_VENUS                   = 4,
   /* 5 is reserved for cross-domain */
   VIRGL_RENDERER_CAPSET_DRM                     = 6,
};
#endif

#endif /* VIRGLRENDERER_HW_H */
