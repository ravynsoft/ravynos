
#include "util/u_viewport.h"

#include "nv50/nv50_context.h"

static inline void
nv50_fb_set_null_rt(struct nouveau_pushbuf *push, unsigned i)
{
   BEGIN_NV04(push, NV50_3D(RT_ADDRESS_HIGH(i)), 4);
   PUSH_DATA (push, 0);
   PUSH_DATA (push, 0);
   PUSH_DATA (push, 0);
   PUSH_DATA (push, 0);
   BEGIN_NV04(push, NV50_3D(RT_HORIZ(i)), 2);
   PUSH_DATA (push, 64);
   PUSH_DATA (push, 0);
}

static void
nv50_validate_fb(struct nv50_context *nv50)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   struct pipe_framebuffer_state *fb = &nv50->framebuffer;
   unsigned i;
   unsigned ms_mode = NV50_3D_MULTISAMPLE_MODE_MS1;
   uint32_t array_size = 0xffff, array_mode = 0;

   nouveau_bufctx_reset(nv50->bufctx_3d, NV50_BIND_3D_FB);

   BEGIN_NV04(push, NV50_3D(RT_CONTROL), 1);
   PUSH_DATA (push, (076543210 << 4) | fb->nr_cbufs);
   BEGIN_NV04(push, NV50_3D(SCREEN_SCISSOR_HORIZ), 2);
   PUSH_DATA (push, fb->width << 16);
   PUSH_DATA (push, fb->height << 16);

   for (i = 0; i < fb->nr_cbufs; ++i) {
      struct nv50_miptree *mt;
      struct nv50_surface *sf;
      struct nouveau_bo *bo;

      if (!fb->cbufs[i]) {
         nv50_fb_set_null_rt(push, i);
         continue;
      }

      mt = nv50_miptree(fb->cbufs[i]->texture);
      sf = nv50_surface(fb->cbufs[i]);
      bo = mt->base.bo;

      array_size = MIN2(array_size, sf->depth);
      if (mt->layout_3d)
         array_mode = NV50_3D_RT_ARRAY_MODE_MODE_3D; /* 1 << 16 */

      /* can't mix 3D with ARRAY or have RTs of different depth/array_size */
      assert(mt->layout_3d || !array_mode || array_size == 1);

      BEGIN_NV04(push, NV50_3D(RT_ADDRESS_HIGH(i)), 5);
      PUSH_DATAh(push, mt->base.address + sf->offset);
      PUSH_DATA (push, mt->base.address + sf->offset);
      PUSH_DATA (push, nv50_format_table[sf->base.format].rt);
      if (likely(nouveau_bo_memtype(bo))) {
         assert(sf->base.texture->target != PIPE_BUFFER);

         PUSH_DATA (push, mt->level[sf->base.u.tex.level].tile_mode);
         PUSH_DATA (push, mt->layer_stride >> 2);
         BEGIN_NV04(push, NV50_3D(RT_HORIZ(i)), 2);
         PUSH_DATA (push, sf->width);
         PUSH_DATA (push, sf->height);
         BEGIN_NV04(push, NV50_3D(RT_ARRAY_MODE), 1);
         PUSH_DATA (push, array_mode | array_size);
         nv50->rt_array_mode = array_mode | array_size;
      } else {
         PUSH_DATA (push, 0);
         PUSH_DATA (push, 0);
         BEGIN_NV04(push, NV50_3D(RT_HORIZ(i)), 2);
         PUSH_DATA (push, NV50_3D_RT_HORIZ_LINEAR | mt->level[0].pitch);
         PUSH_DATA (push, sf->height);
         BEGIN_NV04(push, NV50_3D(RT_ARRAY_MODE), 1);
         PUSH_DATA (push, 0);

         assert(!fb->zsbuf);
         assert(!mt->ms_mode);
      }

      ms_mode = mt->ms_mode;

      if (mt->base.status & NOUVEAU_BUFFER_STATUS_GPU_READING)
         nv50->state.rt_serialize = true;
      mt->base.status |= NOUVEAU_BUFFER_STATUS_GPU_WRITING;
      mt->base.status &= ~NOUVEAU_BUFFER_STATUS_GPU_READING;

      /* only register for writing, otherwise we'd always serialize here */
      BCTX_REFN(nv50->bufctx_3d, 3D_FB, &mt->base, WR);
   }

   if (fb->zsbuf) {
      struct nv50_miptree *mt = nv50_miptree(fb->zsbuf->texture);
      struct nv50_surface *sf = nv50_surface(fb->zsbuf);
      int unk = mt->base.base.target == PIPE_TEXTURE_3D || sf->depth == 1;

      BEGIN_NV04(push, NV50_3D(ZETA_ADDRESS_HIGH), 5);
      PUSH_DATAh(push, mt->base.address + sf->offset);
      PUSH_DATA (push, mt->base.address + sf->offset);
      PUSH_DATA (push, nv50_format_table[fb->zsbuf->format].rt);
      PUSH_DATA (push, mt->level[sf->base.u.tex.level].tile_mode);
      PUSH_DATA (push, mt->layer_stride >> 2);
      BEGIN_NV04(push, NV50_3D(ZETA_ENABLE), 1);
      PUSH_DATA (push, 1);
      BEGIN_NV04(push, NV50_3D(ZETA_HORIZ), 3);
      PUSH_DATA (push, sf->width);
      PUSH_DATA (push, sf->height);
      PUSH_DATA (push, (unk << 16) | sf->depth);

      ms_mode = mt->ms_mode;

      if (mt->base.status & NOUVEAU_BUFFER_STATUS_GPU_READING)
         nv50->state.rt_serialize = true;
      mt->base.status |= NOUVEAU_BUFFER_STATUS_GPU_WRITING;
      mt->base.status &= ~NOUVEAU_BUFFER_STATUS_GPU_READING;

      BCTX_REFN(nv50->bufctx_3d, 3D_FB, &mt->base, WR);
   } else {
      BEGIN_NV04(push, NV50_3D(ZETA_ENABLE), 1);
      PUSH_DATA (push, 0);
   }

   BEGIN_NV04(push, NV50_3D(MULTISAMPLE_MODE), 1);
   PUSH_DATA (push, ms_mode);

   /* Only need to initialize the first viewport, which is used for clears */
   BEGIN_NV04(push, NV50_3D(VIEWPORT_HORIZ(0)), 2);
   PUSH_DATA (push, fb->width << 16);
   PUSH_DATA (push, fb->height << 16);

   if (nv50->screen->tesla->oclass >= NVA3_3D_CLASS) {
      unsigned ms = 1 << ms_mode;
      BEGIN_NV04(push, NV50_3D(CB_ADDR), 1);
      PUSH_DATA (push, (NV50_CB_AUX_SAMPLE_OFFSET << (8 - 2)) | NV50_CB_AUX);
      BEGIN_NI04(push, NV50_3D(CB_DATA(0)), 2 * ms);
      for (i = 0; i < ms; i++) {
         float xy[2];
         nv50->base.pipe.get_sample_position(&nv50->base.pipe, ms, i, xy);
         PUSH_DATAf(push, xy[0]);
         PUSH_DATAf(push, xy[1]);
      }
   }
}

