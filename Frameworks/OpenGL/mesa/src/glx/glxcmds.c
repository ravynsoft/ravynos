/*
 * SGI FREE SOFTWARE LICENSE B (Version 2.0, Sept. 18, 2008)
 * Copyright (C) 1991-2000 Silicon Graphics, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: SGI-B-2.0
 */

/**
 * \file glxcmds.c
 * Client-side GLX interface.
 */

#include "glxclient.h"
#include "glapi.h"
#include "glxextensions.h"
#include "indirect.h"
#include "glx_error.h"

#ifdef GLX_DIRECT_RENDERING
#ifdef GLX_USE_APPLEGL
#include "apple/apple_glx_context.h"
#include "apple/apple_glx.h"
#include "util/u_debug.h"
#else
#ifndef GLX_USE_WINDOWSGL
#include <X11/extensions/xf86vmode.h>
#endif /* GLX_USE_WINDOWSGL */
#endif
#endif
#include <limits.h>
#include <X11/Xlib-xcb.h>
#include <xcb/xcb.h>
#include <xcb/glx.h>
#include "GL/mesa_glinterop.h"

#if defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL)

/**
 * Get the __DRIdrawable for the drawable associated with a GLXContext
 *
 * \param dpy       The display associated with \c drawable.
 * \param drawable  GLXDrawable whose __DRIdrawable part is to be retrieved.
 * \param scrn_num  If non-NULL, the drawables screen is stored there
 * \returns  A pointer to the context's __DRIdrawable on success, or NULL if
 *           the drawable is not associated with a direct-rendering context.
 */
_X_HIDDEN __GLXDRIdrawable *
GetGLXDRIDrawable(Display * dpy, GLXDrawable drawable)
{
   struct glx_display *priv = __glXInitialize(dpy);
   __GLXDRIdrawable *pdraw;

   if (priv == NULL)
      return NULL;

   if (__glxHashLookup(priv->drawHash, drawable, (void *) &pdraw) == 0)
      return pdraw;

   return NULL;
}

#endif

_X_HIDDEN struct glx_drawable *
GetGLXDrawable(Display *dpy, GLXDrawable drawable)
{
   struct glx_display *priv = __glXInitialize(dpy);
   struct glx_drawable *glxDraw;

   if (priv == NULL)
      return NULL;

   if (__glxHashLookup(priv->glXDrawHash, drawable, (void *) &glxDraw) == 0)
      return glxDraw;

   return NULL;
}

_X_HIDDEN int
InitGLXDrawable(Display *dpy, struct glx_drawable *glxDraw, XID xDrawable,
		GLXDrawable drawable)
{
   struct glx_display *priv = __glXInitialize(dpy);

   if (!priv)
      return -1;

   glxDraw->xDrawable = xDrawable;
   glxDraw->drawable = drawable;
   glxDraw->lastEventSbc = 0;
   glxDraw->eventSbcWrap = 0;

   return __glxHashInsert(priv->glXDrawHash, drawable, glxDraw);
}

_X_HIDDEN void
DestroyGLXDrawable(Display *dpy, GLXDrawable drawable)
{
   struct glx_display *priv = __glXInitialize(dpy);
   struct glx_drawable *glxDraw;

   if (!priv)
      return;

   glxDraw = GetGLXDrawable(dpy, drawable);
   __glxHashDelete(priv->glXDrawHash, drawable);
   free(glxDraw);
}

/**
 * Get the GLX per-screen data structure associated with a GLX context.
 *
 * \param dpy   Display for which the GLX per-screen information is to be
 *              retrieved.
 * \param scrn  Screen on \c dpy for which the GLX per-screen information is
 *              to be retrieved.
 * \returns A pointer to the GLX per-screen data if \c dpy and \c scrn
 *          specify a valid GLX screen, or NULL otherwise.
 *
 * \todo Should this function validate that \c scrn is within the screen
 *       number range for \c dpy?
 */

_X_HIDDEN struct glx_screen *
GetGLXScreenConfigs(Display * dpy, int scrn)
{
   struct glx_display *const priv = __glXInitialize(dpy);

   return (priv
           && priv->screens !=
           NULL) ? priv->screens[scrn] : NULL;
}


static int
GetGLXPrivScreenConfig(Display * dpy, int scrn, struct glx_display ** ppriv,
                       struct glx_screen ** ppsc)
{
   /* Initialize the extension, if needed .  This has the added value
    * of initializing/allocating the display private
    */

   if (dpy == NULL) {
      return GLX_NO_EXTENSION;
   }

   *ppriv = __glXInitialize(dpy);
   if (*ppriv == NULL) {
      return GLX_NO_EXTENSION;
   }

   /* Check screen number to see if its valid */
   if ((scrn < 0) || (scrn >= ScreenCount(dpy))) {
      return GLX_BAD_SCREEN;
   }

   /* Check to see if the GL is supported on this screen */
   *ppsc = (*ppriv)->screens[scrn];
   if ((*ppsc)->configs == NULL && (*ppsc)->visuals == NULL) {
      /* No support for GL on this screen regardless of visual */
      return GLX_BAD_VISUAL;
   }

   return Success;
}


/**
 * Determine if a \c GLXFBConfig supplied by the application is valid.
 *
 * \param dpy     Application supplied \c Display pointer.
 * \param config  Application supplied \c GLXFBConfig.
 *
 * \returns If the \c GLXFBConfig is valid, the a pointer to the matching
 *          \c struct glx_config structure is returned.  Otherwise, \c NULL
 *          is returned.
 */
static struct glx_config *
ValidateGLXFBConfig(Display * dpy, GLXFBConfig fbconfig)
{
   struct glx_display *const priv = __glXInitialize(dpy);
   int num_screens = ScreenCount(dpy);
   unsigned i;
   struct glx_config *config;

   if (priv != NULL) {
      for (i = 0; i < num_screens; i++) {
	 for (config = priv->screens[i]->configs; config != NULL;
	      config = config->next) {
	    if (config == (struct glx_config *) fbconfig) {
	       return config;
	    }
	 }
      }
   }

   return NULL;
}

/**
 * Verifies context's GLX_RENDER_TYPE value with config.
 *
 * \param config GLX FBConfig which will support the returned renderType.
 * \param renderType The context render type to be verified.
 * \return True if the value of context renderType was approved, or 0 if no
 * valid value was found.
 */
Bool
validate_renderType_against_config(const struct glx_config *config,
                                   int renderType)
{
   /* GLX_EXT_no_config_context supports any render type */
   if (!config)
      return renderType == GLX_DONT_CARE;

   switch (renderType) {
      case GLX_RGBA_TYPE:
         return (config->renderType & GLX_RGBA_BIT) != 0;
      case GLX_COLOR_INDEX_TYPE:
         return (config->renderType & GLX_COLOR_INDEX_BIT) != 0;
      case GLX_RGBA_FLOAT_TYPE_ARB:
         return (config->renderType & GLX_RGBA_FLOAT_BIT_ARB) != 0;
      case GLX_RGBA_UNSIGNED_FLOAT_TYPE_EXT:
         return (config->renderType & GLX_RGBA_UNSIGNED_FLOAT_BIT_EXT) != 0;
      default:
         break;
   }
   return 0;
}

_X_HIDDEN Bool
glx_context_init(struct glx_context *gc,
		 struct glx_screen *psc, struct glx_config *config)
{
   gc->majorOpcode = __glXSetupForCommand(psc->display->dpy);
   if (!gc->majorOpcode)
      return False;

   gc->psc = psc;
   gc->config = config;
   gc->isDirect = GL_TRUE;
   gc->currentContextTag = -1;

   if (!config)
      gc->renderType = GLX_DONT_CARE;

   return True;
}

/**
 * Determine if a context uses direct rendering.
 *
 * \param dpy        Display where the context was created.
 * \param contextID  ID of the context to be tested.
 * \param error      Out parameter, set to True on error if not NULL,
 *                   otherwise raise the error to the application.
 *
 * \returns \c True if the context is direct rendering or not.
 */
static Bool
__glXIsDirect(Display * dpy, GLXContextID contextID, Bool *error)
{
   xcb_connection_t *c;
   xcb_generic_error_t *err;
   xcb_glx_is_direct_reply_t *reply;
   Bool is_direct;

   c = XGetXCBConnection(dpy);
   reply = xcb_glx_is_direct_reply(c, xcb_glx_is_direct(c, contextID), &err);
   is_direct = (reply != NULL && reply->is_direct) ? True : False;

   if (err != NULL) {
      if (error)
         *error = True;
      else
         __glXSendErrorForXcb(dpy, err);
      free(err);
   }

   free(reply);

   return is_direct;
}

/**
 * Create a new context.
 *
 * \param renderType   For FBConfigs, what is the rendering type?
 */

