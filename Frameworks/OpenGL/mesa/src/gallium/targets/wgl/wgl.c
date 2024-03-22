/**************************************************************************
 *
 * Copyright 2009-2010 VMware, Inc.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 *
 **************************************************************************/

/**
 * @file
 * Softpipe/LLVMpipe support.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 */


#include <windows.h>

#include "util/u_debug.h"
#include "stw_winsys.h"
#include "stw_device.h"
#include "gdi/gdi_sw_winsys.h"
#include "pipe/p_screen.h"
#include "pipe/p_context.h"

#ifdef GALLIUM_SOFTPIPE
#include "softpipe/sp_texture.h"
#include "softpipe/sp_screen.h"
#include "softpipe/sp_public.h"
#endif

#ifdef GALLIUM_LLVMPIPE
#include "llvmpipe/lp_texture.h"
#include "llvmpipe/lp_screen.h"
#include "llvmpipe/lp_public.h"
#endif

#ifdef GALLIUM_D3D12
#include "d3d12/wgl/d3d12_wgl_public.h"
#endif

#ifdef GALLIUM_ZINK
#include "zink/zink_public.h"
#endif

#ifdef GALLIUM_LLVMPIPE
static bool use_llvmpipe = false;
#endif
#ifdef GALLIUM_D3D12
static bool use_d3d12 = false;
#endif
#ifdef GALLIUM_ZINK
static bool use_zink = false;
#endif

static const char *created_driver_name = NULL;

static struct pipe_screen *
wgl_screen_create_by_name(HDC hDC, const char* driver, struct sw_winsys *winsys)
{
   struct pipe_screen* screen = NULL;

#ifdef GALLIUM_LLVMPIPE
   if (strcmp(driver, "llvmpipe") == 0) {
      screen = llvmpipe_create_screen(winsys);
      if (screen)
         use_llvmpipe = true;
   }
#endif
#ifdef GALLIUM_D3D12
   if (strcmp(driver, "d3d12") == 0) {
      screen = d3d12_wgl_create_screen(winsys, hDC);
      if (screen)
         use_d3d12 = true;
   }
#endif
#ifdef GALLIUM_ZINK
   if (strcmp(driver, "zink") == 0) {
      screen = zink_create_screen(winsys, NULL);
      if (screen)
         use_zink = true;
   }
#endif
#ifdef GALLIUM_SOFTPIPE
   if (strcmp(driver, "softpipe") == 0) {
      screen = softpipe_create_screen(winsys);
   }
#endif

   return screen;
}

static struct pipe_screen *
wgl_screen_create(HDC hDC)
{
   struct sw_winsys *winsys;
   UNUSED bool sw_only = debug_get_bool_option("LIBGL_ALWAYS_SOFTWARE", false);

   winsys = gdi_create_sw_winsys();
   if (!winsys)
      return NULL;

   const char *const drivers[] = {
      debug_get_option("GALLIUM_DRIVER", ""),
#ifdef GALLIUM_D3D12
      sw_only ? "" : "d3d12",
#endif
#ifdef GALLIUM_ZINK
      sw_only ? "" : "zink",
#endif
#if defined(GALLIUM_LLVMPIPE)
      "llvmpipe",
#endif
#if defined(GALLIUM_SOFTPIPE)
      "softpipe",
#endif
   };

   /* If the default driver screen creation fails, fall back to the next option in the
    * sorted list. Don't do this if GALLIUM_DRIVER is specified.
    */
   for (unsigned i = 0; i < ARRAY_SIZE(drivers); ++i) {
      struct pipe_screen* screen = wgl_screen_create_by_name(hDC, drivers[i], winsys);
      if (screen) {
         created_driver_name = drivers[i];
         return screen;
      }
      if (i == 0 && drivers[i][0] != '\0')
         break;
   }

   winsys->destroy(winsys);
   return NULL;
}


