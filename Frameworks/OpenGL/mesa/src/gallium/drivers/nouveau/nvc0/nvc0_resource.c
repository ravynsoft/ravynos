#include "drm-uapi/drm_fourcc.h"

#include "pipe/p_context.h"
#include "nvc0/nvc0_resource.h"
#include "nouveau_screen.h"


static struct pipe_resource *
nvc0_resource_create(struct pipe_screen *screen,
                     const struct pipe_resource *templ)
{
   switch (templ->target) {
   case PIPE_BUFFER:
      return nouveau_buffer_create(screen, templ);
   default:
      return nvc0_miptree_create(screen, templ, NULL, 0);
   }
}

static struct pipe_resource *
nvc0_resource_create_with_modifiers(struct pipe_screen *screen,
                                    const struct pipe_resource *templ,
                                    const uint64_t *modifiers, int count)
{
   switch (templ->target) {
   case PIPE_BUFFER:
      return nouveau_buffer_create(screen, templ);
   default:
      return nvc0_miptree_create(screen, templ, modifiers, count);
   }
}

static void
nvc0_resource_destroy(struct pipe_screen *pscreen, struct pipe_resource *res)
{
   if (res->target == PIPE_BUFFER)
      nouveau_buffer_destroy(pscreen, res);
   else
      nv50_miptree_destroy(pscreen, res);
}

static void
nvc0_query_dmabuf_modifiers(struct pipe_screen *screen,
                            enum pipe_format format, int max,
                            uint64_t *modifiers, unsigned int *external_only,
                            int *count)
{
   const int s = nouveau_screen(screen)->tegra_sector_layout ? 0 : 1;
   const uint32_t uc_kind =
      nvc0_choose_tiled_storage_type(screen, format, 0, false);
   const uint32_t num_uc = uc_kind ? 6 : 0; /* max block height = 32 GOBs */
   const int num_supported = num_uc + 1; /* LINEAR is always supported */
   const uint32_t kind_gen = nvc0_get_kind_generation(screen);
   int i, num = 0;

   if (max > num_supported)
      max = num_supported;

   if (!max) {
      max = num_supported;
      external_only = NULL;
      modifiers = NULL;
   }

#define NVC0_ADD_MOD(m) do { \
   if (modifiers) modifiers[num] = m; \
   if (external_only) external_only[num] = 0; \
   num++; \
} while (0)

   for (i = 0; i < max && i < num_uc; i++)
      NVC0_ADD_MOD(DRM_FORMAT_MOD_NVIDIA_BLOCK_LINEAR_2D(0, s, kind_gen,
                                                         uc_kind, 5 - i));

   if (i < max)
      NVC0_ADD_MOD(DRM_FORMAT_MOD_LINEAR);

#undef NVC0_ADD_MOD

   *count = num;
}

static bool
nvc0_is_dmabuf_modifier_supported(struct pipe_screen *screen,
                                  uint64_t modifier, enum pipe_format format,
                                  bool *external_only)
{
   const int s = nouveau_screen(screen)->tegra_sector_layout ? 0 : 1;
   const uint32_t uc_kind =
      nvc0_choose_tiled_storage_type(screen, format, 0, false);
   const uint32_t num_uc = uc_kind ? 6 : 0; /* max block height = 32 GOBs */
   const uint32_t kind_gen = nvc0_get_kind_generation(screen);
   int i;

   if (modifier == DRM_FORMAT_MOD_LINEAR) {
      if (external_only)
         *external_only = false;

      return true;
   }

   for (i = 0; i < num_uc; i++) {
      if (DRM_FORMAT_MOD_NVIDIA_BLOCK_LINEAR_2D(0, s, kind_gen, uc_kind, i) == modifier) {
         if (external_only)
            *external_only = false;

         return true;
      }
   }

   return false;
}

static struct pipe_resource *
nvc0_resource_from_handle(struct pipe_screen * screen,
                          const struct pipe_resource *templ,
                          struct winsys_handle *whandle,
                          unsigned usage)
{
   if (templ->target == PIPE_BUFFER) {
      return NULL;
   } else {
      struct pipe_resource *res = nv50_miptree_from_handle(screen,
                                                           templ, whandle);
      return res;
   }
}

static struct pipe_surface *
nvc0_surface_create(struct pipe_context *pipe,
                    struct pipe_resource *pres,
                    const struct pipe_surface *templ)
{
   if (unlikely(pres->target == PIPE_BUFFER))
      return nv50_surface_from_buffer(pipe, pres, templ);
   return nvc0_miptree_surface_new(pipe, pres, templ);
}

static struct pipe_resource *
nvc0_resource_from_user_memory(struct pipe_screen *pipe,
                               const struct pipe_resource *templ,
                               void *user_memory)
{
   ASSERTED struct nouveau_screen *screen = nouveau_screen(pipe);

   assert(screen->has_svm);
   assert(templ->target == PIPE_BUFFER);

   return nouveau_buffer_create_from_user(pipe, templ, user_memory);
}

void
nvc0_init_resource_functions(struct pipe_context *pcontext)
{
   pcontext->buffer_map = nouveau_buffer_transfer_map;
   pcontext->texture_map = nvc0_miptree_transfer_map;
   pcontext->transfer_flush_region = nouveau_buffer_transfer_flush_region;
   pcontext->buffer_unmap = nouveau_buffer_transfer_unmap;
   pcontext->texture_unmap = nvc0_miptree_transfer_unmap;
   pcontext->buffer_subdata = u_default_buffer_subdata;
   pcontext->texture_subdata = u_default_texture_subdata;
   pcontext->create_surface = nvc0_surface_create;
   pcontext->surface_destroy = nv50_surface_destroy;
   pcontext->invalidate_resource = nv50_invalidate_resource;
}

void
nvc0_screen_init_resource_functions(struct pipe_screen *pscreen)
{
   pscreen->resource_create = nvc0_resource_create;
   pscreen->resource_create_with_modifiers = nvc0_resource_create_with_modifiers;
   pscreen->query_dmabuf_modifiers = nvc0_query_dmabuf_modifiers;
   pscreen->is_dmabuf_modifier_supported = nvc0_is_dmabuf_modifier_supported;
   pscreen->resource_from_handle = nvc0_resource_from_handle;
   pscreen->resource_get_handle = nvc0_miptree_get_handle;
   pscreen->resource_destroy = nvc0_resource_destroy;
   pscreen->resource_from_user_memory = nvc0_resource_from_user_memory;

   pscreen->memobj_create_from_handle = nv50_memobj_create_from_handle;
   pscreen->resource_from_memobj = nv50_resource_from_memobj;
   pscreen->memobj_destroy = nv50_memobj_destroy;
}
