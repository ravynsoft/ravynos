dnl
dnl  fontconfig/configure.in
dnl
dnl  Copyright © 2003 Keith Packard
dnl
dnl  Permission to use, copy, modify, distribute, and sell this software and its
dnl  documentation for any purpose is hereby granted without fee, provided that
dnl  the above copyright notice appear in all copies and that both that
dnl  copyright notice and this permission notice appear in supporting
dnl  documentation, and that the name of the author(s) not be used in
dnl  advertising or publicity pertaining to distribution of the software without
dnl  specific, written prior permission.  The authors make no
dnl  representations about the suitability of this software for any purpose.  It
dnl  is provided "as is" without express or implied warranty.
dnl
dnl  THE AUTHOR(S) DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
dnl  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
dnl  EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR
dnl  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
dnl  DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
dnl  TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
dnl  PERFORMANCE OF THIS SOFTWARE.
dnl
dnl Process this file with autoconf to create configure.

AC_PREREQ(2.61)

dnl ==========================================================================
dnl                               Versioning
dnl ==========================================================================

dnl This is the package version number, not the shared library
dnl version.  This same version number must appear in fontconfig/fontconfig.h
dnl Yes, it is a pain to synchronize version numbers.  Unfortunately, it's
dnl not possible to extract the version number here from fontconfig.h
AC_INIT([fontconfig], [2.14.2], [https://gitlab.freedesktop.org/fontconfig/fontconfig/issues/new])
AM_INIT_AUTOMAKE([1.11 parallel-tests dist-xz])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

dnl ==========================================================================

AC_CONFIG_HEADERS(config.h)
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG
m4_ifdef([PKG_INSTALLDIR], [PKG_INSTALLDIR], AC_SUBST([pkgconfigdir], ${libdir}/pkgconfig))

AM_MISSING_PROG([GIT], [git])
AM_MISSING_PROG([GPERF], [gperf])
AM_PATH_PYTHON([3])

AC_MSG_CHECKING([for RM macro])
_predefined_rm=`make -p -f /dev/null 2>/dev/null|grep '^RM ='|sed -e 's/^RM = //'`
if test "x$_predefined_rm" = "x"; then
	AC_MSG_RESULT([no predefined RM])
	AC_CHECK_PROG(RM, rm, [rm -f])
else
	AC_MSG_RESULT($_predefined_rm)
fi

dnl Initialize libtool
LT_PREREQ([2.2])
LT_INIT([disable-static win32-dll])

dnl libtool versioning

dnl bump revision when fixing bugs
dnl bump current and age, reset revision to zero when adding APIs
dnl bump current, leave age, reset revision to zero when changing/removing APIS
LIBT_CURRENT=13
LIBT_REVISION=0
AC_SUBST(LIBT_CURRENT)
AC_SUBST(LIBT_REVISION)
LIBT_AGE=12

LIBT_VERSION_INFO="$LIBT_CURRENT:$LIBT_REVISION:$LIBT_AGE"
AC_SUBST(LIBT_VERSION_INFO)

LIBT_CURRENT_MINUS_AGE=`expr $LIBT_CURRENT - $LIBT_AGE`
AC_SUBST(LIBT_CURRENT_MINUS_AGE)

PKGCONFIG_REQUIRES=
PKGCONFIG_REQUIRES_PRIVATELY=

dnl ==========================================================================
AC_CANONICAL_HOST
os_win32=no
os_darwin=no
case "${host_os}" in
	cygwin*|mingw*)
		os_win32=yes
		;;
	darwin*)
		os_darwin=yes
		;;
esac
AM_CONDITIONAL(OS_WIN32, test "$os_win32" = "yes")
AM_CONDITIONAL(OS_DARWIN, test "$os_darwin" = "yes")

dnl ==========================================================================
dnl gettext stuff
dnl ==========================================================================
GETTEXT_PACKAGE=$PACKAGE
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext package])

AM_GNU_GETTEXT_VERSION([0.19.7])
AM_GNU_GETTEXT([external])

dnl ==========================================================================

if test "$os_win32" = "yes"; then
  AC_CHECK_PROG(ms_librarian, lib.exe, yes, no)
