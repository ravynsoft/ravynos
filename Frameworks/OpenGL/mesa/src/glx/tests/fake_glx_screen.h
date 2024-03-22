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
#include "glxclient.h"

class fake_glx_screen : public glx_screen {
public:
   fake_glx_screen(struct glx_display *glx_dpy, int num, const char *ext)
   {
      this->vtable = &fake_glx_screen::vt;
      this->serverGLXexts = 0;
      this->effectiveGLXexts = 0;
      this->display = 0;
      this->dpy = 0;
      this->scr = num;
      this->visuals = 0;
      this->configs = 0;
      this->force_direct_context = false;
      this->allow_invalid_glx_destroy_window = false;

      this->display = glx_dpy;
      this->dpy = (glx_dpy != NULL) ? glx_dpy->dpy : NULL;

      this->serverGLXexts = new char[strlen(ext) + 1];
      strcpy((char *) this->serverGLXexts, ext);
   }

   ~fake_glx_screen()
   {
      delete [] this->serverGLXexts;
   }

private:
   static struct glx_screen_vtable vt;
};

class fake_glx_screen_direct : public fake_glx_screen {
public:
   fake_glx_screen_direct(struct glx_display *glx_dpy, int num,
			  const char *ext)
      : fake_glx_screen(glx_dpy, num, ext)
   {
      this->vtable = &fake_glx_screen_direct::vt;
   }

private:
   static struct glx_screen_vtable vt;
};

class fake_glx_context : public glx_context {
public:
   fake_glx_context(struct glx_screen *psc, struct glx_config *mode)
   {
      contexts_allocated++;

      this->vtable = &fake_glx_context::vt;
      this->majorOpcode = 123;
      this->psc = psc;
      this->config = mode;
      this->isDirect = false;
      this->currentContextTag = -1;

      this->client_state_private = (struct __GLXattributeRec *) 0xcafebabe;
   }

   ~fake_glx_context()
   {
      contexts_allocated--;
   }

   /** Number of context that are allocated (and not freed). */
   static int contexts_allocated;

private:
   static const struct glx_context_vtable vt;

   static void destroy(struct glx_context *gc)
   {
      delete gc;
   }
};

class fake_glx_context_direct : public fake_glx_context {
public:
   fake_glx_context_direct(struct glx_screen *psc, struct glx_config *mode)
      : fake_glx_context(psc, mode)
   {
      this->isDirect = True;
   }

   static glx_context *create(struct glx_screen *psc, struct glx_config *mode,
			      struct glx_context *shareList, int renderType)
   {
      (void) shareList;
      (void) renderType;

      return new fake_glx_context_direct(psc, mode);
   }

   static glx_context *create_attribs(struct glx_screen *psc,
				      struct glx_config *mode,
				      struct glx_context *shareList,
				      unsigned num_attribs,
				      const uint32_t *attribs,
				      unsigned *error)
   {
      (void) shareList;
      (void) num_attribs;
      (void) attribs;

      *error = 0;
      return new fake_glx_context_direct(psc, mode);
   }
};
