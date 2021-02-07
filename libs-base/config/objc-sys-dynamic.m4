AC_DEFUN(OBJC_SYS_DYNAMIC_LINKER,
[dnl
AC_REQUIRE([OBJC_CON_AUTOLOAD])dnl
# Copyright (C) 2005 Free Software Foundation
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.
#--------------------------------------------------------------------
# Guess the type of dynamic linker for the system
#
# Makes the following substitutions:
#	DYNAMIC_LINKER	- cooresponds to the interface that is included
#		in objc-load.c (i.e. #include "${DYNAMIC_LINKER}-load.h")
#--------------------------------------------------------------------
DYNAMIC_LINKER=null
AC_CHECK_HEADER(windows.h, DYNAMIC_LINKER=win32)
if test $DYNAMIC_LINKER = null; then
  AC_CHECK_HEADER(dlfcn.h, DYNAMIC_LINKER=simple)
fi
if test $DYNAMIC_LINKER = null; then
  AC_CHECK_HEADER(dl.h, DYNAMIC_LINKER=hpux)
fi
if test $DYNAMIC_LINKER = null; then
  AC_CHECK_HEADER(dld/defs.h, DYNAMIC_LINKER=dld)
fi

# NB: This is used as follows: in Source/Makefile.postamble we copy
# $(DYNAMIC_LINKER)-load.h into dynamic-load.h
AC_MSG_CHECKING([for dynamic linker type])
AC_MSG_RESULT([$DYNAMIC_LINKER])

if test $DYNAMIC_LINKER = simple; then
  AC_CHECK_LIB(dl, dladdr)
fi

AC_SUBST(DYNAMIC_LINKER)dnl
])