fi
AM_CONDITIONAL(MS_LIB_AVAILABLE, test x$ms_librarian = xyes)

AC_CHECK_DECL([__SUNPRO_C], [SUNCC="yes"], [SUNCC="no"])
WARN_CFLAGS=""
WARNING_CPP_DIRECTIVE="no"
if test "x$GCC" = "xyes"; then
	WARN_CFLAGS="-Wall -Wpointer-arith -Wstrict-prototypes \
	-Wmissing-prototypes -Wmissing-declarations \
	-Wnested-externs -fno-strict-aliasing"
	WARNING_CPP_DIRECTIVE="yes"
elif test "x$SUNCC" = "xyes"; then
	WARN_CFLAGS="-v -fd"
	WARNING_CPP_DIRECTIVE="yes"
fi
if test "x$WARNING_CPP_DIRECTIVE" = "xyes"; then
	AC_DEFINE_UNQUOTED(HAVE_WARNING_CPP_DIRECTIVE,1,
	[Can use #warning in C files])
fi
AC_SUBST(WARN_CFLAGS)


dnl ==========================================================================

AX_CC_FOR_BUILD()
AC_ARG_VAR(CC_FOR_BUILD, [build system C compiler])
AM_CONDITIONAL(CROSS_COMPILING, test $cross_compiling = yes)
AM_CONDITIONAL(ENABLE_SHARED, test "$enable_shared" = "yes")

dnl ==========================================================================

AC_ARG_WITH(arch,
	[AC_HELP_STRING([--with-arch=ARCH],
			[Force architecture to ARCH])],
	arch="$withval", arch=auto)

if test "x$arch" != xauto; then
	AC_DEFINE_UNQUOTED([FC_ARCHITECTURE], "$arch", [Architecture prefix to use for cache file names])
fi


dnl ==========================================================================

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([dirent.h fcntl.h stdlib.h string.h unistd.h sys/statvfs.h sys/vfs.h sys/statfs.h sys/param.h sys/mount.h])
AX_CREATE_STDINT_H([src/fcstdint.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_C_FLEXIBLE_ARRAY_MEMBER
AC_TYPE_PID_T

# Checks for library functions.
AC_FUNC_VPRINTF
AC_FUNC_MMAP
AC_CHECK_FUNCS([link mkstemp mkostemp _mktemp_s mkdtemp getopt getopt_long getprogname getexecname rand random lrand48 random_r rand_r readlink fstatvfs fstatfs lstat strerror strerror_r])

dnl AC_CHECK_FUNCS doesn't check for header files.
dnl posix_fadvise() may be not available in older libc.
AC_CHECK_SYMBOL([posix_fadvise], [fcntl.h], [fc_func_posix_fadvise=1], [fc_func_posix_fadvise=0])
AC_DEFINE_UNQUOTED([HAVE_POSIX_FADVISE], [$fc_func_posix_fadvise], [Define to 1 if you have the 'posix_fadvise' function.])

#
AC_CHECK_MEMBERS([struct stat.st_mtim],,, [#include <sys/stat.h>])

#
if test "x$ac_cv_func_fstatvfs" = "xyes"; then
	AC_CHECK_MEMBERS([struct statvfs.f_basetype, struct statvfs.f_fstypename],,,
		[#include <sys/statvfs.h>])
fi
if test "x$ac_cv_func_fstatfs" = "xyes"; then
	AC_CHECK_MEMBERS([struct statfs.f_flags, struct statfs.f_fstypename],,, [
#ifdef HAVE_SYS_VFS_H
#include <sys/vfs.h>
#endif
#ifdef HAVE_SYS_STATFS_H
#include <sys/statfs.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif])
fi
AC_CHECK_MEMBERS([struct dirent.d_type],,,
	[#include <dirent.h>])

# Check the argument type of the gperf hash/lookup function
AC_MSG_CHECKING([The type of len parameter of gperf hash/lookup function])
fc_gperf_test="$(echo 'foo' | gperf -L ANSI-C)"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
	#include <string.h>

	const char *in_word_set(register const char *, register size_t);
	$fc_gperf_test
	]])], [FC_GPERF_SIZE_T=size_t],
	[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
		#include <string.h>

		const char *in_word_set(register const char *, register unsigned int);
		$fc_gperf_test
	]])], [FC_GPERF_SIZE_T="unsigned int"],
	[AC_MSG_ERROR([Unable to determine the type of the len parameter of the gperf hash/lookup function])]
)])
AC_DEFINE_UNQUOTED(FC_GPERF_SIZE_T, $FC_GPERF_SIZE_T, [The type of len parameter of the gperf hash/lookup function])
AC_MSG_RESULT($FC_GPERF_SIZE_T)

#
# Checks for iconv
#
AC_ARG_ENABLE(iconv,
	[AC_HELP_STRING([--enable-iconv],
			[Use iconv to support non-Unicode SFNT name])],
	,enable_iconv=no)
AC_ARG_WITH(libiconv,
	[AC_HELP_STRING([--with-libiconv=DIR],
			[Use libiconv in DIR])],
	[if test "x$withval" = "xyes"; then
		libiconv_prefix=$prefix
	 else
		libiconv_prefix=$withval
	 fi],
	[libiconv_prefix=auto])
AC_ARG_WITH(libiconv-includes,
	[AC_HELP_STRING([--with-libiconv-includes=DIR],
			[Use libiconv includes in DIR])],
	[libiconv_includes=$withval],
	[libiconv_includes=auto])
AC_ARG_WITH(libiconv-lib,
	[AC_HELP_STRING([--with-libiconv-lib=DIR],
			[Use libiconv library in DIR])],
	[libiconv_lib=$withval],
	[libiconv_lib=auto])

# if no libiconv,libiconv-includes,libiconv-lib are specified,
# libc's iconv has a priority.
if test "$libiconv_includes" != "auto" -a -r ${libiconv_includes}/iconv.h; then
	libiconv_cflags="-I${libiconv_includes}"
elif test "$libiconv_prefix" != "auto" -a -r ${libiconv_prefix}/include/iconv.h; then
	libiconv_cflags="-I${libiconv_prefix}/include"
else
	libiconv_cflags=""
fi
libiconv_libs=""
if test "x$libiconv_cflags" != "x"; then
	if test "$libiconv_lib" != "auto" -a -d ${libiconv_lib}; then
		libiconv_libs="-L${libiconv_lib} -liconv"
	elif test "$libiconv_prefix" != "auto" -a -d ${libiconv_prefix}/lib; then
		libiconv_libs="-L${libiconv_prefix}/lib -liconv"
	else
		libiconv_libs="-liconv"
	fi
fi

use_iconv=0
if test "x$enable_iconv" != "xno"; then
	AC_MSG_CHECKING([for a usable iconv])
	if test "x$libiconv_cflags" != "x" -o "x$libiconv_libs" != "x"; then
		iconvsaved_CFLAGS="$CFLAGS"
		iconvsaved_LIBS="$LIBS"
		CFLAGS="$CFLAGS $libiconv_cflags"
		LIBS="$LIBS $libiconv_libs"

		AC_TRY_LINK([#include <iconv.h>],
			[iconv_open ("from", "to");],
			[iconv_type="libiconv"
			 use_iconv=1
			 ICONV_CFLAGS="$libiconv_cflags"
			 ICONV_LIBS="$libiconv_libs"
			 ],
			[use_iconv=0])

		CFLAGS="$iconvsaved_CFLAGS"
		LIBS="$iconvsaved_LIBS"
	fi
	if test "x$use_iconv" = "x0"; then
		AC_TRY_LINK([#include <iconv.h>],
			[iconv_open ("from", "to");],
			[iconv_type="libc"
			 use_iconv=1],
			[iconv_type="not found"
			 use_iconv=0])
	fi

	AC_MSG_RESULT([$iconv_type])
	AC_SUBST(ICONV_CFLAGS)
	AC_SUBST(ICONV_LIBS)
fi
AC_DEFINE_UNQUOTED(USE_ICONV,$use_iconv,[Use iconv.])
#
# Checks for FreeType
#
dnl See http://git.savannah.gnu.org/cgit/freetype/freetype2.git/tree/docs/VERSIONS.TXT for versioning in freetype
PKG_CHECK_MODULES(FREETYPE, freetype2 >= 21.0.15)
PKGCONFIG_REQUIRES="$PKGCONFIG_REQUIRES freetype2 >= 21.0.15"

AC_SUBST(FREETYPE_LIBS)
AC_SUBST(FREETYPE_CFLAGS)

fontconfig_save_libs="$LIBS"
fontconfig_save_cflags="$CFLAGS"
LIBS="$LIBS $FREETYPE_LIBS"
CFLAGS="$CFLAGS $FREETYPE_CFLAGS"
AC_CHECK_FUNCS(FT_Get_BDF_Property FT_Get_PS_Font_Info FT_Has_PS_Glyph_Names FT_Get_X11_Font_Format FT_Done_MM_Var)

AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
	#include <ft2build.h>
	#include FT_CONFIG_OPTIONS_H
	#ifndef PCF_CONFIG_OPTION_LONG_FAMILY_NAMES
	#  error "No pcf long family names support"
	#endif
	]])], [have_pcf_long_family_names=yes], [have_pcf_long_family_names=no])
