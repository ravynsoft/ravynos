#ifndef __NVC0_CONTEXT_H__
#define __NVC0_CONTEXT_H__

#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "pipe/p_state.h"

#include "util/list.h"
#include "util/u_memory.h"
#include "util/u_math.h"
#include "util/u_inlines.h"
#include "util/u_dynarray.h"

#include "nvc0/nvc0_winsys.h"
#include "nvc0/nvc0_stateobj.h"
#include "nvc0/nvc0_screen.h"
#include "nvc0/nvc0_program.h"
#include "nvc0/nvc0_resource.h"
#include "nvc0/nvc0_query.h"

#include "nv50/nv50_transfer.h"

#include "nouveau_context.h"
#include "nouveau_debug.h"

#include "nv50/nv50_3ddefs.xml.h"
#include "nvc0/nvc0_3d.xml.h"
#include "nv50/nv50_2d.xml.h"
#include "nvc0/nvc0_m2mf.xml.h"
#include "nvc0/nve4_copy.xml.h"
#include "nvc0/nve4_p2mf.xml.h"
#include "nvc0/nvc0_compute.xml.h"
#include "nvc0/nvc0_macros.h"

/* NOTE: must keep NVC0_NEW_3D_...PROG in consecutive bits in this order */
#define NVC0_NEW_3D_BLEND        (1 << 0)
#define NVC0_NEW_3D_RASTERIZER   (1 << 1)
#define NVC0_NEW_3D_ZSA          (1 << 2)
#define NVC0_NEW_3D_VERTPROG     (1 << 3)
#define NVC0_NEW_3D_TCTLPROG     (1 << 4)
#define NVC0_NEW_3D_TEVLPROG     (1 << 5)
#define NVC0_NEW_3D_GMTYPROG     (1 << 6)
#define NVC0_NEW_3D_FRAGPROG     (1 << 7)
#define NVC0_NEW_3D_BLEND_COLOUR (1 << 8)
#define NVC0_NEW_3D_STENCIL_REF  (1 << 9)
#define NVC0_NEW_3D_CLIP         (1 << 10)
#define NVC0_NEW_3D_SAMPLE_MASK  (1 << 11)
#define NVC0_NEW_3D_FRAMEBUFFER  (1 << 12)
#define NVC0_NEW_3D_STIPPLE      (1 << 13)
#define NVC0_NEW_3D_SCISSOR      (1 << 14)
#define NVC0_NEW_3D_VIEWPORT     (1 << 15)
#define NVC0_NEW_3D_ARRAYS       (1 << 16)
#define NVC0_NEW_3D_VERTEX       (1 << 17)
#define NVC0_NEW_3D_CONSTBUF     (1 << 18)
#define NVC0_NEW_3D_TEXTURES     (1 << 19)
#define NVC0_NEW_3D_SAMPLERS     (1 << 20)
#define NVC0_NEW_3D_TFB_TARGETS  (1 << 21)

#define NVC0_NEW_3D_SURFACES     (1 << 23)
#define NVC0_NEW_3D_MIN_SAMPLES  (1 << 24)
#define NVC0_NEW_3D_TESSFACTOR   (1 << 25)
#define NVC0_NEW_3D_BUFFERS      (1 << 26)
#define NVC0_NEW_3D_DRIVERCONST  (1 << 27)
#define NVC0_NEW_3D_WINDOW_RECTS (1 << 28)

#define NVC0_NEW_3D_SAMPLE_LOCATIONS (1 << 29)

#define NVC0_NEW_CP_PROGRAM   (1 << 0)
#define NVC0_NEW_CP_SURFACES  (1 << 1)
#define NVC0_NEW_CP_TEXTURES  (1 << 2)
#define NVC0_NEW_CP_SAMPLERS  (1 << 3)
#define NVC0_NEW_CP_CONSTBUF  (1 << 4)
#define NVC0_NEW_CP_GLOBALS   (1 << 5)
#define NVC0_NEW_CP_DRIVERCONST (1 << 6)
#define NVC0_NEW_CP_BUFFERS   (1 << 7)

/* 3d bufctx (during draw_vbo, blit_3d) */
#define NVC0_BIND_3D_FB            0
#define NVC0_BIND_3D_VTX           1
#define NVC0_BIND_3D_VTX_TMP       2
#define NVC0_BIND_3D_IDX           3
#define NVC0_BIND_3D_TEX(s, i)  (  4 + 32 * (s) + (i))
#define NVC0_BIND_3D_CB(s, i)   (164 + 16 * (s) + (i))
#define NVC0_BIND_3D_TFB         244
#define NVC0_BIND_3D_SUF         245
#define NVC0_BIND_3D_BUF         246
#define NVC0_BIND_3D_SCREEN      247
#define NVC0_BIND_3D_BINDLESS    248
#define NVC0_BIND_3D_TLS         249
#define NVC0_BIND_3D_TEXT        250
#define NVC0_BIND_3D_COUNT       251