static void
nv50_validate_blend_colour(struct nv50_context *nv50)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;

   BEGIN_NV04(push, NV50_3D(BLEND_COLOR(0)), 4);
   PUSH_DATAf(push, nv50->blend_colour.color[0]);
   PUSH_DATAf(push, nv50->blend_colour.color[1]);
   PUSH_DATAf(push, nv50->blend_colour.color[2]);
   PUSH_DATAf(push, nv50->blend_colour.color[3]);
}

static void
nv50_validate_stencil_ref(struct nv50_context *nv50)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;

   BEGIN_NV04(push, NV50_3D(STENCIL_FRONT_FUNC_REF), 1);
   PUSH_DATA (push, nv50->stencil_ref.ref_value[0]);
   BEGIN_NV04(push, NV50_3D(STENCIL_BACK_FUNC_REF), 1);
   PUSH_DATA (push, nv50->stencil_ref.ref_value[1]);
}

static void
nv50_validate_stipple(struct nv50_context *nv50)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   unsigned i;

   BEGIN_NV04(push, NV50_3D(POLYGON_STIPPLE_PATTERN(0)), 32);
   for (i = 0; i < 32; ++i)
      PUSH_DATA(push, util_bswap32(nv50->stipple.stipple[i]));
}

static void
nv50_validate_scissor(struct nv50_context *nv50)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
#ifdef NV50_SCISSORS_CLIPPING
   int minx, maxx, miny, maxy, i;
   bool rast_scissor = nv50->rast ? nv50->rast->pipe.scissor : false;

   if (!(nv50->dirty_3d &
         (NV50_NEW_3D_SCISSOR | NV50_NEW_3D_VIEWPORT | NV50_NEW_3D_FRAMEBUFFER)) &&
       nv50->state.scissor == rast_scissor)
      return;

   if (nv50->state.scissor != rast_scissor)
      nv50->scissors_dirty = (1 << NV50_MAX_VIEWPORTS) - 1;

   nv50->state.scissor = rast_scissor;

   if ((nv50->dirty_3d & NV50_NEW_3D_FRAMEBUFFER) && !nv50->state.scissor)
      nv50->scissors_dirty = (1 << NV50_MAX_VIEWPORTS) - 1;

   for (i = 0; i < NV50_MAX_VIEWPORTS; i++) {
      struct pipe_scissor_state *s = &nv50->scissors[i];
      struct pipe_viewport_state *vp = &nv50->viewports[i];

      if (!(nv50->scissors_dirty & (1 << i)) &&
          !(nv50->viewports_dirty & (1 << i)))
         continue;

      if (nv50->state.scissor) {
         minx = s->minx;
         maxx = s->maxx;
         miny = s->miny;
         maxy = s->maxy;
      } else {
         minx = 0;
         maxx = nv50->framebuffer.width;
         miny = 0;
         maxy = nv50->framebuffer.height;
      }

      minx = MAX2(minx, (int)(vp->translate[0] - fabsf(vp->scale[0])));
      maxx = MIN2(maxx, (int)(vp->translate[0] + fabsf(vp->scale[0])));
      miny = MAX2(miny, (int)(vp->translate[1] - fabsf(vp->scale[1])));
      maxy = MIN2(maxy, (int)(vp->translate[1] + fabsf(vp->scale[1])));

      minx = MIN2(minx, 8192);
      maxx = MAX2(maxx, 0);
      miny = MIN2(miny, 8192);
      maxy = MAX2(maxy, 0);

      BEGIN_NV04(push, NV50_3D(SCISSOR_HORIZ(i)), 2);
      PUSH_DATA (push, (maxx << 16) | minx);
      PUSH_DATA (push, (maxy << 16) | miny);
#else
      BEGIN_NV04(push, NV50_3D(SCISSOR_HORIZ(i)), 2);
      PUSH_DATA (push, (s->maxx << 16) | s->minx);
      PUSH_DATA (push, (s->maxy << 16) | s->miny);
#endif
   }

   nv50->scissors_dirty = 0;
}

