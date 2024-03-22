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

#include "windowsgl.h"
#include "windowsgl_internal.h"

#include "glapi.h"
#include "wgl.h"

#include <dlfcn.h>
#include <assert.h>
#include <stdio.h>
#include <strings.h>

static struct _glapi_table *windows_api = NULL;

static void *
windows_get_dl_handle(void)
{
   static void *dl_handle = NULL;

   if (!dl_handle)
      dl_handle = dlopen("cygnativeGLthunk.dll", RTLD_NOW);

   return dl_handle;
}

static void
windows_glapi_create_table(void)
{
   if (windows_api)
      return;

   windows_api = _glapi_create_table_from_handle(windows_get_dl_handle(), "gl");
   assert(windows_api);
}

static void windows_glapi_set_dispatch(void)
{
   windows_glapi_create_table();
   _glapi_set_dispatch(windows_api);
}

windowsContext *
windows_create_context(int pxfi, windowsContext *shared)
{
   windowsContext *gc;

   gc = calloc(1, sizeof *gc);
   if (gc == NULL)
      return NULL;

   // create a temporary, invisible window
#define GL_TEMP_WINDOW_CLASS "glTempWndClass"
   {
      static wATOM glTempWndClass = 0;

      if (glTempWndClass == 0) {
         WNDCLASSEX wc;
         wc.cbSize = sizeof(WNDCLASSEX);
         wc.style = CS_HREDRAW | CS_VREDRAW;
         wc.lpfnWndProc = DefWindowProc;
         wc.cbClsExtra = 0;
         wc.cbWndExtra = 0;
         wc.hInstance = GetModuleHandle(NULL);
         wc.hIcon = 0;
         wc.hCursor = 0;
         wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
         wc.lpszMenuName = NULL;
         wc.lpszClassName = GL_TEMP_WINDOW_CLASS;
         wc.hIconSm = 0;
         RegisterClassEx(&wc);
      }
   }

   HWND hwnd = CreateWindowExA(0,
                               GL_TEMP_WINDOW_CLASS,
                               "glWindow",
                               0,
                               0, 0, 0, 0,
                               NULL, NULL, GetModuleHandle(NULL), NULL);
   HDC hdc = GetDC(hwnd);

   // We must set the windows pixel format before we can create a WGL context
   gc->pxfi = pxfi;
   SetPixelFormat(hdc, gc->pxfi, NULL);

   gc->ctx = wglCreateContext(hdc);

   if (shared && gc->ctx)
      wglShareLists(shared->ctx, gc->ctx);

   ReleaseDC(hwnd, hdc);
   DestroyWindow(hwnd);

   if (!gc->ctx)
   {
     free(gc);
     return NULL;
   }

   return gc;
}

windowsContext *
windows_create_context_attribs(int pxfi, windowsContext *shared, const int *attribList)
{
   windowsContext *gc;

   gc = calloc(1, sizeof *gc);
   if (gc == NULL)
      return NULL;

   // create a temporary, invisible window
#define GL_TEMP_WINDOW_CLASS "glTempWndClass"
   {
      static wATOM glTempWndClass = 0;

      if (glTempWndClass == 0) {
         WNDCLASSEX wc;
         wc.cbSize = sizeof(WNDCLASSEX);
         wc.style = CS_HREDRAW | CS_VREDRAW;
         wc.lpfnWndProc = DefWindowProc;
         wc.cbClsExtra = 0;
         wc.cbWndExtra = 0;
         wc.hInstance = GetModuleHandle(NULL);
         wc.hIcon = 0;
         wc.hCursor = 0;
         wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
         wc.lpszMenuName = NULL;
         wc.lpszClassName = GL_TEMP_WINDOW_CLASS;
         wc.hIconSm = 0;
         RegisterClassEx(&wc);
      }
   }

   HWND hwnd = CreateWindowExA(0,
                               GL_TEMP_WINDOW_CLASS,
                               "glWindow",
                               0,
                               0, 0, 0, 0,
                               NULL, NULL, GetModuleHandle(NULL), NULL);
   HDC hdc = GetDC(hwnd);
   HGLRC shareContext = NULL;
   if (shared)
      shareContext = shared->ctx;

   // We must set the windows pixel format before we can create a WGL context
   gc->pxfi = pxfi;
   SetPixelFormat(hdc, gc->pxfi, NULL);

   gc->ctx = wglCreateContextAttribsARB(hdc, shareContext, attribList);

   ReleaseDC(hwnd, hdc);
   DestroyWindow(hwnd);

   if (!gc->ctx)
   {
     free(gc);
     return NULL;
   }

   return gc;
}

void
windows_destroy_context(windowsContext *context)
{
   wglDeleteContext(context->ctx);
   context->ctx = NULL;
   free(context);
}

