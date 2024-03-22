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

/**
 * EGL Configuration (pixel format) functions.
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "util/macros.h"

#include "eglconfig.h"
#include "eglconfigdebug.h"
#include "eglcurrent.h"
#include "egldisplay.h"
#include "egllog.h"

/**
 * Init the given _EGLconfig to default values.
 * \param id  the configuration's ID.
 *
 * Note that id must be positive for the config to be valid.
 * It is also recommended that when there are N configs, their
 * IDs are from 1 to N respectively.
 */
void
_eglInitConfig(_EGLConfig *conf, _EGLDisplay *disp, EGLint id)
{
   memset(conf, 0, sizeof(*conf));

   conf->Display = disp;

   /* some attributes take non-zero default values */
   conf->ConfigID = id;
   conf->ConfigCaveat = EGL_NONE;
   conf->TransparentType = EGL_NONE;
   conf->NativeVisualType = EGL_NONE;
   conf->ColorBufferType = EGL_RGB_BUFFER;
   conf->ComponentType = EGL_COLOR_COMPONENT_TYPE_FIXED_EXT;
}

/**
 * Link a config to its display and return the handle of the link.
 * The handle can be passed to client directly.
 *
 * Note that we just save the ptr to the config (we don't copy the config).
 */
EGLConfig
_eglLinkConfig(_EGLConfig *conf)
{
   _EGLDisplay *disp = conf->Display;

   /* sanity check */
   assert(disp);
   assert(conf->ConfigID > 0);

   if (!disp->Configs) {
      disp->Configs = _eglCreateArray("Config", 16);
      if (!disp->Configs)
         return (EGLConfig)NULL;
   }

   _eglAppendArray(disp->Configs, (void *)conf);

   return (EGLConfig)conf;
}

/**
 * Lookup a handle to find the linked config.
 * Return NULL if the handle has no corresponding linked config.
 */
_EGLConfig *
_eglLookupConfig(EGLConfig config, _EGLDisplay *disp)
{
   _EGLConfig *conf;

   if (!disp)
      return NULL;

   conf = (_EGLConfig *)_eglFindArray(disp->Configs, (void *)config);
   if (conf)
      assert(conf->Display == disp);

   return conf;
}

enum type {
   ATTRIB_TYPE_INTEGER,
   ATTRIB_TYPE_BOOLEAN,
   ATTRIB_TYPE_BITMASK,
   ATTRIB_TYPE_ENUM,
   ATTRIB_TYPE_PSEUDO,   /* non-queryable */
   ATTRIB_TYPE_PLATFORM, /* platform-dependent */
};

enum criterion {
   ATTRIB_CRITERION_EXACT,
   ATTRIB_CRITERION_ATLEAST,
   ATTRIB_CRITERION_MASK,
   ATTRIB_CRITERION_SPECIAL,
   ATTRIB_CRITERION_IGNORE
};

