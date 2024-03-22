/**************************************************************************
 *
 * Copyright 2007-2010 VMware, Inc.
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
 * Rasterization for binned triangles within a tile
 */



/**
 * Prototype for a 8 plane rasterizer function.  Will codegenerate
 * several of these.
 *
 * XXX: Varients for more/fewer planes.
 * XXX: Need ways of dropping planes as we descend.
 * XXX: SIMD
 */
static void
TAG(do_block_4)(struct lp_rasterizer_task *task,
                const struct lp_rast_triangle *tri,
                const struct lp_rast_plane *plane,
                int x, int y,
                const int64_t *c)
{
#ifndef MULTISAMPLE
   unsigned mask = 0xffff;
#else
   uint64_t mask = UINT64_MAX;
#endif

   for (unsigned j = 0; j < NR_PLANES; j++) {
#ifndef MULTISAMPLE
#ifdef RASTER_64
      mask &= ~BUILD_MASK_LINEAR(((c[j] - 1) >> (int64_t)FIXED_ORDER),
                                 -plane[j].dcdx >> FIXED_ORDER,
                                 plane[j].dcdy >> FIXED_ORDER);
#else
      mask &= ~BUILD_MASK_LINEAR((c[j] - 1),
                                 -plane[j].dcdx,
                                 plane[j].dcdy);
#endif
#else
      for (unsigned s = 0; s < 4; s++) {
         int64_t new_c = (c[j]) + ((IMUL64(task->scene->fixed_sample_pos[s][1], plane[j].dcdy) + IMUL64(task->scene->fixed_sample_pos[s][0], -plane[j].dcdx)) >> FIXED_ORDER);
         uint32_t build_mask;
#ifdef RASTER_64
         build_mask = BUILD_MASK_LINEAR((int32_t)((new_c - 1) >> (int64_t)FIXED_ORDER),
                                        -plane[j].dcdx >> FIXED_ORDER,
                                        plane[j].dcdy >> FIXED_ORDER);
#else
         build_mask = BUILD_MASK_LINEAR((new_c - 1),
                                        -plane[j].dcdx,
                                        plane[j].dcdy);
#endif
         mask &= ~((uint64_t)build_mask << (s * 16));
      }
#endif
   }

   /* Now pass to the shader:
    */
   if (mask)
      lp_rast_shade_quads_mask_sample(task, &tri->inputs, x, y, mask);
}


/**
 * Evaluate a 16x16 block of pixels to determine which 4x4 subblocks are in/out
 * of the triangle's bounds.
 */
