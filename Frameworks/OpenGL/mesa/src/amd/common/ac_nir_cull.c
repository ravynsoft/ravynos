/*
 * Copyright 2019 Advanced Micro Devices, Inc.
 * Copyright 2021 Valve Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#include "ac_nir.h"
#include "nir_builder.h"

/* This code is adapted from ac_llvm_cull.c, hence the copyright to AMD. */

typedef struct
{
   nir_def *w_reflection;
   nir_def *all_w_negative;
   nir_def *any_w_negative;
} position_w_info;

static void
analyze_position_w(nir_builder *b, nir_def *pos[][4], unsigned num_vertices,
                   position_w_info *w_info)
{
   w_info->all_w_negative = nir_imm_true(b);
   w_info->w_reflection = nir_imm_false(b);
   w_info->any_w_negative = nir_imm_false(b);

   for (unsigned i = 0; i < num_vertices; ++i) {
      nir_def *neg_w = nir_flt_imm(b, pos[i][3], 0.0f);
      w_info->w_reflection = nir_ixor(b, neg_w, w_info->w_reflection);
      w_info->any_w_negative = nir_ior(b, neg_w, w_info->any_w_negative);
      w_info->all_w_negative = nir_iand(b, neg_w, w_info->all_w_negative);
   }
}

static nir_def *
cull_face_triangle(nir_builder *b, nir_def *pos[3][4], const position_w_info *w_info)
{
   nir_def *det_t0 = nir_fsub(b, pos[2][0], pos[0][0]);
   nir_def *det_t1 = nir_fsub(b, pos[1][1], pos[0][1]);
   nir_def *det_t2 = nir_fsub(b, pos[0][0], pos[1][0]);
   nir_def *det_t3 = nir_fsub(b, pos[0][1], pos[2][1]);
   nir_def *det_p0 = nir_fmul(b, det_t0, det_t1);
   nir_def *det_p1 = nir_fmul(b, det_t2, det_t3);
   nir_def *det = nir_fsub(b, det_p0, det_p1);

   det = nir_bcsel(b, w_info->w_reflection, nir_fneg(b, det), det);

   nir_def *front_facing_ccw = nir_fgt_imm(b, det, 0.0f);
   nir_def *zero_area = nir_feq_imm(b, det, 0.0f);
   nir_def *ccw = nir_load_cull_ccw_amd(b);
   nir_def *front_facing = nir_ieq(b, front_facing_ccw, ccw);
   nir_def *cull_front = nir_load_cull_front_face_enabled_amd(b);
   nir_def *cull_back = nir_load_cull_back_face_enabled_amd(b);

   nir_def *face_culled = nir_bcsel(b, front_facing, cull_front, cull_back);
   face_culled = nir_ior(b, face_culled, zero_area);

   /* Don't reject NaN and +/-infinity, these are tricky.
    * Just trust fixed-function HW to handle these cases correctly.
    */
   return nir_iand(b, face_culled, nir_fisfinite(b, det));
}

static void
calc_bbox_triangle(nir_builder *b, nir_def *pos[3][4], nir_def *bbox_min[2], nir_def *bbox_max[2])
{
   for (unsigned chan = 0; chan < 2; ++chan) {
      bbox_min[chan] = nir_fmin(b, pos[0][chan], nir_fmin(b, pos[1][chan], pos[2][chan]));
      bbox_max[chan] = nir_fmax(b, pos[0][chan], nir_fmax(b, pos[1][chan], pos[2][chan]));
   }
}

static nir_def *
cull_frustrum(nir_builder *b, nir_def *bbox_min[2], nir_def *bbox_max[2])
{
   nir_def *prim_outside_view = nir_imm_false(b);

   for (unsigned chan = 0; chan < 2; ++chan) {
      prim_outside_view = nir_ior(b, prim_outside_view, nir_flt_imm(b, bbox_max[chan], -1.0f));
      prim_outside_view = nir_ior(b, prim_outside_view, nir_fgt_imm(b, bbox_min[chan], 1.0f));
   }

   return prim_outside_view;
}

static nir_def *
cull_small_primitive_triangle(nir_builder *b, nir_def *bbox_min[2], nir_def *bbox_max[2],
                              nir_def *prim_is_small_else)
{
   nir_def *prim_is_small = NULL;

   nir_if *if_cull_small_prims = nir_push_if(b, nir_load_cull_small_primitives_enabled_amd(b));
   {
      nir_def *vp = nir_load_viewport_xy_scale_and_offset(b);
      nir_def *small_prim_precision = nir_load_cull_small_prim_precision_amd(b);
      prim_is_small = prim_is_small_else;

      for (unsigned chan = 0; chan < 2; ++chan) {
         nir_def *vp_scale = nir_channel(b, vp, chan);
         nir_def *vp_translate = nir_channel(b, vp, 2 + chan);

         /* Convert the position to screen-space coordinates. */
         nir_def *min = nir_ffma(b, bbox_min[chan], vp_scale, vp_translate);
         nir_def *max = nir_ffma(b, bbox_max[chan], vp_scale, vp_translate);

         /* Scale the bounding box according to precision. */
         min = nir_fsub(b, min, small_prim_precision);
         max = nir_fadd(b, max, small_prim_precision);

         /* Determine if the bbox intersects the sample point, by checking if the min and max round to the same int. */
         min = nir_fround_even(b, min);
         max = nir_fround_even(b, max);

         nir_def *rounded_to_eq = nir_feq(b, min, max);
         prim_is_small = nir_ior(b, prim_is_small, rounded_to_eq);
      }
   }
   nir_pop_if(b, if_cull_small_prims);

   return nir_if_phi(b, prim_is_small, prim_is_small_else);
}

