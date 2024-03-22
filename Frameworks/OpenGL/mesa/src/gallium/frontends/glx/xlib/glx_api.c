/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2007  Brian Paul   All Rights Reserved.
 * Copyright (C) 2009  VMware, Inc.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */


/**
 * "Fake" GLX API implemented in terms of the XMesa*() functions.
 */



#define GLX_GLXEXT_PROTOTYPES
#include "GL/glx.h"

#include <stdio.h>
#include <string.h>
#include <X11/Xmd.h>
#include <GL/glxproto.h>

#include "xm_api.h"
#include "main/errors.h"
#include "main/config.h"
#include "util/compiler.h"
#include "util/u_math.h"
#include "util/u_memory.h"

/* An "Atrribs/Attribs" typo was fixed in glxproto.h in Nov 2014.
 * This is in case we don't have the updated header.
 */
#if !defined(X_GLXCreateContextAttribsARB) && \
     defined(X_GLXCreateContextAtrribsARB)
#define X_GLXCreateContextAttribsARB X_GLXCreateContextAtrribsARB
#endif

/* This indicates the client-side GLX API and GLX encoder version. */
#define CLIENT_MAJOR_VERSION 1
#define CLIENT_MINOR_VERSION 4  /* but don't have 1.3's pbuffers, etc yet */

/* This indicates the server-side GLX decoder version.
 * GLX 1.4 indicates OpenGL 1.3 support
 */
#define SERVER_MAJOR_VERSION 1
#define SERVER_MINOR_VERSION 4

/* Who implemented this GLX? */
#define VENDOR "Brian Paul"

#define EXTENSIONS \
   "GLX_MESA_copy_sub_buffer " \
   "GLX_MESA_pixmap_colormap " \
   "GLX_MESA_release_buffers " \
   "GLX_ARB_create_context " \
   "GLX_ARB_create_context_profile " \
   "GLX_ARB_get_proc_address " \
   "GLX_EXT_create_context_es_profile " \
   "GLX_EXT_create_context_es2_profile " \
   "GLX_EXT_texture_from_pixmap " \
   "GLX_EXT_visual_info " \
   "GLX_EXT_visual_rating " \
   /*"GLX_SGI_video_sync "*/ \
   "GLX_SGIX_fbconfig " \
   "GLX_SGIX_pbuffer "

#define DEFAULT_DIRECT GL_TRUE


/** XXX this could be based on gallium's max texture size */
#define PBUFFER_MAX_SIZE 16384


/**
 * The GLXContext typedef is defined as a pointer to this structure.
 */
struct __GLXcontextRec
{
   Display *currentDpy;
   GLboolean isDirect;
   GLXDrawable currentDrawable;
   GLXDrawable currentReadable;
   XID xid;

   XMesaContext xmesaContext;
};


thread_local GLXContext ContextTSD;

/** Set current context for calling thread */
static void
SetCurrentContext(GLXContext c)
{
   ContextTSD = c;
}

/** Get current context for calling thread */
static GLXContext
GetCurrentContext(void)
{
   return ContextTSD;
}



/**********************************************************************/
/***                       GLX Visual Code                          ***/
/**********************************************************************/

#define DONT_CARE -1


static XMesaVisual *VisualTable = NULL;
static int NumVisuals = 0;



/* Macro to handle c_class vs class field name in XVisualInfo struct */
#if defined(__cplusplus) || defined(c_plusplus)
#define CLASS c_class
#else
#define CLASS class
#endif



/*
 * Test if the given XVisualInfo is usable for Mesa rendering.
 */
static GLboolean
is_usable_visual( XVisualInfo *vinfo )
{
   switch (vinfo->CLASS) {
      case StaticGray:
      case GrayScale:
         /* Any StaticGray/GrayScale visual works in RGB or CI mode */
         return GL_TRUE;
      case StaticColor:
      case PseudoColor:
	 /* Any StaticColor/PseudoColor visual of at least 4 bits */
	 if (vinfo->depth>=4) {
	    return GL_TRUE;
	 }
	 else {
	    return GL_FALSE;
	 }
      case TrueColor:
      case DirectColor:
	 /* Any depth of TrueColor or DirectColor works in RGB mode */
	 return GL_TRUE;
      default:
	 /* This should never happen */
	 return GL_FALSE;
   }
}


/*
 * Given an XVisualInfo and RGB, Double, and Depth buffer flags, save the
 * configuration in our list of GLX visuals.
 */
static XMesaVisual
save_glx_visual( Display *dpy, XVisualInfo *vinfo,
                 GLboolean rgbFlag, GLboolean alphaFlag, GLboolean dbFlag,
                 GLboolean stereoFlag,
                 GLint depth_size, GLint stencil_size,
                 GLint accumRedSize, GLint accumGreenSize,
                 GLint accumBlueSize, GLint accumAlphaSize,
                 GLint level, GLint numAuxBuffers, GLuint num_samples )
{
   GLboolean ximageFlag = GL_TRUE;
   XMesaVisual xmvis;
   GLint i;
   GLboolean comparePointers;

   if (!rgbFlag)
      return NULL;

   if (dbFlag) {
      /* Check if the MESA_BACK_BUFFER env var is set */
      char *backbuffer = getenv("MESA_BACK_BUFFER");
      if (backbuffer) {
         if (backbuffer[0]=='p' || backbuffer[0]=='P') {
            ximageFlag = GL_FALSE;
         }
         else if (backbuffer[0]=='x' || backbuffer[0]=='X') {
            ximageFlag = GL_TRUE;
         }
         else {
            _mesa_warning(NULL, "Mesa: invalid value for MESA_BACK_BUFFER environment variable, using an XImage.");
         }
      }
   }

   if (stereoFlag) {
      /* stereo not supported */
      return NULL;
   }

   if (stencil_size > 0 && depth_size > 0)
      depth_size = 24;

   /* Comparing IDs uses less memory but sometimes fails. */
   /* XXX revisit this after 3.0 is finished. */
   if (getenv("MESA_GLX_VISUAL_HACK"))
      comparePointers = GL_TRUE;
   else
      comparePointers = GL_FALSE;

   /* Force the visual to have an alpha channel */
   if (rgbFlag && getenv("MESA_GLX_FORCE_ALPHA"))
      alphaFlag = GL_TRUE;

   /* First check if a matching visual is already in the list */
   for (i=0; i<NumVisuals; i++) {
      XMesaVisual v = VisualTable[i];
      if (v->display == dpy
          && v->mesa_visual.samples == num_samples
          && v->ximage_flag == ximageFlag
          && v->mesa_visual.doubleBufferMode == dbFlag
          && v->mesa_visual.stereoMode == stereoFlag
          && (v->mesa_visual.alphaBits > 0) == alphaFlag
          && (v->mesa_visual.depthBits >= depth_size || depth_size == 0)
          && (v->mesa_visual.stencilBits >= stencil_size || stencil_size == 0)
          && (v->mesa_visual.accumRedBits >= accumRedSize || accumRedSize == 0)
          && (v->mesa_visual.accumGreenBits >= accumGreenSize || accumGreenSize == 0)
          && (v->mesa_visual.accumBlueBits >= accumBlueSize || accumBlueSize == 0)
          && (v->mesa_visual.accumAlphaBits >= accumAlphaSize || accumAlphaSize == 0)) {
         /* now either compare XVisualInfo pointers or visual IDs */
         if ((!comparePointers && v->visinfo->visualid == vinfo->visualid)
             || (comparePointers && v->vishandle == vinfo)) {
            return v;
         }
      }
   }

   /* Create a new visual and add it to the list. */

   xmvis = XMesaCreateVisual( dpy, vinfo, rgbFlag, alphaFlag, dbFlag,
                              stereoFlag, ximageFlag,
                              depth_size, stencil_size,
                              accumRedSize, accumBlueSize,
                              accumBlueSize, accumAlphaSize, num_samples, level,
                              GLX_NONE_EXT );
   if (xmvis) {
      /* Save a copy of the pointer now so we can find this visual again
       * if we need to search for it in find_glx_visual().
       */
      xmvis->vishandle = vinfo;
      /* Allocate more space for additional visual */
      VisualTable = realloc(VisualTable, sizeof(XMesaVisual) * (NumVisuals + 1));
      /* add xmvis to the list */
      VisualTable[NumVisuals] = xmvis;
      NumVisuals++;
   }
   return xmvis;
}


/**
 * Return the default number of bits for the Z buffer.
 * If defined, use the MESA_GLX_DEPTH_BITS env var value.
 * Otherwise, use 24.
 * XXX probably do the same thing for stencil, accum, etc.
 */
static GLint
default_depth_bits(void)
{
   int zBits;
   const char *zEnv = getenv("MESA_GLX_DEPTH_BITS");
   if (zEnv)
      zBits = atoi(zEnv);
   else
      zBits = 24;
   return zBits;
}

static GLint
default_alpha_bits(void)
{
   int aBits;
   const char *aEnv = getenv("MESA_GLX_ALPHA_BITS");
   if (aEnv)
      aBits = atoi(aEnv);
   else
      aBits = 0;
   return aBits;
}

static GLint
default_accum_bits(void)
{
   return 16;
}



/*
 * Create a GLX visual from a regular XVisualInfo.
 * This is called when Fake GLX is given an XVisualInfo which wasn't
 * returned by glXChooseVisual.  Since this is the first time we're
 * considering this visual we'll take a guess at reasonable values
 * for depth buffer size, stencil size, accum size, etc.
 * This is the best we can do with a client-side emulation of GLX.
 */
