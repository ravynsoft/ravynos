/* sampler template functions */

#ifndef NO_MEMCPY
/*
 * Unstretched blit of a bgrx texture.
 */
static const uint32_t *
CONCAT2(fetch_memcpy_, FETCH_TYPE)(struct lp_linear_elem *elem)
{
   struct lp_linear_sampler *samp = (struct lp_linear_sampler *)elem;
   const struct lp_jit_texture *texture = samp->texture;
   const uint32_t *src_row =
      (const uint32_t *)((const uint8_t *)texture->base +
                         (samp->t >> FIXED16_SHIFT) * texture->row_stride[0]);
   const int s     = samp->s;
   const int width = samp->width;
   uint32_t *row   = samp->row;

   src_row = &src_row[s >> FIXED16_SHIFT];

   for (int i = 0; i < width; i++) {
      row[i] = OP(src_row[i]);
   }

   samp->t += samp->dtdy;
   return row;
}
#endif

/*
 * Perform nearest filtered lookup of a row of texels.  Texture lookup
 * is assumed to be axis aligned but with arbitrary scaling.
 *
 * Texture coordinate interpolation is performed in 16.16 fixed point,
 * not to be confused with the 1.15 format used by the interpolants.
 *
 * After 64 pixels (ie. in the next tile), the starting point will be
 * recalculated with floating point arithmetic.
 */
static const uint32_t *
CONCAT2(fetch_axis_aligned_, FETCH_TYPE)(struct lp_linear_elem *elem)
{
   struct lp_linear_sampler *samp = (struct lp_linear_sampler *)elem;
   const struct lp_jit_texture *texture = samp->texture;
   const uint32_t *src_row =
      (const uint32_t *)((const uint8_t *)texture->base +
                         (samp->t >> FIXED16_SHIFT) * texture->row_stride[0]);
   const int dsdx  = samp->dsdx;
   const int width = samp->width;
   uint32_t *row   = samp->row;
   int s = samp->s;

   for (int i = 0; i < width; i++) {
      row[i] = OP(src_row[s>>FIXED16_SHIFT]);
      s += dsdx;
   }

   samp->t += samp->dtdy;
   return row;
}

/* Non-axis aligned, but no clamping or wrapping required
 */
static const uint32_t *
CONCAT2(fetch_, FETCH_TYPE)(struct lp_linear_elem *elem)
{
   struct lp_linear_sampler *samp = (struct lp_linear_sampler *)elem;
   const struct lp_jit_texture *texture = samp->texture;
   const uint8_t *src = texture->base;
   const int stride = texture->row_stride[0];
   const int dsdx  = samp->dsdx;
   const int dtdx  = samp->dtdx;
   const int width = samp->width;
   uint32_t *row   = samp->row;
   int s = samp->s;
   int t = samp->t;

   for (int i = 0; i < width; i++) {
      const uint8_t *texel = (src +
                              (t>>FIXED16_SHIFT) * stride +
                              (s>>FIXED16_SHIFT) * 4);

      row[i] = OP(*(const uint32_t *)texel);

      s += dsdx;
      t += dtdx;
   }

   samp->s += samp->dsdy;
   samp->t += samp->dtdy;
   return row;
}

/* Non-axis aligned, clamped.
 */
static const uint32_t *
CONCAT2(fetch_clamp_, FETCH_TYPE)(struct lp_linear_elem *elem)
{
   struct lp_linear_sampler *samp = (struct lp_linear_sampler *)elem;
   const struct lp_jit_texture *texture = samp->texture;
   const uint8_t *src   = texture->base;
   const int stride     = texture->row_stride[0];
   const int tex_height = texture->height - 1;
   const int tex_width  = texture->width - 1;
   const int dsdx  = samp->dsdx;
   const int dtdx  = samp->dtdx;
   const int width = samp->width;
   uint32_t *row   = samp->row;
   int s = samp->s;
   int t = samp->t;

   for (int i = 0; i < width; i++) {
      int ct = CLAMP(t>>FIXED16_SHIFT, 0, tex_height);
      int cs = CLAMP(s>>FIXED16_SHIFT, 0, tex_width);

      const uint8_t *texel = src + ct * stride + cs * 4;

      row[i] = OP(*(const uint32_t *)texel);

      s += dsdx;
      t += dtdx;
   }

   samp->s += samp->dsdy;
   samp->t += samp->dtdy;
   return row;
}

#ifdef OP128
static const uint32_t *
CONCAT2(fetch_axis_aligned_linear_, FETCH_TYPE)(struct lp_linear_elem *elem)
{
   struct lp_linear_sampler *samp = (struct lp_linear_sampler *)elem;

   uint32_t *dst_row = samp->row;
   const uint32_t *src_row = fetch_axis_aligned_linear_bgra(&samp->base);
   const int width = samp->width;

   for (int i = 0; i < width; i += 4) {
      __m128i bgra = *(__m128i *)&src_row[i];
      __m128i rgba = OP128(bgra);
      *(__m128i *)&dst_row[i] = rgba;
   }

   return dst_row;
}

static const uint32_t *
CONCAT2(fetch_clamp_linear_, FETCH_TYPE)(struct lp_linear_elem *elem)
{
   struct lp_linear_sampler *samp = (struct lp_linear_sampler *)elem;
   uint32_t *row = samp->row;
   const int width = samp->width;

   fetch_clamp_linear_bgra(&samp->base);

   for (int i = 0; i < width; i += 4) {
      __m128i bgra = *(__m128i *)&row[i];
      __m128i rgba = OP128(bgra);
      *(__m128i *)&row[i] = rgba;
   }

   return row;
}

static const uint32_t *
CONCAT2(fetch_linear_, FETCH_TYPE)(struct lp_linear_elem *elem)
{
   struct lp_linear_sampler *samp = (struct lp_linear_sampler *)elem;
   uint32_t *row = samp->row;
   const int width = samp->width;

   fetch_linear_bgra(&samp->base);

   for (int i = 0; i < width; i += 4) {
      __m128i bgra = *(__m128i *)&row[i];
      __m128i rgba = OP128(bgra);
      *(__m128i *)&row[i] = rgba;
   }

   return row;
}
#endif

#undef OP
#undef OP128
#undef FETCH_TYPE
#undef NO_MEMCPY