static void
nv50_validate_viewport(struct nv50_context *nv50)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   float zmin, zmax;
   int i;

   for (i = 0; i < NV50_MAX_VIEWPORTS; i++) {
      struct pipe_viewport_state *vpt = &nv50->viewports[i];

      if (!(nv50->viewports_dirty & (1 << i)))
         continue;

      BEGIN_NV04(push, NV50_3D(VIEWPORT_TRANSLATE_X(i)), 3);
      PUSH_DATAf(push, vpt->translate[0]);
      PUSH_DATAf(push, vpt->translate[1]);
      PUSH_DATAf(push, vpt->translate[2]);
      BEGIN_NV04(push, NV50_3D(VIEWPORT_SCALE_X(i)), 3);
      PUSH_DATAf(push, vpt->scale[0]);
      PUSH_DATAf(push, vpt->scale[1]);
      PUSH_DATAf(push, vpt->scale[2]);

      /* If the halfz setting ever changes, the viewports will also get
       * updated. The rast will get updated before the validate function has a
       * chance to hit, so we can just use it directly without an atom
       * dependency.
       */
      util_viewport_zmin_zmax(vpt, nv50->rast->pipe.clip_halfz, &zmin, &zmax);

#ifdef NV50_SCISSORS_CLIPPING
      BEGIN_NV04(push, NV50_3D(DEPTH_RANGE_NEAR(i)), 2);
      PUSH_DATAf(push, zmin);
      PUSH_DATAf(push, zmax);
#endif
   }

   nv50->viewports_dirty = 0;
}