static GLXContext
CreateContext(Display *dpy, int generic_id, struct glx_config *config,
              GLXContext shareList_user, Bool allowDirect,
	      unsigned code, int renderType)
{
   struct glx_context *gc;
   struct glx_screen *psc;
   struct glx_context *shareList = (struct glx_context *) shareList_user;
   if (dpy == NULL)
      return NULL;

   psc = GetGLXScreenConfigs(dpy, config->screen);
   if (psc == NULL)
      return NULL;

   if (generic_id == None)
      return NULL;

   /* Some application may request an indirect context but we may want to force a direct
    * one because Xorg only allows indirect contexts if they were enabled.
    */
   if (!allowDirect &&
       psc->force_direct_context) {
      allowDirect = 1;
   }

   gc = NULL;
#ifdef GLX_USE_APPLEGL
   gc = applegl_create_context(psc, config, shareList, renderType);
#else
   if (allowDirect && psc->vtable->create_context)
      gc = psc->vtable->create_context(psc, config, shareList, renderType);
   if (!gc)
      gc = indirect_create_context(psc, config, shareList, renderType);
#endif
   if (!gc)
      return NULL;

   LockDisplay(dpy);
   switch (code) {
   case X_GLXCreateContext: {
      xGLXCreateContextReq *req;

      /* Send the glXCreateContext request */
      GetReq(GLXCreateContext, req);
      req->reqType = gc->majorOpcode;
      req->glxCode = X_GLXCreateContext;
      req->context = gc->xid = XAllocID(dpy);
      req->visual = generic_id;
      req->screen = config->screen;
      req->shareList = shareList ? shareList->xid : None;
      req->isDirect = gc->isDirect;
      break;
   }

   case X_GLXCreateNewContext: {
      xGLXCreateNewContextReq *req;

      /* Send the glXCreateNewContext request */
      GetReq(GLXCreateNewContext, req);
      req->reqType = gc->majorOpcode;
      req->glxCode = X_GLXCreateNewContext;
      req->context = gc->xid = XAllocID(dpy);
      req->fbconfig = generic_id;
      req->screen = config->screen;
      req->renderType = renderType;
      req->shareList = shareList ? shareList->xid : None;
      req->isDirect = gc->isDirect;
      break;
   }

   default:
      /* What to do here?  This case is the sign of an internal error.  It
       * should never be reachable.
       */
      break;
   }

   UnlockDisplay(dpy);
   SyncHandle();

   gc->share_xid = shareList ? shareList->xid : None;
   gc->imported = GL_FALSE;

   /* Unlike most X resource creation requests, we're about to return a handle
    * with client-side state, not just an XID. To simplify error handling
    * elsewhere in libGL, force a round-trip here to ensure the CreateContext
    * request above succeeded.
    */
   {
      Bool error = False;
      int isDirect = __glXIsDirect(dpy, gc->xid, &error);

      if (error != False || isDirect != gc->isDirect) {
         gc->vtable->destroy(gc);
         gc = NULL;
      }
   }

   return (GLXContext) gc;
}

_GLX_PUBLIC GLXContext
glXCreateContext(Display * dpy, XVisualInfo * vis,
                 GLXContext shareList, Bool allowDirect)
{
   struct glx_config *config = NULL;
   int renderType = GLX_RGBA_TYPE;

#if defined(GLX_DIRECT_RENDERING) || defined(GLX_USE_APPLEGL)
   struct glx_screen *const psc = GetGLXScreenConfigs(dpy, vis->screen);

   if (psc)
      config = glx_config_find_visual(psc->visuals, vis->visualid);

   if (config == NULL) {
      __glXSendError(dpy, BadValue, vis->visualid, X_GLXCreateContext, True);
      return None;
   }

   /* Choose the context render type based on DRI config values.  It is
    * unusual to set this type from config, but we have no other choice, as
    * this old API does not provide renderType parameter.
    */
   if (config->renderType & GLX_RGBA_FLOAT_BIT_ARB) {
       renderType = GLX_RGBA_FLOAT_TYPE_ARB;
   } else if (config->renderType & GLX_RGBA_UNSIGNED_FLOAT_BIT_EXT) {
       renderType = GLX_RGBA_UNSIGNED_FLOAT_TYPE_EXT;
   } else if (config->renderType & GLX_RGBA_BIT) {
       renderType = GLX_RGBA_TYPE;
   } else if (config->renderType & GLX_COLOR_INDEX_BIT) {
       renderType = GLX_COLOR_INDEX_TYPE;
   }
#endif

   return CreateContext(dpy, vis->visualid, config, shareList, allowDirect,
                        X_GLXCreateContext, renderType);
}

static void
glx_send_destroy_context(Display *dpy, XID xid)
{
   CARD8 opcode = __glXSetupForCommand(dpy);
   xGLXDestroyContextReq *req;

   LockDisplay(dpy);
   GetReq(GLXDestroyContext, req);
   req->reqType = opcode;
   req->glxCode = X_GLXDestroyContext;
   req->context = xid;
   UnlockDisplay(dpy);
   SyncHandle();
}

/*
** Destroy the named context
*/

_GLX_PUBLIC void
glXDestroyContext(Display * dpy, GLXContext ctx)
{
   struct glx_context *gc = (struct glx_context *) ctx;

   if (gc == NULL || gc->xid == None)
      return;

   __glXLock();
   if (!gc->imported)
      glx_send_destroy_context(dpy, gc->xid);

   if (gc->currentDpy) {
      /* This context is bound to some thread.  According to the man page,
       * we should not actually delete the context until it's unbound.
       * Note that we set gc->xid = None above.  In MakeContextCurrent()
       * we check for that and delete the context there.
       */
      gc->xid = None;
   } else {
      gc->vtable->destroy(gc);
   }
   __glXUnlock();
}

/*
** Return the major and minor version #s for the GLX extension
*/
_GLX_PUBLIC Bool
glXQueryVersion(Display * dpy, int *major, int *minor)
{
   struct glx_display *priv;

   /* Init the extension.  This fetches the major and minor version. */
   priv = __glXInitialize(dpy);
   if (!priv)
      return False;

   if (major)
      *major = GLX_MAJOR_VERSION;
   if (minor)
      *minor = priv->minorVersion;
   return True;
}

/*
** Query the existence of the GLX extension
*/
_GLX_PUBLIC Bool
glXQueryExtension(Display * dpy, int *errorBase, int *eventBase)
{
   int major_op, erb, evb;
   Bool rv;

   rv = XQueryExtension(dpy, GLX_EXTENSION_NAME, &major_op, &evb, &erb);
   if (rv) {
      if (errorBase)
         *errorBase = erb;
      if (eventBase)
         *eventBase = evb;
   }
   return rv;
}

/*
** Put a barrier in the token stream that forces the GL to finish its
** work before X can proceed.
*/
_GLX_PUBLIC void
glXWaitGL(void)
{
   struct glx_context *gc = __glXGetCurrentContext();

   if (gc->vtable->wait_gl)
      gc->vtable->wait_gl(gc);
}

/*
** Put a barrier in the token stream that forces X to finish its
** work before GL can proceed.
*/
_GLX_PUBLIC void
glXWaitX(void)
{
   struct glx_context *gc = __glXGetCurrentContext();

   if (gc->vtable->wait_x)
      gc->vtable->wait_x(gc);
}

_GLX_PUBLIC void
glXUseXFont(Font font, int first, int count, int listBase)
{
   struct glx_context *gc = __glXGetCurrentContext();
   xGLXUseXFontReq *req;
   Display *dpy = gc->currentDpy;

#ifdef GLX_DIRECT_RENDERING
   if (gc->isDirect) {
      DRI_glXUseXFont(gc, font, first, count, listBase);
      return;
   }
#endif

   /* Flush any pending commands out */
   __glXFlushRenderBuffer(gc, gc->pc);

   /* Send the glXUseFont request */
   LockDisplay(dpy);
   GetReq(GLXUseXFont, req);
   req->reqType = gc->majorOpcode;
   req->glxCode = X_GLXUseXFont;
   req->contextTag = gc->currentContextTag;
   req->font = font;
   req->first = first;
   req->count = count;
   req->listBase = listBase;
   UnlockDisplay(dpy);
   SyncHandle();
}

/************************************************************************/

/*
** Copy the source context to the destination context using the
** attribute "mask".
*/
_GLX_PUBLIC void
glXCopyContext(Display * dpy, GLXContext source_user,
	       GLXContext dest_user, unsigned long mask)
{
   struct glx_context *source = (struct glx_context *) source_user;
   struct glx_context *dest = (struct glx_context *) dest_user;

   /* GLX spec 3.3: If the destination context is current for some thread
    * then a BadAccess error is generated
    */
   if (dest && dest->currentDpy) {
      __glXSendError(dpy, BadAccess, 0, X_GLXCopyContext, true);
      return;
   }
#ifdef GLX_USE_APPLEGL
   struct glx_context *gc = __glXGetCurrentContext();
   int errorcode;
   bool x11error;

   if(apple_glx_copy_context(gc->driContext, source->driContext, dest->driContext,
                             mask, &errorcode, &x11error)) {
      __glXSendError(dpy, errorcode, 0, X_GLXCopyContext, x11error);
   }

#else
   xGLXCopyContextReq *req;
   struct glx_context *gc = __glXGetCurrentContext();
   GLXContextTag tag;
   CARD8 opcode;

   opcode = __glXSetupForCommand(dpy);
   if (!opcode) {
      return;
   }

#if defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL)
   if (gc->isDirect) {
      /* NOT_DONE: This does not work yet */
   }
#endif

   /*
    ** If the source is the current context, send its tag so that the context
    ** can be flushed before the copy.
    */
   if (source == gc && dpy == gc->currentDpy) {
      tag = gc->currentContextTag;
   }
   else {
      tag = 0;
   }

   /* Send the glXCopyContext request */
   LockDisplay(dpy);
   GetReq(GLXCopyContext, req);
   req->reqType = opcode;
   req->glxCode = X_GLXCopyContext;
   req->source = source ? source->xid : None;
   req->dest = dest ? dest->xid : None;
   req->mask = mask;
   req->contextTag = tag;
   UnlockDisplay(dpy);
   SyncHandle();
#endif /* GLX_USE_APPLEGL */
}


_GLX_PUBLIC Bool
glXIsDirect(Display * dpy, GLXContext gc_user)
{
   struct glx_context *gc = (struct glx_context *) gc_user;

   /* This is set for us at context creation */
   return gc ? gc->isDirect : False;
}