static XMesaVisual
create_glx_visual( Display *dpy, XVisualInfo *visinfo )
{
   GLint zBits = default_depth_bits();
   GLint accBits = default_accum_bits();
   GLboolean alphaFlag = default_alpha_bits() > 0;

   if (is_usable_visual( visinfo )) {
      /* Configure this visual as RGB, double-buffered, depth-buffered. */
      /* This is surely wrong for some people's needs but what else */
      /* can be done?  They should use glXChooseVisual(). */
      return save_glx_visual( dpy, visinfo,
                              GL_TRUE,   /* rgb */
                              alphaFlag, /* alpha */
                              GL_TRUE,   /* double */
                              GL_FALSE,  /* stereo */
                              zBits,
                              8,       /* stencil bits */
                              accBits, /* r */
                              accBits, /* g */
                              accBits, /* b */
                              accBits, /* a */
                              0,         /* level */
                              0,         /* numAux */
                              0          /* numSamples */
         );
   }
   else {
      _mesa_warning(NULL, "Mesa: error in glXCreateContext: bad visual\n");
      return NULL;
   }
}



/*
 * Find the GLX visual associated with an XVisualInfo.
 */
static XMesaVisual
find_glx_visual( Display *dpy, XVisualInfo *vinfo )
{
   int i;

   /* try to match visual id */
   for (i=0;i<NumVisuals;i++) {
      if (VisualTable[i]->display==dpy
          && VisualTable[i]->visinfo->visualid == vinfo->visualid) {
         return VisualTable[i];
      }
   }

   /* if that fails, try to match pointers */
   for (i=0;i<NumVisuals;i++) {
      if (VisualTable[i]->display==dpy && VisualTable[i]->vishandle==vinfo) {
         return VisualTable[i];
      }
   }

   return NULL;
}


/**
 * Try to get an X visual which matches the given arguments.
 */
static XVisualInfo *
get_visual( Display *dpy, int scr, unsigned int depth, int xclass )
{
   XVisualInfo temp, *vis;
   long mask;
   int n;
   unsigned int default_depth;
   int default_class;

   mask = VisualScreenMask | VisualDepthMask | VisualClassMask;
   temp.screen = scr;
   temp.depth = depth;
   temp.CLASS = xclass;

   default_depth = DefaultDepth(dpy,scr);
   default_class = DefaultVisual(dpy,scr)->CLASS;

   if (depth==default_depth && xclass==default_class) {
      /* try to get root window's visual */
      temp.visualid = DefaultVisual(dpy,scr)->visualid;
      mask |= VisualIDMask;
   }

   vis = XGetVisualInfo( dpy, mask, &temp, &n );

   /* In case bits/pixel > 24, make sure color channels are still <=8 bits.
    * An SGI Infinite Reality system, for example, can have 30bpp pixels:
    * 10 bits per color channel.  Mesa's limited to a max of 8 bits/channel.
    */
   if (vis && depth > 24 && (xclass==TrueColor || xclass==DirectColor)) {
      if (util_bitcount((GLuint) vis->red_mask  ) <= 8 &&
          util_bitcount((GLuint) vis->green_mask) <= 8 &&
          util_bitcount((GLuint) vis->blue_mask ) <= 8) {
         return vis;
      }
      else {
         XFree((void *) vis);
         return NULL;
      }
   }

   return vis;
}


/*
 * Retrieve the value of the given environment variable and find
 * the X visual which matches it.
 * Input:  dpy - the display
 *         screen - the screen number
 *         varname - the name of the environment variable
 * Return:  an XVisualInfo pointer to NULL if error.
 */
static XVisualInfo *
get_env_visual(Display *dpy, int scr, const char *varname)
{
   char value[100], type[100];
   int depth, xclass = -1;
   XVisualInfo *vis;

   if (!getenv( varname )) {
      return NULL;
   }

   strncpy( value, getenv(varname), 100 );
   value[99] = 0;

   sscanf( value, "%s %d", type, &depth );

   if (strcmp(type,"TrueColor")==0)          xclass = TrueColor;
   else if (strcmp(type,"DirectColor")==0)   xclass = DirectColor;
   else if (strcmp(type,"PseudoColor")==0)   xclass = PseudoColor;
   else if (strcmp(type,"StaticColor")==0)   xclass = StaticColor;
   else if (strcmp(type,"GrayScale")==0)     xclass = GrayScale;
   else if (strcmp(type,"StaticGray")==0)    xclass = StaticGray;

   if (xclass>-1 && depth>0) {
      vis = get_visual( dpy, scr, depth, xclass );
      if (vis) {
	 return vis;
      }
   }

   _mesa_warning(NULL, "GLX unable to find visual class=%s, depth=%d.",
                 type, depth);

   return NULL;
}



/*
 * Select an X visual which satisfies the RGBA flag and minimum depth.
 * Input:  dpy,
 *         screen - X display and screen number
 *         min_depth - minimum visual depth
 *         preferred_class - preferred GLX visual class or DONT_CARE
 * Return:  pointer to an XVisualInfo or NULL.
 */
static XVisualInfo *
choose_x_visual( Display *dpy, int screen, int min_depth,
                 int preferred_class )
{
   XVisualInfo *vis;
   int xclass, visclass = 0;
   int depth;

   /* First see if the MESA_RGB_VISUAL env var is defined */
   vis = get_env_visual( dpy, screen, "MESA_RGB_VISUAL" );
   if (vis) {
      return vis;
   }
   /* Otherwise, search for a suitable visual */
   if (preferred_class==DONT_CARE) {
      for (xclass=0;xclass<6;xclass++) {
         switch (xclass) {
         case 0:  visclass = TrueColor;    break;
         case 1:  visclass = DirectColor;  break;
         case 2:  visclass = PseudoColor;  break;
         case 3:  visclass = StaticColor;  break;
         case 4:  visclass = GrayScale;    break;
         case 5:  visclass = StaticGray;   break;
         }
         if (min_depth==0) {
            /* start with shallowest */
            for (depth=0;depth<=32;depth++) {
               if (visclass==TrueColor && depth==8) {
                  /* Special case:  try to get 8-bit PseudoColor before */
                  /* 8-bit TrueColor */
                  vis = get_visual( dpy, screen, 8, PseudoColor );
                  if (vis) {
                     return vis;
                  }
               }
               vis = get_visual( dpy, screen, depth, visclass );
               if (vis) {
                  return vis;
               }
            }
         }
         else {
            /* start with deepest */
            for (depth=32;depth>=min_depth;depth--) {
               if (visclass==TrueColor && depth==8) {
                  /* Special case:  try to get 8-bit PseudoColor before */
                  /* 8-bit TrueColor */
                  vis = get_visual( dpy, screen, 8, PseudoColor );
                  if (vis) {
                     return vis;
                  }
               }
               vis = get_visual( dpy, screen, depth, visclass );
               if (vis) {
                  return vis;
               }
            }
         }
      }
   }
   else {
      /* search for a specific visual class */
      switch (preferred_class) {
      case GLX_TRUE_COLOR_EXT:    visclass = TrueColor;    break;
      case GLX_DIRECT_COLOR_EXT:  visclass = DirectColor;  break;
      case GLX_PSEUDO_COLOR_EXT:  visclass = PseudoColor;  break;
      case GLX_STATIC_COLOR_EXT:  visclass = StaticColor;  break;
      case GLX_GRAY_SCALE_EXT:    visclass = GrayScale;    break;
      case GLX_STATIC_GRAY_EXT:   visclass = StaticGray;   break;
      default:   return NULL;
      }
      if (min_depth==0) {
         /* start with shallowest */
         for (depth=0;depth<=32;depth++) {
            vis = get_visual( dpy, screen, depth, visclass );
            if (vis) {
               return vis;
            }
         }
      }
      else {
         /* start with deepest */
         for (depth=32;depth>=min_depth;depth--) {
            vis = get_visual( dpy, screen, depth, visclass );
            if (vis) {
               return vis;
            }
         }
      }
   }

   /* didn't find a visual */
   return NULL;
}




/**********************************************************************/
/***             Display-related functions                          ***/
/**********************************************************************/


/**
 * Free all XMesaVisuals which are associated with the given display.
 */
static void
destroy_visuals_on_display(Display *dpy)
{
   int i;
   for (i = 0; i < NumVisuals; i++) {
      if (VisualTable[i]->display == dpy) {
         /* remove this visual */
         int j;
         free(VisualTable[i]);
         for (j = i; j < NumVisuals - 1; j++)
            VisualTable[j] = VisualTable[j + 1];
         NumVisuals--;
      }
   }
}


/**
 * Called from XCloseDisplay() to let us free our display-related data.
 */
static int
close_display_callback(Display *dpy, XExtCodes *codes)
{
   xmesa_destroy_buffers_on_display(dpy);
   destroy_visuals_on_display(dpy);
   xmesa_close_display(dpy);
   return 0;
}


/**
 * Look for the named extension on given display and return a pointer
 * to the _XExtension data, or NULL if extension not found.
 */
static _XExtension *
lookup_extension(Display *dpy, const char *extName)
{
   _XExtension *ext;
   for (ext = dpy->ext_procs; ext; ext = ext->next) {
      if (ext->name && strcmp(ext->name, extName) == 0) {
         return ext;
      }
   }
   return NULL;
}


/**
 * Whenever we're given a new Display pointer, call this function to
 * register our close_display_callback function.
 */
static void
register_with_display(Display *dpy)
{
   const char *extName = "MesaGLX";
   _XExtension *ext;

   ext = lookup_extension(dpy, extName);
   if (!ext) {
      XExtCodes *c = XAddExtension(dpy);
      ext = dpy->ext_procs;  /* new extension is at head of list */
      assert(c->extension == ext->codes.extension);
      (void) c;
      ext->name = strdup(extName);
      ext->close_display = close_display_callback;
   }
}


/**
 * Fake an error.
 */
static int
generate_error(Display *dpy,
               unsigned char error_code,
               XID resourceid,
               unsigned char minor_code,
               Bool core)
{
   XErrorHandler handler;
   int major_opcode;
   int first_event;
   int first_error;
   XEvent event;

   handler = XSetErrorHandler(NULL);
   XSetErrorHandler(handler);
   if (!handler) {
      return 0;
   }

   if (!XQueryExtension(dpy, GLX_EXTENSION_NAME, &major_opcode, &first_event, &first_error)) {
      major_opcode = 0;
      first_event = 0;
      first_error = 0;
   }

   if (!core) {
      error_code += first_error;
   }

   memset(&event, 0, sizeof event);

   event.xerror.type = X_Error;
   event.xerror.display = dpy;
   event.xerror.resourceid = resourceid;
   event.xerror.serial = NextRequest(dpy) - 1;
   event.xerror.error_code = error_code;
   event.xerror.request_code = major_opcode;
   event.xerror.minor_code = minor_code;

   return handler(dpy, &event.xerror);
}


