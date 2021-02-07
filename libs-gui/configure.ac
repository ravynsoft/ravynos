# configure.in for GNUstep GUI library
# Process this file with autoconf to produce a configure script.
#
# Copyright (C) 1996-2015 Free Software Foundation, Inc.
#
# Author:  Adam Fedor <fedor@gnu.org>
#
# This file is part of the GNUstep GUI frontend (AppKit).
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 3 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; see the file COPYING.LIB.
# If not, see <http://www.gnu.org/licenses/> or write to the 
# Free Software Foundation, 51 Franklin Street, Fifth Floor, 
# Boston, MA 02110-1301, USA.
#

builtin(include, config/icu.m4)dnl
builtin(include, config/pkg.m4)dnl

AC_INIT
AC_CONFIG_SRCDIR([Source/NSApplication.m])

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

AC_CONFIG_HEADER(Headers/Additions/GNUstepGUI/config.h)

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
# Add target OS directories as necessary
#--------------------------------------------------------------------
CONFIG_SYSTEM_INCL="$CPPFLAGS"
CONFIG_SYSTEM_LIB_DIR="$LDFLAGS"
case "$target_os" in
  freebsd* | openbsd* )   
	      CPPFLAGS="$CPPFLAGS -I/usr/local/include"
              LDFLAGS="$LDFLAGS -L/usr/local/lib";;
  netbsd*)    CPPFLAGS="$CPPFLAGS -I/usr/pkg/include"
	      LDFLAGS="$LDFLAGS -Wl,-R/usr/pkg/lib -L/usr/pkg/lib";;
esac

AC_PATH_X	# Added for checking the existence of ungif.  Note that
                # -gui uses the API of the underlying window system ONLY IF
		# it is definitely necessary.  

#--------------------------------------------------------------------
# The following is so that headers and custom libraries
# in the GNUstep root are used before the standard ones
#--------------------------------------------------------------------

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


AC_CHECK_LIB(m, main)
AC_CHECK_FUNCS(rint rintf atan2f floorf)