/* EGL spec Table 3.1 and 3.4 */
static const struct {
   EGLint attr;
   enum type type;
   enum criterion criterion;
   EGLint default_value;
} _eglValidationTable[] = {
   /* clang-format off */
   /* core */
   { EGL_BUFFER_SIZE,               ATTRIB_TYPE_INTEGER,
                                    ATTRIB_CRITERION_ATLEAST,
                                    0 },
   { EGL_RED_SIZE,                  ATTRIB_TYPE_INTEGER,
                                    ATTRIB_CRITERION_ATLEAST,
                                    0 },
   { EGL_GREEN_SIZE,                ATTRIB_TYPE_INTEGER,
                                    ATTRIB_CRITERION_ATLEAST,
                                    0 },
   { EGL_BLUE_SIZE,                 ATTRIB_TYPE_INTEGER,
                                    ATTRIB_CRITERION_ATLEAST,
                                    0 },
   { EGL_LUMINANCE_SIZE,            ATTRIB_TYPE_INTEGER,
                                    ATTRIB_CRITERION_ATLEAST,
                                    0 },
   { EGL_ALPHA_SIZE,                ATTRIB_TYPE_INTEGER,
                                    ATTRIB_CRITERION_ATLEAST,
                                    0 },
   { EGL_ALPHA_MASK_SIZE,           ATTRIB_TYPE_INTEGER,
                                    ATTRIB_CRITERION_ATLEAST,
                                    0 },
   { EGL_BIND_TO_TEXTURE_RGB,       ATTRIB_TYPE_BOOLEAN,
                                    ATTRIB_CRITERION_EXACT,
                                    EGL_DONT_CARE },
   { EGL_BIND_TO_TEXTURE_RGBA,      ATTRIB_TYPE_BOOLEAN,
                                    ATTRIB_CRITERION_EXACT,
                                    EGL_DONT_CARE },
   { EGL_COLOR_BUFFER_TYPE,         ATTRIB_TYPE_ENUM,
                                    ATTRIB_CRITERION_EXACT,
                                    EGL_RGB_BUFFER },
   { EGL_CONFIG_CAVEAT,             ATTRIB_TYPE_ENUM,
                                    ATTRIB_CRITERION_EXACT,
                                    EGL_DONT_CARE },
   { EGL_CONFIG_ID,                 ATTRIB_TYPE_INTEGER,
                                    ATTRIB_CRITERION_EXACT,
                                    EGL_DONT_CARE },
   { EGL_CONFORMANT,                ATTRIB_TYPE_BITMASK,
                                    ATTRIB_CRITERION_MASK,
                                    0 },
   { EGL_DEPTH_SIZE,                ATTRIB_TYPE_INTEGER,
                                    ATTRIB_CRITERION_ATLEAST,
                                    0 },
   { EGL_LEVEL,                     ATTRIB_TYPE_PLATFORM,
                                    ATTRIB_CRITERION_EXACT,
                                    0 },
   { EGL_MAX_PBUFFER_WIDTH,         ATTRIB_TYPE_INTEGER,
                                    ATTRIB_CRITERION_IGNORE,
                                    0 },
   { EGL_MAX_PBUFFER_HEIGHT,        ATTRIB_TYPE_INTEGER,
                                    ATTRIB_CRITERION_IGNORE,
                                    0 },
   { EGL_MAX_PBUFFER_PIXELS,        ATTRIB_TYPE_INTEGER,
                                    ATTRIB_CRITERION_IGNORE,
                                    0 },
   { EGL_MAX_SWAP_INTERVAL,         ATTRIB_TYPE_INTEGER,
                                    ATTRIB_CRITERION_EXACT,
                                    EGL_DONT_CARE },
   { EGL_MIN_SWAP_INTERVAL,         ATTRIB_TYPE_INTEGER,
                                    ATTRIB_CRITERION_EXACT,
                                    EGL_DONT_CARE },
   { EGL_NATIVE_RENDERABLE,         ATTRIB_TYPE_BOOLEAN,
                                    ATTRIB_CRITERION_EXACT,
                                    EGL_DONT_CARE },
   { EGL_NATIVE_VISUAL_ID,          ATTRIB_TYPE_PLATFORM,
                                    ATTRIB_CRITERION_IGNORE,
                                    0 },
   { EGL_NATIVE_VISUAL_TYPE,        ATTRIB_TYPE_PLATFORM,
                                    ATTRIB_CRITERION_EXACT,
                                    EGL_DONT_CARE },
   { EGL_RENDERABLE_TYPE,           ATTRIB_TYPE_BITMASK,
                                    ATTRIB_CRITERION_MASK,
                                    EGL_OPENGL_ES_BIT },
   { EGL_SAMPLE_BUFFERS,            ATTRIB_TYPE_INTEGER,
                                    ATTRIB_CRITERION_ATLEAST,
                                    0 },
   { EGL_SAMPLES,                   ATTRIB_TYPE_INTEGER,
                                    ATTRIB_CRITERION_ATLEAST,
                                    0 },
   { EGL_STENCIL_SIZE,              ATTRIB_TYPE_INTEGER,
                                    ATTRIB_CRITERION_ATLEAST,
                                    0 },
   { EGL_SURFACE_TYPE,              ATTRIB_TYPE_BITMASK,
                                    ATTRIB_CRITERION_MASK,
                                    EGL_WINDOW_BIT },
   { EGL_TRANSPARENT_TYPE,          ATTRIB_TYPE_ENUM,
                                    ATTRIB_CRITERION_EXACT,
                                    EGL_NONE },
   { EGL_TRANSPARENT_RED_VALUE,     ATTRIB_TYPE_INTEGER,
                                    ATTRIB_CRITERION_EXACT,
                                    EGL_DONT_CARE },
   { EGL_TRANSPARENT_GREEN_VALUE,   ATTRIB_TYPE_INTEGER,
                                    ATTRIB_CRITERION_EXACT,
                                    EGL_DONT_CARE },
   { EGL_TRANSPARENT_BLUE_VALUE,    ATTRIB_TYPE_INTEGER,
                                    ATTRIB_CRITERION_EXACT,
                                    EGL_DONT_CARE },
   { EGL_MATCH_NATIVE_PIXMAP,       ATTRIB_TYPE_PSEUDO,
                                    ATTRIB_CRITERION_SPECIAL,
                                    EGL_NONE },
   /* extensions */
   { EGL_Y_INVERTED_NOK,            ATTRIB_TYPE_BOOLEAN,
                                    ATTRIB_CRITERION_EXACT,
                                    EGL_DONT_CARE },
   { EGL_FRAMEBUFFER_TARGET_ANDROID, ATTRIB_TYPE_BOOLEAN,
                                    ATTRIB_CRITERION_EXACT,
                                    EGL_DONT_CARE },
   { EGL_RECORDABLE_ANDROID,        ATTRIB_TYPE_BOOLEAN,
                                    ATTRIB_CRITERION_EXACT,
                                    EGL_DONT_CARE },
   { EGL_COLOR_COMPONENT_TYPE_EXT,  ATTRIB_TYPE_ENUM,
                                    ATTRIB_CRITERION_EXACT,
                                    EGL_COLOR_COMPONENT_TYPE_FIXED_EXT },
   /* clang-format on */
};