/**********************************************************************/
/***                  Begin Fake GLX API Functions                  ***/
/**********************************************************************/


/**
 * Helper used by glXChooseVisual and glXChooseFBConfig.
 * The fbConfig parameter must be GL_FALSE for the former and GL_TRUE for
 * the later.
 * In either case, the attribute list is terminated with the value 'None'.
 */
static XMesaVisual
choose_visual( Display *dpy, int screen, const int *list, GLboolean fbConfig )
{
   const GLboolean rgbModeDefault = fbConfig;
   const int *parselist;
   XVisualInfo *vis;
   int min_red=0, min_green=0, min_blue=0;
   GLboolean rgb_flag = rgbModeDefault;
   GLboolean alpha_flag = GL_FALSE;
   GLboolean double_flag = GL_FALSE;
   GLboolean stereo_flag = GL_FALSE;
   GLint depth_size = 0;
   GLint stencil_size = 0;
   GLint accumRedSize = 0;
   GLint accumGreenSize = 0;
   GLint accumBlueSize = 0;
   GLint accumAlphaSize = 0;
   int level = 0;
   int visual_type = DONT_CARE;
   GLint caveat = DONT_CARE;
   XMesaVisual xmvis = NULL;
   int desiredVisualID = -1;
   int numAux = 0;
   GLint num_samples = 0;

   if (xmesa_init( dpy ) != 0) {
      _mesa_warning(NULL, "Failed to initialize display");
      return NULL;
   }

   parselist = list;

   while (*parselist) {

      if (fbConfig &&
          parselist[1] == GLX_DONT_CARE &&
          parselist[0] != GLX_LEVEL) {
         /* For glXChooseFBConfig(), skip attributes whose value is
          * GLX_DONT_CARE, unless it's GLX_LEVEL (which can legitimately be
          * a negative value).
          *
          * From page 17 (23 of the pdf) of the GLX 1.4 spec:
          * GLX DONT CARE may be specified for all attributes except GLX LEVEL.
          */
         parselist += 2;
         continue;
      }

      switch (*parselist) {
	 case GLX_USE_GL:
            if (fbConfig) {
               /* invalid token */
               return NULL;
            }
            else {
               /* skip */
               parselist++;
            }
	    break;
	 case GLX_BUFFER_SIZE:
	    parselist++;
	    parselist++;
	    break;
	 case GLX_LEVEL:
	    parselist++;
            level = *parselist++;
	    break;
	 case GLX_RGBA:
            if (fbConfig) {
               /* invalid token */
               return NULL;
            }
            else {
               rgb_flag = GL_TRUE;
               parselist++;
            }
	    break;
	 case GLX_DOUBLEBUFFER:
            parselist++;
            if (fbConfig) {
               double_flag = *parselist++;
            }
            else {
               double_flag = GL_TRUE;
            }
	    break;
	 case GLX_STEREO:
            parselist++;
            if (fbConfig) {
               stereo_flag = *parselist++;
            }
            else {
               stereo_flag = GL_TRUE;
            }
            break;
	 case GLX_AUX_BUFFERS:
	    parselist++;
            numAux = *parselist++;
            if (numAux > MAX_AUX_BUFFERS)
               return NULL;
	    break;
	 case GLX_RED_SIZE:
	    parselist++;
	    min_red = *parselist++;
	    break;
	 case GLX_GREEN_SIZE:
	    parselist++;
	    min_green = *parselist++;
	    break;
	 case GLX_BLUE_SIZE:
	    parselist++;
	    min_blue = *parselist++;
	    break;
	 case GLX_ALPHA_SIZE:
	    parselist++;
            {
               GLint size = *parselist++;
               alpha_flag = size ? GL_TRUE : GL_FALSE;
            }
	    break;
	 case GLX_DEPTH_SIZE:
	    parselist++;
	    depth_size = *parselist++;
	    break;
	 case GLX_STENCIL_SIZE:
	    parselist++;
	    stencil_size = *parselist++;
	    break;
	 case GLX_ACCUM_RED_SIZE:
	    parselist++;
            {
               GLint size = *parselist++;
               accumRedSize = MAX2( accumRedSize, size );
            }
            break;
	 case GLX_ACCUM_GREEN_SIZE:
	    parselist++;
            {
               GLint size = *parselist++;
               accumGreenSize = MAX2( accumGreenSize, size );
            }
            break;
	 case GLX_ACCUM_BLUE_SIZE:
	    parselist++;
            {
               GLint size = *parselist++;
               accumBlueSize = MAX2( accumBlueSize, size );
            }
            break;
	 case GLX_ACCUM_ALPHA_SIZE:
	    parselist++;
            {
               GLint size = *parselist++;
               accumAlphaSize = MAX2( accumAlphaSize, size );
            }
	    break;

         /*
          * GLX_EXT_visual_info extension
          */
         case GLX_X_VISUAL_TYPE_EXT:
            parselist++;
            visual_type = *parselist++;
            break;
         case GLX_TRANSPARENT_TYPE_EXT:
            parselist++;
            parselist++;
            break;
         case GLX_TRANSPARENT_INDEX_VALUE_EXT:
            parselist++;
            parselist++;
            break;
         case GLX_TRANSPARENT_RED_VALUE_EXT:
         case GLX_TRANSPARENT_GREEN_VALUE_EXT:
         case GLX_TRANSPARENT_BLUE_VALUE_EXT:
         case GLX_TRANSPARENT_ALPHA_VALUE_EXT:
	    /* ignore */
	    parselist++;
	    parselist++;
	    break;

         /*
          * GLX_EXT_visual_info extension
          */
         case GLX_VISUAL_CAVEAT_EXT:
            parselist++;
            caveat = *parselist++; /* ignored for now */
            break;

         /*
          * GLX_ARB_multisample
          */
         case GLX_SAMPLE_BUFFERS_ARB:
            /* ignore */
            parselist++;
            parselist++;
            break;
         case GLX_SAMPLES_ARB:
            parselist++;
            num_samples = *parselist++;
            break;

         /*
          * FBConfig attribs.
          */
         case GLX_RENDER_TYPE:
            if (!fbConfig)
               return NULL;
            parselist++;
            if (*parselist & GLX_RGBA_BIT) {
               rgb_flag = GL_TRUE;
            }
            else if (*parselist & GLX_COLOR_INDEX_BIT) {
               rgb_flag = GL_FALSE;
            }
            else if (*parselist == 0) {
               rgb_flag = GL_TRUE;
            }
            parselist++;
            break;
         case GLX_DRAWABLE_TYPE:
            if (!fbConfig)
               return NULL;
            parselist++;
            if (*parselist & ~(GLX_WINDOW_BIT | GLX_PIXMAP_BIT | GLX_PBUFFER_BIT)) {
               return NULL; /* bad bit */
            }
            parselist++;
            break;
         case GLX_FBCONFIG_ID:
         case GLX_VISUAL_ID:
            if (!fbConfig)
               return NULL;
            parselist++;
            desiredVisualID = *parselist++;
            break;
         case GLX_X_RENDERABLE:
         case GLX_MAX_PBUFFER_WIDTH:
         case GLX_MAX_PBUFFER_HEIGHT:
         case GLX_MAX_PBUFFER_PIXELS:
            if (!fbConfig)
               return NULL; /* invalid config option */
            parselist += 2; /* ignore the parameter */
            break;

         case GLX_BIND_TO_TEXTURE_RGB_EXT:
            parselist++; /*skip*/
            break;
         case GLX_BIND_TO_TEXTURE_RGBA_EXT:
            parselist++; /*skip*/
            break;
         case GLX_BIND_TO_MIPMAP_TEXTURE_EXT:
            parselist++; /*skip*/
            break;
         case GLX_BIND_TO_TEXTURE_TARGETS_EXT:
            parselist++;
            if (*parselist & ~(GLX_TEXTURE_1D_BIT_EXT |
                               GLX_TEXTURE_2D_BIT_EXT |
                               GLX_TEXTURE_RECTANGLE_BIT_EXT)) {
               /* invalid bit */
               return NULL;
            }
            break;
         case GLX_Y_INVERTED_EXT:
            parselist++; /*skip*/
            break;

	 case None:
            /* end of list */
	    break;

	 default:
	    /* undefined attribute */
            _mesa_warning(NULL, "unexpected attrib 0x%x in choose_visual()",
                          *parselist);
	    return NULL;
      }
   }

   (void) caveat;

   if (num_samples < 0) {
      _mesa_warning(NULL, "GLX_SAMPLES_ARB: number of samples must not be negative");
      return NULL;
   }

   /*
    * Since we're only simulating the GLX extension this function will never
    * find any real GL visuals.  Instead, all we can do is try to find an RGB
    * or CI visual of appropriate depth.  Other requested attributes such as
    * double buffering, depth buffer, etc. will be associated with the X
    * visual and stored in the VisualTable[].
    */
   if (desiredVisualID != -1) {
      /* try to get a specific visual, by visualID */
      XVisualInfo temp;
      int n;
      temp.visualid = desiredVisualID;
      temp.screen = screen;
      vis = XGetVisualInfo(dpy, VisualIDMask | VisualScreenMask, &temp, &n);
      if (vis) {
         /* give the visual some useful GLX attributes */
         double_flag = GL_TRUE;
         rgb_flag = GL_TRUE;
      }
   }
   else if (level==0) {
      /* normal color planes */
      /* Get an RGB visual */
      int min_rgb = min_red + min_green + min_blue;
      if (min_rgb>1 && min_rgb<8) {
         /* a special case to be sure we can get a monochrome visual */
         min_rgb = 1;
      }
      vis = choose_x_visual( dpy, screen, min_rgb, visual_type );
   }
   else {
      _mesa_warning(NULL, "overlay not supported");
      return NULL;
   }

   if (vis) {
      /* Note: we're not exactly obeying the glXChooseVisual rules here.
       * When GLX_DEPTH_SIZE = 1 is specified we're supposed to choose the
       * largest depth buffer size, which is 32bits/value.  Instead, we
       * return 16 to maintain performance with earlier versions of Mesa.
       */
      if (stencil_size > 0)
         depth_size = 24;  /* if Z and stencil, always use 24+8 format */
      else if (depth_size > 24)
         depth_size = 32;
      else if (depth_size > 16)
         depth_size = 24;
      else if (depth_size > 0) {
         depth_size = default_depth_bits();
      }

      if (!alpha_flag) {
         alpha_flag = default_alpha_bits() > 0;
      }

      /* we only support one size of stencil and accum buffers. */
      if (stencil_size > 0)
         stencil_size = 8;

      if (accumRedSize > 0 ||
          accumGreenSize > 0 ||
          accumBlueSize > 0 ||
          accumAlphaSize > 0) {

         accumRedSize =
            accumGreenSize =
            accumBlueSize = default_accum_bits();

         accumAlphaSize = alpha_flag ? accumRedSize : 0;
      }

      xmvis = save_glx_visual( dpy, vis, rgb_flag, alpha_flag, double_flag,
                               stereo_flag, depth_size, stencil_size,
                               accumRedSize, accumGreenSize,
                               accumBlueSize, accumAlphaSize, level, numAux,
                               num_samples );
   }

   return xmvis;
}


