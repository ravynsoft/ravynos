# (C) Copyright 2016, NVIDIA CORPORATION.
# All Rights Reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# on the rights to use, copy, modify, merge, publish, distribute, sub
# license, and/or sell copies of the Software, and to permit persons to whom
# the Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
# IBM AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#
# Authors:
#    Kyle Brenneman <kbrenneman@nvidia.com>

"""
Contains a list of EGL functions to generate dispatch functions for.

This is used from gen_egl_dispatch.py.

EGL_FUNCTIONS is a sequence of (name, eglData) pairs, where name is the name
of the function, and eglData is a dictionary containing data about that
function.

The values in the eglData dictionary are:
- method (string):
    How to select a vendor library. See "Method values" below.

- prefix (string):
    This string is prepended to the name of the dispatch function. If
    unspecified, the default is "" (an empty string).

- static (boolean)
  If True, this function should be declared static.

- "public" (boolean)
    If True, the function should be exported from the library. Vendor libraries
    generally should not use this.

- extension (string):
    If specified, this is the name of a macro to check for before defining a
    function. Used for checking for extension macros and such.

- retval (string):
    If specified, this is a C expression with the default value to return if we
    can't find a function to call. By default, it will try to guess from the
    return type: EGL_NO_whatever for the various handle types, NULL for
    pointers, and zero for everything else.

method values:
- "custom"
    The dispatch stub will be hand-written instead of generated.

- "none"
    No dispatch function exists at all, but the function should still have an
    entry in the index array. This is for other functions that a stub may need
    to call that are implemented in libEGL itself.

- "display"
    Select a vendor from an EGLDisplay argument.

- "device"
    Select a vendor from an EGLDeviceEXT argument.

- "current"
    Select the vendor that owns the current context.
"""

def _eglFunc(name, method, static=None, public=False, inheader=None, prefix="dispatch_", extension=None, retval=None):
    """
    A convenience function to define an entry in the EGL function list.
    """
    if static is None:
        static = (not public and method != "custom")
    if inheader is None:
        inheader = (not static)
    values = {
        "method" : method,
        "prefix" : prefix,
        "extension" : extension,
        "retval" : retval,
        "static" : static,
        "public" : public,
        "inheader" : inheader,
    }
    return (name, values)