/**
 * Return true if a config is valid.  When for_matching is true,
 * EGL_DONT_CARE is accepted as a valid attribute value, and checks
 * for conflicting attribute values are skipped.
 *
 * Note that some attributes are platform-dependent and are not
 * checked.
 */
EGLBoolean
_eglValidateConfig(const _EGLConfig *conf, EGLBoolean for_matching)
{
   _EGLDisplay *disp = conf->Display;
   EGLint i, attr, val;
   EGLBoolean valid = EGL_TRUE;

   /* check attributes by their types */
   for (i = 0; i < ARRAY_SIZE(_eglValidationTable); i++) {
      EGLint mask;

      attr = _eglValidationTable[i].attr;
      val = _eglGetConfigKey(conf, attr);

      switch (_eglValidationTable[i].type) {
      case ATTRIB_TYPE_INTEGER:
         switch (attr) {
         case EGL_CONFIG_ID:
            /* config id must be positive */
            if (val <= 0)
               valid = EGL_FALSE;
            break;
         case EGL_SAMPLE_BUFFERS:
            /* there can be at most 1 sample buffer */
            if (val > 1 || val < 0)
               valid = EGL_FALSE;
            break;
         default:
            if (val < 0)
               valid = EGL_FALSE;
            break;
         }
         break;
      case ATTRIB_TYPE_BOOLEAN:
         if (val != EGL_TRUE && val != EGL_FALSE)
            valid = EGL_FALSE;
         break;
      case ATTRIB_TYPE_ENUM:
         switch (attr) {
         case EGL_CONFIG_CAVEAT:
            if (val != EGL_NONE && val != EGL_SLOW_CONFIG &&
                val != EGL_NON_CONFORMANT_CONFIG)
               valid = EGL_FALSE;
            break;
         case EGL_TRANSPARENT_TYPE:
            if (val != EGL_NONE && val != EGL_TRANSPARENT_RGB)
               valid = EGL_FALSE;
            break;
         case EGL_COLOR_BUFFER_TYPE:
            if (val != EGL_RGB_BUFFER && val != EGL_LUMINANCE_BUFFER)
               valid = EGL_FALSE;
            break;
         case EGL_COLOR_COMPONENT_TYPE_EXT:
            if (val != EGL_COLOR_COMPONENT_TYPE_FIXED_EXT &&
                val != EGL_COLOR_COMPONENT_TYPE_FLOAT_EXT)
               valid = EGL_FALSE;
            break;
         default:
            unreachable("check _eglValidationTable[]");
            break;
         }
         break;
      case ATTRIB_TYPE_BITMASK:
         switch (attr) {
         case EGL_SURFACE_TYPE:
            mask = EGL_PBUFFER_BIT | EGL_PIXMAP_BIT | EGL_WINDOW_BIT |
                   EGL_VG_COLORSPACE_LINEAR_BIT | EGL_VG_ALPHA_FORMAT_PRE_BIT |
                   EGL_MULTISAMPLE_RESOLVE_BOX_BIT |
                   EGL_SWAP_BEHAVIOR_PRESERVED_BIT;
            if (disp->Extensions.KHR_mutable_render_buffer)
               mask |= EGL_MUTABLE_RENDER_BUFFER_BIT_KHR;
            break;
         case EGL_RENDERABLE_TYPE:
         case EGL_CONFORMANT:
            mask = EGL_OPENGL_ES_BIT | EGL_OPENVG_BIT | EGL_OPENGL_ES2_BIT |
                   EGL_OPENGL_ES3_BIT_KHR | EGL_OPENGL_BIT;
            break;
         default:
            unreachable("check _eglValidationTable[]");
            mask = 0;
            break;
         }
         if (val & ~mask)
            valid = EGL_FALSE;
         break;
      case ATTRIB_TYPE_PLATFORM:
         /* unable to check platform-dependent attributes here */
         break;
      case ATTRIB_TYPE_PSEUDO:
         /* pseudo attributes should not be set */
         if (val != 0)
            valid = EGL_FALSE;
         break;
      }

      if (!valid && for_matching) {
         /* accept EGL_DONT_CARE as a valid value */
         if (val == EGL_DONT_CARE)
            valid = EGL_TRUE;
         if (_eglValidationTable[i].criterion == ATTRIB_CRITERION_SPECIAL)
            valid = EGL_TRUE;
      }
      if (!valid) {
         _eglLog(_EGL_DEBUG, "attribute 0x%04x has an invalid value 0x%x", attr,
                 val);
         break;
      }
   }

   /* any invalid attribute value should have been caught */
   if (!valid || for_matching)
      return valid;

   /* now check for conflicting attribute values */

   switch (conf->ColorBufferType) {
   case EGL_RGB_BUFFER:
      if (conf->LuminanceSize)
         valid = EGL_FALSE;
      if (conf->RedSize + conf->GreenSize + conf->BlueSize + conf->AlphaSize !=
          conf->BufferSize)
         valid = EGL_FALSE;
      break;
   case EGL_LUMINANCE_BUFFER:
      if (conf->RedSize || conf->GreenSize || conf->BlueSize)
         valid = EGL_FALSE;
      if (conf->LuminanceSize + conf->AlphaSize != conf->BufferSize)
         valid = EGL_FALSE;
      break;
   }
   if (!valid) {
      _eglLog(_EGL_DEBUG, "conflicting color buffer type and channel sizes");
      return EGL_FALSE;
   }

   if (!conf->SampleBuffers && conf->Samples)
      valid = EGL_FALSE;
   if (!valid) {
      _eglLog(_EGL_DEBUG, "conflicting samples and sample buffers");
      return EGL_FALSE;
   }

   if (!(conf->SurfaceType & EGL_WINDOW_BIT)) {
      if (conf->NativeVisualID != 0 || conf->NativeVisualType != EGL_NONE)
         valid = EGL_FALSE;
   }
   if (!(conf->SurfaceType & EGL_PBUFFER_BIT)) {
      if (conf->BindToTextureRGB || conf->BindToTextureRGBA)
         valid = EGL_FALSE;
   }
   if (!valid) {
      _eglLog(_EGL_DEBUG,
              "conflicting surface type and native visual/texture binding");
      return EGL_FALSE;
   }

   return valid;
}