PUBLIC XVisualInfo *
glXChooseVisual( Display *dpy, int screen, int *list )
{
   XMesaVisual xmvis;

   /* register ourselves as an extension on this display */
   register_with_display(dpy);

   xmvis = choose_visual(dpy, screen, list, GL_FALSE);
   if (xmvis) {
      /* create a new vishandle - the cached one may be stale */
      xmvis->vishandle = malloc(sizeof(XVisualInfo));
      if (xmvis->vishandle) {
         memcpy(xmvis->vishandle, xmvis->visinfo, sizeof(XVisualInfo));
      }
      return xmvis->vishandle;
   }
   else
      return NULL;
}


/**
 * Helper function used by other glXCreateContext functions.
 */
static GLXContext
create_context(Display *dpy, XMesaVisual xmvis,
               XMesaContext shareCtx, Bool direct,
               unsigned major, unsigned minor,
               unsigned profileMask, unsigned contextFlags)
{
   GLXContext glxCtx;

   if (!dpy || !xmvis)
      return 0;

   glxCtx = CALLOC_STRUCT(__GLXcontextRec);
   if (!glxCtx)
      return 0;

   /* deallocate unused windows/buffers */
#if 0
   XMesaGarbageCollect();
#endif

   glxCtx->xmesaContext = XMesaCreateContext(xmvis, shareCtx, major, minor,
                                             profileMask, contextFlags);
   if (!glxCtx->xmesaContext) {
      free(glxCtx);
      return NULL;
   }

   glxCtx->isDirect = DEFAULT_DIRECT;
   glxCtx->currentDpy = dpy;
   glxCtx->xid = (XID) glxCtx;  /* self pointer */

   return glxCtx;
}


PUBLIC GLXContext
glXCreateContext( Display *dpy, XVisualInfo *visinfo,
                  GLXContext shareCtx, Bool direct )
{
   XMesaVisual xmvis;

   xmvis = find_glx_visual( dpy, visinfo );
   if (!xmvis) {
      /* This visual wasn't found with glXChooseVisual() */
      xmvis = create_glx_visual( dpy, visinfo );
      if (!xmvis) {
         /* unusable visual */
         return NULL;
      }
   }

   return create_context(dpy, xmvis,
                         shareCtx ? shareCtx->xmesaContext : NULL,
                         direct,
                         1, 0, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB, 0x0);
}


/* GLX 1.3 and later */
PUBLIC Bool
glXMakeContextCurrent( Display *dpy, GLXDrawable draw,
                       GLXDrawable read, GLXContext ctx )
{
   GLXContext glxCtx = ctx;
   GLXContext current = GetCurrentContext();
   static bool firsttime = 1, no_rast = 0;

   if (firsttime) {
      no_rast = getenv("SP_NO_RAST") != NULL;
      firsttime = 0;
   }

   if (ctx) {
      XMesaBuffer drawBuffer = NULL, readBuffer = NULL;
      XMesaContext xmctx = glxCtx->xmesaContext;

      /* either both must be null, or both must be non-null */
      if (!draw != !read)
         return False;

      if (draw) {
         /* Find the XMesaBuffer which corresponds to 'draw' */
         drawBuffer = XMesaFindBuffer( dpy, draw );
         if (!drawBuffer) {
            /* drawable must be a new window! */
            drawBuffer = XMesaCreateWindowBuffer( xmctx->xm_visual, draw );
            if (!drawBuffer) {
               /* Out of memory, or context/drawable depth mismatch */
               return False;
            }
         }
      }

      if (read) {
         /* Find the XMesaBuffer which corresponds to 'read' */
         readBuffer = XMesaFindBuffer( dpy, read );
         if (!readBuffer) {
            /* drawable must be a new window! */
            readBuffer = XMesaCreateWindowBuffer( xmctx->xm_visual, read );
            if (!readBuffer) {
               /* Out of memory, or context/drawable depth mismatch */
               return False;
            }
         }
      }

      if (no_rast && current == ctx)
         return True;

      /* Now make current! */
      if (XMesaMakeCurrent2(xmctx, drawBuffer, readBuffer)) {
         ctx->currentDpy = dpy;
         ctx->currentDrawable = draw;
         ctx->currentReadable = read;
         SetCurrentContext(ctx);
         return True;
      }
      else {
         return False;
      }
   }
   else if (!ctx && !draw && !read) {
      /* release current context w/out assigning new one. */
      XMesaMakeCurrent2( NULL, NULL, NULL );
      SetCurrentContext(NULL);
      return True;
   }
   else {
      /* We were given an invalid set of arguments */
      return False;
   }
}


PUBLIC Bool
glXMakeCurrent( Display *dpy, GLXDrawable drawable, GLXContext ctx )
{
   return glXMakeContextCurrent( dpy, drawable, drawable, ctx );
}


PUBLIC GLXContext
glXGetCurrentContext(void)
{
   return GetCurrentContext();
}


PUBLIC Display *
glXGetCurrentDisplay(void)
{
   GLXContext glxCtx = glXGetCurrentContext();

   return glxCtx ? glxCtx->currentDpy : NULL;
}


PUBLIC Display *
glXGetCurrentDisplayEXT(void)
{
   return glXGetCurrentDisplay();
}


PUBLIC GLXDrawable
glXGetCurrentDrawable(void)
{
   GLXContext gc = glXGetCurrentContext();
   return gc ? gc->currentDrawable : 0;
}


PUBLIC GLXDrawable
glXGetCurrentReadDrawable(void)
{
   GLXContext gc = glXGetCurrentContext();
   return gc ? gc->currentReadable : 0;
}


PUBLIC GLXDrawable
glXGetCurrentReadDrawableSGI(void)
{
   return glXGetCurrentReadDrawable();
}


PUBLIC GLXPixmap
glXCreateGLXPixmap( Display *dpy, XVisualInfo *visinfo, Pixmap pixmap )
{
   XMesaVisual v;
   XMesaBuffer b;

   v = find_glx_visual( dpy, visinfo );
   if (!v) {
      v = create_glx_visual( dpy, visinfo );
      if (!v) {
         /* unusable visual */
         return 0;
      }
   }

   b = XMesaCreatePixmapBuffer( v, pixmap, 0 );
   if (!b) {
      return 0;
   }
   return b->ws.drawable;
}


/*** GLX_MESA_pixmap_colormap ***/

PUBLIC GLXPixmap
glXCreateGLXPixmapMESA( Display *dpy, XVisualInfo *visinfo,
                        Pixmap pixmap, Colormap cmap )
{
   XMesaVisual v;
   XMesaBuffer b;

   v = find_glx_visual( dpy, visinfo );
   if (!v) {
      v = create_glx_visual( dpy, visinfo );
      if (!v) {
         /* unusable visual */
         return 0;
      }
   }

   b = XMesaCreatePixmapBuffer( v, pixmap, cmap );
   if (!b) {
      return 0;
   }
   return b->ws.drawable;
}


PUBLIC void
glXDestroyGLXPixmap( Display *dpy, GLXPixmap pixmap )
{
   XMesaBuffer b = XMesaFindBuffer(dpy, pixmap);
   if (b) {
      XMesaDestroyBuffer(b);
   }
   else if (getenv("MESA_DEBUG")) {
      _mesa_warning(NULL, "Mesa: glXDestroyGLXPixmap: invalid pixmap\n");
   }
}


PUBLIC void
glXCopyContext( Display *dpy, GLXContext src, GLXContext dst,
                unsigned long mask )
{
   XMesaContext xm_src = src->xmesaContext;
   XMesaContext xm_dst = dst->xmesaContext;
   (void) dpy;
   if (GetCurrentContext() == src) {
      glFlush();
   }
   XMesaCopyContext(xm_src, xm_dst, mask);
}


PUBLIC Bool
glXQueryExtension( Display *dpy, int *errorBase, int *eventBase )
{
   int op, ev, err;
   /* Mesa's GLX isn't really an X extension but we try to act like one. */
   if (!XQueryExtension(dpy, GLX_EXTENSION_NAME, &op, &ev, &err))
      ev = err = 0;
   if (errorBase)
      *errorBase = err;
   if (eventBase)
      *eventBase = ev;
   return True; /* we're faking GLX so always return success */
}


PUBLIC void
glXDestroyContext( Display *dpy, GLXContext ctx )
{
   GLXContext glxCtx = ctx;

   if (glxCtx == NULL || glxCtx->xid == None)
      return;

   if (ctx->currentDpy) {
      ctx->xid = None;
   } else {
      (void) dpy;
      XMesaDestroyContext( glxCtx->xmesaContext );
      XMesaGarbageCollect();
      free(glxCtx);
   }
}


