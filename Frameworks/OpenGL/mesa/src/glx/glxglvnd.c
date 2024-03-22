#include <string.h>
#include <stdlib.h>
#include <X11/Xlib.h>

#include "glvnd/libglxabi.h"

#include "glxglvnd.h"

static Bool __glXGLVNDIsScreenSupported(Display *dpy, int screen)
{
    /* TODO: Think of a better heuristic... */
    return True;
}

static void *__glXGLVNDGetProcAddress(const GLubyte *procName)
{
    return glXGetProcAddressARB(procName);
}

static int
compare(const void *l, const void *r)
{
    const char *s = *(const char **)r;
    return strcmp(l, s);
}

static unsigned FindGLXFunction(const GLubyte *name)
{
    const char **match;

    match = bsearch(name, __glXDispatchTableStrings, DI_FUNCTION_COUNT,
                    sizeof(const char *), compare);

    if (match == NULL)
        return DI_FUNCTION_COUNT;

    return match - __glXDispatchTableStrings;
}

static void *__glXGLVNDGetDispatchAddress(const GLubyte *procName)
{
    unsigned internalIndex = FindGLXFunction(procName);

    return (void*)__glXDispatchFunctions[internalIndex];
}

static void __glXGLVNDSetDispatchIndex(const GLubyte *procName, int index)
{
    unsigned internalIndex = FindGLXFunction(procName);

    if (internalIndex == DI_FUNCTION_COUNT)
        return; /* unknown or static dispatch */

    __glXDispatchTableIndices[internalIndex] = index;
}

_X_EXPORT Bool __glx_Main(uint32_t version, const __GLXapiExports *exports,
                          __GLXvendorInfo *vendor, __GLXapiImports *imports)
{
    static Bool initDone = False;

    if (GLX_VENDOR_ABI_GET_MAJOR_VERSION(version) !=
        GLX_VENDOR_ABI_MAJOR_VERSION ||
        GLX_VENDOR_ABI_GET_MINOR_VERSION(version) <
        GLX_VENDOR_ABI_MINOR_VERSION)
        return False;

    if (!initDone) {
        initDone = True;
        __glXGLVNDAPIExports = exports;

        imports->isScreenSupported = __glXGLVNDIsScreenSupported;
        imports->getProcAddress = __glXGLVNDGetProcAddress;
        imports->getDispatchAddress = __glXGLVNDGetDispatchAddress;
        imports->setDispatchIndex = __glXGLVNDSetDispatchIndex;
        imports->notifyError = NULL;
        imports->isPatchSupported = NULL;
        imports->initiatePatch = NULL;
    }

    return True;
}
