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

#ifndef STW_CONTEXT_H
#define STW_CONTEXT_H

#include <windows.h>
#include <GL/gl.h>
#include <stw_gdishim.h>
#include <gldrv.h>

struct hud_context;
struct stw_framebuffer;
struct st_context;
struct pipe_frontend_screen;

struct stw_context
{
   struct st_context *st;
   DHGLRC dhglrc;
   const struct stw_pixelformat_info *pfi;
   HDC hDrawDC;
   HDC hReadDC;
   BOOL shared;

   struct stw_framebuffer *current_framebuffer;
   struct stw_framebuffer *current_read_framebuffer;

   struct hud_context *hud;
};

struct stw_context *stw_create_context_attribs(HDC hdc, INT iLayerPlane,
                                               struct stw_context *shareCtx,
                                               struct pipe_frontend_screen *fscreen,
                                               int majorVersion, int minorVersion,
                                               int contextFlags, int profileMask,
                                               const struct stw_pixelformat_info *pfi,
                                               int resetStrategy);

DHGLRC stw_create_context_handle(struct stw_context *context, DHGLRC handle);

void stw_destroy_context(struct stw_context *);

BOOL stw_unbind_context(struct stw_context *);

DHGLRC stw_get_current_context( void );

struct stw_context *stw_current_context(void);

HDC stw_get_current_dc( void );

HDC stw_get_current_read_dc( void );

BOOL stw_make_current( struct stw_framebuffer *fb, struct stw_framebuffer *fbRead, struct stw_context *ctx );

BOOL stw_make_current_by_handles( HDC hDrawDC, HDC hReadDC, DHGLRC dhglrc );

void stw_notify_current_locked( struct stw_framebuffer *fb );

#endif /* STW_CONTEXT_H */
