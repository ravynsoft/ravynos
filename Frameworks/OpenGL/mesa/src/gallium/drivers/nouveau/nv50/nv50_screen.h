#ifndef __NV50_SCREEN_H__
#define __NV50_SCREEN_H__

#include "nouveau_screen.h"
#include "nouveau_fence.h"
#include "nouveau_mm.h"
#include "nouveau_heap.h"

#include "nv50/nv50_winsys.h"
#include "nv50/nv50_stateobj.h"

#define NV50_TIC_MAX_ENTRIES 2048
#define NV50_TSC_MAX_ENTRIES 2048

/* doesn't count reserved slots (for auxiliary constants, immediates, etc.) */
#define NV50_MAX_PIPE_CONSTBUFS 14

struct nv50_context;

#define NV50_CODE_BO_SIZE_LOG2 19

#define NV50_SCREEN_RESIDENT_BO_COUNT 5

#define NV50_MAX_VIEWPORTS 16

#define NV50_MAX_WINDOW_RECTANGLES 8

#define NV50_MAX_GLOBALS 16

#define ONE_TEMP_SIZE (4/*vector*/ * sizeof(float))

struct nv50_blitter;

struct nv50_graph_state {
   uint32_t instance_elts; /* bitmask of per-instance elements */
   uint32_t instance_base;
   uint32_t interpolant_ctrl;
   uint32_t semantic_color;
   uint32_t semantic_psize;
   int32_t index_bias;
   uint32_t clip_mode;
   bool uniform_buffer_bound[4];
   bool prim_restart;
   bool point_sprite;
   bool rt_serialize;
   bool flushed;
   bool rasterizer_discard;
   uint8_t tls_required;
   bool new_tls_space;
   uint8_t num_vtxbufs;
   uint8_t num_vtxelts;
   uint8_t num_textures[4];
   uint8_t num_samplers[4];
   uint8_t prim_size;
   uint16_t scissor;
   bool seamless_cube_map;
   bool mul_zero_wins;
};

struct nv50_screen {
   struct nouveau_screen base;

   struct nv50_context *cur_ctx;
   struct nv50_graph_state save_state;
   simple_mtx_t state_lock;

   int num_occlusion_queries_active;

   struct nouveau_bo *code;
   struct nouveau_bo *uniforms;
   struct nouveau_bo *txc; /* TIC (offset 0) and TSC (65536) */
   struct nouveau_bo *stack_bo;
   struct nouveau_bo *tls_bo;

   unsigned TPs;
   unsigned MPsInTP;
   unsigned max_tls_space;
   unsigned cur_tls_space;
   unsigned mp_count;

   struct nouveau_heap *vp_code_heap;
   struct nouveau_heap *gp_code_heap;
   struct nouveau_heap *fp_code_heap;

   struct nv50_blitter *blitter;

   struct {
      void **entries;
      int next;
      uint32_t lock[NV50_TIC_MAX_ENTRIES / 32];
   } tic;

   struct {
      void **entries;
      int next;
      uint32_t lock[NV50_TSC_MAX_ENTRIES / 32];
   } tsc;

   struct {
      uint32_t *map;
      struct nouveau_bo *bo;
   } fence;

   struct {
      struct nv50_program *prog; /* compute state object to read MP counters */
      struct nv50_hw_sm_query *mp_counter[4]; /* counter to query allocation */
      uint8_t num_hw_sm_active;
   } pm;

   struct nouveau_object *sync;

   struct nouveau_object *tesla;
   struct nouveau_object *compute;
   struct nouveau_object *eng2d;
   struct nouveau_object *m2mf;
};

static inline struct nv50_screen *
nv50_screen(struct pipe_screen *screen)
{
   return (struct nv50_screen *)screen;
}

int nv50_screen_get_driver_query_info(struct pipe_screen *, unsigned,
                                      struct pipe_driver_query_info *);
int nv50_screen_get_driver_query_group_info(struct pipe_screen *, unsigned,
                                            struct pipe_driver_query_group_info *);

bool nv50_blitter_create(struct nv50_screen *);
void nv50_blitter_destroy(struct nv50_screen *);

int nv50_screen_tic_alloc(struct nv50_screen *, void *);
int nv50_screen_tsc_alloc(struct nv50_screen *, void *);

int nv50_screen_compute_setup(struct nv50_screen *, struct nouveau_pushbuf *);

struct nv50_format {
   uint32_t rt;
   struct {
      unsigned format:6;
      unsigned type_r:3;
      unsigned type_g:3;
      unsigned type_b:3;
      unsigned type_a:3;
      unsigned src_x:3;
      unsigned src_y:3;
      unsigned src_z:3;
      unsigned src_w:3;
   } tic;
   uint32_t usage;
};

struct nv50_vertex_format {
   uint32_t vtx;
   uint32_t usage;
};

extern const struct nv50_format nv50_format_table[];
extern const struct nv50_vertex_format nv50_vertex_format[];

static inline void
nv50_screen_tic_unlock(struct nv50_screen *screen, struct nv50_tic_entry *tic)
{
   if (tic->id >= 0)
      screen->tic.lock[tic->id / 32] &= ~(1 << (tic->id % 32));
}

static inline void
nv50_screen_tsc_unlock(struct nv50_screen *screen, struct nv50_tsc_entry *tsc)
{
   if (tsc->id >= 0)
      screen->tsc.lock[tsc->id / 32] &= ~(1 << (tsc->id % 32));
}

static inline void
nv50_screen_tic_free(struct nv50_screen *screen, struct nv50_tic_entry *tic)
{
   if (tic->id >= 0) {
      screen->tic.entries[tic->id] = NULL;
      screen->tic.lock[tic->id / 32] &= ~(1 << (tic->id % 32));
   }
}

static inline void
nv50_screen_tsc_free(struct nv50_screen *screen, struct nv50_tsc_entry *tsc)
{
   if (tsc->id >= 0) {
      screen->tsc.entries[tsc->id] = NULL;
      screen->tsc.lock[tsc->id / 32] &= ~(1 << (tsc->id % 32));
   }
}

extern int nv50_tls_realloc(struct nv50_screen *screen, unsigned tls_space);

#endif