/**
 * Return true if a config matches the criteria.  This and
 * _eglParseConfigAttribList together implement the algorithm
 * described in "Selection of EGLConfigs".
 *
 * Note that attributes that are special (currently, only
 * EGL_MATCH_NATIVE_PIXMAP) are ignored.
 */
EGLBoolean
_eglMatchConfig(const _EGLConfig *conf, const _EGLConfig *criteria)
{
   EGLint attr, val, i;
   EGLBoolean matched = EGL_TRUE;

   for (i = 0; i < ARRAY_SIZE(_eglValidationTable); i++) {
      EGLint cmp;
      if (_eglValidationTable[i].criterion == ATTRIB_CRITERION_IGNORE)
         continue;

      attr = _eglValidationTable[i].attr;
      cmp = _eglGetConfigKey(criteria, attr);
      if (cmp == EGL_DONT_CARE)
         continue;

      val = _eglGetConfigKey(conf, attr);
      switch (_eglValidationTable[i].criterion) {
      case ATTRIB_CRITERION_EXACT:
         if (val != cmp)
            matched = EGL_FALSE;
         break;
      case ATTRIB_CRITERION_ATLEAST:
         if (val < cmp)
            matched = EGL_FALSE;
         break;
      case ATTRIB_CRITERION_MASK:
         if ((val & cmp) != cmp)
            matched = EGL_FALSE;
         break;
      case ATTRIB_CRITERION_SPECIAL:
         /* ignored here */
         break;
      case ATTRIB_CRITERION_IGNORE:
         unreachable("already handled above");
         break;
      }

      if (!matched) {
#ifndef DEBUG
         /* only print the common errors when DEBUG is not defined */
         if (attr != EGL_RENDERABLE_TYPE)
            break;
#endif
         _eglLog(_EGL_DEBUG,
                 "the value (0x%x) of attribute 0x%04x did not meet the "
                 "criteria (0x%x)",
                 val, attr, cmp);
         break;
      }
   }

   return matched;
}

