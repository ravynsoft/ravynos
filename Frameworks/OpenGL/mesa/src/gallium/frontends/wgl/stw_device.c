/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include "state_tracker/st_context.h"

#include <windows.h>

#include "glapi/glapi.h"
#include "util/u_debug.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/u_driconf.h"
#include "util/driconf.h"
#include "pipe/p_screen.h"

#include "stw_device.h"
#include "stw_winsys.h"
#include "stw_pixelformat.h"
#include "stw_gdishim.h"
#include "gldrv.h"
#include "stw_tls.h"
#include "stw_framebuffer.h"
#include "stw_st.h"


struct stw_device *stw_dev = NULL;

static int
stw_get_param(struct pipe_frontend_screen *fscreen,
              enum st_manager_param param)
{
   switch (param) {
   case ST_MANAGER_BROKEN_INVALIDATE:
      /*
       * Force framebuffer validation on glViewport.
       *
       * Certain applications, like Rhinoceros 4, uses glReadPixels
       * exclusively (never uses SwapBuffers), so framebuffers never get
       * resized unless we check on glViewport.
       */
      return 1;
   default:
      return 0;
   }
}


/** Get the refresh rate for the monitor, in Hz */
static int
get_refresh_rate(void)
{
#ifndef _GAMING_XBOX
   DEVMODE devModes = { .dmSize = sizeof(DEVMODE) };

   if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devModes)) {
      /* clamp the value, just in case we get garbage */
      return CLAMP(devModes.dmDisplayFrequency, 30, 120);
   }
   else {
      /* reasonable default */
      return 60;
   }
#else
   return 60;
#endif /* _GAMING_XBOX */
}

static bool
init_screen(const struct stw_winsys *stw_winsys, HDC hdc)
{
   struct pipe_screen *screen = stw_winsys->create_screen(hdc);
   if (!screen)
      return false;

   if (stw_winsys->get_adapter_luid)
      stw_winsys->get_adapter_luid(screen, hdc, &stw_dev->AdapterLuid);

   stw_dev->fscreen->screen = screen;
   stw_dev->screen = screen;
   stw_dev->zink = !memcmp(screen->get_name(screen), "zink", 4);

   stw_dev->max_2d_length = screen->get_param(screen,
                                              PIPE_CAP_MAX_TEXTURE_2D_SIZE);
   return true;
}

static const driOptionDescription gallium_driconf[] = {
   #include "pipe-loader/driinfo_gallium.h"
};

static void
init_options()
{
   const char *driver_name = stw_dev->stw_winsys->get_name ? stw_dev->stw_winsys->get_name() : NULL;
   driParseOptionInfo(&stw_dev->option_info, gallium_driconf, ARRAY_SIZE(gallium_driconf));
   driParseConfigFiles(&stw_dev->option_cache, &stw_dev->option_info, 0,
      driver_name ? driver_name : "", NULL, NULL, NULL, 0, NULL, 0);
   
   u_driconf_fill_st_options(&stw_dev->st_options, &stw_dev->option_cache);
}

char *
stw_get_config_xml(void)
{
   return driGetOptionsXml(gallium_driconf, ARRAY_SIZE(gallium_driconf));
}

bool
stw_init(const struct stw_winsys *stw_winsys)
{
   static struct stw_device stw_dev_storage;

   if (debug_get_bool_option("WGL_DISABLE_ERROR_DIALOGS", false))
      debug_disable_win32_error_dialogs();

   assert(!stw_dev);

   stw_tls_init();

   stw_dev = &stw_dev_storage;
   memset(stw_dev, 0, sizeof(*stw_dev));

   stw_dev->stw_winsys = stw_winsys;

   stw_dev->fscreen = CALLOC_STRUCT(pipe_frontend_screen);
   if (!stw_dev->fscreen)
      goto error1;

   stw_dev->fscreen->get_param = stw_get_param;

   InitializeCriticalSection(&stw_dev->screen_mutex);
   InitializeCriticalSection(&stw_dev->ctx_mutex);
   InitializeCriticalSection(&stw_dev->fb_mutex);

   stw_dev->ctx_table = handle_table_create();
   if (!stw_dev->ctx_table) {
      goto error1;
   }

   /* env var override for WGL_EXT_swap_control, useful for testing/debugging */
   const char *s = os_get_option("WGL_SWAP_INTERVAL");
   if (s) {
      stw_dev->swap_interval = atoi(s);
   }
   stw_dev->refresh_rate = get_refresh_rate();

   stw_dev->initialized = true;

   return true;

error1:
   FREE(stw_dev->fscreen);

   stw_dev = NULL;
   return false;
}

