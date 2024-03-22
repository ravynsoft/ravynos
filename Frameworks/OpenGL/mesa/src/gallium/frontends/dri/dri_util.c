/*
 * (C) Copyright IBM Corporation 2002, 2004
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * \file dri_util.c
 * DRI utility functions.
 *
 * This module acts as glue between GLX and the actual hardware driver.  A DRI
 * driver doesn't really \e have to use any of this - it's optional.  But, some
 * useful stuff is done here that otherwise would have to be duplicated in most
 * drivers.
 *
 * Basically, these utility functions take care of some of the dirty details of
 * screen initialization, context creation, context binding, DRM setup, etc.
 *
 * These functions are compiled into each DRI driver so libGL.so knows nothing
 * about them.
 */


#include <stdbool.h>
#include "dri_util.h"
#include "dri_context.h"
#include "dri_screen.h"
#include "dri_drawable.h"
#include "util/u_endian.h"
#include "util/u_memory.h"
#include "util/driconf.h"
#include "main/framebuffer.h"
#include "main/version.h"
#include "main/debug_output.h"
#include "main/errors.h"
#include "loader/loader.h"
#include "GL/internal/mesa_interface.h"

driOptionDescription __dri2ConfigOptions[] = {
      DRI_CONF_SECTION_DEBUG
         DRI_CONF_GLX_EXTENSION_OVERRIDE()
         DRI_CONF_INDIRECT_GL_EXTENSION_OVERRIDE()
      DRI_CONF_SECTION_END

      DRI_CONF_SECTION_PERFORMANCE
         DRI_CONF_VBLANK_MODE(DRI_CONF_VBLANK_DEF_INTERVAL_1)
         DRI_CONF_BLOCK_ON_DEPLETED_BUFFERS(false)
      DRI_CONF_SECTION_END
};

/*****************************************************************/
/** \name Screen handling functions                              */
/*****************************************************************/
/*@{*/

static void
setupLoaderExtensions(struct dri_screen *screen,
                      const __DRIextension **extensions)
{
   static const struct dri_extension_match matches[] = {
       {__DRI_DRI2_LOADER, 1, offsetof(struct dri_screen, dri2.loader), true},
       {__DRI_IMAGE_LOOKUP, 1, offsetof(struct dri_screen, dri2.image), true},
       {__DRI_USE_INVALIDATE, 1, offsetof(struct dri_screen, dri2.useInvalidate), true},
       {__DRI_BACKGROUND_CALLABLE, 1, offsetof(struct dri_screen, dri2.backgroundCallable), true},
       {__DRI_SWRAST_LOADER, 1, offsetof(struct dri_screen, swrast_loader), true},
       {__DRI_IMAGE_LOADER, 1, offsetof(struct dri_screen, image.loader), true},
       {__DRI_MUTABLE_RENDER_BUFFER_LOADER, 1, offsetof(struct dri_screen, mutableRenderBuffer.loader), true},
       {__DRI_KOPPER_LOADER, 1, offsetof(struct dri_screen, kopper_loader), true},
   };
   loader_bind_extensions(screen, matches, ARRAY_SIZE(matches), extensions);
}

/**
 * This is the first entrypoint in the driver called by the DRI driver loader
 * after dlopen()ing it.
 *
 * It's used to create global state for the driver across contexts on the same
 * Display.
 */
__DRIscreen *
driCreateNewScreen2(int scrn, int fd,
                    const __DRIextension **loader_extensions,
                    const __DRIextension **driver_extensions,
                    const __DRIconfig ***driver_configs, void *data)
{
    static const __DRIextension *emptyExtensionList[] = { NULL };
    struct dri_screen *screen;
    const __DRImesaCoreExtension *mesa = NULL;

    screen = CALLOC_STRUCT(dri_screen);
    if (!screen)
       return NULL;

    assert(driver_extensions);
    for (int i = 0; driver_extensions[i]; i++) {
       if (strcmp(driver_extensions[i]->name, __DRI_MESA) == 0) {
          mesa = (__DRImesaCoreExtension *)driver_extensions[i];
       }
    }

    setupLoaderExtensions(screen, loader_extensions);
    // dri2 drivers require working invalidate
    if (fd != -1 && !screen->dri2.useInvalidate) {
       free(screen);
       return NULL;
    }

    screen->loaderPrivate = data;

    /* This will be filled in by mesa->initScreen(). */
    screen->extensions = emptyExtensionList;
    screen->fd = fd;
    screen->myNum = scrn;

    /* Option parsing before ->InitScreen(), as some options apply there. */
    driParseOptionInfo(&screen->optionInfo,
                       __dri2ConfigOptions, ARRAY_SIZE(__dri2ConfigOptions));
    driParseConfigFiles(&screen->optionCache, &screen->optionInfo, screen->myNum,
                        "dri2", NULL, NULL, NULL, 0, NULL, 0);

    *driver_configs = mesa->initScreen(screen);
    if (*driver_configs == NULL) {
        dri_destroy_screen(screen);
        return NULL;
    }

    struct gl_constants consts = { 0 };
    gl_api api;
    unsigned version;

    api = API_OPENGLES2;
    if (_mesa_override_gl_version_contextless(&consts, &api, &version))
       screen->max_gl_es2_version = version;

    api = API_OPENGL_COMPAT;
    if (_mesa_override_gl_version_contextless(&consts, &api, &version)) {
       screen->max_gl_core_version = version;
       if (api == API_OPENGL_COMPAT)
          screen->max_gl_compat_version = version;
    }

    screen->api_mask = 0;
    if (screen->max_gl_compat_version > 0)
       screen->api_mask |= (1 << __DRI_API_OPENGL);
    if (screen->max_gl_core_version > 0)
       screen->api_mask |= (1 << __DRI_API_OPENGL_CORE);
    if (screen->max_gl_es1_version > 0)
       screen->api_mask |= (1 << __DRI_API_GLES);
    if (screen->max_gl_es2_version > 0)
       screen->api_mask |= (1 << __DRI_API_GLES2);
    if (screen->max_gl_es2_version >= 30)
       screen->api_mask |= (1 << __DRI_API_GLES3);

    return opaque_dri_screen(screen);
}

