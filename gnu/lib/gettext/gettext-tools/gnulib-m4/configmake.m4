# configmake.m4 serial 5
dnl Copyright (C) 2010-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_PREREQ([2.60])

# gl_CONFIGMAKE_PREP
# ------------------
# Guarantee all of the standard directory variables, even when used with
# autoconf 2.64 (runstatedir wasn't supported before 2.70) or
# automake 1.11 (runstatedir isn't supported even in 1.16.1).
AC_DEFUN([gl_CONFIGMAKE_PREP],
[
  if test "x$lispdir" = x; then
    AC_SUBST([lispdir], ['${datarootdir}/emacs/site-lisp'])
  fi
  dnl Added in autoconf 2.70.
  if test "x$runstatedir" = x; then
    AC_SUBST([runstatedir], ['${localstatedir}/run'])
  fi

  dnl Automake 1.11 provides the pkg*dir variables merely without AC_SUBST,
  dnl that is, only at the Makefile.am level.  AC_SUBST them, so that
  dnl gl_CONFIGMAKE can compute the final values at configure time.
  dnl Blindly assigning the value at configure time is OK, since configure
  dnl does not have --pkg*dir=... options.
  AC_SUBST([pkgdatadir], ['${datadir}/${PACKAGE}'])
  AC_SUBST([pkgincludedir], ['${includedir}/${PACKAGE}'])
  AC_SUBST([pkglibdir], ['${libdir}/${PACKAGE}'])
  AC_SUBST([pkglibexecdir], ['${libexecdir}/${PACKAGE}'])
])