static void
TAG(do_block_16)(struct lp_rasterizer_task *task,
                 const struct lp_rast_triangle *tri,
                 const struct lp_rast_plane *plane,
                 int x, int y,
                 const int64_t *c)
{
   unsigned outmask = 0;      /* outside one or more trivial reject planes */
   unsigned partmask = 0;     /* outside one or more trivial accept planes */

   for (unsigned j = 0; j < NR_PLANES; j++) {
#ifdef RASTER_64
      int32_t dcdx = -plane[j].dcdx >> FIXED_ORDER;
      int32_t dcdy = plane[j].dcdy >> FIXED_ORDER;
      const int32_t cox = plane[j].eo >> FIXED_ORDER;
      const int32_t ei = (dcdy + dcdx - cox) << 2;
      const int32_t cox_s = cox << 2;
      const int32_t co = (int32_t)(c[j] >> (int64_t)FIXED_ORDER) + cox_s;
      int32_t cdiff;
      cdiff = ei - cox_s + ((int32_t)((c[j] - 1) >> (int64_t)FIXED_ORDER) -
                            (int32_t)(c[j] >> (int64_t)FIXED_ORDER));
      dcdx <<= 2;
      dcdy <<= 2;
#else
      const int64_t dcdx = -IMUL64(plane[j].dcdx, 4);
      const int64_t dcdy = IMUL64(plane[j].dcdy, 4);
      const int64_t cox = IMUL64(plane[j].eo, 4);
      const int32_t ei = plane[j].dcdy - plane[j].dcdx - (int64_t)plane[j].eo;
      const int64_t cio = IMUL64(ei, 4) - 1;
      int32_t co, cdiff;
      co = c[j] + cox;
      cdiff = cio - cox;
#endif

      BUILD_MASKS(co, cdiff,
                  dcdx, dcdy,
                  &outmask,   /* sign bits from c[i][0..15] + cox */
                  &partmask); /* sign bits from c[i][0..15] + cio */
   }

   if (outmask == 0xffff)
      return;

   /* Mask of sub-blocks which are inside all trivial accept planes:
    */
   unsigned inmask = ~partmask & 0xffff;

   /* Mask of sub-blocks which are inside all trivial reject planes,
    * but outside at least one trivial accept plane:
    */
   unsigned partial_mask = partmask & ~outmask;

   assert((partial_mask & inmask) == 0);

   LP_COUNT_ADD(nr_empty_4, util_bitcount(0xffff & ~(partial_mask | inmask)));

   /* Iterate over partials:
    */
   while (partial_mask) {
      int i = ffs(partial_mask) - 1;
      int ix = (i & 3) * 4;
      int iy = (i >> 2) * 4;
      int px = x + ix;
      int py = y + iy;
      int64_t cx[NR_PLANES];

      partial_mask &= ~(1 << i);

      LP_COUNT(nr_partially_covered_4);

      for (unsigned j = 0; j < NR_PLANES; j++) {
         cx[j] = (c[j]
                  - IMUL64(plane[j].dcdx, ix)
                  + IMUL64(plane[j].dcdy, iy));
      }

      TAG(do_block_4)(task, tri, plane, px, py, cx);
   }

   /* Iterate over fulls:
    */
   while (inmask) {
      int i = ffs(inmask) - 1;
      int ix = (i & 3) * 4;
      int iy = (i >> 2) * 4;
      int px = x + ix;
      int py = y + iy;

      inmask &= ~(1 << i);

      LP_COUNT(nr_fully_covered_4);
      block_full_4(task, tri, px, py);
   }
}


/**
 * Scan the tile in chunks and figure out which pixels to rasterize
 * for this triangle.
 */
