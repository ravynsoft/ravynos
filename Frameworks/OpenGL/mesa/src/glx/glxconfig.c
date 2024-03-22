/*
 * (C) Copyright IBM Corporation 2003
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
 * VA LINUX SYSTEM, IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * \file glxconfig.c
 * Utility routines for working with \c struct glx_config structures.  At
 * some point most or all of these functions will be moved to the Mesa
 * code base.
 *
 * \author Ian Romanick <idr@us.ibm.com>
 */

#include <GL/glx.h>
#include <stdlib.h>
#include <string.h>

#include "glxconfig.h"

#define NUM_VISUAL_TYPES   6

/**
 * Get data from a GLX config
 * 
 * \param mode         GL context mode whose data is to be returned.
 * \param attribute    Attribute of \c mode that is to be returned.
 * \param value_return Location to store the data member of \c mode.
 * \return  If \c attribute is a valid attribute of \c mode, zero is
 *          returned.  Otherwise \c GLX_BAD_ATTRIBUTE is returned.
 */
_X_HIDDEN int
glx_config_get(struct glx_config * mode, int attribute, int *value_return)
{
   switch (attribute) {
   case GLX_USE_GL:
      *value_return = GL_TRUE;
      return 0;
   case GLX_BUFFER_SIZE:
      *value_return = mode->rgbBits;
      return 0;
   case GLX_RGBA:
      *value_return = !(mode->renderType & GLX_COLOR_INDEX_BIT);
      return 0;
   case GLX_RED_SIZE:
      *value_return = mode->redBits;
      return 0;
   case GLX_GREEN_SIZE:
      *value_return = mode->greenBits;
      return 0;
   case GLX_BLUE_SIZE:
      *value_return = mode->blueBits;
      return 0;
   case GLX_ALPHA_SIZE:
      *value_return = mode->alphaBits;
      return 0;
   case GLX_DOUBLEBUFFER:
      *value_return = mode->doubleBufferMode;
      return 0;
   case GLX_STEREO:
      *value_return = mode->stereoMode;
      return 0;
   case GLX_AUX_BUFFERS:
      *value_return = mode->numAuxBuffers;
      return 0;
   case GLX_DEPTH_SIZE:
      *value_return = mode->depthBits;
      return 0;
   case GLX_STENCIL_SIZE:
      *value_return = mode->stencilBits;
      return 0;
   case GLX_ACCUM_RED_SIZE:
      *value_return = mode->accumRedBits;
      return 0;
   case GLX_ACCUM_GREEN_SIZE:
      *value_return = mode->accumGreenBits;
      return 0;
   case GLX_ACCUM_BLUE_SIZE:
      *value_return = mode->accumBlueBits;
      return 0;
   case GLX_ACCUM_ALPHA_SIZE:
      *value_return = mode->accumAlphaBits;
      return 0;
   case GLX_LEVEL:
      *value_return = mode->level;
      return 0;
#ifndef GLX_USE_APPLEGL               /* This isn't supported by CGL. */
   case GLX_TRANSPARENT_TYPE_EXT:
      *value_return = mode->transparentPixel;
      return 0;
#endif
   case GLX_TRANSPARENT_RED_VALUE:
      *value_return = mode->transparentRed;
      return 0;
   case GLX_TRANSPARENT_GREEN_VALUE:
      *value_return = mode->transparentGreen;
      return 0;
   case GLX_TRANSPARENT_BLUE_VALUE:
      *value_return = mode->transparentBlue;
      return 0;
   case GLX_TRANSPARENT_ALPHA_VALUE:
      *value_return = mode->transparentAlpha;
      return 0;
   case GLX_TRANSPARENT_INDEX_VALUE:
      *value_return = mode->transparentIndex;
      return 0;
   case GLX_X_VISUAL_TYPE:
      *value_return = mode->visualType;
      return 0;
   case GLX_CONFIG_CAVEAT:
      *value_return = mode->visualRating;
      return 0;
   case GLX_VISUAL_ID:
      *value_return = mode->visualID;
      return 0;
   case GLX_DRAWABLE_TYPE:
      *value_return = mode->drawableType;
      return 0;
   case GLX_RENDER_TYPE:
      *value_return = mode->renderType;
      return 0;
   case GLX_X_RENDERABLE:
      *value_return = mode->xRenderable;
      return 0;
   case GLX_FBCONFIG_ID:
      *value_return = mode->fbconfigID;
      return 0;
   case GLX_MAX_PBUFFER_WIDTH:
      *value_return = 4096; /* _EGL_MAX_PBUFFER_WIDTH */
      return 0;
   case GLX_MAX_PBUFFER_HEIGHT:
      *value_return = 4096; /* _EGL_MAX_PBUFFER_HEIGHT */
      return 0;
   case GLX_MAX_PBUFFER_PIXELS:
      *value_return = mode->maxPbufferPixels;
      return 0;
#ifndef GLX_USE_APPLEGL               /* These aren't supported by CGL. */
   case GLX_OPTIMAL_PBUFFER_WIDTH_SGIX:
      *value_return = mode->optimalPbufferWidth;
      return 0;
   case GLX_OPTIMAL_PBUFFER_HEIGHT_SGIX:
      *value_return = mode->optimalPbufferHeight;
      return 0;
#endif
   case GLX_SAMPLE_BUFFERS_SGIS:
      *value_return = mode->sampleBuffers;
      return 0;
   case GLX_SAMPLES_SGIS:
      *value_return = mode->samples;
      return 0;
   case GLX_BIND_TO_TEXTURE_RGB_EXT:
      *value_return = mode->bindToTextureRgb;
      return 0;
   case GLX_BIND_TO_TEXTURE_RGBA_EXT:
      *value_return = mode->bindToTextureRgba;
      return 0;
   case GLX_BIND_TO_MIPMAP_TEXTURE_EXT:
      *value_return = mode->bindToMipmapTexture == GL_TRUE ? GL_TRUE :
         GL_FALSE;
      return 0;
   case GLX_BIND_TO_TEXTURE_TARGETS_EXT:
      *value_return = mode->bindToTextureTargets;
      return 0;
   case GLX_Y_INVERTED_EXT:
      *value_return = mode->yInverted;
      return 0;
   case GLX_FRAMEBUFFER_SRGB_CAPABLE_EXT:
      *value_return = mode->sRGBCapable;
      return 0;
   case GLX_FLOAT_COMPONENTS_NV:
      *value_return = mode->floatComponentsNV;
      return 0;

      /* Applications are NOT allowed to query GLX_VISUAL_SELECT_GROUP_SGIX.
       * It is ONLY for communication between the GLX client and the GLX
       * server.
       */
   case GLX_VISUAL_SELECT_GROUP_SGIX:
   default:
      return GLX_BAD_ATTRIBUTE;
   }
}


