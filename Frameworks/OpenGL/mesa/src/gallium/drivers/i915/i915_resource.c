#include "util/u_debug.h"

#include "i915_context.h"
#include "i915_resource.h"
#include "i915_screen.h"

static struct pipe_resource *
i915_resource_create(struct pipe_screen *screen,
                     const struct pipe_resource *template)
{
   if (template->target == PIPE_BUFFER)
      return i915_buffer_create(screen, template);
   else {
      if (!(template->bind & PIPE_BIND_LINEAR))
         return i915_texture_create(screen, template, false);
      else
         return i915_texture_create(screen, template, true);
   }
}

static struct pipe_resource *
i915_resource_from_handle(struct pipe_screen *screen,
                          const struct pipe_resource *template,
                          struct winsys_handle *whandle, unsigned usage)
{
   if (template->target == PIPE_BUFFER)
      return NULL;
   else
      return i915_texture_from_handle(screen, template, whandle);
}

void
i915_init_resource_functions(struct i915_context *i915)
{
   i915->base.buffer_map = i915_buffer_transfer_map;
   i915->base.texture_map = i915_texture_transfer_map;
   i915->base.transfer_flush_region = u_default_transfer_flush_region;
   i915->base.buffer_unmap = i915_buffer_transfer_unmap;
   i915->base.texture_unmap = i915_texture_transfer_unmap;
   i915->base.buffer_subdata = i915_buffer_subdata;
   i915->base.texture_subdata = i915_texture_subdata;
}

void
i915_init_screen_resource_functions(struct i915_screen *is)
{
   is->base.resource_create = i915_resource_create;
   is->base.resource_from_handle = i915_resource_from_handle;
   is->base.resource_get_handle = i915_resource_get_handle;
   is->base.resource_destroy = i915_resource_destroy;
}
