dnl                                            -*- Autoconf -*-
dnl Process this file with autoconf to produce a configure script.
dnl
dnl   Copyright (C) 2019-2023 Free Software Foundation, Inc.
dnl
dnl This file is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; see the file COPYING.  If not see
dnl <http://www.gnu.org/licenses/>.
dnl

AC_PREREQ(2.64)
AC_INIT([libctf], 1.2.0)
AC_CONFIG_SRCDIR(ctf-impl.h)
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AC_USE_SYSTEM_EXTENSIONS
AM_INIT_AUTOMAKE
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_MAKE_SET
AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_AR
ZW_GNU_GETTEXT_SISTER_DIR

dnl Default to a non shared library.  This may be overridden by the
dnl configure option --enable-shared.
AC_DISABLE_SHARED

LT_INIT
AC_SYS_LARGEFILE
LT_OUTPUT

MISSING=`cd $ac_aux_dir && ${PWDCMD-pwd}`/missing
AC_CHECK_PROGS([ACLOCAL], [aclocal], [$MISSING aclocal])
AC_CHECK_PROGS([AUTOCONF], [autoconf], [$MISSING autoconf])
AC_CHECK_PROGS([AUTOHEADER], [autoheader], [$MISSING autoheader])

# Figure out what compiler warnings we can enable.
# See config/warnings.m4 for details.

ACX_PROG_CC_WARNING_OPTS([-W -Wall -Wno-narrowing -Wwrite-strings \
			  -Wmissing-format-attribute], [warn])
ACX_PROG_CC_WARNING_OPTS([-Wstrict-prototypes -Wmissing-prototypes \
			  -Wold-style-definition], [c_warn])
ACX_PROG_CC_WARNING_ALMOST_PEDANTIC([-Wno-long-long])

# Only enable with --enable-werror-always until existing warnings are
# corrected.
ACX_PROG_CC_WARNINGS_ARE_ERRORS([manual])

AM_MAINTAINER_MODE
AM_INSTALL_LIBBFD
ACX_PROG_CC_WARNING_OPTS([-Wall], [ac_libctf_warn_cflags])

AC_FUNC_MMAP
# Needed for BFD capability checks.
AC_SEARCH_LIBS(dlsym, dl)
AM_ZLIB

GCC_ENABLE([libctf-hash-debugging], [no], [], [Enable expensive debugging of CTF deduplication type hashing])
if test "${enable_libctf_hash_debugging}" = yes; then
    AC_DEFINE(ENABLE_LIBCTF_HASH_DEBUGGING, 1, [Enable expensive debugging of CTF deduplication type hashing])
fi
AM_CONDITIONAL(ENABLE_LIBCTF_HASH_DEBUGGING, test "${enable_libctf_hash_debugging}" = yes)

