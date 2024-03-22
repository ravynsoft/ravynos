/**************************************************************************
 *
 * Copyright 2010, VMware Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/*
 * Binning code for points
 */

#include "util/u_math.h"
#include "util/u_memory.h"
#include "lp_setup_context.h"
#include "lp_perf.h"
#include "lp_rast.h"
#include "lp_state_fs.h"
#include "lp_state_setup.h"
#include "lp_context.h"
#include "draw/draw_context.h"

#define NUM_CHANNELS 4

struct point_info {
   /* x,y deltas */
   int dy01, dy12;
   int dx01, dx12;

   const float (*v0)[4];

   float (*a0)[4];
   float (*dadx)[4];
   float (*dady)[4];

   bool frontfacing;
};


/**
 * Compute a0 for a constant-valued coefficient (GL_FLAT shading).
 */
static void
constant_coef(struct lp_setup_context *setup,
              struct point_info *info,
              unsigned slot,
              const float value,
              unsigned i)
{
   info->a0[slot][i] = value;
   info->dadx[slot][i] = 0.0f;
   info->dady[slot][i] = 0.0f;
}


static void
point_persp_coeff(struct lp_setup_context *setup,
                  const struct point_info *info,
                  unsigned slot,
                  unsigned i)
{
   /*
    * Fragment shader expects pre-multiplied w for LP_INTERP_PERSPECTIVE. A
    * better strategy would be to take the primitive in consideration when
    * generating the fragment shader key, and therefore avoid the per-fragment
    * perspective divide.
    */

   float w0 = info->v0[0][3];

   assert(i < 4);

   info->a0[slot][i] = info->v0[slot][i]*w0;
   info->dadx[slot][i] = 0.0f;
   info->dady[slot][i] = 0.0f;
}


/**
 * Setup automatic texcoord coefficients (for sprite rendering).
 * \param slot  the vertex attribute slot to setup
 * \param i  the attribute channel in [0,3]
 * \param sprite_coord_origin  one of PIPE_SPRITE_COORD_x
 * \param perspective  does the shader expects pre-multiplied w, i.e.,
 *    LP_INTERP_PERSPECTIVE is specified in the shader key
 */
static void
texcoord_coef(struct lp_setup_context *setup,
              const struct point_info *info,
              unsigned slot,
              unsigned i,
              unsigned sprite_coord_origin,
              bool perspective)
{
   float w0 = info->v0[0][3];

   assert(i < 4);

   if (i == 0) {
      float dadx = FIXED_ONE / (float)info->dx12;
      float dady =  0.0f;
      float x0 = info->v0[0][0] - setup->pixel_offset;
      float y0 = info->v0[0][1] - setup->pixel_offset;

      info->dadx[slot][0] = dadx;
      info->dady[slot][0] = dady;
      info->a0[slot][0] = 0.5 - (dadx * x0 + dady * y0);

      if (perspective) {
         info->dadx[slot][0] *= w0;
         info->dady[slot][0] *= w0;
         info->a0[slot][0] *= w0;
      }
   } else if (i == 1) {
      float dadx = 0.0f;
      float dady = FIXED_ONE / (float)info->dx12;
      float x0 = info->v0[0][0] - setup->pixel_offset;
      float y0 = info->v0[0][1] - setup->pixel_offset;

      if (sprite_coord_origin == PIPE_SPRITE_COORD_LOWER_LEFT) {
         dady = -dady;
      }

      info->dadx[slot][1] = dadx;
      info->dady[slot][1] = dady;
      info->a0[slot][1] = 0.5 - (dadx * x0 + dady * y0);

      if (perspective) {
         info->dadx[slot][1] *= w0;
         info->dady[slot][1] *= w0;
         info->a0[slot][1] *= w0;
      }
   } else if (i == 2) {
      info->a0[slot][2] = 0.0f;
      info->dadx[slot][2] = 0.0f;
      info->dady[slot][2] = 0.0f;
   } else {
      info->a0[slot][3] = perspective ? w0 : 1.0f;
      info->dadx[slot][3] = 0.0f;
      info->dady[slot][3] = 0.0f;
   }
}


