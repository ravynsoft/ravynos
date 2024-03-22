#ifndef __NV50_CONTEXT_H__
#define __NV50_CONTEXT_H__

#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "pipe/p_state.h"

#include "util/u_memory.h"
#include "util/u_math.h"
#include "util/u_inlines.h"
#include "util/u_dynarray.h"

#include "nv50/nv50_winsys.h"
#include "nv50/nv50_stateobj.h"
#include "nv50/nv50_screen.h"
#include "nv50/nv50_program.h"
#include "nv50/nv50_resource.h"
#include "nv50/nv50_transfer.h"
#include "nv50/nv50_query.h"

#include "nouveau_context.h"
#include "nouveau_debug.h"
#include "nv_object.xml.h"
#include "nv_m2mf.xml.h"
#include "nv50/nv50_3ddefs.xml.h"
#include "nv50/nv50_3d.xml.h"
#include "nv50/nv50_2d.xml.h"
#include "nv50/nv50_compute.xml.h"

// NOTE: the VS/GS/FS order is based on how command methods are laid out for
// TSC/TIC setting.
#define NV50_SHADER_STAGE_VERTEX   0
#define NV50_SHADER_STAGE_GEOMETRY 1
#define NV50_SHADER_STAGE_FRAGMENT 2
#define NV50_SHADER_STAGE_COMPUTE  3
#define NV50_MAX_SHADER_STAGES     4

#define NV50_MAX_3D_SHADER_STAGES  3

#define NV50_NEW_3D_BLEND        (1 << 0)
#define NV50_NEW_3D_RASTERIZER   (1 << 1)
#define NV50_NEW_3D_ZSA          (1 << 2)
#define NV50_NEW_3D_VERTPROG     (1 << 3)
#define NV50_NEW_3D_GMTYPROG     (1 << 6)
#define NV50_NEW_3D_FRAGPROG     (1 << 7)
#define NV50_NEW_3D_BLEND_COLOUR (1 << 8)
#define NV50_NEW_3D_STENCIL_REF  (1 << 9)
#define NV50_NEW_3D_CLIP         (1 << 10)
#define NV50_NEW_3D_SAMPLE_MASK  (1 << 11)
#define NV50_NEW_3D_FRAMEBUFFER  (1 << 12)
#define NV50_NEW_3D_STIPPLE      (1 << 13)
#define NV50_NEW_3D_SCISSOR      (1 << 14)
#define NV50_NEW_3D_VIEWPORT     (1 << 15)
#define NV50_NEW_3D_ARRAYS       (1 << 16)
#define NV50_NEW_3D_VERTEX       (1 << 17)
#define NV50_NEW_3D_CONSTBUF     (1 << 18)
#define NV50_NEW_3D_TEXTURES     (1 << 19)
#define NV50_NEW_3D_SAMPLERS     (1 << 20)
#define NV50_NEW_3D_STRMOUT      (1 << 21)
#define NV50_NEW_3D_MIN_SAMPLES  (1 << 22)
#define NV50_NEW_3D_WINDOW_RECTS (1 << 23)

#define NV50_NEW_CP_PROGRAM   (1 << 0)
#define NV50_NEW_CP_SURFACES  (1 << 1)
#define NV50_NEW_CP_TEXTURES  (1 << 2)
#define NV50_NEW_CP_SAMPLERS  (1 << 3)
#define NV50_NEW_CP_CONSTBUF  (1 << 4)
#define NV50_NEW_CP_GLOBALS   (1 << 5)
#define NV50_NEW_CP_DRIVERCONST (1 << 6)
#define NV50_NEW_CP_BUFFERS   (1 << 7)

/* 3d bufctx (during draw_vbo, blit_3d) */
#define NV50_BIND_3D_FB          0
#define NV50_BIND_3D_VERTEX      1
#define NV50_BIND_3D_VERTEX_TMP  2
#define NV50_BIND_3D_INDEX       3
#define NV50_BIND_3D_TEXTURES    4
#define NV50_BIND_3D_CB(s, i)   (5 + 16 * (s) + (i))
#define NV50_BIND_3D_SO         53
#define NV50_BIND_3D_SCREEN     54
#define NV50_BIND_3D_TLS        55
#define NV50_BIND_3D_COUNT      56

/* compute bufctx (during launch_grid) */
#define NV50_BIND_CP_CB(i)    ( 0 + (i))
#define NV50_BIND_CP_TEXTURES  16
#define NV50_BIND_CP_SUF       17
#define NV50_BIND_CP_BUF       18
#define NV50_BIND_CP_GLOBAL    19
#define NV50_BIND_CP_SCREEN    20
#define NV50_BIND_CP_QUERY     21
#define NV50_BIND_CP_COUNT     22