PUBLIC Bool
glXIsDirect( Display *dpy, GLXContext ctx )
{
   return ctx ? ctx->isDirect : False;
}



PUBLIC void
glXSwapBuffers( Display *dpy, GLXDrawable drawable )
{
   XMesaBuffer buffer = XMesaFindBuffer( dpy, drawable );
   static bool firsttime = 1, no_rast = 0;

   if (firsttime) {
      no_rast = getenv("SP_NO_RAST") != NULL;
      firsttime = 0;
   }

   if (no_rast)
      return;

   if (buffer) {
      XMesaSwapBuffers(buffer);
   }
   else if (getenv("MESA_DEBUG")) {
      _mesa_warning(NULL, "glXSwapBuffers: invalid drawable 0x%x\n",
                    (int) drawable);
   }
}



/*** GLX_MESA_copy_sub_buffer ***/

PUBLIC void
glXCopySubBufferMESA(Display *dpy, GLXDrawable drawable,
                     int x, int y, int width, int height)
{
   XMesaBuffer buffer = XMesaFindBuffer( dpy, drawable );
   if (buffer) {
      XMesaCopySubBuffer(buffer, x, y, width, height);
   }
   else if (getenv("MESA_DEBUG")) {
      _mesa_warning(NULL, "Mesa: glXCopySubBufferMESA: invalid drawable\n");
   }
}


PUBLIC Bool
glXQueryVersion( Display *dpy, int *maj, int *min )
{
   (void) dpy;
   /* Return GLX version, not Mesa version */
   assert(CLIENT_MAJOR_VERSION == SERVER_MAJOR_VERSION);
   *maj = CLIENT_MAJOR_VERSION;
   *min = MIN2( CLIENT_MINOR_VERSION, SERVER_MINOR_VERSION );
   return True;
}


/*
 * Query the GLX attributes of the given XVisualInfo.
 */
static int
get_config( XMesaVisual xmvis, int attrib, int *value, GLboolean fbconfig )
{
   assert(xmvis);
   switch(attrib) {
      case GLX_USE_GL:
         if (fbconfig)
            return GLX_BAD_ATTRIBUTE;
         *value = (int) True;
	 return 0;
      case GLX_BUFFER_SIZE:
	 *value = xmvis->visinfo->depth;
	 return 0;
      case GLX_LEVEL:
	 *value = 0;
	 return 0;
      case GLX_RGBA:
         if (fbconfig)
            return GLX_BAD_ATTRIBUTE;
         *value = True;
	 return 0;
      case GLX_DOUBLEBUFFER:
	 *value = (int) xmvis->mesa_visual.doubleBufferMode;
	 return 0;
      case GLX_STEREO:
	 *value = (int) xmvis->mesa_visual.stereoMode;
	 return 0;
      case GLX_AUX_BUFFERS:
	 *value = 0;
	 return 0;
      case GLX_RED_SIZE:
         *value = xmvis->mesa_visual.redBits;
	 return 0;
      case GLX_GREEN_SIZE:
         *value = xmvis->mesa_visual.greenBits;
	 return 0;
      case GLX_BLUE_SIZE:
         *value = xmvis->mesa_visual.blueBits;
	 return 0;
      case GLX_ALPHA_SIZE:
         *value = xmvis->mesa_visual.alphaBits;
	 return 0;
      case GLX_DEPTH_SIZE:
         *value = xmvis->mesa_visual.depthBits;
	 return 0;
      case GLX_STENCIL_SIZE:
	 *value = xmvis->mesa_visual.stencilBits;
	 return 0;
      case GLX_ACCUM_RED_SIZE:
	 *value = xmvis->mesa_visual.accumRedBits;
	 return 0;
      case GLX_ACCUM_GREEN_SIZE:
	 *value = xmvis->mesa_visual.accumGreenBits;
	 return 0;
      case GLX_ACCUM_BLUE_SIZE:
	 *value = xmvis->mesa_visual.accumBlueBits;
	 return 0;
      case GLX_ACCUM_ALPHA_SIZE:
         *value = xmvis->mesa_visual.accumAlphaBits;
	 return 0;

      /*
       * GLX_EXT_visual_info extension
       */
      case GLX_X_VISUAL_TYPE_EXT:
         switch (xmvis->visinfo->CLASS) {
            case StaticGray:   *value = GLX_STATIC_GRAY_EXT;   return 0;
            case GrayScale:    *value = GLX_GRAY_SCALE_EXT;    return 0;
            case StaticColor:  *value = GLX_STATIC_GRAY_EXT;   return 0;
            case PseudoColor:  *value = GLX_PSEUDO_COLOR_EXT;  return 0;
            case TrueColor:    *value = GLX_TRUE_COLOR_EXT;    return 0;
            case DirectColor:  *value = GLX_DIRECT_COLOR_EXT;  return 0;
         }
         return 0;
      case GLX_TRANSPARENT_TYPE_EXT:
         /* normal planes */
         *value = GLX_NONE_EXT;
         return 0;
      case GLX_TRANSPARENT_INDEX_VALUE_EXT:
         /* undefined */
         return 0;
      case GLX_TRANSPARENT_RED_VALUE_EXT:
         /* undefined */
         return 0;
      case GLX_TRANSPARENT_GREEN_VALUE_EXT:
         /* undefined */
         return 0;
      case GLX_TRANSPARENT_BLUE_VALUE_EXT:
         /* undefined */
         return 0;
      case GLX_TRANSPARENT_ALPHA_VALUE_EXT:
         /* undefined */
         return 0;

      /*
       * GLX_EXT_visual_info extension
       */
      case GLX_VISUAL_CAVEAT_EXT:
         *value = GLX_NONE_EXT;
         return 0;

      /*
       * GLX_ARB_multisample
       */
      case GLX_SAMPLE_BUFFERS_ARB:
         *value = xmvis->mesa_visual.samples > 0;
         return 0;
      case GLX_SAMPLES_ARB:
         *value = xmvis->mesa_visual.samples;
         return 0;

      /*
       * For FBConfigs:
       */
      case GLX_SCREEN_EXT:
         if (!fbconfig)
            return GLX_BAD_ATTRIBUTE;
         *value = xmvis->visinfo->screen;
         break;
      case GLX_DRAWABLE_TYPE: /*SGIX too */
         if (!fbconfig)
            return GLX_BAD_ATTRIBUTE;
         *value = GLX_WINDOW_BIT | GLX_PIXMAP_BIT | GLX_PBUFFER_BIT;
         break;
      case GLX_RENDER_TYPE_SGIX:
         if (!fbconfig)
            return GLX_BAD_ATTRIBUTE;
         *value = GLX_RGBA_BIT;
         break;
      case GLX_X_RENDERABLE_SGIX:
         if (!fbconfig)
            return GLX_BAD_ATTRIBUTE;
         *value = True; /* XXX really? */
         break;
      case GLX_FBCONFIG_ID_SGIX:
         if (!fbconfig)
            return GLX_BAD_ATTRIBUTE;
         *value = xmvis->visinfo->visualid;
         break;
      case GLX_MAX_PBUFFER_WIDTH:
         if (!fbconfig)
            return GLX_BAD_ATTRIBUTE;
         /* XXX should be same as ctx->Const.MaxRenderbufferSize */
         *value = DisplayWidth(xmvis->display, xmvis->visinfo->screen);
         break;
      case GLX_MAX_PBUFFER_HEIGHT:
         if (!fbconfig)
            return GLX_BAD_ATTRIBUTE;
         *value = DisplayHeight(xmvis->display, xmvis->visinfo->screen);
         break;
      case GLX_MAX_PBUFFER_PIXELS:
         if (!fbconfig)
            return GLX_BAD_ATTRIBUTE;
         *value = DisplayWidth(xmvis->display, xmvis->visinfo->screen) *
                  DisplayHeight(xmvis->display, xmvis->visinfo->screen);
         break;
      case GLX_VISUAL_ID:
         if (!fbconfig)
            return GLX_BAD_ATTRIBUTE;
         *value = xmvis->visinfo->visualid;
         break;

      case GLX_BIND_TO_TEXTURE_RGB_EXT:
         *value = True; /*XXX*/
         break;
      case GLX_BIND_TO_TEXTURE_RGBA_EXT:
         /* XXX review */
         *value = xmvis->mesa_visual.alphaBits > 0 ? True : False;
         break;
      case GLX_BIND_TO_MIPMAP_TEXTURE_EXT:
         *value = True; /*XXX*/
         break;
      case GLX_BIND_TO_TEXTURE_TARGETS_EXT:
         *value = (GLX_TEXTURE_1D_BIT_EXT |
                   GLX_TEXTURE_2D_BIT_EXT |
                   GLX_TEXTURE_RECTANGLE_BIT_EXT); /*XXX*/
         break;
      case GLX_Y_INVERTED_EXT:
         *value = True; /*XXX*/
         break;

      default:
	 return GLX_BAD_ATTRIBUTE;
   }
   return Success;
}


PUBLIC int
glXGetConfig( Display *dpy, XVisualInfo *visinfo,
                   int attrib, int *value )
{
   XMesaVisual xmvis;
   int k;
   if (!dpy || !visinfo)
      return GLX_BAD_ATTRIBUTE;

   xmvis = find_glx_visual( dpy, visinfo );
   if (!xmvis) {
      /* this visual wasn't obtained with glXChooseVisual */
      xmvis = create_glx_visual( dpy, visinfo );
      if (!xmvis) {
	 /* this visual can't be used for GL rendering */
	 if (attrib==GLX_USE_GL) {
	    *value = (int) False;
	    return 0;
	 }
	 else {
	    return GLX_BAD_VISUAL;
	 }
      }
   }

   k = get_config(xmvis, attrib, value, GL_FALSE);
   return k;
}


