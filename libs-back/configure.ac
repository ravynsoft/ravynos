#  configure.ac for GNUstep GUI Backend
#  Process this file with autoconf to produce a configure script.
#
#  Copyright (C) 1996-2015 Free Software Foundation, Inc.
#
#  Author:  Adam Fedor <fedor@gnu.org>
#
#  This file is part of the GNUstep Backend.
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 3 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; see the file COPYING.LIB.
#  If not, see <http://www.gnu.org/licenses/> or write to the 
#  Free Software Foundation, 51 Franklin Street, Fifth Floor, 
#  Boston, MA 02110-1301, USA.

builtin(include, pkg.m4)dnl

AC_INIT
AC_PREREQ(2.57)
AC_CONFIG_SRCDIR([back.make.in])
AC_CONFIG_HEADER(config.h)

# If GNUSTEP_MAKEFILES is undefined, try to use gnustep-config to determine it.
if test -z "$GNUSTEP_MAKEFILES"; then
  GNUSTEP_MAKEFILES=`gnustep-config --variable=GNUSTEP_MAKEFILES 2>&5`
fi

if test -z "$GNUSTEP_MAKEFILES"; then
  AC_MSG_ERROR([You must have the gnustep-make package installed and set up the GNUSTEP_MAKEFILES environment variable to contain the path to the makefiles directory before configuring!])
fi

#--------------------------------------------------------------------
# Use config.guess, config.sub and install-sh provided by gnustep-make
#--------------------------------------------------------------------
AC_CONFIG_AUX_DIR($GNUSTEP_MAKEFILES)

#--------------------------------------------------------------------
# Determine the host, build, and target systems
#--------------------------------------------------------------------
AC_CANONICAL_TARGET([])

#--------------------------------------------------------------------
# Find the compiler
#--------------------------------------------------------------------
MAKECC=`gnustep-config --variable=CC`
MAKECPP=`gnustep-config --variable=CPP`
MAKECXX=`gnustep-config --variable=CXX`
if test "$CC" = ""; then
  CC=$MAKECC
else
  if test "$CC" != "$MAKECC"; then
    AC_MSG_WARN([You are running configure with the compiler ($CC) set to a different value from that used by gnustep-make ($MAKECC).  To a
void conflicts/problems, reconfigure/reinstall gnustep-make to use $CC or run the gnustep-base configure again with your CC environment var
iable set to $MAKECC])
  fi
fi
if test "$CPP" = ""; then
  CPP=$MAKECPP
else
  if test "$CPP" != "$MAKECPP"; then
    AC_MSG_WARN([You are running configure with the preprocessor ($CPP) set to a different value from that used by gnustep-make ($MAKECPP).
  To avoid conflicts/problems, reconfigure/reinstall gnustep-make to use $CPP or run the gnustep-base configure again with your CPP environ
ment variable set to $MAKECPP])
  fi
fi
if test "$CXX" = ""; then
  CXX=$MAKECXX
else
  if test "$CXX" != "$MAKECXX"; then
    AC_MSG_WARN([You are running configure with the compiler ($CXX) set to a different value from that used by gnustep-make ($MAKECXX).  To
 avoid conflicts/problems, reconfigure/reinstall gnustep-make to use $CXX or run the gnustep-base configure again with your CXX environment
 variable set to $MAKECXX])
  fi
fi
AC_PROG_CC
AC_PROG_CPP

#--------------------------------------------------------------------
# The following is so that headers and custom libraries
# in the GNUstep root are used before the standard ones
#--------------------------------------------------------------------
GRAPHIC_CFLAGS="$CPPFLAGS"
GRAPHIC_LFLAGS="$LDFLAGS"

#
# It looks like we ought to source the whole GNUstep.sh here, and even
# ask it to output all variables!  That way we have access to (eg)
# GNUSTEP_SYSTEM_HEADERS below.
#
GNUSTEP_SH_EXPORT_ALL_VARIABLES=yes
. "$GNUSTEP_MAKEFILES/GNUstep.sh"
unset GNUSTEP_SH_EXPORT_ALL_VARIABLES

# For backwards compatibility, define GNUSTEP_SYSTEM_HEADERS from
# GNUSTEP_SYSTEM_ROOT if not set yet.
if test x"$GNUSTEP_SYSTEM_HEADERS" = x""; then
  GNUSTEP_SYSTEM_HEADERS="$GNUSTEP_SYSTEM_ROOT/Library/Headers"
fi

if test x"$GNUSTEP_SYSTEM_LIBRARIES" = x""; then
  GNUSTEP_SYSTEM_LIBRARIES="$GNUSTEP_SYSTEM_ROOT/Library/Libraries"
fi

