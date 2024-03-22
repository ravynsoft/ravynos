#include <stdlib.h>

#include "glxclient.h"
#include "glxglvnd.h"
#include "glxglvnddispatchfuncs.h"
#include "g_glxglvnddispatchindices.h"

#include "GL/mesa_glinterop.h"

const int DI_FUNCTION_COUNT = DI_LAST_INDEX;
/* Allocate an extra 'dummy' to ease lookup. See FindGLXFunction() */
int __glXDispatchTableIndices[DI_LAST_INDEX + 1];
const __GLXapiExports *__glXGLVNDAPIExports;

const char * const __glXDispatchTableStrings[DI_LAST_INDEX] = {
#define __ATTRIB(field) \
    [DI_##field] = "glX"#field

    __ATTRIB(BindSwapBarrierSGIX),
    __ATTRIB(BindTexImageEXT),
    // glXChooseFBConfig implemented by libglvnd
    __ATTRIB(ChooseFBConfigSGIX),
    // glXChooseVisual implemented by libglvnd
    // glXCopyContext implemented by libglvnd
    __ATTRIB(CopySubBufferMESA),
    // glXCreateContext implemented by libglvnd
    __ATTRIB(CreateContextAttribsARB),
    __ATTRIB(CreateContextWithConfigSGIX),
    __ATTRIB(CreateGLXPbufferSGIX),
    // glXCreateGLXPixmap implemented by libglvnd
    __ATTRIB(CreateGLXPixmapMESA),
    __ATTRIB(CreateGLXPixmapWithConfigSGIX),
    // glXCreateNewContext implemented by libglvnd
    // glXCreatePbuffer implemented by libglvnd
    // glXCreatePixmap implemented by libglvnd
    // glXCreateWindow implemented by libglvnd
    // glXDestroyContext implemented by libglvnd
    __ATTRIB(DestroyGLXPbufferSGIX),
    // glXDestroyGLXPixmap implemented by libglvnd
    // glXDestroyPbuffer implemented by libglvnd
    // glXDestroyPixmap implemented by libglvnd
    // glXDestroyWindow implemented by libglvnd
    // glXFreeContextEXT implemented by libglvnd
    __ATTRIB(GLInteropExportObjectMESA),
    __ATTRIB(GLInteropFlushObjectsMESA),
    __ATTRIB(GLInteropQueryDeviceInfoMESA),
    // glXGetClientString implemented by libglvnd
    // glXGetConfig implemented by libglvnd
    __ATTRIB(GetContextIDEXT),
    // glXGetCurrentContext implemented by libglvnd
    // glXGetCurrentDisplay implemented by libglvnd
    __ATTRIB(GetCurrentDisplayEXT),
    // glXGetCurrentDrawable implemented by libglvnd
    // glXGetCurrentReadDrawable implemented by libglvnd
    __ATTRIB(GetDriverConfig),
    // glXGetFBConfigAttrib implemented by libglvnd
    __ATTRIB(GetFBConfigAttribSGIX),
    __ATTRIB(GetFBConfigFromVisualSGIX),
    // glXGetFBConfigs implemented by libglvnd
    __ATTRIB(GetMscRateOML),
    // glXGetProcAddress implemented by libglvnd
    // glXGetProcAddressARB implemented by libglvnd
    __ATTRIB(GetScreenDriver),
    // glXGetSelectedEvent implemented by libglvnd
    __ATTRIB(GetSelectedEventSGIX),
    __ATTRIB(GetSwapIntervalMESA),
    __ATTRIB(GetSyncValuesOML),
    __ATTRIB(GetVideoSyncSGI),
    // glXGetVisualFromFBConfig implemented by libglvnd
    __ATTRIB(GetVisualFromFBConfigSGIX),
    // glXImportContextEXT implemented by libglvnd
    // glXIsDirect implemented by libglvnd
    __ATTRIB(JoinSwapGroupSGIX),
    // glXMakeContextCurrent implemented by libglvnd
    // glXMakeCurrent implemented by libglvnd
    // glXQueryContext implemented by libglvnd
    __ATTRIB(QueryContextInfoEXT),
    __ATTRIB(QueryCurrentRendererIntegerMESA),
    __ATTRIB(QueryCurrentRendererStringMESA),
    // glXQueryDrawable implemented by libglvnd
    // glXQueryExtension implemented by libglvnd
    // glXQueryExtensionsString implemented by libglvnd
    __ATTRIB(QueryGLXPbufferSGIX),
    __ATTRIB(QueryMaxSwapBarriersSGIX),
    __ATTRIB(QueryRendererIntegerMESA),
    __ATTRIB(QueryRendererStringMESA),
    // glXQueryServerString implemented by libglvnd
    // glXQueryVersion implemented by libglvnd
    __ATTRIB(ReleaseBuffersMESA),
    __ATTRIB(ReleaseTexImageEXT),
    // glXSelectEvent implemented by libglvnd
    __ATTRIB(SelectEventSGIX),
    // glXSwapBuffers implemented by libglvnd
    __ATTRIB(SwapBuffersMscOML),
    __ATTRIB(SwapIntervalEXT),
    __ATTRIB(SwapIntervalMESA),
    __ATTRIB(SwapIntervalSGI),
    // glXUseXFont implemented by libglvnd
    __ATTRIB(WaitForMscOML),
    __ATTRIB(WaitForSbcOML),
    // glXWaitGL implemented by libglvnd
    __ATTRIB(WaitVideoSyncSGI),
    // glXWaitX implemented by libglvnd

#undef __ATTRIB
};

#define __FETCH_FUNCTION_PTR(func_name) \
    p##func_name = (void *) \
        __VND->fetchDispatchEntry(dd, __glXDispatchTableIndices[DI_##func_name])


static void dispatch_BindTexImageEXT(Display *dpy, GLXDrawable drawable,
                                     int buffer, const int *attrib_list)
{
    PFNGLXBINDTEXIMAGEEXTPROC pBindTexImageEXT;
    __GLXvendorInfo *dd;

    dd = GetDispatchFromDrawable(dpy, drawable);
    if (dd == NULL)
        return;

    __FETCH_FUNCTION_PTR(BindTexImageEXT);
    if (pBindTexImageEXT == NULL)
        return;

    pBindTexImageEXT(dpy, drawable, buffer, attrib_list);
}



static GLXFBConfigSGIX *dispatch_ChooseFBConfigSGIX(Display *dpy, int screen,
                                                    int *attrib_list,
                                                    int *nelements)
{
    PFNGLXCHOOSEFBCONFIGSGIXPROC pChooseFBConfigSGIX;
    __GLXvendorInfo *dd;
    GLXFBConfigSGIX *ret;

    dd = __VND->getDynDispatch(dpy, screen);
    if (dd == NULL)
        return NULL;

    __FETCH_FUNCTION_PTR(ChooseFBConfigSGIX);
    if (pChooseFBConfigSGIX == NULL)
        return NULL;

    ret = pChooseFBConfigSGIX(dpy, screen, attrib_list, nelements);
    if (AddFBConfigsMapping(dpy, ret, nelements, dd)) {
        free(ret);
        return NULL;
    }

    return ret;
}



static GLXContext dispatch_CreateContextAttribsARB(Display *dpy,
                                                   GLXFBConfig config,
                                                   GLXContext share_list,
                                                   Bool direct,
                                                   const int *attrib_list)
{
    PFNGLXCREATECONTEXTATTRIBSARBPROC pCreateContextAttribsARB;
    __GLXvendorInfo *dd = NULL;
    GLXContext ret;

    if (config) {
       dd = GetDispatchFromFBConfig(dpy, config);
    } else if (attrib_list) {
       int i, screen;

       for (i = 0; attrib_list[i * 2] != None; i++) {
          if (attrib_list[i * 2] == GLX_SCREEN) {
             screen = attrib_list[i * 2 + 1];
             dd = GetDispatchFromDrawable(dpy, RootWindow(dpy, screen));
             break;
          }
       }
    }
    if (dd == NULL)
        return None;

    __FETCH_FUNCTION_PTR(CreateContextAttribsARB);
    if (pCreateContextAttribsARB == NULL)
        return None;

    ret = pCreateContextAttribsARB(dpy, config, share_list, direct, attrib_list);
    if (AddContextMapping(dpy, ret, dd)) {
        /* XXX: Call glXDestroyContext which lives in libglvnd. If we're not
         * allowed to call it from here, should we extend __glXDispatchTableIndices ?
         */
        return None;
    }

    return ret;
}



static GLXContext dispatch_CreateContextWithConfigSGIX(Display *dpy,
                                                       GLXFBConfigSGIX config,
                                                       int render_type,
                                                       GLXContext share_list,
                                                       Bool direct)
{
    PFNGLXCREATECONTEXTWITHCONFIGSGIXPROC pCreateContextWithConfigSGIX;
    __GLXvendorInfo *dd;
    GLXContext ret;

    dd = GetDispatchFromFBConfig(dpy, config);
    if (dd == NULL)
        return None;

    __FETCH_FUNCTION_PTR(CreateContextWithConfigSGIX);
    if (pCreateContextWithConfigSGIX == NULL)
        return None;

    ret = pCreateContextWithConfigSGIX(dpy, config, render_type, share_list, direct);
    if (AddContextMapping(dpy, ret, dd)) {
        /* XXX: Call glXDestroyContext which lives in libglvnd. If we're not
         * allowed to call it from here, should we extend __glXDispatchTableIndices ?
         */
        return None;
    }

    return ret;
}



static GLXPbuffer dispatch_CreateGLXPbufferSGIX(Display *dpy,
                                                GLXFBConfig config,
                                                unsigned int width,
                                                unsigned int height,
                                                int *attrib_list)
{
    PFNGLXCREATEGLXPBUFFERSGIXPROC pCreateGLXPbufferSGIX;
    __GLXvendorInfo *dd;
    GLXPbuffer ret;

    dd = GetDispatchFromFBConfig(dpy, config);
    if (dd == NULL)
        return None;

    __FETCH_FUNCTION_PTR(CreateGLXPbufferSGIX);
    if (pCreateGLXPbufferSGIX == NULL)
        return None;

    ret = pCreateGLXPbufferSGIX(dpy, config, width, height, attrib_list);
    if (AddDrawableMapping(dpy, ret, dd)) {
        PFNGLXDESTROYGLXPBUFFERSGIXPROC pDestroyGLXPbufferSGIX;

        __FETCH_FUNCTION_PTR(DestroyGLXPbufferSGIX);
        if (pDestroyGLXPbufferSGIX)
            pDestroyGLXPbufferSGIX(dpy, ret);

        return None;
    }

    return ret;
}



static GLXPixmap dispatch_CreateGLXPixmapWithConfigSGIX(Display *dpy,
                                                        GLXFBConfigSGIX config,
                                                        Pixmap pixmap)
{
    PFNGLXCREATEGLXPIXMAPWITHCONFIGSGIXPROC pCreateGLXPixmapWithConfigSGIX;
    __GLXvendorInfo *dd;
    GLXPixmap ret;

    dd = GetDispatchFromFBConfig(dpy, config);
    if (dd == NULL)
        return None;

    __FETCH_FUNCTION_PTR(CreateGLXPixmapWithConfigSGIX);
    if (pCreateGLXPixmapWithConfigSGIX == NULL)
        return None;

    ret = pCreateGLXPixmapWithConfigSGIX(dpy, config, pixmap);
    if (AddDrawableMapping(dpy, ret, dd)) {
        /* XXX: Call glXDestroyGLXPixmap which lives in libglvnd. If we're not
         * allowed to call it from here, should we extend __glXDispatchTableIndices ?
         */
        return None;
    }

    return ret;
}



static void dispatch_DestroyGLXPbufferSGIX(Display *dpy, GLXPbuffer pbuf)
{
    PFNGLXDESTROYGLXPBUFFERSGIXPROC pDestroyGLXPbufferSGIX;
    __GLXvendorInfo *dd;

    dd = GetDispatchFromDrawable(dpy, pbuf);
    if (dd == NULL)
        return;

    __FETCH_FUNCTION_PTR(DestroyGLXPbufferSGIX);
    if (pDestroyGLXPbufferSGIX == NULL)
        return;

    pDestroyGLXPbufferSGIX(dpy, pbuf);
}



static int dispatch_GLInteropExportObjectMESA(Display *dpy, GLXContext ctx,
                                              struct mesa_glinterop_export_in *in,
                                              struct mesa_glinterop_export_out *out)
{
    PFNMESAGLINTEROPGLXEXPORTOBJECTPROC pGLInteropExportObjectMESA;
    __GLXvendorInfo *dd;

    dd = GetDispatchFromContext(ctx);
    if (dd == NULL)
        return 0;

    __FETCH_FUNCTION_PTR(GLInteropExportObjectMESA);
    if (pGLInteropExportObjectMESA == NULL)
        return 0;

    return pGLInteropExportObjectMESA(dpy, ctx, in, out);
}


static int dispatch_GLInteropFlushObjectsMESA(Display *dpy, GLXContext ctx,
                                              unsigned count,
                                              struct mesa_glinterop_export_in *resources,
                                              struct mesa_glinterop_flush_out *out)
{
    PFNMESAGLINTEROPGLXFLUSHOBJECTSPROC pGLInteropFlushObjectsMESA;
    __GLXvendorInfo *dd;

    dd = GetDispatchFromContext(ctx);
    if (dd == NULL)
        return 0;

    __FETCH_FUNCTION_PTR(GLInteropFlushObjectsMESA);
    if (pGLInteropFlushObjectsMESA == NULL)
        return 0;

    return pGLInteropFlushObjectsMESA(dpy, ctx, count, resources, out);
}


static int dispatch_GLInteropQueryDeviceInfoMESA(Display *dpy, GLXContext ctx,
                                                 struct mesa_glinterop_device_info *out)
{
    PFNMESAGLINTEROPGLXQUERYDEVICEINFOPROC pGLInteropQueryDeviceInfoMESA;
    __GLXvendorInfo *dd;

    dd = GetDispatchFromContext(ctx);
    if (dd == NULL)
        return 0;

    __FETCH_FUNCTION_PTR(GLInteropQueryDeviceInfoMESA);
    if (pGLInteropQueryDeviceInfoMESA == NULL)
        return 0;

    return pGLInteropQueryDeviceInfoMESA(dpy, ctx, out);
}


static GLXContextID dispatch_GetContextIDEXT(const GLXContext ctx)
{
    PFNGLXGETCONTEXTIDEXTPROC pGetContextIDEXT;
    __GLXvendorInfo *dd;

    dd = GetDispatchFromContext(ctx);
    if (dd == NULL)
        return None;

    __FETCH_FUNCTION_PTR(GetContextIDEXT);
    if (pGetContextIDEXT == NULL)
        return None;

    return pGetContextIDEXT(ctx);
}



static Display *dispatch_GetCurrentDisplayEXT(void)
{
    PFNGLXGETCURRENTDISPLAYEXTPROC pGetCurrentDisplayEXT;
    __GLXvendorInfo *dd;

    if (!__VND->getCurrentContext())
        return NULL;

    dd = __VND->getCurrentDynDispatch();
    if (dd == NULL)
        return NULL;

    __FETCH_FUNCTION_PTR(GetCurrentDisplayEXT);
    if (pGetCurrentDisplayEXT == NULL)
        return NULL;

    return pGetCurrentDisplayEXT();
}



static const char *dispatch_GetDriverConfig(const char *driverName)
{
#if defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL)
    /*
     * The options are constant for a given driverName, so we do not need
     * a context (and apps expect to be able to call this without one).
     */
    return glXGetDriverConfig(driverName);
#else
    return NULL;
#endif
}



static int dispatch_GetFBConfigAttribSGIX(Display *dpy, GLXFBConfigSGIX config,
                                          int attribute, int *value_return)
{
    PFNGLXGETFBCONFIGATTRIBSGIXPROC pGetFBConfigAttribSGIX;
    __GLXvendorInfo *dd;

    dd = GetDispatchFromFBConfig(dpy, config);
    if (dd == NULL)
        return GLX_NO_EXTENSION;

    __FETCH_FUNCTION_PTR(GetFBConfigAttribSGIX);
    if (pGetFBConfigAttribSGIX == NULL)
        return GLX_NO_EXTENSION;

    return pGetFBConfigAttribSGIX(dpy, config, attribute, value_return);
}



static GLXFBConfigSGIX dispatch_GetFBConfigFromVisualSGIX(Display *dpy,
                                                          XVisualInfo *vis)
{
    PFNGLXGETFBCONFIGFROMVISUALSGIXPROC pGetFBConfigFromVisualSGIX;
    __GLXvendorInfo *dd;
    GLXFBConfigSGIX ret = NULL;

    dd = GetDispatchFromVisual(dpy, vis);
    if (dd == NULL)
        return NULL;

    __FETCH_FUNCTION_PTR(GetFBConfigFromVisualSGIX);
    if (pGetFBConfigFromVisualSGIX == NULL)
        return NULL;

    ret = pGetFBConfigFromVisualSGIX(dpy, vis);
    if (AddFBConfigMapping(dpy, ret, dd))
        /* XXX: dealloc ret ? */
        return NULL;

    return ret;
}



static void dispatch_GetSelectedEventSGIX(Display *dpy, GLXDrawable drawable,
                                          unsigned long *mask)
{
    PFNGLXGETSELECTEDEVENTSGIXPROC pGetSelectedEventSGIX;
    __GLXvendorInfo *dd;

    dd = GetDispatchFromDrawable(dpy, drawable);
    if (dd == NULL)
        return;

    __FETCH_FUNCTION_PTR(GetSelectedEventSGIX);
    if (pGetSelectedEventSGIX == NULL)
        return;

    pGetSelectedEventSGIX(dpy, drawable, mask);
}



static int dispatch_GetVideoSyncSGI(unsigned int *count)
{
    PFNGLXGETVIDEOSYNCSGIPROC pGetVideoSyncSGI;
    __GLXvendorInfo *dd;

    if (!__VND->getCurrentContext())
        return GLX_BAD_CONTEXT;

    dd = __VND->getCurrentDynDispatch();
    if (dd == NULL)
        return GLX_NO_EXTENSION;

    __FETCH_FUNCTION_PTR(GetVideoSyncSGI);
    if (pGetVideoSyncSGI == NULL)
        return GLX_NO_EXTENSION;

    return pGetVideoSyncSGI(count);
}



static XVisualInfo *dispatch_GetVisualFromFBConfigSGIX(Display *dpy,
                                                       GLXFBConfigSGIX config)
{
    PFNGLXGETVISUALFROMFBCONFIGSGIXPROC pGetVisualFromFBConfigSGIX;
    __GLXvendorInfo *dd;

    dd = GetDispatchFromFBConfig(dpy, config);
    if (dd == NULL)
        return NULL;

    __FETCH_FUNCTION_PTR(GetVisualFromFBConfigSGIX);
    if (pGetVisualFromFBConfigSGIX == NULL)
        return NULL;

    return pGetVisualFromFBConfigSGIX(dpy, config);
}



static int dispatch_QueryContextInfoEXT(Display *dpy, GLXContext ctx,
                                        int attribute, int *value)
{
    PFNGLXQUERYCONTEXTINFOEXTPROC pQueryContextInfoEXT;
    __GLXvendorInfo *dd;

    dd = GetDispatchFromContext(ctx);
    if (dd == NULL)
        return GLX_NO_EXTENSION;

    __FETCH_FUNCTION_PTR(QueryContextInfoEXT);
    if (pQueryContextInfoEXT == NULL)
        return GLX_NO_EXTENSION;

    return pQueryContextInfoEXT(dpy, ctx, attribute, value);
}



static void dispatch_QueryGLXPbufferSGIX(Display *dpy, GLXPbuffer pbuf,
                                         int attribute, unsigned int *value)
{
    PFNGLXQUERYGLXPBUFFERSGIXPROC pQueryGLXPbufferSGIX;
    __GLXvendorInfo *dd;

    dd = GetDispatchFromDrawable(dpy, pbuf);
    if (dd == NULL)
        return;

    __FETCH_FUNCTION_PTR(QueryGLXPbufferSGIX);
    if (pQueryGLXPbufferSGIX == NULL)
        return;

    pQueryGLXPbufferSGIX(dpy, pbuf, attribute, value);
}



static void dispatch_ReleaseTexImageEXT(Display *dpy, GLXDrawable drawable,
                                        int buffer)
{
    PFNGLXRELEASETEXIMAGEEXTPROC pReleaseTexImageEXT;
    __GLXvendorInfo *dd;

    dd = GetDispatchFromDrawable(dpy, drawable);
    if (dd == NULL)
        return;

    __FETCH_FUNCTION_PTR(ReleaseTexImageEXT);
    if (pReleaseTexImageEXT == NULL)
        return;

    pReleaseTexImageEXT(dpy, drawable, buffer);
}



static void dispatch_SelectEventSGIX(Display *dpy, GLXDrawable drawable,
                                     unsigned long mask)
{
    PFNGLXSELECTEVENTSGIXPROC pSelectEventSGIX;
    __GLXvendorInfo *dd;

    dd = GetDispatchFromDrawable(dpy, drawable);
    if (dd == NULL)
        return;

    __FETCH_FUNCTION_PTR(SelectEventSGIX);
    if (pSelectEventSGIX == NULL)
        return;

    pSelectEventSGIX(dpy, drawable, mask);
}



static int dispatch_SwapIntervalSGI(int interval)
{
    PFNGLXSWAPINTERVALSGIPROC pSwapIntervalSGI;
    __GLXvendorInfo *dd;

    if (!__VND->getCurrentContext())
        return GLX_BAD_CONTEXT;

    dd = __VND->getCurrentDynDispatch();
    if (dd == NULL)
        return GLX_NO_EXTENSION;

    __FETCH_FUNCTION_PTR(SwapIntervalSGI);
    if (pSwapIntervalSGI == NULL)
        return GLX_NO_EXTENSION;

    return pSwapIntervalSGI(interval);
}



static int dispatch_WaitVideoSyncSGI(int divisor, int remainder,
                                     unsigned int *count)
{
    PFNGLXWAITVIDEOSYNCSGIPROC pWaitVideoSyncSGI;
    __GLXvendorInfo *dd;

    if (!__VND->getCurrentContext())
        return GLX_BAD_CONTEXT;

    dd = __VND->getCurrentDynDispatch();
    if (dd == NULL)
        return GLX_NO_EXTENSION;

    __FETCH_FUNCTION_PTR(WaitVideoSyncSGI);
    if (pWaitVideoSyncSGI == NULL)
        return GLX_NO_EXTENSION;

    return pWaitVideoSyncSGI(divisor, remainder, count);
}



static void dispatch_BindSwapBarrierSGIX(Display *dpy, GLXDrawable drawable,
                                            int barrier)
{
    PFNGLXBINDSWAPBARRIERSGIXPROC pBindSwapBarrierSGIX;
    __GLXvendorInfo *dd;

    dd = GetDispatchFromDrawable(dpy, drawable);
    if (dd == NULL)
        return;

    __FETCH_FUNCTION_PTR(BindSwapBarrierSGIX);
    if (pBindSwapBarrierSGIX == NULL)
        return;

    pBindSwapBarrierSGIX(dpy, drawable, barrier);
}



static void dispatch_CopySubBufferMESA(Display *dpy, GLXDrawable drawable,
                                          int x, int y, int width, int height)
{
    PFNGLXCOPYSUBBUFFERMESAPROC pCopySubBufferMESA;
    __GLXvendorInfo *dd;

    dd = GetDispatchFromDrawable(dpy, drawable);
    if (dd == NULL)
        return;

    __FETCH_FUNCTION_PTR(CopySubBufferMESA);
    if (pCopySubBufferMESA == NULL)
        return;

    pCopySubBufferMESA(dpy, drawable, x, y, width, height);
}



static GLXPixmap dispatch_CreateGLXPixmapMESA(Display *dpy,
                                                 XVisualInfo *visinfo,
                                                 Pixmap pixmap, Colormap cmap)
{
    PFNGLXCREATEGLXPIXMAPMESAPROC pCreateGLXPixmapMESA;
    __GLXvendorInfo *dd;
    GLXPixmap ret;

    dd = GetDispatchFromVisual(dpy, visinfo);
    if (dd == NULL)
        return None;

    __FETCH_FUNCTION_PTR(CreateGLXPixmapMESA);
    if (pCreateGLXPixmapMESA == NULL)
        return None;

    ret = pCreateGLXPixmapMESA(dpy, visinfo, pixmap, cmap);
    if (AddDrawableMapping(dpy, ret, dd)) {
        /* XXX: Call glXDestroyGLXPixmap which lives in libglvnd. If we're not
         * allowed to call it from here, should we extend __glXDispatchTableIndices ?
         */
        return None;
    }

    return ret;
}



static GLboolean dispatch_GetMscRateOML(Display *dpy, GLXDrawable drawable,
                                           int32_t *numerator, int32_t *denominator)
{
    PFNGLXGETMSCRATEOMLPROC pGetMscRateOML;
    __GLXvendorInfo *dd;

    dd = GetDispatchFromDrawable(dpy, drawable);
    if (dd == NULL)
        return GL_FALSE;

    __FETCH_FUNCTION_PTR(GetMscRateOML);
    if (pGetMscRateOML == NULL)
        return GL_FALSE;

    return pGetMscRateOML(dpy, drawable, numerator, denominator);
}



static const char *dispatch_GetScreenDriver(Display *dpy, int scrNum)
{
    typedef const char *(*fn_glXGetScreenDriver_ptr)(Display *dpy, int scrNum);
    fn_glXGetScreenDriver_ptr pGetScreenDriver;
    __GLXvendorInfo *dd;

    dd = __VND->getDynDispatch(dpy, scrNum);
    if (dd == NULL)
        return NULL;

    __FETCH_FUNCTION_PTR(GetScreenDriver);
    if (pGetScreenDriver == NULL)
        return NULL;

    return pGetScreenDriver(dpy, scrNum);
}



static int dispatch_GetSwapIntervalMESA(void)
{
    PFNGLXGETSWAPINTERVALMESAPROC pGetSwapIntervalMESA;
    __GLXvendorInfo *dd;

    if (!__VND->getCurrentContext())
        return GLX_BAD_CONTEXT;

    dd = __VND->getCurrentDynDispatch();
    if (dd == NULL)
        return 0;

    __FETCH_FUNCTION_PTR(GetSwapIntervalMESA);
    if (pGetSwapIntervalMESA == NULL)
        return 0;

    return pGetSwapIntervalMESA();
}



static Bool dispatch_GetSyncValuesOML(Display *dpy, GLXDrawable drawable,
                                         int64_t *ust, int64_t *msc, int64_t *sbc)
{
    PFNGLXGETSYNCVALUESOMLPROC pGetSyncValuesOML;
    __GLXvendorInfo *dd;

    dd = GetDispatchFromDrawable(dpy, drawable);
    if (dd == NULL)
        return False;

    __FETCH_FUNCTION_PTR(GetSyncValuesOML);
    if (pGetSyncValuesOML == NULL)
        return False;

    return pGetSyncValuesOML(dpy, drawable, ust, msc, sbc);
}



static void dispatch_JoinSwapGroupSGIX(Display *dpy, GLXDrawable drawable,
                                          GLXDrawable member)
{
    PFNGLXJOINSWAPGROUPSGIXPROC pJoinSwapGroupSGIX;
    __GLXvendorInfo *dd;

    dd = GetDispatchFromDrawable(dpy, drawable);
    if (dd == NULL)
        return;

    __FETCH_FUNCTION_PTR(JoinSwapGroupSGIX);
    if (pJoinSwapGroupSGIX == NULL)
        return;

    pJoinSwapGroupSGIX(dpy, drawable, member);
}



static Bool dispatch_QueryCurrentRendererIntegerMESA(int attribute,
                                                        unsigned int *value)
{
    PFNGLXQUERYCURRENTRENDERERINTEGERMESAPROC pQueryCurrentRendererIntegerMESA;
    __GLXvendorInfo *dd;

    if (!__VND->getCurrentContext())
        return False;

    dd = __VND->getCurrentDynDispatch();
    if (dd == NULL)
        return False;

    __FETCH_FUNCTION_PTR(QueryCurrentRendererIntegerMESA);
    if (pQueryCurrentRendererIntegerMESA == NULL)
        return False;

    return pQueryCurrentRendererIntegerMESA(attribute, value);
}



static const char *dispatch_QueryCurrentRendererStringMESA(int attribute)
{
    PFNGLXQUERYCURRENTRENDERERSTRINGMESAPROC pQueryCurrentRendererStringMESA;
    __GLXvendorInfo *dd;

    if (!__VND->getCurrentContext())
        return NULL;

    dd = __VND->getCurrentDynDispatch();
    if (dd == NULL)
        return NULL;

    __FETCH_FUNCTION_PTR(QueryCurrentRendererStringMESA);
    if (pQueryCurrentRendererStringMESA == NULL)
        return NULL;

    return pQueryCurrentRendererStringMESA(attribute);
}



static Bool dispatch_QueryMaxSwapBarriersSGIX(Display *dpy, int screen,
                                                 int *max)
{
    PFNGLXQUERYMAXSWAPBARRIERSSGIXPROC pQueryMaxSwapBarriersSGIX;
    __GLXvendorInfo *dd;

    dd = __VND->getDynDispatch(dpy, screen);
    if (dd == NULL)
        return False;

    __FETCH_FUNCTION_PTR(QueryMaxSwapBarriersSGIX);
    if (pQueryMaxSwapBarriersSGIX == NULL)
        return False;

    return pQueryMaxSwapBarriersSGIX(dpy, screen, max);
}



static Bool dispatch_QueryRendererIntegerMESA(Display *dpy, int screen,
                                                 int renderer, int attribute,
                                                 unsigned int *value)
{
    PFNGLXQUERYRENDERERINTEGERMESAPROC pQueryRendererIntegerMESA;
    __GLXvendorInfo *dd;

    dd = __VND->getDynDispatch(dpy, screen);
    if (dd == NULL)
        return False;

    __FETCH_FUNCTION_PTR(QueryRendererIntegerMESA);
    if (pQueryRendererIntegerMESA == NULL)
        return False;

    return pQueryRendererIntegerMESA(dpy, screen, renderer, attribute, value);
}



static const char *dispatch_QueryRendererStringMESA(Display *dpy, int screen,
                                                       int renderer, int attribute)
{
    PFNGLXQUERYRENDERERSTRINGMESAPROC pQueryRendererStringMESA;
    __GLXvendorInfo *dd = NULL;

    dd = __VND->getDynDispatch(dpy, screen);
    if (dd == NULL)
        return NULL;

    __FETCH_FUNCTION_PTR(QueryRendererStringMESA);
    if (pQueryRendererStringMESA == NULL)
        return NULL;

    return pQueryRendererStringMESA(dpy, screen, renderer, attribute);
}



static Bool dispatch_ReleaseBuffersMESA(Display *dpy, GLXDrawable d)
{
    PFNGLXRELEASEBUFFERSMESAPROC pReleaseBuffersMESA;
    __GLXvendorInfo *dd;

    dd = GetDispatchFromDrawable(dpy, d);
    if (dd == NULL)
        return False;

    __FETCH_FUNCTION_PTR(ReleaseBuffersMESA);
    if (pReleaseBuffersMESA == NULL)
        return False;

    return pReleaseBuffersMESA(dpy, d);
}



static int64_t dispatch_SwapBuffersMscOML(Display *dpy, GLXDrawable drawable,
                                             int64_t target_msc, int64_t divisor,
                                             int64_t remainder)
{
    PFNGLXSWAPBUFFERSMSCOMLPROC pSwapBuffersMscOML;
    __GLXvendorInfo *dd;

    dd = GetDispatchFromDrawable(dpy, drawable);
    if (dd == NULL)
        return 0;

    __FETCH_FUNCTION_PTR(SwapBuffersMscOML);
    if (pSwapBuffersMscOML == NULL)
        return 0;

    return pSwapBuffersMscOML(dpy, drawable, target_msc, divisor, remainder);
}



static int dispatch_SwapIntervalMESA(unsigned int interval)
{
    PFNGLXSWAPINTERVALMESAPROC pSwapIntervalMESA;
    __GLXvendorInfo *dd;

    if (!__VND->getCurrentContext())
        return GLX_BAD_CONTEXT;

    dd = __VND->getCurrentDynDispatch();
    if (dd == NULL)
        return 0;

    __FETCH_FUNCTION_PTR(SwapIntervalMESA);
    if (pSwapIntervalMESA == NULL)
        return 0;

    return pSwapIntervalMESA(interval);
}



static void dispatch_SwapIntervalEXT(Display *dpy, GLXDrawable drawable, int interval)
{
    PFNGLXSWAPINTERVALEXTPROC pSwapIntervalEXT;
    __GLXvendorInfo *dd;

    dd = GetDispatchFromDrawable(dpy, drawable);
    if (dd == NULL)
        return;

    __FETCH_FUNCTION_PTR(SwapIntervalEXT);
    if (pSwapIntervalEXT == NULL)
        return;

    pSwapIntervalEXT(dpy, drawable, interval);
}



static Bool dispatch_WaitForMscOML(Display *dpy, GLXDrawable drawable,
                                      int64_t target_msc, int64_t divisor,
                                      int64_t remainder, int64_t *ust,
                                      int64_t *msc, int64_t *sbc)
{
    PFNGLXWAITFORMSCOMLPROC pWaitForMscOML;
    __GLXvendorInfo *dd;

    dd = GetDispatchFromDrawable(dpy, drawable);
    if (dd == NULL)
        return False;

    __FETCH_FUNCTION_PTR(WaitForMscOML);
    if (pWaitForMscOML == NULL)
        return False;

    return pWaitForMscOML(dpy, drawable, target_msc, divisor, remainder, ust, msc, sbc);
}



static Bool dispatch_WaitForSbcOML(Display *dpy, GLXDrawable drawable,
                                      int64_t target_sbc, int64_t *ust,
                                      int64_t *msc, int64_t *sbc)
{
    PFNGLXWAITFORSBCOMLPROC pWaitForSbcOML;
    __GLXvendorInfo *dd;

    dd = GetDispatchFromDrawable(dpy, drawable);
    if (dd == NULL)
        return False;

    __FETCH_FUNCTION_PTR(WaitForSbcOML);
    if (pWaitForSbcOML == NULL)
        return False;

    return pWaitForSbcOML(dpy, drawable, target_sbc, ust, msc, sbc);
}

#undef __FETCH_FUNCTION_PTR


/* Allocate an extra 'dummy' to ease lookup. See FindGLXFunction() */
const void * const __glXDispatchFunctions[DI_LAST_INDEX + 1] = {
#define __ATTRIB(field) \
    [DI_##field] = (void *)dispatch_##field

    __ATTRIB(BindSwapBarrierSGIX),
    __ATTRIB(BindTexImageEXT),
    __ATTRIB(ChooseFBConfigSGIX),
    __ATTRIB(CopySubBufferMESA),
    __ATTRIB(CreateContextAttribsARB),
    __ATTRIB(CreateContextWithConfigSGIX),
    __ATTRIB(CreateGLXPbufferSGIX),
    __ATTRIB(CreateGLXPixmapMESA),
    __ATTRIB(CreateGLXPixmapWithConfigSGIX),
    __ATTRIB(DestroyGLXPbufferSGIX),
    __ATTRIB(GLInteropExportObjectMESA),
    __ATTRIB(GLInteropFlushObjectsMESA),
    __ATTRIB(GLInteropQueryDeviceInfoMESA),
    __ATTRIB(GetContextIDEXT),
    __ATTRIB(GetCurrentDisplayEXT),
    __ATTRIB(GetDriverConfig),
    __ATTRIB(GetFBConfigAttribSGIX),
    __ATTRIB(GetFBConfigFromVisualSGIX),
    __ATTRIB(GetMscRateOML),
    __ATTRIB(GetScreenDriver),
    __ATTRIB(GetSelectedEventSGIX),
    __ATTRIB(GetSwapIntervalMESA),
    __ATTRIB(GetSyncValuesOML),
    __ATTRIB(GetVideoSyncSGI),
    __ATTRIB(GetVisualFromFBConfigSGIX),
    __ATTRIB(JoinSwapGroupSGIX),
    __ATTRIB(QueryContextInfoEXT),
    __ATTRIB(QueryCurrentRendererIntegerMESA),
    __ATTRIB(QueryCurrentRendererStringMESA),
    __ATTRIB(QueryGLXPbufferSGIX),
    __ATTRIB(QueryMaxSwapBarriersSGIX),
    __ATTRIB(QueryRendererIntegerMESA),
    __ATTRIB(QueryRendererStringMESA),
    __ATTRIB(ReleaseBuffersMESA),
    __ATTRIB(ReleaseTexImageEXT),
    __ATTRIB(SelectEventSGIX),
    __ATTRIB(SwapBuffersMscOML),
    __ATTRIB(SwapIntervalEXT),
    __ATTRIB(SwapIntervalMESA),
    __ATTRIB(SwapIntervalSGI),
    __ATTRIB(WaitForMscOML),
    __ATTRIB(WaitForSbcOML),
    __ATTRIB(WaitVideoSyncSGI),

    [DI_LAST_INDEX] = NULL,
#undef __ATTRIB
};