static inline EGLBoolean
_eglIsConfigAttribValid(const _EGLConfig *conf, EGLint attr)
{
   if (_eglOffsetOfConfig(attr) < 0)
      return EGL_FALSE;

   switch (attr) {
   case EGL_Y_INVERTED_NOK:
      return conf->Display->Extensions.NOK_texture_from_pixmap;
   case EGL_FRAMEBUFFER_TARGET_ANDROID:
      return conf->Display->Extensions.ANDROID_framebuffer_target;
   case EGL_RECORDABLE_ANDROID:
      return conf->Display->Extensions.ANDROID_recordable;
   default:
      break;
   }

   return EGL_TRUE;
}

/**
 * Initialize a criteria config from the given attribute list.
 * Return EGL_FALSE if any of the attribute is invalid.
 */
EGLBoolean
_eglParseConfigAttribList(_EGLConfig *conf, _EGLDisplay *disp,
                          const EGLint *attrib_list)
{
   EGLint attr, val, i;

   _eglInitConfig(conf, disp, EGL_DONT_CARE);

   /* reset to default values */
   for (i = 0; i < ARRAY_SIZE(_eglValidationTable); i++) {
      attr = _eglValidationTable[i].attr;
      val = _eglValidationTable[i].default_value;
      _eglSetConfigKey(conf, attr, val);
   }

   /* parse the list */
   for (i = 0; attrib_list && attrib_list[i] != EGL_NONE; i += 2) {
      attr = attrib_list[i];
      val = attrib_list[i + 1];

      if (!_eglIsConfigAttribValid(conf, attr))
         return EGL_FALSE;

      _eglSetConfigKey(conf, attr, val);
   }

   if (!_eglValidateConfig(conf, EGL_TRUE))
      return EGL_FALSE;

   /* EGL_LEVEL and EGL_MATCH_NATIVE_PIXMAP cannot be EGL_DONT_CARE */
   if (conf->Level == EGL_DONT_CARE || conf->MatchNativePixmap == EGL_DONT_CARE)
      return EGL_FALSE;

   /* ignore other attributes when EGL_CONFIG_ID is given */
   if (conf->ConfigID != EGL_DONT_CARE) {
      for (i = 0; i < ARRAY_SIZE(_eglValidationTable); i++) {
         attr = _eglValidationTable[i].attr;
         if (attr != EGL_CONFIG_ID)
            _eglSetConfigKey(conf, attr, EGL_DONT_CARE);
      }
   } else {
      if (!(conf->SurfaceType & EGL_WINDOW_BIT))
         conf->NativeVisualType = EGL_DONT_CARE;

      if (conf->TransparentType == EGL_NONE) {
         conf->TransparentRedValue = EGL_DONT_CARE;
         conf->TransparentGreenValue = EGL_DONT_CARE;
         conf->TransparentBlueValue = EGL_DONT_CARE;
      }
   }

   return EGL_TRUE;
}

/**
 * Decide the ordering of conf1 and conf2, under the given criteria.
 * When compare_id is true, this implements the algorithm described
 * in "Sorting of EGLConfigs".  When compare_id is false,
 * EGL_CONFIG_ID is not compared.
 *
 * It returns a negative integer if conf1 is considered to come
 * before conf2;  a positive integer if conf2 is considered to come
 * before conf1;  zero if the ordering cannot be decided.
 *
 * Note that EGL_NATIVE_VISUAL_TYPE is platform-dependent and is
 * ignored here.
 */
