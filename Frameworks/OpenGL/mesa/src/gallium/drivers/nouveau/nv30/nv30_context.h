#ifndef __NV30_CONTEXT_H__
#define __NV30_CONTEXT_H__

#include "util/format/u_formats.h"
#include "util/u_blitter.h"

#include "nv30/nv30_screen.h"
#include "nv30/nv30_state.h"

#include "nouveau_context.h"

#define BUFCTX_FB          0
#define BUFCTX_VTXTMP      1
#define BUFCTX_VTXBUF      2
#define BUFCTX_IDXBUF      3
#define BUFCTX_VERTTEX(n) (4 + (n))
#define BUFCTX_FRAGPROG    8
#define BUFCTX_FRAGTEX(n) (9 + (n))

#define NV30_NEW_BLEND        (1 << 0)
#define NV30_NEW_RASTERIZER   (1 << 1)
#define NV30_NEW_ZSA          (1 << 2)
#define NV30_NEW_VERTPROG     (1 << 3)
#define NV30_NEW_VERTCONST    (1 << 4)
#define NV30_NEW_FRAGPROG     (1 << 5)
#define NV30_NEW_FRAGCONST    (1 << 6)
#define NV30_NEW_BLEND_COLOUR (1 << 7)
#define NV30_NEW_STENCIL_REF  (1 << 8)
#define NV30_NEW_CLIP         (1 << 9)
#define NV30_NEW_SAMPLE_MASK  (1 << 10)
#define NV30_NEW_FRAMEBUFFER  (1 << 11)
#define NV30_NEW_STIPPLE      (1 << 12)
#define NV30_NEW_SCISSOR      (1 << 13)
#define NV30_NEW_VIEWPORT     (1 << 14)
#define NV30_NEW_ARRAYS       (1 << 15)
#define NV30_NEW_VERTEX       (1 << 16)
#define NV30_NEW_CONSTBUF     (1 << 17)
#define NV30_NEW_FRAGTEX      (1 << 18)
#define NV30_NEW_VERTTEX      (1 << 19)
#define NV30_NEW_SWTNL        (1 << 31)
#define NV30_NEW_ALL          0x000fffff

struct nv30_context {
   struct nouveau_context base;
   struct nv30_screen *screen;
   struct blitter_context *blitter;

   struct nouveau_bufctx *bufctx;

   struct {
      unsigned rt_enable;
      unsigned scissor_off;
      unsigned num_vtxelts;
      int index_bias;
      bool prim_restart;
      struct nv30_fragprog *fragprog;
   } state;

   uint32_t dirty;

   struct draw_context *draw;
   uint32_t draw_flags;
   uint32_t draw_dirty;

   struct nv30_blend_stateobj *blend;
   struct nv30_rasterizer_stateobj *rast;
   struct nv30_zsa_stateobj *zsa;
   struct nv30_vertex_stateobj *vertex;

   struct {
      unsigned filter;
      unsigned aniso;
   } config;

   struct {
      struct nv30_vertprog *program;

      struct pipe_resource *constbuf;
      unsigned constbuf_nr;

      struct pipe_sampler_view *textures[PIPE_MAX_SAMPLERS];
      unsigned num_textures;
      struct nv30_sampler_state *samplers[PIPE_MAX_SAMPLERS];
      unsigned num_samplers;
      unsigned dirty_samplers;
   } vertprog;

   struct {
      struct nv30_fragprog *program;

      struct pipe_resource *constbuf;
      unsigned constbuf_nr;

      struct pipe_sampler_view *textures[PIPE_MAX_SAMPLERS];
      unsigned num_textures;
      struct nv30_sampler_state *samplers[PIPE_MAX_SAMPLERS];
      unsigned num_samplers;
      unsigned dirty_samplers;
   } fragprog;

   struct pipe_framebuffer_state framebuffer;
   struct pipe_blend_color blend_colour;
   struct pipe_stencil_ref stencil_ref;
   struct pipe_poly_stipple stipple;
   struct pipe_scissor_state scissor;
   struct pipe_viewport_state viewport;
   struct pipe_clip_state clip;

