# Initialize Autoconf
AC_PREREQ([2.60])
AC_INIT([Multitouch Protocol Translation Library],
        [1.1.7],
        [],
        [mtdev])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([config-aux])

# Initialize Automake
AM_INIT_AUTOMAKE([foreign dist-bzip2])
AM_SILENT_RULES([yes])

LIB_VERSION=1:0:0
AC_SUBST([LIB_VERSION])

# Initialize libtool
AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 test/Makefile
                 mtdev.pc])
AC_OUTPUT