EGLint
_eglCompareConfigs(const _EGLConfig *conf1, const _EGLConfig *conf2,
                   const _EGLConfig *criteria, EGLBoolean compare_id)
{
   const EGLint compare_attribs[] = {
      EGL_BUFFER_SIZE, EGL_SAMPLE_BUFFERS, EGL_SAMPLES,
      EGL_DEPTH_SIZE,  EGL_STENCIL_SIZE,   EGL_ALPHA_MASK_SIZE,
   };
   EGLint val1, val2;
   EGLint i;

   if (conf1 == conf2)
      return 0;

   /* the enum values have the desired ordering */
   STATIC_ASSERT(EGL_NONE < EGL_SLOW_CONFIG);
   STATIC_ASSERT(EGL_SLOW_CONFIG < EGL_NON_CONFORMANT_CONFIG);
   val1 = conf1->ConfigCaveat - conf2->ConfigCaveat;
   if (val1)
      return val1;

   /* the enum values have the desired ordering */
   STATIC_ASSERT(EGL_RGB_BUFFER < EGL_LUMINANCE_BUFFER);
   val1 = conf1->ColorBufferType - conf2->ColorBufferType;
   if (val1)
      return val1;

   if (criteria) {
      val1 = val2 = 0;
      if (conf1->ColorBufferType == EGL_RGB_BUFFER) {
         if (criteria->RedSize > 0) {
            val1 += conf1->RedSize;
            val2 += conf2->RedSize;
         }
         if (criteria->GreenSize > 0) {
            val1 += conf1->GreenSize;
            val2 += conf2->GreenSize;
         }
         if (criteria->BlueSize > 0) {
            val1 += conf1->BlueSize;
            val2 += conf2->BlueSize;
         }
      } else {
         if (criteria->LuminanceSize > 0) {
            val1 += conf1->LuminanceSize;
            val2 += conf2->LuminanceSize;
         }
      }
      if (criteria->AlphaSize > 0) {
         val1 += conf1->AlphaSize;
         val2 += conf2->AlphaSize;
      }
   } else {
      /* assume the default criteria, which gives no specific ordering */
      val1 = val2 = 0;
   }

   /* for color bits, larger one is preferred */
   if (val1 != val2)
      return (val2 - val1);

   for (i = 0; i < ARRAY_SIZE(compare_attribs); i++) {
      val1 = _eglGetConfigKey(conf1, compare_attribs[i]);
      val2 = _eglGetConfigKey(conf2, compare_attribs[i]);
      if (val1 != val2)
         return (val1 - val2);
   }

   /* EGL_NATIVE_VISUAL_TYPE cannot be compared here */

   return (compare_id) ? (conf1->ConfigID - conf2->ConfigID) : 0;
}

static inline void
_eglSwapConfigs(const _EGLConfig **conf1, const _EGLConfig **conf2)
{
   const _EGLConfig *tmp = *conf1;
   *conf1 = *conf2;
   *conf2 = tmp;
}

/**
 * Quick sort an array of configs.  This differs from the standard
 * qsort() in that the compare function accepts an additional
 * argument.
 */
static void
_eglSortConfigs(const _EGLConfig **configs, EGLint count,
                EGLint (*compare)(const _EGLConfig *, const _EGLConfig *,
                                  void *),
                void *priv_data)
{
   const EGLint pivot = 0;
   EGLint i, j;

   if (count <= 1)
      return;

   _eglSwapConfigs(&configs[pivot], &configs[count / 2]);
   i = 1;
   j = count - 1;
   do {
      while (i < count && compare(configs[i], configs[pivot], priv_data) < 0)
         i++;
      while (compare(configs[j], configs[pivot], priv_data) > 0)
         j--;
      if (i < j) {
         _eglSwapConfigs(&configs[i], &configs[j]);
         i++;
         j--;
      } else if (i == j) {
         i++;
         j--;
         break;
      }
   } while (i <= j);
   _eglSwapConfigs(&configs[pivot], &configs[j]);

   _eglSortConfigs(configs, j, compare, priv_data);
   _eglSortConfigs(configs + i, count - i, compare, priv_data);
}