static void
wgl_present(struct pipe_screen *screen,
            struct pipe_context *ctx,
            struct pipe_resource *res,
            HDC hDC)
{
   /* This will fail if any interposing layer (trace, debug, etc) has
    * been introduced between the gallium frontends and the pipe driver.
    *
    * Ideally this would get replaced with a call to
    * pipe_screen::flush_frontbuffer().
    *
    * Failing that, it may be necessary for intervening layers to wrap
    * other structs such as this stw_winsys as well...
    */

#if defined(GALLIUM_LLVMPIPE) || defined(GALLIUM_SOFTPIPE)
   struct sw_winsys *winsys = NULL;
   struct sw_displaytarget *dt = NULL;
#endif

#ifdef GALLIUM_LLVMPIPE
   if (use_llvmpipe) {
      winsys = llvmpipe_screen(screen)->winsys;
      dt = llvmpipe_resource(res)->dt;
      gdi_sw_display(winsys, dt, hDC);
      return;
   }
#endif

#ifdef GALLIUM_D3D12
   if (use_d3d12) {
      d3d12_wgl_present(screen, ctx, res, hDC);
      return;
   }
#endif

#ifdef GALLIUM_ZINK
   if (use_zink) {
      screen->flush_frontbuffer(screen, ctx, res, 0, 0, hDC, NULL);
      return;
   }
#endif

#ifdef GALLIUM_SOFTPIPE
   winsys = softpipe_screen(screen)->winsys,
   dt = softpipe_resource(res)->dt,
   gdi_sw_display(winsys, dt, hDC);
#endif
}


#if WINVER >= 0xA00
static bool
wgl_get_adapter_luid(struct pipe_screen* screen,
   HDC hDC,
   LUID* adapter_luid)
{
   if (!stw_dev || !stw_dev->callbacks.pfnGetAdapterLuid)
      return false;

   stw_dev->callbacks.pfnGetAdapterLuid(hDC, adapter_luid);
   return true;
}
#endif


static unsigned
wgl_get_pfd_flags(struct pipe_screen *screen)
{
#ifdef GALLIUM_D3D12
   if (use_d3d12)
      return d3d12_wgl_get_pfd_flags(screen);
#endif
   return stw_pfd_gdi_support;
}


static struct stw_winsys_framebuffer *
wgl_create_framebuffer(struct pipe_screen *screen,
                       HWND hWnd,
                       int iPixelFormat)
{
#ifdef GALLIUM_D3D12
   if (use_d3d12)
      return d3d12_wgl_create_framebuffer(screen, hWnd, iPixelFormat);
#endif
   return NULL;
}

static const char *
wgl_get_name(void)
{
   return created_driver_name;
}


static const struct stw_winsys stw_winsys = {
   &wgl_screen_create,
   &wgl_present,
#if WINVER >= 0xA00
   &wgl_get_adapter_luid,
#else
   NULL, /* get_adapter_luid */
#endif
   NULL, /* shared_surface_open */
   NULL, /* shared_surface_close */
   NULL, /* compose */
   &wgl_get_pfd_flags,
   &wgl_create_framebuffer,
   &wgl_get_name,
};


EXTERN_C BOOL WINAPI
DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);


BOOL WINAPI
DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
   switch (fdwReason) {
   case DLL_PROCESS_ATTACH:
      stw_init(&stw_winsys);
      stw_init_thread();
      break;

   case DLL_THREAD_ATTACH:
      stw_init_thread();
      break;

   case DLL_THREAD_DETACH:
      stw_cleanup_thread();
      break;

   case DLL_PROCESS_DETACH:
      if (lpvReserved == NULL) {
         // We're being unloaded from the process.
         stw_cleanup_thread();
         stw_cleanup();
      } else {
         // Process itself is terminating, and all threads and modules are
         // being detached.
         //
         // The order threads (including llvmpipe rasterizer threads) are
         // destroyed can not be relied up, so it's not safe to cleanup.
         //
         // However global destructors (e.g., LLVM's) will still be called, and
         // if Microsoft OPENGL32.DLL's DllMain is called after us, it will
         // still try to invoke DrvDeleteContext to destroys all outstanding,
         // so set stw_dev to NULL to return immediately if that happens.
         stw_dev = NULL;
      }
      break;
   }
   return true;
}