   unsigned sample_mask;

   struct pipe_vertex_buffer vtxbuf[PIPE_MAX_ATTRIBS];
   unsigned num_vtxbufs;
   uint32_t vbo_fifo;
   uint32_t vbo_user;
   unsigned vbo_min_index;
   unsigned vbo_max_index;
   bool vbo_push_hint;

   struct nouveau_heap  *blit_vp;
   struct pipe_resource *blit_fp;

   struct pipe_query *render_cond_query;
   unsigned render_cond_mode;
   bool render_cond_cond;
};

static inline struct nv30_context *
nv30_context(struct pipe_context *pipe)
{
   return (struct nv30_context *)pipe;
}

struct pipe_context *
nv30_context_create(struct pipe_screen *pscreen, void *priv, unsigned flags);

void
nv30_vbo_init(struct pipe_context *pipe);

void
nv30_vbo_validate(struct nv30_context *nv30);

void
nv30_query_init(struct pipe_context *pipe);

void
nv30_state_init(struct pipe_context *pipe);

void
nv30_clear_init(struct pipe_context *pipe);

void
nv30_vertprog_init(struct pipe_context *pipe);

void
nv30_vertprog_validate(struct nv30_context *nv30);

void
nv30_fragprog_init(struct pipe_context *pipe);

void
nv30_fragprog_validate(struct nv30_context *nv30);

void
nv30_texture_init(struct pipe_context *pipe);

void
nv30_texture_validate(struct nv30_context *nv30);

void
nv30_fragtex_init(struct pipe_context *pipe);

void
nv30_fragtex_validate(struct nv30_context *nv30);

void
nv40_verttex_init(struct pipe_context *pipe);

void
nv40_verttex_validate(struct nv30_context *nv30);

void
nv30_fragtex_sampler_states_bind(struct pipe_context *pipe,
                                 unsigned nr, void **hwcso);

void
nv40_verttex_sampler_states_bind(struct pipe_context *pipe,
                                 unsigned nr, void **hwcso);

void
nv40_verttex_set_sampler_views(struct pipe_context *pipe, unsigned nr,
                               bool take_ownership,
                               struct pipe_sampler_view **views);

void
nv30_fragtex_set_sampler_views(struct pipe_context *pipe,
                               unsigned nr, bool take_ownership,
                               struct pipe_sampler_view **views);

void
nv30_push_vbo(struct nv30_context *nv30, const struct pipe_draw_info *info,
              const struct pipe_draw_start_count_bias *draw);

void
nv30_draw_init(struct pipe_context *pipe);

void
nv30_render_vbo(struct pipe_context *pipe, const struct pipe_draw_info *info,
                unsigned drawid_offset,
                const struct pipe_draw_start_count_bias *draw);

bool
nv30_state_validate(struct nv30_context *nv30, uint32_t mask, bool hwtnl);

void
nv30_state_release(struct nv30_context *nv30);

#ifdef NV30_3D_VERTEX_BEGIN_END
#define NV30_PRIM_GL_CASE(n) \
   case MESA_PRIM_##n: return NV30_3D_VERTEX_BEGIN_END_##n

static inline unsigned
nv30_prim_gl(unsigned prim)
{
   switch (prim) {
   NV30_PRIM_GL_CASE(POINTS);
   NV30_PRIM_GL_CASE(LINES);
   NV30_PRIM_GL_CASE(LINE_LOOP);
   NV30_PRIM_GL_CASE(LINE_STRIP);
   NV30_PRIM_GL_CASE(TRIANGLES);
   NV30_PRIM_GL_CASE(TRIANGLE_STRIP);
   NV30_PRIM_GL_CASE(TRIANGLE_FAN);
   NV30_PRIM_GL_CASE(QUADS);
   NV30_PRIM_GL_CASE(QUAD_STRIP);
   NV30_PRIM_GL_CASE(POLYGON);
   default:
      return NV30_3D_VERTEX_BEGIN_END_POINTS;
      break;
   }
}
#endif

#endif
