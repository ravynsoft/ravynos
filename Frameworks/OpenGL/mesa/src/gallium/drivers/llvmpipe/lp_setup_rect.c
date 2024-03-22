/**************************************************************************
 *
 * Copyright 2010-2021 VMware, Inc.
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
 * IN NO EVENT SHALL THE AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/**
 * Setup/binning code for screen-aligned quads.
 */

#include "util/u_math.h"
#include "util/u_memory.h"
#include "lp_perf.h"
#include "lp_setup_context.h"
#include "lp_rast.h"
#include "lp_state_fs.h"
#include "lp_state_setup.h"


#define NUM_CHANNELS 4

#define UNDETERMINED_BLIT  -1


static inline int
subpixel_snap(float a)
{
   return util_iround(FIXED_ONE * a);
}


/**
 * Alloc space for a new rectangle plus the input.a0/dadx/dady arrays
 * immediately after it.
 * The memory is allocated from the per-scene pool, not per-tile.
 * \param size  returns number of bytes allocated
 * \param nr_inputs  number of fragment shader inputs
 * \return pointer to rectangle space
 */
struct lp_rast_rectangle *
lp_setup_alloc_rectangle(struct lp_scene *scene, unsigned nr_inputs)
{
   unsigned input_array_sz = NUM_CHANNELS * (nr_inputs + 1) * sizeof(float);
   struct lp_rast_rectangle *rect;
   unsigned bytes = sizeof(*rect) + (3 * input_array_sz);
   rect = lp_scene_alloc_aligned(scene, bytes, 16);
   if (rect == NULL)
      return NULL;

   rect->inputs.stride = input_array_sz;

   return rect;
}


/**
 * The rectangle covers the whole tile- shade whole tile.
 * XXX no rectangle/triangle dependencies in this file - share it with
 * the same code in lp_setup_tri.c
 * \param tx, ty  the tile position in tiles, not pixels
 */
bool
lp_setup_whole_tile(struct lp_setup_context *setup,
                    const struct lp_rast_shader_inputs *inputs,
                    int tx, int ty, bool opaque)
{
   struct lp_scene *scene = setup->scene;

   LP_COUNT(nr_fully_covered_64);

   /* if variant is opaque and scissor doesn't effect the tile */
   if (opaque) {
      /* Several things prevent this optimization from working:
       * - For layered rendering we can't determine if this covers the same
       * layer as previous rendering (or in case of clears those actually
       * always cover all layers so optimization is impossible). Need to use
       * fb_max_layer and not setup->layer_slot to determine this since even
       * if there's currently no slot assigned previous rendering could have
       * used one.
       * - If there were any Begin/End query commands in the scene then those
       * would get removed which would be very wrong. Furthermore, if queries
       * were just active we also can't do the optimization since to get
       * accurate query results we unfortunately need to execute the rendering
       * commands.
       */
      if (!scene->fb.zsbuf && scene->fb_max_layer == 0 &&
          !scene->had_queries) {
         /*
          * All previous rendering will be overwritten so reset the bin.
          */
         lp_scene_bin_reset(scene, tx, ty);
      }

      if (inputs->is_blit) {
         LP_COUNT(nr_blit_64);
         return lp_scene_bin_cmd_with_state(scene, tx, ty,
                                            setup->fs.stored,
                                            LP_RAST_OP_BLIT,
                                            lp_rast_arg_inputs(inputs));
      } else {
         LP_COUNT(nr_shade_opaque_64);
         return lp_scene_bin_cmd_with_state(scene, tx, ty,
                                            setup->fs.stored,
                                            LP_RAST_OP_SHADE_TILE_OPAQUE,
                                            lp_rast_arg_inputs(inputs));
      }
   } else {
      LP_COUNT(nr_shade_64);
      return lp_scene_bin_cmd_with_state(scene, tx, ty,
                                         setup->fs.stored,
                                         LP_RAST_OP_SHADE_TILE,
                                         lp_rast_arg_inputs(inputs));
   }
}


