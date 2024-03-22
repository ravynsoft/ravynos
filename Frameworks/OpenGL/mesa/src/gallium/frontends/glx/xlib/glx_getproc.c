/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 * Copyright (C) 2009  VMware, Inc.  All Rights Reserved.
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


/**
 * glXGetProcAddress()
 */


#define GLX_GLXEXT_PROTOTYPES

#include <string.h>
#include "util/compiler.h"
#include "GL/glx.h"
#include "glapi/glapi.h"


struct name_address_pair {
   const char *Name;
   __GLXextFuncPtr Address;
};


static const struct name_address_pair GLX_functions[] = {
   /*** GLX_VERSION_1_0 ***/
   { "glXChooseVisual", (__GLXextFuncPtr) glXChooseVisual },
   { "glXCopyContext", (__GLXextFuncPtr) glXCopyContext },
   { "glXCreateContext", (__GLXextFuncPtr) glXCreateContext },
   { "glXCreateGLXPixmap", (__GLXextFuncPtr) glXCreateGLXPixmap },
   { "glXDestroyContext", (__GLXextFuncPtr) glXDestroyContext },
   { "glXDestroyGLXPixmap", (__GLXextFuncPtr) glXDestroyGLXPixmap },
   { "glXGetConfig", (__GLXextFuncPtr) glXGetConfig },
   { "glXGetCurrentContext", (__GLXextFuncPtr) glXGetCurrentContext },
   { "glXGetCurrentDrawable", (__GLXextFuncPtr) glXGetCurrentDrawable },
   { "glXIsDirect", (__GLXextFuncPtr) glXIsDirect },
   { "glXMakeCurrent", (__GLXextFuncPtr) glXMakeCurrent },
   { "glXQueryExtension", (__GLXextFuncPtr) glXQueryExtension },
   { "glXQueryVersion", (__GLXextFuncPtr) glXQueryVersion },
   { "glXSwapBuffers", (__GLXextFuncPtr) glXSwapBuffers },
   { "glXUseXFont", (__GLXextFuncPtr) glXUseXFont },
   { "glXWaitGL", (__GLXextFuncPtr) glXWaitGL },
   { "glXWaitX", (__GLXextFuncPtr) glXWaitX },

   /*** GLX_VERSION_1_1 ***/
   { "glXGetClientString", (__GLXextFuncPtr) glXGetClientString },
   { "glXQueryExtensionsString", (__GLXextFuncPtr) glXQueryExtensionsString },
   { "glXQueryServerString", (__GLXextFuncPtr) glXQueryServerString },

   /*** GLX_VERSION_1_2 ***/
   { "glXGetCurrentDisplay", (__GLXextFuncPtr) glXGetCurrentDisplay },

   /*** GLX_VERSION_1_3 ***/
   { "glXChooseFBConfig", (__GLXextFuncPtr) glXChooseFBConfig },
   { "glXCreateNewContext", (__GLXextFuncPtr) glXCreateNewContext },
   { "glXCreatePbuffer", (__GLXextFuncPtr) glXCreatePbuffer },
   { "glXCreatePixmap", (__GLXextFuncPtr) glXCreatePixmap },
   { "glXCreateWindow", (__GLXextFuncPtr) glXCreateWindow },
   { "glXDestroyPbuffer", (__GLXextFuncPtr) glXDestroyPbuffer },
   { "glXDestroyPixmap", (__GLXextFuncPtr) glXDestroyPixmap },
   { "glXDestroyWindow", (__GLXextFuncPtr) glXDestroyWindow },
   { "glXGetCurrentReadDrawable", (__GLXextFuncPtr) glXGetCurrentReadDrawable },
   { "glXGetFBConfigAttrib", (__GLXextFuncPtr) glXGetFBConfigAttrib },
   { "glXGetFBConfigs", (__GLXextFuncPtr) glXGetFBConfigs },
   { "glXGetSelectedEvent", (__GLXextFuncPtr) glXGetSelectedEvent },
   { "glXGetVisualFromFBConfig", (__GLXextFuncPtr) glXGetVisualFromFBConfig },
   { "glXMakeContextCurrent", (__GLXextFuncPtr) glXMakeContextCurrent },
   { "glXQueryContext", (__GLXextFuncPtr) glXQueryContext },
   { "glXQueryDrawable", (__GLXextFuncPtr) glXQueryDrawable },
   { "glXSelectEvent", (__GLXextFuncPtr) glXSelectEvent },

   /*** GLX_VERSION_1_4 ***/
   { "glXGetProcAddress", (__GLXextFuncPtr) glXGetProcAddress },

   /*** GLX_SGI_swap_control ***/
   { "glXSwapIntervalSGI", (__GLXextFuncPtr) glXSwapIntervalSGI },

   /*** GLX_SGI_video_sync ***/
   { "glXGetVideoSyncSGI", (__GLXextFuncPtr) glXGetVideoSyncSGI },
   { "glXWaitVideoSyncSGI", (__GLXextFuncPtr) glXWaitVideoSyncSGI },

   /*** GLX_SGI_make_current_read ***/
   { "glXMakeCurrentReadSGI", (__GLXextFuncPtr) glXMakeCurrentReadSGI },
   { "glXGetCurrentReadDrawableSGI", (__GLXextFuncPtr) glXGetCurrentReadDrawableSGI },

   /*** GLX_SGIX_video_source ***/
#if defined(_VL_H)
   { "glXCreateGLXVideoSourceSGIX", (__GLXextFuncPtr) glXCreateGLXVideoSourceSGIX },
   { "glXDestroyGLXVideoSourceSGIX", (__GLXextFuncPtr) glXDestroyGLXVideoSourceSGIX },
#endif

   /*** GLX_EXT_import_context ***/
   { "glXFreeContextEXT", (__GLXextFuncPtr) glXFreeContextEXT },
   { "glXGetContextIDEXT", (__GLXextFuncPtr) glXGetContextIDEXT },
   { "glXGetCurrentDisplayEXT", (__GLXextFuncPtr) glXGetCurrentDisplayEXT },
   { "glXImportContextEXT", (__GLXextFuncPtr) glXImportContextEXT },
   { "glXQueryContextInfoEXT", (__GLXextFuncPtr) glXQueryContextInfoEXT },

   /*** GLX_SGIX_fbconfig ***/
   { "glXGetFBConfigAttribSGIX", (__GLXextFuncPtr) glXGetFBConfigAttribSGIX },
   { "glXChooseFBConfigSGIX", (__GLXextFuncPtr) glXChooseFBConfigSGIX },
   { "glXCreateGLXPixmapWithConfigSGIX", (__GLXextFuncPtr) glXCreateGLXPixmapWithConfigSGIX },
   { "glXCreateContextWithConfigSGIX", (__GLXextFuncPtr) glXCreateContextWithConfigSGIX },
   { "glXGetVisualFromFBConfigSGIX", (__GLXextFuncPtr) glXGetVisualFromFBConfigSGIX },
   { "glXGetFBConfigFromVisualSGIX", (__GLXextFuncPtr) glXGetFBConfigFromVisualSGIX },

   /*** GLX_SGIX_pbuffer ***/
   { "glXCreateGLXPbufferSGIX", (__GLXextFuncPtr) glXCreateGLXPbufferSGIX },
   { "glXDestroyGLXPbufferSGIX", (__GLXextFuncPtr) glXDestroyGLXPbufferSGIX },
   { "glXQueryGLXPbufferSGIX", (__GLXextFuncPtr) glXQueryGLXPbufferSGIX },
   { "glXSelectEventSGIX", (__GLXextFuncPtr) glXSelectEventSGIX },
   { "glXGetSelectedEventSGIX", (__GLXextFuncPtr) glXGetSelectedEventSGIX },

   /*** GLX_SGI_cushion ***/
   { "glXCushionSGI", (__GLXextFuncPtr) glXCushionSGI },

   /*** GLX_SGIX_video_resize ***/
   { "glXBindChannelToWindowSGIX", (__GLXextFuncPtr) glXBindChannelToWindowSGIX },
   { "glXChannelRectSGIX", (__GLXextFuncPtr) glXChannelRectSGIX },
   { "glXQueryChannelRectSGIX", (__GLXextFuncPtr) glXQueryChannelRectSGIX },
   { "glXQueryChannelDeltasSGIX", (__GLXextFuncPtr) glXQueryChannelDeltasSGIX },
   { "glXChannelRectSyncSGIX", (__GLXextFuncPtr) glXChannelRectSyncSGIX },

   /*** GLX_SGIX_dmbuffer **/
#if defined(_DM_BUFFER_H_)
   { "glXAssociateDMPbufferSGIX", (__GLXextFuncPtr) glXAssociateDMPbufferSGIX },
#endif

   /*** GLX_SUN_get_transparent_index ***/
   { "glXGetTransparentIndexSUN", (__GLXextFuncPtr) glXGetTransparentIndexSUN },

   /*** GLX_MESA_copy_sub_buffer ***/
   { "glXCopySubBufferMESA", (__GLXextFuncPtr) glXCopySubBufferMESA },

   /*** GLX_MESA_pixmap_colormap ***/
   { "glXCreateGLXPixmapMESA", (__GLXextFuncPtr) glXCreateGLXPixmapMESA },

   /*** GLX_MESA_release_buffers ***/
   { "glXReleaseBuffersMESA", (__GLXextFuncPtr) glXReleaseBuffersMESA },

   /*** GLX_ARB_get_proc_address ***/
   { "glXGetProcAddressARB", (__GLXextFuncPtr) glXGetProcAddressARB },

   /*** GLX_ARB_create_context ***/
   { "glXCreateContextAttribsARB", (__GLXextFuncPtr) glXCreateContextAttribsARB },

   /*** GLX_EXT_texture_from_pixmap ***/
   { "glXBindTexImageEXT", (__GLXextFuncPtr) glXBindTexImageEXT },
   { "glXReleaseTexImageEXT", (__GLXextFuncPtr) glXReleaseTexImageEXT },

   { NULL, NULL }   /* end of list */
};



/**
 * Return address of named glX function, or NULL if not found.
 */
static __GLXextFuncPtr
_glxapi_get_proc_address(const char *funcName)
{
   GLuint i;
   for (i = 0; GLX_functions[i].Name; i++) {
      if (strcmp(GLX_functions[i].Name, funcName) == 0)
         return GLX_functions[i].Address;
   }
   return NULL;
}


PUBLIC __GLXextFuncPtr
glXGetProcAddressARB(const GLubyte *procName)
{
   __GLXextFuncPtr f;

   f = _glxapi_get_proc_address((const char *) procName);
   if (f) {
      return f;
   }

   f = (__GLXextFuncPtr) _glapi_get_proc_address((const char *) procName);
   return f;
}


/* GLX 1.4 */
PUBLIC
void (*glXGetProcAddress(const GLubyte *procName))()
{
   return glXGetProcAddressARB(procName);
}
