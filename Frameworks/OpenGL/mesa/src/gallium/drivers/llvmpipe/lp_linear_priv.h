
#ifndef LP_LINEAR_PRIV_H
#define LP_LINEAR_PRIV_H

struct lp_linear_elem;

typedef const uint32_t *(*lp_linear_func)(struct lp_linear_elem *base);


struct lp_linear_elem {
   lp_linear_func fetch;
};


/* "Linear" refers to the fact we're on the linear (non-swizzled)
 * rasterization path.  Filtering mode may be either linear or
 * nearest.
 */
struct lp_linear_sampler {
   struct lp_linear_elem base;

   const struct lp_jit_texture *texture;
   int s;                       /* 16.16, biased by .5 */
   int t;                       /* 16.16, biased by .5 */
   int dsdx;                    /* 16.16 */
   int dsdy;                    /* 16.16 */
   int dtdx;                    /* 16.16 */
   int dtdy;                    /* 16.16 */
   int width;
   bool axis_aligned;

   alignas(16) uint32_t row[64];
   alignas(16) uint32_t stretched_row[2][64];

   /**
    * y coordinate of the rows stored in the stretched_row.
    *
    * Negative number means no stretched row is cached.
    */
   int stretched_row_y[2];

   /**
    * The index of stretched_row to receive the next stretched row.
    */
   int stretched_row_index;
};

/* "Linear" refers to the fact we're on the linear (non-swizzled)
 * rasterization path.  Interpolation mode may be either constant,
 * linear or perspective.
 */
struct lp_linear_interp {
   struct lp_linear_elem base;

#if DETECT_ARCH_SSE
   __m128i a0;
   __m128i dadx;
   __m128i dady;
#endif

   int width;                   /* rounded up to multiple of 4 */

   alignas(16) uint32_t row[64];
};


/* Check for a sampler variant which matches our fetch_row
 * implementation - normalized texcoords, single mipmap with
 * nearest filtering.
 */
static inline bool
is_nearest_sampler(const struct lp_sampler_static_state *sampler)
{
   return
      sampler->texture_state.target == PIPE_TEXTURE_2D &&
      sampler->sampler_state.min_img_filter == PIPE_TEX_FILTER_NEAREST &&
      sampler->sampler_state.mag_img_filter == PIPE_TEX_FILTER_NEAREST &&
      (sampler->texture_state.level_zero_only ||
       sampler->sampler_state.min_mip_filter == PIPE_TEX_MIPFILTER_NONE) &&
      sampler->sampler_state.compare_mode == 0 &&
      sampler->sampler_state.normalized_coords == 1;
}


/* Check for a sampler variant which matches our fetch_row
 * implementation - normalized texcoords, single mipmap with
 * linear filtering.
 */
static inline bool
is_linear_sampler(const struct lp_sampler_static_state *sampler)
{
   return
      sampler->texture_state.target == PIPE_TEXTURE_2D &&
      sampler->sampler_state.min_img_filter == PIPE_TEX_FILTER_LINEAR &&
      sampler->sampler_state.mag_img_filter == PIPE_TEX_FILTER_LINEAR &&
      (sampler->texture_state.level_zero_only ||
       sampler->sampler_state.min_mip_filter == PIPE_TEX_MIPFILTER_NONE) &&
      sampler->sampler_state.compare_mode == 0 &&
      sampler->sampler_state.normalized_coords == 1;
}


/* Check for a sampler variant which matches is_nearest_sampler
 * but has the additional constraints of using clamp wrapping
 */
static inline bool
is_nearest_clamp_sampler(const struct lp_sampler_static_state *sampler)
{
   return
      is_nearest_sampler(sampler) &&
      sampler->sampler_state.wrap_s == PIPE_TEX_WRAP_CLAMP_TO_EDGE &&
      sampler->sampler_state.wrap_t == PIPE_TEX_WRAP_CLAMP_TO_EDGE;
}


/* Check for a sampler variant which matches is_linear_sampler
 * but has the additional constraints of using clamp wrapping
 */
static inline bool
is_linear_clamp_sampler(const struct lp_sampler_static_state *sampler)
{
   return
      is_linear_sampler(sampler) &&
      sampler->sampler_state.wrap_s == PIPE_TEX_WRAP_CLAMP_TO_EDGE &&
      sampler->sampler_state.wrap_t == PIPE_TEX_WRAP_CLAMP_TO_EDGE;
}


bool
lp_linear_init_interp(struct lp_linear_interp *interp,
                      int x, int y, int width, int height,
                      unsigned usage_mask,
                      bool perspective,
                      float oow,
                      const float *a0,
                      const float *dadx,
                      const float *dady);

bool
lp_linear_init_sampler(struct lp_linear_sampler *samp,
                       const struct lp_tgsi_texture_info *info,
                       const struct lp_sampler_static_state *sampler_state,
                       const struct lp_jit_texture *texture,
                       int x0, int y0, int width, int height,
                       const float (*a0)[4],
                       const float (*dadx)[4],
                       const float (*dady)[4], bool rgba_order);


bool
lp_linear_check_fastpath(struct lp_fragment_shader_variant *variant);

bool
lp_linear_check_sampler(const struct lp_sampler_static_state *sampler,
                        const struct lp_tgsi_texture_info *tex);


void
lp_linear_init_noop_interp(struct lp_linear_interp *interp);

void
lp_linear_init_noop_sampler(struct lp_linear_sampler *samp);


#define FAIL(s) do {                                    \
      if (LP_DEBUG & DEBUG_LINEAR)                      \
         debug_printf("%s: %s\n", __func__, s);         \
      return false;                                     \
} while (0)

#endif