PUBLIC void
glXWaitGL( void )
{
   XMesaContext xmesa = XMesaGetCurrentContext();
   XMesaFlush( xmesa );
}



PUBLIC void
glXWaitX( void )
{
   XMesaContext xmesa = XMesaGetCurrentContext();
   XMesaFlush( xmesa );
}


static const char *
get_extensions( void )
{
   return EXTENSIONS;
}



/* GLX 1.1 and later */
PUBLIC const char *
glXQueryExtensionsString( Display *dpy, int screen )
{
   (void) dpy;
   (void) screen;
   return get_extensions();
}



/* GLX 1.1 and later */
PUBLIC const char *
glXQueryServerString( Display *dpy, int screen, int name )
{
   static char version[1000];
   sprintf(version, "%d.%d %s",
	   SERVER_MAJOR_VERSION, SERVER_MINOR_VERSION, xmesa_get_name());

   (void) dpy;
   (void) screen;

   switch (name) {
      case GLX_EXTENSIONS:
         return get_extensions();
      case GLX_VENDOR:
	 return VENDOR;
      case GLX_VERSION:
	 return version;
      default:
         return NULL;
   }
}



/* GLX 1.1 and later */
PUBLIC const char *
glXGetClientString( Display *dpy, int name )
{
   static char version[1000];
   sprintf(version, "%d.%d %s", CLIENT_MAJOR_VERSION,
	   CLIENT_MINOR_VERSION, xmesa_get_name());

   (void) dpy;

   switch (name) {
      case GLX_EXTENSIONS:
         return get_extensions();
      case GLX_VENDOR:
	 return VENDOR;
      case GLX_VERSION:
	 return version;
      default:
         return NULL;
   }
}



/*
 * GLX 1.3 and later
 */


PUBLIC int
glXGetFBConfigAttrib(Display *dpy, GLXFBConfig config,
                     int attribute, int *value)
{
   XMesaVisual v = (XMesaVisual) config;
   (void) dpy;
   (void) config;

   if (!dpy || !config || !value)
      return -1;

   return get_config(v, attribute, value, GL_TRUE);
}


PUBLIC GLXFBConfig *
glXGetFBConfigs( Display *dpy, int screen, int *nelements )
{
   XVisualInfo *visuals, visTemplate;
   const long visMask = VisualScreenMask;
   int i;

   /* Get list of all X visuals */
   visTemplate.screen = screen;
   visuals = XGetVisualInfo(dpy, visMask, &visTemplate, nelements);
   if (*nelements > 0) {
      XMesaVisual *results = malloc(*nelements * sizeof(XMesaVisual));
      if (!results) {
         *nelements = 0;
         return NULL;
      }
      for (i = 0; i < *nelements; i++) {
         results[i] = create_glx_visual(dpy, visuals + i);
         if (!results[i]) {
            *nelements = i;
            break;
         }
      }
      return (GLXFBConfig *) results;
   }
   return NULL;
}


PUBLIC GLXFBConfig *
glXChooseFBConfig(Display *dpy, int screen,
                  const int *attribList, int *nitems)
{
   XMesaVisual xmvis;

   /* register ourselves as an extension on this display */
   register_with_display(dpy);

   if (!attribList || !attribList[0]) {
      /* return list of all configs (per GLX_SGIX_fbconfig spec) */
      return glXGetFBConfigs(dpy, screen, nitems);
   }

   xmvis = choose_visual(dpy, screen, attribList, GL_TRUE);
   if (xmvis) {
      GLXFBConfig *config = malloc(sizeof(XMesaVisual));
      if (!config) {
         *nitems = 0;
         return NULL;
      }
      *nitems = 1;
      config[0] = (GLXFBConfig) xmvis;
      return (GLXFBConfig *) config;
   }
   else {
      *nitems = 0;
      return NULL;
   }
}


PUBLIC XVisualInfo *
glXGetVisualFromFBConfig( Display *dpy, GLXFBConfig config )
{
   if (dpy && config) {
      XMesaVisual xmvis = (XMesaVisual) config;
#if 0
      return xmvis->vishandle;
#else
      /* create a new vishandle - the cached one may be stale */
      xmvis->vishandle = malloc(sizeof(XVisualInfo));
      if (xmvis->vishandle) {
         memcpy(xmvis->vishandle, xmvis->visinfo, sizeof(XVisualInfo));
      }
      return xmvis->vishandle;
#endif
   }
   else {
      return NULL;
   }
}


PUBLIC GLXWindow
glXCreateWindow(Display *dpy, GLXFBConfig config, Window win,
                const int *attribList)
{
   XMesaVisual xmvis = (XMesaVisual) config;
   XMesaBuffer xmbuf;
   if (!xmvis)
      return 0;

   xmbuf = XMesaCreateWindowBuffer(xmvis, win);
   if (!xmbuf)
      return 0;

   (void) dpy;
   (void) attribList;  /* Ignored in GLX 1.3 */

   return win;  /* A hack for now */
}


PUBLIC void
glXDestroyWindow( Display *dpy, GLXWindow window )
{
   XMesaBuffer b = XMesaFindBuffer(dpy, (Drawable) window);
   if (b)
      XMesaDestroyBuffer(b);
   /* don't destroy X window */
}


/* XXX untested */
PUBLIC GLXPixmap
glXCreatePixmap(Display *dpy, GLXFBConfig config, Pixmap pixmap,
                const int *attribList)
{
   XMesaVisual v = (XMesaVisual) config;
   XMesaBuffer b;
   const int *attr;
   int target = 0, format = 0, mipmap = 0;
   int value;

   if (!dpy || !config || !pixmap)
      return 0;

   for (attr = attribList; attr && *attr; attr++) {
      switch (*attr) {
      case GLX_TEXTURE_FORMAT_EXT:
         attr++;
         switch (*attr) {
         case GLX_TEXTURE_FORMAT_NONE_EXT:
         case GLX_TEXTURE_FORMAT_RGB_EXT:
         case GLX_TEXTURE_FORMAT_RGBA_EXT:
            format = *attr;
            break;
         default:
            /* error */
            return 0;
         }
         break;
      case GLX_TEXTURE_TARGET_EXT:
         attr++;
         switch (*attr) {
         case GLX_TEXTURE_1D_EXT:
         case GLX_TEXTURE_2D_EXT:
         case GLX_TEXTURE_RECTANGLE_EXT:
            target = *attr;
            break;
         default:
            /* error */
            return 0;
         }
         break;
      case GLX_MIPMAP_TEXTURE_EXT:
         attr++;
         if (*attr)
            mipmap = 1;
         break;
      default:
         /* error */
         return 0;
      }
   }

   if (format == GLX_TEXTURE_FORMAT_RGB_EXT) {
      if (get_config(v, GLX_BIND_TO_TEXTURE_RGB_EXT,
                     &value, GL_TRUE) != Success
          || !value) {
         return 0; /* error! */
      }
   }
   else if (format == GLX_TEXTURE_FORMAT_RGBA_EXT) {
      if (get_config(v, GLX_BIND_TO_TEXTURE_RGBA_EXT,
                     &value, GL_TRUE) != Success
          || !value) {
         return 0; /* error! */
      }
   }
   if (mipmap) {
      if (get_config(v, GLX_BIND_TO_MIPMAP_TEXTURE_EXT,
                     &value, GL_TRUE) != Success
          || !value) {
         return 0; /* error! */
      }
   }
   if (target == GLX_TEXTURE_1D_EXT) {
      if (get_config(v, GLX_BIND_TO_TEXTURE_TARGETS_EXT,
                     &value, GL_TRUE) != Success
          || (value & GLX_TEXTURE_1D_BIT_EXT) == 0) {
         return 0; /* error! */
      }
   }
   else if (target == GLX_TEXTURE_2D_EXT) {
      if (get_config(v, GLX_BIND_TO_TEXTURE_TARGETS_EXT,
                     &value, GL_TRUE) != Success
          || (value & GLX_TEXTURE_2D_BIT_EXT) == 0) {
         return 0; /* error! */
      }
   }
   if (target == GLX_TEXTURE_RECTANGLE_EXT) {
      if (get_config(v, GLX_BIND_TO_TEXTURE_TARGETS_EXT,
                     &value, GL_TRUE) != Success
          || (value & GLX_TEXTURE_RECTANGLE_BIT_EXT) == 0) {
         return 0; /* error! */
      }
   }

   if (format || target || mipmap) {
      /* texture from pixmap */
      b = XMesaCreatePixmapTextureBuffer(v, pixmap, 0, format, target, mipmap);
   }
   else {
      b = XMesaCreatePixmapBuffer( v, pixmap, 0 );
   }
   if (!b) {
      return 0;
   }

   return pixmap;
}


PUBLIC void
glXDestroyPixmap( Display *dpy, GLXPixmap pixmap )
{
   XMesaBuffer b = XMesaFindBuffer(dpy, (Drawable)pixmap);
   if (b)
      XMesaDestroyBuffer(b);
   /* don't destroy X pixmap */
}


PUBLIC GLXPbuffer
glXCreatePbuffer(Display *dpy, GLXFBConfig config, const int *attribList)
{
   XMesaVisual xmvis = (XMesaVisual) config;
   XMesaBuffer xmbuf;
   const int *attrib;
   int width = 0, height = 0;
   GLboolean useLargest = GL_FALSE, preserveContents = GL_FALSE;

   (void) dpy;

   for (attrib = attribList; *attrib; attrib++) {
      switch (*attrib) {
         case GLX_PBUFFER_WIDTH:
            attrib++;
            width = *attrib;
            break;
         case GLX_PBUFFER_HEIGHT:
            attrib++;
            height = *attrib;
            break;
         case GLX_PRESERVED_CONTENTS:
            attrib++;
            preserveContents = *attrib;
            break;
         case GLX_LARGEST_PBUFFER:
            attrib++;
            useLargest = *attrib;
            break;
         default:
            return 0;
      }
   }

   if (width == 0 || height == 0)
      return 0;

   if (width > PBUFFER_MAX_SIZE || height > PBUFFER_MAX_SIZE) {
      /* If allocation would have failed and GLX_LARGEST_PBUFFER is set,
       * allocate the largest possible buffer.
       */
      if (useLargest) {
         width = PBUFFER_MAX_SIZE;
         height = PBUFFER_MAX_SIZE;
      }
   }

   xmbuf = XMesaCreatePBuffer( xmvis, 0, width, height);
   /* A GLXPbuffer handle must be an X Drawable because that's what
    * glXMakeCurrent takes.
    */
   if (xmbuf) {
      xmbuf->largestPbuffer = useLargest;
      xmbuf->preservedContents = preserveContents;
      return (GLXPbuffer) xmbuf->ws.drawable;
   }
   else {
      return 0;
   }
}


