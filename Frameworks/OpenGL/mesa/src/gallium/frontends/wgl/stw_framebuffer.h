/**************************************************************************
 * 
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

#ifndef STW_FRAMEBUFFER_H
#define STW_FRAMEBUFFER_H

#include <windows.h>

#include <GL/gl.h>
#include <GL/wglext.h>

#include "util/u_debug.h"
#include "stw_st.h"


struct pipe_resource;
struct pipe_frontend_drawable;
struct stw_pixelformat_info;
struct pipe_frontend_screen;

enum stw_framebuffer_owner
{
   /* WGL window framebuffers have no corresponding destroy, and therefore
    * a window hook is needed to clean them up.
    */
   STW_FRAMEBUFFER_WGL_WINDOW,
   /* PBuffers behave like WGL window framebuffers, except that the window
    * lifetime is managed by us. We can explicitly clean up the window.
    */
   STW_FRAMEBUFFER_PBUFFER,
   /* EGL window framebuffers do have a corresponding destroy, so they don't
    * need to be registered in the global framebuffer list. This means they
    * will only be cleaned up from a destroy, and don't need to live until the
    * window goes away.
    */
   STW_FRAMEBUFFER_EGL_WINDOW,
};

/**
 * Windows framebuffer.
 */
struct stw_framebuffer
{
   /**
    * This mutex has two purposes:
    * - protect the access to the mutable data members below
    * - prevent the framebuffer from being deleted while being accessed.
    *
    * Note: if both this mutex and the stw_device::fb_mutex need to be locked,
    * the stw_device::fb_mutex needs to be locked first.
    */
   CRITICAL_SECTION mutex;
   
   /*
    * Immutable members.
    * 
    * Note that even access to immutable members implies acquiring the mutex 
    * above, to prevent the framebuffer from being destroyed.
    */
   
   HWND hWnd;

   const struct stw_pixelformat_info *pfi;

   /* A pixel format that can be used by GDI */
   int iDisplayablePixelFormat;
   enum stw_framebuffer_owner owner;

   struct pipe_frontend_drawable *drawable;

   /*
    * Mutable members. 
    */

   unsigned refcnt;

   
   /* FIXME: Make this work for multiple contexts bound to the same framebuffer */
   bool must_resize;

   bool minimized;  /**< Is the window currently minimized? */

   unsigned width;
   unsigned height;
   
   /** WGL_ARB_render_texture - set at Pbuffer creation time */
   unsigned textureFormat;  /**< WGL_NO_TEXTURE or WGL_TEXTURE_RGB[A]_ARB */
   unsigned textureTarget;  /**< WGL_NO_TEXTURE or WGL_TEXTURE_1D/2D/
                                 CUBE_MAP_ARB */
   bool textureMipmap;   /**< TRUE/FALSE */
   /** WGL_ARB_render_texture - set with wglSetPbufferAttribARB() */
   unsigned textureLevel;
   unsigned textureFace;    /**< [0..6] */

   /**
    * Client area rectangle, relative to the window upper-left corner.
    *
    * @sa GLCBPRESENTBUFFERSDATA::rect.
    */
   RECT client_rect;

   HANDLE hSharedSurface;
   struct stw_shared_surface *shared_surface;

   struct stw_winsys_framebuffer *winsys_framebuffer;

   /* For WGL_EXT_swap_control */
   int swap_interval;
   int64_t prev_swap_time;

#ifdef _GAMING_XBOX
   /* For the WndProc hook chain */
   WNDPROC prev_wndproc;
#endif

   /** 
    * This is protected by stw_device::fb_mutex, not the mutex above.
    * 
    * Deletions must be done by first acquiring stw_device::fb_mutex, and then
    * acquiring the stw_framebuffer::mutex of the framebuffer to be deleted. 
    * This ensures that nobody else is reading/writing to the.
    * 
    * It is not necessary to acquire the mutex above to navigate the linked list
    * given that deletions are done with stw_device::fb_mutex held, so no other
    * thread can delete.
    */
   struct stw_framebuffer *next;
};

/**
 * Create a new framebuffer object which will correspond to the given HDC.
 * 
 * This function will acquire stw_framebuffer::mutex. stw_framebuffer_unlock
 * must be called when done 
 */
struct stw_framebuffer *
stw_framebuffer_create(HWND hwnd, const struct stw_pixelformat_info *pfi, enum stw_framebuffer_owner owner,
                       struct pipe_frontend_screen *fscreen);

struct stw_framebuffer *
stw_pbuffer_create(const struct stw_pixelformat_info *pfi, int iWidth, int iHeight, struct pipe_frontend_screen *fscreen);


/**
 * Increase fb reference count.  The referenced framebuffer should be locked.
 *
 * It's not necessary to hold stw_dev::fb_mutex global lock.
 */
void
stw_framebuffer_reference_locked(struct stw_framebuffer *fb);


void
stw_framebuffer_release_locked(struct stw_framebuffer *fb,
                               struct st_context *st);

/**
 * Search a framebuffer with a matching HWND.
 * 
 * This function will acquire stw_framebuffer::mutex. stw_framebuffer_unlock
 * must be called when done 
 */
struct stw_framebuffer *
stw_framebuffer_from_hwnd(HWND hwnd);

/**
 * Search a framebuffer with a matching HDC.
 * 
 * This function will acquire stw_framebuffer::mutex. stw_framebuffer_unlock
 * must be called when done 
 */
struct stw_framebuffer *
stw_framebuffer_from_hdc(HDC hdc);

BOOL
stw_framebuffer_present_locked(HDC hdc,
                               struct stw_framebuffer *fb,
                               struct pipe_resource *res);

void
stw_framebuffer_update(struct stw_framebuffer *fb);

BOOL
stw_framebuffer_swap_locked(HDC hdc, struct stw_framebuffer *fb);


static inline void
stw_framebuffer_lock(struct stw_framebuffer *fb)
{
   assert(fb);
   EnterCriticalSection(&fb->mutex);
}


/**
 * Release stw_framebuffer::mutex lock. This framebuffer must not be accessed
 * after calling this function, as it may have been deleted by another thread
 * in the meanwhile.
 */
void
stw_framebuffer_unlock(struct stw_framebuffer *fb);


/**
 * Cleanup any existing framebuffers when exiting application.
 */
void
stw_framebuffer_cleanup(void);


static inline struct stw_st_framebuffer *
stw_st_framebuffer(struct pipe_frontend_drawable *drawable)
{
   return (struct stw_st_framebuffer *) drawable;
}


static inline struct stw_framebuffer *
stw_framebuffer_from_HPBUFFERARB(HPBUFFERARB hPbuffer)
{
   return (struct stw_framebuffer *) hPbuffer;
}


#endif /* STW_FRAMEBUFFER_H */