# Similar to GDB_AC_CHECK_BFD.
OLD_CFLAGS=$CFLAGS
OLD_LDFLAGS=$LDFLAGS
OLD_LIBS=$LIBS
OLD_CC=$CC
# Put the old CFLAGS/LDFLAGS last, in case the user's (C|LD)FLAGS
# points somewhere with bfd, with -I/foo/lib and -L/foo/lib.  We
# always want our bfd.
CC="./libtool --quiet --mode=link $OLD_CC"
CFLAGS="-I${srcdir}/../include -I../bfd -I${srcdir}/../bfd $CFLAGS"
LDFLAGS="-L../bfd -L../libiberty $LDFLAGS"
intl=`echo $LIBINTL | sed 's,${top_builddir}/,,g'`
LIBS="-lbfd -liberty $intl $LIBS"
AC_CACHE_CHECK([for ELF support in BFD], ac_cv_libctf_bfd_elf,
[AC_TRY_LINK([#include <stdlib.h>
	     #include <string.h>
	     #include "bfd.h"
	     #include "elf-bfd.h"],
	    [(void) bfd_section_from_elf_index (NULL, 0);
	     return 0;],
	    [ac_cv_libctf_bfd_elf=yes],
	    [ac_cv_libctf_bfd_elf=no])])
CFLAGS=$OLD_CFLAGS
LDFLAGS=$OLD_LDFLAGS
LIBS=$OLD_LIBS
CC=$OLD_CC

if test $ac_cv_libctf_bfd_elf = yes; then
  AC_DEFINE([HAVE_BFD_ELF], 1,
	    [Whether libbfd was configured for an ELF target.])
fi

AC_C_BIGENDIAN
AC_CHECK_HEADERS(byteswap.h endian.h)
AC_CHECK_FUNCS(pread)

dnl Check for bswap_{16,32,64}
AC_CHECK_DECLS([bswap_16, bswap_32, bswap_64], [], [], [[#include <byteswap.h>]])
AC_CHECK_DECLS([asprintf, vasprintf, stpcpy])

dnl Check for qsort_r.  (Taken from gnulib.)
AC_CHECK_FUNCS_ONCE([qsort_r])
if test $ac_cv_func_qsort_r = yes; then
  AC_CACHE_CHECK([for qsort_r signature], [ac_cv_libctf_qsort_r_signature],
    [AC_LINK_IFELSE(
       [AC_LANG_PROGRAM([[#undef qsort_r
                          #include <stdlib.h>
                          void qsort_r (void *, size_t, size_t,
                                        int (*) (void const *, void const *,
                                                 void *),
                                        void *);
                          void (*p) (void *, size_t, size_t,
                                     int (*) (void const *, void const *,
                                              void *),
                                     void *) = qsort_r;
                        ]])],
       [ac_cv_libctf_qsort_r_signature=GNU],
       [AC_LINK_IFELSE(
          [AC_LANG_PROGRAM([[#undef qsort_r
	                     #include <stdlib.h>
                             void qsort_r (void *, size_t, size_t, void *,
                                           int (*) (void *,
                                                    void const *,
                                                    void const *));
                             void (*p) (void *, size_t, size_t, void *,
                                        int (*) (void *, void const *,
                                                 void const *)) = qsort_r;
                           ]])],
          [ac_cv_libctf_qsort_r_signature=BSD],
          [ac_cv_libctf_qsort_r_signature=unknown])])])
fi

case x$ac_cv_libctf_qsort_r_signature in
  xGNU)     AC_DEFINE([HAVE_QSORT_R_ARG_LAST], 1,
	     [Whether a qsort_r exists with a void *arg as its last arg.]);;
  xBSD)     AC_DEFINE([HAVE_QSORT_R_COMPAR_LAST], 1,
	     [Whether a qsort_r exists with the compar function as its last arg.]);;
  *) ac_cv_libctf_qsort_r_signature=unknown;;
esac

AM_CONDITIONAL(NEED_CTF_QSORT_R, test "${ac_cv_libctf_qsort_r_signature}" = unknown)

AC_CACHE_CHECK([for O_CLOEXEC], [ac_cv_libctf_macro_O_CLOEXEC],
  [AC_LINK_IFELSE(
    [AC_LANG_PROGRAM([[#include <fcntl.h>
                       #ifndef O_CLOEXEC
                         choke me;
                       #endif
                     ]],
                     [[return O_CLOEXEC;]])],
    [ac_cv_libctf_macro_O_CLOEXEC=yes],
    [ac_cv_libctf_macro_O_CLOEXEC=no])])

if test $ac_cv_libctf_macro_O_CLOEXEC = yes; then
  AC_DEFINE([HAVE_O_CLOEXEC], 1,
	    [Whether the platform has a definition of O_CLOEXEC.])
fi

build_info=
makeinfo_too_old=
AC_CHECK_PROGS([MAKEINFO], makeinfo, [$MISSING makeinfo])
case " $build_configdirs " in
  *" texinfo "*) MAKEINFO='$$r/$(BUILD_SUBDIR)/texinfo/makeinfo/makeinfo' ;;
esac
changequote(,)
    # We require texinfo to be 6.3 or later, for a working synindex
    # and validatemenus: otherwise we fall back to /bin/true.
    if ${MAKEINFO} --version \
       | egrep 'texinfo[^0-9]*(6\.[3-9]|[7-9]|[1-6][0-9])' >/dev/null 2>&1; then
      build_info=yes
    else
        build_info=
        makeinfo_too_old=t
    fi
changequote([,])
if test -n "$makeinfo_too_old"; then
    AC_MSG_WARN([
*** Makeinfo is too old. Info documentation will not be built.])
fi
AC_SUBST(MAKEINFO)
AM_CONDITIONAL(BUILD_INFO, test "${build_info}" = yes)

CTF_LIBADD="-L`pwd`/../libiberty -liberty"
SHARED_LDFLAGS=

# Horrible hacks to build DLLs on Windows and a shared library elsewhere.
if test "$enable_shared" = "yes"; then
# When building a shared libctf, link against the pic version of libiberty
# so that apps that use libctf won't need libiberty just to satisfy any
# libctf references.  We can't do that if a pic libiberty is unavailable
# since including non-pic # code would insert text relocations into libctf.
# Note that linking against libbfd as we do here, which is itself linked
# against libiberty, may not satisfy all the libctf libiberty references
# since libbfd may not pull in the entirety of libiberty.
# Also, jam libintl into the right place in all of this: after libiberty,
# which uses it, but before -lcygwin, which it uses.
changequote(,)dnl
  x=`sed -n -e 's/^[ 	]*PICFLAG[ 	]*=[ 	]*//p' < ../libiberty/Makefile | sed -n '$p'`
