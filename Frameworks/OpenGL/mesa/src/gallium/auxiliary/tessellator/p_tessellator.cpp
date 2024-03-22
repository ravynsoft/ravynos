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

#include "util/u_math.h"
#include "util/u_memory.h"
#include "pipe/p_defines.h"
#include "p_tessellator.h"
#include "tessellator.hpp"

#include <new>

namespace pipe_tessellator_wrap
{
   /// Wrapper class for the CHWTessellator reference tessellator from MSFT
   /// This class will store data not originally stored in CHWTessellator
   class pipe_ts : private CHWTessellator
   {
   private:
      typedef CHWTessellator SUPER;
      enum mesa_prim    prim_mode;
      alignas(32) float      domain_points_u[MAX_POINT_COUNT];
      alignas(32) float      domain_points_v[MAX_POINT_COUNT];
      uint32_t               num_domain_points;

   public:
      void Init(enum mesa_prim tes_prim_mode,
                enum pipe_tess_spacing ts_spacing,
                bool tes_vertex_order_cw, bool tes_point_mode)
      {
         static PIPE_TESSELLATOR_PARTITIONING CVT_TS_D3D_PARTITIONING[] = {
                                                                            PIPE_TESSELLATOR_PARTITIONING_FRACTIONAL_ODD,  // PIPE_TESS_SPACING_ODD
                                                                            PIPE_TESSELLATOR_PARTITIONING_FRACTIONAL_EVEN, // PIPE_TESS_SPACING_EVEN
                                                                            PIPE_TESSELLATOR_PARTITIONING_INTEGER,         // PIPE_TESS_SPACING_EQUAL
         };

         PIPE_TESSELLATOR_OUTPUT_PRIMITIVE out_prim;
         if (tes_point_mode)
            out_prim = PIPE_TESSELLATOR_OUTPUT_POINT;
         else if (tes_prim_mode == MESA_PRIM_LINES)
            out_prim = PIPE_TESSELLATOR_OUTPUT_LINE;
         else if (tes_vertex_order_cw)
            out_prim = PIPE_TESSELLATOR_OUTPUT_TRIANGLE_CW;
         else
            out_prim = PIPE_TESSELLATOR_OUTPUT_TRIANGLE_CCW;

         SUPER::Init(CVT_TS_D3D_PARTITIONING[ts_spacing],
                     out_prim);

         prim_mode          = tes_prim_mode;
         num_domain_points = 0;
      }

      void Tessellate(const struct pipe_tessellation_factors *tess_factors,
                      struct pipe_tessellator_data *tess_data)
      {
         switch (prim_mode)
            {
            case MESA_PRIM_QUADS:
               SUPER::TessellateQuadDomain(
                                           tess_factors->outer_tf[0],
                                           tess_factors->outer_tf[1],
                                           tess_factors->outer_tf[2],
                                           tess_factors->outer_tf[3],
                                           tess_factors->inner_tf[0],
                                           tess_factors->inner_tf[1]);
               break;

            case MESA_PRIM_TRIANGLES:
               SUPER::TessellateTriDomain(
                                          tess_factors->outer_tf[0],
                                          tess_factors->outer_tf[1],
                                          tess_factors->outer_tf[2],
                                          tess_factors->inner_tf[0]);
               break;

            case MESA_PRIM_LINES:
               SUPER::TessellateIsoLineDomain(
                                              tess_factors->outer_tf[0],
                                              tess_factors->outer_tf[1]);
               break;

            default:
               assert(0);
               return;
            }

         num_domain_points = (uint32_t)SUPER::GetPointCount();

         DOMAIN_POINT *points = SUPER::GetPoints();
         for (uint32_t i = 0; i < num_domain_points; i++) {
            domain_points_u[i] = points[i].u;
            domain_points_v[i] = points[i].v;
         }
         tess_data->num_domain_points = num_domain_points;
         tess_data->domain_points_u = &domain_points_u[0];
         tess_data->domain_points_v = &domain_points_v[0];

         tess_data->num_indices = (uint32_t)SUPER::GetIndexCount();

         tess_data->indices = (uint32_t*)SUPER::GetIndices();
      }
   };
} // namespace Tessellator

/* allocate tessellator */
struct pipe_tessellator *
p_tess_init(enum mesa_prim tes_prim_mode,
            enum pipe_tess_spacing spacing,
            bool tes_vertex_order_cw, bool tes_point_mode)
{
   void *mem;
   using pipe_tessellator_wrap::pipe_ts;

   mem = align_malloc(sizeof(pipe_ts), 256);

   pipe_ts* tessellator = new (mem) pipe_ts();

   tessellator->Init(tes_prim_mode, spacing, tes_vertex_order_cw, tes_point_mode);

   return (struct pipe_tessellator *)tessellator;
}

/* destroy tessellator */
void p_tess_destroy(struct pipe_tessellator *pipe_tess)
{
   using pipe_tessellator_wrap::pipe_ts;
   pipe_ts *tessellator = (pipe_ts*)pipe_tess;

   tessellator->~pipe_ts();
   align_free(tessellator);
}

/* perform tessellation */
void p_tessellate(struct pipe_tessellator *pipe_tess,
                  const struct pipe_tessellation_factors *tess_factors,
                  struct pipe_tessellator_data *tess_data)
{
   using pipe_tessellator_wrap::pipe_ts;
   pipe_ts *tessellator = (pipe_ts*)pipe_tess;

   tessellator->Tessellate(tess_factors, tess_data);
}

