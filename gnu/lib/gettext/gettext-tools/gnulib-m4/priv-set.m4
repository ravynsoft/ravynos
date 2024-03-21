# serial 8

# Copyright (C) 2009-2023 Free Software Foundation, Inc.
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# Written by David Bartley.

AC_DEFUN([gl_PRIV_SET],
[
  AC_CHECK_FUNCS([getppriv])
  AC_CHECK_HEADERS_ONCE([priv.h])
])