/* bufctx for other operations */
#define NV50_BIND_2D          0
#define NV50_BIND_M2MF        0
#define NV50_BIND_FENCE       1

/* fixed constant buffer binding points - low indices for user's constbufs */
#define NV50_CB_PVP 123
#define NV50_CB_PGP 124
#define NV50_CB_PFP 125
#define NV50_CB_PCP 126
/* constant buffer permanently mapped in as c15[] */
#define NV50_CB_AUX 127
/* size of the buffer: 64k. not all taken up, can be reduced if needed. */
#define NV50_CB_AUX_SIZE          (1 << 16)
/* 8 user clip planes, at 4 32-bit floats each */
#define NV50_CB_AUX_UCP_OFFSET    0x0000
#define NV50_CB_AUX_UCP_SIZE      (8 * 4 * 4)
/* 16 textures * NV50_MAX_SHADER_STAGES shaders, each with ms_x, ms_y u32 pairs */
#define NV50_CB_AUX_TEX_MS_OFFSET 0x0080
#define NV50_CB_AUX_TEX_MS_SIZE   (16 * NV50_MAX_SHADER_STAGES * 2 * 4)
/* For each MS level (4), 8 sets of 32-bit integer pairs sample offsets */
#define NV50_CB_AUX_MS_OFFSET     0x280
#define NV50_CB_AUX_MS_SIZE       (4 * 8 * 4 * 2)
/* Sample position pairs for the current output MS level */
#define NV50_CB_AUX_SAMPLE_OFFSET 0x380
#define NV50_CB_AUX_SAMPLE_OFFSET_SIZE (4 * 8 * 2)
/* Alpha test ref value */
#define NV50_CB_AUX_ALPHATEST_OFFSET 0x3c0
#define NV50_CB_AUX_ALPHATEST_SIZE (4)
/* Compute buffer info: 16 surfaces, 12 32-bit integers each */
#define NV50_CB_AUX_BUF_INFO(i)   (0x3c4 + (i) * 12 * 4)
#define NV50_CB_AUX_BUF_SIZE      (NV50_MAX_GLOBALS * 12 * 4)
/* Compute membar mapped area */
#define NV50_CB_AUX_MEMBAR_OFFSET 0x6c4
/* next spot: 0x6c8 */
/* 0x800 from the end for compute shader membars, reads only. */
#define NV50_CB_AUX_MEMBAR        (NV50_CB_AUX_SIZE - 0x800)
/* 4 32-bit floats for the vertex runout, put at the end */
#define NV50_CB_AUX_RUNOUT_OFFSET (NV50_CB_AUX_SIZE - 0x10)



struct nv50_blitctx;

bool nv50_blitctx_create(struct nv50_context *);

struct nv50_context {
   struct nouveau_context base;

   struct nv50_screen *screen;

   struct nouveau_bufctx *bufctx_3d;
   struct nouveau_bufctx *bufctx;
   struct nouveau_bufctx *bufctx_cp;

   uint32_t dirty_3d; /* dirty flags for 3d state */
   uint32_t dirty_cp; /* dirty flags for compute state */
   bool cb_dirty;

   struct nv50_graph_state state;

   struct nv50_blend_stateobj *blend;
   struct nv50_rasterizer_stateobj *rast;
   struct nv50_zsa_stateobj *zsa;
   struct nv50_vertex_stateobj *vertex;

   struct nv50_program *vertprog;
   struct nv50_program *gmtyprog;
   struct nv50_program *fragprog;
   struct nv50_program *compprog;

   struct nv50_constbuf constbuf[NV50_MAX_SHADER_STAGES][NV50_MAX_PIPE_CONSTBUFS];
   uint16_t constbuf_dirty[NV50_MAX_SHADER_STAGES];
   uint16_t constbuf_valid[NV50_MAX_SHADER_STAGES];
   uint16_t constbuf_coherent[NV50_MAX_SHADER_STAGES];

   struct pipe_vertex_buffer vtxbuf[PIPE_MAX_ATTRIBS];
   unsigned num_vtxbufs;
   uint32_t vtxbufs_coherent;
   uint32_t vbo_fifo; /* bitmask of vertex elements to be pushed to FIFO */
   uint32_t vbo_user; /* bitmask of vertex buffers pointing to user memory */
   uint32_t vbo_constant; /* bitmask of user buffers with stride 0 */
   uint32_t vb_elt_first; /* from pipe_draw_info, for vertex upload */
   uint32_t vb_elt_limit; /* max - min element (count - 1) */
   uint32_t instance_off; /* base vertex for instanced arrays */
   uint32_t instance_max; /* max instance for current draw call */