bool
lp_setup_is_blit(const struct lp_setup_context *setup,
                 const struct lp_rast_shader_inputs *inputs)
{
   const struct lp_fragment_shader_variant *variant =
      setup->fs.current.variant;

   if (variant->blit) {
      /*
       * Detect blits.
       */
      const struct lp_jit_texture *texture =
         &setup->fs.current.jit_resources.textures[0];

      /* XXX: dadx vs dady confusion below?
       */
      const float dsdx = GET_DADX(inputs)[1][0] * texture->width;
      const float dsdy = GET_DADX(inputs)[1][1] * texture->width;
      const float dtdx = GET_DADY(inputs)[1][0] * texture->height;
      const float dtdy = GET_DADY(inputs)[1][1] * texture->height;

      /*
       * We don't need to check s0/t0 tolerances
       * as we establish as pre-condition that there is no
       * texture filtering.
       */

      ASSERTED struct lp_sampler_static_state *samp0 = lp_fs_variant_key_sampler_idx(&variant->key, 0);
      assert(samp0);
      assert(samp0->sampler_state.min_img_filter == PIPE_TEX_FILTER_NEAREST);
      assert(samp0->sampler_state.mag_img_filter == PIPE_TEX_FILTER_NEAREST);

      /*
       * Check for 1:1 match of texels to dest pixels
       */

      if (util_is_approx(dsdx, 1.0f, 1.0f/LP_MAX_WIDTH) &&
          util_is_approx(dsdy, 0.0f, 1.0f/LP_MAX_HEIGHT) &&
          util_is_approx(dtdx, 0.0f, 1.0f/LP_MAX_WIDTH) &&
          util_is_approx(dtdy, 1.0f, 1.0f/LP_MAX_HEIGHT)) {
         return true;
      } else {
#if 0
         debug_printf("dsdx = %f\n", dsdx);
         debug_printf("dsdy = %f\n", dsdy);
         debug_printf("dtdx = %f\n", dtdx);
         debug_printf("dtdy = %f\n", dtdy);
         debug_printf("\n");
#endif
         return false;
      }
   }

   return false;
}


static inline void
partial(struct lp_setup_context *setup,
        const struct lp_rast_rectangle *rect,
        bool opaque,
        unsigned ix, unsigned iy,
        unsigned mask) // RECT_PLANE_x bits
{
   if (mask == 0) {
      assert(rect->box.x0 <= ix * TILE_SIZE);
      assert(rect->box.y0 <= iy * TILE_SIZE);
      assert(rect->box.x1 >= (ix+1) * TILE_SIZE - 1);
      assert(rect->box.y1 >= (iy+1) * TILE_SIZE - 1);

      lp_setup_whole_tile(setup, &rect->inputs, ix, iy, opaque);
   } else {
      LP_COUNT(nr_partially_covered_64);
      lp_scene_bin_cmd_with_state(setup->scene,
                                  ix, iy,
                                  setup->fs.stored,
                                  LP_RAST_OP_RECTANGLE,
                                  lp_rast_arg_rectangle(rect));
   }
}


/**
 * Setup/bin a screen-aligned rect.
 * We need three corner vertices in order to correctly setup
 * interpolated parameters.  We *could* get away with just the
 * diagonal vertices but it'd cause ugliness elsewhere.
 *
 *   + -------v0
 *   |        |
 *  v2 ------ v1
 *
 * By an unfortunate mixup between GL and D3D coordinate spaces, half
 * of this file talks about clockwise rectangles (which were CCW in GL
 * coordinate space), while the other half prefers to work with D3D
 * CCW rectangles.
 */