AM_CONDITIONAL(FREETYPE_PCF_LONG_FAMILY_NAMES, test "x$have_pcf_long_family_names" = xyes)

LIBS="$fontconfig_save_libs"
CFLAGS="$fontconfig_save_cflags"

#
# Check expat configuration
#
AC_ARG_WITH(expat,
	[AC_HELP_STRING([--with-expat=DIR],
			[Use Expat in DIR])],
	[expat_prefix=$withval],
	[expat_prefix=auto])
AC_ARG_WITH(expat-includes,
	[AC_HELP_STRING([--with-expat-includes=DIR],
			[Use Expat includes in DIR])],
	[expat_includes=$withval],
	[expat_includes=auto])
AC_ARG_WITH(expat-lib,
	[AC_HELP_STRING([--with-expat-lib=DIR])],
	[expat_lib=$withval],
	[expat_lib=auto])

if test "$enable_libxml2" != "yes"; then
	use_pkgconfig_for_expat=yes
	if test "$expat_prefix" = "auto" -a "$expat_includes" = "auto" -a "$expat_lib" = "auto"; then
		PKG_CHECK_MODULES(EXPAT, expat,,use_pkgconfig_for_expat=no)
	else
		use_pkgconfig_for_expat=no
	fi
	if test "x$use_pkgconfig_for_expat" = "xno"; then
		if test "$expat_includes" != "auto" -a -r ${expat_includes}/expat.h; then
			EXPAT_CFLAGS="-I${expat_includes}"
		elif test "$expat_prefix" != "auto" -a -r ${expat_prefix}/include/expat.h; then
			EXPAT_CFLAGS="-I${expat_prefix}/include"
		else
			EXPAT_CFLAGS=""
		fi
		if test "$expat_lib" != "auto"; then
			EXPAT_LIBS="-L${expat_lib} -lexpat"
		elif test "$expat_prefix" != "auto"; then
			EXPAT_LIBS="-L${expat_prefix}/lib -lexpat"
		else
			EXPAT_LIBS="-lexpat"
		fi
		PKG_EXPAT_CFLAGS=$EXPAT_CFLAGS
		PKG_EXPAT_LIBS=$EXPAT_LIBS
	else
		PKGCONFIG_REQUIRES_PRIVATELY="$PKGCONFIG_REQUIRES_PRIVATELY expat"
		PKG_EXPAT_CFLAGS=
		PKG_EXPAT_LIBS=
	fi

	expatsaved_CPPFLAGS="$CPPFLAGS"
	expatsaved_LIBS="$LIBS"
	CPPFLAGS="$CPPFLAGS $EXPAT_CFLAGS"
	LIBS="$LIBS $EXPAT_LIBS"

	AC_CHECK_HEADER(expat.h)
	if test "$ac_cv_header_expat_h" = "no"; then
		AC_CHECK_HEADER(xmlparse.h)
		if test "$ac_cv_header_xmlparse_h" = "yes"; then
			HAVE_XMLPARSE_H=1
			AC_SUBST(HAVE_XMLPARSE_H)
			AC_DEFINE_UNQUOTED(HAVE_XMLPARSE_H,$HAVE_XMLPARSE_H,
				[Use xmlparse.h instead of expat.h])
		else
			AC_MSG_ERROR([
*** expat is required. or try to use --enable-libxml2])
		fi
	fi
	AC_CHECK_FUNCS(XML_SetDoctypeDeclHandler)
	if test "$ac_cv_func_XML_SetDoctypeDeclHandler" = "no"; then
		AC_MSG_ERROR([
*** expat is required. or try to use --enable-libxml2])
	fi
	CPPFLAGS="$expatsaved_CPPFLAGS"
	LIBS="$expatsaved_LIBS"

	AC_SUBST(EXPAT_CFLAGS)
	AC_SUBST(EXPAT_LIBS)
	AC_SUBST(PKG_EXPAT_CFLAGS)
	AC_SUBST(PKG_EXPAT_LIBS)
