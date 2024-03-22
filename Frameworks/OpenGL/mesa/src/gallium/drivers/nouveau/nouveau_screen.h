#ifndef __NOUVEAU_SCREEN_H__
#define __NOUVEAU_SCREEN_H__

#include "pipe/p_screen.h"
#include "util/disk_cache.h"
#include "util/u_atomic.h"
#include "util/u_memory.h"

#include "nouveau_fence.h"

#ifndef NDEBUG
# define NOUVEAU_ENABLE_DRIVER_STATISTICS
#endif

typedef uint32_t u32;
typedef uint16_t u16;

extern int nouveau_mesa_debug;

struct nouveau_bo;

#define NOUVEAU_SHADER_CACHE_FLAGS_IR_TGSI 0 << 0
#define NOUVEAU_SHADER_CACHE_FLAGS_IR_NIR  1 << 0

struct nouveau_screen {
   struct pipe_screen base;
   struct nouveau_drm *drm;
   struct nouveau_device *device;
   struct nouveau_object *channel;
   struct nouveau_client *client;
   struct nouveau_pushbuf *pushbuf;

   char chipset_name[8];

   int refcount;

   unsigned transfer_pushbuf_threshold;

   unsigned vidmem_bindings; /* PIPE_BIND_* where VRAM placement is desired */
   unsigned sysmem_bindings; /* PIPE_BIND_* where GART placement is desired */
   unsigned lowmem_bindings; /* PIPE_BIND_* that require an address < 4 GiB */
   /*
    * For bindings with (vidmem & sysmem) bits set, PIPE_USAGE_* decides
    * placement.
    */

   uint16_t class_3d;

   struct nouveau_fence_list fence;

   struct nouveau_mman *mm_VRAM;
   struct nouveau_mman *mm_GART;

   int64_t cpu_gpu_time_delta;

   bool hint_buf_keep_sysmem_copy;
   bool tegra_sector_layout;

   unsigned vram_domain;

   struct {
      unsigned profiles_checked;
      unsigned profiles_present;
   } firmware_info;

   struct disk_cache *disk_shader_cache;

   bool force_enable_cl;
   bool has_svm;
   bool is_uma;
   bool disable_fences;
   void *svm_cutout;
   size_t svm_cutout_size;

#ifdef NOUVEAU_ENABLE_DRIVER_STATISTICS
   union {
      uint64_t v[29];
      struct {
         uint64_t tex_obj_current_count;
         uint64_t tex_obj_current_bytes;
         uint64_t buf_obj_current_count;
         uint64_t buf_obj_current_bytes_vid;
         uint64_t buf_obj_current_bytes_sys;
         uint64_t tex_transfers_rd;
         uint64_t tex_transfers_wr;
         uint64_t tex_copy_count;
         uint64_t tex_blit_count;
         uint64_t tex_cache_flush_count;
         uint64_t buf_transfers_rd;
         uint64_t buf_transfers_wr;
         uint64_t buf_read_bytes_staging_vid;
         uint64_t buf_write_bytes_direct;
         uint64_t buf_write_bytes_staging_vid;
         uint64_t buf_write_bytes_staging_sys;
         uint64_t buf_copy_bytes;
         uint64_t buf_non_kernel_fence_sync_count;
         uint64_t any_non_kernel_fence_sync_count;
         uint64_t query_sync_count;
         uint64_t gpu_serialize_count;
         uint64_t draw_calls_array;
         uint64_t draw_calls_indexed;
         uint64_t draw_calls_fallback_count;
         uint64_t user_buffer_upload_bytes;
         uint64_t constbuf_upload_count;
         uint64_t constbuf_upload_bytes;
         uint64_t pushbuf_count;
         uint64_t resource_validate_count;
      } named;
   } stats;
#endif
};

struct nouveau_pushbuf_priv {
   struct nouveau_screen *screen;
   struct nouveau_context *context;
};

#define NV_VRAM_DOMAIN(screen) ((screen)->vram_domain)

#ifdef NOUVEAU_ENABLE_DRIVER_STATISTICS
# define NOUVEAU_DRV_STAT(s, n, v) do {         \
      p_atomic_add(&(s)->stats.named.n, (v));   \
   } while(0)
# define NOUVEAU_DRV_STAT_RES(r, n, v) do {                                \
      p_atomic_add(&nouveau_screen((r)->base.screen)->stats.named.n, v);   \
   } while(0)
# define NOUVEAU_DRV_STAT_IFD(x) x
#else
# define NOUVEAU_DRV_STAT(s, n, v)     do { } while(0)
# define NOUVEAU_DRV_STAT_RES(r, n, v) do { } while(0)
# define NOUVEAU_DRV_STAT_IFD(x)
#endif

static inline struct nouveau_screen *
nouveau_screen(struct pipe_screen *pscreen)
{
   return (struct nouveau_screen *)pscreen;
}

bool nouveau_drm_screen_unref(struct nouveau_screen *screen);

bool
nouveau_screen_bo_get_handle(struct pipe_screen *pscreen,
                             struct nouveau_bo *bo,
                             unsigned stride,
                             struct winsys_handle *whandle);
struct nouveau_bo *
nouveau_screen_bo_from_handle(struct pipe_screen *pscreen,
                              struct winsys_handle *whandle,
                              unsigned *out_stride);


int nouveau_screen_init(struct nouveau_screen *, struct nouveau_device *);
void nouveau_screen_fini(struct nouveau_screen *);

void nouveau_screen_init_vdec(struct nouveau_screen *);

int
nouveau_pushbuf_create(struct nouveau_screen *, struct nouveau_context *, struct nouveau_client *,
                       struct nouveau_object *chan, int nr, uint32_t size, bool immediate,
                       struct nouveau_pushbuf **);
void nouveau_pushbuf_destroy(struct nouveau_pushbuf **);

#endif