static bool
try_rect_cw(struct lp_setup_context *setup,
            const float (*v0)[4],
            const float (*v1)[4],
            const float (*v2)[4],
            bool frontfacing)
{
   const struct lp_fragment_shader_variant *variant =
      setup->fs.current.variant;
   const struct lp_setup_variant_key *key = &setup->setup.variant->key;
   struct lp_scene *scene = setup->scene;

   /* x/y positions in fixed point */
   int x0 = subpixel_snap(v0[0][0] - setup->pixel_offset);
   int x1 = subpixel_snap(v1[0][0] - setup->pixel_offset);
   int x2 = subpixel_snap(v2[0][0] - setup->pixel_offset);
   int y0 = subpixel_snap(v0[0][1] - setup->pixel_offset);
   int y1 = subpixel_snap(v1[0][1] - setup->pixel_offset);
   int y2 = subpixel_snap(v2[0][1] - setup->pixel_offset);

   LP_COUNT(nr_rects);

   /* Cull clockwise rects without overflowing.
    */
   const bool cw = (x2 < x1) ^ (y0 < y2);
   if (cw) {
      LP_COUNT(nr_culled_rects);
      return true;
   }

   const float (*pv)[4];
   if (setup->flatshade_first) {
      pv = v0;
   } else {
      pv = v2;
   }

   unsigned viewport_index = 0;
   if (setup->viewport_index_slot > 0) {
      unsigned *udata = (unsigned*)pv[setup->viewport_index_slot];
      viewport_index = lp_clamp_viewport_idx(*udata);
   }

   unsigned layer = 0;
   if (setup->layer_slot > 0) {
      layer = *(unsigned*)pv[setup->layer_slot];
      layer = MIN2(layer, scene->fb_max_layer);
   }

   /* Bounding rectangle (in pixels) */
   struct u_rect bbox;
   {
      /* Yes this is necessary to accurately calculate bounding boxes
       * with the two fill-conventions we support.  GL (normally) ends
       * up needing a bottom-left fill convention, which requires
       * slightly different rounding.
       */
      int adj = (setup->bottom_edge_rule != 0) ? 1 : 0;

      bbox.x0 = (MIN3(x0, x1, x2) + (FIXED_ONE-1)) >> FIXED_ORDER;
      bbox.x1 = (MAX3(x0, x1, x2) + (FIXED_ONE-1)) >> FIXED_ORDER;
      bbox.y0 = (MIN3(y0, y1, y2) + (FIXED_ONE-1) + adj) >> FIXED_ORDER;
      bbox.y1 = (MAX3(y0, y1, y2) + (FIXED_ONE-1) + adj) >> FIXED_ORDER;

      /* Inclusive coordinates:
       */
      bbox.x1--;
      bbox.y1--;
   }

   if (!u_rect_test_intersection(&setup->draw_regions[viewport_index], &bbox)) {
      if (0) debug_printf("no intersection\n");
      LP_COUNT(nr_culled_rects);
      return true;
   }

   u_rect_find_intersection(&setup->draw_regions[viewport_index], &bbox);

   struct lp_rast_rectangle *rect =
      lp_setup_alloc_rectangle(scene, key->num_inputs);
   if (!rect)
      return false;

#ifdef DEBUG
   rect->v[0][0] = v0[0][0];
   rect->v[0][1] = v0[0][1];
   rect->v[1][0] = v1[0][0];
   rect->v[1][1] = v1[0][1];
#endif

   rect->box.x0 = bbox.x0;
   rect->box.x1 = bbox.x1;
   rect->box.y0 = bbox.y0;
   rect->box.y1 = bbox.y1;

   /* Setup parameter interpolants:
    */
   setup->setup.variant->jit_function(v0,
                                      v1,
                                      v2,
                                      frontfacing,
                                      GET_A0(&rect->inputs),
                                      GET_DADX(&rect->inputs),
                                      GET_DADY(&rect->inputs),
                                      &setup->setup.variant->key);

   rect->inputs.frontfacing = frontfacing;
   rect->inputs.disable = false;
   rect->inputs.is_blit = lp_setup_is_blit(setup, &rect->inputs);
   rect->inputs.layer = layer;
   rect->inputs.viewport_index = viewport_index;
   rect->inputs.view_index = setup->view_index;

   return lp_setup_bin_rectangle(setup, rect, variant->opaque);
}