   struct pipe_sampler_view *textures[NV50_MAX_SHADER_STAGES][PIPE_MAX_SAMPLERS];
   unsigned num_textures[NV50_MAX_SHADER_STAGES];
   uint32_t textures_coherent[NV50_MAX_SHADER_STAGES];
   struct nv50_tsc_entry *samplers[NV50_MAX_SHADER_STAGES][PIPE_MAX_SAMPLERS];
   unsigned num_samplers[NV50_MAX_SHADER_STAGES];
   bool seamless_cube_map;

   uint8_t num_so_targets;
   uint8_t so_targets_dirty;
   struct pipe_stream_output_target *so_target[4];
   /* keeps track of how much of an SO is used. normally this doesn't work in
    * the presence of GS, but this only needs to work for ES 3.0 which doesn't
    * have GS or any other oddities. only used pre-NVA0.
    */
   uint32_t so_used[4];

   struct pipe_framebuffer_state framebuffer;
   struct pipe_blend_color blend_colour;
   struct pipe_stencil_ref stencil_ref;
   struct pipe_poly_stipple stipple;
   struct pipe_scissor_state scissors[NV50_MAX_VIEWPORTS];
   unsigned scissors_dirty;
   struct pipe_viewport_state viewports[NV50_MAX_VIEWPORTS];
   unsigned viewports_dirty;
   struct pipe_clip_state clip;
   struct nv50_window_rect_stateobj window_rect;

   unsigned sample_mask;
   unsigned min_samples;

   bool vbo_push_hint;

   uint32_t rt_array_mode;

   struct pipe_query *cond_query;
   bool cond_cond; /* inverted rendering condition */
   uint cond_mode;
   uint32_t cond_condmode; /* the calculated condition */

   struct nv50_blitctx *blit;

   /* compute stage only */
   struct pipe_shader_buffer buffers[NV50_MAX_GLOBALS];
   uint16_t buffers_dirty;
   uint16_t buffers_valid;

   struct pipe_image_view images[NV50_MAX_GLOBALS];
   uint16_t images_dirty;
   uint16_t images_valid;

   struct util_dynarray global_residents;

   uint64_t compute_invocations;
};

static inline struct nv50_context *
nv50_context(struct pipe_context *pipe)
{
   return (struct nv50_context *)pipe;
}

/* return index used in nv50_context arrays for a specific shader type */
static inline unsigned
nv50_context_shader_stage(unsigned pipe)
{
   switch (pipe) {
   case PIPE_SHADER_VERTEX: return NV50_SHADER_STAGE_VERTEX;
   case PIPE_SHADER_FRAGMENT: return NV50_SHADER_STAGE_FRAGMENT;
   case PIPE_SHADER_GEOMETRY: return NV50_SHADER_STAGE_GEOMETRY;
   case PIPE_SHADER_COMPUTE: return NV50_SHADER_STAGE_COMPUTE;
   default:
      assert(!"invalid/unhandled shader type");
      return 0;
   }
}

static inline void
nv50_resource_validate(struct nv50_context *context, struct nv04_resource *res, uint32_t flags)
{
   if (likely(res->bo)) {
      if (flags & NOUVEAU_BO_WR)
         res->status |= NOUVEAU_BUFFER_STATUS_GPU_WRITING |
            NOUVEAU_BUFFER_STATUS_DIRTY;
      if (flags & NOUVEAU_BO_RD)
         res->status |= NOUVEAU_BUFFER_STATUS_GPU_READING;

      if (res->mm) {
         nouveau_fence_ref(context->base.fence, &res->fence);
         if (flags & NOUVEAU_BO_WR)
            nouveau_fence_ref(context->base.fence, &res->fence_wr);
      }
   }
}

/* nv50_context.c */
struct pipe_context *nv50_create(struct pipe_screen *, void *, unsigned flags);

void nv50_bufctx_fence(struct nv50_context *, struct nouveau_bufctx *, bool on_flush);

void nv50_default_kick_notify(struct nouveau_context *);

/* nv50_draw.c */
extern struct draw_stage *nv50_draw_render_stage(struct nv50_context *);

/* nv50_shader_state.c */
void nv50_vertprog_validate(struct nv50_context *);
void nv50_gmtyprog_validate(struct nv50_context *);
void nv50_fragprog_validate(struct nv50_context *);
void nv50_compprog_validate(struct nv50_context *);
void nv50_fp_linkage_validate(struct nv50_context *);
void nv50_gp_linkage_validate(struct nv50_context *);
void nv50_constbufs_validate(struct nv50_context *);
void nv50_validate_derived_rs(struct nv50_context *);
void nv50_stream_output_validate(struct nv50_context *);

