/**************************************************************************
 * 
 * Copyright 2010 VMware, Inc.
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

#include "pipe/p_defines.h"
#include "pipe/p_screen.h"

#include "util/u_debug.h"

#include "stw_device.h"
#include "stw_pixelformat.h"
#include "stw_framebuffer.h"


#define LARGE_WINDOW_SIZE 60000


static LRESULT CALLBACK
WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifndef _GAMING_XBOX
    MINMAXINFO *pMMI;
    switch (uMsg) {
    case WM_GETMINMAXINFO:
        // Allow to create a window bigger than the desktop
        pMMI = (MINMAXINFO *)lParam;
        pMMI->ptMaxSize.x = LARGE_WINDOW_SIZE;
        pMMI->ptMaxSize.y = LARGE_WINDOW_SIZE;
        pMMI->ptMaxTrackSize.x = LARGE_WINDOW_SIZE;
        pMMI->ptMaxTrackSize.y = LARGE_WINDOW_SIZE;
        break;
    default:
        break;
    }
#endif /* _GAMING_XBOX */

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

struct stw_framebuffer *
stw_pbuffer_create(const struct stw_pixelformat_info *pfi, int iWidth, int iHeight, struct pipe_frontend_screen *fscreen)
{
   static bool first = true;

   /*
    * Implement pbuffers through invisible windows
    */

   if (first) {
      WNDCLASS wc;
      memset(&wc, 0, sizeof wc);
      wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
#ifndef _GAMING_XBOX
      wc.hCursor = LoadCursor(NULL, IDC_ARROW);
      wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
#endif
      wc.lpfnWndProc = WndProc;
      wc.lpszClassName = "wglpbuffer";
      wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
      RegisterClass(&wc);
      first = false;
   }

   DWORD dwExStyle = 0;
   DWORD dwStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

   if (0) {
      /*
       * Don't hide the window -- useful for debugging what the application is
       * drawing
       */

      dwStyle |= WS_VISIBLE | WS_OVERLAPPEDWINDOW;
   } else {
      dwStyle |= WS_POPUPWINDOW;
   }

   RECT rect = { 0, 0, iWidth, iHeight };

   /*
    * The CreateWindowEx parameters are the total (outside) dimensions of the
    * window, which can vary with Windows version and user settings.  Use
    * AdjustWindowRect to get the required total area for the given client area.
    *
    * AdjustWindowRectEx does not accept WS_OVERLAPPED style (which is defined
    * as 0), which means we need to use some other style instead, e.g.,
    * WS_OVERLAPPEDWINDOW or WS_POPUPWINDOW as above.
    */

   AdjustWindowRectEx(&rect, dwStyle, false, dwExStyle);

   HWND hWnd = CreateWindowEx(dwExStyle,
                              "wglpbuffer", /* wc.lpszClassName */
                              NULL,
                              dwStyle,
                              CW_USEDEFAULT, /* x */
                              CW_USEDEFAULT, /* y */
                              rect.right - rect.left, /* width */
                              rect.bottom - rect.top, /* height */
                              NULL,
                              NULL,
                              NULL,
                              NULL);
   if (!hWnd) {
      return 0;
   }

#ifdef DEBUG
   /*
    * Verify the client area size matches the specified size.
    */

   GetClientRect(hWnd, &rect);
   assert(rect.left == 0);
   assert(rect.top == 0);
   assert(rect.right - rect.left == iWidth);
   assert(rect.bottom - rect.top == iHeight);
#endif

   return stw_framebuffer_create(hWnd, pfi, STW_FRAMEBUFFER_PBUFFER, fscreen);
}