PUBLIC void
glXDestroyPbuffer( Display *dpy, GLXPbuffer pbuf )
{
   XMesaBuffer b = XMesaFindBuffer(dpy, pbuf);
   if (b) {
      XMesaDestroyBuffer(b);
   }
}


PUBLIC void
glXQueryDrawable(Display *dpy, GLXDrawable draw, int attribute,
                 unsigned int *value)
{
   GLuint width, height;
   XMesaBuffer xmbuf = XMesaFindBuffer(dpy, draw);
   if (!xmbuf) {
      generate_error(dpy, GLXBadDrawable, draw, X_GLXGetDrawableAttributes, False);
      return;
   }

   /* make sure buffer's dimensions are up to date */
   xmesa_get_window_size(dpy, xmbuf, &width, &height);

   switch (attribute) {
      case GLX_WIDTH:
         *value = width;
         break;
      case GLX_HEIGHT:
         *value = height;
         break;
      case GLX_PRESERVED_CONTENTS:
         *value = xmbuf->preservedContents;
         break;
      case GLX_LARGEST_PBUFFER:
         *value = xmbuf->largestPbuffer;
         break;
      case GLX_FBCONFIG_ID:
         *value = xmbuf->xm_visual->visinfo->visualid;
         return;
      case GLX_TEXTURE_FORMAT_EXT:
         *value = xmbuf->TextureFormat;
         break;
      case GLX_TEXTURE_TARGET_EXT:
         *value = xmbuf->TextureTarget;
         break;
      case GLX_MIPMAP_TEXTURE_EXT:
         *value = xmbuf->TextureMipmap;
         break;

      default:
         generate_error(dpy, BadValue, 0, X_GLXCreateContextAttribsARB, true);
         return;
   }
}


PUBLIC GLXContext
glXCreateNewContext( Display *dpy, GLXFBConfig config,
                     int renderType, GLXContext shareCtx, Bool direct )
{
   XMesaVisual xmvis = (XMesaVisual) config;

   if (!dpy || !config ||
       (renderType != GLX_RGBA_TYPE && renderType != GLX_COLOR_INDEX_TYPE))
      return 0;

   return create_context(dpy, xmvis,
                         shareCtx ? shareCtx->xmesaContext : NULL,
                         direct,
                         1, 0, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB, 0x0);
}


PUBLIC int
glXQueryContext( Display *dpy, GLXContext ctx, int attribute, int *value )
{
   GLXContext glxCtx = ctx;
   XMesaContext xmctx = glxCtx->xmesaContext;

   (void) dpy;
   (void) ctx;

   switch (attribute) {
   case GLX_FBCONFIG_ID:
      *value = xmctx->xm_visual->visinfo->visualid;
      break;
   case GLX_RENDER_TYPE:
      *value = GLX_RGBA_TYPE;
      break;
   case GLX_SCREEN:
      *value = 0;
      return Success;
   default:
      return GLX_BAD_ATTRIBUTE;
   }
   return 0;
}


PUBLIC void
glXSelectEvent( Display *dpy, GLXDrawable drawable, unsigned long mask )
{
   XMesaBuffer xmbuf = XMesaFindBuffer(dpy, drawable);
   if (xmbuf)
      xmbuf->selectedEvents = mask;
}


PUBLIC void
glXGetSelectedEvent(Display *dpy, GLXDrawable drawable, unsigned long *mask)
{
   XMesaBuffer xmbuf = XMesaFindBuffer(dpy, drawable);
   if (xmbuf)
      *mask = xmbuf->selectedEvents;
   else
      *mask = 0;
}



/*** GLX_SGI_swap_control ***/

PUBLIC int
glXSwapIntervalSGI(int interval)
{
   (void) interval;
   return 0;
}



/*** GLX_SGI_video_sync ***/

static unsigned int FrameCounter = 0;

PUBLIC int
glXGetVideoSyncSGI(unsigned int *count)
{
   /* this is a bogus implementation */
   *count = FrameCounter++;
   return 0;
}

PUBLIC int
glXWaitVideoSyncSGI(int divisor, int remainder, unsigned int *count)
{
   if (divisor <= 0 || remainder < 0)
      return GLX_BAD_VALUE;
   /* this is a bogus implementation */
   FrameCounter++;
   while (FrameCounter % divisor != remainder)
      FrameCounter++;
   *count = FrameCounter;
   return 0;
}



/*** GLX_SGI_make_current_read ***/

PUBLIC Bool
glXMakeCurrentReadSGI(Display *dpy, GLXDrawable draw, GLXDrawable read,
                      GLXContext ctx)
{
   return glXMakeContextCurrent( dpy, draw, read, ctx );
}

/* not used
static GLXDrawable
glXGetCurrentReadDrawableSGI(void)
{
   return 0;
}
*/


/*** GLX_SGIX_video_source ***/
#if defined(_VL_H)

PUBLIC GLXVideoSourceSGIX
glXCreateGLXVideoSourceSGIX(Display *dpy, int screen, VLServer server,
                            VLPath path, int nodeClass, VLNode drainNode)
{
   (void) dpy;
   (void) screen;
   (void) server;
   (void) path;
   (void) nodeClass;
   (void) drainNode;
   return 0;
}

PUBLIC void
glXDestroyGLXVideoSourceSGIX(Display *dpy, GLXVideoSourceSGIX src)
{
   (void) dpy;
   (void) src;
}

#endif


/*** GLX_EXT_import_context ***/

PUBLIC void
glXFreeContextEXT(Display *dpy, GLXContext context)
{
   (void) dpy;
   (void) context;
}

PUBLIC GLXContextID
glXGetContextIDEXT(const GLXContext context)
{
   (void) context;
   return 0;
}

PUBLIC GLXContext
glXImportContextEXT(Display *dpy, GLXContextID contextID)
{
   (void) dpy;
   (void) contextID;
   return 0;
}

PUBLIC int
glXQueryContextInfoEXT(Display *dpy, GLXContext context, int attribute,
                       int *value)
{
   (void) dpy;
   (void) context;
   (void) attribute;
   (void) value;
   return 0;
}



/*** GLX_SGIX_fbconfig ***/

PUBLIC int
glXGetFBConfigAttribSGIX(Display *dpy, GLXFBConfigSGIX config,
                         int attribute, int *value)
{
   return glXGetFBConfigAttrib(dpy, config, attribute, value);
}

PUBLIC GLXFBConfigSGIX *
glXChooseFBConfigSGIX(Display *dpy, int screen, int *attrib_list,
                      int *nelements)
{
   return (GLXFBConfig *) glXChooseFBConfig(dpy, screen,
                                            attrib_list, nelements);
}


PUBLIC GLXPixmap
glXCreateGLXPixmapWithConfigSGIX(Display *dpy, GLXFBConfigSGIX config,
                                 Pixmap pixmap)
{
   XMesaVisual xmvis = (XMesaVisual) config;
   XMesaBuffer xmbuf = XMesaCreatePixmapBuffer(xmvis, pixmap, 0);
   return xmbuf->ws.drawable; /* need to return an X ID */
}


PUBLIC GLXContext
glXCreateContextWithConfigSGIX(Display *dpy, GLXFBConfigSGIX config,
                               int renderType, GLXContext shareCtx,
                               Bool direct)
{
   XMesaVisual xmvis = (XMesaVisual) config;

   if (!dpy || !config ||
       (renderType != GLX_RGBA_TYPE && renderType != GLX_COLOR_INDEX_TYPE))
      return 0;

   return create_context(dpy, xmvis,
                         shareCtx ? shareCtx->xmesaContext : NULL,
                         direct,
                         1, 0, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB, 0x0);
}


PUBLIC XVisualInfo *
glXGetVisualFromFBConfigSGIX(Display *dpy, GLXFBConfigSGIX config)
{
   return glXGetVisualFromFBConfig(dpy, config);
}


PUBLIC GLXFBConfigSGIX
glXGetFBConfigFromVisualSGIX(Display *dpy, XVisualInfo *vis)
{
   XMesaVisual xmvis = find_glx_visual(dpy, vis);
   if (!xmvis) {
      /* This visual wasn't found with glXChooseVisual() */
      xmvis = create_glx_visual(dpy, vis);
   }

   return (GLXFBConfigSGIX) xmvis;
}



/*** GLX_SGIX_pbuffer ***/

PUBLIC GLXPbufferSGIX
glXCreateGLXPbufferSGIX(Display *dpy, GLXFBConfigSGIX config,
                        unsigned int width, unsigned int height,
                        int *attribList)
{
   XMesaVisual xmvis = (XMesaVisual) config;
   XMesaBuffer xmbuf;
   const int *attrib;
   GLboolean useLargest = GL_FALSE, preserveContents = GL_FALSE;

   (void) dpy;

   for (attrib = attribList; attrib && *attrib; attrib++) {
      switch (*attrib) {
         case GLX_PRESERVED_CONTENTS_SGIX:
            attrib++;
            preserveContents = *attrib; /* ignored */
            break;
         case GLX_LARGEST_PBUFFER_SGIX:
            attrib++;
            useLargest = *attrib; /* ignored */
            break;
         default:
            return 0;
      }
   }

   /* not used at this time */
   (void) useLargest;
   (void) preserveContents;

   xmbuf = XMesaCreatePBuffer( xmvis, 0, width, height);
   /* A GLXPbuffer handle must be an X Drawable because that's what
    * glXMakeCurrent takes.
    */
   return (GLXPbuffer) xmbuf->ws.drawable;
}


