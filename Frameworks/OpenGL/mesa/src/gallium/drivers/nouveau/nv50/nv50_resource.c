
#include "pipe/p_context.h"
#include "util/u_inlines.h"
#include "util/format/u_format.h"

#include "nouveau_screen.h"

#include "nv50/nv50_resource.h"

static struct pipe_resource *
nv50_resource_create(struct pipe_screen *screen,
                     const struct pipe_resource *templ)
{
   switch (templ->target) {
   case PIPE_BUFFER:
      return nouveau_buffer_create(screen, templ);
   default:
      return nv50_miptree_create(screen, templ);
   }
}

static void
nv50_resource_destroy(struct pipe_screen *pscreen, struct pipe_resource *res)
{
   if (res->target == PIPE_BUFFER)
      nouveau_buffer_destroy(pscreen, res);
   else
      nv50_miptree_destroy(pscreen, res);
}

static struct pipe_resource *
nv50_resource_from_handle(struct pipe_screen * screen,
                          const struct pipe_resource *templ,
                          struct winsys_handle *whandle,
                          unsigned usage)
{
   if (templ->target == PIPE_BUFFER)
      return NULL;
   else
      return nv50_miptree_from_handle(screen, templ, whandle);
}

struct pipe_surface *
nv50_surface_from_buffer(struct pipe_context *pipe,
                         struct pipe_resource *pbuf,
                         const struct pipe_surface *templ)
{
   struct nv50_surface *sf = CALLOC_STRUCT(nv50_surface);
   if (!sf)
      return NULL;

   pipe_reference_init(&sf->base.reference, 1);
   pipe_resource_reference(&sf->base.texture, pbuf);

   sf->base.format = templ->format;
   sf->base.writable = templ->writable;
   sf->base.u.buf.first_element = templ->u.buf.first_element;
   sf->base.u.buf.last_element = templ->u.buf.last_element;

   sf->offset =
      templ->u.buf.first_element * util_format_get_blocksize(sf->base.format);

   sf->offset &= ~0x7f; /* FIXME: RT_ADDRESS requires 128 byte alignment */

   sf->width = templ->u.buf.last_element - templ->u.buf.first_element + 1;
   sf->height = 1;
   sf->depth = 1;

   sf->base.width = sf->width;
   sf->base.height = sf->height;

   sf->base.context = pipe;
   return &sf->base;
}

static struct pipe_surface *
nv50_surface_create(struct pipe_context *pipe,
                    struct pipe_resource *pres,
                    const struct pipe_surface *templ)
{
   if (unlikely(pres->target == PIPE_BUFFER))
      return nv50_surface_from_buffer(pipe, pres, templ);
   return nv50_miptree_surface_new(pipe, pres, templ);
}

void
nv50_surface_destroy(struct pipe_context *pipe, struct pipe_surface *ps)
{
   struct nv50_surface *s = nv50_surface(ps);

   pipe_resource_reference(&ps->texture, NULL);

   FREE(s);
}

void
nv50_invalidate_resource(struct pipe_context *pipe, struct pipe_resource *res)
{
   if (res->target == PIPE_BUFFER)
      nouveau_buffer_invalidate(pipe, res);
}

struct pipe_memory_object *
nv50_memobj_create_from_handle(struct pipe_screen *screen,
                               struct winsys_handle *handle,
                               bool dedicated)
{
   struct nv50_memobj *memobj = CALLOC_STRUCT(nv50_memobj);

   memobj->bo = nouveau_screen_bo_from_handle(screen, handle, &memobj->stride);
   if (memobj->bo == NULL) {
      FREE(memobj);
      return NULL;
   }
   memobj->handle = handle;
   memobj->b.dedicated = dedicated;

   return &memobj->b;
}

void
nv50_memobj_destroy(struct pipe_screen *screen,
                    struct pipe_memory_object *pmemobj)
{
   struct nv50_memobj *memobj = (struct nv50_memobj *)pmemobj;

   free(memobj->handle);
   free(memobj->bo);
   free(memobj);
}

struct pipe_resource *
nv50_resource_from_memobj(struct pipe_screen *screen,
                          const struct pipe_resource *templ,
                          struct pipe_memory_object *pmemobj,
                          uint64_t offset)
{
   struct nv50_miptree *mt;
   struct nv50_memobj *memobj = (struct nv50_memobj *)pmemobj;

   /* only supports 2D, non-mipmapped textures for the moment */
   if ((templ->target != PIPE_TEXTURE_2D &&
        templ->target != PIPE_TEXTURE_RECT) ||
       templ->last_level != 0 ||
       templ->depth0 != 1 ||
       templ->array_size > 1)
      return NULL;

   mt = CALLOC_STRUCT(nv50_miptree);
   if (!mt)
      return NULL;

   mt->base.bo = memobj->bo;

   mt->base.domain = mt->base.bo->flags & NOUVEAU_BO_APER;
   mt->base.address = mt->base.bo->offset;

   mt->base.base = *templ;
   pipe_reference_init(&mt->base.base.reference, 1);
   mt->base.base.screen = screen;
   mt->level[0].offset = 0;
   mt->level[0].tile_mode = mt->base.bo->config.nv50.tile_mode;

   NOUVEAU_DRV_STAT(nouveau_screen(screen), tex_obj_current_count, 1);

   /* no need to adjust bo reference count */
   return &mt->base.base;
}

void
nv50_init_resource_functions(struct pipe_context *pcontext)
{
   pcontext->buffer_map = nouveau_buffer_transfer_map;
   pcontext->texture_map = nv50_miptree_transfer_map;
   pcontext->transfer_flush_region = nouveau_buffer_transfer_flush_region;
   pcontext->buffer_unmap = nouveau_buffer_transfer_unmap;
   pcontext->texture_unmap = nv50_miptree_transfer_unmap;
   pcontext->buffer_subdata = u_default_buffer_subdata;
   pcontext->texture_subdata = u_default_texture_subdata;
   pcontext->create_surface = nv50_surface_create;
   pcontext->surface_destroy = nv50_surface_destroy;
   pcontext->invalidate_resource = nv50_invalidate_resource;
}

void
nv50_screen_init_resource_functions(struct pipe_screen *pscreen)
{
   pscreen->resource_create = nv50_resource_create;
   pscreen->resource_from_handle = nv50_resource_from_handle;
   pscreen->resource_get_handle = nv50_miptree_get_handle;
   pscreen->resource_destroy = nv50_resource_destroy;

   pscreen->memobj_create_from_handle = nv50_memobj_create_from_handle;
   pscreen->resource_from_memobj = nv50_resource_from_memobj;
   pscreen->memobj_destroy = nv50_memobj_destroy;
}