_GLX_PUBLIC void
glXSwapBuffers(Display * dpy, GLXDrawable drawable)
{
#ifdef GLX_USE_APPLEGL
   struct glx_context * gc = __glXGetCurrentContext();
   if(gc != &dummyContext && apple_glx_is_current_drawable(dpy, gc->driContext, drawable)) {
      apple_glx_swap_buffers(gc->driContext);
   } else {
      __glXSendError(dpy, GLXBadCurrentWindow, 0, X_GLXSwapBuffers, false);
   }
#else
   struct glx_context *gc;
   GLXContextTag tag;
   CARD8 opcode;
   xcb_connection_t *c;

   gc = __glXGetCurrentContext();

#if defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL)
   {
      __GLXDRIdrawable *pdraw = GetGLXDRIDrawable(dpy, drawable);

      if (pdraw != NULL) {
         Bool flush = gc != &dummyContext && drawable == gc->currentDrawable;

         if (pdraw->psc->driScreen->swapBuffers(pdraw, 0, 0, 0, flush) == -1)
             __glXSendError(dpy, GLXBadCurrentWindow, 0, X_GLXSwapBuffers, false);
         return;
      }
   }
#endif

   opcode = __glXSetupForCommand(dpy);
   if (!opcode) {
      return;
   }

   /*
    ** The calling thread may or may not have a current context.  If it
    ** does, send the context tag so the server can do a flush.
    */
   if ((gc != &dummyContext) && (dpy == gc->currentDpy) &&
       ((drawable == gc->currentDrawable)
        || (drawable == gc->currentReadable))) {
      tag = gc->currentContextTag;
   }
   else {
      tag = 0;
   }

   c = XGetXCBConnection(dpy);
   xcb_glx_swap_buffers(c, tag, drawable);
   xcb_flush(c);
#endif /* GLX_USE_APPLEGL */
}


/*
** Return configuration information for the given display, screen and
** visual combination.
*/
_GLX_PUBLIC int
glXGetConfig(Display * dpy, XVisualInfo * vis, int attribute,
             int *value_return)
{
   struct glx_display *priv;
   struct glx_screen *psc;
   struct glx_config *config;
   int status;

   status = GetGLXPrivScreenConfig(dpy, vis->screen, &priv, &psc);
   if (status == Success) {
      config = glx_config_find_visual(psc->visuals, vis->visualid);

      /* Lookup attribute after first finding a match on the visual */
      if (config != NULL) {
	 return glx_config_get(config, attribute, value_return);
      }

      status = GLX_BAD_VISUAL;
   }

   /*
    ** If we can't find the config for this visual, this visual is not
    ** supported by the OpenGL implementation on the server.
    */
   if ((status == GLX_BAD_VISUAL) && (attribute == GLX_USE_GL)) {
      *value_return = False;
      status = Success;
   }

   return status;
}

/************************************************************************/

static void
init_fbconfig_for_chooser(struct glx_config * config,
                          GLboolean fbconfig_style_tags)
{
   memset(config, 0, sizeof(struct glx_config));
   config->visualID = (XID) GLX_DONT_CARE;
   config->visualType = GLX_DONT_CARE;

   /* glXChooseFBConfig specifies different defaults for these properties than
    * glXChooseVisual.
    */
   if (fbconfig_style_tags) {
      config->doubleBufferMode = GLX_DONT_CARE;
      config->renderType = GLX_RGBA_BIT;
   }

   config->drawableType = GLX_WINDOW_BIT;
   config->visualRating = GLX_DONT_CARE;
   config->transparentPixel = GLX_NONE;
   config->transparentRed = GLX_DONT_CARE;
   config->transparentGreen = GLX_DONT_CARE;
   config->transparentBlue = GLX_DONT_CARE;
   config->transparentAlpha = GLX_DONT_CARE;
   config->transparentIndex = GLX_DONT_CARE;

   config->xRenderable = GLX_DONT_CARE;
   config->fbconfigID = (GLXFBConfigID) (GLX_DONT_CARE);

   config->sRGBCapable = GLX_DONT_CARE;
}

#define MATCH_DONT_CARE( param )        \
  do {                                  \
    if ( ((int) a-> param != (int) GLX_DONT_CARE)   \
         && (a-> param != b-> param) ) {        \
      return False;                             \
    }                                           \
  } while ( 0 )

#define MATCH_MINIMUM( param )                  \
  do {                                          \
    if ( ((int) a-> param != (int) GLX_DONT_CARE)	\
         && (a-> param > b-> param) ) {         \
      return False;                             \
    }                                           \
  } while ( 0 )

#define MATCH_EXACT( param )                    \
  do {                                          \
    if ( a-> param != b-> param) {              \
      return False;                             \
    }                                           \
  } while ( 0 )

/* Test that all bits from a are contained in b */
#define MATCH_MASK(param)			\
  do {						\
    if ( ((int) a-> param != (int) GLX_DONT_CARE)	\
         && ((a->param & ~b->param) != 0) ) {   \
      return False;				\
    }                                           \
  } while (0);

/**
 * Determine if two GLXFBConfigs are compatible.
 *
 * \param a  Application specified config to test.
 * \param b  Server specified config to test against \c a.
 */
static Bool
fbconfigs_compatible(const struct glx_config * const a,
                     const struct glx_config * const b)
{
   MATCH_DONT_CARE(doubleBufferMode);
   MATCH_DONT_CARE(visualType);
   MATCH_DONT_CARE(visualRating);
   MATCH_DONT_CARE(xRenderable);
   MATCH_DONT_CARE(fbconfigID);

   MATCH_MINIMUM(rgbBits);
   MATCH_MINIMUM(numAuxBuffers);
   MATCH_MINIMUM(redBits);
   MATCH_MINIMUM(greenBits);
   MATCH_MINIMUM(blueBits);
   MATCH_MINIMUM(alphaBits);
   MATCH_MINIMUM(depthBits);
   MATCH_MINIMUM(stencilBits);
   MATCH_MINIMUM(accumRedBits);
   MATCH_MINIMUM(accumGreenBits);
   MATCH_MINIMUM(accumBlueBits);
   MATCH_MINIMUM(accumAlphaBits);
   MATCH_MINIMUM(sampleBuffers);
   MATCH_MINIMUM(maxPbufferWidth);
   MATCH_MINIMUM(maxPbufferHeight);
   MATCH_MINIMUM(maxPbufferPixels);
   MATCH_MINIMUM(samples);

   MATCH_DONT_CARE(stereoMode);
   MATCH_EXACT(level);

   MATCH_MASK(drawableType);
   MATCH_MASK(renderType);
   MATCH_DONT_CARE(sRGBCapable);
   MATCH_DONT_CARE(floatComponentsNV);

   /* There is a bug in a few of the XFree86 DDX drivers.  They contain
    * visuals with a "transparent type" of 0 when they really mean GLX_NONE.
    * Technically speaking, it is a bug in the DDX driver, but there is
    * enough of an installed base to work around the problem here.  In any
    * case, 0 is not a valid value of the transparent type, so we'll treat 0
    * from the app as GLX_DONT_CARE. We'll consider GLX_NONE from the app and
    * 0 from the server to be a match to maintain backward compatibility with
    * the (broken) drivers.
    */

   if (a->transparentPixel != (int) GLX_DONT_CARE && a->transparentPixel != 0) {
      if (a->transparentPixel == GLX_NONE) {
         if (b->transparentPixel != GLX_NONE && b->transparentPixel != 0)
            return False;
      }
      else {
         MATCH_EXACT(transparentPixel);
      }

      switch (a->transparentPixel) {
      case GLX_TRANSPARENT_RGB:
         MATCH_DONT_CARE(transparentRed);
         MATCH_DONT_CARE(transparentGreen);
         MATCH_DONT_CARE(transparentBlue);
         MATCH_DONT_CARE(transparentAlpha);
         break;

      case GLX_TRANSPARENT_INDEX:
         MATCH_DONT_CARE(transparentIndex);
         break;

      default:
         break;
      }
   }

   return True;
}


/* There's some tricky language in the GLX spec about how this is supposed
 * to work.  Basically, if a given component size is either not specified
 * or the requested size is zero, it is supposed to act like PREFER_SMALLER.
 * Well, that's really hard to do with the code as-is.  This behavior is
 * closer to correct, but still not technically right.
 */
#define PREFER_LARGER_OR_ZERO(comp)             \
  do {                                          \
    if ( ((*a)-> comp) != ((*b)-> comp) ) {     \
      if ( ((*a)-> comp) == 0 ) {               \
        return -1;                              \
      }                                         \
      else if ( ((*b)-> comp) == 0 ) {          \
        return 1;                               \
      }                                         \
      else {                                    \
        return ((*b)-> comp) - ((*a)-> comp) ;  \
      }                                         \
    }                                           \
  } while( 0 )

#define PREFER_LARGER(comp)                     \
  do {                                          \
    if ( ((*a)-> comp) != ((*b)-> comp) ) {     \
      return ((*b)-> comp) - ((*a)-> comp) ;    \
    }                                           \
  } while( 0 )

#define PREFER_SMALLER(comp)                    \
  do {                                          \
    if ( ((*a)-> comp) != ((*b)-> comp) ) {     \
      return ((*a)-> comp) - ((*b)-> comp) ;    \
    }                                           \
  } while( 0 )

/**
 * Compare two GLXFBConfigs.  This function is intended to be used as the
 * compare function passed in to qsort.
 *
 * \returns If \c a is a "better" config, according to the specification of
 *          SGIX_fbconfig, a number less than zero is returned.  If \c b is
 *          better, then a number greater than zero is return.  If both are
 *          equal, zero is returned.
 * \sa qsort, glXChooseVisual, glXChooseFBConfig, glXChooseFBConfigSGIX
 */
