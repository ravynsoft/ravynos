/*
 * (C) Copyright IBM Corporation 2004
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file glx_query.c
 * Generic utility functions to query internal data from the server.
 *
 * \author Ian Romanick <idr@us.ibm.com>
 */

#include "glxclient.h"

# include <X11/Xlib-xcb.h>
# include <xcb/xcb.h>
# include <xcb/glx.h>


/**
 * Exchange a protocol request for glXQueryServerString.
 */
char *
__glXQueryServerString(Display * dpy, CARD32 screen, CARD32 name)
{
   xcb_connection_t *c = XGetXCBConnection(dpy);
   xcb_glx_query_server_string_reply_t *reply =
      xcb_glx_query_server_string_reply(c,
                                        xcb_glx_query_server_string(c,
                                                                    screen,
                                                                    name),
                                        NULL);

   if (!reply)
      return NULL;

   /* The spec doesn't mention this, but the Xorg server replies with
    * a string already terminated with '\0'. */
   uint32_t len = xcb_glx_query_server_string_string_length(reply);
   char *buf = malloc(len);
   memcpy(buf, xcb_glx_query_server_string_string(reply), len);
   free(reply);

   return buf;
}

/**
 * Exchange a protocol request for glGetString.
 */
char *
__glXGetString(Display * dpy, CARD32 contextTag, CARD32 name)
{
   xcb_connection_t *c = XGetXCBConnection(dpy);
   xcb_glx_get_string_reply_t *reply = xcb_glx_get_string_reply(c,
                                                                xcb_glx_get_string
                                                                (c,
                                                                 contextTag,
                                                                 name),
                                                                NULL);

   if (!reply)
      return NULL;

   /* The spec doesn't mention this, but the Xorg server replies with
    * a string already terminated with '\0'. */
   uint32_t len = xcb_glx_get_string_string_length(reply);
   char *buf = malloc(len);
   memcpy(buf, xcb_glx_get_string_string(reply), len);
   free(reply);

   return buf;
}