void
TAG(lp_rast_triangle)(struct lp_rasterizer_task *task,
                      const union lp_rast_cmd_arg arg)
{
   const struct lp_rast_triangle *tri = arg.triangle.tri;
   unsigned plane_mask = arg.triangle.plane_mask;
   const struct lp_rast_plane *tri_plane = GET_PLANES(tri);
   const int x = task->x, y = task->y;
   struct lp_rast_plane plane[NR_PLANES];
   int64_t c[NR_PLANES];
   unsigned outmask, inmask, partmask, partial_mask;
   unsigned j = 0;

   if (tri->inputs.disable) {
      /* This triangle was partially binned and has been disabled */
      return;
   }

   outmask = 0;                 /* outside one or more trivial reject planes */
   partmask = 0;                /* outside one or more trivial accept planes */

   while (plane_mask) {
      int i = ffs(plane_mask) - 1;
      plane[j] = tri_plane[i];
      plane_mask &= ~(1 << i);
      c[j] = plane[j].c + IMUL64(plane[j].dcdy, y) - IMUL64(plane[j].dcdx, x);

      {
#ifdef RASTER_64
         /*
          * Strip off lower FIXED_ORDER bits. Note that those bits from
          * dcdx, dcdy, eo are always 0 (by definition).
          * c values, however, are not. This means that for every
          * addition of the form c + n*dcdx the lower FIXED_ORDER bits will
          * NOT change. And those bits are not relevant to the sign bit (which
          * is only what we need!) that is,
          * sign(c + n*dcdx) == sign((c >> FIXED_ORDER) + n*(dcdx >> FIXED_ORDER))
          * This means we can get away with using 32bit math for the most part.
          * Only tricky part is the -1 adjustment for cdiff.
          */
         int32_t dcdx = -plane[j].dcdx >> FIXED_ORDER;
         int32_t dcdy = plane[j].dcdy >> FIXED_ORDER;
         const int32_t cox = plane[j].eo >> FIXED_ORDER;
         const int32_t ei = (dcdy + dcdx - cox) << 4;
         const int32_t cox_s = cox << 4;
         const int32_t co = (int32_t)(c[j] >> (int64_t)FIXED_ORDER) + cox_s;
         int32_t cdiff;
         /*
          * Plausibility check to ensure the 32bit math works.
          * Note that within a tile, the max we can move the edge function
          * is essentially dcdx * TILE_SIZE + dcdy * TILE_SIZE.
          * TILE_SIZE is 64, dcdx/dcdy are nominally 21 bit (for 8192 max size
          * and 8 subpixel bits), I'd be happy with 2 bits more too (1 for
          * increasing fb size to 16384, the required d3d11 value, another one
          * because I'm not quite sure we can't be _just_ above the max value
          * here). This gives us 30 bits max - hence if c would exceed that here
          * that means the plane is either trivial reject for the whole tile
          * (in which case the tri will not get binned), or trivial accept for
          * the whole tile (in which case plane_mask will not include it).
          */
#if 0
         assert((c[j] >> (int64_t)FIXED_ORDER) > (int32_t)0xb0000000 &&
                (c[j] >> (int64_t)FIXED_ORDER) < (int32_t)0x3fffffff);
#endif
         /*
          * Note the fixup part is constant throughout the tile - thus could
          * just calculate this and avoid _all_ 64bit math in rasterization
          * (except exactly this fixup calc).
          * In fact theoretically could move that even to setup, albeit that
          * seems tricky (pre-bin certainly can have values larger than 32bit,
          * and would need to communicate that fixup value through).
          * And if we want to support msaa, we'd probably don't want to do the
          * downscaling in setup in any case...
          */
         cdiff = ei - cox_s + ((int32_t)((c[j] - 1) >> (int64_t)FIXED_ORDER) -
                               (int32_t)(c[j] >> (int64_t)FIXED_ORDER));
         dcdx <<= 4;
         dcdy <<= 4;
#else
         const int32_t dcdx = -plane[j].dcdx << 4;
         const int32_t dcdy = plane[j].dcdy << 4;
         const int32_t cox = plane[j].eo << 4;
         const int32_t ei = plane[j].dcdy - plane[j].dcdx - (int32_t)plane[j].eo;
         const int32_t cio = (ei << 4) - 1;
         int32_t co, cdiff;
         co = c[j] + cox;
         cdiff = cio - cox;
#endif
         BUILD_MASKS(co, cdiff,
                     dcdx, dcdy,
                     &outmask,   /* sign bits from c[i][0..15] + cox */
                     &partmask); /* sign bits from c[i][0..15] + cio */
      }

      j++;
   }

   if (outmask == 0xffff)
      return;

   /* Mask of sub-blocks which are inside all trivial accept planes:
    */
   inmask = ~partmask & 0xffff;

   /* Mask of sub-blocks which are inside all trivial reject planes,
    * but outside at least one trivial accept plane:
    */
   partial_mask = partmask & ~outmask;

   assert((partial_mask & inmask) == 0);

   LP_COUNT_ADD(nr_empty_16, util_bitcount(0xffff & ~(partial_mask | inmask)));

   /* Iterate over partials:
    */
   while (partial_mask) {
      int i = ffs(partial_mask) - 1;
      int ix = (i & 3) * 16;
      int iy = (i >> 2) * 16;
      int px = x + ix;
      int py = y + iy;
      int64_t cx[NR_PLANES];

      for (j = 0; j < NR_PLANES; j++)
         cx[j] = (c[j]
                  - IMUL64(plane[j].dcdx, ix)
                  + IMUL64(plane[j].dcdy, iy));

      partial_mask &= ~(1 << i);

      LP_COUNT(nr_partially_covered_16);
      TAG(do_block_16)(task, tri, plane, px, py, cx);
   }

   /* Iterate over fulls:
    */
   while (inmask) {
      int i = ffs(inmask) - 1;
      int ix = (i & 3) * 16;
      int iy = (i >> 2) * 16;
      int px = x + ix;
      int py = y + iy;

      inmask &= ~(1 << i);

      LP_COUNT(nr_fully_covered_16);
      block_full_16(task, tri, px, py);
   }
}


#if DETECT_ARCH_SSE && defined(TRI_16)
/* XXX: special case this when intersection is not required.
 *      - tile completely within bbox,
 *      - bbox completely within tile.
 */