bool
lp_setup_bin_rectangle(struct lp_setup_context *setup,
                       struct lp_rast_rectangle *rect,
                       bool opaque)
{
   struct lp_scene *scene = setup->scene;
   unsigned left_mask = 0;
   unsigned right_mask = 0;
   unsigned top_mask = 0;
   unsigned bottom_mask = 0;

   /*
    * All fields of 'rect' are now set.  The remaining code here is
    * concerned with binning.
    */

   /* Convert to inclusive tile coordinates:
    */
   const unsigned ix0 = rect->box.x0 / TILE_SIZE;
   const unsigned iy0 = rect->box.y0 / TILE_SIZE;
   const unsigned ix1 = rect->box.x1 / TILE_SIZE;
   const unsigned iy1 = rect->box.y1 / TILE_SIZE;

   /*
    * Clamp to framebuffer size
    */
   assert(ix0 == MAX2(ix0, 0));
   assert(iy0 == MAX2(iy0, 0));
   assert(ix1 == MIN2(ix1, scene->tiles_x - 1));
   assert(iy1 == MIN2(iy1, scene->tiles_y - 1));

   if (ix0 * TILE_SIZE != rect->box.x0)
      left_mask = RECT_PLANE_LEFT;

   if (ix1 * TILE_SIZE + TILE_SIZE - 1 != rect->box.x1)
      right_mask  = RECT_PLANE_RIGHT;

   if (iy0 * TILE_SIZE != rect->box.y0)
      top_mask    = RECT_PLANE_TOP;

   if (iy1 * TILE_SIZE + TILE_SIZE - 1 != rect->box.y1)
      bottom_mask = RECT_PLANE_BOTTOM;

   /* Determine which tile(s) intersect the rectangle's bounding box
    */
   if (iy0 == iy1 && ix0 == ix1) {
      partial(setup, rect, opaque, ix0, iy0,
              (left_mask | right_mask | top_mask | bottom_mask));
   } else if (ix0 == ix1) {
      unsigned mask = left_mask | right_mask;
      partial(setup, rect, opaque, ix0, iy0, mask | top_mask);
      for (unsigned i = iy0 + 1; i < iy1; i++)
         partial(setup, rect, opaque, ix0, i, mask);
      partial(setup, rect, opaque, ix0, iy1, mask | bottom_mask);
   } else if (iy0 == iy1) {
      unsigned mask = top_mask | bottom_mask;
      partial(setup, rect, opaque, ix0, iy0, mask | left_mask);
      for (unsigned i = ix0 + 1; i < ix1; i++)
         partial(setup, rect, opaque, i, iy0, mask);
      partial(setup, rect, opaque, ix1, iy0, mask | right_mask);
   } else {
      partial(setup, rect, opaque, ix0, iy0, left_mask  | top_mask);
      partial(setup, rect, opaque, ix0, iy1, left_mask  | bottom_mask);
      partial(setup, rect, opaque, ix1, iy0, right_mask | top_mask);
      partial(setup, rect, opaque, ix1, iy1, right_mask | bottom_mask);

      /* Top/Bottom fringes
       */
      for (unsigned i = ix0 + 1; i < ix1; i++) {
         partial(setup, rect, opaque, i, iy0, top_mask);
         partial(setup, rect, opaque, i, iy1, bottom_mask);
      }

      /* Left/Right fringes
       */
      for (unsigned i = iy0 + 1; i < iy1; i++) {
         partial(setup, rect, opaque, ix0, i, left_mask);
         partial(setup, rect, opaque, ix1, i, right_mask);
      }

      /* Full interior tiles
       */
      for (unsigned j = iy0 + 1; j < iy1; j++) {
         for (unsigned i = ix0 + 1; i < ix1; i++) {
            lp_setup_whole_tile(setup, &rect->inputs, i, j, opaque);
         }
      }
   }

   /* Catch any out-of-memory which occurred during binning.  Do this
    * once here rather than checking all the return values throughout.
    */
   if (lp_scene_is_oom(scene)) {
      /* Disable rasterization of this partially-binned rectangle.
       * We'll flush this scene and re-bin the entire rectangle:
       */
      rect->inputs.disable = true;
      return false;
   }

   return true;
}


