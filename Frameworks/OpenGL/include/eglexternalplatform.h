/*
 * Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef EGL_EXTERNAL_PLATFORM_H
#define EGL_EXTERNAL_PLATFORM_H

#import <OpenGL/egl.h>
#import <OpenGL/eglext.h>
#include "eglexternalplatformversion.h"

/* EGL external platform interface objects */
typedef struct EGLExtPlatformExports EGLExtPlatformExports;
typedef struct EGLExtPlatform        EGLExtPlatform;
typedef struct EGLExtDriver          EGLExtDriver;

/* EGLExtPlatformString enum. This indicates what string to query through
 * queryString() */
typedef enum {
    /*
     * Platform client extensions
     *
     * Returns a extension string including the specific platform client
     * extensions. E.g. EGL_EXT_platform_wayland on Wayland platform.
     *
     * The <dpy> parameter is EGL_NO_DISPLAY.
     */
    EGL_EXT_PLATFORM_PLATFORM_CLIENT_EXTENSIONS = 0,

    /*
     * Display extensions
     *
     * Returns a extension string including all display extensions supported on
     * the given display by the external platform implementation.
     */
    EGL_EXT_PLATFORM_DISPLAY_EXTENSIONS,

    EGL_EXT_PLATFORM_STRING_NAME_COUNT
} EGLExtPlatformString;


/*
 * loadEGLExternalPlatform()
 *
 * Loads the EGL external platform and returns a EGLExtPlatform object where
 * external platform data and exported functions have been properly set
 * according to the given major and minor version numbers
 *
 * If a compatible external platform is found for <major>.<minor>, EGL_TRUE is
 * returned and <platform> is properly initialized. Otherwise, EGL_FALSE is
 * returned and <platform> remains unchanged
 *
 * A reference to the underlying EGL implementation that must be used is passed
 * in <driver>.
 */
typedef EGLBoolean (*PEGLEXTFNLOADEGLEXTERNALPLATFORM) (int major, int minor, const EGLExtDriver *driver, EGLExtPlatform *platform);

/*
 * unloadEGLExternalPlatform()
 *
 * Unloads the EGL external platform, freeing any resources associated to the
 * given platform data structure that may have been allocated and not yet freed.
 *
 * If all resources are properly freed, EGL_TRUE is returned and the given
 * platform data pointer becomes invalid. Otherwise, EGL_FALSE is returned.
 */
typedef EGLBoolean (*PEGLEXTFNUNLOADEGLEXTERNALPLATFORM) (void *platformData);

/*
 * getHookAddress()
 *
 * The EGL external platform interface defines a minimum set of functions that
 * must be provided by any EGL external platform at loadEGLExternalPlatform()
 * time.
 *
 * However, most of the other EGL functions can be overwritten by an EGL
 * external platform.
 *
 * The EGL implementation will call into getHookAddress() to retrieve any
 * additional EGL function that the external platform may implement. Its
 * behavior is comparable to eglGetProcAddress().
 *
 * Returns the hook address if the given functions is implemented; otherwise,
 * returns NULL.
 */
typedef void* (*PEGLEXTFNGETHOOKADDRESS) (void *platformData, const char *name);

/*
 * isValidNativeDisplay()
 *
 * Validity check function for a native display. It will return EGL_TRUE if the
 * given native display is valid and belongs to the external platform
 * implementation; otherwise, it will return EGL_FALSE.
 */
typedef EGLBoolean (*PEGLEXTFNISVALIDNATIVEDISPLAY) (void *platformData, void *nativeDisplay);

/*
 * getPlatformDisplay()
 *
 * Same as eglGetPlatformDisplay()
 */
typedef EGLDisplay (*PEGLEXTFNGETPLATFORMDISPLAY) (void *platformData, EGLenum platform, void *nativeDisplay, const EGLAttrib* attribs);

/*
 * queryString()
 *
 * Similar to eglQueryString(), but takes its own enumeration as the string name
 * parameter.
 *
 * Returns the appropriate extension string which is supported by the external
 * platform. See descriptions of EGLExtPlatformString enums for more details.
 */
typedef const char * (*PEGLEXTFNQUERYSTRING) (void *platformData, EGLDisplay dpy, EGLExtPlatformString name);

/*
 * getInternalHandle()
 *
 * Conversion function from an EGL external object handle to its corresponding
 * EGL internal one. It will return the internal EGL object handle if the given
 * external handle is valid and belongs to the given EGLDisplay; otherwise, it
 * will return NULL.
 *
 * Note that the object handle type must be provided by the caller. Its value
 * must be one of the object type enums as defined in EGL_KHR_debug.
 */