EGL_FUNCTIONS = (
    # EGL_VERSION_1_0
    _eglFunc("eglChooseConfig",                      "none"),
    _eglFunc("eglCopyBuffers",                       "none"),
    _eglFunc("eglCreateContext",                     "none"),
    _eglFunc("eglCreatePbufferSurface",              "none"),
    _eglFunc("eglCreatePixmapSurface",               "none"),
    _eglFunc("eglCreateWindowSurface",               "none"),
    _eglFunc("eglDestroyContext",                    "none"),
    _eglFunc("eglDestroySurface",                    "none"),
    _eglFunc("eglGetConfigAttrib",                   "none"),
    _eglFunc("eglGetConfigs",                        "none"),
    _eglFunc("eglQueryContext",                      "none"),
    _eglFunc("eglQuerySurface",                      "none"),
    _eglFunc("eglSwapBuffers",                       "none"),
    _eglFunc("eglWaitGL",                            "none"),
    _eglFunc("eglWaitNative",                        "none"),
    _eglFunc("eglTerminate",                         "none"),
    _eglFunc("eglInitialize",                        "none"),

    _eglFunc("eglGetCurrentDisplay",                 "none"),
    _eglFunc("eglGetCurrentSurface",                 "none"),
    _eglFunc("eglGetDisplay",                        "none"),
    _eglFunc("eglGetError",                          "none"),
    _eglFunc("eglGetProcAddress",                    "none"),
    _eglFunc("eglMakeCurrent",                       "none"),
    _eglFunc("eglQueryString",                       "none"),

    # EGL_VERSION_1_1
    _eglFunc("eglBindTexImage",                      "none"),
    _eglFunc("eglReleaseTexImage",                   "none"),
    _eglFunc("eglSurfaceAttrib",                     "none"),
    _eglFunc("eglSwapInterval",                      "none"),

    # EGL_VERSION_1_2
    _eglFunc("eglCreatePbufferFromClientBuffer",     "none"),
    _eglFunc("eglWaitClient",                        "none"),
    _eglFunc("eglBindAPI",                           "none"),
    _eglFunc("eglQueryAPI",                          "none"),
    _eglFunc("eglReleaseThread",                     "none"),

    # EGL_VERSION_1_4
    _eglFunc("eglGetCurrentContext",                 "none"),

    # EGL_VERSION_1_5
    _eglFunc("eglCreateSync",                        "none"),
    _eglFunc("eglDestroySync",                       "none"),
    _eglFunc("eglClientWaitSync",                    "none"),
    _eglFunc("eglGetSyncAttrib",                     "none"),
    _eglFunc("eglCreateImage",                       "none"),
    _eglFunc("eglDestroyImage",                      "none"),
    _eglFunc("eglCreatePlatformWindowSurface",       "none"),
    _eglFunc("eglCreatePlatformPixmapSurface",       "none"),
    _eglFunc("eglWaitSync",                          "none"),
    _eglFunc("eglGetPlatformDisplay",                "none"),

    # EGL_EXT_platform_base
    _eglFunc("eglCreatePlatformWindowSurfaceEXT",    "display"),
    _eglFunc("eglCreatePlatformPixmapSurfaceEXT",    "display"),
    _eglFunc("eglGetPlatformDisplayEXT",             "none"),

    # TODO: Most of these extensions should be provided by the vendor
    # libraries, not by libEGL. They're here now to make testing everything
    # else easier.

    # EGL_EXT_swap_buffers_with_damage
    _eglFunc("eglSwapBuffersWithDamageEXT",          "display"),

    # KHR_EXT_swap_buffers_with_damage
    _eglFunc("eglSwapBuffersWithDamageKHR",          "display"),

    # EGL_KHR_cl_event2
    _eglFunc("eglCreateSync64KHR",                   "display"),

    # EGL_KHR_fence_sync
    _eglFunc("eglCreateSyncKHR",                     "display"),
    _eglFunc("eglDestroySyncKHR",                    "display"),
    _eglFunc("eglClientWaitSyncKHR",                 "display"),
    _eglFunc("eglGetSyncAttribKHR",                  "display"),

    # EGL_KHR_image
    _eglFunc("eglCreateImageKHR",                    "display"),
    _eglFunc("eglDestroyImageKHR",                   "display"),

    # EGL_KHR_image_base
    # eglCreateImageKHR already defined in EGL_KHR_image
    # eglDestroyImageKHR already defined in EGL_KHR_image

    # EGL_KHR_reusable_sync
    _eglFunc("eglSignalSyncKHR",                     "display"),
    # eglCreateSyncKHR already defined in EGL_KHR_fence_sync
    # eglDestroySyncKHR already defined in EGL_KHR_fence_sync
    # eglClientWaitSyncKHR already defined in EGL_KHR_fence_sync
    # eglGetSyncAttribKHR already defined in EGL_KHR_fence_sync

    # EGL_KHR_wait_sync
    _eglFunc("eglWaitSyncKHR",                       "display"),

    # EGL_MESA_drm_image
    _eglFunc("eglCreateDRMImageMESA",                "display"),
    _eglFunc("eglExportDRMImageMESA",                "display"),

    # EGL_MESA_image_dma_buf_export
    _eglFunc("eglExportDMABUFImageQueryMESA",        "display"),
    _eglFunc("eglExportDMABUFImageMESA",             "display"),

    # EGL_NOK_swap_region
    _eglFunc("eglSwapBuffersRegionNOK",              "display"),

    # EGL_NV_post_sub_buffer
    _eglFunc("eglPostSubBufferNV",                   "display"),

    # EGL_WL_bind_wayland_display
    _eglFunc("eglCreateWaylandBufferFromImageWL",    "display"),
    _eglFunc("eglUnbindWaylandDisplayWL",            "display"),
    _eglFunc("eglQueryWaylandBufferWL",              "display"),
    _eglFunc("eglBindWaylandDisplayWL",              "display"),

    # EGL_CHROMIUM_get_sync_values
    _eglFunc("eglGetSyncValuesCHROMIUM",             "display"),

    # EGL_ANGLE_sync_control_rate
    _eglFunc("eglGetMscRateANGLE",                   "display"),

    # EGL_ANDROID_native_fence_sync
    _eglFunc("eglDupNativeFenceFDANDROID",           "display"),

    # EGL_ANDROID_blob_cache
    _eglFunc("eglSetBlobCacheFuncsANDROID",          "display"),

    # EGL_EXT_image_dma_buf_import_modifiers
    _eglFunc("eglQueryDmaBufFormatsEXT",             "display"),
    _eglFunc("eglQueryDmaBufModifiersEXT",           "display"),

    # EGL_EXT_device_base
    _eglFunc("eglQueryDeviceAttribEXT",              "device"),
    _eglFunc("eglQueryDeviceStringEXT",              "device"),
    _eglFunc("eglQueryDevicesEXT",                   "none"),
    _eglFunc("eglQueryDisplayAttribEXT",             "display"),

    # EGL_MESA_query_driver
    _eglFunc("eglGetDisplayDriverName",              "display"),
    _eglFunc("eglGetDisplayDriverConfig",            "display"),

    # EGL_KHR_partial_update
    _eglFunc("eglSetDamageRegionKHR",                "display"),

    # EGL_MESA_gl_interop
    _eglFunc("eglGLInteropQueryDeviceInfoMESA",      "display"),
    _eglFunc("eglGLInteropExportObjectMESA",         "display"),
    _eglFunc("eglGLInteropFlushObjectsMESA",         "display"),
)

