/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
 * Copyright 2009-2010 Chia-I Wu <olvaffe@gmail.com>
 * Copyright 2010-2011 LunarG, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef EGLCONFIG_INCLUDED
#define EGLCONFIG_INCLUDED

#include <assert.h>
#include <stddef.h>

#include "egltypedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* update _eglValidationTable and _eglOffsetOfConfig before updating this
 * struct */
struct _egl_config {
   _EGLDisplay *Display;

   /* core */
   EGLint BufferSize;
   EGLint AlphaSize;
   EGLint BlueSize;
   EGLint GreenSize;
   EGLint RedSize;
   EGLint DepthSize;
   EGLint StencilSize;
   EGLint ConfigCaveat;
   EGLint ConfigID;
   EGLint Level;
   EGLint MaxPbufferHeight;
   EGLint MaxPbufferPixels;
   EGLint MaxPbufferWidth;
   EGLint NativeRenderable;
   EGLint NativeVisualID;
   EGLint NativeVisualType;
   EGLint Samples;
   EGLint SampleBuffers;
   EGLint SurfaceType;
   EGLint TransparentType;
   EGLint TransparentBlueValue;
   EGLint TransparentGreenValue;
   EGLint TransparentRedValue;
   EGLint BindToTextureRGB;
   EGLint BindToTextureRGBA;
   EGLint MinSwapInterval;
   EGLint MaxSwapInterval;
   EGLint LuminanceSize;
   EGLint AlphaMaskSize;
   EGLint ColorBufferType;
   EGLint RenderableType;
   EGLint MatchNativePixmap;
   EGLint Conformant;

   /* extensions */
   EGLint YInvertedNOK;
   EGLint FramebufferTargetAndroid;
   EGLint RecordableAndroid;
   EGLint ComponentType;
};

/**
 * Map an EGL attribute enum to the offset of the member in _EGLConfig.
 */
static inline EGLint
_eglOffsetOfConfig(EGLint attr)
{
   switch (attr) {
#define ATTRIB_MAP(attr, memb)                                                 \
   case attr:                                                                  \
      return offsetof(_EGLConfig, memb)
      /* core */
      ATTRIB_MAP(EGL_BUFFER_SIZE, BufferSize);
      ATTRIB_MAP(EGL_ALPHA_SIZE, AlphaSize);
      ATTRIB_MAP(EGL_BLUE_SIZE, BlueSize);
      ATTRIB_MAP(EGL_GREEN_SIZE, GreenSize);
      ATTRIB_MAP(EGL_RED_SIZE, RedSize);
      ATTRIB_MAP(EGL_DEPTH_SIZE, DepthSize);
      ATTRIB_MAP(EGL_STENCIL_SIZE, StencilSize);
      ATTRIB_MAP(EGL_CONFIG_CAVEAT, ConfigCaveat);
      ATTRIB_MAP(EGL_CONFIG_ID, ConfigID);
      ATTRIB_MAP(EGL_LEVEL, Level);
      ATTRIB_MAP(EGL_MAX_PBUFFER_HEIGHT, MaxPbufferHeight);
      ATTRIB_MAP(EGL_MAX_PBUFFER_PIXELS, MaxPbufferPixels);
      ATTRIB_MAP(EGL_MAX_PBUFFER_WIDTH, MaxPbufferWidth);
      ATTRIB_MAP(EGL_NATIVE_RENDERABLE, NativeRenderable);
      ATTRIB_MAP(EGL_NATIVE_VISUAL_ID, NativeVisualID);
      ATTRIB_MAP(EGL_NATIVE_VISUAL_TYPE, NativeVisualType);
      ATTRIB_MAP(EGL_SAMPLES, Samples);
      ATTRIB_MAP(EGL_SAMPLE_BUFFERS, SampleBuffers);
      ATTRIB_MAP(EGL_SURFACE_TYPE, SurfaceType);
      ATTRIB_MAP(EGL_TRANSPARENT_TYPE, TransparentType);
      ATTRIB_MAP(EGL_TRANSPARENT_BLUE_VALUE, TransparentBlueValue);
      ATTRIB_MAP(EGL_TRANSPARENT_GREEN_VALUE, TransparentGreenValue);
      ATTRIB_MAP(EGL_TRANSPARENT_RED_VALUE, TransparentRedValue);
      ATTRIB_MAP(EGL_BIND_TO_TEXTURE_RGB, BindToTextureRGB);
      ATTRIB_MAP(EGL_BIND_TO_TEXTURE_RGBA, BindToTextureRGBA);
      ATTRIB_MAP(EGL_MIN_SWAP_INTERVAL, MinSwapInterval);
      ATTRIB_MAP(EGL_MAX_SWAP_INTERVAL, MaxSwapInterval);
      ATTRIB_MAP(EGL_LUMINANCE_SIZE, LuminanceSize);
      ATTRIB_MAP(EGL_ALPHA_MASK_SIZE, AlphaMaskSize);
      ATTRIB_MAP(EGL_COLOR_BUFFER_TYPE, ColorBufferType);
      ATTRIB_MAP(EGL_RENDERABLE_TYPE, RenderableType);
      ATTRIB_MAP(EGL_MATCH_NATIVE_PIXMAP, MatchNativePixmap);
      ATTRIB_MAP(EGL_CONFORMANT, Conformant);
      /* extensions */
      ATTRIB_MAP(EGL_Y_INVERTED_NOK, YInvertedNOK);
      ATTRIB_MAP(EGL_FRAMEBUFFER_TARGET_ANDROID, FramebufferTargetAndroid);
      ATTRIB_MAP(EGL_RECORDABLE_ANDROID, RecordableAndroid);
      ATTRIB_MAP(EGL_COLOR_COMPONENT_TYPE_EXT, ComponentType);
#undef ATTRIB_MAP
   default:
      return -1;
   }
}

