/*
 * Copyright © 2013 Intel Corporation
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

#if defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL)

#include "glxclient.h"
#include "glx_error.h"
#include "GL/internal/dri_interface.h"
#include "dri2_priv.h"
#if defined(HAVE_DRI3)
#include "dri3_priv.h"
#endif
#include "drisw_priv.h"

#define __RENDERER(attrib) \
    { GLX_RENDERER_##attrib##_MESA, __DRI2_RENDERER_##attrib }

static const struct {
   unsigned int glx_attrib, dri2_attrib;
} query_renderer_map[] = {
  __RENDERER(VENDOR_ID),
  __RENDERER(DEVICE_ID),
  __RENDERER(VERSION),
  __RENDERER(ACCELERATED),
  __RENDERER(VIDEO_MEMORY),
  __RENDERER(UNIFIED_MEMORY_ARCHITECTURE),
  __RENDERER(PREFERRED_PROFILE),
  __RENDERER(OPENGL_CORE_PROFILE_VERSION),
  __RENDERER(OPENGL_COMPATIBILITY_PROFILE_VERSION),
  __RENDERER(OPENGL_ES_PROFILE_VERSION),
  __RENDERER(OPENGL_ES2_PROFILE_VERSION),
};

#undef __RENDERER

static int
dri2_convert_glx_query_renderer_attribs(int attribute)
{
   unsigned i;

   for (i = 0; i < ARRAY_SIZE(query_renderer_map); i++)
      if (query_renderer_map[i].glx_attrib == attribute)
         return query_renderer_map[i].dri2_attrib;

   return -1;
}

/* Convert internal dri context profile bits into GLX context profile bits */
static inline void
dri_convert_context_profile_bits(int attribute, unsigned int *value)
{
   if (attribute == GLX_RENDERER_PREFERRED_PROFILE_MESA) {
      if (value[0] == (1U << __DRI_API_OPENGL_CORE))
         value[0] = GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
      else if (value[0] == (1U << __DRI_API_OPENGL))
         value[0] = GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
   }
}

_X_HIDDEN int
dri2_query_renderer_integer(struct glx_screen *base, int attribute,
                            unsigned int *value)
{
   int ret;
   struct dri2_screen *const psc = (struct dri2_screen *) base;

   /* Even though there are invalid values (and
    * dri2_convert_glx_query_renderer_attribs may return -1), the higher level
    * GLX code is required to perform the filtering.  Assume that we got a
    * good value.
    */
   const int dri_attribute = dri2_convert_glx_query_renderer_attribs(attribute);

   if (psc->rendererQuery == NULL)
      return -1;

   ret = psc->rendererQuery->queryInteger(psc->driScreen, dri_attribute,
                                          value);
   dri_convert_context_profile_bits(attribute, value);

   return ret;
}

_X_HIDDEN int
dri2_query_renderer_string(struct glx_screen *base, int attribute,
                           const char **value)
{
   struct dri2_screen *const psc = (struct dri2_screen *) base;

   /* Even though queryString only accepts a subset of the possible GLX
    * queries, the higher level GLX code is required to perform the filtering.
    * Assume that we got a good value.
    */
   const int dri_attribute = dri2_convert_glx_query_renderer_attribs(attribute);

   if (psc->rendererQuery == NULL)
      return -1;

   return psc->rendererQuery->queryString(psc->driScreen, dri_attribute, value);
}

#if defined(HAVE_DRI3)
_X_HIDDEN int
dri3_query_renderer_integer(struct glx_screen *base, int attribute,
                            unsigned int *value)
{
   int ret;
   struct dri3_screen *const psc = (struct dri3_screen *) base;

   /* Even though there are invalid values (and
    * dri2_convert_glx_query_renderer_attribs may return -1), the higher level
    * GLX code is required to perform the filtering.  Assume that we got a
    * good value.
    */
   const int dri_attribute = dri2_convert_glx_query_renderer_attribs(attribute);

   if (psc->rendererQuery == NULL)
      return -1;

   ret = psc->rendererQuery->queryInteger(psc->driScreenRenderGPU, dri_attribute,
                                          value);
   dri_convert_context_profile_bits(attribute, value);

   return ret;
}

_X_HIDDEN int
dri3_query_renderer_string(struct glx_screen *base, int attribute,
                           const char **value)
{
   struct dri3_screen *const psc = (struct dri3_screen *) base;

   /* Even though queryString only accepts a subset of the possible GLX
    * queries, the higher level GLX code is required to perform the filtering.
    * Assume that we got a good value.
    */
   const int dri_attribute = dri2_convert_glx_query_renderer_attribs(attribute);

   if (psc->rendererQuery == NULL)
      return -1;

   return psc->rendererQuery->queryString(psc->driScreenRenderGPU, dri_attribute, value);
}
#endif /* HAVE_DRI3 */

_X_HIDDEN int
drisw_query_renderer_integer(struct glx_screen *base, int attribute,
                             unsigned int *value)
{
   int ret;
   struct drisw_screen *const psc = (struct drisw_screen *) base;

   /* Even though there are invalid values (and
    * dri2_convert_glx_query_renderer_attribs may return -1), the higher level
    * GLX code is required to perform the filtering.  Assume that we got a
    * good value.
    */
   const int dri_attribute = dri2_convert_glx_query_renderer_attribs(attribute);

   if (psc->rendererQuery == NULL)
      return -1;

   ret = psc->rendererQuery->queryInteger(psc->driScreen, dri_attribute,
                                          value);
   dri_convert_context_profile_bits(attribute, value);

   return ret;
}

_X_HIDDEN int
drisw_query_renderer_string(struct glx_screen *base, int attribute,
                            const char **value)
{
   struct drisw_screen *const psc = (struct drisw_screen *) base;

   /* Even though queryString only accepts a subset of the possible GLX
    * queries, the higher level GLX code is required to perform the filtering.
    * Assume that we got a good value.
    */
   const int dri_attribute = dri2_convert_glx_query_renderer_attribs(attribute);

   if (psc->rendererQuery == NULL)
      return -1;

   return psc->rendererQuery->queryString(psc->driScreen, dri_attribute, value);
}


#endif /* GLX_DIRECT_RENDERING */