/* nv50_state.c */
extern void nv50_init_state_functions(struct nv50_context *);

/* nv50_state_validate.c */
struct nv50_state_validate {
   void (*func)(struct nv50_context *);
   uint32_t states;
};

bool nv50_state_validate(struct nv50_context *, uint32_t,
                         struct nv50_state_validate *, int, uint32_t *,
                         struct nouveau_bufctx *);
bool nv50_state_validate_3d(struct nv50_context *, uint32_t);

/* nv50_surface.c */
extern void nv50_clear(struct pipe_context *, unsigned buffers,
                       const struct pipe_scissor_state *scissor_state,
                       const union pipe_color_union *color,
                       double depth, unsigned stencil);
extern void nv50_init_surface_functions(struct nv50_context *);

/* nv50_tex.c */
bool nv50_validate_tic(struct nv50_context *nv50, int s);
void nv50_validate_textures(struct nv50_context *);
bool nv50_validate_tsc(struct nv50_context *nv50, int s);
void nv50_validate_samplers(struct nv50_context *);
void nv50_upload_ms_info(struct nouveau_pushbuf *);
void nv50_upload_tsc0(struct nv50_context *);

struct pipe_sampler_view *
nv50_create_texture_view(struct pipe_context *,
                         struct pipe_resource *,
                         const struct pipe_sampler_view *,
                         uint32_t flags);
struct pipe_sampler_view *
nv50_create_sampler_view(struct pipe_context *,
                         struct pipe_resource *,
                         const struct pipe_sampler_view *);

/* nv50_transfer.c */
void
nv50_m2mf_transfer_rect(struct nv50_context *,
                        const struct nv50_m2mf_rect *dst,
                        const struct nv50_m2mf_rect *src,
                        uint32_t nblocksx, uint32_t nblocksy);
void
nv50_sifc_linear_u8(struct nouveau_context *pipe,
                    struct nouveau_bo *dst, unsigned offset, unsigned domain,
                    unsigned size, const void *data);
void
nv50_m2mf_copy_linear(struct nouveau_context *pipe,
                      struct nouveau_bo *dst, unsigned dstoff, unsigned dstdom,
                      struct nouveau_bo *src, unsigned srcoff, unsigned srcdom,
                      unsigned size);
void
nv50_cb_push(struct nouveau_context *nv,
             struct nv04_resource *res,
             unsigned offset, unsigned words, const uint32_t *data);

/* nv50_vbo.c */
void nv50_draw_vbo(struct pipe_context *, const struct pipe_draw_info *, unsigned,
                   const struct pipe_draw_indirect_info *indirect,
                   const struct pipe_draw_start_count_bias *draws,
                   unsigned num_draws);

void *
nv50_vertex_state_create(struct pipe_context *pipe,
                         unsigned num_elements,
                         const struct pipe_vertex_element *elements);
void
nv50_vertex_state_delete(struct pipe_context *pipe, void *hwcso);

void nv50_vertex_arrays_validate(struct nv50_context *nv50);

/* nv50_push.c */
void nv50_push_vbo(struct nv50_context *, const struct pipe_draw_info *,
                   const struct pipe_draw_indirect_info *indirect,
                   const struct pipe_draw_start_count_bias *draw);

/* nv84_video.c */
struct pipe_video_codec *
nv84_create_decoder(struct pipe_context *context,
                    const struct pipe_video_codec *templ);

struct pipe_video_buffer *
nv84_video_buffer_create(struct pipe_context *pipe,
                         const struct pipe_video_buffer *template);

int
nv84_screen_get_video_param(struct pipe_screen *pscreen,
                            enum pipe_video_profile profile,
                            enum pipe_video_entrypoint entrypoint,
                            enum pipe_video_cap param);

bool
nv84_screen_video_supported(struct pipe_screen *screen,
                            enum pipe_format format,
                            enum pipe_video_profile profile,
                            enum pipe_video_entrypoint entrypoint);

/* nv98_video.c */
struct pipe_video_codec *
nv98_create_decoder(struct pipe_context *context,
                    const struct pipe_video_codec *templ);

struct pipe_video_buffer *
nv98_video_buffer_create(struct pipe_context *pipe,
                         const struct pipe_video_buffer *template);

/* nv50_compute.c */
void
nv50_launch_grid(struct pipe_context *, const struct pipe_grid_info *);

#endif
