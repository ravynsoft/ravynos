#include "glxclient.h"
#include "glxextensions.h"
#include "glxconfig.h"

/*
** GLX_SGI_swap_control
*/
int
glXSwapIntervalSGI(int interval)
{
   (void) interval;
   return 0;
}


/*
** GLX_MESA_swap_control
*/
int
glXSwapIntervalMESA(unsigned int interval)
{
   (void) interval;
   return GLX_BAD_CONTEXT;
}


int
glXGetSwapIntervalMESA(void)
{
   return 0;
}


/*
** GLX_SGI_video_sync
*/
int
glXGetVideoSyncSGI(unsigned int *count)
{
   (void) count;
   return GLX_BAD_CONTEXT;
}

int
glXWaitVideoSyncSGI(int divisor, int remainder, unsigned int *count)
{
   (void) count;
   return GLX_BAD_CONTEXT;
}


/*
** GLX_OML_sync_control
*/
Bool
glXGetSyncValuesOML(Display * dpy, GLXDrawable drawable,
                    int64_t * ust, int64_t * msc, int64_t * sbc)
{
   (void) dpy;
   (void) drawable;
   (void) ust;
   (void) msc;
   (void) sbc;
   return False;
}

int64_t
glXSwapBuffersMscOML(Display * dpy, GLXDrawable drawable,
                     int64_t target_msc, int64_t divisor, int64_t remainder)
{
   (void) dpy;
   (void) drawable;
   (void) target_msc;
   (void) divisor;
   (void) remainder;
   return 0;
}


Bool
glXWaitForMscOML(Display * dpy, GLXDrawable drawable,
                 int64_t target_msc, int64_t divisor,
                 int64_t remainder, int64_t * ust,
                 int64_t * msc, int64_t * sbc)
{
   (void) dpy;
   (void) drawable;
   (void) target_msc;
   (void) divisor;
   (void) remainder;
   (void) ust;
   (void) msc;
   (void) sbc;
   return False;
}


Bool
glXWaitForSbcOML(Display * dpy, GLXDrawable drawable,
                 int64_t target_sbc, int64_t * ust,
                 int64_t * msc, int64_t * sbc)
{
   (void) dpy;
   (void) drawable;
   (void) target_sbc;
   (void) ust;
   (void) msc;
   (void) sbc;
   return False;
}


Bool
glXReleaseBuffersMESA(Display * dpy, GLXDrawable d)
{
   (void) dpy;
   (void) d;
   return False;
}


_X_EXPORT GLXPixmap
glXCreateGLXPixmapMESA(Display * dpy, XVisualInfo * visual,
                       Pixmap pixmap, Colormap cmap)
{
   (void) dpy;
   (void) visual;
   (void) pixmap;
   (void) cmap;
   return 0;
}


/**
 * GLX_MESA_copy_sub_buffer
 */
void
glXCopySubBufferMESA(Display * dpy, GLXDrawable drawable,
                     int x, int y, int width, int height)
{
   (void) dpy;
   (void) drawable;
   (void) x;
   (void) y;
   (void) width;
   (void) height;
}


_X_EXPORT void
glXQueryGLXPbufferSGIX(Display * dpy, GLXDrawable drawable,
                       int attribute, unsigned int *value)
{
   (void) dpy;
   (void) drawable;
   (void) attribute;
   (void) value;
}

_X_EXPORT GLXDrawable
glXCreateGLXPbufferSGIX(Display * dpy, GLXFBConfig config,
                        unsigned int width, unsigned int height,
                        int *attrib_list)
{
   (void) dpy;
   (void) config;
   (void) width;
   (void) height;
   (void) attrib_list;
   return None;
}

#if 0
/* GLX_SGIX_fbconfig */
_X_EXPORT int
glXGetFBConfigAttribSGIX(Display * dpy, void *config, int a, int *b)
{
   (void) dpy;
   (void) config;
   (void) a;
   (void) b;
   return 0;
}

_X_EXPORT void *
glXChooseFBConfigSGIX(Display * dpy, int a, int *b, int *c)
{
   (void) dpy;
   (void) a;
   (void) b;
   (void) c;
   return NULL;
}

_X_EXPORT GLXPixmap
glXCreateGLXPixmapWithConfigSGIX(Display * dpy, void *config, Pixmap p)
{
   (void) dpy;
   (void) config;
   (void) p;
   return None;
}

_X_EXPORT GLXContext
glXCreateContextWithConfigSGIX(Display * dpy, void *config, int a,
                               GLXContext b, Bool c)
{
   (void) dpy;
   (void) config;
   (void) a;
   (void) b;
   (void) c;
   return NULL;
}

_X_EXPORT XVisualInfo *
glXGetVisualFromFBConfigSGIX(Display * dpy, void *config)
{
   (void) dpy;
   (void) config;
   return NULL;
}

_X_EXPORT void *
glXGetFBConfigFromVisualSGIX(Display * dpy, XVisualInfo * visinfo)
{
   (void) dpy;
   (void) visinfo;
   return NULL;
}
#endif
