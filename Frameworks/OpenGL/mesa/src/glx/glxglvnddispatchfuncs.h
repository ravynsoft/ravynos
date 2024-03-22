#ifndef __glx_glvnd_dispatch_funcs_h__
#define __glx_glvnd_dispatch_funcs_h__
/*
 * Helper functions used by g_glxglvnddispatchfuncs.c.
 */
#include "glvnd/libglxabi.h"
#include "glxglvnd.h"

#define __VND __glXGLVNDAPIExports

static inline int AddFBConfigMapping(Display *dpy, GLXFBConfig config,
                                     __GLXvendorInfo *vendor)
{
    return __VND->addVendorFBConfigMapping(dpy, config, vendor);
}

static inline int AddFBConfigsMapping(Display *dpy, const GLXFBConfig *ret,
                                      int *nelements, __GLXvendorInfo *vendor)
{
    int i, r;

    if (!nelements || !ret)
        return 0;

    for (i = 0; i < *nelements; i++) {
        r = __VND->addVendorFBConfigMapping(dpy, ret[i], vendor);
        if (r) {
            for (; i >= 0; i--)
                __VND->removeVendorFBConfigMapping(dpy, ret[i]);
            break;
        }
    }
    return r;
}

static inline int AddDrawableMapping(Display *dpy, GLXDrawable drawable,
                                     __GLXvendorInfo *vendor)
{
    return __VND->addVendorDrawableMapping(dpy, drawable, vendor);
}

static inline int AddContextMapping(Display *dpy, GLXContext ctx,
                                    __GLXvendorInfo *vendor)
{
    return __VND->addVendorContextMapping(dpy, ctx, vendor);
}

static inline __GLXvendorInfo *GetDispatchFromDrawable(Display *dpy,
                                                       GLXDrawable drawable)
{
    return __VND->vendorFromDrawable(dpy, drawable);
}

static inline __GLXvendorInfo *GetDispatchFromContext(GLXContext ctx)
{
    return __VND->vendorFromContext(ctx);
}

static inline __GLXvendorInfo *GetDispatchFromFBConfig(Display *dpy, GLXFBConfig config)
{
    return __VND->vendorFromFBConfig(dpy, config);
}

static inline __GLXvendorInfo *GetDispatchFromVisual(Display *dpy,
                                                     const XVisualInfo *visual)
{
    return __VND->getDynDispatch(dpy, visual->screen);
}

#endif // __glx_glvnd_dispatch_funcs_h__