static __DRIscreen *
dri2CreateNewScreen(int scrn, int fd,
                    const __DRIextension **extensions,
                    const __DRIconfig ***driver_configs, void *data)
{
   return driCreateNewScreen2(scrn, fd, extensions,
                              galliumdrm_driver_extensions,
                              driver_configs, data);
}

static __DRIscreen *
swkmsCreateNewScreen(int scrn, int fd,
                     const __DRIextension **extensions,
                     const __DRIconfig ***driver_configs, void *data)
{
   return driCreateNewScreen2(scrn, fd, extensions,
                              dri_swrast_kms_driver_extensions,
                              driver_configs, data);
}

/** swrast driver createNewScreen entrypoint. */
static __DRIscreen *
driSWRastCreateNewScreen(int scrn, const __DRIextension **extensions,
                         const __DRIconfig ***driver_configs, void *data)
{
   return driCreateNewScreen2(scrn, -1, extensions,
                              galliumsw_driver_extensions,
                              driver_configs, data);
}

static __DRIscreen *
driSWRastCreateNewScreen2(int scrn, const __DRIextension **extensions,
                          const __DRIextension **driver_extensions,
                          const __DRIconfig ***driver_configs, void *data)
{
   return driCreateNewScreen2(scrn, -1, extensions, driver_extensions,
                               driver_configs, data);
}

/**
 * Destroy the per-screen private information.
 *
 * \internal
 * This function calls __DriverAPIRec::DestroyScreen on \p screenPrivate, calls
 * drmClose(), and finally frees \p screenPrivate.
 */
static void driDestroyScreen(__DRIscreen *psp)
{
    if (psp) {
        /* No interaction with the X-server is possible at this point.  This
         * routine is called after XCloseDisplay, so there is no protocol
         * stream open to the X-server anymore.
         */

        dri_destroy_screen(dri_screen(psp));
    }
}

static const __DRIextension **driGetExtensions(__DRIscreen *psp)
{
    return dri_screen(psp)->extensions;
}

/*@}*/

/* WARNING: HACK: Local defines to avoid pulling glx.h.
 */
#define GLX_NONE                                                0x8000
#define GLX_DONT_CARE                                           0xFFFFFFFF

#define SIMPLE_CASE(attrib, field) case attrib: *value = config->modes.field; break

/**
 * Return the value of a configuration attribute.  The attribute is
 * indicated by the index.
 */