if test "$GNUSTEP_IS_FLATTENED" = no; then
  clean_target_os=`$GNUSTEP_MAKEFILES/clean_os.sh $target_os`
  clean_target_cpu=`$GNUSTEP_MAKEFILES/clean_cpu.sh $target_cpu`
  obj_dir=$clean_target_cpu/$clean_target_os
  GNUSTEP_LDIR=$GNUSTEP_SYSTEM_LIBRARIES/$obj_dir
  GNUSTEP_HDIR=$GNUSTEP_SYSTEM_HEADERS/$LIBRARY_COMBO
else
  GNUSTEP_LDIR=$GNUSTEP_SYSTEM_LIBRARIES
  GNUSTEP_HDIR=$GNUSTEP_SYSTEM_HEADERS
fi
CPPFLAGS="$CPPFLAGS -I$GNUSTEP_HDIR"
LDFLAGS="$LDFLAGS -L$GNUSTEP_LDIR/$LIBRARY_COMBO -L$GNUSTEP_LDIR"

#--------------------------------------------------------------------
# Add target OS directories as necessary
#--------------------------------------------------------------------
case "$target_os" in
  freebsd* | openbsd* )   
	      CPPFLAGS="-I/usr/pkg/include $CPPFLAGS"
              LDFLAGS="-L/usr/pkg/lib $LDFLAGS";;
  netbsd*)    CPPFLAGS="-I/usr/pkg/include $CPPFLAGS"
	      LDFLAGS="-Wl,-R/usr/pkg/lib -L/usr/pkg/lib $LDFLAGS";;
esac

#--------------------------------------------------------------------
# These headers/functions needed by gpbs.m
#--------------------------------------------------------------------
AC_CHECK_HEADERS(syslog.h)
AC_CHECK_FUNCS(syslog)

#--------------------------------------------------------------------
# Find TIFF (Solaris requires this to find the correct library, even though
# it does not link directly to it)
#--------------------------------------------------------------------
AC_ARG_WITH(tiff_library,
           [  --with-tiff-library=DIR TIFF library file are in DIR], ,
           with_tiff_library=)

if test -n "$with_tiff_library"; then
  GRAPHIC_LFLAGS="$GRAPHIC_LFLAGS -L$with_tiff_library"
fi

#--------------------------------------------------------------------
# Find for X windows
#--------------------------------------------------------------------
set_x_paths=yes
AC_PATH_XTRA
if test "x$no_x" = xyes; then
  set_x_paths=no
fi

PKG_PROG_PKG_CONFIG