static void
nv50_validate_window_rects(struct nv50_context *nv50)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   bool enable = nv50->window_rect.rects > 0 || nv50->window_rect.inclusive;
   int i;

   BEGIN_NV04(push, NV50_3D(CLIP_RECTS_EN), 1);
   PUSH_DATA (push, enable);
   if (!enable)
      return;

   BEGIN_NV04(push, NV50_3D(CLIP_RECTS_MODE), 1);
   PUSH_DATA (push, !nv50->window_rect.inclusive);
   BEGIN_NV04(push, NV50_3D(CLIP_RECT_HORIZ(0)), NV50_MAX_WINDOW_RECTANGLES * 2);
   for (i = 0; i < nv50->window_rect.rects; i++) {
      struct pipe_scissor_state *s = &nv50->window_rect.rect[i];
      PUSH_DATA(push, (s->maxx << 16) | s->minx);
      PUSH_DATA(push, (s->maxy << 16) | s->miny);
   }
   for (; i < NV50_MAX_WINDOW_RECTANGLES; i++) {
      PUSH_DATA(push, 0);
      PUSH_DATA(push, 0);
   }
}

static inline void
nv50_check_program_ucps(struct nv50_context *nv50,
                        struct nv50_program *vp, uint8_t mask)
{
   const unsigned n = util_logbase2(mask) + 1;

   if (vp->vp.clpd_nr >= n)
      return;
   nv50_program_destroy(nv50, vp);

   vp->vp.clpd_nr = n;
   if (likely(vp == nv50->vertprog)) {
      nv50->dirty_3d |= NV50_NEW_3D_VERTPROG;
      nv50_vertprog_validate(nv50);
   } else {
      nv50->dirty_3d |= NV50_NEW_3D_GMTYPROG;
      nv50_gmtyprog_validate(nv50);
   }
   nv50_fp_linkage_validate(nv50);
}

/* alpha test is disabled if there are no color RTs, so make sure we have at
 * least one if alpha test is enabled. Note that this must run after
 * nv50_validate_fb, otherwise that will override the RT count setting.
 */
static void
nv50_validate_derived_2(struct nv50_context *nv50)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;

   if (nv50->zsa && nv50->zsa->pipe.alpha_enabled &&
       nv50->framebuffer.nr_cbufs == 0) {
      nv50_fb_set_null_rt(push, 0);
      BEGIN_NV04(push, NV50_3D(RT_CONTROL), 1);
      PUSH_DATA (push, (076543210 << 4) | 1);
   }
}

static void
nv50_validate_clip(struct nv50_context *nv50)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   struct nv50_program *vp;
   uint8_t clip_enable = nv50->rast->pipe.clip_plane_enable;

   if (nv50->dirty_3d & NV50_NEW_3D_CLIP) {
      BEGIN_NV04(push, NV50_3D(CB_ADDR), 1);
      PUSH_DATA (push, NV50_CB_AUX_UCP_OFFSET << (8 - 2) | NV50_CB_AUX);
      BEGIN_NI04(push, NV50_3D(CB_DATA(0)), PIPE_MAX_CLIP_PLANES * 4);
      PUSH_DATAp(push, &nv50->clip.ucp[0][0], PIPE_MAX_CLIP_PLANES * 4);
   }

   vp = nv50->gmtyprog;
   if (likely(!vp))
      vp = nv50->vertprog;

   if (clip_enable)
      nv50_check_program_ucps(nv50, vp, clip_enable);

   clip_enable &= vp->vp.clip_enable;
   clip_enable |= vp->vp.cull_enable;

   BEGIN_NV04(push, NV50_3D(CLIP_DISTANCE_ENABLE), 1);
   PUSH_DATA (push, clip_enable);

   if (nv50->state.clip_mode != vp->vp.clip_mode) {
      nv50->state.clip_mode = vp->vp.clip_mode;
      BEGIN_NV04(push, NV50_3D(CLIP_DISTANCE_MODE), 1);
      PUSH_DATA (push, vp->vp.clip_mode);
   }
}

static void
nv50_validate_blend(struct nv50_context *nv50)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;

   PUSH_SPACE(push, nv50->blend->size);
   PUSH_DATAp(push, nv50->blend->state, nv50->blend->size);
}

static void
nv50_validate_zsa(struct nv50_context *nv50)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;

   PUSH_SPACE(push, nv50->zsa->size);
   PUSH_DATAp(push, nv50->zsa->state, nv50->zsa->size);
}

