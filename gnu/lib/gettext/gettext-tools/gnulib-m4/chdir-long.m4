#serial 17

# Use Gnulib's robust chdir function.
# It can handle arbitrarily long directory names, which means
# that when it is given the name of an existing directory, it
# never fails with ENAMETOOLONG.
# Arrange to compile chdir-long.c only on systems that define PATH_MAX.

dnl Copyright (C) 2004-2007, 2009-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# Written by Jim Meyering.

AC_DEFUN([gl_FUNC_CHDIR_LONG],
[
  AC_REQUIRE([gl_PATHMAX_SNIPPET_PREREQ])
  AC_CACHE_CHECK([whether this system supports file names of any length],
    [gl_cv_have_unlimited_file_name_length],
    [AC_EGREP_CPP([have_arbitrary_file_name_length_limit],
                  gl_PATHMAX_SNIPPET[
#ifdef PATH_MAX
have_arbitrary_file_name_length_limit
#endif],
    [gl_cv_have_unlimited_file_name_length=no],
    [gl_cv_have_unlimited_file_name_length=yes])])
])

AC_DEFUN([gl_PREREQ_CHDIR_LONG], [:])