static int
driGetConfigAttribIndex(const __DRIconfig *config,
                        unsigned int index, unsigned int *value)
{
    switch (index + 1) {
    SIMPLE_CASE(__DRI_ATTRIB_BUFFER_SIZE, rgbBits);
    SIMPLE_CASE(__DRI_ATTRIB_RED_SIZE, redBits);
    SIMPLE_CASE(__DRI_ATTRIB_GREEN_SIZE, greenBits);
    SIMPLE_CASE(__DRI_ATTRIB_BLUE_SIZE, blueBits);
    case __DRI_ATTRIB_LEVEL:
    case __DRI_ATTRIB_LUMINANCE_SIZE:
    case __DRI_ATTRIB_AUX_BUFFERS:
        *value = 0;
        break;
    SIMPLE_CASE(__DRI_ATTRIB_ALPHA_SIZE, alphaBits);
    case __DRI_ATTRIB_ALPHA_MASK_SIZE:
        /* I have no idea what this value was ever meant to mean, it's
         * never been set to anything, just say 0.
         */
        *value = 0;
        break;
    SIMPLE_CASE(__DRI_ATTRIB_DEPTH_SIZE, depthBits);
    SIMPLE_CASE(__DRI_ATTRIB_STENCIL_SIZE, stencilBits);
    SIMPLE_CASE(__DRI_ATTRIB_ACCUM_RED_SIZE, accumRedBits);
    SIMPLE_CASE(__DRI_ATTRIB_ACCUM_GREEN_SIZE, accumGreenBits);
    SIMPLE_CASE(__DRI_ATTRIB_ACCUM_BLUE_SIZE, accumBlueBits);
    SIMPLE_CASE(__DRI_ATTRIB_ACCUM_ALPHA_SIZE, accumAlphaBits);
    case __DRI_ATTRIB_SAMPLE_BUFFERS:
        *value = !!config->modes.samples;
        break;
    SIMPLE_CASE(__DRI_ATTRIB_SAMPLES, samples);
    case __DRI_ATTRIB_RENDER_TYPE:
        /* no support for color index mode */
        *value = __DRI_ATTRIB_RGBA_BIT;
        if (config->modes.floatMode)
            *value |= __DRI_ATTRIB_FLOAT_BIT;
        break;
    case __DRI_ATTRIB_CONFIG_CAVEAT:
        if (config->modes.accumRedBits != 0)
            *value = __DRI_ATTRIB_SLOW_BIT;
        else
            *value = 0;
        break;
    case __DRI_ATTRIB_CONFORMANT:
        *value = GL_TRUE;
        break;
    SIMPLE_CASE(__DRI_ATTRIB_DOUBLE_BUFFER, doubleBufferMode);
    SIMPLE_CASE(__DRI_ATTRIB_STEREO, stereoMode);
    case __DRI_ATTRIB_TRANSPARENT_TYPE:
    case __DRI_ATTRIB_TRANSPARENT_INDEX_VALUE: /* horrible bc hack */
        *value = GLX_NONE;
        break;
    case __DRI_ATTRIB_TRANSPARENT_RED_VALUE:
    case __DRI_ATTRIB_TRANSPARENT_GREEN_VALUE:
    case __DRI_ATTRIB_TRANSPARENT_BLUE_VALUE:
    case __DRI_ATTRIB_TRANSPARENT_ALPHA_VALUE:
        *value = GLX_DONT_CARE;
        break;
    case __DRI_ATTRIB_FLOAT_MODE:
        *value = config->modes.floatMode;
        break;
    SIMPLE_CASE(__DRI_ATTRIB_RED_MASK, redMask);
    SIMPLE_CASE(__DRI_ATTRIB_GREEN_MASK, greenMask);
    SIMPLE_CASE(__DRI_ATTRIB_BLUE_MASK, blueMask);
    SIMPLE_CASE(__DRI_ATTRIB_ALPHA_MASK, alphaMask);
    case __DRI_ATTRIB_MAX_PBUFFER_WIDTH:
    case __DRI_ATTRIB_MAX_PBUFFER_HEIGHT:
    case __DRI_ATTRIB_MAX_PBUFFER_PIXELS:
    case __DRI_ATTRIB_OPTIMAL_PBUFFER_WIDTH:
    case __DRI_ATTRIB_OPTIMAL_PBUFFER_HEIGHT:
    case __DRI_ATTRIB_VISUAL_SELECT_GROUP:
        *value = 0;
        break;
    case __DRI_ATTRIB_SWAP_METHOD:
        /* Not supported any more, but we have the __DRI_ATTRIB still defined
         * for the X server's sake, and EGL will expect us to handle it because
         * it iterates all __DRI_ATTRIBs.
         */
        *value = __DRI_ATTRIB_SWAP_UNDEFINED;
        break;
    case __DRI_ATTRIB_MAX_SWAP_INTERVAL:
        *value = INT_MAX;
        break;
    case __DRI_ATTRIB_MIN_SWAP_INTERVAL:
        *value = 0;
        break;
    case __DRI_ATTRIB_BIND_TO_TEXTURE_RGB:
    case __DRI_ATTRIB_BIND_TO_TEXTURE_RGBA:
    case __DRI_ATTRIB_YINVERTED:
        *value = GL_TRUE;
        break;
    case __DRI_ATTRIB_BIND_TO_MIPMAP_TEXTURE:
        *value = GL_FALSE;
        break;
    case __DRI_ATTRIB_BIND_TO_TEXTURE_TARGETS:
        *value = __DRI_ATTRIB_TEXTURE_1D_BIT |
                 __DRI_ATTRIB_TEXTURE_2D_BIT |
                 __DRI_ATTRIB_TEXTURE_RECTANGLE_BIT;
        break;
    SIMPLE_CASE(__DRI_ATTRIB_FRAMEBUFFER_SRGB_CAPABLE, sRGBCapable);
    case __DRI_ATTRIB_MUTABLE_RENDER_BUFFER:
        *value = GL_FALSE;
        break;
    SIMPLE_CASE(__DRI_ATTRIB_RED_SHIFT, redShift);
    SIMPLE_CASE(__DRI_ATTRIB_GREEN_SHIFT, greenShift);
    SIMPLE_CASE(__DRI_ATTRIB_BLUE_SHIFT, blueShift);
    SIMPLE_CASE(__DRI_ATTRIB_ALPHA_SHIFT, alphaShift);
    default:
        /* XXX log an error or smth */
        return GL_FALSE;
    }

    return GL_TRUE;
}

/**
 * Get the value of a configuration attribute.
 * \param attrib  the attribute (one of the _DRI_ATTRIB_x tokens)
 * \param value  returns the attribute's value
 * \return 1 for success, 0 for failure
 */
static int
driGetConfigAttrib(const __DRIconfig *config,
                   unsigned int attrib, unsigned int *value)
{
    return driGetConfigAttribIndex(config, attrib - 1, value);
}

/**
 * Get a configuration attribute name and value, given an index.
 * \param index  which field of the __DRIconfig to query
 * \param attrib  returns the attribute name (one of the _DRI_ATTRIB_x tokens)
 * \param value  returns the attribute's value
 * \return 1 for success, 0 for failure
 */
static int
driIndexConfigAttrib(const __DRIconfig *config, int index,
                     unsigned int *attrib, unsigned int *value)
{
    if (driGetConfigAttribIndex(config, index, value)) {
        *attrib = index + 1;
        return GL_TRUE;
    }

    return GL_FALSE;
}