void
TRI_16(struct lp_rasterizer_task *task,
       const union lp_rast_cmd_arg arg)
{
   const struct lp_rast_triangle *tri = arg.triangle.tri;
   const struct lp_rast_plane *plane = GET_PLANES(tri);
   unsigned mask = arg.triangle.plane_mask;
   __m128i cstep4[NR_PLANES][4];
   int x = (mask & 0xff);
   int y = (mask >> 8);
   unsigned outmask = 0;    /* outside one or more trivial reject planes */

   if (x + 12 >= 64) {
      int i = ((x + 12) - 64) / 4;
      outmask |= right_mask_tab[i];
   }

   if (y + 12 >= 64) {
      int i = ((y + 12) - 64) / 4;
      outmask |= bottom_mask_tab[i];
   }

   x += task->x;
   y += task->y;

   for (unsigned j = 0; j < NR_PLANES; j++) {
      const int dcdx = -plane[j].dcdx * 4;
      const int dcdy = plane[j].dcdy * 4;
      __m128i xdcdy = _mm_set1_epi32(dcdy);

      cstep4[j][0] = _mm_setr_epi32(0, dcdx, dcdx*2, dcdx*3);
      cstep4[j][1] = _mm_add_epi32(cstep4[j][0], xdcdy);
      cstep4[j][2] = _mm_add_epi32(cstep4[j][1], xdcdy);
      cstep4[j][3] = _mm_add_epi32(cstep4[j][2], xdcdy);

      {
         const int c = plane[j].c + plane[j].dcdy * y - plane[j].dcdx * x;
         const int cox = plane[j].eo * 4;

         outmask |= sign_bits4(cstep4[j], c + cox);
      }
   }

   if (outmask == 0xffff)
      return;


   /* Mask of sub-blocks which are inside all trivial reject planes,
    * but outside at least one trivial accept plane:
    */
   unsigned partial_mask = 0xffff & ~outmask;

   /* Iterate over partials:
    */
   while (partial_mask) {
      int i = ffs(partial_mask) - 1;
      int ix = (i & 3) * 4;
      int iy = (i >> 2) * 4;
      int px = x + ix;
      int py = y + iy;
      unsigned mask = 0xffff;

      partial_mask &= ~(1 << i);

      for (unsigned j = 0; j < NR_PLANES; j++) {
         const int cx = (plane[j].c - 1
                         - plane[j].dcdx * px
                         + plane[j].dcdy * py) * 4;

         mask &= ~sign_bits4(cstep4[j], cx);
      }

      if (mask)
         lp_rast_shade_quads_mask(task, &tri->inputs, px, py, mask);
   }
}
#endif


#if DETECT_ARCH_SSE && defined(TRI_4)
void
TRI_4(struct lp_rasterizer_task *task,
      const union lp_rast_cmd_arg arg)
{
   const struct lp_rast_triangle *tri = arg.triangle.tri;
   const struct lp_rast_plane *plane = GET_PLANES(tri);
   unsigned mask = arg.triangle.plane_mask;
   const int x = task->x + (mask & 0xff);
   const int y = task->y + (mask >> 8);

   /* Iterate over partials:
    */
   unsigned mask = 0xffff;

   for (unsigned j = 0; j < NR_PLANES; j++) {
      const int cx = (plane[j].c
                      - plane[j].dcdx * x
                      + plane[j].dcdy * y);

      const int dcdx = -plane[j].dcdx;
      const int dcdy = plane[j].dcdy;
      __m128i xdcdy = _mm_set1_epi32(dcdy);

      __m128i cstep0 = _mm_setr_epi32(cx, cx + dcdx, cx + dcdx*2, cx + dcdx*3);
      __m128i cstep1 = _mm_add_epi32(cstep0, xdcdy);
      __m128i cstep2 = _mm_add_epi32(cstep1, xdcdy);
      __m128i cstep3 = _mm_add_epi32(cstep2, xdcdy);

      __m128i cstep01 = _mm_packs_epi32(cstep0, cstep1);
      __m128i cstep23 = _mm_packs_epi32(cstep2, cstep3);
      __m128i result = _mm_packs_epi16(cstep01, cstep23);

      /* Extract the sign bits
       */
      mask &= ~_mm_movemask_epi8(result);
   }

   if (mask)
      lp_rast_shade_quads_mask(task, &tri->inputs, x, y, mask);
}
#endif


#undef TAG
#undef TRI_4
#undef TRI_16
#undef NR_PLANES