PUBLIC void
glXDestroyGLXPbufferSGIX(Display *dpy, GLXPbufferSGIX pbuf)
{
   XMesaBuffer xmbuf = XMesaFindBuffer(dpy, pbuf);
   if (xmbuf) {
      XMesaDestroyBuffer(xmbuf);
   }
}


PUBLIC void
glXQueryGLXPbufferSGIX(Display *dpy, GLXPbufferSGIX pbuf, int attribute,
                       unsigned int *value)
{
   const XMesaBuffer xmbuf = XMesaFindBuffer(dpy, pbuf);

   if (!xmbuf) {
      /* Generate GLXBadPbufferSGIX for bad pbuffer */
      return;
   }

   switch (attribute) {
      case GLX_PRESERVED_CONTENTS_SGIX:
         *value = True;
         break;
      case GLX_LARGEST_PBUFFER_SGIX:
         *value = xmesa_buffer_width(xmbuf) * xmesa_buffer_height(xmbuf);
         break;
      case GLX_WIDTH_SGIX:
         *value = xmesa_buffer_width(xmbuf);
         break;
      case GLX_HEIGHT_SGIX:
         *value = xmesa_buffer_height(xmbuf);
         break;
      case GLX_EVENT_MASK_SGIX:
         *value = 0;  /* XXX might be wrong */
         break;
      default:
         *value = 0;
   }
}


PUBLIC void
glXSelectEventSGIX(Display *dpy, GLXDrawable drawable, unsigned long mask)
{
   XMesaBuffer xmbuf = XMesaFindBuffer(dpy, drawable);
   if (xmbuf) {
      /* Note: we'll never generate clobber events */
      xmbuf->selectedEvents = mask;
   }
}


PUBLIC void
glXGetSelectedEventSGIX(Display *dpy, GLXDrawable drawable,
                        unsigned long *mask)
{
   XMesaBuffer xmbuf = XMesaFindBuffer(dpy, drawable);
   if (xmbuf) {
      *mask = xmbuf->selectedEvents;
   }
   else {
      *mask = 0;
   }
}



/*** GLX_SGI_cushion ***/

PUBLIC void
glXCushionSGI(Display *dpy, Window win, float cushion)
{
   (void) dpy;
   (void) win;
   (void) cushion;
}



/*** GLX_SGIX_video_resize ***/

PUBLIC int
glXBindChannelToWindowSGIX(Display *dpy, int screen, int channel,
                           Window window)
{
   (void) dpy;
   (void) screen;
   (void) channel;
   (void) window;
   return 0;
}

PUBLIC int
glXChannelRectSGIX(Display *dpy, int screen, int channel,
                   int x, int y, int w, int h)
{
   (void) dpy;
   (void) screen;
   (void) channel;
   (void) x;
   (void) y;
   (void) w;
   (void) h;
   return 0;
}

PUBLIC int
glXQueryChannelRectSGIX(Display *dpy, int screen, int channel,
                        int *x, int *y, int *w, int *h)
{
   (void) dpy;
   (void) screen;
   (void) channel;
   (void) x;
   (void) y;
   (void) w;
   (void) h;
   return 0;
}

PUBLIC int
glXQueryChannelDeltasSGIX(Display *dpy, int screen, int channel,
                          int *dx, int *dy, int *dw, int *dh)
{
   (void) dpy;
   (void) screen;
   (void) channel;
   (void) dx;
   (void) dy;
   (void) dw;
   (void) dh;
   return 0;
}

PUBLIC int
glXChannelRectSyncSGIX(Display *dpy, int screen, int channel, GLenum synctype)
{
   (void) dpy;
   (void) screen;
   (void) channel;
   (void) synctype;
   return 0;
}



/*** GLX_SGIX_dmbuffer **/

#if defined(_DM_BUFFER_H_)
PUBLIC Bool
glXAssociateDMPbufferSGIX(Display *dpy, GLXPbufferSGIX pbuffer,
                          DMparams *params, DMbuffer dmbuffer)
{
   (void) dpy;
   (void) pbuffer;
   (void) params;
   (void) dmbuffer;
   return False;
}
#endif


/*** GLX_SUN_get_transparent_index ***/

PUBLIC Status
glXGetTransparentIndexSUN(Display *dpy, Window overlay, Window underlay,
                          unsigned long *pTransparent)
{
   (void) dpy;
   (void) overlay;
   (void) underlay;
   (void) pTransparent;
   return 0;
}



/*** GLX_MESA_release_buffers ***/

/*
 * Release the depth, stencil, accum buffers attached to a GLXDrawable
 * (a window or pixmap) prior to destroying the GLXDrawable.
 */
PUBLIC Bool
glXReleaseBuffersMESA( Display *dpy, GLXDrawable d )
{
   XMesaBuffer b = XMesaFindBuffer(dpy, d);
   if (b) {
      XMesaDestroyBuffer(b);
      return True;
   }
   return False;
}

/*** GLX_EXT_texture_from_pixmap ***/

PUBLIC void
glXBindTexImageEXT(Display *dpy, GLXDrawable drawable, int buffer,
                        const int *attrib_list)
{
   XMesaBuffer b = XMesaFindBuffer(dpy, drawable);
   if (b)
      XMesaBindTexImage(dpy, b, buffer, attrib_list);
}

PUBLIC void
glXReleaseTexImageEXT(Display *dpy, GLXDrawable drawable, int buffer)
{
   XMesaBuffer b = XMesaFindBuffer(dpy, drawable);
   if (b)
      XMesaReleaseTexImage(dpy, b, buffer);
}



/*** GLX_ARB_create_context ***/


GLXContext
glXCreateContextAttribsARB(Display *dpy, GLXFBConfig config,
                           GLXContext shareCtx, Bool direct,
                           const int *attrib_list)
{
   XMesaVisual xmvis = (XMesaVisual) config;
   int majorVersion = 1, minorVersion = 0;
   int contextFlags = 0x0;
   int profileMask = GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
   int renderType = GLX_RGBA_TYPE;
   unsigned i;
   Bool done = False;
   const int contextFlagsAll = (GLX_CONTEXT_DEBUG_BIT_ARB |
                                GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB);
   GLXContext ctx;

   /* parse attrib_list */
   for (i = 0; !done && attrib_list && attrib_list[i]; i++) {
      switch (attrib_list[i]) {
      case GLX_CONTEXT_MAJOR_VERSION_ARB:
         majorVersion = attrib_list[++i];
         break;
      case GLX_CONTEXT_MINOR_VERSION_ARB:
         minorVersion = attrib_list[++i];
         break;
      case GLX_CONTEXT_FLAGS_ARB:
         contextFlags = attrib_list[++i];
         break;
      case GLX_CONTEXT_PROFILE_MASK_ARB:
         profileMask = attrib_list[++i];
         break;
      case GLX_RENDER_TYPE:
         renderType = attrib_list[++i];
         break;
      case 0:
         /* end of list */
         done = True;
         break;
      default:
         /* bad attribute */
         generate_error(dpy, BadValue, 0, X_GLXCreateContextAttribsARB, True);
         return NULL;
      }
   }

   /* check contextFlags */
   if (contextFlags & ~contextFlagsAll) {
      generate_error(dpy, BadValue, 0, X_GLXCreateContextAttribsARB, True);
      return NULL;
   }

   /* check profileMask */
   if (profileMask != GLX_CONTEXT_CORE_PROFILE_BIT_ARB &&
       profileMask != GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB &&
       profileMask != GLX_CONTEXT_ES_PROFILE_BIT_EXT) {
      generate_error(dpy, GLXBadProfileARB, 0, X_GLXCreateContextAttribsARB, False);
      return NULL;
   }

   /* check renderType */
   if (renderType != GLX_RGBA_TYPE &&
       renderType != GLX_COLOR_INDEX_TYPE) {
      generate_error(dpy, BadValue, 0, X_GLXCreateContextAttribsARB, True);
      return NULL;
   }

   /* check version */
   if (majorVersion <= 0 ||
       minorVersion < 0 ||
       (profileMask != GLX_CONTEXT_ES_PROFILE_BIT_EXT &&
        ((majorVersion == 1 && minorVersion > 5) ||
         (majorVersion == 2 && minorVersion > 1) ||
         (majorVersion == 3 && minorVersion > 3) ||
         (majorVersion == 4 && minorVersion > 5) ||
         majorVersion > 4))) {
      generate_error(dpy, BadMatch, 0, X_GLXCreateContextAttribsARB, True);
      return NULL;
   }
   if (profileMask == GLX_CONTEXT_ES_PROFILE_BIT_EXT &&
       ((majorVersion == 1 && minorVersion > 1) ||
        (majorVersion == 2 && minorVersion > 0) ||
        (majorVersion == 3 && minorVersion > 1) ||
        majorVersion > 3)) {
      /* GLX_EXT_create_context_es2_profile says nothing to justifying a
       * different error code for invalid ES versions, but this is what NVIDIA
       * does and piglit expects.
       */
      generate_error(dpy, GLXBadProfileARB, 0, X_GLXCreateContextAttribsARB, False);
      return NULL;
   }

   if ((contextFlags & GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB) &&
       majorVersion < 3) {
      generate_error(dpy, BadMatch, 0, X_GLXCreateContextAttribsARB, True);
      return NULL;
   }

   if (renderType == GLX_COLOR_INDEX_TYPE && majorVersion >= 3) {
      generate_error(dpy, BadMatch, 0, X_GLXCreateContextAttribsARB, True);
      return NULL;
   }

   ctx = create_context(dpy, xmvis,
                        shareCtx ? shareCtx->xmesaContext : NULL,
                        direct,
                        majorVersion, minorVersion,
                        profileMask, contextFlags);
   if (!ctx) {
      generate_error(dpy, GLXBadFBConfig, 0, X_GLXCreateContextAttribsARB, False);
   }

   return ctx;
}