static int
validate_context_version(struct dri_screen *screen,
                         int mesa_api,
                         unsigned major_version,
                         unsigned minor_version)
{
   unsigned req_version = 10 * major_version + minor_version;
   unsigned max_version = 0;

   if (major_version == 0 || major_version > 4)
      return __DRI_CTX_ERROR_BAD_API;

   if (mesa_api == API_OPENGL_COMPAT) {
      if ((major_version == 4 && minor_version > 6) ||
          (major_version == 3 && minor_version > 3) ||
          (major_version == 2 && minor_version > 1) ||
          (major_version == 1 && minor_version > 5))
         return __DRI_CTX_ERROR_BAD_API;
      max_version = screen->max_gl_compat_version;
   } else if (mesa_api == API_OPENGLES) {
      if (major_version > 1 || minor_version > 1)
         return __DRI_CTX_ERROR_BAD_API;
      max_version = screen->max_gl_es1_version;
   } else if (mesa_api == API_OPENGLES2) {
      if ((major_version >  3) ||
          (major_version == 3 && minor_version > 2) ||
          (major_version == 2 && minor_version > 0) ||
          (major_version <  2))
         return __DRI_CTX_ERROR_BAD_API;
      max_version = screen->max_gl_es2_version;
   } else if (mesa_api == API_OPENGL_CORE) {
      if ((major_version == 4 && minor_version > 6) ||
          (major_version == 3 && minor_version > 3) ||
          (major_version < 3))
         return __DRI_CTX_ERROR_BAD_API;
      max_version = screen->max_gl_core_version;
   } else {
      return __DRI_CTX_ERROR_BAD_API;
   }

   if (max_version == 0)
      return __DRI_CTX_ERROR_BAD_VERSION;

   if (req_version > max_version)
      return __DRI_CTX_ERROR_BAD_VERSION;

   return __DRI_CTX_ERROR_SUCCESS;
}

/*****************************************************************/
/** \name Context handling functions                             */
/*****************************************************************/
/*@{*/