void
lp_rect_cw(struct lp_setup_context *setup,
           const float (*v0)[4],
           const float (*v1)[4],
           const float (*v2)[4],
           bool frontfacing)
{
   if (lp_setup_zero_sample_mask(setup)) {
      if (0) debug_printf("zero sample mask\n");
      LP_COUNT(nr_culled_rects);
      return;
   }

   if (!try_rect_cw(setup, v0, v1, v2, frontfacing)) {
      if (!lp_setup_flush_and_restart(setup))
         return;

      if (!try_rect_cw(setup, v0, v1, v2, frontfacing))
         return;
   }
}


/**
 * Take the six vertices for two triangles and try to determine if they
 * form a screen-aligned quad/rectangle.  If so, draw the rect directly
 * and return true.  Else, return false.
 */
static bool
do_rect_ccw(struct lp_setup_context *setup,
            const float (*v0)[4],
            const float (*v1)[4],
            const float (*v2)[4],
            const float (*v3)[4],
            const float (*v4)[4],
            const float (*v5)[4],
            bool front)
{
   const float (*rv0)[4], (*rv1)[4], (*rv2)[4], (*rv3)[4];  /* rect verts */

#define SAME_POS(A, B)   (A[0][0] == B[0][0] && \
                          A[0][1] == B[0][1] && \
                          A[0][2] == B[0][2] && \
                          A[0][3] == B[0][3])

   /* Only need to consider CCW orientations.  There are nine ways
    * that two counter-clockwise triangles can join up:
    */
   if (SAME_POS(v0, v3)) {
      if (SAME_POS(v2, v4)) {
         /*
          *    v5   v4/v2
          *     +-----+
          *     |   / |
          *     |  /  |
          *     | /   |
          *     +-----+
          *   v3/v0   v1
          */
         rv0 = v5;
         rv1 = v0;
         rv2 = v1;
         rv3 = v2;
      } else if (SAME_POS(v1, v5)) {
         /*
          *    v4   v3/v0
          *     +-----+
          *     |   / |
          *     |  /  |
          *     | /   |
          *     +-----+
          *   v5/v1   v2
          */
         rv0 = v4;
         rv1 = v1;
         rv2 = v2;
         rv3 = v0;
      } else {
         return false;
      }
   } else if (SAME_POS(v0, v5)) {
      if (SAME_POS(v2, v3)) {
         /*
          *    v4   v3/v2
          *     +-----+
          *     |   / |
          *     |  /  |
          *     | /   |
          *     +-----+
          *   v5/v0   v1
          */
         rv0 = v4;
         rv1 = v0;
         rv2 = v1;
         rv3 = v2;
      } else if (SAME_POS(v1, v4)) {
         /*
          *    v3   v5/v0
          *     +-----+
          *     |   / |
          *     |  /  |
          *     | /   |
          *     +-----+
          *   v4/v1   v2
          */
         rv0 = v3;
         rv1 = v1;
         rv2 = v2;
         rv3 = v0;
      } else {
         return false;
      }
   } else if (SAME_POS(v0, v4)) {
      if (SAME_POS(v2, v5)) {
         /*
          *    v3   v5/v2
          *     +-----+
          *     |   / |
          *     |  /  |
          *     | /   |
          *     +-----+
          *   v4/v0   v1
          */
         rv0 = v3;
         rv1 = v0;
         rv2 = v1;
         rv3 = v2;
      } else if (SAME_POS(v1, v3)) {
         /*
          *    v5   v4/v0
          *     +-----+
          *     |   / |
          *     |  /  |
          *     | /   |
          *     +-----+
          *   v3/v1   v2
          */
         rv0 = v5;
         rv1 = v1;
         rv2 = v2;
         rv3 = v0;
      } else {
         return false;
      }
   } else if (SAME_POS(v2, v3)) {
      if (SAME_POS(v1, v4)) {
         /*
          *    v5   v4/v1
          *     +-----+
          *     |   / |
          *     |  /  |
          *     | /   |
          *     +-----+
          *   v3/v2   v0
          */
         rv0 = v5;
         rv1 = v2;
         rv2 = v0;
         rv3 = v1;
      } else {
         return false;
      }
   } else if (SAME_POS(v2, v5)) {
      if (SAME_POS(v1, v3)) {
         /*
          *    v4   v3/v1
          *     +-----+
          *     |   / |
          *     |  /  |
          *     | /   |
          *     +-----+
          *   v5/v2   v0
          */
         rv0 = v4;
         rv1 = v2;
         rv2 = v0;
         rv3 = v1;
      } else {
         return false;
      }
   } else if (SAME_POS(v2, v4)) {
      if (SAME_POS(v1, v5)) {
         /*
          *    v3   v5/v1
          *     +-----+
          *     |   / |
          *     |  /  |
          *     | /   |
          *     +-----+
          *   v4/v2   v0
          */
         rv0 = v3;
         rv1 = v2;
         rv2 = v0;
         rv3 = v1;
      } else {
         return false;
      }
   } else {
      return false;
   }

