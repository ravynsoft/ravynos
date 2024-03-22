
#ifndef __NVC0_RESOURCE_H__
#define __NVC0_RESOURCE_H__

#include "nv50/nv50_resource.h"
#include "nouveau_screen.h"

#define NVC0_RESOURCE_FLAG_VIDEO (NOUVEAU_RESOURCE_FLAG_DRV_PRIV << 0)

#define NVC0_TILE_MODE_X(m) (((m) >> 0) & 0xf)
#define NVC0_TILE_MODE_Y(m) (((m) >> 4) & 0xf)
#define NVC0_TILE_MODE_Z(m) (((m) >> 8) & 0xf)

#define NVC0_TILE_SHIFT_X(m) (NVC0_TILE_MODE_X(m) + 6)
#define NVC0_TILE_SHIFT_Y(m) (NVC0_TILE_MODE_Y(m) + 3)
#define NVC0_TILE_SHIFT_Z(m) (NVC0_TILE_MODE_Z(m) + 0)

#define NVC0_TILE_SIZE_X(m) (64 << NVC0_TILE_MODE_X(m))
#define NVC0_TILE_SIZE_Y(m) ( 8 << NVC0_TILE_MODE_Y(m))
#define NVC0_TILE_SIZE_Z(m) ( 1 << NVC0_TILE_MODE_Z(m))

/* it's ok to mask only in the end because max value is 3 * 5 */

#define NVC0_TILE_SIZE_2D(m) ((64 * 8) << (((m) + ((m) >> 4)) & 0xf))

#define NVC0_TILE_SIZE(m) ((64 * 8) << (((m) + ((m) >> 4) + ((m) >> 8)) & 0xf))

static inline uint32_t
nvc0_get_kind_generation(struct pipe_screen *pscreen)
{
   if (nouveau_screen(pscreen)->device->chipset >= 0x160)
      return 2;
   else
      return 0;
}

void
nvc0_init_resource_functions(struct pipe_context *pcontext);

void
nvc0_screen_init_resource_functions(struct pipe_screen *pscreen);

/* Internal functions:
 */
uint32_t
nvc0_choose_tiled_storage_type(struct pipe_screen *pscreen,
                               enum pipe_format format,
                               unsigned ms,
                               bool compressed);

struct pipe_resource *
nvc0_miptree_create(struct pipe_screen *pscreen,
                    const struct pipe_resource *tmp,
                    const uint64_t *modifiers, unsigned int count);

bool
nvc0_miptree_get_handle(struct pipe_screen *pscreen,
                        struct pipe_context *context,
                        struct pipe_resource *pt,
                        struct winsys_handle *whandle,
                        unsigned usage);

struct pipe_surface *
nvc0_miptree_surface_new(struct pipe_context *,
                         struct pipe_resource *,
                         const struct pipe_surface *templ);

unsigned
nvc0_mt_zslice_offset(const struct nv50_miptree *, unsigned l, unsigned z);

void *
nvc0_miptree_transfer_map(struct pipe_context *pctx,
                          struct pipe_resource *res,
                          unsigned level,
                          unsigned usage,
                          const struct pipe_box *box,
                          struct pipe_transfer **ptransfer);
void
nvc0_miptree_transfer_unmap(struct pipe_context *pcontext,
                            struct pipe_transfer *ptx);

#endif