fi

#
# Check libxml2 configuration
#
AC_ARG_ENABLE(libxml2,
	[AC_HELP_STRING([--enable-libxml2],
			[Use libxml2 instead of Expat])])

if test "$enable_libxml2" = "yes"; then
    PKG_CHECK_MODULES([LIBXML2], [libxml-2.0 >= 2.6])
    PKGCONFIG_REQUIRES_PRIVATELY="$PKGCONFIG_REQUIRES_PRIVATELY libxml-2.0 >= 2.6"
    AC_DEFINE_UNQUOTED(ENABLE_LIBXML2,1,[Use libxml2 instead of Expat])

    AC_SUBST(LIBXML2_CFLAGS)
    AC_SUBST(LIBXML2_LIBS)

    fc_saved_CFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS $LIBXML2_CFLAGS"
    AC_MSG_CHECKING([SAX1 support in libxml2])
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
	#include <libxml/xmlversion.h>
	#if !defined(LIBXML_SAX1_ENABLED)
	#  include "error: No SAX1 support in libxml2"
	#endif
	]])], [AC_MSG_RESULT([found])], [AC_MSG_ERROR([
*** SAX1 support in libxml2 is required. enable it or use expat instead.])])
    CFLAGS="$fc_saved_CFLAGS"
fi