static int
fbconfig_compare(struct glx_config **a, struct glx_config **b)
{
   /* The order of these comparisons must NOT change.  It is defined by
    * the GLX 1.4 specification.
    */

   PREFER_SMALLER(visualSelectGroup);

   /* The sort order for the visualRating is GLX_NONE, GLX_SLOW, and
    * GLX_NON_CONFORMANT_CONFIG.  It just so happens that this is the
    * numerical sort order of the enums (0x8000, 0x8001, and 0x800D).
    */
   PREFER_SMALLER(visualRating);

   /* This isn't quite right.  It is supposed to compare the sum of the
    * components the user specifically set minimums for.
    */
   PREFER_LARGER_OR_ZERO(redBits);
   PREFER_LARGER_OR_ZERO(greenBits);
   PREFER_LARGER_OR_ZERO(blueBits);
   PREFER_LARGER_OR_ZERO(alphaBits);

   PREFER_SMALLER(rgbBits);

   if (((*a)->doubleBufferMode != (*b)->doubleBufferMode)) {
      /* Prefer single-buffer.
       */
      return (!(*a)->doubleBufferMode) ? -1 : 1;
   }

   PREFER_SMALLER(numAuxBuffers);

   PREFER_SMALLER(sampleBuffers);
   PREFER_SMALLER(samples);

   PREFER_LARGER_OR_ZERO(depthBits);
   PREFER_SMALLER(stencilBits);

   /* This isn't quite right.  It is supposed to compare the sum of the
    * components the user specifically set minimums for.
    */
   PREFER_LARGER_OR_ZERO(accumRedBits);
   PREFER_LARGER_OR_ZERO(accumGreenBits);
   PREFER_LARGER_OR_ZERO(accumBlueBits);
   PREFER_LARGER_OR_ZERO(accumAlphaBits);

   PREFER_SMALLER(visualType);

   /* None of the pbuffer or fbconfig specs say that this comparison needs
    * to happen at all, but it seems like it should.
    */
   PREFER_LARGER(maxPbufferWidth);
   PREFER_LARGER(maxPbufferHeight);
   PREFER_LARGER(maxPbufferPixels);

   return 0;
}


/**
 * Selects and sorts a subset of the supplied configs based on the attributes.
 * This function forms to basis of \c glXChooseFBConfig and
 * \c glXChooseFBConfigSGIX.
 *
 * \param configs   Array of pointers to possible configs.  The elements of
 *                  this array that do not meet the criteria will be set to
 *                  NULL.  The remaining elements will be sorted according to
 *                  the various visual / FBConfig selection rules.
 * \param num_configs  Number of elements in the \c configs array.
 * \param attribList   Attributes used select from \c configs.  This array is
 *                     terminated by a \c None tag.  The array is of the form
 *                     expected by \c glXChooseFBConfig (where every tag has a
 *                     value).
 * \returns The number of valid elements left in \c configs.
 *
 * \sa glXChooseFBConfig, glXChooseFBConfigSGIX
 */
static int
choose_fbconfig(struct glx_config ** configs, int num_configs,
              const int *attribList)
{
   struct glx_config test_config;
   int base;
   int i;

   /* This is a fairly direct implementation of the selection method
    * described by GLX_SGIX_fbconfig.  Start by culling out all the
    * configs that are not compatible with the selected parameter
    * list.
    */

   init_fbconfig_for_chooser(&test_config, GL_TRUE);
   __glXInitializeVisualConfigFromTags(&test_config, 512,
                                       (const INT32 *) attribList,
                                       GL_TRUE, GL_TRUE);

   base = 0;
   for (i = 0; i < num_configs; i++) {
      if (fbconfigs_compatible(&test_config, configs[i])) {
         configs[base] = configs[i];
         base++;
      }
   }

   if (base == 0) {
      return 0;
   }

   if (base < num_configs) {
      (void) memset(&configs[base], 0, sizeof(void *) * (num_configs - base));
   }

   /* After the incompatible configs are removed, the resulting
    * list is sorted according to the rules set out in the various
    * specifications.
    */

   qsort(configs, base, sizeof(struct glx_config *),
         (int (*)(const void *, const void *)) fbconfig_compare);
   return base;
}




/*
** Return the visual that best matches the template.  Return None if no
** visual matches the template.
*/
_GLX_PUBLIC XVisualInfo *
glXChooseVisual(Display * dpy, int screen, int *attribList)
{
   XVisualInfo *visualList = NULL;
   struct glx_display *priv;
   struct glx_screen *psc;
   struct glx_config test_config;
   struct glx_config *config;
   struct glx_config *best_config = NULL;

   /*
    ** Get a list of all visuals, return if list is empty
    */
   if (GetGLXPrivScreenConfig(dpy, screen, &priv, &psc) != Success) {
      return None;
   }


   /*
    ** Build a template from the defaults and the attribute list
    ** Free visual list and return if an unexpected token is encountered
    */
   init_fbconfig_for_chooser(&test_config, GL_FALSE);
   __glXInitializeVisualConfigFromTags(&test_config, 512,
                                       (const INT32 *) attribList,
                                       GL_TRUE, GL_FALSE);

   /*
    ** Eliminate visuals that don't meet minimum requirements
    ** Compute a score for those that do
    ** Remember which visual, if any, got the highest score
    ** If no visual is acceptable, return None
    ** Otherwise, create an XVisualInfo list with just the selected X visual
    ** and return this.
    */
   for (config = psc->visuals; config != NULL; config = config->next) {
      if (fbconfigs_compatible(&test_config, config)
          && ((best_config == NULL) ||
              (fbconfig_compare (&config, &best_config) < 0))) {
         XVisualInfo visualTemplate;
         XVisualInfo *newList;
         int i;

         visualTemplate.screen = screen;
         visualTemplate.visualid = config->visualID;
         newList = XGetVisualInfo(dpy, VisualScreenMask | VisualIDMask,
                                  &visualTemplate, &i);

         if (newList) {
            XFree(visualList);
            visualList = newList;
            best_config = config;
         }
      }
   }

#ifdef GLX_USE_APPLEGL
   if(visualList && debug_get_bool_option("LIBGL_DUMP_VISUALID", false)) {
      printf("visualid 0x%lx\n", visualList[0].visualid);
   }
#endif

   return visualList;
}


_GLX_PUBLIC const char *
glXQueryExtensionsString(Display * dpy, int screen)
{
   struct glx_screen *psc;
   struct glx_display *priv;
   int is_direct_capable = GL_FALSE;

   if (GetGLXPrivScreenConfig(dpy, screen, &priv, &psc) != Success) {
      return NULL;
   }

   if (!psc->effectiveGLXexts) {
      if (!psc->serverGLXexts) {
         psc->serverGLXexts =
            __glXQueryServerString(dpy, screen, GLX_EXTENSIONS);
      }

#if defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL)
      is_direct_capable = (psc->driScreen != NULL);
#endif
      __glXCalculateUsableExtensions(psc, is_direct_capable);
   }

   return psc->effectiveGLXexts;
}

_GLX_PUBLIC const char *
glXGetClientString(Display * dpy, int name)
{
   (void) dpy;

   switch (name) {
   case GLX_VENDOR:
      return "Mesa Project and SGI";
   case GLX_VERSION:
      return "1.4";
   case GLX_EXTENSIONS:
      return __glXGetClientExtensions(dpy);
   default:
      return NULL;
   }
}

_GLX_PUBLIC const char *
glXQueryServerString(Display * dpy, int screen, int name)
{
   struct glx_screen *psc;
   struct glx_display *priv;
   const char **str;

   if (GetGLXPrivScreenConfig(dpy, screen, &priv, &psc) != Success) {
      return NULL;
   }

   switch (name) {
   case GLX_VENDOR:
      str = &psc->serverGLXvendor;
      break;
   case GLX_VERSION:
      str = &psc->serverGLXversion;
      break;
   case GLX_EXTENSIONS:
      str = &psc->serverGLXexts;
      break;
   default:
      return NULL;
   }

   if (*str == NULL) {
      *str = __glXQueryServerString(dpy, screen, name);
   }

   return *str;
}


/*
** EXT_import_context
*/

_GLX_PUBLIC Display *
glXGetCurrentDisplay(void)
{
   struct glx_context *gc = __glXGetCurrentContext();
   if (gc == &dummyContext)
      return NULL;
   return gc->currentDpy;
}

_GLX_PUBLIC
GLX_ALIAS(Display *, glXGetCurrentDisplayEXT, (void), (),
          glXGetCurrentDisplay)

#ifndef GLX_USE_APPLEGL
_GLX_PUBLIC GLXContext
glXImportContextEXT(Display *dpy, GLXContextID contextID)
{
   struct glx_display *priv = __glXInitialize(dpy);
   struct glx_screen *psc = NULL;
   xGLXQueryContextReply reply;
   CARD8 opcode;
   struct glx_context *ctx;
   int i, renderType = GLX_RGBA_TYPE; /* By default, assume RGBA context */
   XID share = None;
   struct glx_config *mode = NULL;
   uint32_t fbconfigID = 0;
   uint32_t visualID = 0;
   uint32_t screen = 0;
   Bool got_screen = False;

   if (priv == NULL)
      return NULL;

   /* The GLX_EXT_import_context spec says:
    *
    *     "If <contextID> does not refer to a valid context, then a BadContext
    *     error is generated; if <contextID> refers to direct rendering
    *     context then no error is generated but glXImportContextEXT returns
    *     NULL."
    *
    * We can handle both conditions with the __glXIsDirect call, because
    * passing None to a GLXIsDirect request will throw GLXBadContext.
    */
   if (__glXIsDirect(dpy, contextID, NULL))
      return NULL;

   opcode = __glXSetupForCommand(dpy);
   if (!opcode)
      return 0;

   /* Send the glXQueryContextInfoEXT request */
   LockDisplay(dpy);

   xGLXQueryContextReq *req;
   GetReq(GLXQueryContext, req);

   req->reqType = opcode;
   req->glxCode = X_GLXQueryContext;
   req->context = contextID;

   if (_XReply(dpy, (xReply *) & reply, 0, False) &&
       reply.n < (INT32_MAX / 2)) {

      for (i = 0; i < reply.n; i++) {
         int prop[2];

         _XRead(dpy, (char *)prop, sizeof(prop));
         switch (prop[0]) {
         case GLX_SCREEN:
            screen = prop[1];
            got_screen = True;
            break;
         case GLX_SHARE_CONTEXT_EXT:
            share = prop[1];
            break;
         case GLX_VISUAL_ID_EXT:
            visualID = prop[1];
            break;
         case GLX_FBCONFIG_ID:
            fbconfigID = prop[1];
            break;
         case GLX_RENDER_TYPE:
            renderType = prop[1];
            break;
         }
      }
   }
   UnlockDisplay(dpy);
   SyncHandle();

   if (!got_screen)
      return NULL;

   psc = GetGLXScreenConfigs(dpy, screen);
   if (psc == NULL)
      return NULL;

   if (fbconfigID != 0) {
      mode = glx_config_find_fbconfig(psc->configs, fbconfigID);
   } else if (visualID != 0) {
      mode = glx_config_find_visual(psc->visuals, visualID);
   }

   if (mode == NULL)
      return NULL;

   ctx = indirect_create_context(psc, mode, NULL, renderType);
   if (ctx == NULL)
      return NULL;

   ctx->xid = contextID;
   ctx->imported = GL_TRUE;
   ctx->share_xid = share;

   return (GLXContext) ctx;
}