__DRIcontext *
driCreateContextAttribs(__DRIscreen *psp, int api,
                        const __DRIconfig *config,
                        __DRIcontext *shared,
                        unsigned num_attribs,
                        const uint32_t *attribs,
                        unsigned *error,
                        void *data)
{
    struct dri_screen *screen = dri_screen(psp);
    const struct gl_config *modes = (config != NULL) ? &config->modes : NULL;
    gl_api mesa_api;
    struct __DriverContextConfig ctx_config;

    ctx_config.major_version = 1;
    ctx_config.minor_version = 0;
    ctx_config.flags = 0;
    ctx_config.attribute_mask = 0;
    ctx_config.priority = __DRI_CTX_PRIORITY_MEDIUM;

    assert((num_attribs == 0) || (attribs != NULL));

    switch (api) {
    case __DRI_API_OPENGL:
        mesa_api = API_OPENGL_COMPAT;
        break;
    case __DRI_API_GLES:
        mesa_api = API_OPENGLES;
        break;
    case __DRI_API_GLES2:
    case __DRI_API_GLES3:
        mesa_api = API_OPENGLES2;
        break;
    case __DRI_API_OPENGL_CORE:
        mesa_api = API_OPENGL_CORE;
        break;
    default:
        *error = __DRI_CTX_ERROR_BAD_API;
        return NULL;
    }

    for (unsigned i = 0; i < num_attribs; i++) {
        switch (attribs[i * 2]) {
        case __DRI_CTX_ATTRIB_MAJOR_VERSION:
            ctx_config.major_version = attribs[i * 2 + 1];
            break;
        case __DRI_CTX_ATTRIB_MINOR_VERSION:
            ctx_config.minor_version = attribs[i * 2 + 1];
            break;
        case __DRI_CTX_ATTRIB_FLAGS:
            ctx_config.flags = attribs[i * 2 + 1];
            break;
        case __DRI_CTX_ATTRIB_RESET_STRATEGY:
            if (attribs[i * 2 + 1] != __DRI_CTX_RESET_NO_NOTIFICATION) {
                ctx_config.attribute_mask |=
                    __DRIVER_CONTEXT_ATTRIB_RESET_STRATEGY;
                ctx_config.reset_strategy = attribs[i * 2 + 1];
            } else {
                ctx_config.attribute_mask &=
                    ~__DRIVER_CONTEXT_ATTRIB_RESET_STRATEGY;
            }
            break;
        case __DRI_CTX_ATTRIB_PRIORITY:
            ctx_config.attribute_mask |= __DRIVER_CONTEXT_ATTRIB_PRIORITY;
            ctx_config.priority = attribs[i * 2 + 1];
            break;
        case __DRI_CTX_ATTRIB_RELEASE_BEHAVIOR:
            if (attribs[i * 2 + 1] != __DRI_CTX_RELEASE_BEHAVIOR_FLUSH) {
                ctx_config.attribute_mask |=
                    __DRIVER_CONTEXT_ATTRIB_RELEASE_BEHAVIOR;
                ctx_config.release_behavior = attribs[i * 2 + 1];
            } else {
                ctx_config.attribute_mask &=
                    ~__DRIVER_CONTEXT_ATTRIB_RELEASE_BEHAVIOR;
            }
            break;
        case __DRI_CTX_ATTRIB_NO_ERROR:
            if (attribs[i * 2 + 1] != 0) {
               ctx_config.attribute_mask |=
                  __DRIVER_CONTEXT_ATTRIB_NO_ERROR;
               ctx_config.no_error = attribs[i * 2 + 1];
            } else {
               ctx_config.attribute_mask &=
                  ~__DRIVER_CONTEXT_ATTRIB_NO_ERROR;
            }
            break;
        case __DRI_CTX_ATTRIB_PROTECTED:
           if (attribs[i * 2 + 1]) {
              ctx_config.attribute_mask |= __DRIVER_CONTEXT_ATTRIB_PROTECTED;
           } else {
              ctx_config.attribute_mask &= ~__DRIVER_CONTEXT_ATTRIB_PROTECTED;
           }
           break;
        default:
            /* We can't create a context that satisfies the requirements of an
             * attribute that we don't understand.  Return failure.
             */
            assert(!"Should not get here.");
            *error = __DRI_CTX_ERROR_UNKNOWN_ATTRIBUTE;
            return NULL;
        }
    }

    /* The specific Mesa driver may not support the GL_ARB_compatibilty
     * extension or the compatibility profile.  In that case, we treat an
     * API_OPENGL_COMPAT 3.1 as API_OPENGL_CORE. We reject API_OPENGL_COMPAT
     * 3.2+ in any case.
     */
    if (mesa_api == API_OPENGL_COMPAT &&
        ctx_config.major_version == 3 && ctx_config.minor_version == 1 &&
        screen->max_gl_compat_version < 31)
       mesa_api = API_OPENGL_CORE;

    /* The latest version of EGL_KHR_create_context spec says:
     *
     *     "If the EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR flag bit is set in
     *     EGL_CONTEXT_FLAGS_KHR, then a <debug context> will be created.
     *     [...] This bit is supported for OpenGL and OpenGL ES contexts.
     *
     * No other EGL_CONTEXT_OPENGL_*_BIT is legal for an ES context.
     *
     * However, Mesa's EGL layer translates the context attribute
     * EGL_CONTEXT_OPENGL_ROBUST_ACCESS into the context flag
     * __DRI_CTX_FLAG_ROBUST_BUFFER_ACCESS.  That attribute is legal for ES
     * (with EGL 1.5 or EGL_EXT_create_context_robustness) and GL (only with
     * EGL 1.5).
     *
     * From the EGL_EXT_create_context_robustness spec:
     *
     *     This extension is written against the OpenGL ES 2.0 Specification
     *     but can apply to OpenGL ES 1.1 and up.
     *
     * From the EGL 1.5 (2014.08.27) spec, p55:
     *
     *     If the EGL_CONTEXT_OPENGL_ROBUST_ACCESS attribute is set to
     *     EGL_TRUE, a context supporting robust buffer access will be created.
     *     OpenGL contexts must support the GL_ARB_robustness extension, or
     *     equivalent core API functional- ity. OpenGL ES contexts must support
     *     the GL_EXT_robustness extension, or equivalent core API
     *     functionality.
     */
    if (mesa_api != API_OPENGL_COMPAT
        && mesa_api != API_OPENGL_CORE
        && (ctx_config.flags & ~(__DRI_CTX_FLAG_DEBUG |
                                 __DRI_CTX_FLAG_ROBUST_BUFFER_ACCESS))) {
        *error = __DRI_CTX_ERROR_BAD_FLAG;
        return NULL;
    }

    /* There are no forward-compatible contexts before OpenGL 3.0.  The
     * GLX_ARB_create_context spec says:
     *
     *     "Forward-compatible contexts are defined only for OpenGL versions
     *     3.0 and later."
     *
     * Forward-looking contexts are supported by silently converting the
     * requested API to API_OPENGL_CORE.
     *
     * In Mesa, a debug context is the same as a regular context.
     */
    if ((ctx_config.flags & __DRI_CTX_FLAG_FORWARD_COMPATIBLE) != 0) {
       mesa_api = API_OPENGL_CORE;
    }

    const uint32_t allowed_flags = (__DRI_CTX_FLAG_DEBUG
                                    | __DRI_CTX_FLAG_FORWARD_COMPATIBLE
                                    | __DRI_CTX_FLAG_ROBUST_BUFFER_ACCESS
                                    | __DRI_CTX_FLAG_RESET_ISOLATION);
    if (ctx_config.flags & ~allowed_flags) {
        *error = __DRI_CTX_ERROR_UNKNOWN_FLAG;
        return NULL;
    }

    *error = validate_context_version(screen, mesa_api,
                                      ctx_config.major_version,
                                      ctx_config.minor_version);
    if (*error != __DRI_CTX_ERROR_SUCCESS)
       return NULL;

    struct dri_context *ctx = dri_create_context(screen, mesa_api,
                                                 modes, &ctx_config, error,
                                                 dri_context(shared),
                                                 data);
    return opaque_dri_context(ctx);
}

static __DRIcontext *
driCreateNewContextForAPI(__DRIscreen *screen, int api,
                          const __DRIconfig *config,
                          __DRIcontext *shared, void *data)
{
    unsigned error;

    return driCreateContextAttribs(screen, api, config, shared, 0, NULL,
                                   &error, data);
}

static __DRIcontext *
driCreateNewContext(__DRIscreen *screen, const __DRIconfig *config,
                    __DRIcontext *shared, void *data)
{
    return driCreateNewContextForAPI(screen, __DRI_API_OPENGL,
                                     config, shared, data);
}

