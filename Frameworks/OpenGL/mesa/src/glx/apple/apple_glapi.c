/*
 * GLX implementation that uses Apple's OpenGL.framework
 *
 * Copyright (c) 2007-2011 Apple Inc.
 * Copyright (c) 2004 Torrey T. Lyons. All Rights Reserved.
 * Copyright (c) 2002 Greg Parker. All Rights Reserved.
 *
 * Portions of this file are copied from Mesa's xf86glx.c,
 * which contains the following copyright:
 *
 * Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <assert.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <GL/gl.h>

#include "glapi.h"
#include "glapitable.h"

#include "apple_glx.h"
#include "apple_xgl_api.h"
#include "apple_cgl.h"

struct _glapi_table * __ogl_framework_api = NULL;
struct _glapi_table * __applegl_api = NULL;

static void _apple_glapi_create_table(void) {
    if (__applegl_api)
        return;

    __ogl_framework_api = _glapi_create_table_from_handle(apple_cgl_get_dl_handle(), "gl");
    assert(__ogl_framework_api);

    __applegl_api = malloc(sizeof(struct _glapi_table));
    assert(__applegl_api);
    memcpy(__applegl_api, __ogl_framework_api, sizeof(struct _glapi_table));

    _glapi_table_patch(__applegl_api, "ReadPixels", __applegl_glReadPixels);
    _glapi_table_patch(__applegl_api, "CopyPixels", __applegl_glCopyPixels);
    _glapi_table_patch(__applegl_api, "CopyColorTable", __applegl_glCopyColorTable);
    _glapi_table_patch(__applegl_api, "DrawBuffers", __applegl_glDrawBuffer);
    _glapi_table_patch(__applegl_api, "Viewport", __applegl_glViewport);
}

void apple_glapi_set_dispatch(void) {
    _apple_glapi_create_table();
    _glapi_set_dispatch(__applegl_api);
}

void apple_glapi_oglfw_viewport_scissor(GLint x, GLint y, GLsizei width, GLsizei height) {
    _apple_glapi_create_table();
    __ogl_framework_api->Viewport(x, y, width, height);
    __ogl_framework_api->Scissor(x, y, width, height);
}