/**
 * A helper function for implementing eglChooseConfig.  See _eglFilterArray and
 * _eglSortConfigs for the meanings of match and compare.
 */
static EGLBoolean
_eglFilterConfigArray(_EGLArray *array, EGLConfig *configs, EGLint config_size,
                      EGLint *num_configs,
                      EGLBoolean (*match)(const _EGLConfig *,
                                          const _EGLConfig *),
                      EGLint (*compare)(const _EGLConfig *, const _EGLConfig *,
                                        void *),
                      void *priv_data)
{
   _EGLConfig **configList;
   EGLint i, count;

   /* get the number of matched configs */
   count = _eglFilterArray(array, NULL, 0, (_EGLArrayForEach)match, priv_data);
   if (!count) {
      *num_configs = count;
      return EGL_TRUE;
   }

   configList = malloc(sizeof(*configList) * count);
   if (!configList)
      return _eglError(EGL_BAD_ALLOC, "eglChooseConfig(out of memory)");

   /* get the matched configs */
   _eglFilterArray(array, (void **)configList, count, (_EGLArrayForEach)match,
                   priv_data);

   /* perform sorting of configs */
   if (configs && count) {
      _eglSortConfigs((const _EGLConfig **)configList, count, compare,
                      priv_data);
      count = MIN2(count, config_size);
      for (i = 0; i < count; i++)
         configs[i] = _eglGetConfigHandle(configList[i]);
   }

   free(configList);

   *num_configs = count;

   return EGL_TRUE;
}

static EGLint
_eglFallbackCompare(const _EGLConfig *conf1, const _EGLConfig *conf2,
                    void *priv_data)
{
   return _eglCompareConfigs(conf1, conf2, (const _EGLConfig *)priv_data,
                             EGL_TRUE);
}

/**
 * Typical fallback routine for eglChooseConfig
 */
EGLBoolean
_eglChooseConfig(_EGLDisplay *disp, const EGLint *attrib_list,
                 EGLConfig *configs, EGLint config_size, EGLint *num_configs)
{
   _EGLConfig criteria;
   EGLBoolean result;

   if (!_eglParseConfigAttribList(&criteria, disp, attrib_list))
      return _eglError(EGL_BAD_ATTRIBUTE, "eglChooseConfig");

   result = _eglFilterConfigArray(disp->Configs, configs, config_size,
                                  num_configs, _eglMatchConfig,
                                  _eglFallbackCompare, (void *)&criteria);

   if (result && (_eglGetLogLevel() == _EGL_DEBUG))
      eglPrintConfigDebug(disp, configs, *num_configs, EGL_TRUE);

   return result;
}

/**
 * Fallback for eglGetConfigAttrib.
 */
EGLBoolean
_eglGetConfigAttrib(const _EGLDisplay *disp, const _EGLConfig *conf,
                    EGLint attribute, EGLint *value)
{
   if (!_eglIsConfigAttribValid(conf, attribute))
      return _eglError(EGL_BAD_ATTRIBUTE, "eglGetConfigAttrib");

   /* nonqueryable attributes */
   switch (attribute) {
   case EGL_MATCH_NATIVE_PIXMAP:
      return _eglError(EGL_BAD_ATTRIBUTE, "eglGetConfigAttrib");
      break;
   default:
      break;
   }

   if (!value)
      return _eglError(EGL_BAD_PARAMETER, "eglGetConfigAttrib");

   *value = _eglGetConfigKey(conf, attribute);
   return EGL_TRUE;
}

static EGLBoolean
_eglFlattenConfig(void *elem, void *buffer)
{
   _EGLConfig *conf = (_EGLConfig *)elem;
   EGLConfig *handle = (EGLConfig *)buffer;
   *handle = _eglGetConfigHandle(conf);
   return EGL_TRUE;
}

/**
 * Fallback for eglGetConfigs.
 */
EGLBoolean
_eglGetConfigs(_EGLDisplay *disp, EGLConfig *configs, EGLint config_size,
               EGLint *num_config)
{
   *num_config =
      _eglFlattenArray(disp->Configs, (void *)configs, sizeof(configs[0]),
                       config_size, _eglFlattenConfig);

   if (_eglGetLogLevel() == _EGL_DEBUG)
      eglPrintConfigDebug(disp, configs, *num_config, EGL_FALSE);

   return EGL_TRUE;
}
