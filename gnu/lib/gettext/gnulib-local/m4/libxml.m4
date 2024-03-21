# libxml.m4 serial 10
dnl Copyright (C) 2006, 2008, 2011, 2013, 2016, 2019-2020 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Bruno Haible.

dnl gl_LIBXML
dnl   gives the user the option to decide whether to use the included or
dnl   an external libxml.
dnl gl_LIBXML(FORCE-INCLUDED)
dnl   forces the use of the included or an external libxml.
AC_DEFUN([gl_LIBXML],
[
  AC_REQUIRE([AM_ICONV_LINK])

  ifelse([$1], , [
    AC_MSG_CHECKING([whether included libxml is requested])
    AC_ARG_WITH([included-libxml],
      [  --with-included-libxml  use the libxml2 included here],
      [gl_cv_libxml_force_included=$withval],
      [gl_cv_libxml_force_included=no])
    AC_MSG_RESULT([$gl_cv_libxml_force_included])
  ], [gl_cv_libxml_force_included=$1])

  gl_cv_libxml_use_included="$gl_cv_libxml_force_included"
  LIBXML=
  LTLIBXML=
  INCXML=
  ifelse([$1], [yes], , [
    if test "$gl_cv_libxml_use_included" != yes; then
      dnl Figure out whether we can use a preinstalled libxml2, or have to use
      dnl the included one.
      AC_CACHE_VAL([gl_cv_libxml], [
        gl_cv_libxml=no
        gl_cv_LIBXML=
        gl_cv_LTLIBXML=
        gl_cv_INCXML=
        gl_save_LIBS="$LIBS"
        LIBS="$LIBS $LIBICONV"
        dnl Search for libxml2 and define LIBXML2, LTLIBXML2 and INCXML2
        dnl accordingly.
        dnl Don't use xml2-config nor pkg-config, since it doesn't work when
        dnl cross-compiling or when the C compiler in use is different from the
        dnl one that built the library.
        dnl Use a test program that tries to invoke xmlFree. On Cygwin 1.7.x,
        dnl libxml2 is built in such a way that uses of xmlFree work fine with
        dnl -Wl,--enable-auto-import but lead to a link error with
        dnl -Wl,--disable-auto-import.
        AC_LIB_LINKFLAGS_BODY([xml2])
        LIBS="$gl_save_LIBS $LIBXML2 $LIBICONV"
        AC_LINK_IFELSE(
          [AC_LANG_PROGRAM(
             [[#include <libxml/xmlversion.h>
               #include <libxml/xmlmemory.h>
               #include <libxml/xpath.h>
             ]],
             [[xmlCheckVersion (0);
               xmlFree ((void *) 0);
               xmlXPathSetContextNode ((void *)0, (void *)0);
             ]])],
          [gl_cv_libxml=yes
           gl_cv_LIBXML="$LIBXML2 $LIBICONV"
           gl_cv_LTLIBXML="$LTLIBXML2 $LTLIBICONV"
          ])
        if test "$gl_cv_libxml" != yes; then
          gl_save_CPPFLAGS="$CPPFLAGS"
          CPPFLAGS="$CPPFLAGS $INCXML2"
          AC_LINK_IFELSE(
            [AC_LANG_PROGRAM(
               [[#include <libxml/xmlversion.h>
                 #include <libxml/xmlmemory.h>
                 #include <libxml/xpath.h>
               ]],
               [[xmlCheckVersion (0);
                 xmlFree ((void *) 0);
                 xmlXPathSetContextNode ((void *)0, (void *)0);
               ]])],
            [gl_cv_libxml=yes
             gl_cv_LIBXML="$LIBXML2 $LIBICONV"
             gl_cv_LTLIBXML="$LTLIBXML2 $LTLIBICONV"
             gl_cv_INCXML="$INCXML2"
            ])
          if test "$gl_cv_libxml" != yes; then
            dnl Often the include files are installed in /usr/include/libxml2.
            dnl In libxml2-2.5, <libxml/xmlversion.h> is self-contained.
            dnl In libxml2-2.6, it includes <libxml/xmlexports.h> which is
            dnl self-contained.
            libxml2_include_dir=
            AC_PREPROC_IFELSE([AC_LANG_SOURCE([[#include <libxml2/libxml/xmlexports.h>]])],
              [gl_ABSOLUTE_HEADER([libxml2/libxml/xmlexports.h])
               libxml2_include_dir=`echo "$gl_cv_absolute_libxml2_libxml_xmlexports_h" | sed -e 's,.libxml.xmlexports\.h$,,'`
              ])
            if test -z "$libxml2_include_dir"; then
              AC_PREPROC_IFELSE([AC_LANG_SOURCE([[#include <libxml2/libxml/xmlversion.h>]])],
                [gl_ABSOLUTE_HEADER([libxml2/libxml/xmlversion.h])
                 libxml2_include_dir=`echo "$gl_cv_absolute_libxml2_libxml_xmlversion_h" | sed -e 's,.libxml.xmlversion\.h$,,'`
                ])
            fi
            if test -n "$libxml2_include_dir" && test -d "$libxml2_include_dir"; then
              CPPFLAGS="$gl_save_CPPFLAGS -I$libxml2_include_dir"
              AC_LINK_IFELSE(
                [AC_LANG_PROGRAM(
                   [[#include <libxml/xmlversion.h>
                     #include <libxml/xmlmemory.h>
                     #include <libxml/xpath.h>
                   ]],
                   [[xmlCheckVersion (0);
                     xmlFree ((void *) 0);
                     xmlXPathSetContextNode ((void *)0, (void *)0);
                   ]])],
                [gl_cv_libxml=yes
                 gl_cv_LIBXML="$LIBXML2 $LIBICONV"
                 gl_cv_LTLIBXML="$LTLIBXML2 $LTLIBICONV"
                 gl_cv_INCXML="-I$libxml2_include_dir"
                ])
            fi
          fi
          CPPFLAGS="$gl_save_CPPFLAGS"
        fi
        LIBS="$gl_save_LIBS"
      ])
      AC_MSG_CHECKING([for libxml])
      AC_MSG_RESULT([$gl_cv_libxml])
      if test $gl_cv_libxml = yes; then
        LIBXML="$gl_cv_LIBXML"
        LTLIBXML="$gl_cv_LTLIBXML"
        INCXML="$gl_cv_INCXML"
      else
        gl_cv_libxml_use_included=yes
      fi
    fi
  ])
  AC_SUBST([LIBXML])
  AC_SUBST([LTLIBXML])
  AC_SUBST([INCXML])
  AC_MSG_CHECKING([whether to use the included libxml])
  AC_MSG_RESULT([$gl_cv_libxml_use_included])

  if test "$gl_cv_libxml_use_included" = yes; then
    LIBXML_H=
    LIBXML_H="$LIBXML_H libxml/DOCBparser.h"
    LIBXML_H="$LIBXML_H libxml/HTMLparser.h"
    LIBXML_H="$LIBXML_H libxml/HTMLtree.h"
    LIBXML_H="$LIBXML_H libxml/SAX2.h"
    LIBXML_H="$LIBXML_H libxml/SAX.h"
    LIBXML_H="$LIBXML_H libxml/c14n.h"
    LIBXML_H="$LIBXML_H libxml/catalog.h"
    LIBXML_H="$LIBXML_H libxml/chvalid.h"
    LIBXML_H="$LIBXML_H libxml/debugXML.h"
    LIBXML_H="$LIBXML_H libxml/dict.h"
    LIBXML_H="$LIBXML_H libxml/encoding.h"
    LIBXML_H="$LIBXML_H libxml/entities.h"
    LIBXML_H="$LIBXML_H libxml/globals.h"
    LIBXML_H="$LIBXML_H libxml/hash.h"
    LIBXML_H="$LIBXML_H libxml/list.h"
    LIBXML_H="$LIBXML_H libxml/nanoftp.h"
    LIBXML_H="$LIBXML_H libxml/nanohttp.h"
    LIBXML_H="$LIBXML_H libxml/parser.h"
    LIBXML_H="$LIBXML_H libxml/parserInternals.h"
    LIBXML_H="$LIBXML_H libxml/pattern.h"
    LIBXML_H="$LIBXML_H libxml/relaxng.h"
    LIBXML_H="$LIBXML_H libxml/schemasInternals.h"
    LIBXML_H="$LIBXML_H libxml/schematron.h"
    LIBXML_H="$LIBXML_H libxml/threads.h"
    LIBXML_H="$LIBXML_H libxml/tree.h"
    LIBXML_H="$LIBXML_H libxml/uri.h"
    LIBXML_H="$LIBXML_H libxml/valid.h"
    LIBXML_H="$LIBXML_H libxml/xinclude.h"
    LIBXML_H="$LIBXML_H libxml/xlink.h"
    LIBXML_H="$LIBXML_H libxml/xmlIO.h"
    LIBXML_H="$LIBXML_H libxml/xmlautomata.h"
    LIBXML_H="$LIBXML_H libxml/xmlerror.h"
    LIBXML_H="$LIBXML_H libxml/xmlexports.h"
    LIBXML_H="$LIBXML_H libxml/xmlmemory.h"
    LIBXML_H="$LIBXML_H libxml/xmlmodule.h"
    LIBXML_H="$LIBXML_H libxml/xmlreader.h"
    LIBXML_H="$LIBXML_H libxml/xmlregexp.h"
    LIBXML_H="$LIBXML_H libxml/xmlsave.h"
    LIBXML_H="$LIBXML_H libxml/xmlschemas.h"
    LIBXML_H="$LIBXML_H libxml/xmlschemastypes.h"
    LIBXML_H="$LIBXML_H libxml/xmlstring.h"
    LIBXML_H="$LIBXML_H libxml/xmlunicode.h"
    LIBXML_H="$LIBXML_H libxml/xmlversion.h"
    LIBXML_H="$LIBXML_H libxml/xmlwriter.h"
    LIBXML_H="$LIBXML_H libxml/xpath.h"
    LIBXML_H="$LIBXML_H libxml/xpathInternals.h"
    LIBXML_H="$LIBXML_H libxml/xpointer.h"
    AC_CHECK_HEADERS([arpa/inet.h ctype.h dlfcn.h dl.h errno.h \
                      fcntl.h float.h limits.h malloc.h math.h netdb.h \
                      netinet/in.h signal.h stdlib.h string.h \
                      strings.h sys/select.h sys/socket.h sys/stat.h \
                      sys/time.h sys/types.h time.h unistd.h])
    AC_CHECK_HEADERS([arpa/nameser.h], [], [], [
      #if HAVE_SYS_TYPES_H
      # include <sys/types.h>
      #endif
    ])
    AC_CHECK_HEADERS([resolv.h], [], [], [
      #if HAVE_SYS_TYPES_H
      # include <sys/types.h>
      #endif
      #if HAVE_NETINET_IN_H
      # include <netinet/in.h>
      #endif 
      #if HAVE_ARPA_NAMESER_H 
      # include <arpa/nameser.h>
      #endif
    ])
    AC_CHECK_FUNCS([getaddrinfo localtime stat strftime])
    dnl This relies on the va_copy replacement from the stdarg module.
    AC_DEFINE([VA_COPY], [va_copy],
      [Define to a working va_copy macro or replacement.])
    dnl Don't bother checking for pthread.h and other multithread facilities.
    dnl Don't bother checking for zlib.h and how to link with libz.
  else
    LIBXML_H=
  fi
  AC_SUBST([LIBXML_H])

  AM_CONDITIONAL([INCLUDED_LIBXML],
    [test "$gl_cv_libxml_use_included" = yes])
])
