/**************************************************************************
 *
 * Copyright 2012-2021 VMware, Inc.
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


#include "util/u_debug.h"
#include "target-helpers/inline_debug_helper.h"
#include "llvmpipe/lp_public.h"
#include "softpipe/sp_public.h"
#include "sw/gdi/gdi_sw_winsys.h"


extern struct pipe_screen *
d3d10_create_screen(void);


struct pipe_screen *
d3d10_create_screen(void)
{
   const char *default_driver;
   const char *driver;
   struct pipe_screen *screen = NULL;
   struct sw_winsys *winsys;

   winsys = gdi_create_sw_winsys();
   if(!winsys)
      goto no_winsys;

#ifdef GALLIUM_LLVMPIPE
   default_driver = "llvmpipe";
#else
   default_driver = "softpipe";
#endif

   driver = debug_get_option("GALLIUM_DRIVER", default_driver);

#ifdef GALLIUM_LLVMPIPE
   if (strcmp(driver, "llvmpipe") == 0) {
      screen = llvmpipe_create_screen( winsys );
   }
#else
   (void)driver;
#endif

   if (screen == NULL) {
      screen = softpipe_create_screen( winsys );
   }

   if (screen == NULL)
      goto no_screen;

   return debug_screen_wrap( screen );

no_screen:
   winsys->destroy(winsys);
no_winsys:
   return NULL;
}
