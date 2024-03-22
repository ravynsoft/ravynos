/*
 Copyright (c) 2008 Apple Inc.
 
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

#ifndef APPLE_GLX_H
#define APPLE_GLX_H

#include <stdbool.h>
#include <GL/gl.h>
#include <X11/Xlib.h>

#define XP_NO_X_HEADERS
#include <Xplugin.h>

#include "apple_glx_log.h"

xp_client_id apple_glx_get_client_id(void);
bool apple_init_glx(Display * dpy);
void apple_glx_swap_buffers(void *ptr);
void apple_glx_waitx(Display * dpy, void *ptr);
int apple_get_dri_event_base(void);

void apple_glapi_set_dispatch(void);
void apple_glapi_oglfw_viewport_scissor(GLint x, GLint y, GLsizei width, GLsizei height);

#endif