static nir_def *
ac_nir_cull_triangle(nir_builder *b,
                     nir_def *initially_accepted,
                     nir_def *pos[3][4],
                     position_w_info *w_info,
                     ac_nir_cull_accepted accept_func,
                     void *state)
{
   nir_def *accepted = initially_accepted;
   accepted = nir_iand(b, accepted, nir_inot(b, w_info->all_w_negative));
   accepted = nir_iand(b, accepted, nir_inot(b, cull_face_triangle(b, pos, w_info)));

   nir_def *bbox_accepted = NULL;

   nir_if *if_accepted = nir_push_if(b, accepted);
   {
      nir_def *bbox_min[2] = {0}, *bbox_max[2] = {0};
      calc_bbox_triangle(b, pos, bbox_min, bbox_max);

      nir_def *prim_outside_view = cull_frustrum(b, bbox_min, bbox_max);
      nir_def *prim_invisible =
         cull_small_primitive_triangle(b, bbox_min, bbox_max, prim_outside_view);

      bbox_accepted = nir_ior(b, nir_inot(b, prim_invisible), w_info->any_w_negative);

      /* for caller which need to react when primitive is accepted */
      if (accept_func) {
         nir_if *if_still_accepted = nir_push_if(b, bbox_accepted);
         if_still_accepted->control = nir_selection_control_divergent_always_taken;
         {
            accept_func(b, state);
         }
         nir_pop_if(b, if_still_accepted);
      }
   }
   nir_pop_if(b, if_accepted);

   return nir_if_phi(b, bbox_accepted, accepted);
}

static void
rotate_45degrees(nir_builder *b, nir_def *v[2])
{
   /* sin(45) == cos(45) */
   nir_def *sincos45 = nir_imm_float(b, 0.707106781);

   /* x2  =  x*cos45 - y*sin45  =  x*sincos45 - y*sincos45
    * y2  =  x*sin45 + y*cos45  =  x*sincos45 + y*sincos45
    */
   nir_def *first = nir_fmul(b, v[0], sincos45);

   /* Doing 2x ffma while duplicating the multiplication is 33% faster than fmul+fadd+fadd. */
   nir_def *result[2] = {
      nir_ffma(b, nir_fneg(b, v[1]), sincos45, first),
      nir_ffma(b, v[1], sincos45, first),
   };

   memcpy(v, result, sizeof(result));
}

static void
calc_bbox_line(nir_builder *b, nir_def *pos[3][4], nir_def *bbox_min[2], nir_def *bbox_max[2])
{
   nir_def *clip_half_line_width = nir_load_clip_half_line_width_amd(b);

   for (unsigned chan = 0; chan < 2; ++chan) {
      bbox_min[chan] = nir_fmin(b, pos[0][chan], pos[1][chan]);
      bbox_max[chan] = nir_fmax(b, pos[0][chan], pos[1][chan]);

      nir_def *width = nir_channel(b, clip_half_line_width, chan);
      bbox_min[chan] = nir_fsub(b, bbox_min[chan], width);
      bbox_max[chan] = nir_fadd(b, bbox_max[chan], width);
   }
}

