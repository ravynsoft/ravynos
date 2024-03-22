/*
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

#ifndef U_SPLIT_DRAW_H
#define U_SPLIT_DRAW_H

#include "pipe/p_state.h"

/**
 * For non-indexed drawing, this function helps work around hardware
 * limits on the number of verts in a single draw.
 *
 * For the given mode of primitive from info, calculate the count and
 * step in the buffer so the draw can be split into multiple draws.
 *
 * \param info      pointer to the original pipe_draw_info from draw_vbo
 * \param max_verts max number of vertices that can be handled by the hardware
 * \param count     number of vertices remaining in the draw call. It is also
 *                  used as a return parameter, containing how many vertices
 *                  should be sent in the next job to the hardware.
 * \param step      return parameter, will contain how many vertices should be
 *                  skipped from the original count on the next call to this
 *                  function (may differ from count if the primitive mode
 *                  requires the last vertices to be reused in the next draw)
 */
bool
u_split_draw(const struct pipe_draw_info *info, uint32_t max_verts,
             uint32_t *count, uint32_t *step);

#endif
