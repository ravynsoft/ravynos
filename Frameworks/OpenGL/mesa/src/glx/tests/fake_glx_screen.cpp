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
#include "fake_glx_screen.h"

struct glx_screen_vtable fake_glx_screen::vt = {
   indirect_create_context,
   indirect_create_context_attribs,
   NULL,
   NULL,
};

struct glx_screen_vtable fake_glx_screen_direct::vt = {
   fake_glx_context_direct::create,
   fake_glx_context_direct::create_attribs,
   NULL,
   NULL,
};

const struct glx_context_vtable fake_glx_context::vt = {
   fake_glx_context::destroy,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
};

int fake_glx_context::contexts_allocated = 0;

extern "C" struct glx_context *
indirect_create_context(struct glx_screen *psc, struct glx_config *mode,
			struct glx_context *shareList, int renderType)
{
   (void) shareList;
   (void) renderType;

   return new fake_glx_context(psc, mode);
}

extern "C" struct glx_context *
indirect_create_context_attribs(struct glx_screen *base,
				struct glx_config *config_base,
				struct glx_context *shareList,
				unsigned num_attribs,
				const uint32_t *attribs,
				unsigned *error)
{
   (void) num_attribs;
   (void) attribs;
   (void) error;

   return indirect_create_context(base, config_base, shareList, 0);
}

#ifdef GLX_USE_APPLEGL
#warning Indirect GLX tests are not built
extern "C" struct glx_context *
applegl_create_context(struct glx_screen *base,
		       struct glx_config *config_base,
		       struct glx_context *shareList,
		       int renderType)
{
   return indirect_create_context(base, config_base, shareList, renderType);
}
#endif

/* This is necessary so that we don't have to link with glxcurrent.c
 * which would require us to link with X libraries and what not.
 */
GLubyte dummyBuffer[__GLX_BUFFER_LIMIT_SIZE];
struct glx_context_vtable dummyVtable;
struct glx_context dummyContext = {
   &dummyBuffer[0],
   &dummyBuffer[0],
   &dummyBuffer[0],
   &dummyBuffer[__GLX_BUFFER_LIMIT_SIZE],
   sizeof(dummyBuffer),
   &dummyVtable
};
__THREAD_INITIAL_EXEC void *__glX_tls_Context = &dummyContext;