/**
 * Special coefficient setup for gl_FragCoord.  X and Y are trivial.  Z and W
 * are copied from position_coef which should have already been computed.  We
 * could do a bit less work if we'd examine gl_FragCoord's swizzle mask.
 */
static void
setup_point_fragcoord_coef(struct lp_setup_context *setup,
                           struct point_info *info,
                           unsigned slot,
                           unsigned usage_mask)
{
   /*X*/
   if (usage_mask & TGSI_WRITEMASK_X) {
      info->a0[slot][0] = 0.0;
      info->dadx[slot][0] = 1.0;
      info->dady[slot][0] = 0.0;
   }

   /*Y*/
   if (usage_mask & TGSI_WRITEMASK_Y) {
      info->a0[slot][1] = 0.0;
      info->dadx[slot][1] = 0.0;
      info->dady[slot][1] = 1.0;
   }

   /*Z*/
   if (usage_mask & TGSI_WRITEMASK_Z) {
      constant_coef(setup, info, slot, info->v0[0][2], 2);
   }

   /*W*/
   if (usage_mask & TGSI_WRITEMASK_W) {
      constant_coef(setup, info, slot, info->v0[0][3], 3);
   }
}


/**
 * Compute the point->coef[] array dadx, dady, a0 values.
 */
static void
setup_point_coefficients(struct lp_setup_context *setup,
                         struct point_info *info)
{
   const struct lp_setup_variant_key *key = &setup->setup.variant->key;
   const struct lp_fragment_shader *shader = setup->fs.current.variant->shader;
   unsigned fragcoord_usage_mask = TGSI_WRITEMASK_XYZ;

   /* setup interpolation for all the remaining attributes:
    */
   for (unsigned slot = 0; slot < key->num_inputs; slot++) {
      unsigned vert_attr = key->inputs[slot].src_index;
      unsigned usage_mask = key->inputs[slot].usage_mask;
      enum lp_interp interp = key->inputs[slot].interp;
      bool perspective = !!(interp == LP_INTERP_PERSPECTIVE);
      unsigned i;

      if (perspective && usage_mask) {
         fragcoord_usage_mask |= TGSI_WRITEMASK_W;
      }

      switch (interp) {
      case LP_INTERP_POSITION:
         /*
          * The generated pixel interpolators will pick up the coeffs from
          * slot 0, so all need to ensure that the usage mask is covers all
          * usages.
          */
         fragcoord_usage_mask |= usage_mask;
         break;
      case LP_INTERP_LINEAR:
         /* Sprite tex coords may use linear interpolation someday */
         FALLTHROUGH;
      case LP_INTERP_PERSPECTIVE: {
         /* check if the sprite coord flag is set for this attribute.
          * If so, set it up so it up so x and y vary from 0 to 1.
          */
         bool do_texcoord_coef = false;
         if (shader->info.base.input_semantic_name[slot] ==
             TGSI_SEMANTIC_PCOORD) {
            do_texcoord_coef = true;
         } else if (shader->info.base.input_semantic_name[slot] ==
                  TGSI_SEMANTIC_TEXCOORD) {
            unsigned semantic_index =
               shader->info.base.input_semantic_index[slot];
            /* Note that sprite_coord enable is a bitfield of
             * PIPE_MAX_SHADER_OUTPUTS bits.
             */
            if (semantic_index < PIPE_MAX_SHADER_OUTPUTS &&
                (setup->sprite_coord_enable & (1u << semantic_index))) {
               do_texcoord_coef = true;
            }
         }
         if (do_texcoord_coef) {
            for (i = 0; i < NUM_CHANNELS; i++) {
               if (usage_mask & (1 << i)) {
                  texcoord_coef(setup, info, slot + 1, i,
                                setup->sprite_coord_origin,
                                perspective);
               }
            }
            break;
         }
      }
         FALLTHROUGH;
      case LP_INTERP_CONSTANT:
         for (i = 0; i < NUM_CHANNELS; i++) {
            if (usage_mask & (1 << i)) {
               if (perspective) {
                  point_persp_coeff(setup, info, slot+1, i);
               } else {
                  constant_coef(setup, info, slot+1, info->v0[vert_attr][i], i);
               }
            }
         }
         break;
      case LP_INTERP_FACING:
         for (i = 0; i < NUM_CHANNELS; i++)
            if (usage_mask & (1 << i))
               constant_coef(setup, info, slot+1,
                             info->frontfacing ? 1.0f : -1.0f, i);
         break;
      default:
         assert(0);
         break;
      }
   }