#
# Check json-c
#
PKG_CHECK_MODULES([JSONC], [json-c], [use_jsonc=yes], [use_jsonc=no])

AM_CONDITIONAL(ENABLE_JSONC, test "x$use_jsonc" = "xyes")
AC_SUBST(JSONC_CFLAGS)
AC_SUBST(JSONC_LIBS)

#
# Set default sub-pixel rendering
#

AC_ARG_WITH(default-sub-pixel-rendering,
    [AC_HELP_STRING([--with-default-sub-pixel-rendering=NAME],
            [Enable your preferred sub-pixel rendering configuration (none/bgr/rgb/vbgr/vrgb) [default=none]])],
    preferred_sub_pixel_rendering="$withval", preferred_sub_pixel_rendering=none)

case "$preferred_sub_pixel_rendering" in
none|bgr|rgb|vbgr|vrgb)
    PREFERRED_SUB_PIXEL_RENDERING="$preferred_sub_pixel_rendering"
    AC_SUBST(PREFERRED_SUB_PIXEL_RENDERING)
    ;;
*)
    AC_MSG_ERROR([Invalid sub-pixel rendering. please choose one of none, bgr, rgb, vbgr, or vrgb])
    ;;
esac

#
# Set default hinting
#

AC_ARG_WITH(default-hinting,
	[AC_HELP_STRING([--with-default-hinting=NAME],
			[Enable your preferred hinting configuration (none/slight/medium/full) [default=slight]])],
	preferred_hinting="$withval", preferred_hinting=slight)

case "$preferred_hinting" in
none|slight|medium|full)
	PREFERRED_HINTING="$preferred_hinting"
	AC_SUBST(PREFERRED_HINTING)
	;;
*)
	AC_MSG_ERROR([Invalid hinting. please choose one of none, slight, medium, or full])
	;;
esac

#
# Set default font directory
#

AC_ARG_WITH(default-fonts,
	[AC_HELP_STRING([--with-default-fonts=DIR1,DIR2,...],
			[Use fonts from DIR1,DIR2,... when config is busted])],
	default_fonts="$withval", default_fonts=yes)