/**
 * Destroy the per-context private information.
 *
 * \internal
 * This function calls __DriverAPIRec::DestroyContext on \p contextPrivate, calls
 * drmDestroyContext(), and finally frees \p contextPrivate.
 */
static void
driDestroyContext(__DRIcontext *pcp)
{
    if (pcp)
        dri_destroy_context(dri_context(pcp));
}

static int
driCopyContext(__DRIcontext *dest, __DRIcontext *src, unsigned long mask)
{
    (void) dest;
    (void) src;
    (void) mask;
    return GL_FALSE;
}

/*@}*/


/*****************************************************************/
/** \name Context (un)binding functions                          */
/*****************************************************************/
/*@{*/

/**
 * This function takes both a read buffer and a draw buffer.  This is needed
 * for \c glXMakeCurrentReadSGI or GLX 1.3's \c glXMakeContextCurrent
 * function.
 */
static int driBindContext(__DRIcontext *pcp,
                          __DRIdrawable *pdp,
                          __DRIdrawable *prp)
{
   /*
    ** Assume error checking is done properly in glXMakeCurrent before
    ** calling driBindContext.
    */

    if (!pcp)
        return GL_FALSE;

    return dri_make_current(dri_context(pcp), dri_drawable(pdp),
                            dri_drawable(prp));
}

/**
 * Unbind context.
 *
 * \param scrn the screen.
 * \param gc context.
 *
 * \return \c GL_TRUE on success, or \c GL_FALSE on failure.
 *
 * \internal
 * This function calls __DriverAPIRec::UnbindContext, and then decrements
 * dri_drawable::refcount which must be non-zero for a successful
 * return.
 *
 * While casting the opaque private pointers associated with the parameters
 * into their respective real types it also assures they are not \c NULL.
 */
static int driUnbindContext(__DRIcontext *pcp)
{
    /*
    ** Assume error checking is done properly in glXMakeCurrent before
    ** calling driUnbindContext.
    */

    if (pcp == NULL)
        return GL_FALSE;

    /*
    ** Call dri_unbind_context before checking for valid drawables
    ** to handle surfaceless contexts properly.
    */
    return dri_unbind_context(dri_context(pcp));
}

/*@}*/

static __DRIdrawable *
driCreateNewDrawable(__DRIscreen *psp,
                     const __DRIconfig *config,
                     void *data)
{
    assert(data != NULL);

    struct dri_screen *screen = dri_screen(psp);
    struct dri_drawable *drawable =
       screen->create_drawable(screen, &config->modes, GL_FALSE, data);

    return opaque_dri_drawable(drawable);
}

static void
driDestroyDrawable(__DRIdrawable *pdp)
{
    dri_put_drawable(dri_drawable(pdp));
}

static __DRIbuffer *
dri2AllocateBuffer(__DRIscreen *psp,
                   unsigned int attachment, unsigned int format,
                   int width, int height)
{
   struct dri_screen *screen = dri_screen(psp);

   return screen->allocate_buffer(screen, attachment, format, width, height);
}

static void
dri2ReleaseBuffer(__DRIscreen *psp, __DRIbuffer *buffer)
{
   struct dri_screen *screen = dri_screen(psp);

   screen->release_buffer(buffer);
}


static int
dri2ConfigQueryb(__DRIscreen *psp, const char *var, unsigned char *val)
{
   struct dri_screen *screen = dri_screen(psp);

   if (!driCheckOption(&screen->optionCache, var, DRI_BOOL))
      return -1;

   *val = driQueryOptionb(&screen->optionCache, var);

   return 0;
}

static int
dri2ConfigQueryi(__DRIscreen *psp, const char *var, int *val)
{
   struct dri_screen *screen = dri_screen(psp);

   if (!driCheckOption(&screen->optionCache, var, DRI_INT) &&
       !driCheckOption(&screen->optionCache, var, DRI_ENUM))
      return -1;

    *val = driQueryOptioni(&screen->optionCache, var);

    return 0;
}

static int
dri2ConfigQueryf(__DRIscreen *psp, const char *var, float *val)
{
   struct dri_screen *screen = dri_screen(psp);

   if (!driCheckOption(&screen->optionCache, var, DRI_FLOAT))
      return -1;

    *val = driQueryOptionf(&screen->optionCache, var);

    return 0;
}

static int
dri2ConfigQuerys(__DRIscreen *psp, const char *var, char **val)
{
   struct dri_screen *screen = dri_screen(psp);

   if (!driCheckOption(&screen->optionCache, var, DRI_STRING))
      return -1;

    *val = driQueryOptionstr(&screen->optionCache, var);

    return 0;
}

static unsigned int
driGetAPIMask(__DRIscreen *screen)
{
    return dri_screen(screen)->api_mask;
}

/**
 * swrast swapbuffers entrypoint.
 *
 * DRI2 implements this inside the loader with only flushes handled by the
 * driver.
 */
static void
driSwapBuffers(__DRIdrawable *pdp)
{
   struct dri_drawable *drawable = dri_drawable(pdp);

   assert(drawable->screen->swrast_loader);

   drawable->swap_buffers(drawable);
}

