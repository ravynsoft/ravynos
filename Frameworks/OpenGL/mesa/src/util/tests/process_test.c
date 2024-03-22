/*
 * Copyright © 2020 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/* A collection of unit tests for u_process.c */

#include "util/detect_os.h"
#include "util/u_process.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>

#if DETECT_OS_WINDOWS && !defined(PATH_MAX)
#include <windows.h>
#define PATH_MAX MAX_PATH
#endif

static bool error = false;

static void
expect_equal_str(const char *expected, const char *actual, const char *test)
{
   if (strcmp(expected, actual)) {
      fprintf (stderr, "Error: Test '%s' failed:\n\t"
               "Expected=\"%s\", Actual=\"%s\"\n",
               test, expected, actual);
      error = true;
   }
}

static void
test_util_get_process_name (void)
{
#if DETECT_OS_WINDOWS
   const char *expected = "process_test.exe";
#else
   const char *expected = "process_test";
#endif

   const char *name_override = getenv("MESA_PROCESS_NAME");
   if (name_override)
      expected = name_override;

   const char *name = util_get_process_name();
   expect_equal_str(expected, name, "util_get_process_name");
}

static void posixify_path(char *path) {
   /* Always using posix separator '/' to check path equal */
   char *p = path;
   for (; *p != '\0'; p += 1) {
      if (*p == '\\') {
         *p = '/';
      }
   }
}

/* This test gets the real path from Meson (BUILD_FULL_PATH env var),
 * and compares it to the output of util_get_process_exec_path.
 */
static void
test_util_get_process_exec_path (void)
{
   char path[PATH_MAX];
   if (util_get_process_exec_path(path, PATH_MAX) == 0) {
      error = true;
      return;
   }
   posixify_path(path);
   char* build_path = getenv("BUILD_FULL_PATH");
   if (!build_path) {
      fprintf(stderr, "BUILD_FULL_PATH environment variable should be set\n");
      error = true;
      return;
   }
   build_path = strdup(build_path);
   posixify_path(build_path);
#ifdef __CYGWIN__
   int i = strlen(build_path) - 4;
   if ((i > 0) && (strcmp(&build_path[i], ".exe") == 0))
      build_path[i] = 0;
#endif
   expect_equal_str(build_path, path, "test_util_get_process_exec_path");
   free(build_path);
}

int
main (void)
{
   test_util_get_process_name();
   test_util_get_process_exec_path();

   return error ? 1 : 0;
}
