
#ifndef INLINE_SW_HELPER_H
#define INLINE_SW_HELPER_H

#include "util/compiler.h"
#include "pipe/p_screen.h"
#include "util/u_debug.h"
#include "frontend/sw_winsys.h"
#include "target-helpers/inline_debug_helper.h"

/* Helper function to choose and instantiate one of the software rasterizers:
 * llvmpipe, softpipe.
 */

#ifdef GALLIUM_SOFTPIPE
#include "softpipe/sp_public.h"
#endif

#ifdef GALLIUM_LLVMPIPE
#include "llvmpipe/lp_public.h"
#endif

#ifdef GALLIUM_VIRGL
#include "virgl/virgl_public.h"
#include "virgl/vtest/virgl_vtest_public.h"
#endif

#ifdef GALLIUM_D3D12
#include "d3d12/d3d12_public.h"
#endif

static inline struct pipe_screen *
sw_screen_create_named(struct sw_winsys *winsys, const char *driver)
{
   struct pipe_screen *screen = NULL;

#if defined(GALLIUM_LLVMPIPE)
   if (screen == NULL && strcmp(driver, "llvmpipe") == 0)
      screen = llvmpipe_create_screen(winsys);
#endif

#if defined(GALLIUM_VIRGL)
   if (screen == NULL && strcmp(driver, "virpipe") == 0) {
      struct virgl_winsys *vws;
      vws = virgl_vtest_winsys_wrap(winsys);
      screen = virgl_create_screen(vws, NULL);
   }
#endif

#if defined(GALLIUM_SOFTPIPE)
   if (screen == NULL && strcmp(driver, "softpipe") == 0)
      screen = softpipe_create_screen(winsys);
#endif

#if defined(GALLIUM_ZINK)
   if (screen == NULL && strcmp(driver, "zink") == 0)
      screen = zink_create_screen(winsys, NULL);
#endif

#if defined(GALLIUM_D3D12)
   if (screen == NULL && strcmp(driver, "d3d12") == 0)
      screen = d3d12_create_dxcore_screen(winsys, NULL);
#endif

   return screen ? debug_screen_wrap(screen) : NULL;
}


static inline struct pipe_screen *
sw_screen_create_vk(struct sw_winsys *winsys, bool sw_vk)
{
   UNUSED bool only_sw = debug_get_bool_option("LIBGL_ALWAYS_SOFTWARE", false);
   const char *drivers[] = {
      (sw_vk ? "" : debug_get_option("GALLIUM_DRIVER", "")),
#if defined(GALLIUM_D3D12)
      (sw_vk || only_sw) ? "" : "d3d12",
#endif
#if defined(GALLIUM_LLVMPIPE)
      "llvmpipe",
#endif
#if defined(GALLIUM_SOFTPIPE)
      (sw_vk ? "" : "softpipe"),
#endif
   };

   for (unsigned i = 0; i < ARRAY_SIZE(drivers); i++) {
      struct pipe_screen *screen = sw_screen_create_named(winsys, drivers[i]);
      if (screen)
         return screen;
      /* If the env var is set, don't keep trying things */
      else if (i == 0 && drivers[i][0] != '\0')
         return NULL;
   }
   return NULL;
}

static inline struct pipe_screen *
sw_screen_create_zink(struct sw_winsys *winsys, const struct pipe_screen_config *config, bool whatever)
{
#if defined(GALLIUM_ZINK)
   return zink_create_screen(winsys, config);
#else
   return NULL;
#endif
}

static inline struct pipe_screen *
sw_screen_create(struct sw_winsys *winsys)
{
   return sw_screen_create_vk(winsys, false);
}
#endif