/** Core interface */
const __DRIcoreExtension driCoreExtension = {
    .base = { __DRI_CORE, 2 },

    .createNewScreen            = NULL,
    .destroyScreen              = driDestroyScreen,
    .getExtensions              = driGetExtensions,
    .getConfigAttrib            = driGetConfigAttrib,
    .indexConfigAttrib          = driIndexConfigAttrib,
    .createNewDrawable          = NULL,
    .destroyDrawable            = driDestroyDrawable,
    .swapBuffers                = driSwapBuffers, /* swrast */
    .createNewContext           = driCreateNewContext, /* swrast */
    .copyContext                = driCopyContext,
    .destroyContext             = driDestroyContext,
    .bindContext                = driBindContext,
    .unbindContext              = driUnbindContext
};

#if HAVE_DRI2

/** DRI2 interface */
const __DRIdri2Extension driDRI2Extension = {
    .base = { __DRI_DRI2, 4 },

    .createNewScreen            = dri2CreateNewScreen,
    .createNewDrawable          = driCreateNewDrawable,
    .createNewContext           = driCreateNewContext,
    .getAPIMask                 = driGetAPIMask,
    .createNewContextForAPI     = driCreateNewContextForAPI,
    .allocateBuffer             = dri2AllocateBuffer,
    .releaseBuffer              = dri2ReleaseBuffer,
    .createContextAttribs       = driCreateContextAttribs,
    .createNewScreen2           = driCreateNewScreen2,
};

const __DRIdri2Extension swkmsDRI2Extension = {
    .base = { __DRI_DRI2, 4 },

    .createNewScreen            = swkmsCreateNewScreen,
    .createNewDrawable          = driCreateNewDrawable,
    .createNewContext           = driCreateNewContext,
    .getAPIMask                 = driGetAPIMask,
    .createNewContextForAPI     = driCreateNewContextForAPI,
    .allocateBuffer             = dri2AllocateBuffer,
    .releaseBuffer              = dri2ReleaseBuffer,
    .createContextAttribs       = driCreateContextAttribs,
    .createNewScreen2           = driCreateNewScreen2,
};

#endif

const __DRIswrastExtension driSWRastExtension = {
    .base = { __DRI_SWRAST, 4 },

    .createNewScreen            = driSWRastCreateNewScreen,
    .createNewDrawable          = driCreateNewDrawable,
    .createNewContextForAPI     = driCreateNewContextForAPI,
    .createContextAttribs       = driCreateContextAttribs,
    .createNewScreen2           = driSWRastCreateNewScreen2,
};

const __DRI2configQueryExtension dri2ConfigQueryExtension = {
   .base = { __DRI2_CONFIG_QUERY, 2 },

   .configQueryb        = dri2ConfigQueryb,
   .configQueryi        = dri2ConfigQueryi,
   .configQueryf        = dri2ConfigQueryf,
   .configQuerys        = dri2ConfigQuerys,
};

const __DRI2flushControlExtension dri2FlushControlExtension = {
   .base = { __DRI2_FLUSH_CONTROL, 1 }
};

/*
 * Note: the first match is returned, which is important for formats like
 * __DRI_IMAGE_FORMAT_R8 which maps to both MESA_FORMAT_{R,L}_UNORM8
 */
