/*
 * Copyright © 2011 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <string.h>
#include <ctype.h>

#include "glxclient.h"
#include <xcb/glx.h>
#include <X11/Xlib-xcb.h>

void
glxSendClientInfo(struct glx_display *glx_dpy, int screen)
{
   const unsigned ext_length = strlen("GLX_ARB_create_context");
   const unsigned prof_length = strlen("_profile");
   char *gl_extension_string;
   int gl_extension_length;
   xcb_connection_t *c;
   Bool any_screen_has_ARB_create_context = False;
   Bool any_screen_has_ARB_create_context_profile = False;
   unsigned i;
   /* You need GLX_ARB_create_context_profile to get beyond 3.1 anyway */
   static const uint32_t gl_versions[] = {
      2, 1,
      3, 0,
      3, 1,
   };
   /*
    * This is weird, but it matches what NVIDIA does/expects. For big-GL
    * below 3.2 there is no such thing as a "profile", so we name them all
    * with no profile bits. Except we don't name anything lower than 2.1,
    * since GLX_ARB_create_context_profile says:
    *
    *   "Only the highest supported version below 3.0 should be sent, since
    *   OpenGL 2.1 is backwards compatible with all earlier versions."
    *
    * In order to also support GLES below 3.2, we name every possible GLES
    * version with the ES2 bit set, which happens to just mean GLES generally
    * and not a particular major version. 3.2 happens to be a legal version
    * number for both big-GL and GLES, so it gets all three bits set.
    * Everything 3.3 and above is big-GL only so gets the core and compat
    * bits set.
    */
   static const uint32_t gl_versions_profiles[] = {
      1, 0, GLX_CONTEXT_ES2_PROFILE_BIT_EXT,
      1, 1, GLX_CONTEXT_ES2_PROFILE_BIT_EXT,
      2, 0, GLX_CONTEXT_ES2_PROFILE_BIT_EXT,
      2, 1, 0x0,
      3, 0, 0x0,
      3, 0, GLX_CONTEXT_ES2_PROFILE_BIT_EXT,
      3, 1, 0x0,
      3, 1, GLX_CONTEXT_ES2_PROFILE_BIT_EXT,
      3, 2, GLX_CONTEXT_CORE_PROFILE_BIT_ARB |
            GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB |
            GLX_CONTEXT_ES2_PROFILE_BIT_EXT,
      3, 3, GLX_CONTEXT_CORE_PROFILE_BIT_ARB |
            GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
      4, 0, GLX_CONTEXT_CORE_PROFILE_BIT_ARB |
            GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
      4, 1, GLX_CONTEXT_CORE_PROFILE_BIT_ARB |
            GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
      4, 2, GLX_CONTEXT_CORE_PROFILE_BIT_ARB |
            GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
      4, 3, GLX_CONTEXT_CORE_PROFILE_BIT_ARB |
            GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
      4, 4, GLX_CONTEXT_CORE_PROFILE_BIT_ARB |
            GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
      4, 5, GLX_CONTEXT_CORE_PROFILE_BIT_ARB |
            GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
      4, 6, GLX_CONTEXT_CORE_PROFILE_BIT_ARB |
            GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
   };
   static const char glx_extensions[] =
      "GLX_ARB_create_context GLX_ARB_create_context_profile";

   /* There are three possible flavors of the client info structure that the
    * client could send to the server.  The version sent depends on the
    * combination of GLX versions and extensions supported by the client and
    * the server. This client only supports GLX version >= 1.3.
    *
    * Server supports                  Client sends
    * ----------------------------------------------------------------------
    * GLX version = 1.0                Nothing.
    *
    * GLX version >= 1.1               struct GLXClientInfo
    *
    * GLX version >= 1.4 and
    * GLX_ARB_create_context           struct glXSetClientInfoARB
    *
    * GLX version >= 1.4 and
    * GLX_ARB_create_context_profile   struct glXSetClientInfo2ARB
    *
    * GLX_ARB_create_context and GLX_ARB_create_context_profile use FBConfigs,
    * and these only exist in GLX 1.4 or with GLX_SGIX_fbconfig.  I can't
    * imagine an implementation that supports GLX_SGIX_fbconfig and
    * GLX_ARB_create_context but not GLX 1.4.  Making GLX 1.4 a hard
    * requirement in this case does not seem like a limitation.
    */

   /* Determine whether any screen on the server supports either of the
    * create-context extensions.
    */
   for (i = 0; i < ScreenCount(glx_dpy->dpy); i++) {
      struct glx_screen *src = glx_dpy->screens[i];

      const char *haystack = src->serverGLXexts;
      while (haystack != NULL) {
	 char *match = strstr(haystack, "GLX_ARB_create_context");

	 if (match == NULL)
	    break;

	 match += ext_length;

	 switch (match[0]) {
	 case '\0':
	 case ' ':
	    any_screen_has_ARB_create_context = True;
	    break;

	 case '_':
	    if (strncmp(match, "_profile", prof_length) == 0
		    && (match[prof_length] == '\0'
			|| match[prof_length] == ' ')) {
	       any_screen_has_ARB_create_context_profile = True;
	       match += prof_length;
	    }
	    break;
	 }

	 haystack = match;
      }
   }

   gl_extension_string = __glXGetClientGLExtensionString(screen);
   gl_extension_length = strlen(gl_extension_string) + 1;

   c = XGetXCBConnection(glx_dpy->dpy);

   /* Depending on the GLX version and the available extensions on the server,
    * send the correct "flavor" of protocol to the server.
    *
    * THE ORDER IS IMPORTANT.  We want to send the most recent version of the
    * protocol that the server can support.
    */
   if (glx_dpy->minorVersion == 4
       && any_screen_has_ARB_create_context_profile) {
      xcb_glx_set_client_info_2arb(c,
				  GLX_MAJOR_VERSION, GLX_MINOR_VERSION,
				   sizeof(gl_versions_profiles)
				   / (3 * sizeof(gl_versions_profiles[0])),
				  gl_extension_length,
				  strlen(glx_extensions) + 1,
				  gl_versions_profiles,
				  gl_extension_string,
				  glx_extensions);
   } else if (glx_dpy->minorVersion == 4
	      && any_screen_has_ARB_create_context) {
      xcb_glx_set_client_info_arb(c,
				  GLX_MAJOR_VERSION, GLX_MINOR_VERSION,
				  sizeof(gl_versions)
				  / (2 * sizeof(gl_versions[0])),
				  gl_extension_length,
				  strlen(glx_extensions) + 1,
				  gl_versions,
				  gl_extension_string,
				  glx_extensions);
   } else {
      xcb_glx_client_info(c,
			  GLX_MAJOR_VERSION, GLX_MINOR_VERSION,
			  gl_extension_length,
			  gl_extension_string);
   }

   free(gl_extension_string);
}
