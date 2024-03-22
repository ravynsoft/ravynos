/**************************************************************************
 * 
 * Copyright 2009 VMware, Inc.
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

#include <windows.h>

#define WGL_WGLEXT_PROTOTYPES

#include <GL/gl.h>
#include <GL/wglext.h>
#include "util/u_debug.h"
#include "stw_device.h"


/**
 * Note that our implementation of swap intervals is a bit of a hack.
 * We implement it based on querying the time and Sleep()'ing.  We don't
 * sync to the vblank.
 */
WINGDIAPI BOOL APIENTRY
wglSwapIntervalEXT(int interval)
{
   if (interval < 0) {
      SetLastError(ERROR_INVALID_DATA);
      return false;
   }
   if (stw_dev && !os_get_option("WGL_SWAP_INTERVAL")) {
      stw_dev->swap_interval = interval;
   }
   return true;
}


WINGDIAPI int APIENTRY
wglGetSwapIntervalEXT(void)
{
   return stw_dev ? stw_dev->swap_interval : 0;
}