case "$default_fonts" in
yes)
	if test "$os_win32" = "yes"; then
		default_fonts="WINDOWSFONTDIR,WINDOWSUSERFONTDIR"
	elif test "$os_darwin" = "yes"; then
		default_fonts="/System/Library/Fonts,/Library/Fonts,~/Library/Fonts,/System/Library/Assets/com_apple_MobileAsset_Font3,/System/Library/Assets/com_apple_MobileAsset_Font4"
	else
		default_fonts="/usr/share/fonts"
	fi
	;;
esac

FC_DEFAULT_FONTS=""
if test x${default_fonts+set} = xset; then
	fc_IFS=$IFS
	IFS=","
	for p in $default_fonts; do
		if test x"$FC_DEFAULT_FONTS" != x; then
			FC_DEFAULT_FONTS="$FC_DEFAULT_FONTS "
		fi
		FC_DEFAULT_FONTS="$FC_DEFAULT_FONTS<dir>$p</dir>"
	done
	IFS=$fc_IFS
fi

AC_DEFINE_UNQUOTED(FC_DEFAULT_FONTS, "$FC_DEFAULT_FONTS",
			   [System font directory])

AC_SUBST(FC_DEFAULT_FONTS)

#
# Add more fonts if available.  By default, add only the directories
# with outline fonts; those with bitmaps can be added as desired in
# local.conf or ~/.fonts.conf
#
AC_ARG_WITH(add-fonts,
	[AC_HELP_STRING([--with-add-fonts=DIR1,DIR2,...],
			[Find additional fonts in DIR1,DIR2,... ])],
	add_fonts="$withval", add_fonts=yes)

case "$add_fonts" in
yes)
	FC_ADD_FONTS=""
	for dir in /usr/X11R6/lib/X11 /usr/X11/lib/X11 /usr/lib/X11; do
		case x"$FC_ADD_FONTS" in
		x)
			sub="$dir/fonts"
			if test -d "$sub"; then
				case x$FC_ADD_FONTS in
				x)
					FC_ADD_FONTS="$sub"
					;;
				*)
					FC_ADD_FONTS="$FC_ADD_FONTS,$sub"
					;;
				esac
			fi
			;;
		esac
	done
	AC_DEFINE_UNQUOTED(FC_ADD_FONTS,"$add_fonts",[Additional font directories])
	;;
no)
	FC_ADD_FONTS=""
	;;
*)
	FC_ADD_FONTS="$add_fonts"
	AC_DEFINE_UNQUOTED(FC_ADD_FONTS,"$add_fonts",[Additional font directories])
	;;
esac

AC_SUBST(FC_ADD_FONTS)

FC_FONTPATH=""

case "$FC_ADD_FONTS" in
"")
	;;
*)
	FC_FONTPATH=`echo $FC_ADD_FONTS |
			sed -e 's/^/<dir>/' -e 's/$/<\/dir>/' -e 's/,/<\/dir> <dir>/g'`
	;;
esac

AC_SUBST(FC_FONTPATH)

#
# Set default cache directory path
#
AC_ARG_WITH(cache-dir,
	[AC_HELP_STRING([--with-cache-dir=DIR],
			[Use DIR to store cache files [default=LOCALSTATEDIR/cache/fontconfig]])],
	fc_cachedir="$withval", fc_cachedir=yes)

case $fc_cachedir in
no|yes)
	if test "$os_win32" = "yes"; then
		fc_cachedir="LOCAL_APPDATA_FONTCONFIG_CACHE"
	else
		fc_cachedir='${localstatedir}/cache/${PACKAGE}'
	fi
	;;
*)
	;;
esac
AC_SUBST(fc_cachedir)
FC_CACHEDIR=${fc_cachedir}
AC_SUBST(FC_CACHEDIR)

FC_FONTDATE=`LC_ALL=C date`

AC_SUBST(FC_FONTDATE)

#
# Set configuration paths
#

AC_ARG_WITH(templatedir,
	[AC_HELP_STRING([--with-templatedir=DIR],
			[Use DIR to store the configuration template files [default=DATADIR/fontconfig/conf.avail]])],
	[templatedir="$withval"],
	[templatedir=yes])
