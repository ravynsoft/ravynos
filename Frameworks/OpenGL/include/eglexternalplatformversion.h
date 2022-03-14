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

#ifndef EGL_EXTERNAL_PLATFORM_VERSION_H
#define EGL_EXTERNAL_PLATFORM_VERSION_H

/*
 * <EGL_EXTERNAL_PLATFORM_VERSION_MAJOR>.<EGL_EXTERNAL_PLATFORM_VERSION_MINOR>
 * defines the EGL external platform interface version.
 *
 * The includer of this file can override either
 * EGL_EXTERNAL_PLATFORM_VERSION_MAJOR or EGL_EXTERNAL_PLATFORM_VERSION_MINOR in
 * order to build against a certain EGL external platform interface version.
 *
 * Note that, if only EGL_EXTERNAL_PLATFORM_VERSION_MAJOR is overridden, the
 * least possible value for EGL_EXTERNAL_PLATFORM_VERSION_MINOR is taken.
 *
 *
 * How to update these version numbers:
 *
 *  - If a backwards-compatible change is made to the interface, increase
 *    EGL_EXTERNAL_PLATFORM_VERSION_MINOR by 1
 *
 *  - If backwards-compatibility is broken by a change, increase
 *    EGL_EXTERNAL_PLATFORM_VERSION_MAJOR by 1 and set
 *    EGL_EXTERNAL_PLATFORM_VERSION_MINOR to 0 (keep these kind of changes to
 *    the minimum)
 */
#if !defined(EGL_EXTERNAL_PLATFORM_VERSION_MAJOR)
 #define EGL_EXTERNAL_PLATFORM_VERSION_MAJOR                      1
 #if !defined(EGL_EXTERNAL_PLATFORM_VERSION_MINOR)
  #define EGL_EXTERNAL_PLATFORM_VERSION_MINOR                     1
 #endif
#elif !defined(EGL_EXTERNAL_PLATFORM_VERSION_MINOR)
 #define EGL_EXTERNAL_PLATFORM_VERSION_MINOR                      0
#endif


/*
 * EGL_EXTERNAL_PLATFORM_VERSION_CMP
 *
 * Helper macro to compare two different version numbers. It evaluates to true
 * if <_MAJOR1_>.<_MINOR1_> is compatible with <_MAJOR2_>.<_MINOR2_>
 */
#define EGL_EXTERNAL_PLATFORM_VERSION_CMP(_MAJOR1_, _MINOR1_, _MAJOR2_, _MINOR2_) \
    (((_MAJOR1_) == (_MAJOR2_)) && ((_MINOR1_) >= (_MINOR2_)))

/*
 * EGL_EXTERNAL_PLATFORM_VERSION_CHECK
 *
 * Helper macro to check whether the current EGL external platform interface
 * version is compatible with the given version number <_MAJOR_>.<_MINOR_>
 */
#define EGL_EXTERNAL_PLATFORM_VERSION_CHECK(_MAJOR_, _MINOR_)              \
    EGL_EXTERNAL_PLATFORM_VERSION_CMP(EGL_EXTERNAL_PLATFORM_VERSION_MAJOR, \
                                      EGL_EXTERNAL_PLATFORM_VERSION_MINOR, \
                                      _MAJOR_, _MINOR_)

/*
 * EGL_EXTERNAL_PLATFORM_HAS
 *
 * Helper macro to check whether the current EGL external platform interface
 * version implements the given feature <_FEATURE_>
 */
#define EGL_EXTERNAL_PLATFORM_HAS(_FEATURE_)                                                 \
    EGL_EXTERNAL_PLATFORM_VERSION_CHECK(EGL_EXTERNAL_PLATFORM_ ## _FEATURE_ ## _SINCE_MAJOR, \
                                        EGL_EXTERNAL_PLATFORM_ ## _FEATURE_ ## _SINCE_MINOR)

/*
 * EGL_EXTERNAL_PLATFORM_SUPPORTS
 *
 * Helper macro to check whether the given EGL external platform interface
 * version number <_MAJOR_>.<_MINOR_> supports the given feature <_FEATURE_>
 */
#define EGL_EXTERNAL_PLATFORM_SUPPORTS(_MAJOR_, _MINOR_, _FEATURE_)                        \
    EGL_EXTERNAL_PLATFORM_VERSION_CMP(_MAJOR_, _MINOR_,                                    \
                                      EGL_EXTERNAL_PLATFORM_ ## _FEATURE_ ## _SINCE_MAJOR, \
                                      EGL_EXTERNAL_PLATFORM_ ## _FEATURE_ ## _SINCE_MINOR)

/*
 * List of supported features
 *
 * Whenever a new feature/function is added to the EGL external platform
 * interface, along with the corresponding version number bump, a pair of
 * <EGL_EXTERNAL_PLATFORM_"FEATURE
 * NAME"_SINCE_MAJOR>.<EGL_EXTERNAL_PLATFORM_"FEATURE NAME"_SINCE_MINOR> numbers
 * must be added.
 *
 * All new symbols and usages of the new feature/function must be protected with
 * EGL_EXTERNAL_PLATFORM_HAS(<feature-name>).
 *
 * Additionally, any external platform implementation that supports the new
 * feature/function, must also protect the corresponding export initialization
 * in function 'loadEGLExternalPlatform()' with
 * EGL_EXTERNAL_PLATFORM_SUPPORTS(<major>, <minor>, <feature-name>) using the
 * given version number.
 *
 * Example:
 *
 *    In eglexternalplatformversion.h:
 *
 *        #define EGL_EXTERNAL_PLATFORM_FOO_SINCE_MAJOR 1
 *        #define EGL_EXTERNAL_PLATFORM_FOO_SINCE_MINOR 0
 *
 *    In eglexternalplatform.h:
 *
 *        #if EGL_EXTERNAL_PLATFORM_HAS(FOO)
 *        typedef void* (*PEGLEXTFNFOO)(void *fooAttr);
 *        #endif
 *
 *        sitruct EGLExtPlatformExports {
 *            [...]
 *
 *        #if EGL_EXTERNAL_PLATFORM_HAS(FOO)
 *            PEGLEXTFNFOO foo;
 *        #endif
 *        };
 *
 *    In platform's loadEGLExternalPlatform() implementation:
 *
 *        EGLBoolean loadEGLExternalPlatform(int major, int minor,
 *                                           const EGLExtDriver *driver,
 *                                           EGLExtPlatform *platform)
 *        {
 *            if (!EGL_EXTERNAL_PLATFORM_VERSION_CHECK(major, minor)) {
 *                return EGL_FALSE;
 *            }
 *
 *            [...]
 *
 *        #if EGL_EXTERNAL_PLATFORM_HAS(FOO)
 *            if (EGL_EXTERNAL_PLATFORM_SUPPORTS(major, minor, FOO)) {
 *                platform->exports.foo = fooImpl;
 *            }
 *        #endif
 *
 *            [...]
 *        }
 */

/*
 * DRIVER_VERSION
 *
 * <major> and <minor> fields added to EGLExtDriver for drivers to let the
 * external platform know the supported EGL version
 */
#define EGL_EXTERNAL_PLATFORM_DRIVER_VERSION_SINCE_MAJOR 1
#define EGL_EXTERNAL_PLATFORM_DRIVER_VERSION_SINCE_MINOR 1

#endif // EGL_EXTERNAL_PLATFORM_VERSION_H