if test $set_x_paths = yes; then
  GRAPHIC_CFLAGS="$GRAPHIC_CFLAGS $X_CFLAGS"
  CPPFLAGS="$CPPFLAGS $X_CFLAGS"
  # Note: Don't include $X_LIBS in LDFLAGS as it may conflict with
  # other libraries installed in /sw, /sw/lib/freetyp2/lib
  #GRAPHIC_LFLAGS="$X_LIBS $GRAPHIC_LFLAGS"
  # But do include it here just to find the extra x libraries 
  LDFLAGS="$LDFLAGS $X_LIBS"
  LIBS="-lX11 $X_EXTRA_LIBS $LIBS"

  AC_CHECK_LIB(Xext, main)
  if test "$HAVE_LIBXext" = no; then
    PKG_CHECK_MODULES([XEXT], [xext])
  fi

  AC_CHECK_LIB(Xt, main)
  if test "$HAVE_LIBXt" = no; then
    PKG_CHECK_MODULES([XT], [xt])
  fi

  AC_CHECK_LIB(Xmu, main)
  if test "$HAVE_LIBXmu" = no; then
    PKG_CHECK_MODULES([XMU], [xmu])
  fi

  AC_CHECK_LIB(Xfixes, XFixesSelectSelectionInput,
    [
      LIBS="-lXfixes $LIBS"
      AC_DEFINE(HAVE_XFIXES, 1, [Define to enable Xfixes support])
    ]
    ,)

  AC_CHECK_LIB(Xcursor, XcursorImageCreate,
    [
      LIBS="-lXcursor $LIBS"
      AC_DEFINE(HAVE_XCURSOR, 1, [Define to enable Xcursor support])
    ]
    ,)

  have_xshape=no
  AC_CHECK_HEADERS(X11/extensions/shape.h,
	have_xshape=yes,,
	[#include <X11/Xlib.h>])
  if test $have_xshape = yes; then
  AC_CHECK_LIB(Xext, XShapeCombineMask,
    [
      AC_DEFINE(HAVE_XSHAPE, 1, [Define to enable Xshape support])
    ]
    ,)
  fi

  AC_CHECK_HEADERS(X11/extensions/sync.h,,,
  [#include <X11/X.h>
   #include <X11/Xutil.h>
  ])

  have_xrandr=no
  AC_CHECK_HEADERS(X11/extensions/Xrandr.h, have_xrandr=yes,,)
  if test $have_xrandr = yes; then
  AC_CHECK_LIB(Xrandr, XRRUpdateConfiguration,
    [
       LIBS="-lXrandr $LIBS"
       AC_DEFINE(HAVE_XRANDR, 1, [Define to enable Xrandr support])
    ]
    ,)
  fi

  LIBS="$X_LIBS $LIBS"
fi
AC_SUBST(X_PRE_LIBS)

# Old X11 support
AC_MSG_CHECKING([for X11 function prototypes])
AC_EGREP_CPP(yes, 
 [#include <X11/Xlib.h>
  #if NeedFunctionPrototypes
    yes
  #endif
  ], have_funcproto=yes, have_funcproto=no)
AC_MSG_RESULT([$have_funcproto])
if test "$have_funcproto" = no; then
  AC_DEFINE(FUNCPROTO,1,[Define if X11 doesn't automatically use prototypes])
fi

#--------------------------------------------------------------------
# Find for DPS
#--------------------------------------------------------------------
save_CPPFLAGS=${CPPFLAGS}
if test -r $x_includes/X11/DPS; then
  CPPFLAGS="-I$x_includes/X11 $CPPFLAGS"
fi
AC_CHECK_HEADERS([DPS/dpsclient.h DPS/dpsNXargs.h])
if test $ac_cv_header_DPS_dpsNXargs_h = no; then
  CPPFLAGS="${save_CPPFLAGS}"
fi

#--------------------------------------------------------------------
# freetype libraries
#--------------------------------------------------------------------
# Mac OS X with macports has a freetype which includes Apple's Foundation
# which we do not want
AC_ARG_WITH(freetype,
           [  --without-freetype Do not check for or include freetype], ,
	   with_freetype=yes)

have_freetype=no
if test $with_freetype = yes; then
  PKG_CHECK_MODULES([FREETYPE], [freetype2])

  if test x"$FREETYPE_LIBS" != x; then
    save_CPPFLAGS=${CPPFLAGS}
    CPPFLAGS="${FREETYPE_CFLAGS} ${CPPFLAGS}"
    AC_CHECK_HEADER(ft2build.h,have_freetype=yes, have_freetype=no)
    CPPFLAGS="${save_CPPFLAGS}"
  fi
fi

#--------------------------------------------------------------------
# Extended font support & UTF8 support
#--------------------------------------------------------------------
PKG_CHECK_MODULES(XFT, xft, WITH_XFT=yes, WITH_XFT=no)

if test $WITH_XFT = no; then
  # pkg-config failed for some reason, Check the old way?
  save_CPPFLAGS=${CPPFLAGS}
  save_LIBS=${LIBS}
  if test $have_freetype = yes; then
    LIBS="$FREETYPE_LIBS $LIBS"
    CPPFLAGS="$FREETYPE_CFLAGS $CPPFLAGS"
  fi
  AC_CHECK_LIB(Xft, XftFontOpen)
  AC_CHECK_HEADER(X11/Xft/Xft.h)
  if test "$ac_cv_lib_Xft_XftFontOpen" = yes -a "$ac_cv_header_X11_Xft_Xft_h" = yes; then
    WITH_XFT=yes
    XFT_LIBS="-lXft $FREETYPE_LIBS"
    XFT_CFLAGS="$FREETYPE_CFLAGS"
  fi
  CPPFLAGS=${save_CPPFLAGS}
  LIBS=${save_LIBS}
else
  WITH_XFT=yes
fi

if test $WITH_XFT = yes; then
  # Now add in some extra defines depending on what we have.
  save_CPPFLAGS=${CPPFLAGS}
  save_LIBS=${LIBS}
  CPPFLAGS="$XFT_CFLAGS $CPPFLAGS"
  LIBS="$XFT_LIBS $LIBS"
  AC_DEFINE(HAVE_XFT,1,[Define if you have a functional XFreeType installation, including libXft.])
  AC_CHECK_LIB(Xft, XftDrawStringUtf8, have_utf8=yes, have_utf8=no)
  if test "$have_utf8" = yes; then
    AC_DEFINE(HAVE_UTF8,1,[Define if you have XftDrawStringUtf8])
  fi
  AC_CHECK_LIB(Xft, XftPatternGetString, have_xftpgs=yes, have_xftpgs=no)
  AC_HAVE_FUNCS(Xutf8LookupString)
  AC_CHECK_LIB(fontconfig, FcPatternCreate, have_fc=yes, have_fc=no)
  AC_CHECK_HEADER(fontconfig/fontconfig.h)
  if test "$have_fc" = yes -a "$ac_cv_header_fontconfig_fontconfig_h" = yes; then
    XFT_LIBS="${XFT_LIBS} -lfontconfig"
    AC_DEFINE(HAVE_FC,1,[Define if you have FcPatternCreate])
  fi
  CPPFLAGS=${save_CPPFLAGS}
  LIBS=${save_LIBS}
fi
AC_SUBST(WITH_XFT)

#--------------------------------------------------------------------
# GLX support
#--------------------------------------------------------------------
WITH_GLX=no
AC_ARG_ENABLE(glx, 
  [  --disable-glx           Disable GLX support],,
  enable_glx=yes)
if test "x$enable_glx" = "xyes"; then
   AC_CHECK_LIB(GL, glXMakeContextCurrent, have_glx=yes, have_glx=no)
   AC_CHECK_HEADER(GL/glx.h)
   AC_MSG_CHECKING([for GLX_RGBA_TYPE])
   AC_EGREP_CPP(yes, 
     [#include <GL/glx.h>
      #ifdef GLX_RGBA_TYPE
        yes
      #endif
      ], have_glx_rgba=yes, have_glx_rgba=no)
   AC_MSG_RESULT([$have_glx_rgba])
   if test "$have_glx" = yes -a "$ac_cv_header_GL_glx_h" = yes -a "$have_glx_rgba" = yes; then
     GLX_LIBS="-lGL"
     GLX_FLAGS=""
     WITH_GLX=yes
     AC_DEFINE(HAVE_GLX,1,[Define if you have the glx library])
   fi
fi
AC_SUBST(WITH_GLX)

#--------------------------------------------------------------------
# XIM support
#--------------------------------------------------------------------
AC_ARG_ENABLE(xim, 
  [  --disable-xim           Disable XIM support],,
  enable_xim=yes)
if test "x$enable_xim" = "xyes"; then
 AC_DEFINE(USE_XIM,1,[Define to enable XIM support])
fi

#--------------------------------------------------------------------
# Functions
#--------------------------------------------------------------------
AC_HAVE_FUNCS(usleep)

have_xshm=no
AC_CHECK_HEADERS(X11/extensions/XShm.h,
	have_xshm=yes,,
	[#include <X11/Xlib.h>])
AC_CHECK_FUNCS(shmctl)
if test "$ac_cv_header_X11_extensions_XShm_h" = yes -a "$ac_cv_func_shmctl" = yes; then
  AC_DEFINE(XSHM,1,[Define if you have X11 XShm extensions])
fi

AC_CHECK_LIB(X11, XInternAtoms,
  AC_DEFINE([HAVE_XINTERNATOMS], 1, 
    [Define to 1 if you have 'XInternAtoms' function.]),                  
  ,
  $X_LIBS)

#--------------------------------------------------------------------
# Window's graphics library
#--------------------------------------------------------------------
AC_CHECK_LIB(gdi32, main, have_gdi32=yes, have_gdi32=no)
if test "$have_gdi32" = yes; then
  WIN32_LIBS="-lgdi32 -lm"
fi
AC_CHECK_LIB(msimg32, main, have_msimg32=yes, have_msimg32=no)
if test "$have_msimg32" = yes; then
  WIN32_LIBS="${WIN32_LIBS} -lmsimg32"
fi
AC_CHECK_LIB(imm32, main, have_imm32=yes, have_imm32=no)
if test "$have_imm32" = yes; then
  WIN32_LIBS="${WIN32_LIBS} -limm32"
fi

#--------------------------------------------------------------------
# WGL support
#--------------------------------------------------------------------
WITH_WGL=no
AC_ARG_ENABLE(wgl, 
  [  --disable-wgl           Disable WGL support],,
  enable_wgl=yes)
if test "x$enable_wgl" = "xyes"; then
   AC_CHECK_LIB(opengl32, main, have_wgl=yes, have_wgl=no)
   save_CPPFLAGS="$CPPFLAGS"
   CPPFLAGS="$CPPFLAGS"
   AC_CHECK_HEADER(windows.h)
   CPPFLAGS="$save_CPPFLAGS"
   if test "$have_wgl" = yes -a "$ac_cv_header_windows_h" = yes; then
     AC_MSG_CHECKING([for wgl])
     WGL_LIBS="-lopengl32 -lgdi32"
     WGL_CFLAGS=""

     save_libs="$LIBS"
     LIBS="$WGL_LIBS"
     AC_TRY_LINK([#include <windows.h>], [ wglCreateContext(0); ],
	   [use_wgl=yes], [use_wgl=no])
     LIBS="$save_libs"
     AC_MSG_RESULT([$use_wgl])
     WITH_WGL=$use_wgl
     AC_DEFINE(HAVE_WGL,1,[Define if you have the wgl library])
   fi
fi
AC_SUBST(WITH_WGL)

#--------------------------------------------------------------------
# libart graphics libraries
#--------------------------------------------------------------------
AC_MSG_CHECKING([for libart2])
LIBART_VERSION="`libart2-config --version 2>/dev/null`"
if test x"$LIBART_VERSION" = x; then
  LIBART_VERSION="none"
fi
AC_MSG_RESULT([$LIBART_VERSION])
LIBART_LIBS="`libart2-config --libs 2>/dev/null`"
LIBART_CFLAGS="`libart2-config --cflags 2>/dev/null`"

if test x"$LIBART_LIBS" = x; then
  # No libart2-config, is there any point in checking for individual libs?
  AC_CHECK_LIB(art_lgpl_2, main, have_libart=yes, have_libart=no)
  if test "$have_libart" = yes; then
    AC_CHECK_HEADER(libart_lgpl/libart.h,have_libart=yes, have_libart=no)
  fi
  if test "$have_libart" = yes; then
    LIBART_LIBS="-lart_lgpl_2"
  fi
else
  have_libart=yes
fi

#--------------------------------------------------------------------
# Cairo graphics libraries
#--------------------------------------------------------------------
PKG_CHECK_MODULES(CAIRO, cairo, have_cairo=yes, have_cairo=no)
PKG_CHECK_MODULES(CAIRO_FT, cairo-ft, have_cairo_ft=yes, have_cairo_ft=no)
PKG_CHECK_MODULES(CAIRO_XLIB, cairo-xlib, have_cairo_xlib=yes, have_cairo_xlib=no)
PKG_CHECK_MODULES(CAIRO_WIN32, cairo-win32, have_cairo_win32=yes, have_cairo_win32=no)
PKG_CHECK_MODULES(CAIRO_GLITZ, cairo-glitz, have_cairo_glitz=yes, have_cairo_glitz=no)
PKG_CHECK_MODULES(FONTCONFIG, fontconfig, have_fontconfig=yes, have_fontconfig=no)

if test "$have_cairo" = no; then
  AC_CHECK_LIB(cairo, cairo_create, have_cairo=yes)
fi

if test "$have_cairo_ft" = no; then
  AC_CHECK_LIB(cairo, cairo_ft_font_face_create_for_ft_face, have_cairo_ft=yes)
fi

if test "$have_cairo_xlib" = no; then
  AC_CHECK_LIB(cairo, cairo_xlib_surface_create, have_cairo_xlib=yes)
fi

if test "$have_cairo_win32" = no; then
  AC_CHECK_LIB(cairo, cairo_win32_surface_create, have_cairo_win32=yes)
fi

#--------------------------------------------------------------------
# XRender support
#--------------------------------------------------------------------

have_xrender=no
AC_CHECK_LIB(Xrender, XRenderFindVisualFormat, have_xrender=yes)

#--------------------------------------------------------------------
# Glitz libraries
#--------------------------------------------------------------------
WITH_GLITZ=no
AC_ARG_ENABLE(glitz, 
  [  --enable-glitz          Enable Glitz support],,
  enable_glitz=no)

if test "x$enable_glitz" = "xyes"; then
  PKG_CHECK_MODULES(GLITZ, glitz, have_glitz=yes, have_glitz=no)
  save_CPPFLAGS=$CPPFLAGS
    CPPFLAGS="$CPPFLAGS $GLITZ_CFLAGS"
    AC_CHECK_HEADER(glitz.h,have_glitz_h=yes, have_glitz_h=no)
  CPPFLAGS=$save_CPPFLAGS
  if test "$have_glitz" = yes -a "$have_glitz_h" = yes; then
  
    if test "x$WITH_GLX" = "xyes"; then
      PKG_CHECK_MODULES(GLITZ_GLX, glitz-glx, have_glitz_glx=yes, have_glitz_glx=no)
      save_CPPFLAGS=$CPPFLAGS
        CPPFLAGS="$CPPFLAGS $GLITZ_GLX_CFLAGS"
        AC_CHECK_HEADER(glitz-glx.h,have_glitz_glx_h=yes, have_glitz_glx_h=no)
      CPPFLAGS=$save_CPPFLAGS
      if test "$have_glitz_glx" = yes -a "$have_glitz_glx_h" = yes; then
        GLITZ_LIBS="$GLITZ_LIBS $GLITZ_GLX_LIBS"
        GLITZ_CFLAGS="$GLITZ_CFLAGS $GLITZ_GLX_CFLAGS"
        AC_DEFINE(HAVE_GLITZ_GLX,1,[Define if you have glitz-glx])
        WITH_GLITZ=yes
      fi
    fi
  
    if test "x$WITH_WGL" = "xyes"; then
      PKG_CHECK_MODULES(GLITZ_WGL, glitz-wgl, have_glitz_wgl=yes, have_glitz_wgl=no)
      save_CPPFLAGS=$CPPFLAGS
        CPPFLAGS="$CPPFLAGS $GLITZ_WGL_CFLAGS"
        AC_CHECK_HEADER(glitz-wgl.h,have_glitz_wgl_h=yes, have_glitz_wgl_h=no)
      CPPFLAGS=$save_CPPFLAGS
      if test "$have_glitz_wgl" = yes -a "$have_glitz_wgl_h" = yes; then
        GLITZ_LIBS="$GLITZ_LIBS $GLITZ_WGL_LIBS"
        GLITZ_CFLAGS="$GLITZ_CFLAGS $GLITZ_WGL_CFLAGS"
        AC_DEFINE(HAVE_GLITZ_WGL,1,[Define if you have glitz-wgl])
        WITH_GLITZ=yes
      fi
    fi

    if test "$WITH_GLITZ" != "yes" ; then
      AC_MSG_ERROR([Invalid glitz backend : glitz-glx or glitz-wgl required.])
      AC_MSG_NOTICE([Glitz disable])
    fi
    AC_DEFINE(USE_GLITZ,1,[Define to enable glitz support])
  fi
  AC_SUBST(WITH_GLITZ)
fi

#--------------------------------------------------------------------
# Set definitions
#--------------------------------------------------------------------
WITH_WRASTER=no
AC_SUBST(WITH_WRASTER)

# Revert to previous flags (removing added GNUstep flags)
CPPFLAGS=${GRAPHIC_CFLAGS}
LDFLAGS=${GRAPHIC_LFLAGS}
#--------------------------------------------------------------------
# Which projects should we build?
#--------------------------------------------------------------------
BUILD_SERVER=x11
BUILD_GRAPHICS=cairo
case $target_os in
  *mingw32* ) BUILD_SERVER=win32
              BUILD_GRAPHICS=winlib;;
esac

AC_ARG_ENABLE(server, 
  [  --enable-server=SRV     Build server type: x11, win32, wayland],,
  enable_server=$BUILD_SERVER)
AC_ARG_ENABLE(graphics, 
  [  --enable-graphics=GPH   Build graphics: xlib, xdps, winlib, art, cairo, opal],,
  enable_graphics="$BUILD_GRAPHICS")

BUILD_SERVER=$enable_server
BUILD_GRAPHICS="$enable_graphics"
AC_MSG_CHECKING(Backend Server)
AC_MSG_RESULT($BUILD_SERVER)

AC_MSG_CHECKING(Backend Graphics)
AC_MSG_RESULT($BUILD_GRAPHICS)

if test x"$BUILD_GRAPHICS" = "xcairo"; then
  if test "$have_freetype" = no; then
    AC_MSG_WARN([can't find freetype, required for graphics=cairo])
    if test $BUILD_SERVER = win32; then
      BUILD_GRAPHICS=winlib
    elif test $BUILD_SERVER = wayland; then
      AC_MSG_ERROR([wayland backend requires cairo])
    else
      BUILD_GRAPHICS=xlib
    fi
    AC_MSG_NOTICE([Switching to $BUILD_GRAPHICS])
  elif test "$have_cairo" = no -o "$have_cairo-ft" = no ; then
    AC_MSG_WARN([can't find cairo, required for graphics=cairo!])
    if test $BUILD_SERVER = win32; then
      BUILD_GRAPHICS=winlib
    elif test $BUILD_SERVER = wayland; then
      AC_MSG_ERROR([wayland backend requires cairo])
    else
      BUILD_GRAPHICS=art
    fi
    AC_MSG_NOTICE([Switching to $BUILD_GRAPHICS])
  elif test $BUILD_SERVER = win32 -a "$have_fontconfig" = no ; then
    AC_MSG_WARN([can't find fontconfig, required for graphics=cairo!])
    BUILD_GRAPHICS=winlib
    AC_MSG_NOTICE([Switching to $BUILD_GRAPHICS])
  else
    AC_MSG_CHECKING(Cairo backend)
    CAIRO_LIBS="$CAIRO_FT_LIBS"
    CAIRO_CFLAGS="$CAIRO_FT_CFLAGS"
    if test $BUILD_SERVER = win32; then
      CAIRO_LIBS="$CAIRO_LIBS $FONTCONFIG_LIBS"
      CAIRO_CFLAGS="$CAIRO_CFLAGS $FONTCONFIG_CFLAGS"
    fi
    
    if test "$have_xrender" = yes; then
      if test $BUILD_SERVER = x11 -a "x$have_cairo_xlib" = "xyes"; then
        AC_DEFINE(XRENDER,1,[Define if you have X11 XRender extension])
        CAIRO_LIBS="$CAIRO_LIBS -lXrender"
      fi
    fi
    if test "x$WITH_GLITZ" = "xyes" -a "x$have_cairo_glitz" = "xyes"; then
      if test $BUILD_SERVER = win32 -a "x$have_cairo_win32" = "xyes"; then
        CAIRO_GLITZ_LIBS="$CAIRO_GLITZ_LIBS $GLITZ_WGL_LIBS"
        CAIRO_GLITZ_CFLAGS="$CAIRO_GLITZ_CFLAGS $GLITZ_WGL_CFLAGS"
      elif test $BUILD_SERVER = x11 -a "x$have_cairo_xlib" = "xyes"; then
        CAIRO_GLITZ_LIBS="$CAIRO_GLITZ_LIBS $GLITZ_GLX_LIBS"
        CAIRO_GLITZ_CFLAGS="$CAIRO_GLITZ_CFLAGS $GLITZ_GLX_CFLAGS"
      else
        AC_MSG_ERROR([Invalid Glitz installation])
      fi
      CAIRO_LIBS="$CAIRO_LIBS $CAIRO_GLITZ_LIBS"
      CAIRO_CFLAGS="$CAIRO_CFLAGS $CAIRO_GLITZ_CFLAGS"
      AC_MSG_RESULT(glitz)
      AC_MSG_WARN([**** Cairo-over-Glitz backend is highly experimental])
      AC_MSG_WARN([**** You definitively shouldn't do that.])
      AC_MSG_WARN([**** It actually does not work !])
      AC_MSG_WARN([**** Are you a backend developer ?])
    elif test $BUILD_SERVER = win32 -a "x$have_cairo_win32" = "xyes"; then
      CAIRO_LIBS="$CAIRO_LIBS $CAIRO_WIN32_LIBS $WIN32_LIBS"
      CAIRO_CFLAGS="$CAIRO_CFLAGS $CAIRO_WIN32_CFLAGS"
      AC_MSG_RESULT(winlib)
      #AC_MSG_WARN([**** Cairo backend on win32 is experimental and is not supported.])
      #AC_MSG_WARN([**** Are you a backend developer ?])
    elif test $BUILD_SERVER = x11 -a "x$have_cairo_xlib" = "xyes"; then
      CAIRO_LIBS="$CAIRO_LIBS $CAIRO_XLIB_LIBS $XFT_LIBS"
      CAIRO_CFLAGS="$CAIRO_CFLAGS $CAIRO_XLIB_CFLAGS"
      AC_MSG_RESULT(xlib)
    elif test $BUILD_SERVER = wayland; then
      AC_CHECK_HEADERS(wayland-util.h,,
        [AC_MSG_ERROR([**** No wayland-util.h. Install libwayland-dev or equivalent.])])
      AC_CHECK_LIB(wayland-client, wl_display_flush,,
        [AC_MSG_ERROR([**** No wl_display_flush in libwayland-client. Install correct version of libwayland-dev or equivalent.])])
      AC_CHECK_HEADERS(xkbcommon/xkbcommon.h,,
        [AC_MSG_ERROR([**** No xkbcommon/xkbcommon.h. Required for wayland. Install libxkbcommon-dev or equivalent.])])
      AC_CHECK_LIB(xkbcommon, xkb_context_new,,
        [AC_MSG_ERROR([**** No xkb_context_new in libxkbcommon. Install correct version of libxkbcommon-dev or equivalent.])])
      CAIRO_LIBS="$CAIRO_LIBS $XFT_LIBS"
      CAIRO_CFLAGS="$CAIRO_CFLAGS"
      LIBS="-lwayland-client -lxkbcommon $LIBS"
    else
      AC_MSG_ERROR([Invalid Cairo installation])
    fi
    LIBS="$CAIRO_LIBS $LIBS"
    CPPFLAGS="$CAIRO_CFLAGS $CPPFLAGS"
  fi
fi

if test x"$BUILD_GRAPHICS" = "xxdps"; then
  CPPFLAGS="-I$x_includes/X11 $CPPFLAGS"
  LIBS="-ldpstk -ldps -lpsres -lXt $X_PRE_LIBS $LIBS"
elif test x"$BUILD_GRAPHICS" = "xcairo"; then
 : # Alread handled above
elif test x"$BUILD_GRAPHICS" = "xart"; then
  if test "$have_freetype" = no; then
    AC_MSG_WARN([can't find freetype, required for graphics=art])
    if test $BUILD_SERVER = win32; then
      BUILD_GRAPHICS=winlib
    else
      BUILD_GRAPHICS=xlib
    fi
    AC_MSG_NOTICE([Switching to $BUILD_GRAPHICS])
  elif test "$have_libart" = no; then
    AC_MSG_WARN([can't find libart, required for graphics=art])
    if test $BUILD_SERVER = win32; then
      BUILD_GRAPHICS=winlib
    else
      BUILD_GRAPHICS=xlib
    fi
    AC_MSG_NOTICE([Switching to $BUILD_GRAPHICS])
  else
    LIBS="$LIBART_LIBS $FREETYPE_LIBS $LIBS"
    CPPFLAGS="$LIBART_CFLAGS $FREETYPE_CFLAGS $CPPFLAGS"
  fi
elif test x"$BUILD_GRAPHICS" = "xxlib"; then
 LIBS="$FREETYPE_LIBS $LIBS"
elif test x"$BUILD_GRAPHICS" = "xwinlib"; then
 : # Nothing to do
elif test x"$BUILD_GRAPHICS" = "xopal"; then
 CPPFLAGS="$FONTCONFIG_CFLAGS $FREETYPE_CFLAGS $CPPFLAGS"
 LIBS="-lopal -lgnustep-corebase $LIBS"
 LIBS="$FONTCONFIG_LIBS $FREETYPE_LIBS $LIBS"
 if test "$have_freetype" = no ; then # FCFaceInfo requires this
    AC_MSG_WARN([can't find freetype, required for graphics=opal!])
    if test $BUILD_SERVER = win32; then
      BUILD_GRAPHICS=winlib
    else
      BUILD_GRAPHICS=xlib
    fi
    AC_MSG_NOTICE([Switching to $BUILD_GRAPHICS])
 elif test "$have_fontconfig" = no ; then
    AC_MSG_WARN([can't find fontconfig, required for graphics=opal!])
    if test $BUILD_SERVER = win32; then
      BUILD_GRAPHICS=winlib
    else
      BUILD_GRAPHICS=xlib
    fi
    AC_MSG_NOTICE([Switching to $BUILD_GRAPHICS])
  fi
else
  AC_MSG_ERROR([Invalid graphics backend $BUILD_GRAPHICS])
fi

if test x"$BUILD_GRAPHICS" = "xwinlib"; then
  LIBS="$WIN32_LIBS $LIBS"
fi
if test x"$BUILD_GRAPHICS" = "xxlib"; then
  LIBS="$XFT_LIBS $LIBS"
  CPPFLAGS="$XFT_CFLAGS $CPPFLAGS"
fi

if test $BUILD_SERVER = x11; then
  if test $set_x_paths = no; then
    AC_MSG_ERROR([No X11 libraries/headers for building x11 server])
  fi
  if test "$ac_cv_lib_Xt_main" = no; then
    AC_MSG_ERROR([libXt not found - required for building x11 server])
  fi
  if test "$ac_cv_lib_Xext" = no; then
    AC_MSG_ERROR([libXext not found - required for building x11 server])
  fi
  if test "x$WITH_GLX" = "xyes"; then
    LIBS="$GLX_LIBS $LIBS"
    CPPFLAGS="$GLX_CFLAGS $CPPFLAGS"
  else
    AC_MSG_NOTICE([Warning : no OpenGL support for X11 backend])
  fi
fi

if test $BUILD_SERVER = win32; then
  if test "x$WITH_WGL" = "xyes"; then
    LIBS="$LIBS $WGL_LIBS"
    CPPFLAGS="$CPPFLAGS $WGL_CFLAGS"
  else
    AC_MSG_NOTICE([Warning : no OpenGL support for win32 backend])
  fi
fi

AH_TOP([
#define SERVER_x11      1
#define SERVER_win32    2
#define SERVER_wayland  3
#define GRAPHICS_xdps   0
#define GRAPHICS_art    1
#define GRAPHICS_xlib   2
#define GRAPHICS_winlib 3
#define GRAPHICS_cairo  4
#define GRAPHICS_opal   5
])
AC_DEFINE_UNQUOTED(BUILD_SERVER,SERVER_$BUILD_SERVER,
	[Define to type of window server to build])
AC_DEFINE_UNQUOTED(BUILD_GRAPHICS,GRAPHICS_$BUILD_GRAPHICS,
	[Define to type of graphics context to build])
AC_SUBST(BUILD_GRAPHICS)
AC_SUBST(BUILD_SERVER)

#--------------------------------------------------------------------
# Check for -Wdeclaration-after-statement
#--------------------------------------------------------------------
AC_MSG_CHECKING(whether the compiler supports -Wdeclaration-after-statement)

saved_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Wdeclaration-after-statement"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM()],HAS_W_DECL_AFTER_STATEMENT=yes,HAS_W_DECL_AFTER_STATEMENT=no)
CFLAGS="$saved_CFLAGS"

AC_MSG_RESULT($HAS_W_DECL_AFTER_STATEMENT)

if test x"$HAS_W_DECL_AFTER_STATEMENT" = x"yes"; then
  WARN_FLAGS="-Wall -Wdeclaration-after-statement"
else
  WARN_FLAGS=-Wall
fi

AC_SUBST(WARN_FLAGS)

#--------------------------------------------------------------------
# Set the name of the backend
#--------------------------------------------------------------------
AC_ARG_WITH(name, 
           [  --with-name=PREFIX      Set the name of the backend (def=back)])
AC_MSG_CHECKING(Backend name)
if test "x$with_name" = "x"; then
 BACKEND_NAME=back
else
 BACKEND_NAME=${with_name}
fi
AC_MSG_RESULT($BACKEND_NAME)
AC_SUBST(BACKEND_NAME)

AC_CONFIG_FILES([back.make config.make])
AC_OUTPUT

AS_IF([test $BUILD_GRAPHICS = art || test $BUILD_GRAPHICS = xlib || test $BUILD_GRAPHICS = xdps], [
cat <<EOF
m4_text_box([WARNING])
You are about to build the $BUILD_GRAPHICS backend which is deprecated
and likely to be removed in the future.
EOF
])
