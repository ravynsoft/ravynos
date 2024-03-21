/* Create a temporary file or directory.

   Copyright (C) 2006, 2009-2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* header written by Eric Blake */

#ifndef GL_TEMPNAME_H
# define GL_TEMPNAME_H

# include <stdio.h>

# ifdef __GT_FILE
#  define GT_FILE     __GT_FILE
#  define GT_DIR      __GT_DIR
#  define GT_NOCREATE __GT_NOCREATE
# else
#  define GT_FILE     0
#  define GT_DIR      1
#  define GT_NOCREATE 2
# endif

#ifdef __cplusplus
extern "C" {
#endif

/* Generate a temporary file name based on TMPL.  TMPL must match the
   rules for mk[s]temp (i.e. end in "XXXXXX", possibly with a suffix).
   The name constructed does not exist at the time of the call to
   gen_tempname.  TMPL is overwritten with the result.

   KIND may be one of:
   GT_NOCREATE:         simply verify that the name does not exist
                        at the time of the call.
   GT_FILE:             create a large file using open(O_CREAT|O_EXCL)
                        and return a read-write fd.  The file is mode 0600.
   GT_DIR:              create a directory, which will be mode 0700.

   */
extern int gen_tempname (char *tmpl, int suffixlen, int flags, int kind);
/* Similar, except X_SUFFIX_LEN gives the number of Xs.  */
extern int gen_tempname_len (char *tmpl, int suffixlen, int flags, int kind,
                             size_t x_suffix_len);

/* Similar to gen_tempname, but TRYFUNC is called for each temporary
   name to try.  If TRYFUNC returns a non-negative number, TRY_GEN_TEMPNAME
   returns with this value.  Otherwise, if errno is set to EEXIST, another
   name is tried, or else TRY_GEN_TEMPNAME returns -1. */
extern int try_tempname (char *tmpl, int suffixlen, void *args,
                         int (*tryfunc) (char *, void *));
/* Similar, except X_SUFFIX_LEN gives the number of Xs.  */
extern int try_tempname_len (char *tmpl, int suffixlen, void *args,
                             int (*tryfunc) (char *, void *),
                             size_t x_suffix_len);

#ifdef __cplusplus
}
#endif

#endif /* GL_TEMPNAME_H */