#define SAME_X(A, B)   (A[0][0] == B[0][0])
#define SAME_Y(A, B)   (A[0][1] == B[0][1])

   /* The vertices are now counter clockwise, as such:
    *
    *  rv0 -------rv3
    *    |        |
    *  rv1 ------ rv2
    *
    * To render as a rectangle,
    *   * The X values should be the same at v0, v1 and v2, v3.
    *   * The Y values should be the same at v0, v3 and v1, v2.
    */
   if (SAME_Y(rv0, rv1)) {
      const float (*tmp)[4];
      tmp = rv0;
      rv0 = rv1;
      rv1 = rv2;
      rv2 = rv3;
      rv3 = tmp;
   }

   if (SAME_X(rv0, rv1) && SAME_X(rv2, rv3) &&
       SAME_Y(rv0, rv3) && SAME_Y(rv1, rv2)) {
      /* We have a rectangle */

      /* Check that all vertex W components are equal.  When we divide by W in
       * lp_linear_init_interp() we assume all vertices have the same W value.
       */
      const float v0_w = rv0[0][3];
      if (rv1[0][3] != v0_w ||
          rv2[0][3] != v0_w ||
          rv3[0][3] != v0_w) {
         return false;
      }

      const struct lp_setup_variant_key *key = &setup->setup.variant->key;
      const unsigned n = key->num_inputs;

      /* Check that the other attributes are coplanar */
      for (unsigned i = 0; i < n; i++) {
         for (unsigned j = 0; j < 4; j++) {
            if (key->inputs[i].usage_mask & (1<<j)) {
               unsigned k = key->inputs[i].src_index;
               float dxdx1, dxdx2, dxdy1, dxdy2;
               dxdx1 = rv0[k][j] - rv3[k][j];
               dxdx2 = rv1[k][j] - rv2[k][j];
               dxdy1 = rv0[k][j] - rv1[k][j];
               dxdy2 = rv3[k][j] - rv2[k][j];
               if (dxdx1 != dxdx2 ||
                   dxdy1 != dxdy2) {
                  return false;
               }
            }
         }
      }

      /* Note we're changing to clockwise here.  Fix this by reworking
       * lp_rect_cw to expect/operate on ccw rects.  Note that
       * function was previously misnamed.
       */
      lp_rect_cw(setup, rv0, rv2, rv1, front);
      return true;
   } else {
      /* setup->quad(setup, rv0, rv1, rv2, rv3); */
   }

   return false;
}


enum winding {
   WINDING_NONE = 0,
   WINDING_CCW,
   WINDING_CW
};


static inline enum winding
winding(const float (*v0)[4],
        const float (*v1)[4],
        const float (*v2)[4])
{
   /* edge vectors e = v0 - v2, f = v1 - v2 */
   const float ex = v0[0][0] - v2[0][0];
   const float ey = v0[0][1] - v2[0][1];
   const float fx = v1[0][0] - v2[0][0];
   const float fy = v1[0][1] - v2[0][1];

   /* det = cross(e,f).z */
   const float det = ex * fy - ey * fx;

   if (det < 0.0f)
      return WINDING_CCW;
   else if (det > 0.0f)
      return WINDING_CW;
   else
      return WINDING_NONE;
}


