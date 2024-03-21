# xattr.m4 - check for Extended Attributes (Linux)
# serial 7

# Copyright (C) 2003-2023 Free Software Foundation, Inc.
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_XATTR],
[
  AC_ARG_ENABLE([xattr],
        AS_HELP_STRING([[--disable-xattr]],
                       [do not support extended attributes]),
        [use_xattr=$enableval], [use_xattr=yes])

  LIB_XATTR=
  AC_SUBST([LIB_XATTR])

  if test "$use_xattr" = yes; then
    AC_CACHE_CHECK([for xattr library with ATTR_ACTION_PERMISSIONS],
      [gl_cv_xattr_lib],
      [gl_cv_xattr_lib=no
       AC_LANG_CONFTEST(
         [AC_LANG_PROGRAM(
            [[#include <attr/error_context.h>
              #include <attr/libattr.h>
              static int
              is_attr_permissions (const char *name, struct error_context *ctx)
              {
                return attr_copy_action (name, ctx) == ATTR_ACTION_PERMISSIONS;
              }
            ]],
            [[return attr_copy_fd ("/", 0, "/", 0, is_attr_permissions, 0);
            ]])])
       AC_LINK_IFELSE([],
         [gl_cv_xattr_lib='none required'],
         [xattr_saved_LIBS=$LIBS
          LIBS="-lattr $LIBS"
          AC_LINK_IFELSE([], [gl_cv_xattr_lib=-lattr])
          LIBS=$xattr_saved_LIBS])])
    if test "$gl_cv_xattr_lib" = no; then
      AC_MSG_WARN([libattr development library was not found or not usable.])
      AC_MSG_WARN([AC_PACKAGE_NAME will be built without xattr support.])
      use_xattr=no
    elif test "$gl_cv_xattr_lib" != 'none required'; then
      LIB_XATTR=$gl_cv_xattr_lib
    fi
  fi
  if test "$use_xattr" = yes; then
    AC_DEFINE([USE_XATTR], [1],
      [Define to 1 to use the Linux extended attributes library.])
  fi
])