typedef void* (*PEGLEXTFNGETINTERNALHANDLE) (EGLDisplay dpy, EGLenum type, void *handle);

/*
 * getObjectLabel()
 *
 * Returns an EGL external object label previously attached with
 * eglLabelObjectKHR() from EGL_KHR_debug.
 *
 * Note that the object handle type must be provided by the caller. Its value
 * must be one of the object type enums as defined in EGL_KHR_debug.
 */
typedef void* (*PEGLEXTFNGETOBJECTLABEL) (EGLDisplay dpy, EGLenum type, void *handle);


/*
 * EGLExtPlatformExports definition. This is the exports table an external
 * platform must fill out and make available for the EGL implementation to use
 */
struct EGLExtPlatformExports {
    PEGLEXTFNUNLOADEGLEXTERNALPLATFORM unloadEGLExternalPlatform;
    PEGLEXTFNGETHOOKADDRESS            getHookAddress;

    PEGLEXTFNISVALIDNATIVEDISPLAY      isValidNativeDisplay;
    PEGLEXTFNGETPLATFORMDISPLAY        getPlatformDisplay;

    PEGLEXTFNQUERYSTRING               queryString;

    PEGLEXTFNGETINTERNALHANDLE         getInternalHandle;

    PEGLEXTFNGETOBJECTLABEL            getObjectLabel;
};

/*
 * EGLExtPlatform definition. This is common to all external platforms
 *
 * Fields:
 *  - major/minor: External platform major/minor version number. Specify the EGL
 *                 external platform interface version number the given platform
 *                 implements. They are tied to EGL external platform interface
 *                 changes.
 *  - micro:       External platform micro version number. Similar to
 *                 major/minor numbers, but it is tied to specific external
 *                 platform implementation changes.
 *  - platform:    EGL platform enumeration the corresponding external platform
 *                 implements.
 *  - data:        Opaque pointer to platform specific data. At platform
 *                 load time, the external platform can initialize its own data
 *                 structure to store any information that may be required by
 *                 any function that does not take an EGLDisplay or the display
 *                 belongs to another platform.
 *  - exports:     External platform exports table.
 */
struct EGLExtPlatform {
    struct {
        int major;
        int minor;
        int micro;
    }                      version;
    EGLenum                platform;
    void                  *data;
    EGLExtPlatformExports  exports;
};


/*
 * getProcAddress()
 *
 * Equivalent to eglGetProcAddress() to fetch EGL methods provided by a
 * specific EGL driver.
 */
typedef void* (*PEGLEXTFNGETPROCADDRESS) (const char *name);

/*
 * setError()
 *
 * Sets the last EGL error, which can be queried with eglGetError() later on. It
 * also calls the EGL_KHR_debug callback if such extension is supported by the
 * driver.
 *
 * Takes the EGL error code and both message type and string as defined in
 * EGL_KHR_debug for the debug callback function.
 */
typedef void (*PEGLEXTFNSETERROR) (EGLint error, EGLint msgType, const char *msg);

/*
 * debugMessage()
 *
 * Calls the EGL_KHR_debug callback if such extension is supported by the
 * driver.
 *
 * Takes both message type and string as defined in EGL_KHR_debug for the debug
 * callback function.
 */
typedef void* (*PEGLEXTFNDEBUGMESSAGE) (EGLint msgType, const char *msg);

/*
 * streamSwapInterval()
 *
 * Handle swapinterval on the EGLStream consumer side. Should be a noop for
 * any consumer that does not present directly to a display.
 *
 * Takes the stream handle and a pointer to the interval value as parameters.
 *
 * Returns one of the following EGL error codes:
 *  - EGL_SUCCESS:    The interval setting operation succeeded (or noop).
 *  - EGL_BAD_MATCH:  A server-side interval override is in place. The override
 *                    value is returned in the <interval> parameter.
 *  - EGL_BAD_ACCESS: The interval setting operation failed.
 */
typedef EGLint (*PEGLEXTFNSTREAMSWAPINTERVAL) (EGLStreamKHR stream, int *interval);

/*
 * EGLExtDriver definition. The EGL external driver is the component in
 * charge of dispatching EGL calls to the underlying EGL implementation.
 */
struct EGLExtDriver {
    PEGLEXTFNGETPROCADDRESS     getProcAddress;
    PEGLEXTFNSETERROR           setError;
    PEGLEXTFNDEBUGMESSAGE       debugMessage;
    PEGLEXTFNSTREAMSWAPINTERVAL streamSwapInterval;
#if EGL_EXTERNAL_PLATFORM_HAS(DRIVER_VERSION)
    int major;
    int minor;
#endif
};

#endif // EGL_EXTERNAL_PLATFORM_H