#--------------------------------------------------------------------
# Support for determining mountpoints
#--------------------------------------------------------------------
AC_CHECK_HEADERS(mntent.h)
AC_CHECK_HEADERS(sys/mntent.h)
AC_CHECK_MEMBER(struct mntent.mnt_fsname,[AC_DEFINE(MNT_FSNAME,mnt_fsname,mntent structure member name)],,[#include <mntent.h>])
AC_CHECK_MEMBER(struct mntent.mnt_fsname,[AC_DEFINE(MNT_FSNAME,mnt_fsname,mntent structure member name)],,[#include <sys/mntent.h>])
AC_CHECK_MEMBER(struct mntent.mnt_dir,[AC_DEFINE(MNT_MEMB,mnt_dir,mntent structure member name)],,[#include <mntent.h>])
AC_CHECK_MEMBER(struct mntent.mnt_mountp,[AC_DEFINE(MNT_MEMB,mnt_mountp,mntent structure member name)],,[#include <sys/mntent.h>])
AC_FUNC_GETMNTENT
AC_CHECK_FUNCS(getmntinfo)
AC_CHECK_FUNCS(statfs)
AC_CHECK_FUNCS(statvfs)
AC_CHECK_HEADERS(sys/statvfs.h)
AC_CHECK_HEADERS(sys/vfs.h)

AC_CHECK_MEMBERS([struct statfs.f_flags, struct statfs.f_owner],[],[],[
#if	defined(HAVE_GETMNTINFO)
#include <sys/param.h>
#include <sys/mount.h>
#endif
#if defined (HAVE_SYS_STATVFS_H)
#include <sys/statvfs.h>
#endif
#if defined (HAVE_SYS_VFS_H)
#include <sys/vfs.h>
#endif
])
AC_CHECK_MEMBERS([struct statvfs.f_flag, struct statvfs.f_owner],[],[],[
#if defined (HAVE_SYS_STATVFS_H)
#include <sys/statvfs.h>
#endif
])

#--------------------------------------------------------------------
# Simple way to add a bunch of paths to the flags
#--------------------------------------------------------------------
AC_ARG_WITH(include-flags,
    [  --with-include-flags=FLAGS specify all include flags at once],
    include_flags="$withval", include_flags="no")
if test ${include_flags} != "no"; then
    CPPFLAGS="$CPPFLAGS ${include_flags}"
    CONFIG_SYSTEM_INCL="$CONFIG_SYSTEM_INCL ${include_flags}"
fi

AC_ARG_WITH(library-flags,
    [  --with-library-flags=FLAGS specify all library flags at once],
    library_flags="$withval", library_flags="no")
if test ${library_flags} != "no"; then
    LDFLAGS="$LDFLAGS ${library_flags}"
    CONFIG_SYSTEM_LIB_DIR="$CONFIG_SYSTEM_LIB_DIR ${library_flags}"
fi

#--------------------------------------------------------------------
# Find JPEG
#--------------------------------------------------------------------
GRAPHIC_CFLAGS=
GRAPHIC_LFLAGS=

AC_ARG_ENABLE(jpeg,
  [  --disable-jpeg          Disable JPEG support],,
  enable_jpeg=yes)

if test $enable_jpeg = yes; then
  AC_ARG_WITH(jpeg_library,
             [  --with-jpeg-library=DIR JPEG library file are in DIR], ,
             with_jpeg_library=)
  AC_ARG_WITH(jpeg_include,
          [  --with-jpeg-include=DIR JPEG include files are in DIR], ,
          with_jpeg_include=)

  if test -n "$with_jpeg_library"; then
    with_jpeg_library="-L$with_jpeg_library"
  fi
  if test -n "$with_jpeg_include"; then
    with_jpeg_include="-I$with_jpeg_include"
  fi

  CPPFLAGS="$with_jpeg_include ${CPPFLAGS}"
  LDFLAGS="$with_jpeg_library ${LDFLAGS}"
  AC_CHECK_HEADERS(jpeglib.h, have_jpeg=yes, have_jpeg=no)
  if test "$have_jpeg" = yes; then
    AC_CHECK_LIB(jpeg, jpeg_destroy_decompress)
  else
    ac_cv_lib_jpeg_jpeg_destroy_decompress=no
  fi

  if test "$ac_cv_lib_jpeg_jpeg_destroy_decompress" = yes; then
    GRAPHIC_LFLAGS="$with_jpeg_library $GRAPHIC_LFLAGS"
    GRAPHIC_CFLAGS="$with_jpeg_include $GRAPHIC_CFLAGS"
  else
    echo "GNUstep requires libjpeg to support JPEG images, but this library"
    echo "can't be found. If you really want to compile GNUstep without"
    echo "JPEG support, pass the --disable-jpeg option to ./configure."
    echo "Note that this isn't recommended. For more information, see:"
    echo "http://gnustep.made-it.com/BuildGuide/index.html#JPEG"
    AC_MSG_ERROR([libjpeg not found.])
  fi
fi

#--------------------------------------------------------------------
# Find TIFF
#--------------------------------------------------------------------
AC_ARG_WITH(tiff_library, 
           [  --with-tiff-library=DIR TIFF library file are in DIR], ,
           with_tiff_library=)
AC_ARG_WITH(tiff_include,  
	[  --with-tiff-include=DIR TIFF include files are in DIR], ,
        with_tiff_include=)

if test -n "$with_tiff_library"; then
  with_tiff_library="-L$with_tiff_library"
fi
if test -n "$with_tiff_include"; then
  with_tiff_include="-I$with_tiff_include"
fi

CPPFLAGS="$with_tiff_include ${CPPFLAGS}"
LDFLAGS="$with_tiff_library ${LDFLAGS}"
AC_CHECK_LIB(z, main)
AC_CHECK_HEADER(tiffio.h, have_tiff=yes, have_tiff=no)
if test "$have_tiff" = yes; then
  AC_CHECK_LIB(tiff, TIFFReadScanline)
  if test "$ac_cv_lib_tiff_TIFFReadScanline" = yes; then
    GRAPHIC_LFLAGS="$with_tiff_library $GRAPHIC_LFLAGS"
    GRAPHIC_CFLAGS="$with_tiff_include $GRAPHIC_CFLAGS"
  else
    have_tiff=no
  fi
fi

if test "$have_tiff" = no; then
  AC_MSG_WARN(Cannot find libtiff header and/or library)
  echo "* The GUI library reqiures the TIFF library"
  echo "* Use --with-tiff-library to specify the tiff library"
  echo "* directory if it is not in the usual place(s)"
  echo "* You may also have to specify --with-jpeg-library if the jpeg"
  echo "* library is needed by tiff",
  AC_MSG_ERROR(gnustep-gui will not compile without tiff)
fi

#--------------------------------------------------------------------
# Find PNG
#--------------------------------------------------------------------
AC_ARG_ENABLE(png,
  [  --disable-png           Disable PNG support],,
  enable_png=yes)

if test $enable_png = yes; then
  # use libpng-config if available
  AC_CHECK_PROG(HAVE_LIBPNG_CONFIG, [libpng-config], yes)
  if test "$HAVE_LIBPNG_CONFIG" = "yes"; then
    PNG_CFLAGS="`libpng-config --I_opts`"
    PNG_LDFLAGS="`libpng-config --ldflags`"
    CPPFLAGS="${CPPFLAGS} $PNG_CFLAGS"
    LDFLAGS="$PNG_LDFLAGS ${LDFLAGS}"
    AC_CHECK_HEADERS(png.h)
    AC_CHECK_LIB(png, png_sig_cmp)
    if test "$ac_cv_search_png_sig_cmp" != no; then
      GRAPHIC_CFLAGS="$PNG_CFLAGS $GRAPHIC_CFLAGS"
      GRAPHIC_LFLAGS="$PNG_LDFLAGS $GRAPHIC_LFLAGS"
      AC_DEFINE([HAVE_LIBPNG], [1])
    fi
  else
    AC_MSG_WARN([Can't find libpng-config, guessing required headers and libraries.])
    AC_CHECK_HEADERS(png.h)
    AC_CHECK_LIB(png, png_sig_cmp)
  fi



  if test "$ac_cv_lib_png_png_sig_cmp" = no && test "$ac_cv_search_png_sig_cmp" = no; then
    echo "GNUstep requires libpng to support PNG images, but this library"
    echo "can't be found. If you really want to compile GNUstep without"
    echo "PNG support, pass the --disable-png option to ./configure."
    echo "Note that this isn't recommended."
    AC_MSG_ERROR([libpng not found.])
  fi
fi

#--------------------------------------------------------------------
# Find additional image libs
#--------------------------------------------------------------------
AC_ARG_ENABLE(ungif,
  [  --disable-ungif         Disable libungif-based GIF support],,
  enable_ungif=yes)
AC_ARG_ENABLE(libgif,
  [  --enable-libgif         Enable libgif-based GIF support],,
  enable_libgif=no)
if test "${enable_libgif}" = yes; then
  enable_ungif=no
fi

have_ungif=no
if test "${enable_ungif}" = yes; then
  AC_CHECK_LIB(ungif, DGifOpen)
  if test "${ac_cv_lib_ungif_DGifOpen}" = yes; then
    have_ungif=yes
  fi

  if test "${with_x}" != no && test "${ac_cv_lib_ungif_DGifOpen}" = no; then
    orig_CPPFLAGS="${CPPFLAGS}"
    orig_LDFLAGS="${LDFLAGS}"
    orig_LIBS="${LIBS}"
    AC_MSG_NOTICE([Checking if ungif is linked against -lX11])
    # This implies either that ungif is not installed at all, or that it
    # explicitly refers to the symbols defined in X11.  Now see if the latter
    # is the case.
    CPPFLAGS="${CPPFLAGS} -I${ac_x_includes}"
    LDFLAGS="${LDFLAGS} -L${ac_x_libraries}"
    LIBS="${LIBS} -lX11"

    AC_CHECK_LIB(ungif, DGifCloseFile)
    if test "${ac_cv_lib_ungif_DGifCloseFile}" = no; then
      # This indicates ungif is not installed at all.  The flags reverts to
      # the orignal value.
      CPPFLAGS="${orig_CPPFLAGS}"
      LDFLAGS="${orig_LDFLAGS}"
      LIBS="${orig_LIBS}"
    else
      # This indicates ungif actually refers to the X11's symbols.  We modify
      # the flags so that -gui gets linked against the X11 library to support
      # ungif.
      AC_MSG_NOTICE([-gui will be linked against -lX11 to support ungif images])
      GRAPHIC_CFLAGS="-I${ac_x_includes} $GRAPHIC_CFLAGS"
      GRAPHIC_LFLAGS="-L${ac_x_libraries} $GRAPHIC_LFLAGS"
      have_ungif=yes
    fi
  fi
fi

if test "$have_ungif" = no -o "${enable_libgif}" = yes; then
  AC_CHECK_LIB(gif, DGifOpen)

  if test "${with_x}" != no && test "${ac_cv_lib_gif_DGifOpen}" = no; then
    orig_CPPFLAGS="${CPPFLAGS}"
    orig_LDFLAGS="${LDFLAGS}"
    orig_LIBS="${LIBS}"
    AC_MSG_NOTICE([Checking if libgif is linked against -lX11])
    # This implies either that libgif is not installed at all, or that it
    # explicitly refers to the symbols defined in X11.  Now see if the latter
    # is the case.
    CPPFLAGS="${CPPFLAGS} -I${ac_x_includes}"
    LDFLAGS="${LDFLAGS} -L${ac_x_libraries}"
    LIBS="${LIBS} -lX11"

    AC_CHECK_LIB(gif, DGifCloseFile)
    if test "${ac_cv_lib_gif_DGifCloseFile}" = no; then
      # This indicates libgif is not installed at all.  The flags reverts to
      # the orignal value.
      CPPFLAGS="${orig_CPPFLAGS}"
      LDFLAGS="${orig_LDFLAGS}"
      LIBS="${orig_LIBS}"
    else
      # This indicates libgif actually refers to the X11's symbols.  We modify
      # the flags so that -gui gets linked against the X11 library to support
      # libgif.
      AC_MSG_NOTICE([-gui will be linked against -lX11 to support libgif images])
      GRAPHIC_CFLAGS="-I${ac_x_includes} $GRAPHIC_CFLAGS"
      GRAPHIC_LFLAGS="-L${ac_x_libraries} $GRAPHIC_LFLAGS"
    fi
  fi
fi

# QuantizeBuffer was removed from giflib in version 4.2 but was reintroduced
# in the 5.0 release with a new name. If this function is not present
# we disable the support to create gif image representations.
AC_CHECK_FUNCS(QuantizeBuffer)
AC_CHECK_FUNCS(GifQuantizeBuffer)

#--------------------------------------------------------------------
# Find ImageMagick
#--------------------------------------------------------------------

AC_ARG_ENABLE(imagemagick, 
  [  --enable-imagemagick        Enable ImageMagick support])
AS_IF([test "x$enable_imagemagick" = "xyes"], [
   PKG_CHECK_MODULES([IMAGEMAGICK], [MagickCore],
    [
	GRAPHIC_CFLAGS="$IMAGEMAGICK_CFLAGS $GRAPHIC_CFLAGS"
        GRAPHIC_LFLAGS="$IMAGEMAGICK_LIBS $GRAPHIC_LFLAGS"
	AC_DEFINE([HAVE_IMAGEMAGICK], [1], [Have ImageMagick])
    ],
    [AC_DEFINE([HAVE_IMAGEMAGICK], [0], [Don't have ImageMagick])])
])

#--------------------------------------------------------------------
# Check for International Components for Unicode
# See DEPENDENCIES POLICY at the start of thsi file.
#--------------------------------------------------------------------
HAVE_ICU=0
AC_ARG_ENABLE(icu,
  [  --disable-icu		Disable International Components for Unicode],,
  enable_icu=yes)

AC_ARG_ENABLE(icu-config,
  [  --disable-icu-config		Do not use the icu-config script to check
				for ICU.],,
  enable_icu_config=yes)

AC_ARG_WITH(icu-library,
  [  --with-icu-library=PATH  	library path for ICU libraries (only checked
				if not using icu-config)],
  icu_libdir="$withval", icu_libdir="no")

if test $enable_icu = yes; then
  if test "$enable_icu_config" = "yes"; then
    AC_CHECK_ICU(4.0, have_icu=yes, have_icu=no)
  else
    have_icu=no;
    AC_CHECKING([for libicu (icu-config disabled)])
    if test "$icu_libdir" != "no"; then
      ICU_LDFLAGS="-L$icu_libdir";
    fi
    saved_LDFLAGS="$LDFLAGS";
    LDFLAGS="$LDFLAGS $LIBS $ICU_LDFLAGS -licui18n -licuuc -licudata -lm"
    AC_TRY_LINK([],[], have_icu="yes", have_icu="no");
    LDFLAGS="$saved_LDFLAGS";
    ICU_LIBS="-licui18n -licuuc -licudata -lm" 
  fi
    
  if test "$have_icu" = "yes"; then
    AC_MSG_RESULT(yes)
    AC_CHECK_HEADERS(unicode/uchar.h unicode/ustring.h)
    GRAPHIC_LFLAGS="$ICU_LDFLAGS $ICU_LIBS $GRAPHIC_LFLAGS"
    HAVE_ICU=1
  else
    AC_MSG_RESULT(no)
    AC_MSG_WARN([The International Components for Unicode (ICU) development headers and libraries do not appear to be available on this system.])
  fi
fi
AC_SUBST(HAVE_ICU)

#--------------------------------------------------------------------
# Check for the spelling lib, for the built-in spell checker...
# Note: you might want to disable aspell on MacOSX, as it linkes in
# Apple's libobjc.A, screwing up the links to the GNU libobjc.
#--------------------------------------------------------------------
AC_ARG_ENABLE(aspell, 
  [  --disable-aspell        Disable aspell for spellchecker],,
  enable_aspell=yes)
if test "$enable_aspell" = yes; then
  AC_CHECK_LIB(aspell, new_aspell_document_checker)
fi
if test "${ac_cv_lib_aspell_new_aspell_document_checker}" = yes; then
   AC_CHECK_HEADERS(aspell.h)
   AC_DEFINE(HAVE_ASPELL,1,[Define if you have the aspell header])
fi

#--------------------------------------------------------------------
# Check for ICNS library.
#--------------------------------------------------------------------
AC_CHECK_LIB(icns, icns_read_family_from_file)

#--------------------------------------------------------------------
# NSSound
#--------------------------------------------------------------------
AC_ARG_ENABLE(sound, 
  [  --disable-sound          Disable sound],,
  enable_sound=yes)
# Initialize to nothing...
BUILD_SOUND=
# Check for the headers...
AC_CHECK_HEADERS(sndfile.h, have_sndfile=yes, have_sndfile=no)
AC_CHECK_HEADERS(ao/ao.h, have_ao=yes, have_ao=no)
# Only if we have both...
if test $have_sndfile = yes -a $have_ao = yes -a $enable_sound = yes; then
   BUILD_SOUND="sound"
fi
AC_SUBST(BUILD_SOUND)

#--------------------------------------------------------------------
# NSSpeechSynthesizer
#--------------------------------------------------------------------
AC_ARG_ENABLE(speech, 
  [  --disable-speech          Disable speech server],,
  enable_speech=yes)
BUILD_SPEECH=

# has flite, for speech synthesis.
AC_CHECK_LIB(flite, new_utterance, have_speech=yes, have_speech=no)
AC_CHECK_HEADERS(flite/flite.h, have_flite=yes, have_flite=no)
if test $have_flite = yes -a $have_speech = yes -a $enable_speech = yes; then
   BUILD_SPEECH="speech say"
   FLITE_BASE_LIBS="-lflite_usenglish -lflite_cmulex -lflite"
   AC_CHECK_LIB(flite_cmu_us_kal16, register_cmu_us_kal16, have_kal16=yes, have_kal16=no, $FLITE_BASE_LIBS)
   if test $have_kal16 = no; then
      BUILD_SPEECH=
   fi
fi
AC_SUBST(BUILD_SPEECH)

#--------------------------------------------------------------------
# NSSpeechRecognizer
#--------------------------------------------------------------------
AC_ARG_ENABLE(speech-recognizer, 
  [  --disable-speech-recognizer          Disable speech recognition server],,
  enable_speech_recognizer=yes)
BUILD_SPEECH_RECOGNIZER=

# has pocketsphinx, for speech recognition.
AC_CHECK_LIB(pocketsphinx, ps_start_utt, have_speech_recognizer=yes, have_speech_recognizer=no)
AC_CHECK_HEADERS(pocketsphinx/pocketsphinx_export.h, have_pocketsphinx=yes, have_pocketsphinx=no)
if test $have_pocketsphinx = yes -a $have_speech_recognizer = yes -a $enable_speech_recognizer = yes; then
   BUILD_SPEECH_RECOGNIZER="speech_recognizer"
   RECOGNIZER_BASE_LIBS=`pkg-config --libs pocketsphinx sphinxbase`
   RECOGNIZER_BASE_CFLAGS=`pkg-config --cflags pocketsphinx sphinxbase`
   RECOGNIZER_ENGINE_CLASS=PocketsphinxSpeechRecognitionEngine.m
fi
AC_SUBST(BUILD_SPEECH_RECOGNIZER)
AC_SUBST(RECOGNIZER_BASE_LIBS)
AC_SUBST(RECOGNIZER_BASE_CFLAGS)
AC_SUBST(RECOGNIZER_ENGINE_CLASS)

#--------------------------------------------------------------------
# Find CUPS
#--------------------------------------------------------------------
GSCUPS_CFLAGS=
GSCUPS_LDFLAGS=
GSCUPS_LIBS=
GSCUPS_DATADIR=
BUILD_GSCUPS=NO

AC_ARG_ENABLE(cups,
  [  --disable-cups          Disable cups printing support],,
  enable_cups=yes)

if test $enable_cups = yes; then
  BUILD_GSCUPS=YES
  AC_CHECK_PROG(have_cups, cups-config, yes, no)

  if test $have_cups = no; then
    echo "Could not find cups-config program, cups printing support will not be built."
    enable_cups=no
    BUILD_GSCUPS=NO
  else
    GSCUPS_CFLAGS=`cups-config --cflags`
    GSCUPS_LDFLAGS=`cups-config --ldflags`
    GSCUPS_LIBS=`cups-config --libs`
    GSCUPS_DATADIR=`cups-config --datadir`

    orig_CPPFLAGS="${CPPFLAGS}"
    orig_LDFLAGS="${LDFLAGS}"
    CPPFLAGS="$GSCUPS_CFLAGS ${CPPFLAGS}"
    LDFLAGS="$GSCUPS_LDFLAGS ${LDFLAGS}"
    
    AC_CHECK_HEADERS(cups/cups.h, have_cups=yes, have_cups=no)
    if test "$have_cups" = no; then
      enable_cups=no
      BUILD_GSCUPS=NO
      echo "Could not find cups.h, cups printing support will not be built."
    fi
    AC_CHECK_LIB(cups, cupsServer, have_cups=yes, have_cups=no)
    if test "$have_cups" = no; then
      enable_cups=no
      BUILD_GSCUPS=NO
      echo "Could not find the cups library, cups printing support will not be built."
    fi
    CPPFLAGS="${orig_CPPFLAGS}"
    LDFLAGS="${orig_LDFLAGS}"
  fi
fi
AC_SUBST(GSCUPS_CFLAGS)
AC_SUBST(GSCUPS_LDFLAGS)
AC_SUBST(GSCUPS_LIBS)
AC_SUBST(GSCUPS_DATADIR)
AC_SUBST(BUILD_GSCUPS)

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
# Record the version
#--------------------------------------------------------------------
AC_MSG_CHECKING(for the version of gnustep-gui we are compiling)
if test -f "Version"; then
  . ./Version
fi
AC_MSG_RESULT($GNUSTEP_GUI_VERSION)
AC_SUBST(GNUSTEP_GUI_VERSION)
AC_SUBST(GNUSTEP_GUI_MAJOR_VERSION)
AC_SUBST(GNUSTEP_GUI_MINOR_VERSION)
AC_SUBST(GNUSTEP_GUI_SUBMINOR_VERSION)

CONFIG_SYSTEM_INCL="$CONFIG_SYSTEM_INCL $GRAPHIC_CFLAGS"
CONFIG_SYSTEM_LIB_DIR="$CONFIG_SYSTEM_LIB_DIR $GRAPHIC_LFLAGS"

AC_SUBST(CONFIG_SYSTEM_LIB_DIR)
AC_SUBST(CONFIG_SYSTEM_INCL)
AC_SUBST(cross_compiling)

#--------------------------------------------------------------------
# Configure
#--------------------------------------------------------------------
AH_BOTTOM([
/* Define for those who don't have rintf and/or rint */
#ifndef HAVE_RINTF
#define rintf rint
#endif
#ifndef HAVE_RINT
#define DEFINE_RINT_IF_MISSING   \
static double rint(double a)     \
{                                \
  return (floor(a+0.5));         \
}
#else
#define DEFINE_RINT_IF_MISSING
#endif
])

AC_CONFIG_FILES([config.make gui.make Headers/Additions/GNUstepGUI/GSVersion.h])
AC_OUTPUT
