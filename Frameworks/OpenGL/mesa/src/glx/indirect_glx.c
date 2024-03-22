/*
 * Copyright © 2010 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Soft-
 * ware"), to deal in the Software without restriction, including without
 * limitation the rights to use, copy, modify, merge, publish, distribute,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, provided that the above copyright
 * notice(s) and this permission notice appear in all copies of the Soft-
 * ware and that both the above copyright notice(s) and this permission
 * notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABIL-
 * ITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY
 * RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN
 * THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSE-
 * QUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFOR-
 * MANCE OF THIS SOFTWARE.
 *
 * Except as contained in this notice, the name of a copyright holder shall
 * not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization of
 * the copyright holder.
 *
 * Authors:
 *   Kristian Høgsberg (krh@bitplanet.net)
 */

#include <stdbool.h>

#include "glapi.h"
#include "glxclient.h"
#include "indirect.h"
#include "util/u_debug.h"

#ifndef GLX_USE_APPLEGL

extern struct _glapi_table *__glXNewIndirectAPI(void);

/*
** All indirect rendering contexts will share the same indirect dispatch table.
*/
static struct _glapi_table *IndirectAPI = NULL;

static void
__glFreeAttributeState(struct glx_context * gc)
{
   __GLXattribute *sp, **spp;

   for (spp = &gc->attributes.stack[0];
        spp < &gc->attributes.stack[__GL_CLIENT_ATTRIB_STACK_DEPTH]; spp++) {
      sp = *spp;
      if (sp) {
         free((char *) sp);
      }
      else {
         break;
      }
   }
}

static void
indirect_destroy_context(struct glx_context *gc)
{
   __glXFreeVertexArrayState(gc);

   free((char *) gc->vendor);
   free((char *) gc->renderer);
   free((char *) gc->version);
   free((char *) gc->extensions);
   __glFreeAttributeState(gc);
   free((char *) gc->buf);
   free((char *) gc->client_state_private);
   free((char *) gc);
}

static Bool
SendMakeCurrentRequest(Display * dpy, GLXContextID gc_id,
                       GLXContextTag gc_tag, GLXDrawable draw,
                       GLXDrawable read, GLXContextTag *out_tag)
{
   xGLXMakeCurrentReply reply;
   Bool ret;
   int opcode = __glXSetupForCommand(dpy);

   LockDisplay(dpy);

   if (draw == read) {
      xGLXMakeCurrentReq *req;

      GetReq(GLXMakeCurrent, req);
      req->reqType = opcode;
      req->glxCode = X_GLXMakeCurrent;
      req->drawable = draw;
      req->context = gc_id;
      req->oldContextTag = gc_tag;
   }
   else {
      xGLXMakeContextCurrentReq *req;

      GetReq(GLXMakeContextCurrent, req);
      req->reqType = opcode;
      req->glxCode = X_GLXMakeContextCurrent;
      req->drawable = draw;
      req->readdrawable = read;
      req->context = gc_id;
      req->oldContextTag = gc_tag;
   }

   ret = _XReply(dpy, (xReply *) &reply, 0, False);

   if (out_tag)
      *out_tag = reply.contextTag;

   UnlockDisplay(dpy);
   SyncHandle();

   return ret;
}

static int
indirect_bind_context(struct glx_context *gc,
		      GLXDrawable draw, GLXDrawable read)
{
   Display *dpy = gc->psc->dpy;
   Bool sent;

   sent = SendMakeCurrentRequest(dpy, gc->xid, 0, draw, read,
				 &gc->currentContextTag);

   if (sent) {
      if (!IndirectAPI)
         IndirectAPI = __glXNewIndirectAPI();
      _glapi_set_dispatch(IndirectAPI);

      /* The indirect vertex array state must to be initialised after we
       * have setup the context, as it needs to query server attributes.
       *
       * At the point this is called gc->currentDpy is not initialized
       * nor is the thread's current context actually set. Hence the
       * cleverness before the GetString calls.
       */
      __GLXattribute *state = gc->client_state_private;
      if (state && state->array_state == NULL) {
         gc->currentDpy = gc->psc->dpy;
         __glXSetCurrentContext(gc);
         __indirect_glGetString(GL_EXTENSIONS);
         __indirect_glGetString(GL_VERSION);
         __glXInitVertexArrayState(gc);
      }
   }

   return !sent;
}

static void
indirect_unbind_context(struct glx_context *gc)
{
   Display *dpy = gc->psc->dpy;

   SendMakeCurrentRequest(dpy, None, gc->currentContextTag, None, None, NULL);
   gc->currentContextTag = 0;
}

static void
indirect_wait_gl(struct glx_context *gc)
{
   xGLXWaitGLReq *req;
   Display *dpy = gc->currentDpy;

   /* Flush any pending commands out */
   __glXFlushRenderBuffer(gc, gc->pc);

   /* Send the glXWaitGL request */
   LockDisplay(dpy);
   GetReq(GLXWaitGL, req);
   req->reqType = gc->majorOpcode;
   req->glxCode = X_GLXWaitGL;
   req->contextTag = gc->currentContextTag;
   UnlockDisplay(dpy);
   SyncHandle();
}

static void
indirect_wait_x(struct glx_context *gc)
{
   xGLXWaitXReq *req;
   Display *dpy = gc->currentDpy;

   /* Flush any pending commands out */
   __glXFlushRenderBuffer(gc, gc->pc);

   LockDisplay(dpy);
   GetReq(GLXWaitX, req);
   req->reqType = gc->majorOpcode;
   req->glxCode = X_GLXWaitX;
   req->contextTag = gc->currentContextTag;
   UnlockDisplay(dpy);
   SyncHandle();
}