AC_ARG_WITH(baseconfigdir,
	[AC_HELP_STRING([--with-baseconfigdir=DIR],
			[Use DIR to store the base configuration files [default=SYSCONFDIR/fonts]])],
	[baseconfigdir="$withval"],
	[baseconfigdir=yes])
AC_ARG_WITH(configdir,
	[AC_HELP_STRING([--with-configdir=DIR],
			[Use DIR to store active configuration files [default=BASECONFIGDIR/conf.d]])],
	[configdir="$withval"],
	[configdir=yes])
AC_ARG_WITH(xmldir,
	[AC_HELP_STRING([--with-xmldir=DIR],
			[Use DIR to store XML schema files [default=DATADIR/xml/fontconfig]])],
	[xmldir="$withval"],
	[xmldir=yes])

case "$templatedir" in
no|yes)
	templatedir='${datadir}'/fontconfig/conf.avail
	;;
*)
	;;
esac
case "$baseconfigdir" in
no|yes)
	baseconfigdir='${sysconfdir}'/fonts
	;;
*)
	;;
esac
case "$configdir" in
no|yes)
	configdir='${BASECONFIGDIR}'/conf.d
	;;
*)
	;;
esac
case "$xmldir" in
no|yes)
	xmldir='${datadir}'/xml/fontconfig
	;;
*)
	;;
esac

TEMPLATEDIR=${templatedir}
BASECONFIGDIR=${baseconfigdir}
CONFIGDIR=${configdir}
XMLDIR=${xmldir}
AC_SUBST(TEMPLATEDIR)
AC_SUBST(BASECONFIGDIR)
AC_SUBST(CONFIGDIR)
AC_SUBST(XMLDIR)


dnl ===========================================================================

#
# Thread-safety primitives
#

AC_CACHE_CHECK([stdatomic.h atomic primitives], fc_cv_have_stdatomic_atomic_primitives, [
	fc_cv_have_stdatomic_atomic_primitives=false
	AC_TRY_LINK([
		#include <stdatomic.h>

		void memory_barrier (void) { atomic_thread_fence (memory_order_acq_rel); }
		int atomic_add (atomic_int *i) { return atomic_fetch_add_explicit (i, 1, memory_order_relaxed); }
		int mutex_trylock (atomic_flag *m) { return atomic_flag_test_and_set_explicit (m, memory_order_acquire); }
		void mutex_unlock (atomic_flag *m) { atomic_flag_clear_explicit (m, memory_order_release); }
		], [], fc_cv_have_stdatomic_atomic_primitives=true
	)
])
if $fc_cv_have_stdatomic_atomic_primitives; then
	AC_DEFINE(HAVE_STDATOMIC_PRIMITIVES, 1, [Have C99 stdatomic atomic primitives])
fi

AC_CACHE_CHECK([for Intel atomic primitives], fc_cv_have_intel_atomic_primitives, [
	fc_cv_have_intel_atomic_primitives=false
	AC_TRY_LINK([
		void memory_barrier (void) { __sync_synchronize (); }
		int atomic_add (int *i) { return __sync_fetch_and_add (i, 1); }
		int mutex_trylock (int *m) { return __sync_lock_test_and_set (m, 1); }
		void mutex_unlock (int *m) { __sync_lock_release (m); }
		], [], fc_cv_have_intel_atomic_primitives=true
	)
])
if $fc_cv_have_intel_atomic_primitives; then
	AC_DEFINE(HAVE_INTEL_ATOMIC_PRIMITIVES, 1, [Have Intel __sync_* atomic primitives])
fi

AC_CACHE_CHECK([for Solaris atomic operations], fc_cv_have_solaris_atomic_ops, [
	fc_cv_have_solaris_atomic_ops=false
	AC_TRY_LINK([
		#include <atomic.h>
		/* This requires Solaris Studio 12.2 or newer: */
		#include <mbarrier.h>
		void memory_barrier (void) { __machine_rw_barrier (); }
		int atomic_add (volatile unsigned *i) { return atomic_add_int_nv (i, 1); }
		void *atomic_ptr_cmpxchg (volatile void **target, void *cmp, void *newval) { return atomic_cas_ptr (target, cmp, newval); }
		], [], fc_cv_have_solaris_atomic_ops=true
	)
])
if $fc_cv_have_solaris_atomic_ops; then
	AC_DEFINE(HAVE_SOLARIS_ATOMIC_OPS, 1, [Have Solaris __machine_*_barrier and atomic_* operations])
