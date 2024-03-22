/*
 * Copyright (c) 2013  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */


#include "target-helpers/inline_sw_helper.h"
#include "target-helpers/inline_debug_helper.h"

#include "sw/null/null_sw_winsys.h"


struct pipe_screen *
osmesa_create_screen(void);


struct pipe_screen *
osmesa_create_screen(void)
{
   struct sw_winsys *winsys;
   struct pipe_screen *screen;

   /* We use a null software winsys since we always just render to ordinary
    * driver resources.
    */
   winsys = null_sw_create();
   if (!winsys)
      return NULL;

   /* Create llvmpipe or softpipe screen */
   screen = sw_screen_create(winsys);
   if (!screen) {
      winsys->destroy(winsys);
      return NULL;
   }

   /* Inject optional trace, debug, etc. wrappers */
   return debug_screen_wrap(screen);
}
