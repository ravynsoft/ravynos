/*
 * Copyright © 2014 Jon Turney
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef windowsgl_h
#define windowsgh_h

struct _windowsContext;
struct _windowsDrawable;

typedef struct _windowsContext windowsContext;
typedef struct _windowsDrawable windowsDrawable;

windowsContext *windows_create_context(int pxfi, windowsContext *shared);
windowsContext *windows_create_context_attribs(int pxfi, windowsContext *shared, const int *attribList);
void windows_destroy_context(windowsContext *contex);
int windows_bind_context(windowsContext *context, windowsDrawable *, windowsDrawable *);
void windows_unbind_context(windowsContext *context);

windowsDrawable *windows_create_drawable(int type, void *handle);
void windows_destroy_drawable(windowsDrawable *);
void windows_swap_buffers(windowsDrawable *);
void windows_copy_subbuffer(windowsDrawable *windowsDrawable, int x, int y, int width, int height);

int windows_check_renderer(void);
void windows_extensions(char **gl_extensions, char **wgl_extensions);

#endif /* windowsgl_h */