bool
stw_init_screen(HDC hdc)
{
   EnterCriticalSection(&stw_dev->screen_mutex);

   if (!stw_dev->screen_initialized) {
      stw_dev->screen_initialized = true;
      if (!init_screen(stw_dev->stw_winsys, hdc)) {
         LeaveCriticalSection(&stw_dev->screen_mutex);
         return false;
      }
      init_options();
      stw_pixelformat_init();
   }

   LeaveCriticalSection(&stw_dev->screen_mutex);
   return stw_dev->screen != NULL;
}

struct stw_device *
stw_get_device(void)
{
   return stw_dev;
}

bool
stw_init_thread(void)
{
   return stw_tls_init_thread();
}


void
stw_cleanup_thread(void)
{
   stw_tls_cleanup_thread();
}


void
stw_cleanup(void)
{
   DHGLRC dhglrc;

   debug_printf("%s\n", __func__);

   if (!stw_dev)
      return;

   /*
    * Abort cleanup if there are still active contexts. In some situations
    * this DLL may be unloaded before the DLL that is using GL contexts is.
    */
   stw_lock_contexts(stw_dev);
   dhglrc = handle_table_get_first_handle(stw_dev->ctx_table);
   stw_unlock_contexts(stw_dev);
   if (dhglrc) {
      debug_printf("%s: contexts still active -- cleanup aborted\n", __func__);
      stw_dev = NULL;
      return;
   }

   free(stw_dev->st_options.force_gl_vendor);
   free(stw_dev->st_options.force_gl_renderer);
   free(stw_dev->st_options.mesa_extension_override);
   driDestroyOptionCache(&stw_dev->option_cache);
   driDestroyOptionInfo(&stw_dev->option_info);

   handle_table_destroy(stw_dev->ctx_table);

   stw_framebuffer_cleanup();

   DeleteCriticalSection(&stw_dev->fb_mutex);
   DeleteCriticalSection(&stw_dev->ctx_mutex);
   DeleteCriticalSection(&stw_dev->screen_mutex);

   st_screen_destroy(stw_dev->fscreen);
   FREE(stw_dev->fscreen);

   if (stw_dev->screen)
      stw_dev->screen->destroy(stw_dev->screen);

   stw_tls_cleanup();

   util_dynarray_fini(&stw_dev->pixelformats);

   stw_dev = NULL;
}


void APIENTRY
DrvSetCallbackProcs(INT nProcs, PROC *pProcs)
{
   size_t size;

   if (stw_dev == NULL)
      return;

   size = MIN2(nProcs * sizeof *pProcs, sizeof stw_dev->callbacks);
   memcpy(&stw_dev->callbacks, pProcs, size);

   return;
}


BOOL APIENTRY
DrvValidateVersion(ULONG ulVersion)
{
   /* ulVersion is the version reported by the KMD:
    * - via D3DKMTQueryAdapterInfo(KMTQAITYPE_UMOPENGLINFO) on WDDM,
    * - or ExtEscape on XPDM and can be used to ensure the KMD and OpenGL ICD
    *   versions match.
    *
    * We should get the expected version number from the winsys, but for now
    * ignore it.
    */
   (void)ulVersion;
   return true;
}
