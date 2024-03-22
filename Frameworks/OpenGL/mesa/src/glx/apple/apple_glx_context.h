/*
 Copyright (c) 2008, 2009 Apple Inc.
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation files
 (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT.  IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT
 HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
 
 Except as contained in this notice, the name(s) of the above
 copyright holders shall not be used in advertising or otherwise to
 promote the sale, use or other dealings in this Software without
 prior written authorization.
*/
#ifndef APPLE_GLX_CONTEXT_H
#define APPLE_GLX_CONTEXT_H

/* <rdar://problem/6953344> */
#define glTexImage1D glTexImage1D_OSX
#define glTexImage2D glTexImage2D_OSX
#define glTexImage3D glTexImage3D_OSX
#include <OpenGL/CGLTypes.h>
#include <OpenGL/CGLContext.h>
#undef glTexImage1D
#undef glTexImage2D
#undef glTexImage3D

#include <stdbool.h>
#include <X11/Xlib.h>
#include <GL/glx.h>
#define XP_NO_X_HEADERS
#include <Xplugin.h>
#undef XP_NO_X_HEADERS

#include "apple_glx_drawable.h"

struct apple_glx_context
{
   CGLContextObj context_obj;
   CGLPixelFormatObj pixel_format_obj;
   struct apple_glx_drawable *drawable;
   pthread_t thread_id;
   int screen;
   bool double_buffered;
   bool uses_stereo;
   bool need_update;
   bool is_current;             /* True if the context is current in some thread. */
   bool made_current;           /* True if the context has ever been made current. */

   /*
    * last_surface is set by the pending_destroy code handler for a drawable.
    * Due to a CG difference, we have to recreate a surface if the window
    * is unmapped and mapped again.
    */
   Window last_surface_window;
   struct apple_glx_context *previous, *next;
};

bool apple_glx_create_context(void **ptr, Display * dpy, int screen,
                              const void *mode, void *sharedContext,
                              int *errorptr, bool * x11errorptr);
void apple_glx_destroy_context(void **ptr, Display * dpy);

bool apple_glx_make_current_context(Display * dpy, void *oldptr, void *ptr,
                                    GLXDrawable drawable);
bool apple_glx_is_current_drawable(Display * dpy, void *ptr,
                                   GLXDrawable drawable);

bool apple_glx_copy_context(void *currentptr, void *srcptr, void *destptr,
                            unsigned long mask, int *errorptr,
                            bool * x11errorptr);

int apple_glx_context_surface_changed(unsigned int uid, pthread_t caller);

void apple_glx_context_update(Display * dpy, void *ptr);

bool apple_glx_context_uses_stereo(void *ptr);

#endif /*APPLE_GLX_CONTEXT_H */