static const struct glx_context_vtable indirect_context_vtable = {
   .destroy             = indirect_destroy_context,
   .bind                = indirect_bind_context,
   .unbind              = indirect_unbind_context,
   .wait_gl             = indirect_wait_gl,
   .wait_x              = indirect_wait_x,
};

_X_HIDDEN struct glx_context *
indirect_create_context(struct glx_screen *psc,
			struct glx_config *mode,
			struct glx_context *shareList, int renderType)
{
   unsigned error = 0;
   const uint32_t attribs[] = { GLX_RENDER_TYPE, renderType };

   return indirect_create_context_attribs(psc, mode, shareList,
                                          1, attribs, &error);
}

/**
 * \todo Eliminate \c __glXInitVertexArrayState.  Replace it with a new
 * function called \c __glXAllocateClientState that allocates the memory and
 * does all the initialization (including the pixel pack / unpack).
 */
_X_HIDDEN struct glx_context *
indirect_create_context_attribs(struct glx_screen *psc,
				struct glx_config *mode,
				struct glx_context *shareList,
				unsigned num_attribs,
				const uint32_t *attribs,
				unsigned *error)
{
   struct glx_context *gc;
   int bufSize;
   CARD8 opcode;
   __GLXattribute *state;
   int i, renderType = GLX_RGBA_TYPE;
   uint32_t mask = GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
   uint32_t major = 1;
   uint32_t minor = 0;

   opcode = __glXSetupForCommand(psc->dpy);
   if (!opcode) {
      *error = BadImplementation;
      return NULL;
   }

   for (i = 0; i < num_attribs; i++) {
      uint32_t attr = attribs[i*2], val = attribs[i*2 + 1];

      if (attr == GLX_RENDER_TYPE)
         renderType = val;
      if (attr == GLX_CONTEXT_PROFILE_MASK_ARB)
         mask = val;
      if (attr == GLX_CONTEXT_MAJOR_VERSION_ARB)
         major = val;
      if (attr == GLX_CONTEXT_MINOR_VERSION_ARB)
         minor = val;
   }

   /* We have no indirect support for core or ES contexts, and our compat
    * context support is limited to GL 1.4.
    */
   if (mask != GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB ||
       major != 1 ||
       minor > 4) {
      *error = GLXBadFBConfig;
      return NULL;
   }

   /* We can't share with a direct context */
   if (shareList && shareList->isDirect)
      return NULL;

   /* Allocate our context record */
   gc = calloc(1, sizeof *gc);
   if (!gc) {
      *error = BadAlloc;
      /* Out of memory */
      return NULL;
   }

   glx_context_init(gc, psc, mode);
   gc->isDirect = GL_FALSE;
   gc->vtable = &indirect_context_vtable;
   state = calloc(1, sizeof(struct __GLXattributeRec));
   gc->renderType = renderType;

   if (state == NULL) {
      /* Out of memory */
      *error = BadAlloc;
      free(gc);
      return NULL;
   }
   gc->client_state_private = state;
   state->NoDrawArraysProtocol = debug_get_bool_option("LIBGL_NO_DRAWARRAYS", false);

   /*
    ** Create a temporary buffer to hold GLX rendering commands.  The size
    ** of the buffer is selected so that the maximum number of GLX rendering
    ** commands can fit in a single X packet and still have room in the X
    ** packet for the GLXRenderReq header.
    */

   bufSize = (XMaxRequestSize(psc->dpy) * 4) - sz_xGLXRenderReq;
   gc->buf = malloc(bufSize);
   if (!gc->buf) {
      *error = BadAlloc;
      free(gc->client_state_private);
      free(gc);
      return NULL;
   }
   gc->bufSize = bufSize;

   /* Fill in the new context */
   gc->renderMode = GL_RENDER;

   state->storePack.alignment = 4;
   state->storeUnpack.alignment = 4;

   gc->attributes.stackPointer = &gc->attributes.stack[0];

   gc->pc = gc->buf;
   gc->bufEnd = gc->buf + bufSize;
   gc->isDirect = GL_FALSE;
   if (__glXDebug) {
      /*
       ** Set limit register so that there will be one command per packet
       */
      gc->limit = gc->buf;
   }
   else {
      gc->limit = gc->buf + bufSize - __GLX_BUFFER_LIMIT_SIZE;
   }
   gc->majorOpcode = opcode;

   /*
    ** Constrain the maximum drawing command size allowed to be
    ** transferred using the X_GLXRender protocol request.  First
    ** constrain by a software limit, then constrain by the protocol
    ** limit.
    */
   gc->maxSmallRenderCommandSize = MIN3(bufSize, __GLX_RENDER_CMD_SIZE_LIMIT,
                                        __GLX_MAX_RENDER_CMD_SIZE);


   return gc;
}

static const struct glx_screen_vtable indirect_screen_vtable = {
   .create_context         = indirect_create_context,
   .create_context_attribs = indirect_create_context_attribs,
   .query_renderer_integer = NULL,
   .query_renderer_string  = NULL,
};

_X_HIDDEN struct glx_screen *
indirect_create_screen(int screen, struct glx_display * priv)
{
   struct glx_screen *psc;

   psc = calloc(1, sizeof *psc);
   if (psc == NULL)
      return NULL;

   glx_screen_init(psc, screen, priv);
   psc->vtable = &indirect_screen_vtable;

   return psc;
}

#endif
