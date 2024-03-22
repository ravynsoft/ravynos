/*
 * SGI FREE SOFTWARE LICENSE B (Version 2.0, Sept. 18, 2008)
 * Copyright (C) 1991-2000 Silicon Graphics, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: SGI-B-2.0
 */

#include <assert.h>
#include "glxclient.h"
#include "indirect.h"
#include "indirect_vertex_array.h"

/*****************************************************************************/

#ifndef GLX_USE_APPLEGL
static void
do_enable_disable(GLenum array, GLboolean val)
{
   struct glx_context *gc = __glXGetCurrentContext();
   __GLXattribute *state = (__GLXattribute *) (gc->client_state_private);
   unsigned index = 0;

   if (array == GL_TEXTURE_COORD_ARRAY) {
      index = __glXGetActiveTextureUnit(state);
   }

   if (!__glXSetArrayEnable(state, array, index, val)) {
      __glXSetError(gc, GL_INVALID_ENUM);
   }
}

void
__indirect_glEnableClientState(GLenum array)
{
   do_enable_disable(array, GL_TRUE);
}

void
__indirect_glDisableClientState(GLenum array)
{
   do_enable_disable(array, GL_FALSE);
}

/************************************************************************/

void
__indirect_glPushClientAttrib(GLuint mask)
{
   struct glx_context *gc = __glXGetCurrentContext();
   __GLXattribute *state = (__GLXattribute *) (gc->client_state_private);
   __GLXattribute **spp = gc->attributes.stackPointer, *sp;

   if (spp < &gc->attributes.stack[__GL_CLIENT_ATTRIB_STACK_DEPTH]) {
      if (!(sp = *spp)) {
         sp = malloc(sizeof(__GLXattribute));
         if (sp == NULL) {
            __glXSetError(gc, GL_OUT_OF_MEMORY);
            return;
         }
         *spp = sp;
      }
      sp->mask = mask;
      gc->attributes.stackPointer = spp + 1;
      if (mask & GL_CLIENT_PIXEL_STORE_BIT) {
         sp->storePack = state->storePack;
         sp->storeUnpack = state->storeUnpack;
      }
      if (mask & GL_CLIENT_VERTEX_ARRAY_BIT) {
         __glXPushArrayState(state);
      }
   }
   else {
      __glXSetError(gc, GL_STACK_OVERFLOW);
      return;
   }
}

void
__indirect_glPopClientAttrib(void)
{
   struct glx_context *gc = __glXGetCurrentContext();
   __GLXattribute *state = (__GLXattribute *) (gc->client_state_private);
   __GLXattribute **spp = gc->attributes.stackPointer, *sp;
   GLuint mask;

   if (spp > &gc->attributes.stack[0]) {
      --spp;
      sp = *spp;
      assert(sp != 0);
      mask = sp->mask;
      gc->attributes.stackPointer = spp;

      if (mask & GL_CLIENT_PIXEL_STORE_BIT) {
         state->storePack = sp->storePack;
         state->storeUnpack = sp->storeUnpack;
      }
      if (mask & GL_CLIENT_VERTEX_ARRAY_BIT) {
         __glXPopArrayState(state);
      }

      sp->mask = 0;
   }
   else {
      __glXSetError(gc, GL_STACK_UNDERFLOW);
      return;
   }
}
#endif