   /* The internal position input is in slot zero:
    */
   setup_point_fragcoord_coef(setup, info, 0,
                              fragcoord_usage_mask);
}


static inline int
subpixel_snap(float a)
{
   return util_iround(FIXED_ONE * a);
}


/**
 * Print point vertex attribs (for debug).
 */
static void
print_point(struct lp_setup_context *setup,
            const float (*v0)[4],
            const float size)
{
   const struct lp_setup_variant_key *key = &setup->setup.variant->key;

   debug_printf("llvmpipe point, width %f\n", size);
   for (unsigned i = 0; i < 1 + key->num_inputs; i++) {
      debug_printf("  v0[%d]:  %f %f %f %f\n", i,
                   v0[i][0], v0[i][1], v0[i][2], v0[i][3]);
   }
}


static bool
try_setup_point(struct lp_setup_context *setup,
                const float (*v0)[4])
{
   struct llvmpipe_context *lp_context = (struct llvmpipe_context *)setup->pipe;
   /* x/y positions in fixed point */
   const struct lp_setup_variant_key *key = &setup->setup.variant->key;
   const int sizeAttr = setup->psize_slot;
   float size
      = (setup->point_size_per_vertex && sizeAttr > 0) ? v0[sizeAttr][0]
      : setup->point_size;

   if (size > LP_MAX_POINT_WIDTH)
      size = LP_MAX_POINT_WIDTH;

   /* Yes this is necessary to accurately calculate bounding boxes
    * with the two fill-conventions we support.  GL (normally) ends
    * up needing a bottom-left fill convention, which requires
    * slightly different rounding.
    */
   const int adj = (setup->bottom_edge_rule != 0) ? 1 : 0;
   const float pixel_offset = setup->multisample ? 0.0 : setup->pixel_offset;
   struct lp_scene *scene = setup->scene;
   int x[2], y[2];

   unsigned viewport_index = 0;
   if (setup->viewport_index_slot > 0) {
      unsigned *udata = (unsigned*)v0[setup->viewport_index_slot];
      viewport_index = lp_clamp_viewport_idx(*udata);
   }

   unsigned layer = 0;
   if (setup->layer_slot > 0) {
      layer = *(unsigned*)v0[setup->layer_slot];
      layer = MIN2(layer, scene->fb_max_layer);
   }

   int fixed_width;

   if (0)
      print_point(setup, v0, size);

   /* Bounding rectangle (in pixels) */
   struct u_rect bbox;
   if (!setup->legacy_points) {
      /*
       * Rasterize points as quads.
       */
      int x0, y0;
      /* Point size as fixed point integer, remove rounding errors
       * and gives minimum width for very small points.
       */
      fixed_width = MAX2(FIXED_ONE, subpixel_snap(size));

      x0 = subpixel_snap(v0[0][0] - pixel_offset) - fixed_width/2;
      y0 = subpixel_snap(v0[0][1] - pixel_offset) - fixed_width/2;

      x[0] = x0;
      x[1] = x0 + fixed_width;
      y[0] = y0;
      y[1] = y0 + fixed_width;
      bbox.x0 = x[0] >> FIXED_ORDER;
      bbox.x1 = (x[1] + (FIXED_ONE-1)) >> FIXED_ORDER;
      bbox.y0 = (y[0] + adj) >> FIXED_ORDER;
      bbox.y1 = (y[1] + (FIXED_ONE-1) + adj) >> FIXED_ORDER;

      /* Inclusive coordinates:
       */
      bbox.x1--;
      bbox.y1--;
   } else {
      /*
       * OpenGL legacy rasterization rules for non-sprite points.
       *
       * Per OpenGL 2.1 spec, section 3.3.1, "Basic Point Rasterization".
       *
       * This type of point rasterization is only available in pre 3.0 contexts
       * (or compatibility contexts which we don't support) anyway.
       */

      const int x0 = subpixel_snap(v0[0][0]);
      const int y0 = subpixel_snap(v0[0][1]) - adj;

      /* Point size as fixed point integer. For GL legacy points
       * the point size is always a whole integer.
       */
      fixed_width = MAX2(FIXED_ONE,
                         (subpixel_snap(size) + FIXED_ONE/2 - 1) & ~(FIXED_ONE-1));
      int int_width = fixed_width >> FIXED_ORDER;

      assert(setup->pixel_offset != 0);

      if (int_width == 1) {
         bbox.x0 = x0 >> FIXED_ORDER;
         bbox.y0 = y0 >> FIXED_ORDER;
         bbox.x1 = bbox.x0;
         bbox.y1 = bbox.y0;
      } else {
         if (int_width & 1) {
            /* Odd width */
            bbox.x0 = (x0 >> FIXED_ORDER) - (int_width - 1)/2;
            bbox.y0 = (y0 >> FIXED_ORDER) - (int_width - 1)/2;
         } else {
            /* Even width */
            bbox.x0 = ((x0 + FIXED_ONE/2) >> FIXED_ORDER) - int_width/2;
            bbox.y0 = ((y0 + FIXED_ONE/2) >> FIXED_ORDER) - int_width/2;
         }

         bbox.x1 = bbox.x0 + int_width - 1;
         bbox.y1 = bbox.y0 + int_width - 1;
      }

      x[0] = (bbox.x0 - 1) << 8;
      x[1] = (bbox.x1 + 1) << 8;
      y[0] = (bbox.y0 - 1) << 8;
      y[1] = (bbox.y1 + 1) << 8;
   }

   if (0) {
      debug_printf("  bbox: (%i, %i) - (%i, %i)\n",
                   bbox.x0, bbox.y0,
                   bbox.x1, bbox.y1);
   }

   if (lp_context->active_statistics_queries) {
      lp_context->pipeline_statistics.c_primitives++;
   }

   if (lp_setup_zero_sample_mask(setup)) {
      if (0) debug_printf("zero sample mask\n");
      LP_COUNT(nr_culled_tris);
      return true;
   }

   if (!u_rect_test_intersection(&setup->draw_regions[viewport_index], &bbox)) {
      if (0) debug_printf("no intersection\n");
      LP_COUNT(nr_culled_tris);
      return true;
   }

   u_rect_find_intersection(&setup->draw_regions[viewport_index], &bbox);

   /* We can't use rectangle reasterizer for non-legacy points for now. */
   if (!setup->legacy_points || setup->multisample) {
      struct lp_rast_triangle *point;
      struct lp_rast_plane *plane;
      unsigned nr_planes = 4;

      point = lp_setup_alloc_triangle(scene,
                                      key->num_inputs,
                                      nr_planes);
     if (!point)
        return false;

#ifdef DEBUG
      point->v[0][0] = v0[0][0];
      point->v[0][1] = v0[0][1];
#endif

      LP_COUNT(nr_tris);

      if (draw_will_inject_frontface(lp_context->draw) &&
          setup->face_slot > 0) {
         point->inputs.frontfacing = v0[setup->face_slot][0];
      } else {
         point->inputs.frontfacing = true;
      }

      struct point_info info;
      info.v0 = v0;
      info.dx01 = 0;
      info.dx12 = fixed_width;
      info.dy01 = fixed_width;
      info.dy12 = 0;
      info.a0 = GET_A0(&point->inputs);
      info.dadx = GET_DADX(&point->inputs);
      info.dady = GET_DADY(&point->inputs);
      info.frontfacing = point->inputs.frontfacing;

      /* Setup parameter interpolants:
       */
      setup_point_coefficients(setup, &info);

      point->inputs.disable = false;
      point->inputs.is_blit = false;
      point->inputs.layer = layer;
      point->inputs.viewport_index = viewport_index;
      point->inputs.view_index = setup->view_index;

      plane = GET_PLANES(point);

      plane[0].dcdx = ~0U << 8;
      plane[0].dcdy = 0;
      plane[0].c = -MAX2(x[0], bbox.x0 << 8);
      plane[0].eo = 1 << 8;

      plane[1].dcdx = 1 << 8;
      plane[1].dcdy = 0;
      plane[1].c = MIN2(x[1], (bbox.x1 + 1) << 8);
      plane[1].eo = 0;

      plane[2].dcdx = 0;
      plane[2].dcdy = 1 << 8;
      plane[2].c = -MAX2(y[0], (bbox.y0 << 8) - adj);
      plane[2].eo = 1 << 8;

      plane[3].dcdx = 0;
      plane[3].dcdy = ~0U << 8;
      plane[3].c = MIN2(y[1], (bbox.y1 + 1) << 8);
      plane[3].eo = 0;

      if (!setup->legacy_points) {
         /* adjust for fill-rule*/
         plane[0].c++; /* left */
         if (setup->bottom_edge_rule == 0)
            plane[2].c++; /* top-left */
         else
            plane[3].c++; /* bottom-left */
      }

      int max_szorig = ((bbox.x1 - (bbox.x0 & ~3)) |
                        (bbox.y1 - (bbox.y0 & ~3)));
      bool use_32bits = max_szorig <= MAX_FIXED_LENGTH32;

      return lp_setup_bin_triangle(setup, point, use_32bits,
                                   setup->fs.current.variant->opaque,
                                   &bbox, nr_planes, viewport_index);

   } else {
      struct lp_rast_rectangle *point =
         lp_setup_alloc_rectangle(scene, key->num_inputs);
      if (!point)
         return false;
#ifdef DEBUG
      point->v[0][0] = v0[0][0];
      point->v[0][1] = v0[0][1];
#endif

      point->box.x0 = bbox.x0;
      point->box.x1 = bbox.x1;
      point->box.y0 = bbox.y0;
      point->box.y1 = bbox.y1;

      LP_COUNT(nr_tris);

      if (draw_will_inject_frontface(lp_context->draw) &&
          setup->face_slot > 0) {
         point->inputs.frontfacing = v0[setup->face_slot][0];
      } else {
         point->inputs.frontfacing = true;
      }

      struct point_info info;
      info.v0 = v0;
      info.dx01 = 0;
      info.dx12 = fixed_width;
      info.dy01 = fixed_width;
      info.dy12 = 0;
      info.a0 = GET_A0(&point->inputs);
      info.dadx = GET_DADX(&point->inputs);
      info.dady = GET_DADY(&point->inputs);
      info.frontfacing = point->inputs.frontfacing;

      /* Setup parameter interpolants:
       */
      setup_point_coefficients(setup, &info);

      point->inputs.disable = false;
      point->inputs.is_blit = false;
      point->inputs.layer = layer;
      point->inputs.viewport_index = viewport_index;
      point->inputs.view_index = setup->view_index;

      return lp_setup_bin_rectangle(setup, point,
                                    setup->fs.current.variant->opaque);
   }
}


static void
lp_setup_point_discard(struct lp_setup_context *setup,
                       const float (*v0)[4])
{
}


static void
lp_setup_point(struct lp_setup_context *setup,
               const float (*v0)[4])
{
   if (!try_setup_point(setup, v0)) {
      if (!lp_setup_flush_and_restart(setup))
         return;

      if (!try_setup_point(setup, v0))
         return;
   }
}


void
lp_setup_choose_point(struct lp_setup_context *setup)
{
   if (setup->rasterizer_discard) {
      setup->point = lp_setup_point_discard;
   } else {
      setup->point = lp_setup_point;
   }
}