HPBUFFERARB WINAPI
wglCreatePbufferARB(HDC hCurrentDC,
                    int iPixelFormat,
                    int iWidth,
                    int iHeight,
                    const int *piAttribList)
{
   const int *piAttrib;
   int useLargest = 0;
   struct stw_framebuffer *fb;
   HWND hWnd;
   int iDisplayablePixelFormat;
   PIXELFORMATDESCRIPTOR pfd;
   BOOL bRet;
   int textureFormat = WGL_NO_TEXTURE_ARB;
   int textureTarget = WGL_NO_TEXTURE_ARB;
   BOOL textureMipmap = false;
   const struct stw_pixelformat_info *pfi = stw_pixelformat_get_info(iPixelFormat);

   if (!pfi) {
      SetLastError(ERROR_INVALID_PIXEL_FORMAT);
      return 0;
   }

   if (iWidth <= 0 || iHeight <= 0) {
      SetLastError(ERROR_INVALID_DATA);
      return 0;
   }

   if (piAttribList) {
      for (piAttrib = piAttribList; *piAttrib; piAttrib++) {
         switch (*piAttrib) {
         case WGL_PBUFFER_LARGEST_ARB:
            piAttrib++;
            useLargest = *piAttrib;
            break;
          case WGL_TEXTURE_FORMAT_ARB:
             /* WGL_ARB_render_texture */
             piAttrib++;
             textureFormat = *piAttrib;
             if (textureFormat != WGL_TEXTURE_RGB_ARB &&
                textureFormat != WGL_TEXTURE_RGBA_ARB &&
                textureFormat != WGL_NO_TEXTURE_ARB) {
                SetLastError(ERROR_INVALID_DATA);
                return 0;
             }
             break;
          case WGL_TEXTURE_TARGET_ARB:
             /* WGL_ARB_render_texture */
             piAttrib++;
             textureTarget = *piAttrib;
             if (textureTarget != WGL_TEXTURE_CUBE_MAP_ARB &&
                 textureTarget != WGL_TEXTURE_1D_ARB &&
                 textureTarget != WGL_TEXTURE_2D_ARB &&
                 textureTarget != WGL_NO_TEXTURE_ARB) {
                SetLastError(ERROR_INVALID_DATA);
                return 0;
             }
             break;
         case WGL_MIPMAP_TEXTURE_ARB:
            /* WGL_ARB_render_texture */
            piAttrib++;
            textureMipmap = !!*piAttrib;
            break;
         default:
            SetLastError(ERROR_INVALID_DATA);
            debug_printf("wgl: Unsupported attribute 0x%x in %s\n",
                         *piAttrib, __func__);
            return 0;
         }
      }
   }

   if (iWidth > stw_dev->max_2d_length) {
      if (useLargest) {
         iWidth = stw_dev->max_2d_length;
      } else {
         SetLastError(ERROR_NO_SYSTEM_RESOURCES);
         return 0;
      }
   }

   if (iHeight > stw_dev->max_2d_length) {
      if (useLargest) {
         iHeight = stw_dev->max_2d_length;
      } else {
         SetLastError(ERROR_NO_SYSTEM_RESOURCES);
         return 0;
      }
   }

   /*
    * We can't pass non-displayable pixel formats to GDI, which is why we
    * create the framebuffer object before calling SetPixelFormat().
    */
   fb = stw_pbuffer_create(pfi, iWidth, iHeight, stw_dev->fscreen);
   if (!fb) {
      SetLastError(ERROR_NO_SYSTEM_RESOURCES);
      return NULL;
   }

   /* WGL_ARB_render_texture fields */
   fb->textureTarget = textureTarget;
   fb->textureFormat = textureFormat;
   fb->textureMipmap = textureMipmap;

   iDisplayablePixelFormat = fb->iDisplayablePixelFormat;
   hWnd = fb->hWnd;

   stw_framebuffer_unlock(fb);

   /*
    * We need to set a displayable pixel format on the hidden window DC
    * so that wglCreateContext and wglMakeCurrent are not overruled by GDI.
    */
   bRet = SetPixelFormat(GetDC(hWnd), iDisplayablePixelFormat, &pfd);
   assert(bRet);

   return (HPBUFFERARB)fb;
}


HDC WINAPI
wglGetPbufferDCARB(HPBUFFERARB hPbuffer)
{
   struct stw_framebuffer *fb;
   HDC hDC;

   if (!hPbuffer) {
      SetLastError(ERROR_INVALID_HANDLE);
      return NULL;
   }

   fb = stw_framebuffer_from_HPBUFFERARB(hPbuffer);

   hDC = GetDC(fb->hWnd);

   return hDC;
}


int WINAPI
wglReleasePbufferDCARB(HPBUFFERARB hPbuffer,
                       HDC hDC)
{
   struct stw_framebuffer *fb;

   if (!hPbuffer) {
      SetLastError(ERROR_INVALID_HANDLE);
      return 0;
   }

   fb = stw_framebuffer_from_HPBUFFERARB(hPbuffer);

   return ReleaseDC(fb->hWnd, hDC);
}


BOOL WINAPI
wglDestroyPbufferARB(HPBUFFERARB hPbuffer)
{
   struct stw_framebuffer *fb;

   if (!hPbuffer) {
      SetLastError(ERROR_INVALID_HANDLE);
      return false;
   }

   fb = stw_framebuffer_from_HPBUFFERARB(hPbuffer);

   /* This will destroy all our data */
   return DestroyWindow(fb->hWnd);
}


BOOL WINAPI
wglQueryPbufferARB(HPBUFFERARB hPbuffer,
                   int iAttribute,
                   int *piValue)
{
   struct stw_framebuffer *fb;

   if (!hPbuffer) {
      SetLastError(ERROR_INVALID_HANDLE);
      return false;
   }

   fb = stw_framebuffer_from_HPBUFFERARB(hPbuffer);

   switch (iAttribute) {
   case WGL_PBUFFER_WIDTH_ARB:
      *piValue = fb->width;
      return true;
   case WGL_PBUFFER_HEIGHT_ARB:
      *piValue = fb->height;
      return true;
   case WGL_PBUFFER_LOST_ARB:
      /* We assume that no content is ever lost due to display mode change */
      *piValue = false;
      return true;
   /* WGL_ARB_render_texture */
   case WGL_TEXTURE_TARGET_ARB:
      *piValue = fb->textureTarget;
      return true;
   case WGL_TEXTURE_FORMAT_ARB:
      *piValue = fb->textureFormat;
      return true;
   case WGL_MIPMAP_TEXTURE_ARB:
      *piValue = fb->textureMipmap;
      return true;
   case WGL_MIPMAP_LEVEL_ARB:
      *piValue = fb->textureLevel;
      return true;
   case WGL_CUBE_MAP_FACE_ARB:
      *piValue = fb->textureFace + WGL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB;
      return true;
   default:
      SetLastError(ERROR_INVALID_DATA);
      return false;
   }
}