changequote([,])dnl
  if test -n "$x"; then
    CTF_LIBADD="-L`pwd`/../libiberty/pic -liberty"
  fi
fi

CTF_LIBADD="$CTF_LIBADD $LIBINTL"

if test "$enable_shared" = "yes"; then
  case "${host}" in
  # More hacks to build DLLs on Windows.
    *-*-cygwin*)
      SHARED_LDFLAGS="-no-undefined"
      CTF_LIBADD="$CTF_LIBADD -lcygwin"
      ;;
  esac
fi
AC_SUBST(SHARED_LDFLAGS)
AC_SUBST(CTF_LIBADD)

AC_PATH_PROG([EXPECT], [expect])
AC_CACHE_CHECK([for Tcl supporting try/catch], [ac_cv_libctf_tcl_try],
  [ac_cv_libctf_tcl_try=`if test -z $EXPECT; then echo no; else $EXPECT << EOF
if @<:@llength @<:@info commands try@:>@@:>@ then { puts yes } else { puts no }
EOF
fi`
])

HAVE_TCL_TRY=false
if test "${ac_cv_libctf_tcl_try}" = "yes"; then
    HAVE_TCL_TRY=true
fi
AC_SUBST(HAVE_TCL_TRY)

# Use a version script, if possible, or an -export-symbols-regex otherwise.
decommented_version_script=
AC_CACHE_CHECK([for linker versioning flags], [ac_cv_libctf_version_script],
  [echo 'FOO { global: mai*; local: ctf_fo*; };' > conftest.ver
   old_LDFLAGS="$LDFLAGS"
   old_CFLAGS="$CFLAGS"
   LDFLAGS="$LDFLAGS -shared -Wl,--version-script=conftest.ver"
   CFLAGS="$CFLAGS -fPIC"
   AC_LINK_IFELSE([AC_LANG_SOURCE([[int ctf_foo (void) { return 0; }
				    int main (void) { return ctf_foo(); }]])],
		  [ac_cv_libctf_version_script="-Wl,--version-script='$srcdir/libctf.ver'"],
		  [])
   LDFLAGS="$old_LDFLAGS"

   if test -z "$ac_cv_libctf_version_script"; then
     LDFLAGS="$LDFLAGS -shared -Wl,-B,local -Wl,-z,gnu-version-script=conftest.ver"
     AC_LINK_IFELSE([AC_LANG_SOURCE([[int ctf_foo (void) { return 0; }
				      int main (void) { return ctf_foo(); }]])],
		    [ac_cv_libctf_version_script="-Wl,-B,local -Wl,-z,gnu-version-script"
		     decommented_version_script=t],
		    [])
     LDFLAGS="$old_LDFLAGS"
   fi
   CFLAGS="$old_CFLAGS"

   if test -z "$ac_cv_libctf_version_script"; then
     ac_cv_libctf_version_script='-export-symbols-regex ctf_.*'
   fi
   rm -f conftest.ver])
if test -n "$decommented_version_script"; then
   # Solaris's version scripts use shell-style comments rather than the C-style
   # used by GNU ld.  Use cpp to strip the comments out.  (cpp exists under this
   # name on all platforms that support ld -z gnu-version-script.)
   # Also ensure that no symbols exist in the version script for libctf-nobfd.so
   # that do not exist in the shared library itself, since some linkers add such
   # symbols with type NOTYPE.
   /lib/cpp < $srcdir/libctf.ver > libctf-decommented.ver
   grep -v 'libctf only' $srcdir/libctf.ver | /lib/cpp > libctf-nobfd-decommented.ver
   VERSION_FLAGS="$ac_cv_libctf_version_script='libctf-decommented.ver'"
   VERSION_FLAGS_NOBFD="$ac_cv_libctf_version_script='libctf-nobfd-decommented.ver'"
else
   VERSION_FLAGS="$ac_cv_libctf_version_script"
   VERSION_FLAGS_NOBFD="$ac_cv_libctf_version_script"
fi
AC_SUBST(VERSION_FLAGS)
AC_SUBST(VERSION_FLAGS_NOBFD)

AC_CONFIG_FILES(Makefile)
AC_CONFIG_HEADERS(config.h)
AC_OUTPUT

GNU_MAKE_JOBSERVER