static const struct {
   uint32_t    image_format;
   mesa_format mesa_format;
   GLenum internal_format;
} format_mapping[] = {
   {
      .image_format    = __DRI_IMAGE_FORMAT_RGB565,
      .mesa_format     =        MESA_FORMAT_B5G6R5_UNORM,
      .internal_format =        GL_RGB565,
   },
   {
      .image_format    = __DRI_IMAGE_FORMAT_ARGB1555,
      .mesa_format     =        MESA_FORMAT_B5G5R5A1_UNORM,
      .internal_format =        GL_RGB5_A1,
   },
   {
      .image_format    = __DRI_IMAGE_FORMAT_ABGR1555,
      .mesa_format     =        MESA_FORMAT_R5G5B5A1_UNORM,
      .internal_format =        GL_RGB5_A1,
   },
   {
      .image_format    = __DRI_IMAGE_FORMAT_XRGB8888,
      .mesa_format     =        MESA_FORMAT_B8G8R8X8_UNORM,
      .internal_format =        GL_RGB8,
   },
   {
      .image_format    = __DRI_IMAGE_FORMAT_ABGR16161616F,
      .mesa_format     =        MESA_FORMAT_RGBA_FLOAT16,
      .internal_format =        GL_RGBA16F,
   },
   {
      .image_format    = __DRI_IMAGE_FORMAT_XBGR16161616F,
      .mesa_format     =        MESA_FORMAT_RGBX_FLOAT16,
      .internal_format =        GL_RGB16F,
   },
   {
      .image_format    = __DRI_IMAGE_FORMAT_ABGR16161616,
      .mesa_format     =        MESA_FORMAT_RGBA_UNORM16,
      .internal_format =        GL_RGBA16,
   },
   {
      .image_format    = __DRI_IMAGE_FORMAT_XBGR16161616,
      .mesa_format     =        MESA_FORMAT_RGBX_UNORM16,
      .internal_format =        GL_RGB16,
   },
   {
      .image_format    = __DRI_IMAGE_FORMAT_ARGB2101010,
      .mesa_format     =        MESA_FORMAT_B10G10R10A2_UNORM,
      .internal_format =        GL_RGB10_A2,
   },
   {
      .image_format    = __DRI_IMAGE_FORMAT_XRGB2101010,
      .mesa_format     =        MESA_FORMAT_B10G10R10X2_UNORM,
      .internal_format =        GL_RGB10,
   },
   {
      .image_format    = __DRI_IMAGE_FORMAT_ABGR2101010,
      .mesa_format     =        MESA_FORMAT_R10G10B10A2_UNORM,
      .internal_format =        GL_RGB10_A2,
   },
   {
      .image_format    = __DRI_IMAGE_FORMAT_XBGR2101010,
      .mesa_format     =        MESA_FORMAT_R10G10B10X2_UNORM,
      .internal_format =        GL_RGB10,
   },
   {
      .image_format    = __DRI_IMAGE_FORMAT_ARGB8888,
      .mesa_format     =        MESA_FORMAT_B8G8R8A8_UNORM,
      .internal_format =        GL_RGBA8,
   },
   {
      .image_format    = __DRI_IMAGE_FORMAT_ABGR8888,
      .mesa_format     =        MESA_FORMAT_R8G8B8A8_UNORM,
      .internal_format =        GL_RGBA8,
   },
   {
      .image_format    = __DRI_IMAGE_FORMAT_XBGR8888,
      .mesa_format     =        MESA_FORMAT_R8G8B8X8_UNORM,
      .internal_format =        GL_RGB8,
   },
   {
      .image_format    = __DRI_IMAGE_FORMAT_R8,
      .mesa_format     =        MESA_FORMAT_R_UNORM8,
      .internal_format =        GL_R8,
   },
   {
      .image_format    = __DRI_IMAGE_FORMAT_R8,
      .mesa_format     =        MESA_FORMAT_L_UNORM8,
      .internal_format =        GL_R8,
   },
#if UTIL_ARCH_LITTLE_ENDIAN
   {
      .image_format    = __DRI_IMAGE_FORMAT_GR88,
      .mesa_format     =        MESA_FORMAT_RG_UNORM8,
      .internal_format =        GL_RG8,
   },
   {
      .image_format    = __DRI_IMAGE_FORMAT_GR88,
      .mesa_format     =        MESA_FORMAT_LA_UNORM8,
      .internal_format =        GL_RG8,
   },
#endif
   {
      .image_format    = __DRI_IMAGE_FORMAT_SABGR8,
      .mesa_format     =        MESA_FORMAT_R8G8B8A8_SRGB,
      .internal_format =        GL_SRGB8_ALPHA8,
   },
   {
      .image_format    = __DRI_IMAGE_FORMAT_SARGB8,
      .mesa_format     =        MESA_FORMAT_B8G8R8A8_SRGB,
      .internal_format =        GL_SRGB8_ALPHA8,
   },
   {
      .image_format = __DRI_IMAGE_FORMAT_SXRGB8,
      .mesa_format  =           MESA_FORMAT_B8G8R8X8_SRGB,
      .internal_format =        GL_SRGB8,
   },
   {
      .image_format    = __DRI_IMAGE_FORMAT_R16,
      .mesa_format     =        MESA_FORMAT_R_UNORM16,
      .internal_format =        GL_R16,
   },
   {
      .image_format    = __DRI_IMAGE_FORMAT_R16,
      .mesa_format     =        MESA_FORMAT_L_UNORM16,
      .internal_format =        GL_R16,
   },
#if UTIL_ARCH_LITTLE_ENDIAN
   {
      .image_format    = __DRI_IMAGE_FORMAT_GR1616,
      .mesa_format     =        MESA_FORMAT_RG_UNORM16,
      .internal_format =        GL_RG16,
   },
   {
      .image_format    = __DRI_IMAGE_FORMAT_GR1616,
      .mesa_format     =        MESA_FORMAT_LA_UNORM16,
      .internal_format =        GL_RG16,
   },
#endif
   {
      .image_format    = __DRI_IMAGE_FORMAT_ARGB4444,
      .mesa_format     =        MESA_FORMAT_B4G4R4A4_UNORM,
      .internal_format =        GL_RGBA4,
   },
   {
      .image_format    = __DRI_IMAGE_FORMAT_ABGR4444,
      .mesa_format     =        MESA_FORMAT_R4G4B4A4_UNORM,
      .internal_format =        GL_RGBA4,
   },
};

uint32_t
driGLFormatToImageFormat(mesa_format format)
{
   for (size_t i = 0; i < ARRAY_SIZE(format_mapping); i++)
      if (format_mapping[i].mesa_format == format)
         return format_mapping[i].image_format;

   return __DRI_IMAGE_FORMAT_NONE;
}

uint32_t
driGLFormatToSizedInternalGLFormat(mesa_format format)
{
   for (size_t i = 0; i < ARRAY_SIZE(format_mapping); i++)
      if (format_mapping[i].mesa_format == format)
         return format_mapping[i].internal_format;

   return GL_NONE;
}

mesa_format
driImageFormatToGLFormat(uint32_t image_format)
{
   for (size_t i = 0; i < ARRAY_SIZE(format_mapping); i++)
      if (format_mapping[i].image_format == image_format)
         return format_mapping[i].mesa_format;

   return MESA_FORMAT_NONE;
}

/** Image driver interface */
const __DRIimageDriverExtension driImageDriverExtension = {
    .base = { __DRI_IMAGE_DRIVER, 1 },

    .createNewScreen2           = driCreateNewScreen2,
    .createNewDrawable          = driCreateNewDrawable,
    .getAPIMask                 = driGetAPIMask,
    .createContextAttribs       = driCreateContextAttribs,
};