/**
 * Update a config for a given key.
 *
 * Note that a valid key is not necessarily a valid attribute.  There are gaps
 * in the attribute enums.  The separation is to catch application errors.
 * Drivers should never set a key that is an invalid attribute.
 */
static inline void
_eglSetConfigKey(_EGLConfig *conf, EGLint key, EGLint val)
{
   EGLint offset = _eglOffsetOfConfig(key);
   assert(offset >= 0);
   *((EGLint *)((char *)conf + offset)) = val;
}

/**
 * Return the value for a given key.
 */
static inline EGLint
_eglGetConfigKey(const _EGLConfig *conf, EGLint key)
{
   EGLint offset = _eglOffsetOfConfig(key);
   assert(offset >= 0);
   return *((EGLint *)((char *)conf + offset));
}

extern void
_eglInitConfig(_EGLConfig *config, _EGLDisplay *disp, EGLint id);

extern EGLConfig
_eglLinkConfig(_EGLConfig *conf);

extern _EGLConfig *
_eglLookupConfig(EGLConfig config, _EGLDisplay *disp);

/**
 * Return the handle of a linked config.
 */
static inline EGLConfig
_eglGetConfigHandle(_EGLConfig *conf)
{
   return (EGLConfig)conf;
}

extern EGLBoolean
_eglValidateConfig(const _EGLConfig *conf, EGLBoolean for_matching);

extern EGLBoolean
_eglMatchConfig(const _EGLConfig *conf, const _EGLConfig *criteria);

extern EGLBoolean
_eglParseConfigAttribList(_EGLConfig *conf, _EGLDisplay *disp,
                          const EGLint *attrib_list);

extern EGLint
_eglCompareConfigs(const _EGLConfig *conf1, const _EGLConfig *conf2,
                   const _EGLConfig *criteria, EGLBoolean compare_id);

extern EGLBoolean
_eglChooseConfig(_EGLDisplay *disp, const EGLint *attrib_list,
                 EGLConfig *configs, EGLint config_size, EGLint *num_config);

extern EGLBoolean
_eglGetConfigAttrib(const _EGLDisplay *disp, const _EGLConfig *conf,
                    EGLint attribute, EGLint *value);

extern EGLBoolean
_eglGetConfigs(_EGLDisplay *disp, EGLConfig *configs, EGLint config_size,
               EGLint *num_config);

#ifdef __cplusplus
}
#endif

#endif /* EGLCONFIG_INCLUDED */