#endif

_GLX_PUBLIC int
glXQueryContext(Display * dpy, GLXContext ctx_user, int attribute, int *value)
{
   struct glx_context *ctx = (struct glx_context *) ctx_user;

   switch (attribute) {
      case GLX_SHARE_CONTEXT_EXT:
      *value = ctx->share_xid;
      break;
   case GLX_VISUAL_ID_EXT:
      *value = ctx->config ? ctx->config->visualID : None;
      break;
   case GLX_SCREEN:
      *value = ctx->psc->scr;
      break;
   case GLX_FBCONFIG_ID:
      *value = ctx->config ? ctx->config->fbconfigID : None;
      break;
   case GLX_RENDER_TYPE:
      *value = ctx->renderType;
      break;
   default:
      return GLX_BAD_ATTRIBUTE;
   }
   return Success;
}

_GLX_PUBLIC
GLX_ALIAS(int, glXQueryContextInfoEXT,
          (Display * dpy, GLXContext ctx, int attribute, int *value),
          (dpy, ctx, attribute, value), glXQueryContext)

_GLX_PUBLIC GLXContextID glXGetContextIDEXT(const GLXContext ctx_user)
{
   struct glx_context *ctx = (struct glx_context *) ctx_user;

   return (ctx == NULL) ? None : ctx->xid;
}

_GLX_PUBLIC void
glXFreeContextEXT(Display *dpy, GLXContext ctx)
{
   struct glx_context *gc = (struct glx_context *) ctx;

   if (gc == NULL || gc->xid == None)
      return;

   /* The GLX_EXT_import_context spec says:
    *
    *     "glXFreeContext does not free the server-side context information or
    *     the XID associated with the server-side context."
    *
    * Don't send any protocol.  Just destroy the client-side tracking of the
    * context.  Also, only release the context structure if it's not current.
    */
   __glXLock();
   if (gc->currentDpy) {
      gc->xid = None;
   } else {
      gc->vtable->destroy(gc);
   }
   __glXUnlock();
}

_GLX_PUBLIC GLXFBConfig *
glXChooseFBConfig(Display * dpy, int screen,
                  const int *attribList, int *nitems)
{
   struct glx_config **config_list;
   int list_size;


   config_list = (struct glx_config **)
      glXGetFBConfigs(dpy, screen, &list_size);

   if ((config_list != NULL) && (list_size > 0) && (attribList != NULL)) {
      list_size = choose_fbconfig(config_list, list_size, attribList);
      if (list_size == 0) {
         free(config_list);
         config_list = NULL;
      }
   }

   *nitems = list_size;
   return (GLXFBConfig *) config_list;
}


_GLX_PUBLIC GLXContext
glXCreateNewContext(Display * dpy, GLXFBConfig fbconfig,
                    int renderType, GLXContext shareList, Bool allowDirect)
{
   struct glx_config *config = (struct glx_config *) fbconfig;
   struct glx_config **config_list;
   int list_size;
   unsigned i;

   if (!config) {
       __glXSendError(dpy, GLXBadFBConfig, 0, X_GLXCreateNewContext, false);
       return NULL;
   }

   config_list = (struct glx_config **)
      glXGetFBConfigs(dpy, config->screen, &list_size);

   for (i = 0; i < list_size; i++) {
       if (config_list[i] == config)
           break;
   }
   free(config_list);

   if (i == list_size) {
       __glXSendError(dpy, GLXBadFBConfig, 0, X_GLXCreateNewContext, false);
       return NULL;
   }

   return CreateContext(dpy, config->fbconfigID, config, shareList,
			allowDirect, X_GLXCreateNewContext, renderType);
}


_GLX_PUBLIC GLXDrawable
glXGetCurrentReadDrawable(void)
{
   struct glx_context *gc = __glXGetCurrentContext();

   return gc->currentReadable;
}


_GLX_PUBLIC GLXFBConfig *
glXGetFBConfigs(Display * dpy, int screen, int *nelements)
{
   struct glx_display *priv = __glXInitialize(dpy);
   struct glx_config **config_list = NULL;
   struct glx_config *config;
   unsigned num_configs = 0;
   int i;

   *nelements = 0;
   if (priv && (priv->screens != NULL)
       && (screen >= 0) && (screen < ScreenCount(dpy))
       && (priv->screens[screen]->configs != NULL)
       && (priv->screens[screen]->configs->fbconfigID
	   != (int) GLX_DONT_CARE)) {

      for (config = priv->screens[screen]->configs; config != NULL;
           config = config->next) {
         if (config->fbconfigID != (int) GLX_DONT_CARE) {
            num_configs++;
         }
      }

      config_list = malloc(num_configs * sizeof *config_list);
      if (config_list != NULL) {
         *nelements = num_configs;
         i = 0;
         for (config = priv->screens[screen]->configs; config != NULL;
              config = config->next) {
            if (config->fbconfigID != (int) GLX_DONT_CARE) {
               config_list[i] = config;
               i++;
            }
         }
      }
   }

   return (GLXFBConfig *) config_list;
}


_GLX_PUBLIC int
glXGetFBConfigAttrib(Display * dpy, GLXFBConfig fbconfig,
                     int attribute, int *value)
{
   struct glx_config *config = ValidateGLXFBConfig(dpy, fbconfig);

   if (config == NULL)
      return GLXBadFBConfig;

   return glx_config_get(config, attribute, value);
}


_GLX_PUBLIC XVisualInfo *
glXGetVisualFromFBConfig(Display * dpy, GLXFBConfig fbconfig)
{
   XVisualInfo visualTemplate;
   struct glx_config *config = (struct glx_config *) fbconfig;
   int count;

   if (!config)
      return NULL;

   /*
    ** Get a list of all visuals, return if list is empty
    */
   visualTemplate.visualid = config->visualID;
   return XGetVisualInfo(dpy, VisualIDMask, &visualTemplate, &count);
}

#ifndef GLX_USE_APPLEGL
/*
** GLX_SGI_swap_control
*/
_X_HIDDEN int
glXSwapIntervalSGI(int interval)
{
   xGLXVendorPrivateReq *req;
   struct glx_context *gc = __glXGetCurrentContext();
#ifdef GLX_DIRECT_RENDERING
   struct glx_screen *psc = gc->psc;
#endif
   Display *dpy;
   CARD32 *interval_ptr;
   CARD8 opcode;

   if (gc == &dummyContext) {
      return GLX_BAD_CONTEXT;
   }

   if (interval <= 0) {
      return GLX_BAD_VALUE;
   }

#ifdef GLX_DIRECT_RENDERING
   if (gc->isDirect && psc && psc->driScreen &&
          psc->driScreen->setSwapInterval) {
      __GLXDRIdrawable *pdraw =
	 GetGLXDRIDrawable(gc->currentDpy, gc->currentDrawable);
      /* Simply ignore the command if the GLX drawable has been destroyed but
       * the context is still bound.
       */
      if (pdraw)
         psc->driScreen->setSwapInterval(pdraw, interval);
      return 0;
   }
#endif

   dpy = gc->currentDpy;
   opcode = __glXSetupForCommand(dpy);
   if (!opcode) {
      return 0;
   }

   /* Send the glXSwapIntervalSGI request */
   LockDisplay(dpy);
   GetReqExtra(GLXVendorPrivate, sizeof(CARD32), req);
   req->reqType = opcode;
   req->glxCode = X_GLXVendorPrivate;
   req->vendorCode = X_GLXvop_SwapIntervalSGI;
   req->contextTag = gc->currentContextTag;

   interval_ptr = (CARD32 *) (req + 1);
   *interval_ptr = interval;

   UnlockDisplay(dpy);
   SyncHandle();
   XFlush(dpy);

   return 0;
}


/*
** GLX_MESA_swap_control
*/
_X_HIDDEN int
glXSwapIntervalMESA(unsigned int interval)
{
#ifdef GLX_DIRECT_RENDERING
   struct glx_context *gc = __glXGetCurrentContext();

   if (interval > INT_MAX)
      return GLX_BAD_VALUE;

   if (gc != &dummyContext && gc->isDirect) {
      struct glx_screen *psc = gc->psc;
      if (psc && psc->driScreen && psc->driScreen->setSwapInterval) {
         __GLXDRIdrawable *pdraw =
	    GetGLXDRIDrawable(gc->currentDpy, gc->currentDrawable);

         /* Simply ignore the command if the GLX drawable has been destroyed but
          * the context is still bound.
          */
         if (!pdraw)
            return 0;

         return psc->driScreen->setSwapInterval(pdraw, interval);
      }
   }
#endif

   return GLX_BAD_CONTEXT;
}


_X_HIDDEN int
glXGetSwapIntervalMESA(void)
{
#ifdef GLX_DIRECT_RENDERING
   struct glx_context *gc = __glXGetCurrentContext();

   if (gc != &dummyContext && gc->isDirect) {
      struct glx_screen *psc = gc->psc;
      if (psc && psc->driScreen && psc->driScreen->getSwapInterval) {
         __GLXDRIdrawable *pdraw =
	    GetGLXDRIDrawable(gc->currentDpy, gc->currentDrawable);
         if (pdraw)
            return psc->driScreen->getSwapInterval(pdraw);
      }
   }
#endif

   return 0;
}