/* compute bufctx (during launch_grid) */
#define NVC0_BIND_CP_CB(i)     (  0 + (i))
#define NVC0_BIND_CP_TEX(i)    ( 16 + (i))
#define NVC0_BIND_CP_SUF         48
#define NVC0_BIND_CP_GLOBAL      49
#define NVC0_BIND_CP_DESC        50
#define NVC0_BIND_CP_SCREEN      51
#define NVC0_BIND_CP_QUERY       52
#define NVC0_BIND_CP_BUF         53
#define NVC0_BIND_CP_TEXT        54
#define NVC0_BIND_CP_BINDLESS    55
#define NVC0_BIND_CP_COUNT       56

/* bufctx for other operations */
#define NVC0_BIND_2D            0
#define NVC0_BIND_M2MF          0
#define NVC0_BIND_FENCE         1

/* 6 user uniform buffers, at 64K each */
#define NVC0_CB_USR_INFO(s)         (s << 16)
#define NVC0_CB_USR_SIZE            (6 << 16)
/* 6 driver constbuts, at 64K each */
#define NVC0_CB_AUX_INFO(s)         NVC0_CB_USR_SIZE + (s << 16)
#define NVC0_CB_AUX_SIZE            (1 << 16)
/* XXX: Figure out what this UNK data is. */
#define NVC0_CB_AUX_UNK_INFO        0x000
#define NVC0_CB_AUX_UNK_SIZE        (8 * 4)
/* 40 textures handles (8 for GM107+ images only), at 1 32-bits integer each */
#define NVC0_CB_AUX_TEX_INFO(i)     0x020 + (i) * 4
#define NVC0_CB_AUX_TEX_SIZE        (40 * 4)
/* 8 sets of 32-bits coordinate offsets */
#define NVC0_CB_AUX_MS_INFO         0x0c0
#define NVC0_CB_AUX_MS_SIZE         (8 * 2 * 4)
/* block/grid size, at 3 32-bits integers each, gridid and work_dim */
#define NVC0_CB_AUX_GRID_INFO(i)    0x100 + (i) * 4 /* CP */
#define NVC0_CB_AUX_GRID_SIZE       (8 * 4)
/* FB texture handle */
#define NVC0_CB_AUX_FB_TEX_INFO     0x100 /* FP */
#define NVC0_CB_AUX_FB_TEX_SIZE     (4)
/* 8 user clip planes, at 4 32-bits floats each */
#define NVC0_CB_AUX_UCP_INFO        0x120
#define NVC0_CB_AUX_UCP_SIZE        (PIPE_MAX_CLIP_PLANES * 4 * 4)
/* 13 ubos, at 4 32-bits integer each */
#define NVC0_CB_AUX_UBO_INFO(i)     0x120 + (i) * 4 * 4 /* CP */
#define NVC0_CB_AUX_UBO_SIZE        ((NVC0_MAX_PIPE_CONSTBUFS - 1) * 4 * 4)
/* 8 sets of 32-bits integer pairs sample offsets */
#define NVC0_CB_AUX_SAMPLE_INFO     0x1a0 /* FP */
/* 256 bytes, though only 64 bytes used before GM200 */
#define NVC0_CB_AUX_SAMPLE_SIZE     (8 * 2 * 4 * 4)
/* draw parameters (index bias, base instance, drawid)
 * be sure to update the indirect draw macros in com9097.mme when changing this
 */
#define NVC0_CB_AUX_DRAW_INFO       0x1a0 /* VP */
/* 32 user buffers, at 4 32-bits integers each */
#define NVC0_CB_AUX_BUF_INFO(i)     0x2a0 + (i) * 4 * 4
#define NVC0_CB_AUX_BUF_SIZE        (NVC0_MAX_BUFFERS * 4 * 4)
/* 8 surfaces, at 16 32-bits integers each */
#define NVC0_CB_AUX_SU_INFO(i)      0x4a0 + (i) * 16 * 4
#define NVC0_CB_AUX_SU_SIZE         (NVC0_MAX_IMAGES * 16 * 4)
/* 1 64-bits address and 1 32-bits sequence
 * be sure to update the shaders in nvc0_query_hw_sm.c when changing this
 */
