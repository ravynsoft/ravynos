/* Test of command line argument processing.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2009.  */

#include <config.h>

/* None of the files accessed by this test are large, so disable the
   ftell link warning if we are not using the gnulib ftell module.  */
#define _GL_NO_LARGE_FILES

/* POSIX and glibc provide the getopt() function in <unistd.h>, see
   https://pubs.opengroup.org/onlinepubs/9699919799/functions/getopt.html
   https://www.gnu.org/software/libc/manual/html_node/Using-Getopt.html
   But gnulib provides the getopt() function in <getopt.h>, not in <unistd.h>.
   This is what we are testing here.  */
#include <getopt.h>

#ifndef __getopt_argv_const
# define __getopt_argv_const const
#endif
#include "signature.h"
SIGNATURE_CHECK (getopt_long, int, (int, char *__getopt_argv_const *,
                                    char const *, struct option const *,
                                    int *));
SIGNATURE_CHECK (getopt_long_only, int, (int, char *__getopt_argv_const *,
                                         char const *, struct option const *,
                                         int *));

#define TEST_GETOPT_GNU 1
#define TEST_GETOPT_TMP_NAME "test-getopt-gnu.tmp"
#include "test-getopt-main.h"