/*
** GLX_EXT_swap_control
*/
_X_HIDDEN void
glXSwapIntervalEXT(Display *dpy, GLXDrawable drawable, int interval)
{
#ifdef GLX_DIRECT_RENDERING
   __GLXDRIdrawable *pdraw = GetGLXDRIDrawable(dpy, drawable);

   /*
    * Strictly, this should throw an error if drawable is not a Window or
    * GLXWindow. We don't actually track that, so, oh well.
    */
   if (!pdraw) {
       __glXSendError(dpy, BadWindow, drawable, 0, True);
      return;
   }

   if (interval < 0 &&
       !__glXExtensionBitIsEnabled(pdraw->psc, EXT_swap_control_tear_bit)) {
      __glXSendError(dpy, BadValue, interval, 0, True);
      return;
   }
   if (pdraw->psc->driScreen->setSwapInterval)
      pdraw->psc->driScreen->setSwapInterval(pdraw, interval);
#endif
}


/*
** GLX_SGI_video_sync
*/
_X_HIDDEN int
glXGetVideoSyncSGI(unsigned int *count)
{
#ifdef GLX_DIRECT_RENDERING
   int64_t ust, msc, sbc;
   int ret;
   struct glx_context *gc = __glXGetCurrentContext();
   struct glx_screen *psc = gc->psc;
   __GLXDRIdrawable *pdraw;

   if (gc == &dummyContext)
      return GLX_BAD_CONTEXT;

   if (!gc->isDirect)
      return GLX_BAD_CONTEXT;

   if (!gc->currentDrawable)
      return GLX_BAD_CONTEXT;

   pdraw = GetGLXDRIDrawable(gc->currentDpy, gc->currentDrawable);

   /* FIXME: Looking at the GLX_SGI_video_sync spec in the extension registry,
    * FIXME: there should be a GLX encoding for this call.  I can find no
    * FIXME: documentation for the GLX encoding.
    */
   if (psc && psc->driScreen && psc->driScreen->getDrawableMSC) {
      ret = psc->driScreen->getDrawableMSC(psc, pdraw, &ust, &msc, &sbc);
      *count = (unsigned) msc;
      return (ret == True) ? 0 : GLX_BAD_CONTEXT;
   }
#endif

   return GLX_BAD_CONTEXT;
}

_X_HIDDEN int
glXWaitVideoSyncSGI(int divisor, int remainder, unsigned int *count)
{
   struct glx_context *gc = __glXGetCurrentContext();
#ifdef GLX_DIRECT_RENDERING
   struct glx_screen *psc = gc->psc;
   __GLXDRIdrawable *pdraw;
   int64_t ust, msc, sbc;
   int ret;
#endif

   if (divisor <= 0 || remainder < 0)
      return GLX_BAD_VALUE;

   if (gc == &dummyContext)
      return GLX_BAD_CONTEXT;

#ifdef GLX_DIRECT_RENDERING
   if (!gc->isDirect)
      return GLX_BAD_CONTEXT;

   if (!gc->currentDrawable)
      return GLX_BAD_CONTEXT;

   pdraw = GetGLXDRIDrawable(gc->currentDpy, gc->currentDrawable);

   if (psc && psc->driScreen && psc->driScreen->waitForMSC) {
      ret = psc->driScreen->waitForMSC(pdraw, 0, divisor, remainder, &ust, &msc,
				       &sbc);
      *count = (unsigned) msc;
      return (ret == True) ? 0 : GLX_BAD_CONTEXT;
   }
#endif

   return GLX_BAD_CONTEXT;
}

#endif /* GLX_USE_APPLEGL */

/*
** GLX_SGIX_fbconfig
** Many of these functions are aliased to GLX 1.3 entry points in the
** GLX_functions table.
*/

_GLX_PUBLIC
GLX_ALIAS(int, glXGetFBConfigAttribSGIX,
          (Display * dpy, GLXFBConfigSGIX config, int attribute, int *value),
          (dpy, config, attribute, value), glXGetFBConfigAttrib)

_GLX_PUBLIC GLX_ALIAS(GLXFBConfigSGIX *, glXChooseFBConfigSGIX,
                 (Display * dpy, int screen, int *attrib_list,
                  int *nelements), (dpy, screen, attrib_list, nelements),
                 glXChooseFBConfig)

_GLX_PUBLIC GLX_ALIAS(XVisualInfo *, glXGetVisualFromFBConfigSGIX,
                 (Display * dpy, GLXFBConfigSGIX config),
                 (dpy, config), glXGetVisualFromFBConfig)

_GLX_PUBLIC GLX_ALIAS(GLXContext, glXCreateContextWithConfigSGIX,
                      (Display *dpy, GLXFBConfigSGIX fbconfig,
                       int renderType, GLXContext shareList, Bool direct),
                      (dpy, fbconfig, renderType, shareList, direct),
                      glXCreateNewContext)

_GLX_PUBLIC GLXFBConfigSGIX
glXGetFBConfigFromVisualSGIX(Display * dpy, XVisualInfo * vis)
{
   int attrib_list[] = { GLX_VISUAL_ID, vis->visualid, None };
   int nconfigs = 0;
   GLXFBConfig *config_list;
   GLXFBConfig config;

   config_list = glXChooseFBConfig(dpy, vis->screen, attrib_list, &nconfigs);
   if (nconfigs == 0)
      return NULL;

   config = config_list[0];
   free(config_list);
   return (GLXFBConfigSGIX)config;
}

#ifndef GLX_USE_APPLEGL
/*
** GLX_OML_sync_control
*/
_X_HIDDEN Bool
glXGetSyncValuesOML(Display *dpy, GLXDrawable drawable,
                    int64_t *ust, int64_t *msc, int64_t *sbc)
{
   struct glx_display * const priv = __glXInitialize(dpy);
#ifdef GLX_DIRECT_RENDERING
   int ret;
   __GLXDRIdrawable *pdraw;
   struct glx_screen *psc;
#endif

   if (!priv)
      return False;

#ifdef GLX_DIRECT_RENDERING
   pdraw = GetGLXDRIDrawable(dpy, drawable);
   psc = pdraw ? pdraw->psc : NULL;
   if (pdraw && psc->driScreen->getDrawableMSC) {
      ret = psc->driScreen->getDrawableMSC(psc, pdraw, ust, msc, sbc);
      return ret;
   }
#endif

   return False;
}

#if defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL)
_X_HIDDEN GLboolean
__glxGetMscRate(struct glx_screen *psc,
		int32_t * numerator, int32_t * denominator)
{
#if !defined(GLX_USE_WINDOWSGL)
   XF86VidModeModeLine mode_line;
   int dot_clock;
   int i;

   if (XF86VidModeQueryVersion(psc->dpy, &i, &i) &&
       XF86VidModeGetModeLine(psc->dpy, psc->scr, &dot_clock, &mode_line)) {
      unsigned n = dot_clock * 1000;
      unsigned d = mode_line.vtotal * mode_line.htotal;

# define V_INTERLACE 0x010
# define V_DBLSCAN   0x020

      if (mode_line.flags & V_INTERLACE)
         n *= 2;
      else if (mode_line.flags & V_DBLSCAN)
         d *= 2;

      /* The OML_sync_control spec requires that if the refresh rate is a
       * whole number, that the returned numerator be equal to the refresh
       * rate and the denominator be 1.
       */

      if (n % d == 0) {
         n /= d;
         d = 1;
      }
      else {
         static const unsigned f[] = { 13, 11, 7, 5, 3, 2, 0 };

         /* This is a poor man's way to reduce a fraction.  It's far from
          * perfect, but it will work well enough for this situation.
          */

         for (i = 0; f[i] != 0; i++) {
            while (n % f[i] == 0 && d % f[i] == 0) {
               d /= f[i];
               n /= f[i];
            }
         }
      }

      *numerator = n;
      *denominator = d;

      return True;
   }
#endif

   return False;
}
#endif

/**
 * Determine the refresh rate of the specified drawable and display.
 *
 * \param dpy          Display whose refresh rate is to be determined.
 * \param drawable     Drawable whose refresh rate is to be determined.
 * \param numerator    Numerator of the refresh rate.
 * \param denominator  Denominator of the refresh rate.
 * \return  If the refresh rate for the specified display and drawable could
 *          be calculated, True is returned.  Otherwise False is returned.
 *
 * \note This function is implemented entirely client-side.  A lot of other
 *       functionality is required to export GLX_OML_sync_control, so on
 *       XFree86 this function can be called for direct-rendering contexts
 *       when GLX_OML_sync_control appears in the client extension string.
 */

_X_HIDDEN Bool
glXGetMscRateOML(Display * dpy, GLXDrawable drawable,
                 int32_t * numerator, int32_t * denominator)
{
#if defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL) && !defined(GLX_USE_WINDOWSGL)
   __GLXDRIdrawable *draw = GetGLXDRIDrawable(dpy, drawable);

   if (draw == NULL)
      return False;

   return __glxGetMscRate(draw->psc, numerator, denominator);
#else
   (void) dpy;
   (void) drawable;
   (void) numerator;
   (void) denominator;
#endif
   return False;
}