#define NVC0_CB_AUX_MP_INFO         0x6a0
#define NVC0_CB_AUX_MP_SIZE         3 * 4
/* 512 64-byte blocks for bindless image handles */
#define NVC0_CB_AUX_BINDLESS_INFO(i) 0x6b0 + (i) * 16 * 4
#define NVC0_CB_AUX_BINDLESS_SIZE   (NVE4_IMG_MAX_HANDLES * 16 * 4)
/* 4 32-bits floats for the vertex runout, put at the end */
#define NVC0_CB_AUX_RUNOUT_INFO     NVC0_CB_USR_SIZE + (NVC0_CB_AUX_SIZE * 6)

struct nvc0_blitctx;

bool nvc0_blitctx_create(struct nvc0_context *);
void nvc0_blitctx_destroy(struct nvc0_context *);

struct nvc0_resident {
   struct list_head list;
   uint64_t handle;
   struct nv04_resource *buf;
   uint32_t flags;
};

struct nvc0_context {
   struct nouveau_context base;

   struct nouveau_bufctx *bufctx_3d;
   struct nouveau_bufctx *bufctx;
   struct nouveau_bufctx *bufctx_cp;

   struct nvc0_screen *screen;

   void (*m2mf_copy_rect)(struct nvc0_context *,
                          const struct nv50_m2mf_rect *dst,
                          const struct nv50_m2mf_rect *src,
                          uint32_t nblocksx, uint32_t nblocksy);

   uint32_t dirty_3d; /* dirty flags for 3d state */
   uint32_t dirty_cp; /* dirty flags for compute state */

   struct nvc0_graph_state state;

   struct nvc0_blend_stateobj *blend;
   struct nvc0_rasterizer_stateobj *rast;
   struct nvc0_zsa_stateobj *zsa;
   struct nvc0_vertex_stateobj *vertex;

   struct nvc0_program *vertprog;
   struct nvc0_program *tctlprog;
   struct nvc0_program *tevlprog;
   struct nvc0_program *gmtyprog;
   struct nvc0_program *fragprog;
   struct nvc0_program *compprog;

   struct nvc0_program *tcp_empty;

   struct nvc0_constbuf constbuf[6][NVC0_MAX_PIPE_CONSTBUFS];
   uint16_t constbuf_dirty[6];
   uint16_t constbuf_valid[6];
   uint16_t constbuf_coherent[6];
   bool cb_dirty;

   struct pipe_vertex_buffer vtxbuf[PIPE_MAX_ATTRIBS];
   unsigned num_vtxbufs;
   uint32_t vtxbufs_coherent;
   uint32_t constant_vbos;
   uint32_t vbo_user; /* bitmask of vertex buffers pointing to user memory */
   uint32_t vb_elt_first; /* from pipe_draw_info, for vertex upload */
   uint32_t vb_elt_limit; /* max - min element (count - 1) */
   uint32_t instance_off; /* current base vertex for instanced arrays */
   uint32_t instance_max; /* last instance for current draw call */

   struct pipe_sampler_view *textures[6][PIPE_MAX_SAMPLERS];
   unsigned num_textures[6];
   uint32_t textures_dirty[6];
   uint32_t textures_coherent[6];
   struct nv50_tsc_entry *samplers[6][PIPE_MAX_SAMPLERS];
   unsigned num_samplers[6];
   uint32_t samplers_dirty[6];
   bool seamless_cube_map;
   struct pipe_sampler_view *fbtexture;

   uint32_t tex_handles[6][PIPE_MAX_SAMPLERS]; /* for nve4 */

   struct list_head tex_head;
   struct list_head img_head;

   struct pipe_framebuffer_state framebuffer;
   bool sample_locations_enabled;
   uint8_t sample_locations[2 * 4 * 8];
   struct pipe_blend_color blend_colour;
   struct pipe_stencil_ref stencil_ref;
   struct pipe_poly_stipple stipple;
   struct pipe_scissor_state scissors[NVC0_MAX_VIEWPORTS];
   unsigned scissors_dirty;
   struct pipe_viewport_state viewports[NVC0_MAX_VIEWPORTS];
   unsigned viewports_dirty;
   struct pipe_clip_state clip;
   struct nvc0_window_rect_stateobj window_rect;

   unsigned sample_mask;
   unsigned min_samples;

   float default_tess_outer[4];
   float default_tess_inner[2];
   uint8_t patch_vertices;

   bool vbo_push_hint;

   uint8_t tfbbuf_dirty;
   struct pipe_stream_output_target *tfbbuf[4];
   unsigned num_tfbbufs;

