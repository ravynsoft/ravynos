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


static const char *stw_extension_string = 
   "WGL_ARB_create_context "
   "WGL_ARB_create_context_profile "
   "WGL_ARB_create_context_robustness "
   "WGL_ARB_extensions_string "
   "WGL_ARB_make_current_read "
   "WGL_ARB_multisample "
   "WGL_ARB_pbuffer "
   "WGL_ARB_pixel_format "
   "WGL_ARB_render_texture "
   "WGL_EXT_create_context_es_profile "
   "WGL_EXT_create_context_es2_profile "
   "WGL_EXT_extensions_string "
   "WGL_EXT_swap_control";


WINGDIAPI const char * APIENTRY
wglGetExtensionsStringARB(
   HDC hdc )
{
   if (!hdc) {
      return NULL;
   }

   return stw_extension_string;
}


WINGDIAPI const char * APIENTRY
wglGetExtensionsStringEXT( void )
{
   return stw_extension_string;
}