static void
nv50_validate_rasterizer(struct nv50_context *nv50)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;

   PUSH_SPACE(push, nv50->rast->size);
   PUSH_DATAp(push, nv50->rast->state, nv50->rast->size);
}

static void
nv50_validate_sample_mask(struct nv50_context *nv50)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;

   unsigned mask[4] =
   {
      nv50->sample_mask & 0xffff,
      nv50->sample_mask & 0xffff,
      nv50->sample_mask & 0xffff,
      nv50->sample_mask & 0xffff
   };

   BEGIN_NV04(push, NV50_3D(MSAA_MASK(0)), 4);
   PUSH_DATA (push, mask[0]);
   PUSH_DATA (push, mask[1]);
   PUSH_DATA (push, mask[2]);
   PUSH_DATA (push, mask[3]);
}

static void
nv50_validate_min_samples(struct nv50_context *nv50)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   int samples;

   if (nv50->screen->tesla->oclass < NVA3_3D_CLASS)
      return;

   samples = util_next_power_of_two(nv50->min_samples);
   if (samples > 1)
      samples |= NVA3_3D_SAMPLE_SHADING_ENABLE;

   BEGIN_NV04(push, SUBC_3D(NVA3_3D_SAMPLE_SHADING), 1);
   PUSH_DATA (push, samples);
}

static void
nv50_switch_pipe_context(struct nv50_context *ctx_to)
{
   struct nv50_context *ctx_from = ctx_to->screen->cur_ctx;

   simple_mtx_assert_locked(&ctx_to->screen->state_lock);
   if (ctx_from)
      ctx_to->state = ctx_from->state;
   else
      ctx_to->state = ctx_to->screen->save_state;

   ctx_to->dirty_3d = ~0;
   ctx_to->dirty_cp = ~0;
   ctx_to->viewports_dirty = ~0;
   ctx_to->scissors_dirty = ~0;

   ctx_to->constbuf_dirty[NV50_SHADER_STAGE_VERTEX] =
   ctx_to->constbuf_dirty[NV50_SHADER_STAGE_GEOMETRY] =
   ctx_to->constbuf_dirty[NV50_SHADER_STAGE_FRAGMENT] = (1 << NV50_MAX_PIPE_CONSTBUFS) - 1;

   if (!ctx_to->vertex)
      ctx_to->dirty_3d &= ~(NV50_NEW_3D_VERTEX | NV50_NEW_3D_ARRAYS);

   if (!ctx_to->vertprog)
      ctx_to->dirty_3d &= ~NV50_NEW_3D_VERTPROG;
   if (!ctx_to->fragprog)
      ctx_to->dirty_3d &= ~NV50_NEW_3D_FRAGPROG;

   if (!ctx_to->blend)
      ctx_to->dirty_3d &= ~NV50_NEW_3D_BLEND;
   if (!ctx_to->rast)
#ifdef NV50_SCISSORS_CLIPPING
      ctx_to->dirty_3d &= ~(NV50_NEW_3D_RASTERIZER | NV50_NEW_3D_SCISSOR);
#else
      ctx_to->dirty_3d &= ~NV50_NEW_3D_RASTERIZER;
#endif
   if (!ctx_to->zsa)
      ctx_to->dirty_3d &= ~NV50_NEW_3D_ZSA;

   ctx_to->screen->cur_ctx = ctx_to;
}