   struct pipe_query *cond_query;
   bool cond_cond; /* inverted rendering condition */
   uint cond_mode;
   uint32_t cond_condmode; /* the calculated condition */

   struct nvc0_blitctx *blit;

   /* NOTE: some of these surfaces may reference buffers */
   struct pipe_surface *surfaces[2][NVC0_MAX_SURFACE_SLOTS];
   uint16_t surfaces_dirty[2];
   uint16_t surfaces_valid[2];

   struct pipe_shader_buffer buffers[6][NVC0_MAX_BUFFERS];
   uint32_t buffers_dirty[6];
   uint32_t buffers_valid[6];

   struct pipe_image_view images[6][NVC0_MAX_IMAGES];
   struct pipe_sampler_view *images_tic[6][NVC0_MAX_IMAGES]; /* GM107+ */
   uint16_t images_dirty[6];
   uint16_t images_valid[6];

   struct util_dynarray global_residents;

   uint64_t compute_invocations;
};

static inline struct nvc0_context *
nvc0_context(struct pipe_context *pipe)
{
   return (struct nvc0_context *)pipe;
}

static inline unsigned
nvc0_shader_stage(unsigned pipe)
{
   switch (pipe) {
   case PIPE_SHADER_VERTEX: return 0;
   case PIPE_SHADER_TESS_CTRL: return 1;
   case PIPE_SHADER_TESS_EVAL: return 2;
   case PIPE_SHADER_GEOMETRY: return 3;
   case PIPE_SHADER_FRAGMENT: return 4;
   case PIPE_SHADER_COMPUTE: return 5;
   default:
      assert(!"invalid PIPE_SHADER type");
      return 0;
   }
}

static inline void
nvc0_resource_fence(struct nvc0_context *nvc0, struct nv04_resource *res, uint32_t flags)
{
   if (res->mm) {
      nouveau_fence_ref(nvc0->base.fence, &res->fence);
      if (flags & NOUVEAU_BO_WR)
         nouveau_fence_ref(nvc0->base.fence, &res->fence_wr);
   }
}

static inline void
nvc0_resource_validate(struct nvc0_context *nvc0, struct nv04_resource *res, uint32_t flags)
{
   if (likely(res->bo)) {
      if (flags & NOUVEAU_BO_WR)
         res->status |= NOUVEAU_BUFFER_STATUS_GPU_WRITING |
            NOUVEAU_BUFFER_STATUS_DIRTY;
      if (flags & NOUVEAU_BO_RD)
         res->status |= NOUVEAU_BUFFER_STATUS_GPU_READING;

      nvc0_resource_fence(nvc0, res, flags);
   }
}

/* nvc0_context.c */
struct pipe_context *nvc0_create(struct pipe_screen *, void *, unsigned flags);
void nvc0_bufctx_fence(struct nvc0_context *, struct nouveau_bufctx *,
                       bool on_flush);
void nvc0_default_kick_notify(struct nouveau_context *);
const void *nvc0_get_sample_locations(unsigned);

/* nvc0_draw.c */
extern struct draw_stage *nvc0_draw_render_stage(struct nvc0_context *);

/* nvc0_program.c */
bool nvc0_program_translate(struct nvc0_program *, uint16_t chipset,
                            struct disk_cache *,
                            struct util_debug_callback *);
bool nvc0_program_upload(struct nvc0_context *, struct nvc0_program *);
void nvc0_program_destroy(struct nvc0_context *, struct nvc0_program *);
void nvc0_program_library_upload(struct nvc0_context *);
void nvc0_program_init_tcp_empty(struct nvc0_context *);

/* nvc0_shader_state.c */
void nvc0_vertprog_validate(struct nvc0_context *);
void nvc0_tctlprog_validate(struct nvc0_context *);
void nvc0_tevlprog_validate(struct nvc0_context *);
void nvc0_gmtyprog_validate(struct nvc0_context *);
void nvc0_fragprog_validate(struct nvc0_context *);
void nvc0_compprog_validate(struct nvc0_context *);

void nvc0_tfb_validate(struct nvc0_context *);
void nvc0_layer_validate(struct nvc0_context *);

/* nvc0_state.c */
extern void nvc0_init_state_functions(struct nvc0_context *);

/* nvc0_state_validate.c */
struct nvc0_state_validate {
   void (*func)(struct nvc0_context *);
   uint32_t states;
};

bool nvc0_state_validate(struct nvc0_context *, uint32_t,
                         struct nvc0_state_validate *, int, uint32_t *,
                         struct nouveau_bufctx *);
