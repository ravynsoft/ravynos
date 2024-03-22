/*
 * Copyright 2019 Imagination Technologies.
 * All Rights Reserved.
 *
 * Based on eglinfo, which has copyright:
 * Copyright (C) 2005  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/macros.h"
#include "eglarray.h"
#include "eglconfig.h"
#include "eglconfigdebug.h"
#include "egldisplay.h"
#include "egllog.h"
#include "egltypedefs.h"

/* Max debug message length */
#define CONFIG_DEBUG_MSG_MAX 1000

/*
 * These are X visual types, so if you're running eglinfo under
 * something not X, they probably don't make sense.
 */
static const char *const vnames[] = {"SG", "GS", "SC", "PC", "TC", "DC"};

static void
_printHeaderFormat(void)
{
   /*
    * EGL configuration output legend:
    *
    * chosen --------------- eglChooseConfig returned config priority,
    *                        only relevant when eglChooseConfig is called.
    * id ------------------- EGL_CONFIG_ID
    * bfsz ----------------- EGL_BUFFER_SIZE
    * lvl ------------------ EGL_LEVEL
    *
    * colourbuffer
    * r -------------------- EGL_RED_SIZE
    * g -------------------- EGL_GREEN_SIZE
    * b -------------------- EGL_BLUE_SIZE
    * a -------------------- EGL_ALPHA_SIZE
    * dpth ----------------- EGL_DEPTH_SIZE
    * stcl ----------------- EGL_STENCIL_SIZE
    *
    * multisample
    * ns ------------------- EGL_SAMPLES
    * b -------------------- EGL_SAMPLE_BUFFERS
    * visid ---------------- EGL_NATIVE_VISUAL_ID/EGL_NATIVE_VISUAL_TYPE
    * caveat --------------- EGL_CONFIG_CAVEAT
    * bind ----------------- EGL_BIND_TO_TEXTURE_RGB/EGL_BIND_TO_TEXTURE_RGBA
    *
    * renderable
    * gl, es, es2, es3, vg - EGL_RENDERABLE_TYPE
    *
    * supported
    * surfaces ------------- EGL_SURFACE_TYPE
    */
   /* clang-format off */
   _eglLog(_EGL_DEBUG, "---------------");
   _eglLog(_EGL_DEBUG, "Configurations:");
   _eglLog(_EGL_DEBUG, "cho       bf lv colourbuffer dp st  ms           vis  cav  bi     renderable           supported");
   _eglLog(_EGL_DEBUG, "sen    id sz  l  r  g  b  a  th cl ns b           id  eat  nd  gl es es2 es3 vg         surfaces");
   _eglLog(_EGL_DEBUG, "---------------");
   /* clang-format on */
}

/* Append a formatted string to the buffer, up to the buffer size */
static inline void
_strnAppend(char *const buf, const int bufSize, const char *fmt, ...)
{
   int maxAllowed;
   va_list args;
   size_t bufLen = strlen(buf);

   maxAllowed = bufSize - bufLen;
   assert(maxAllowed >= 0);

   va_start(args, fmt);
   (void)vsnprintf(&buf[bufLen], maxAllowed, fmt, args);
   va_end(args);
}

static void
_eglPrintConfig(_EGLConfig *const conf, const int chosenIndex)
{
   const char padding[] = "   ";
   char printMsg[CONFIG_DEBUG_MSG_MAX] = "";
   char surfString[32] = "";
   EGLint renderable, surfaces, vtype, bindRgb, bindRgba;

   vtype = conf->NativeVisualType;
   surfaces = conf->SurfaceType;

   STATIC_ASSERT(sizeof(surfString) >= sizeof("win,pb,pix,str,prsv"));

   if (surfaces & EGL_WINDOW_BIT)
      strcat(surfString, "win,");
   if (surfaces & EGL_PBUFFER_BIT)
      strcat(surfString, "pb,");
   if (surfaces & EGL_PIXMAP_BIT)
      strcat(surfString, "pix,");
   if (surfaces & EGL_STREAM_BIT_KHR)
      strcat(surfString, "str,");
   if (surfaces & EGL_SWAP_BEHAVIOR_PRESERVED_BIT)
      strcat(surfString, "prsv");

   /* If one of chosen configs, print its index in the returned config array */
   if (chosenIndex >= 0)
      _strnAppend(printMsg, sizeof(printMsg), "%*d ", strlen(padding),
                  chosenIndex);
   else
      _strnAppend(printMsg, sizeof(printMsg), "%s ", &padding[0]);

   _strnAppend(printMsg, sizeof(printMsg),
               "0x%03x %2d %2d %2d %2d %2d %2d  %2d %2d %2d%2d 0x%08x%2s     ",
               conf->ConfigID, conf->BufferSize, conf->Level, conf->RedSize,
               conf->GreenSize, conf->BlueSize, conf->AlphaSize,
               conf->DepthSize, conf->StencilSize, conf->Samples,
               conf->SampleBuffers, conf->NativeVisualID,
               vtype < 6 ? vnames[vtype] : "--");

   bindRgb = conf->BindToTextureRGB;
   bindRgba = conf->BindToTextureRGBA;
   renderable = conf->RenderableType;

   _strnAppend(printMsg, sizeof(printMsg),
               "%c  %c   %c  %c   %c   %c   %c %15s",
               (conf->ConfigCaveat != EGL_NONE) ? 'y' : ' ',
               (bindRgba)  ? 'a'
               : (bindRgb) ? 'y'
                           : ' ',
               (renderable & EGL_OPENGL_BIT) ? 'y' : ' ',
               (renderable & EGL_OPENGL_ES_BIT) ? 'y' : ' ',
               (renderable & EGL_OPENGL_ES2_BIT) ? 'y' : ' ',
               (renderable & EGL_OPENGL_ES3_BIT) ? 'y' : ' ',
               (renderable & EGL_OPENVG_BIT) ? 'y' : ' ', surfString);

   _eglLog(_EGL_DEBUG, printMsg);
}

void
eglPrintConfigDebug(const _EGLDisplay *const disp,
                    const EGLConfig *const configs, const EGLint numConfigs,
                    const EGLBoolean printChosen)
{
   EGLint numConfigsToPrint;
   _EGLConfig **configsToPrint;
   _EGLConfig **chosenConfigs;

   if (!numConfigs || !configs) {
      _eglLog(_EGL_DEBUG, "%s: nothing to print", __func__);
      return;
   }

   /*
    * If the printout request came from the 'eglChooseConfig', all
    * configs are printed, and the "chosen" configs are marked.
    */
   if (printChosen) {
      configsToPrint = (_EGLConfig **)disp->Configs->Elements;
      numConfigsToPrint = disp->Configs->Size;
      chosenConfigs = (_EGLConfig **)configs;
   } else {
      configsToPrint = (_EGLConfig **)configs;
      numConfigsToPrint = numConfigs;
      chosenConfigs = NULL;
   }

   _printHeaderFormat();
   for (EGLint i = 0; i < numConfigsToPrint; i++) {
      _EGLConfig *configToPrint = configsToPrint[i];
      EGLint chosenIndex = -1;

      /* See if the current config to print is one of the chosen configs */
      if (chosenConfigs)
         for (EGLint j = 0; j < numConfigs; j++)
            if (configToPrint == chosenConfigs[j])
               chosenIndex = j;

      _eglPrintConfig(configToPrint, chosenIndex);
   }
}