_X_HIDDEN int64_t
glXSwapBuffersMscOML(Display *dpy, GLXDrawable drawable,
                     int64_t target_msc, int64_t divisor, int64_t remainder)
{
   struct glx_context *gc = __glXGetCurrentContext();
#ifdef GLX_DIRECT_RENDERING
   __GLXDRIdrawable *pdraw = GetGLXDRIDrawable(dpy, drawable);
   struct glx_screen *psc = pdraw ? pdraw->psc : NULL;
#endif

   if (gc == &dummyContext) /* no GLX for this */
      return -1;

#ifdef GLX_DIRECT_RENDERING
   if (!pdraw || !gc->isDirect)
      return -1;
#endif

   /* The OML_sync_control spec says these should "generate a GLX_BAD_VALUE
    * error", but it also says "It [glXSwapBuffersMscOML] will return a value
    * of -1 if the function failed because of errors detected in the input
    * parameters"
    */
   if (divisor < 0 || remainder < 0 || target_msc < 0)
      return -1;
   if (divisor > 0 && remainder >= divisor)
      return -1;

   if (target_msc == 0 && divisor == 0 && remainder == 0)
      remainder = 1;

#ifdef GLX_DIRECT_RENDERING
   if (psc->driScreen && psc->driScreen->swapBuffers)
      return psc->driScreen->swapBuffers(pdraw, target_msc, divisor,
					    remainder, False);
#endif

   return -1;
}


_X_HIDDEN Bool
glXWaitForMscOML(Display *dpy, GLXDrawable drawable, int64_t target_msc,
                 int64_t divisor, int64_t remainder, int64_t *ust,
                 int64_t *msc, int64_t *sbc)
{
#ifdef GLX_DIRECT_RENDERING
   __GLXDRIdrawable *pdraw = GetGLXDRIDrawable(dpy, drawable);
   struct glx_screen *psc = pdraw ? pdraw->psc : NULL;
   int ret;
#endif


   /* The OML_sync_control spec says these should "generate a GLX_BAD_VALUE
    * error", but the return type in the spec is Bool.
    */
   if (divisor < 0 || remainder < 0 || target_msc < 0)
      return False;
   if (divisor > 0 && remainder >= divisor)
      return False;

#ifdef GLX_DIRECT_RENDERING
   if (pdraw && psc->driScreen && psc->driScreen->waitForMSC) {
      ret = psc->driScreen->waitForMSC(pdraw, target_msc, divisor, remainder,
				       ust, msc, sbc);
      return ret;
   }
#endif

   return False;
}


_X_HIDDEN Bool
glXWaitForSbcOML(Display *dpy, GLXDrawable drawable, int64_t target_sbc,
                 int64_t *ust, int64_t *msc, int64_t *sbc)
{
#ifdef GLX_DIRECT_RENDERING
   __GLXDRIdrawable *pdraw = GetGLXDRIDrawable(dpy, drawable);
   struct glx_screen *psc = pdraw ? pdraw->psc : NULL;
   int ret;
#endif

   /* The OML_sync_control spec says this should "generate a GLX_BAD_VALUE
    * error", but the return type in the spec is Bool.
    */
   if (target_sbc < 0)
      return False;

#ifdef GLX_DIRECT_RENDERING
   if (pdraw && psc->driScreen && psc->driScreen->waitForSBC) {
      ret = psc->driScreen->waitForSBC(pdraw, target_sbc, ust, msc, sbc);
      return ret;
   }
#endif

   return False;
}

/*@}*/


/**
 * GLX_MESA_copy_sub_buffer
 */
#define X_GLXvop_CopySubBufferMESA 5154 /* temporary */
_X_HIDDEN void
glXCopySubBufferMESA(Display * dpy, GLXDrawable drawable,
                     int x, int y, int width, int height)
{
   xGLXVendorPrivateReq *req;
   struct glx_context *gc;
   GLXContextTag tag;
   CARD32 *drawable_ptr;
   INT32 *x_ptr, *y_ptr, *w_ptr, *h_ptr;
   CARD8 opcode;

#if defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL)
   __GLXDRIdrawable *pdraw = GetGLXDRIDrawable(dpy, drawable);
   if (pdraw != NULL) {
      struct glx_screen *psc = pdraw->psc;
      if (psc->driScreen->copySubBuffer != NULL) {
         psc->driScreen->copySubBuffer(pdraw, x, y, width, height, True);
      }

      return;
   }
#endif

   opcode = __glXSetupForCommand(dpy);
   if (!opcode)
      return;

   /*
    ** The calling thread may or may not have a current context.  If it
    ** does, send the context tag so the server can do a flush.
    */
   gc = __glXGetCurrentContext();
   if ((gc != &dummyContext) && (dpy == gc->currentDpy) &&
       ((drawable == gc->currentDrawable) ||
        (drawable == gc->currentReadable))) {
      tag = gc->currentContextTag;
   }
   else {
      tag = 0;
   }

   LockDisplay(dpy);
   GetReqExtra(GLXVendorPrivate, sizeof(CARD32) + sizeof(INT32) * 4, req);
   req->reqType = opcode;
   req->glxCode = X_GLXVendorPrivate;
   req->vendorCode = X_GLXvop_CopySubBufferMESA;
   req->contextTag = tag;

   drawable_ptr = (CARD32 *) (req + 1);
   x_ptr = (INT32 *) (drawable_ptr + 1);
   y_ptr = (INT32 *) (drawable_ptr + 2);
   w_ptr = (INT32 *) (drawable_ptr + 3);
   h_ptr = (INT32 *) (drawable_ptr + 4);

   *drawable_ptr = drawable;
   *x_ptr = x;
   *y_ptr = y;
   *w_ptr = width;
   *h_ptr = height;

   UnlockDisplay(dpy);
   SyncHandle();
}

/*@{*/
_X_HIDDEN void
glXBindTexImageEXT(Display *dpy, GLXDrawable drawable, int buffer,
                   const int *attrib_list)
{
   xGLXVendorPrivateReq *req;
   struct glx_context *gc = __glXGetCurrentContext();
   CARD32 *drawable_ptr;
   INT32 *buffer_ptr;
   CARD32 *num_attrib_ptr;
   CARD32 *attrib_ptr;
   CARD8 opcode;
   unsigned int i = 0;

#ifdef GLX_DIRECT_RENDERING
   __GLXDRIdrawable *pdraw = GetGLXDRIDrawable(dpy, drawable);
   if (pdraw != NULL) {
      struct glx_screen *psc = pdraw->psc;
      if (psc->driScreen->bindTexImage != NULL)
         psc->driScreen->bindTexImage(pdraw, buffer, attrib_list);

      return;
   }
#endif

   if (attrib_list) {
      while (attrib_list[i * 2] != None)
         i++;
   }

   opcode = __glXSetupForCommand(dpy);
   if (!opcode)
      return;

   LockDisplay(dpy);
   GetReqExtra(GLXVendorPrivate, 12 + 8 * i, req);
   req->reqType = opcode;
   req->glxCode = X_GLXVendorPrivate;
   req->vendorCode = X_GLXvop_BindTexImageEXT;
   req->contextTag = gc->currentContextTag;

   drawable_ptr = (CARD32 *) (req + 1);
   buffer_ptr = (INT32 *) (drawable_ptr + 1);
   num_attrib_ptr = (CARD32 *) (buffer_ptr + 1);
   attrib_ptr = (CARD32 *) (num_attrib_ptr + 1);

   *drawable_ptr = drawable;
   *buffer_ptr = buffer;
   *num_attrib_ptr = (CARD32) i;

   i = 0;
   if (attrib_list) {
      while (attrib_list[i * 2] != None) {
         *attrib_ptr++ = (CARD32) attrib_list[i * 2 + 0];
         *attrib_ptr++ = (CARD32) attrib_list[i * 2 + 1];
         i++;
      }
   }

   UnlockDisplay(dpy);
   SyncHandle();
}

_X_HIDDEN void
glXReleaseTexImageEXT(Display * dpy, GLXDrawable drawable, int buffer)
{
   xGLXVendorPrivateReq *req;
   struct glx_context *gc = __glXGetCurrentContext();
   CARD32 *drawable_ptr;
   INT32 *buffer_ptr;
   CARD8 opcode;

#ifdef GLX_DIRECT_RENDERING
   __GLXDRIdrawable *pdraw = GetGLXDRIDrawable(dpy, drawable);
   if (pdraw != NULL) {
      struct glx_screen *psc = pdraw->psc;
      if (psc->driScreen->releaseTexImage != NULL)
         psc->driScreen->releaseTexImage(pdraw, buffer);

      return;
   }
#endif

   opcode = __glXSetupForCommand(dpy);
   if (!opcode)
      return;

   LockDisplay(dpy);
   GetReqExtra(GLXVendorPrivate, sizeof(CARD32) + sizeof(INT32), req);
   req->reqType = opcode;
   req->glxCode = X_GLXVendorPrivate;
   req->vendorCode = X_GLXvop_ReleaseTexImageEXT;
   req->contextTag = gc->currentContextTag;

   drawable_ptr = (CARD32 *) (req + 1);
   buffer_ptr = (INT32 *) (drawable_ptr + 1);

   *drawable_ptr = drawable;
   *buffer_ptr = buffer;

   UnlockDisplay(dpy);
   SyncHandle();
}

/*@}*/

#endif /* GLX_USE_APPLEGL */

/*
** glXGetProcAddress support
*/

struct name_address_pair
{
   const char *Name;
   GLvoid *Address;
};

#define GLX_FUNCTION(f) { # f, (GLvoid *) f }
#define GLX_FUNCTION2(n,f) { # n, (GLvoid *) f }

