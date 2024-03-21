# relocatable.m4 serial 25
dnl Copyright (C) 2003, 2005-2007, 2009-2023 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Bruno Haible.

dnl gl_RELOCATABLE([RELOCWRAPPER-DIR])
dnl ----------------------------------------------------------
dnl Support for relocatable programs.
dnl Supply RELOCWRAPPER-DIR as the directory where relocwrapper.c may be found.
AC_DEFUN([gl_RELOCATABLE],
[
  AC_REQUIRE([gl_RELOCATABLE_BODY])
  gl_RELOCATABLE_LIBRARY
  : ${RELOCATABLE_CONFIG_H_DIR='$(top_builddir)'}
  RELOCATABLE_SRC_DIR="\$(top_srcdir)/$gl_source_base"
  RELOCATABLE_BUILD_DIR="\$(top_builddir)/$gl_source_base"
])
dnl The guts of gl_RELOCATABLE. Needs to be expanded only once.
AC_DEFUN([gl_RELOCATABLE_BODY],
[
  AC_REQUIRE([AC_PROG_INSTALL])

  dnl This AC_BEFORE invocation leads to unjustified autoconf warnings
  dnl when gl_RELOCATABLE_BODY is invoked more than once.
  dnl
  dnl We need this AC_BEFORE because AC_PROG_INSTALL is documented to
  dnl overwrite earlier settings of INSTALL and INSTALL_PROGRAM (even
  dnl though in autoconf-2.52..2.60 it doesn't do so), but we want this
  dnl macro's setting of INSTALL_PROGRAM to persist.
  dnl Arghh: AC_BEFORE does not work in this setting :-(
  dnl AC_BEFORE([AC_PROG_INSTALL],[gl_RELOCATABLE_BODY])
  dnl
  dnl LT_INIT sets LIBTOOL, but we want this macro's setting of LIBTOOL to
  dnl persist.
  dnl Arghh: AC_BEFORE does not work in this setting :-(
  dnl AC_BEFORE([LT_INIT],[gl_RELOCATABLE_BODY])

  AC_REQUIRE([AC_LIB_LIBPATH])
  AC_REQUIRE([gl_RELOCATABLE_LIBRARY_BODY])
  AC_REQUIRE([AC_CANONICAL_HOST])
  is_noop=no
  use_elf_origin_trick=no
  use_macos_tools=no
  use_wrapper=no
  if test $RELOCATABLE = yes; then
    # --enable-relocatable implies --disable-rpath
    enable_rpath=no
    AC_CHECK_HEADERS([mach-o/dyld.h])
    AC_CHECK_FUNCS([_NSGetExecutablePath])
    case "$host_os" in
      mingw* | windows*) is_noop=yes ;;
      # For the platforms that support $ORIGIN, see
      # <https://lekensteyn.nl/rpath.html>.
      # glibc systems, Linux with musl libc: yes. Android: no.
      linux*-android*) ;;
      linux* | kfreebsd*) use_elf_origin_trick=yes ;;
      # Hurd: <https://lists.gnu.org/r/bug-hurd/2019-02/msg00049.html>
      # only after the glibc commit from 2018-01-08
      # <https://sourceware.org/git/?p=glibc.git;a=commitdiff;h=311ba8dc4416467947eff2ab327854f124226309>
      gnu*)
        # Test for a glibc version >= 2.27.
        AC_CHECK_FUNCS([copy_file_range])
        if test $ac_cv_func_copy_file_range = yes; then
          use_elf_origin_trick=yes
        fi
        ;;
changequote(,)dnl
      # FreeBSD >= 7.3, DragonFly >= 3.0, MidnightBSD >= 1.1: yes.
      freebsd | freebsd[1-7] | freebsd[1-6].* | freebsd7.[0-2]) ;;
      dragonfly | dragonfly[1-2] | dragonfly[1-2].*) ;;
      midnightbsd | midnightbsd0* | midnightbsd1.0*) ;;
      freebsd* | dragonfly* | midnightbsd*) use_elf_origin_trick=yes ;;
      # NetBSD >= 8.0: yes.
      netbsd | netbsd[1-7] | netbsd[1-7].*) ;;
      netbsdelf | netbsdelf[1-7] | netbsdelf[1-7].*) ;;
      netbsd*) use_elf_origin_trick=yes ;;
      # OpenBSD >= 5.4: yes.
      openbsd | openbsd[1-5] | openbsd[1-4].* | openbsd5.[0-3]) ;;
      openbsd*) use_elf_origin_trick=yes ;;
      # Solaris >= 10: yes.
      solaris | solaris2.[1-9] | solaris2.[1-9].*) ;;
      solaris*) use_elf_origin_trick=yes ;;
      # Haiku: yes.
      haiku*) use_elf_origin_trick=yes ;;
      # On Mac OS X 10.4 or newer, use Mac OS X tools. See
      # <https://wincent.com/wiki/@executable_path,_@load_path_and_@rpath>.
      darwin | darwin[1-7].*) ;;
      darwin*) use_macos_tools=yes ;;