fi

if test "$os_win32" = no && ! $have_pthread; then
	AC_CHECK_HEADERS(sched.h)
	AC_SEARCH_LIBS(sched_yield,rt,AC_DEFINE(HAVE_SCHED_YIELD, 1, [Have sched_yield]))
fi

have_pthread=false
if test "$os_win32" = no; then
	AX_PTHREAD([have_pthread=true])
fi
if $have_pthread; then
	LIBS="$PTHREAD_LIBS $LIBS"
	CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
	CC="$PTHREAD_CC"
	AC_DEFINE(HAVE_PTHREAD, 1, [Have POSIX threads])
fi
AM_CONDITIONAL(HAVE_PTHREAD, $have_pthread)


dnl ===========================================================================

#
# Let people not build/install docs if they don't have docbook
#

AC_ARG_ENABLE(docbook,
	[AS_HELP_STRING([--disable-docbook],
		[Disable building docs with docbook2html (default: no)])],,)

if test x$enable_docbook != xno; then
	AC_CHECK_PROG(HASDOCBOOK, docbook2html, yes, no)
fi

AM_CONDITIONAL(USEDOCBOOK, test "x$HASDOCBOOK" = xyes)

default_docs="yes"
#
# Check if docs exist or can be created
#
if test x$HASDOCBOOK = xno; then
	if test -f $srcdir/doc/fonts-conf.5; then
		:
	else
		default_docs="no"
	fi
fi

AC_ARG_ENABLE(docs,
	[AC_HELP_STRING([--disable-docs],
			[Don't build and install documentation])],
	,
	enable_docs=$default_docs)

AM_CONDITIONAL(ENABLE_DOCS, test "x$enable_docs" = xyes)

if test "x$enable_docs" = xyes; then
	tmp=funcs.$$
	cat $srcdir/doc/*.fncs | awk '
	/^@TITLE@/	{ if (!done) { printf ("%s\n", $2); done = 1; } }
	/^@FUNC@/	{ if (!done) { printf ("%s\n", $2); done = 1; } }
	/^@@/		{ done = 0; }' > $tmp
	DOCMAN3=`cat $tmp | awk '{ printf ("%s.3 ", $1); }'`
	echo DOCMAN3 $DOCMAN3
	rm -f $tmp
else
	DOCMAN3=""
fi
AC_SUBST(DOCMAN3)

dnl ===========================================================================
default_cache_build="yes"
if test $cross_compiling = "yes"; then
	default_cache_build="no"
fi
AC_ARG_ENABLE(cache-build,
	[AC_HELP_STRING([--disable-cache-build],
			[Don't run fc-cache during the build])],
	,
	enable_cache_build=$default_cache_build)

AM_CONDITIONAL(ENABLE_CACHE_BUILD, test "x$enable_cache_build" = xyes)


dnl Figure out what cache format suffix to use for this architecture
AC_C_BIGENDIAN
AC_CHECK_SIZEOF([void *])
AC_CHECK_ALIGNOF([double])
AC_CHECK_ALIGNOF([void *])

dnl include the header file for workaround of miscalculating size on autoconf
dnl particularly for fat binaries
AH_BOTTOM([#include "config-fixups.h"])

dnl
dnl
AC_SUBST(PKGCONFIG_REQUIRES)
AC_SUBST(PKGCONFIG_REQUIRES_PRIVATELY)

dnl
AC_CONFIG_FILES([
Makefile
fontconfig/Makefile
fc-lang/Makefile
fc-case/Makefile
src/Makefile
conf.d/Makefile
fc-cache/Makefile
fc-cat/Makefile
fc-conflist/Makefile
fc-list/Makefile
fc-match/Makefile
fc-pattern/Makefile
fc-query/Makefile
fc-scan/Makefile
fc-validate/Makefile
doc/Makefile
doc/version.sgml
its/Makefile
po/Makefile.in
po-conf/Makefile.in
test/Makefile
fontconfig.pc
fontconfig-zip
])
AC_OUTPUT