# gl_CONFIGMAKE
# -------------
# Find the final values of the standard directory variables, and create
# AC_SUBSTed *_c and *_c_make variables with the corresponding values in
# target runtime environment ($host_os) syntax.
AC_DEFUN([gl_CONFIGMAKE],
[
  AC_REQUIRE([gl_CONFIGMAKE_PREP])

  dnl Save the values.
  gl_save_prefix="${prefix}"
  gl_save_exec_prefix="${exec_prefix}"
  gl_save_bindir="${bindir}"
  gl_save_sbindir="${sbindir}"
  gl_save_libexecdir="${libexecdir}"
  gl_save_datarootdir="${datarootdir}"
  gl_save_datadir="${datadir}"
  gl_save_sysconfdir="${sysconfdir}"
  gl_save_sharedstatedir="${sharedstatedir}"
  gl_save_localstatedir="${localstatedir}"
  gl_save_runstatedir="${runstatedir}"
  gl_save_includedir="${includedir}"
  gl_save_oldincludedir="${oldincludedir}"
  gl_save_docdir="${docdir}"
  gl_save_infodir="${infodir}"
  gl_save_htmldir="${htmldir}"
  gl_save_dvidir="${dvidir}"
  gl_save_pdfdir="${pdfdir}"
  gl_save_psdir="${psdir}"
  gl_save_libdir="${libdir}"
  gl_save_lispdir="${lispdir}"
  gl_save_localedir="${localedir}"
  gl_save_mandir="${mandir}"
  gl_save_pkgdatadir="${pkgdatadir}"
  gl_save_pkgincludedir="${pkgincludedir}"
  gl_save_pkglibdir="${pkglibdir}"
  gl_save_pkglibexecdir="${pkglibexecdir}"

  dnl Find the final values.
  dnl Unfortunately, prefix gets only finally determined at the end of
  dnl configure.
  if test "X$prefix" = "XNONE"; then
    prefix="$ac_default_prefix"
  fi
  dnl Unfortunately, exec_prefix gets only finally determined at the end of
  dnl configure.
  if test "X$exec_prefix" = "XNONE"; then
    exec_prefix='${prefix}'
  fi
  eval exec_prefix="$exec_prefix"
  eval bindir="$bindir"
  eval sbindir="$sbindir"
  eval libexecdir="$libexecdir"
  eval datarootdir="$datarootdir"
  eval datadir="$datadir"
  eval sysconfdir="$sysconfdir"
  eval sharedstatedir="$sharedstatedir"
  eval localstatedir="$localstatedir"
  eval runstatedir="$runstatedir"
  eval includedir="$includedir"
  eval oldincludedir="$oldincludedir"
  eval docdir="$docdir"
  eval infodir="$infodir"
  eval htmldir="$htmldir"
  eval dvidir="$dvidir"
  eval pdfdir="$pdfdir"
  eval psdir="$psdir"
  eval libdir="$libdir"
  eval lispdir="$lispdir"
  eval localedir="$localedir"
  eval mandir="$mandir"
  eval pkgdatadir="$pkgdatadir"
  eval pkgincludedir="$pkgincludedir"
  eval pkglibdir="$pkglibdir"
  eval pkglibexecdir="$pkglibexecdir"

  dnl Transform the final values.
  gl_BUILD_TO_HOST([prefix])
  gl_BUILD_TO_HOST([exec_prefix])
  gl_BUILD_TO_HOST([bindir])
  gl_BUILD_TO_HOST([sbindir])
  gl_BUILD_TO_HOST([libexecdir])
  gl_BUILD_TO_HOST([datarootdir])
  gl_BUILD_TO_HOST([datadir])
  gl_BUILD_TO_HOST([sysconfdir])
  gl_BUILD_TO_HOST([sharedstatedir])
  gl_BUILD_TO_HOST([localstatedir])
  gl_BUILD_TO_HOST([runstatedir])
  gl_BUILD_TO_HOST([includedir])
  gl_BUILD_TO_HOST([oldincludedir])
  gl_BUILD_TO_HOST([docdir])
  gl_BUILD_TO_HOST([infodir])
  gl_BUILD_TO_HOST([htmldir])
  gl_BUILD_TO_HOST([dvidir])
  gl_BUILD_TO_HOST([pdfdir])
  gl_BUILD_TO_HOST([psdir])
  gl_BUILD_TO_HOST([libdir])
  gl_BUILD_TO_HOST([lispdir])
  gl_BUILD_TO_HOST([localedir])
  gl_BUILD_TO_HOST([mandir])
  gl_BUILD_TO_HOST([pkgdatadir])
  gl_BUILD_TO_HOST([pkgincludedir])
  gl_BUILD_TO_HOST([pkglibdir])
  gl_BUILD_TO_HOST([pkglibexecdir])

  dnl Restore the values.
  pkglibexecdir="${gl_save_pkglibexecdir}"
  pkglibdir="${gl_save_pkglibdir}"
  pkgincludedir="${gl_save_pkgincludedir}"
  pkgdatadir="${gl_save_pkgdatadir}"
  mandir="${gl_save_mandir}"
  localedir="${gl_save_localedir}"
  lispdir="${gl_save_lispdir}"
  libdir="${gl_save_libdir}"
  psdir="${gl_save_psdir}"
  pdfdir="${gl_save_pdfdir}"
  dvidir="${gl_save_dvidir}"
  htmldir="${gl_save_htmldir}"
  infodir="${gl_save_infodir}"
  docdir="${gl_save_docdir}"
  oldincludedir="${gl_save_oldincludedir}"
  includedir="${gl_save_includedir}"
  runstatedir="${gl_save_runstatedir}"
  localstatedir="${gl_save_localstatedir}"
  sharedstatedir="${gl_save_sharedstatedir}"
  sysconfdir="${gl_save_sysconfdir}"
  datadir="${gl_save_datadir}"
  datarootdir="${gl_save_datarootdir}"
  libexecdir="${gl_save_libexecdir}"
  sbindir="${gl_save_sbindir}"
  bindir="${gl_save_bindir}"
  exec_prefix="${gl_save_exec_prefix}"
  prefix="${gl_save_prefix}"
])