static bool
setup_rect_cw(struct lp_setup_context *setup,
              const float (*v0)[4],
              const float (*v1)[4],
              const float (*v2)[4],
              const float (*v3)[4],
              const float (*v4)[4],
              const float (*v5)[4])
{
   enum winding winding0 = winding(v0, v1, v2);
   enum winding winding1 = winding(v3, v4, v5);

   if (winding0 == WINDING_CW &&
       winding1 == WINDING_CW) {
      return do_rect_ccw(setup, v0, v2, v1, v3, v5, v4, !setup->ccw_is_frontface);
   } else if (winding0 == WINDING_CW) {
      setup->triangle(setup, v0, v1, v2);
      return true;
   } else if (winding1 == WINDING_CW) {
      setup->triangle(setup, v3, v4, v5);
      return true;
   } else {
      return true;
   }
}


static bool
setup_rect_ccw(struct lp_setup_context *setup,
               const float (*v0)[4],
               const float (*v1)[4],
               const float (*v2)[4],
               const float (*v3)[4],
               const float (*v4)[4],
               const float (*v5)[4])
{
   enum winding winding0 = winding(v0, v1, v2);
   enum winding winding1 = winding(v3, v4, v5);

   if (winding0 == WINDING_CCW &&
       winding1 == WINDING_CCW) {
      return do_rect_ccw(setup, v0, v1, v2, v3, v4, v5, setup->ccw_is_frontface);
   } else if (winding0 == WINDING_CCW) {
      setup->triangle(setup, v0, v1, v2);
      return true;
   } else if (winding1 == WINDING_CCW) {
      return false;
      setup->triangle(setup, v3, v4, v5);
      return true;
   } else {
      return true;
   }
}


static bool
setup_rect_noop(struct lp_setup_context *setup,
                const float (*v0)[4],
                const float (*v1)[4],
                const float (*v2)[4],
                const float (*v3)[4],
                const float (*v4)[4],
                const float (*v5)[4])
{
   return true;
}


/*
 * Return true if the rect is handled here, else return false indicating
 * the caller should render with triangles instead.
 */
static bool
setup_rect_both(struct lp_setup_context *setup,
                const float (*v0)[4],
                const float (*v1)[4],
                const float (*v2)[4],
                const float (*v3)[4],
                const float (*v4)[4],
                const float (*v5)[4])
{
   enum winding winding0 = winding(v0, v1, v2);
   enum winding winding1 = winding(v3, v4, v5);

   if (winding0 != winding1) {
      /* If we knew that the "front" parameter wasn't going to be
       * referenced, could rearrange one of the two triangles such
       * that they were both CCW.  Aero actually does send mixed
       * CW/CCW rectangles under some circumstances, but we catch them
       * explicitly.
       */
      return false;
   } else if (winding0 == WINDING_CCW) {
      return do_rect_ccw(setup, v0, v1, v2, v3, v4, v5, setup->ccw_is_frontface);
   } else if (winding0 == WINDING_CW) {
      return do_rect_ccw(setup, v0, v2, v1, v3, v5, v4, !setup->ccw_is_frontface);
   } else {
      return true;
   }
}


void
lp_setup_choose_rect(struct lp_setup_context *setup)
{
   if (setup->rasterizer_discard) {
      setup->rect = setup_rect_noop;
      return;
   }

   switch (setup->cullmode) {
   case PIPE_FACE_NONE:
      setup->rect = setup_rect_both;
      break;
   case PIPE_FACE_BACK:
      setup->rect = setup->ccw_is_frontface ? setup_rect_ccw : setup_rect_cw;
      break;
   case PIPE_FACE_FRONT:
      setup->rect = setup->ccw_is_frontface ? setup_rect_cw : setup_rect_ccw;
      break;
   default:
      setup->rect = setup_rect_noop;
      break;
   }
}