static nir_def *
cull_small_primitive_line(nir_builder *b, nir_def *pos[3][4],
                          nir_def *bbox_min[2], nir_def *bbox_max[2],
                          nir_def *prim_is_small_else)
{
   nir_def *prim_is_small = NULL;

   /* Small primitive filter - eliminate lines that are too small to affect a sample. */
   nir_if *if_cull_small_prims = nir_push_if(b, nir_load_cull_small_primitives_enabled_amd(b));
   {
      /* This only works with lines without perpendicular end caps (lines with perpendicular
       * end caps are rasterized as quads and thus can't be culled as small prims in 99% of
       * cases because line_width >= 1).
       *
       * This takes advantage of the diamond exit rule, which says that every pixel
       * has a diamond inside it touching the pixel boundary and only if a line exits
       * the diamond, that pixel is filled. If a line enters the diamond or stays
       * outside the diamond, the pixel isn't filled.
       *
       * This algorithm is a little simpler than that. The space outside all diamonds also
       * has the same diamond shape, which we'll call corner diamonds.
       *
       * The idea is to cull all lines that are entirely inside a diamond, including
       * corner diamonds. If a line is entirely inside a diamond, it can be culled because
       * it doesn't exit it. If a line is entirely inside a corner diamond, it can be culled
       * because it doesn't enter any diamond and thus can't exit any diamond.
       *
       * The viewport is rotated by 45 degrees to turn diamonds into squares, and a bounding
       * box test is used to determine whether a line is entirely inside any square (diamond).
       *
       * The line width doesn't matter. Wide lines only duplicate filled pixels in either X or
       * Y direction from the filled pixels. MSAA also doesn't matter. MSAA should ideally use
       * perpendicular end caps that enable quad rasterization for lines. Thus, this should
       * always use non-MSAA viewport transformation and non-MSAA small prim precision.
       *
       * A good test is piglit/lineloop because it draws 10k subpixel lines in a circle.
       * It should contain no holes if this matches hw behavior.
       */
      nir_def *v0[2], *v1[2];
      nir_def *vp = nir_load_viewport_xy_scale_and_offset(b);

      /* Get vertex positions in pixels. */
      for (unsigned chan = 0; chan < 2; chan++) {
         nir_def *vp_scale = nir_channel(b, vp, chan);
         nir_def *vp_translate = nir_channel(b, vp, 2 + chan);

         v0[chan] = nir_ffma(b, pos[0][chan], vp_scale, vp_translate);
         v1[chan] = nir_ffma(b, pos[1][chan], vp_scale, vp_translate);
      }

      /* Rotate the viewport by 45 degrees, so that diamonds become squares. */
      rotate_45degrees(b, v0);
      rotate_45degrees(b, v1);

      nir_def *small_prim_precision = nir_load_cull_small_prim_precision_amd(b);

      nir_def *rounded_to_eq[2];
      for (unsigned chan = 0; chan < 2; chan++) {
         /* The width of each square is sqrt(0.5), so scale it to 1 because we want
          * round() to give us the position of the closest center of a square (diamond).
          */
         v0[chan] = nir_fmul_imm(b, v0[chan], 1.414213562);
         v1[chan] = nir_fmul_imm(b, v1[chan], 1.414213562);

         /* Compute the bounding box around both vertices. We do this because we must
          * enlarge the line area by the precision of the rasterizer.
          */
         nir_def *min = nir_fmin(b, v0[chan], v1[chan]);
         nir_def *max = nir_fmax(b, v0[chan], v1[chan]);

         /* Enlarge the bounding box by the precision of the rasterizer. */
         min = nir_fsub(b, min, small_prim_precision);
         max = nir_fadd(b, max, small_prim_precision);

         /* Round the bounding box corners. If both rounded corners are equal,
          * the bounding box is entirely inside a square (diamond).
          */
         min = nir_fround_even(b, min);
         max = nir_fround_even(b, max);

         rounded_to_eq[chan] = nir_feq(b, min, max);
      }

      prim_is_small = nir_iand(b, rounded_to_eq[0], rounded_to_eq[1]);
      prim_is_small = nir_ior(b, prim_is_small, prim_is_small_else);
   }
   nir_pop_if(b, if_cull_small_prims);

   return nir_if_phi(b, prim_is_small, prim_is_small_else);
}

static nir_def *
ac_nir_cull_line(nir_builder *b,
                 nir_def *initially_accepted,
                 nir_def *pos[3][4],
                 position_w_info *w_info,
                 ac_nir_cull_accepted accept_func,
                 void *state)
{
   nir_def *accepted = initially_accepted;
   accepted = nir_iand(b, accepted, nir_inot(b, w_info->all_w_negative));

   nir_def *bbox_accepted = NULL;

   nir_if *if_accepted = nir_push_if(b, accepted);
   {
      nir_def *bbox_min[2] = {0}, *bbox_max[2] = {0};
      calc_bbox_line(b, pos, bbox_min, bbox_max);

      /* Frustrum culling - eliminate lines that are fully outside the view. */
      nir_def *prim_outside_view = cull_frustrum(b, bbox_min, bbox_max);
      nir_def *prim_invisible =
         cull_small_primitive_line(b, pos, bbox_min, bbox_max, prim_outside_view);

      bbox_accepted = nir_ior(b, nir_inot(b, prim_invisible), w_info->any_w_negative);

      /* for caller which need to react when primitive is accepted */
      if (accept_func) {
         nir_if *if_still_accepted = nir_push_if(b, bbox_accepted);
         {
            accept_func(b, state);
         }
         nir_pop_if(b, if_still_accepted);
      }
   }
   nir_pop_if(b, if_accepted);

   return nir_if_phi(b, bbox_accepted, accepted);
}

nir_def *
ac_nir_cull_primitive(nir_builder *b,
                      nir_def *initially_accepted,
                      nir_def *pos[3][4],
                      unsigned num_vertices,
                      ac_nir_cull_accepted accept_func,
                      void *state)
{
   position_w_info w_info = {0};
   analyze_position_w(b, pos, num_vertices, &w_info);

   if (num_vertices == 3)
      return ac_nir_cull_triangle(b, initially_accepted, pos, &w_info, accept_func, state);
   else if (num_vertices == 2)
      return ac_nir_cull_line(b, initially_accepted, pos, &w_info, accept_func, state);
   else
      unreachable("point culling not implemented");

   return NULL;
}