bool nvc0_state_validate_3d(struct nvc0_context *, uint32_t);

/* nvc0_surface.c */
extern void nvc0_clear(struct pipe_context *, unsigned buffers,
                       const struct pipe_scissor_state *scissor_state,
                       const union pipe_color_union *color,
                       double depth, unsigned stencil);
extern void nvc0_init_surface_functions(struct nvc0_context *);

/* nvc0_tex.c */
bool nvc0_validate_tic(struct nvc0_context *nvc0, int s);
bool nvc0_validate_tsc(struct nvc0_context *nvc0, int s);
bool nve4_validate_tsc(struct nvc0_context *nvc0, int s);
void nvc0_validate_suf(struct nvc0_context *nvc0, int s);
void nvc0_validate_textures(struct nvc0_context *);
void nvc0_validate_samplers(struct nvc0_context *);
void nvc0_upload_tsc0(struct nvc0_context *);
void nve4_set_tex_handles(struct nvc0_context *);
void nvc0_validate_surfaces(struct nvc0_context *);
void nve4_set_surface_info(struct nouveau_pushbuf *,
                           const struct pipe_image_view *,
                           struct nvc0_context *);
void nvc0_mark_image_range_valid(const struct pipe_image_view *);
bool nvc0_update_tic(struct nvc0_context *, struct nv50_tic_entry *,
                     struct nv04_resource *);

struct pipe_sampler_view *
nvc0_create_texture_view(struct pipe_context *,
                         struct pipe_resource *,
                         const struct pipe_sampler_view *,
                         uint32_t flags);
struct pipe_sampler_view *
nvc0_create_sampler_view(struct pipe_context *,
                         struct pipe_resource *,
                         const struct pipe_sampler_view *);
struct pipe_sampler_view *
gm107_create_texture_view_from_image(struct pipe_context *,
                                     const struct pipe_image_view *);

void nvc0_init_bindless_functions(struct pipe_context *);

/* nvc0_transfer.c */
void
nvc0_init_transfer_functions(struct nvc0_context *);

void
nvc0_m2mf_push_linear(struct nouveau_context *nv,
                      struct nouveau_bo *dst, unsigned offset, unsigned domain,
                      unsigned size, const void *data);
void
nve4_p2mf_push_linear(struct nouveau_context *nv,
                      struct nouveau_bo *dst, unsigned offset, unsigned domain,
                      unsigned size, const void *data);
void
nvc0_cb_bo_push(struct nouveau_context *,
                struct nouveau_bo *bo, unsigned domain,
                unsigned base, unsigned size,
                unsigned offset, unsigned words, const uint32_t *data);

/* nvc0_vbo.c */
void nvc0_draw_vbo(struct pipe_context *, const struct pipe_draw_info *, unsigned,
                   const struct pipe_draw_indirect_info *indirect,
                   const struct pipe_draw_start_count_bias *draws,
                   unsigned num_draws);

void *
nvc0_vertex_state_create(struct pipe_context *pipe,
                         unsigned num_elements,
                         const struct pipe_vertex_element *elements);
void
nvc0_vertex_state_delete(struct pipe_context *pipe, void *hwcso);

void nvc0_vertex_arrays_validate(struct nvc0_context *);

void nvc0_idxbuf_validate(struct nvc0_context *);

/* nvc0_video.c */
struct pipe_video_codec *
nvc0_create_decoder(struct pipe_context *context,
                    const struct pipe_video_codec *templ);

struct pipe_video_buffer *
nvc0_video_buffer_create(struct pipe_context *pipe,
                         const struct pipe_video_buffer *templat);

/* nvc0_push.c */
void nvc0_push_vbo(struct nvc0_context *, const struct pipe_draw_info *,
                   const struct pipe_draw_indirect_info *indirect,
                   const struct pipe_draw_start_count_bias *draw);
void nvc0_push_vbo_indirect(struct nvc0_context *, const struct pipe_draw_info *,
                            unsigned drawid_offset,
                            const struct pipe_draw_indirect_info *indirect,
                            const struct pipe_draw_start_count_bias *draw);

/* nve4_compute.c */
void nve4_launch_grid(struct pipe_context *, const struct pipe_grid_info *);

/* nvc0_compute.c */
void nvc0_launch_grid(struct pipe_context *, const struct pipe_grid_info *);
void nvc0_compute_validate_globals(struct nvc0_context *);
void nvc0_update_compute_invocations_counter(struct nvc0_context *nvc0,
                                             const struct pipe_grid_info *info);

#endif
