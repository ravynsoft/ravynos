/*
 * Copyright (c) 2012-2013 Etnaviv Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/* Common debug stuffl */
#ifndef H_ETNA_DEBUG
#define H_ETNA_DEBUG

#include "util/u_debug.h"
#include "util/log.h"
#include "util/macros.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum etna_dbg {
   /* Logging */
   ETNA_DBG_MSGS            = BITFIELD_BIT(0), /* Warnings and non-fatal errors */
   ETNA_DBG_FRAME_MSGS      = BITFIELD_BIT(1),
   ETNA_DBG_RESOURCE_MSGS   = BITFIELD_BIT(2),
   ETNA_DBG_COMPILER_MSGS   = BITFIELD_BIT(3),
   ETNA_DBG_LINKER_MSGS     = BITFIELD_BIT(4),
   ETNA_DBG_DUMP_SHADERS    = BITFIELD_BIT(5),
   ETNA_DRM_MSGS            = BITFIELD_BIT(6), /* Debug messages from DRM */
   ETNA_DBG_PERF            = BITFIELD_BIT(7),

   /* Bypasses */
   ETNA_DBG_NO_TS           = BITFIELD_BIT(12), /* Disable TS */
   ETNA_DBG_NO_AUTODISABLE  = BITFIELD_BIT(13), /* Disable autodisable */
   ETNA_DBG_NO_SUPERTILE    = BITFIELD_BIT(14), /* Disable supertile */
   ETNA_DBG_NO_EARLY_Z      = BITFIELD_BIT(15), /* Disable early z */
   ETNA_DBG_CFLUSH_ALL      = BITFIELD_BIT(16), /* Flush before every state update + draw call */
   ETNA_DBG_FINISH_ALL      = BITFIELD_BIT(17), /* Finish on every flush */
   ETNA_DBG_FLUSH_ALL       = BITFIELD_BIT(18), /* Flush after every rendered primitive */
   ETNA_DBG_ZERO            = BITFIELD_BIT(19), /* Zero all resources after allocation */
   ETNA_DBG_DRAW_STALL      = BITFIELD_BIT(20), /* Stall FE/PE after every draw op */
   ETNA_DBG_SHADERDB        = BITFIELD_BIT(21), /* dump program compile information */
   ETNA_DBG_NO_SINGLEBUF    = BITFIELD_BIT(22), /* disable single buffer feature */
   ETNA_DBG_DEQP            = BITFIELD_BIT(23), /* Hacks to run dEQP GLES3 tests */
   ETNA_DBG_NOCACHE         = BITFIELD_BIT(24), /* Disable shader cache */
   ETNA_DBG_LINEAR_PE       = BITFIELD_BIT(25), /* Enable linear PE */
   ETNA_DBG_MSAA            = BITFIELD_BIT(26), /* Enable MSAA */
   ETNA_DBG_SHARED_TS       = BITFIELD_BIT(27), /* Enable TS sharing */
};

extern int etna_mesa_debug; /* set in etnaviv_screen.c from ETNA_MESA_DEBUG */

#define DBG_ENABLED(flag) unlikely(etna_mesa_debug & (flag))

#define DBG_F(flag, fmt, ...)                             \
   do {                                                   \
      if (DBG_ENABLED(flag))                              \
         mesa_logd("%s:%d: " fmt, __func__, __LINE__,     \
                   ##__VA_ARGS__);                        \
   } while (0)

#define DBG(fmt, ...)                                     \
   do {                                                   \
      if (DBG_ENABLED(ETNA_DBG_MSGS))                     \
         mesa_logd("%s:%d: " fmt, __func__, __LINE__,     \
                   ##__VA_ARGS__);                        \
   } while (0)

/* A serious bug, show this even in non-debug mode */
#define BUG(fmt, ...)                                                  \
   do {                                                                \
      mesa_loge("%s:%d: " fmt, __func__, __LINE__, ##__VA_ARGS__);     \
   } while (0)

#define perf_debug_message(debug, type, ...)                           \
   do {                                                                \
      if (DBG_ENABLED(ETNA_DBG_PERF))                                  \
         mesa_logw(__VA_ARGS__);                                       \
      struct util_debug_callback *__d = (debug);                       \
      if (__d)                                                         \
         util_debug_message(__d, type, __VA_ARGS__);                   \
   } while (0)

#define perf_debug_ctx(ctx, ...)                                                 \
   do {                                                                          \
      struct etna_context *__c = (ctx);                                          \
      perf_debug_message(__c ? &__c->base.debug : NULL, PERF_INFO, __VA_ARGS__); \
   } while (0)

#define perf_debug(...) perf_debug_ctx(NULL, PERF_INFO, __VA_ARGS__)

#endif