changequote([,])dnl
    esac
    if test $is_noop = yes; then
      RELOCATABLE_LDFLAGS=:
      AC_SUBST([RELOCATABLE_LDFLAGS])
    else
      if test $use_elf_origin_trick = yes || test $use_macos_tools = yes; then
        dnl Use the dynamic linker's support for relocatable programs.
        case "$ac_aux_dir" in
          /*) reloc_ldflags="$ac_aux_dir/reloc-ldflags" ;;
          *) reloc_ldflags="\$(top_builddir)/$ac_aux_dir/reloc-ldflags" ;;
        esac
        RELOCATABLE_LDFLAGS="\"$reloc_ldflags\" \"\$(host)\" \"\$(RELOCATABLE_LIBRARY_PATH)\""
        AC_SUBST([RELOCATABLE_LDFLAGS])
        if test $use_macos_tools = yes; then
          dnl Use a libtool wrapper that uses Mac OS X tools.
          case "$ac_aux_dir" in
            /*) LIBTOOL="${CONFIG_SHELL-$SHELL} $ac_aux_dir/libtool-reloc $LIBTOOL" ;;
            *) LIBTOOL="${CONFIG_SHELL-$SHELL} \$(top_builddir)/$ac_aux_dir/libtool-reloc $LIBTOOL" ;;
          esac
        fi
      else
        use_wrapper=yes
        dnl Unfortunately we cannot define INSTALL_PROGRAM to a command
        dnl consisting of more than one word - libtool doesn't support this.
        dnl So we abuse the INSTALL_PROGRAM_ENV hook, originally meant for the
        dnl 'install-strip' target.
        INSTALL_PROGRAM_ENV="RELOC_LIBRARY_PATH_VAR=\"$shlibpath_var\" RELOC_LIBRARY_PATH_VALUE=\"\$(RELOCATABLE_LIBRARY_PATH)\" RELOC_PREFIX=\"\$(prefix)\" RELOC_DESTDIR=\"\$(DESTDIR)\" RELOC_COMPILE_COMMAND=\"\$(CC) \$(CPPFLAGS) \$(CFLAGS) \$(LDFLAGS)\" RELOC_SRCDIR=\"\$(RELOCATABLE_SRC_DIR)\" RELOC_BUILDDIR=\"\$(RELOCATABLE_BUILD_DIR)\" RELOC_CONFIG_H_DIR=\"\$(RELOCATABLE_CONFIG_H_DIR)\" RELOC_EXEEXT=\"\$(EXEEXT)\" RELOC_STRIP_PROG=\"\$(RELOCATABLE_STRIP)\" RELOC_INSTALL_PROG=\"$INSTALL_PROGRAM\""
        AC_SUBST([INSTALL_PROGRAM_ENV])
        case "$ac_aux_dir" in
          /*) INSTALL_PROGRAM="$ac_aux_dir/install-reloc" ;;
          *) INSTALL_PROGRAM="\$(top_builddir)/$ac_aux_dir/install-reloc" ;;
        esac
      fi
    fi
  fi
  AM_CONDITIONAL([RELOCATABLE_VIA_LD],
    [test $is_noop = yes || test $use_elf_origin_trick = yes || test $use_macos_tools = yes])
  AM_CONDITIONAL([RELOCATABLE_VIA_WRAPPER], [test $use_wrapper = yes])

  dnl RELOCATABLE_LIBRARY_PATH can be set in configure.ac. Default is empty.
  AC_SUBST([RELOCATABLE_LIBRARY_PATH])

  AC_SUBST([RELOCATABLE_CONFIG_H_DIR])
  AC_SUBST([RELOCATABLE_SRC_DIR])
  AC_SUBST([RELOCATABLE_BUILD_DIR])

  dnl Ensure RELOCATABLE_STRIP is defined in Makefiles (at least those
  dnl generated by automake), with value ':'.
  RELOCATABLE_STRIP=':'
  AC_SUBST([RELOCATABLE_STRIP])
])

dnl Determine the platform dependent parameters needed to use relocatability:
dnl shlibpath_var.
AC_DEFUN([AC_LIB_LIBPATH],
[
  AC_REQUIRE([AC_LIB_PROG_LD])            dnl we use $LD
  AC_REQUIRE([AC_CANONICAL_HOST])         dnl we use $host
  AC_REQUIRE([AC_CONFIG_AUX_DIR_DEFAULT]) dnl we use $ac_aux_dir
  AC_CACHE_CHECK([for shared library path variable], [acl_cv_libpath], [
    LD="$LD" \
    ${CONFIG_SHELL-/bin/sh} "$ac_aux_dir/config.libpath" "$host" > conftest.sh
    . ./conftest.sh
    rm -f ./conftest.sh
    acl_cv_libpath=${acl_cv_shlibpath_var:-none}
  ])
  shlibpath_var="$acl_cv_shlibpath_var"
])