static struct nv50_state_validate
validate_list_3d[] = {
    { nv50_validate_fb,            NV50_NEW_3D_FRAMEBUFFER },
    { nv50_validate_blend,         NV50_NEW_3D_BLEND },
    { nv50_validate_zsa,           NV50_NEW_3D_ZSA },
    { nv50_validate_sample_mask,   NV50_NEW_3D_SAMPLE_MASK },
    { nv50_validate_rasterizer,    NV50_NEW_3D_RASTERIZER },
    { nv50_validate_blend_colour,  NV50_NEW_3D_BLEND_COLOUR },
    { nv50_validate_stencil_ref,   NV50_NEW_3D_STENCIL_REF },
    { nv50_validate_stipple,       NV50_NEW_3D_STIPPLE },
#ifdef NV50_SCISSORS_CLIPPING
    { nv50_validate_scissor,       NV50_NEW_3D_SCISSOR | NV50_NEW_3D_VIEWPORT |
                                   NV50_NEW_3D_RASTERIZER |
                                   NV50_NEW_3D_FRAMEBUFFER },
#else
    { nv50_validate_scissor,       NV50_NEW_3D_SCISSOR },
#endif
    { nv50_validate_viewport,      NV50_NEW_3D_VIEWPORT },
    { nv50_validate_window_rects,  NV50_NEW_3D_WINDOW_RECTS },
    { nv50_vertprog_validate,      NV50_NEW_3D_VERTPROG },
    { nv50_gmtyprog_validate,      NV50_NEW_3D_GMTYPROG },
    { nv50_fragprog_validate,      NV50_NEW_3D_FRAGPROG | NV50_NEW_3D_RASTERIZER |
                                   NV50_NEW_3D_MIN_SAMPLES | NV50_NEW_3D_ZSA |
                                   NV50_NEW_3D_FRAMEBUFFER},
    { nv50_fp_linkage_validate,    NV50_NEW_3D_FRAGPROG | NV50_NEW_3D_VERTPROG |
                                   NV50_NEW_3D_GMTYPROG | NV50_NEW_3D_RASTERIZER },
    { nv50_gp_linkage_validate,    NV50_NEW_3D_GMTYPROG | NV50_NEW_3D_VERTPROG },
    { nv50_validate_derived_rs,    NV50_NEW_3D_FRAGPROG | NV50_NEW_3D_RASTERIZER |
                                   NV50_NEW_3D_VERTPROG | NV50_NEW_3D_GMTYPROG },
    { nv50_validate_derived_2,     NV50_NEW_3D_ZSA | NV50_NEW_3D_FRAMEBUFFER },
    { nv50_validate_clip,          NV50_NEW_3D_CLIP | NV50_NEW_3D_RASTERIZER |
                                   NV50_NEW_3D_VERTPROG | NV50_NEW_3D_GMTYPROG },
    { nv50_constbufs_validate,     NV50_NEW_3D_CONSTBUF },
    { nv50_validate_textures,      NV50_NEW_3D_TEXTURES },
    { nv50_validate_samplers,      NV50_NEW_3D_SAMPLERS },
    { nv50_stream_output_validate, NV50_NEW_3D_STRMOUT |
                                   NV50_NEW_3D_VERTPROG | NV50_NEW_3D_GMTYPROG },
    { nv50_vertex_arrays_validate, NV50_NEW_3D_VERTEX | NV50_NEW_3D_ARRAYS },
    { nv50_validate_min_samples,   NV50_NEW_3D_MIN_SAMPLES },
};

bool
nv50_state_validate(struct nv50_context *nv50, uint32_t mask,
                    struct nv50_state_validate *validate_list, int size,
                    uint32_t *dirty, struct nouveau_bufctx *bufctx)
{
   uint32_t state_mask;
   int ret;
   unsigned i;

   simple_mtx_assert_locked(&nv50->screen->state_lock);

   if (nv50->screen->cur_ctx != nv50)
      nv50_switch_pipe_context(nv50);

   state_mask = *dirty & mask;

   if (state_mask) {
      for (i = 0; i < size; i++) {
         struct nv50_state_validate *validate = &validate_list[i];

         if (state_mask & validate->states)
            validate->func(nv50);
      }
      *dirty &= ~state_mask;

      if (nv50->state.rt_serialize) {
         nv50->state.rt_serialize = false;
         BEGIN_NV04(nv50->base.pushbuf, SUBC_3D(NV50_GRAPH_SERIALIZE), 1);
         PUSH_DATA (nv50->base.pushbuf, 0);
      }

      nv50_bufctx_fence(nv50, bufctx, false);
   }
   nouveau_pushbuf_bufctx(nv50->base.pushbuf, bufctx);
   ret = PUSH_VAL(nv50->base.pushbuf);

   return !ret;
}

bool
nv50_state_validate_3d(struct nv50_context *nv50, uint32_t mask)
{
   bool ret;

   ret = nv50_state_validate(nv50, mask, validate_list_3d,
                             ARRAY_SIZE(validate_list_3d), &nv50->dirty_3d,
                             nv50->bufctx_3d);

   if (unlikely(nv50->state.flushed)) {
      nv50->state.flushed = false;
      nv50_bufctx_fence(nv50, nv50->bufctx_3d, true);
   }
   return ret;
}
