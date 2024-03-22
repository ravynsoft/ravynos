/**************************************************************************
 *
 * Copyright 2020 Red Hat.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **************************************************************************/
#ifndef PIPE_TESSELLATOR_H
#define PIPE_TESSELLATOR_H

#include "pipe/p_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

struct pipe_tessellator;
struct pipe_tessellation_factors
{
   float outer_tf[4];
   float inner_tf[2];
   float pad[2];
};

struct pipe_tessellator_data
{
   uint32_t num_indices;
   uint32_t num_domain_points;

   uint32_t *indices;
   float    *domain_points_u;
   float    *domain_points_v;
    // For Tri: domain_points_w[i] = 1.0f - domain_points_u[i] - domain_points_v[i]
};

/// Allocate and initialize a new tessellation context
struct pipe_tessellator *p_tess_init(enum mesa_prim tes_prim_mode,
                                     enum pipe_tess_spacing spacing,
                                     bool tes_vertex_order_cw, bool tes_point_mode);
/// Destroy & de-allocate tessellation context
void p_tess_destroy(struct pipe_tessellator *pipe_ts);


/// Perform Tessellation
void p_tessellate(struct pipe_tessellator *pipe_ts,
                  const struct pipe_tessellation_factors *tess_factors,
                  struct pipe_tessellator_data *tess_data);

#ifdef __cplusplus
}
#endif
#endif