/**
 * Allocate a linked list of \c struct glx_config structures.  The fields of
 * each structure will be initialized to "reasonable" default values.  In
 * most cases this is the default value defined by table 3.4 of the GLX
 * 1.3 specification.  This means that most values are either initialized to
 * zero or \c GLX_DONT_CARE (which is -1).  As support for additional
 * extensions is added, the new values will be initialized to appropriate
 * values from the extension specification.
 * 
 * \param count         Number of structures to allocate.
 * \returns A pointer to the first element in a linked list of \c count
 *          structures on success, or \c NULL on failure.
 */
_X_HIDDEN struct glx_config *
glx_config_create_list(unsigned count)
{
   const size_t size = sizeof(struct glx_config);
   struct glx_config *base = NULL;
   struct glx_config **next;
   unsigned i;

   next = &base;
   for (i = 0; i < count; i++) {
      *next = calloc(1, size);
      if (*next == NULL) {
	 glx_config_destroy_list(base);
	 base = NULL;
	 break;
      }

      (*next)->visualID = GLX_DONT_CARE;
      (*next)->visualType = GLX_DONT_CARE;
      (*next)->visualRating = GLX_NONE;
      (*next)->transparentPixel = GLX_NONE;
      (*next)->transparentRed = GLX_DONT_CARE;
      (*next)->transparentGreen = GLX_DONT_CARE;
      (*next)->transparentBlue = GLX_DONT_CARE;
      (*next)->transparentAlpha = GLX_DONT_CARE;
      (*next)->transparentIndex = GLX_DONT_CARE;
      (*next)->xRenderable = GLX_DONT_CARE;
      (*next)->fbconfigID = GLX_DONT_CARE;
      (*next)->bindToTextureRgb = GLX_DONT_CARE;
      (*next)->bindToTextureRgba = GLX_DONT_CARE;
      (*next)->bindToMipmapTexture = GLX_DONT_CARE;
      (*next)->bindToTextureTargets = GLX_DONT_CARE;
      (*next)->yInverted = GLX_DONT_CARE;
      (*next)->sRGBCapable = GL_FALSE;

      next = &((*next)->next);
   }

   return base;
}

_X_HIDDEN void
glx_config_destroy_list(struct glx_config *configs)
{
   while (configs != NULL) {
      struct glx_config *const next = configs->next;

      free(configs);
      configs = next;
   }
}


/**
 * Find a context mode matching a Visual ID.
 *
 * \param modes  List list of context-mode structures to be searched.
 * \param vid    Visual ID to be found.
 * \returns A pointer to a context-mode in \c modes if \c vid was found in
 *          the list, or \c NULL if it was not.
 */

_X_HIDDEN struct glx_config *
glx_config_find_visual(struct glx_config *configs, int vid)
{
   struct glx_config *c;

   for (c = configs; c != NULL; c = c->next)
      if (c->visualID == vid)
	 return c;

   return NULL;
}

_X_HIDDEN struct glx_config *
glx_config_find_fbconfig(struct glx_config *configs, int fbid)
{
   struct glx_config *c;

   for (c = configs; c != NULL; c = c->next)
      if (c->fbconfigID == fbid)
	 return c;

   return NULL;
}
