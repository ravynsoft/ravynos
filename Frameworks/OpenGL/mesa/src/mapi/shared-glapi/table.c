/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2010 LunarG Inc.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Chia-I Wu <olv@lunarg.com>
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "c11/threads.h"
#include "table.h"

static nop_handler_proc nop_handler = NULL;

void
table_set_noop_handler(nop_handler_proc func)
{
   nop_handler = func;
}

static bool log_noop;

static void check_debug_env(void)
{
   const char *debug = getenv("MESA_DEBUG");
   if (!debug)
      debug = getenv("LIBGL_DEBUG");
   if (debug && strcmp(debug, "silent") != 0)
      log_noop = true;
}

static void
noop_warn(const char *name)
{
   if (nop_handler) {
      nop_handler(name);
   }
   else {
      static once_flag flag = ONCE_FLAG_INIT;
      call_once(&flag, check_debug_env);

      if (log_noop)
         fprintf(stderr, "%s is no-op\n", name);
   }
}

static int
noop_generic(void)
{
   noop_warn("function");
   return 0;
}

/* define noop_array */
#define MAPI_TMP_DEFINES
#define MAPI_TMP_NOOP_ARRAY
#include "mapi_tmp.h"