static const struct name_address_pair GLX_functions[] = {
   /*** GLX_VERSION_1_0 ***/
   GLX_FUNCTION(glXChooseVisual),
   GLX_FUNCTION(glXCopyContext),
   GLX_FUNCTION(glXCreateContext),
   GLX_FUNCTION(glXCreateGLXPixmap),
   GLX_FUNCTION(glXDestroyContext),
   GLX_FUNCTION(glXDestroyGLXPixmap),
   GLX_FUNCTION(glXGetConfig),
   GLX_FUNCTION(glXGetCurrentContext),
   GLX_FUNCTION(glXGetCurrentDrawable),
   GLX_FUNCTION(glXIsDirect),
   GLX_FUNCTION(glXMakeCurrent),
   GLX_FUNCTION(glXQueryExtension),
   GLX_FUNCTION(glXQueryVersion),
   GLX_FUNCTION(glXSwapBuffers),
   GLX_FUNCTION(glXUseXFont),
   GLX_FUNCTION(glXWaitGL),
   GLX_FUNCTION(glXWaitX),

   /*** GLX_VERSION_1_1 ***/
   GLX_FUNCTION(glXGetClientString),
   GLX_FUNCTION(glXQueryExtensionsString),
   GLX_FUNCTION(glXQueryServerString),

   /*** GLX_VERSION_1_2 ***/
   GLX_FUNCTION(glXGetCurrentDisplay),

   /*** GLX_VERSION_1_3 ***/
   GLX_FUNCTION(glXChooseFBConfig),
   GLX_FUNCTION(glXCreateNewContext),
   GLX_FUNCTION(glXCreatePbuffer),
   GLX_FUNCTION(glXCreatePixmap),
   GLX_FUNCTION(glXCreateWindow),
   GLX_FUNCTION(glXDestroyPbuffer),
   GLX_FUNCTION(glXDestroyPixmap),
   GLX_FUNCTION(glXDestroyWindow),
   GLX_FUNCTION(glXGetCurrentReadDrawable),
   GLX_FUNCTION(glXGetFBConfigAttrib),
   GLX_FUNCTION(glXGetFBConfigs),
   GLX_FUNCTION(glXGetSelectedEvent),
   GLX_FUNCTION(glXGetVisualFromFBConfig),
   GLX_FUNCTION(glXMakeContextCurrent),
   GLX_FUNCTION(glXQueryContext),
   GLX_FUNCTION(glXQueryDrawable),
   GLX_FUNCTION(glXSelectEvent),

   /*** GLX_SGIX_fbconfig ***/
   GLX_FUNCTION2(glXGetFBConfigAttribSGIX, glXGetFBConfigAttrib),
   GLX_FUNCTION2(glXChooseFBConfigSGIX, glXChooseFBConfig),
   GLX_FUNCTION(glXCreateGLXPixmapWithConfigSGIX),
   GLX_FUNCTION(glXCreateContextWithConfigSGIX),
   GLX_FUNCTION2(glXGetVisualFromFBConfigSGIX, glXGetVisualFromFBConfig),
   GLX_FUNCTION(glXGetFBConfigFromVisualSGIX),

   /*** GLX_ARB_get_proc_address ***/
   GLX_FUNCTION(glXGetProcAddressARB),

   /*** GLX 1.4 ***/
   GLX_FUNCTION2(glXGetProcAddress, glXGetProcAddressARB),

#ifndef GLX_USE_APPLEGL
   /*** GLX_SGI_swap_control ***/
   GLX_FUNCTION(glXSwapIntervalSGI),

   /*** GLX_SGI_video_sync ***/
   GLX_FUNCTION(glXGetVideoSyncSGI),
   GLX_FUNCTION(glXWaitVideoSyncSGI),

   /*** GLX_SGI_make_current_read ***/
   GLX_FUNCTION2(glXMakeCurrentReadSGI, glXMakeContextCurrent),
   GLX_FUNCTION2(glXGetCurrentReadDrawableSGI, glXGetCurrentReadDrawable),

   /*** GLX_EXT_import_context ***/
   GLX_FUNCTION(glXFreeContextEXT),
   GLX_FUNCTION(glXGetContextIDEXT),
   GLX_FUNCTION2(glXGetCurrentDisplayEXT, glXGetCurrentDisplay),
   GLX_FUNCTION(glXImportContextEXT),
   GLX_FUNCTION2(glXQueryContextInfoEXT, glXQueryContext),

   /*** GLX_SGIX_pbuffer ***/
   GLX_FUNCTION(glXCreateGLXPbufferSGIX),
   GLX_FUNCTION(glXDestroyGLXPbufferSGIX),
   GLX_FUNCTION(glXQueryGLXPbufferSGIX),
   GLX_FUNCTION(glXSelectEventSGIX),
   GLX_FUNCTION(glXGetSelectedEventSGIX),

   /*** GLX_MESA_copy_sub_buffer ***/
   GLX_FUNCTION(glXCopySubBufferMESA),

   /*** GLX_MESA_swap_control ***/
   GLX_FUNCTION(glXSwapIntervalMESA),
   GLX_FUNCTION(glXGetSwapIntervalMESA),

   /*** GLX_OML_sync_control ***/
   GLX_FUNCTION(glXWaitForSbcOML),
   GLX_FUNCTION(glXWaitForMscOML),
   GLX_FUNCTION(glXSwapBuffersMscOML),
   GLX_FUNCTION(glXGetMscRateOML),
   GLX_FUNCTION(glXGetSyncValuesOML),

   /*** GLX_EXT_texture_from_pixmap ***/
   GLX_FUNCTION(glXBindTexImageEXT),
   GLX_FUNCTION(glXReleaseTexImageEXT),

   /*** GLX_EXT_swap_control ***/
   GLX_FUNCTION(glXSwapIntervalEXT),
#endif

#if defined(GLX_DIRECT_RENDERING) && defined(GLX_USE_DRM)
   /*** DRI configuration ***/
   GLX_FUNCTION(glXGetScreenDriver),
   GLX_FUNCTION(glXGetDriverConfig),
#endif

   /*** GLX_ARB_create_context and GLX_ARB_create_context_profile ***/
   GLX_FUNCTION(glXCreateContextAttribsARB),

   /*** GLX_MESA_query_renderer ***/
   GLX_FUNCTION(glXQueryRendererIntegerMESA),
   GLX_FUNCTION(glXQueryRendererStringMESA),
   GLX_FUNCTION(glXQueryCurrentRendererIntegerMESA),
   GLX_FUNCTION(glXQueryCurrentRendererStringMESA),

   /*** GLX_MESA_gl_interop ***/
#if defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL)
   GLX_FUNCTION2(glXGLInteropQueryDeviceInfoMESA, MesaGLInteropGLXQueryDeviceInfo),
   GLX_FUNCTION2(glXGLInteropExportObjectMESA, MesaGLInteropGLXExportObject),
   GLX_FUNCTION2(glXGLInteropFlushObjectsMESA, MesaGLInteropGLXFlushObjects),
#endif

   {NULL, NULL}                 /* end of list */
};

static const GLvoid *
get_glx_proc_address(const char *funcName)
{
   GLuint i;

   /* try static functions */
   for (i = 0; GLX_functions[i].Name; i++) {
      if (strcmp(GLX_functions[i].Name, funcName) == 0)
         return GLX_functions[i].Address;
   }

   return NULL;
}

/**
 * Get the address of a named GL function.  This is the pre-GLX 1.4 name for
 * \c glXGetProcAddress.
 *
 * \param procName  Name of a GL or GLX function.
 * \returns         A pointer to the named function
 *
 * \sa glXGetProcAddress
 */
_GLX_PUBLIC void (*glXGetProcAddressARB(const GLubyte * procName)) (void)
{
   typedef void (*gl_function) (void);
   gl_function f = NULL;

   if (!strncmp((const char *) procName, "glX", 3))
      f = (gl_function) get_glx_proc_address((const char *) procName);

   if (f == NULL)
      f = (gl_function) _glapi_get_proc_address((const char *) procName);

#ifdef GLX_USE_APPLEGL
   if (f == NULL)
      f = applegl_get_proc_address((const char *) procName);
#endif

   return f;
}

/**
 * Get the address of a named GL function.  This is the GLX 1.4 name for
 * \c glXGetProcAddressARB.
 *
 * \param procName  Name of a GL or GLX function.
 * \returns         A pointer to the named function
 *
 * \sa glXGetProcAddressARB
 */
_GLX_PUBLIC
GLX_ALIAS(__GLXextFuncPtr, glXGetProcAddress,
          (const GLubyte * procName),
          (procName), glXGetProcAddressARB)

#if defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL)

PUBLIC int
MesaGLInteropGLXQueryDeviceInfo(Display *dpy, GLXContext context,
                                struct mesa_glinterop_device_info *out)
{
   struct glx_context *gc = (struct glx_context*)context;
   int ret;

   __glXLock();

   if (!gc || gc->xid == None || !gc->isDirect) {
      __glXUnlock();
      return MESA_GLINTEROP_INVALID_CONTEXT;
   }

   if (!gc->vtable->interop_query_device_info) {
      __glXUnlock();
      return MESA_GLINTEROP_UNSUPPORTED;
   }

   ret = gc->vtable->interop_query_device_info(gc, out);
   __glXUnlock();
   return ret;
}

PUBLIC int
MesaGLInteropGLXExportObject(Display *dpy, GLXContext context,
                             struct mesa_glinterop_export_in *in,
                             struct mesa_glinterop_export_out *out)
{
   struct glx_context *gc = (struct glx_context*)context;
   int ret;

   __glXLock();

   if (!gc || gc->xid == None || !gc->isDirect) {
      __glXUnlock();
      return MESA_GLINTEROP_INVALID_CONTEXT;
   }

   if (!gc->vtable->interop_export_object) {
      __glXUnlock();
      return MESA_GLINTEROP_UNSUPPORTED;
   }

   ret = gc->vtable->interop_export_object(gc, in, out);
   __glXUnlock();
   return ret;
}

PUBLIC int
MesaGLInteropGLXFlushObjects(Display *dpy, GLXContext context,
                             unsigned count,
                             struct mesa_glinterop_export_in *resources,
                             struct mesa_glinterop_flush_out *out)
{
   struct glx_context *gc = (struct glx_context*)context;
   int ret;

   __glXLock();

   if (!gc || gc->xid == None || !gc->isDirect) {
      __glXUnlock();
      return MESA_GLINTEROP_INVALID_CONTEXT;
   }

   if (!gc->vtable->interop_flush_objects) {
      __glXUnlock();
      return MESA_GLINTEROP_UNSUPPORTED;
   }

   ret = gc->vtable->interop_flush_objects(gc, count, resources, out);
   __glXUnlock();
   return ret;
}

#endif /* defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL) */