int windows_bind_context(windowsContext *context, windowsDrawable *draw, windowsDrawable *read)
{
   HDC drawDc = draw->callbacks->getdc(draw);

   if (!draw->pxfi)
   {
      SetPixelFormat(drawDc, context->pxfi, NULL);
      draw->pxfi = context->pxfi;
   }

   if ((read != NULL) &&  (read != draw))
   {
      /*
        If there is a separate read drawable, create a separate read DC, and
        use the wglMakeContextCurrent extension to make the context current
        drawing to one DC and reading from the other

        Should only occur when WGL_ARB_make_current_read extension is present
      */
      HDC readDc = read->callbacks->getdc(read);

      BOOL ret = wglMakeContextCurrentARB(drawDc, readDc, context->ctx);

      read->callbacks->releasedc(read, readDc);

      if (!ret) {
         printf("wglMakeContextCurrentARB error: %08x\n", (int)GetLastError());
         return false;
      }
   }
   else
   {
      /* Otherwise, just use wglMakeCurrent */
      BOOL ret = wglMakeCurrent(drawDc, context->ctx);
      if (!ret) {
         printf("wglMakeCurrent error: %08x\n", (int)GetLastError());
         return false;
      }
   }

   draw->callbacks->releasedc(draw, drawDc);

   windows_glapi_set_dispatch();

   return true;
}

void windows_unbind_context(windowsContext * context)
{
   wglMakeCurrent(NULL, NULL);
}

/*
 *
 */

void
windows_swap_buffers(windowsDrawable *draw)
{
   HDC drawDc = GetDC(draw->hWnd);
   SwapBuffers(drawDc);
   ReleaseDC(draw->hWnd, drawDc);
}


typedef void (__stdcall * PFNGLADDSWAPHINTRECTWIN) (GLint x, GLint y,
                                                    GLsizei width,
                                                    GLsizei height);

static void
glAddSwapHintRectWIN(GLint x, GLint y, GLsizei width,
                            GLsizei height)
{
   PFNGLADDSWAPHINTRECTWIN proc = (PFNGLADDSWAPHINTRECTWIN)wglGetProcAddress("glAddSwapHintRectWIN");
   if (proc)
      proc(x, y, width, height);
}

void
windows_copy_subbuffer(windowsDrawable *draw,
                      int x, int y, int width, int height)
{
   glAddSwapHintRectWIN(x, y, width, height);
   windows_swap_buffers(draw);
}

/*
 * Helper function for calling a test function on an initial context
 */
static void
windows_call_with_context(void (*proc)(HDC, void *), void *args)
{
   // create window class
#define WIN_GL_TEST_WINDOW_CLASS "GLTest"
   {
      static wATOM glTestWndClass = 0;

      if (glTestWndClass == 0) {
         WNDCLASSEX wc;

         wc.cbSize = sizeof(WNDCLASSEX);
         wc.style = CS_HREDRAW | CS_VREDRAW;
         wc.lpfnWndProc = DefWindowProc;
         wc.cbClsExtra = 0;
         wc.cbWndExtra = 0;
         wc.hInstance = GetModuleHandle(NULL);
         wc.hIcon = 0;
         wc.hCursor = 0;
         wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
         wc.lpszMenuName = NULL;
         wc.lpszClassName = WIN_GL_TEST_WINDOW_CLASS;
         wc.hIconSm = 0;
         glTestWndClass = RegisterClassEx(&wc);
      }
   }

   // create an invisible window for a scratch DC
   HWND hwnd = CreateWindowExA(0,
                               WIN_GL_TEST_WINDOW_CLASS,
                               "GL Renderer Capabilities Test Window",
                               0, 0, 0, 0, 0, NULL, NULL, GetModuleHandle(NULL),
                               NULL);
   if (hwnd) {
      HDC hdc = GetDC(hwnd);

      // we must set a pixel format before we can create a context, just use the first one...
      SetPixelFormat(hdc, 1, NULL);
      HGLRC hglrc = wglCreateContext(hdc);
      wglMakeCurrent(hdc, hglrc);

      // call the test function
      proc(hdc, args);

      // clean up
      wglMakeCurrent(NULL, NULL);
      wglDeleteContext(hglrc);
      ReleaseDC(hwnd, hdc);
      DestroyWindow(hwnd);
   }
}

static void
windows_check_render_test(HDC hdc, void *args)
{
   int *result = (int *)args;

   /* Rather than play linkage games using stdcall to ensure we get
      glGetString from opengl32.dll here, use dlsym */
   void *dlhandle = windows_get_dl_handle();
   const char *(*proc)(int) = dlsym(dlhandle, "glGetString");
   const char *gl_renderer = (const char *)proc(GL_RENDERER);

   if ((!gl_renderer) || (strcasecmp(gl_renderer, "GDI Generic") == 0))
      *result = false;
   else
      *result = true;
}

int
windows_check_renderer(void)
{
   int result;
   windows_call_with_context(windows_check_render_test, &result);
   return result;
}

typedef struct {
   char *gl_extensions;
   char *wgl_extensions;
} windows_extensions_result;

static void
windows_extensions_test(HDC hdc, void *args)
{
   windows_extensions_result *r = (windows_extensions_result *)args;

   void *dlhandle = windows_get_dl_handle();
   const char *(*proc)(int) = dlsym(dlhandle, "glGetString");

   r->gl_extensions = strdup(proc(GL_EXTENSIONS));

   wglResolveExtensionProcs();
   r->wgl_extensions = strdup(wglGetExtensionsStringARB(hdc));
}

void
windows_extensions(char **gl_extensions, char **wgl_extensions)
{
   windows_extensions_result result;

   *gl_extensions = "";
   *wgl_extensions = "";

   windows_call_with_context(windows_extensions_test, &result);

   *gl_extensions = result.gl_extensions;
   *wgl_extensions = result.wgl_extensions;
}
