#ifndef __NVC0_SCREEN_H__
#define __NVC0_SCREEN_H__

#include "nouveau_screen.h"
#include "nouveau_mm.h"
#include "nouveau_fence.h"
#include "nouveau_heap.h"

#include "nv_object.xml.h"

#include "nvc0/nvc0_winsys.h"
#include "nvc0/nvc0_stateobj.h"

#define NVC0_TIC_MAX_ENTRIES 2048
#define NVC0_TSC_MAX_ENTRIES 2048
#define NVE4_IMG_MAX_HANDLES 512

/* doesn't count driver-reserved slot */
#define NVC0_MAX_PIPE_CONSTBUFS 15
#define NVC0_MAX_CONST_BUFFERS  16
#define NVC0_MAX_CONSTBUF_SIZE  65536

#define NVC0_MAX_SURFACE_SLOTS 16

#define NVC0_MAX_VIEWPORTS 16

#define NVC0_MAX_BUFFERS 32

#define NVC0_MAX_IMAGES 8

#define NVC0_MAX_WINDOW_RECTANGLES 8

struct nvc0_context;

struct nvc0_blitter;

struct nvc0_graph_state {
   bool flushed;
   bool rasterizer_discard;
   bool early_z_forced;
   bool prim_restart;
   uint32_t instance_elts; /* bitmask of per-instance elements */
   uint32_t instance_base;
   uint32_t constant_vbos;
   uint32_t constant_elts;
   int32_t index_bias;
   uint16_t scissor;
   bool flatshade;
   uint8_t patch_vertices;
   uint8_t vbo_mode; /* 0 = normal, 1 = translate, 3 = translate, forced */
   uint8_t num_vtxbufs;
   uint8_t num_vtxelts;
   uint8_t num_textures[6];
   uint8_t num_samplers[6];
   uint8_t tls_required; /* bitmask of shader types using l[] */
   uint8_t clip_enable;
   uint32_t clip_mode;
   bool uniform_buffer_bound[6];
   struct nvc0_transform_feedback_state *tfb;
   bool seamless_cube_map;
   bool post_depth_coverage;
};

struct nvc0_cb_binding {
   uint64_t addr;
   int size;
};

struct nvc0_screen {
   struct nouveau_screen base;

   struct nvc0_context *cur_ctx;
   struct nvc0_graph_state save_state;
   simple_mtx_t state_lock;

   int num_occlusion_queries_active;

   struct nouveau_bo *text;
   struct nouveau_bo *uniform_bo;
   struct nouveau_bo *tls;
   struct nouveau_bo *txc; /* TIC (offset 0) and TSC (65536) */
   struct nouveau_bo *poly_cache;

   uint8_t gpc_count;
   uint16_t mp_count;
   uint16_t mp_count_compute; /* magic reg can make compute use fewer MPs */

   struct nouveau_heap *text_heap;
   struct nouveau_heap *lib_code; /* allocated from text_heap */

   struct nvc0_blitter *blitter;

   struct {
      void **entries;
      int next;
      uint32_t lock[NVC0_TIC_MAX_ENTRIES / 32];
      bool maxwell;
   } tic;

   struct {
      void **entries;
      int next;
      uint32_t lock[NVC0_TSC_MAX_ENTRIES / 32];
   } tsc;

   struct {
      struct pipe_image_view **entries;
      int next;
   } img;

   struct {
      struct nouveau_bo *bo;
      uint32_t *map;
   } fence;

   struct {
      struct nvc0_program *prog; /* compute state object to read MP counters */
      struct nvc0_hw_sm_query *mp_counter[8]; /* counter to query allocation */
      uint8_t num_hw_sm_active[2];
      bool mp_counters_enabled;
   } pm;

   /* only maintained on Maxwell+ */
   struct nvc0_cb_binding cb_bindings[5][NVC0_MAX_CONST_BUFFERS];

   struct nouveau_object *eng3d; /* sqrt(1/2)|kepler> + sqrt(1/2)|fermi> */
   struct nouveau_object *eng2d;
   struct nouveau_object *m2mf;
   struct nouveau_object *copy;
   struct nouveau_object *compute;
   struct nouveau_object *nvsw;
};

static inline struct nvc0_screen *
nvc0_screen(struct pipe_screen *screen)
{
   return (struct nvc0_screen *)screen;
}

int nvc0_screen_get_driver_query_info(struct pipe_screen *, unsigned,
                                      struct pipe_driver_query_info *);

int nvc0_screen_get_driver_query_group_info(struct pipe_screen *, unsigned,
                                            struct pipe_driver_query_group_info *);

bool nvc0_blitter_create(struct nvc0_screen *);
void nvc0_blitter_destroy(struct nvc0_screen *);

void nvc0_screen_make_buffers_resident(struct nvc0_screen *);

int nvc0_screen_tic_alloc(struct nvc0_screen *, void *);
int nvc0_screen_tsc_alloc(struct nvc0_screen *, void *);

int nve4_screen_compute_setup(struct nvc0_screen *, struct nouveau_pushbuf *);
int nvc0_screen_compute_setup(struct nvc0_screen *, struct nouveau_pushbuf *);

int nvc0_screen_resize_text_area(struct nvc0_screen *, struct nouveau_pushbuf *, uint64_t);

// 3D Only
void nvc0_screen_bind_cb_3d(struct nvc0_screen *, struct nouveau_pushbuf *, bool *, int, int, int, uint64_t);

struct nvc0_format {
   uint32_t rt;
   struct {
      unsigned format:7;
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

struct nvc0_vertex_format {
   uint32_t vtx;
   uint32_t usage;
};

extern const struct nvc0_format nvc0_format_table[];
extern const struct nvc0_vertex_format nvc0_vertex_format[];

static inline void
nvc0_screen_tic_unlock(struct nvc0_screen *screen, struct nv50_tic_entry *tic)
{
   if (tic->bindless)
      return;
   if (tic->id >= 0)
      screen->tic.lock[tic->id / 32] &= ~(1 << (tic->id % 32));
}

static inline void
nvc0_screen_tsc_unlock(struct nvc0_screen *screen, struct nv50_tsc_entry *tsc)
{
   if (tsc->id >= 0)
      screen->tsc.lock[tsc->id / 32] &= ~(1 << (tsc->id % 32));
}

static inline void
nvc0_screen_tic_free(struct nvc0_screen *screen, struct nv50_tic_entry *tic)
{
   if (tic->id >= 0) {
      screen->tic.entries[tic->id] = NULL;
      screen->tic.lock[tic->id / 32] &= ~(1 << (tic->id % 32));
   }
}

static inline void
nvc0_screen_tsc_free(struct nvc0_screen *screen, struct nv50_tsc_entry *tsc)
{
   if (tsc->id >= 0) {
      screen->tsc.entries[tsc->id] = NULL;
      screen->tsc.lock[tsc->id / 32] &= ~(1 << (tsc->id % 32));
   }
}

#endif
