## Makefile for the lib subdirectory of GNU libtextstyle.
## Copyright (C) 2009-2023 Free Software Foundation, Inc.
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <https://www.gnu.org/licenses/>.

## Process this file with automake to produce Makefile.in.

AUTOMAKE_OPTIONS = 1.13 gnits subdir-objects no-dependencies
EXTRA_DIST =
BUILT_SOURCES =
MOSTLYCLEANFILES = core *.stackdump
MOSTLYCLEANDIRS =
CLEANFILES =
DISTCLEANFILES =
MAINTAINERCLEANFILES =
SUFFIXES =

lib_LTLIBRARIES = libtextstyle.la

noinst_LTLIBRARIES =

nobase_include_HEADERS = textstyle.h

nobase_nodist_include_HEADERS =

noinst_HEADERS = notinline.h

AM_CPPFLAGS = \
  -I. -I$(srcdir) \
  -I$(top_builddir) -I$(top_srcdir) \
  -Iglib \
  -DIN_LIBTEXTSTYLE

# Parameterization of the 'libxml' module:
# When building a shared library, don't export the variables
# xmlMalloc, xmlMallocAtomic, xmlRealloc, xmlFree, xmlMemStrdup.
if INCLUDED_LIBXML
AM_CPPFLAGS += -DLIBXML_STATIC
endif

if WOE32
# On mingw, disable the declarations of *printf functions as aliases to the
# corresponding __mingw_*printf functions, because they pull in a dependency
# to the libgcc_s_sjlj DLL (through the symbols __udivdi3, __umoddi3).
AM_CPPFLAGS += -D__USE_MINGW_ANSI_STDIO=0
endif

# Enable more warning options in this directory.
AM_CFLAGS = @WARN_CFLAGS@

# Rules generated and collected by gnulib-tool.
include Makefile.gnulib

# Additional source files.
libtextstyle_la_SOURCES += \
  color.h color.c \
  misc.h misc.c \
  version.c

# The <stdbool.h> replacement that can be installed.
nobase_nodist_include_HEADERS += \
  textstyle/stdbool.h

# Produce a textstyle/stdbool.h that is not compiler dependent.
# GCC >= 2.95 has <stdbool.h>.
# AIX >= 5.3 has <stdbool.h>.
# Solaris 10 and some HP-UX 11 versions have <stdbool.h> but it does not
# necessarily work.
textstyle/stdbool.h : $(STDBOOL_H) stdbool.mini.h
	@MKDIR_P@ textstyle
	rm -f $@-t $@
	{ echo '/* DO NOT EDIT! GENERATED AUTOMATICALLY! */'; \
	  echo '#if !defined _GL_STDBOOL_H'; \
	  if test -f /usr/include/stdbool.h; then \
	    echo '#if (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95)) || defined _AIX'; \
	  else \
	    echo '#if (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95))'; \
	  fi; \
	  echo '#include <stdbool.h>'; \
	  echo '#else'; \
	  cat $(srcdir)/stdbool.mini.h; \
	  echo '#endif'; \
	  echo '#endif'; \
	} > $@-t
	mv $@-t $@
BUILT_SOURCES    += textstyle/stdbool.h
MOSTLYCLEANFILES += textstyle/stdbool.h-t
CLEANFILES       += textstyle/stdbool.h
EXTRA_DIST       += stdbool.mini.h

# textstyle/version.h is public.
nobase_nodist_include_HEADERS += textstyle/version.h

# textstyle/woe32dll.h is not public, but is included by other header files.
nobase_nodist_include_HEADERS += textstyle/woe32dll.h

# Directories that contain some CLEANFILES.
CLEANDIRS =
CLEANDIRS_NOT_IN_SRCDIR =
clean-local: clean-generic
	@for dir in '' $(CLEANDIRS); do \
	  if test -n "$$dir" && test -d $$dir; then \
	    echo "rmdir $$dir"; rmdir $$dir; \
	  fi; \
	done; \
	if test '$(srcdir)' != '.'; then \
	  for dir in '' $(CLEANDIRS_NOT_IN_SRCDIR); do \
	    if test -n "$$dir" && test -d $$dir; then \
	      echo "rmdir $$dir"; rmdir $$dir; \
	    fi; \
	  done; \
	fi; \
        :

# Directories that contain some DISTCLEANFILES.
DISTCLEANDIRS = sys
DISTCLEANDIRS_NOT_IN_SRCDIR = textstyle
distclean-local: distclean-generic
	@for dir in '' $(DISTCLEANDIRS); do \
	  if test -n "$$dir" && test -d $$dir; then \
	    echo "rmdir $$dir"; rmdir $$dir; \
	  fi; \
	done; \
	if test '$(srcdir)' != '.'; then \
	  for dir in '' $(DISTCLEANDIRS_NOT_IN_SRCDIR); do \
	    if test -n "$$dir" && test -d $$dir; then \
	      echo "rmdir $$dir"; rmdir $$dir; \
	    fi; \
	  done; \
	fi; \
        :

# List of header files that get installed and that declare 'extern' symbols.
HEADERS_WITH_EXTERNS = \
  textstyle.h \
  textstyle/version.in.h

# List of declared exported symbols.
# We extract it from the header files that get installed.
# This file has the same format as the one expected by the libtool option
# '-export-symbols'.
$(srcdir)/libtextstyle.sym.in : $(HEADERS_WITH_EXTERNS)
	if \
	  for f in $(HEADERS_WITH_EXTERNS); do \
	    if test -f $$f; then \
	      cat $$f; \
	    else \
	      cat $(srcdir)/$$f; \
	    fi; \
	  done \
	  | $(srcdir)/declared.sh > $@-t1; \
	then \
	  LC_ALL=C sort < $@-t1 | LC_ALL=C uniq > $@-t2 && mv $@-t2 $@; \
	else \
	  if test -f $@; then \
	    echo "Continuing with existing libtextstyle.sym.in."; \
	  else \
	    exit 1; \
	  fi; \
	fi
# We distribute it because declared.sh relies on GNU sed.
# The GNU Coding Standards say in
# <https://www.gnu.org/prep/standards/html_node/Makefile-Basics.html>:
#   "GNU distributions usually contain some files which are not source files
#    ... . Since these files normally appear in the source directory, they
#    should always appear in the source directory, not in the build directory.
#    So Makefile rules to update them should put the updated files in the
#    source directory."
# Therefore we put this file in the source directory, not the build directory.
MOSTLYCLEANFILES     += libtextstyle.sym.in-t1 libtextstyle.sym.in-t2
MAINTAINERCLEANFILES += libtextstyle.sym.in
EXTRA_DIST           += libtextstyle.sym.in declared.sh

# List of exported symbols (platform dependent).
libtextstyle.sym : $(srcdir)/libtextstyle.sym.in
	cat $(srcdir)/libtextstyle.sym.in \
	  | case "@host_os@" in \
	      mingw*) cat ;; \
	      *) grep -v '^libtextstyle_isatty$$' ;; \
	    esac \
	  > $@-t
	mv $@-t $@
MOSTLYCLEANFILES += libtextstyle.sym libtextstyle.sym-t

# Hide undesired symbols that are defined by libtextstyle_la_SOURCES or
# gl_LIBOBJS or the dependency libraries from the global namespace,
# by prefixing them with "libtextstyle_".
all check install: config.h
COMPILATION_UNITS = $(libtextstyle_la_SOURCES) $(gl_LIBOBJS)
if INCLUDED_LIBCROCO
COMPILATION_UNITS += $(libcroco_rpl_la_SOURCES)
endif
if INCLUDED_LIBXML
COMPILATION_UNITS += $(libxml_rpl_la_SOURCES)
endif
if INCLUDED_LIBGLIB
COMPILATION_UNITS += $(libglib_rpl_la_SOURCES)
endif
config.h: $(BUILT_SOURCES) libtextstyle.sym
	{ echo '/* DO NOT EDIT! GENERATED AUTOMATICALLY! */'; \
	  : "Avoid double inclusion, to avoid a warning about redefinitions."; \
	  echo '#ifndef LIBTEXTSTYLE_CONFIG_H'; \
	  echo '#define LIBTEXTSTYLE_CONFIG_H'; \
	  echo; \
	  echo '#include "../config.h"'; \
	  echo; \
	  echo '#endif /* LIBTEXTSTYLE_CONFIG_H */'; \
	  case "@host_os@" in \
	    mingw*) echo '#define isatty libtextstyle_isatty' ;; \
	  esac; \
	} > config.h && \
	if test -n "$(NAMESPACING)" && test -n "$(HAVE_GLOBAL_SYMBOL_PIPE)"; then \
	  { \
	    { \
	      for f in $(COMPILATION_UNITS); do \
	        case $$f in \
	          *.res.lo ) ;; \
	          *.c | *.$(OBJEXT) | *.lo ) \
	            sf=`echo "$$f" | sed -e 's,\\.[^.]*$$,,'`.c; \
	            test -f $$sf || sf=$(srcdir)/$$sf; \
	            of=`echo "$$f" | sed -e 's,^.*/,,' -e 's,\\.[^.]*$$,,'`.$(OBJEXT); \
	            echo "$(COMPILE) $(GL_CFLAG_INHIBIT_WARNINGS) -c $$sf" 1>&6; \
	            $(COMPILE) $(GL_CFLAG_INHIBIT_WARNINGS) -c $$sf || { rm -f config.h; exit 1; }; \
	            sh ./exported.sh $$of 1>&5; \
	            rm -f $$of `echo "$$of" | sed -e 's,\\.$(OBJEXT)$$,.lo,'`; \
	            ;; \
	        esac; \
	      done; \
	    } 5>&1 1>&2 \
	      | sed -e 's,.* ,,' | grep -v '@' | LC_ALL=C sort | LC_ALL=C uniq \
	      | LC_ALL=C $(top_srcdir)/build-aux/join-v-1 - libtextstyle.sym \
	      | sed -e 's,^\(.*\)$$,#define \1 libtextstyle_\1,' > config.h-t; \
	  } 6>&1 && \
	  if test -f config.h; then \
	    cat config.h-t >> config.h; \
	    rm -f config.h-t; \
	  else \
	    rm -f config.h-t; \
	    exit 1; \
	  fi \
	fi
MOSTLYCLEANFILES += config.h config.h-t

# Version information according to Woe32 conventions.
EXTRA_DIST += libtextstyle.rc
if WOE32
WOE32_LIBADD = libtextstyle.res.lo
# This rule is executed only on Woe32 systems.
# Use $(RC) with libtool, $(WINDRES) when not using libtool.
# The following sed expressions come from the windres-options script. They are
# inlined here, so that they can be written in a Makefile without requiring a
# temporary file. They must contain literal newlines rather than semicolons,
# so that they work with the sed-3.02 that is shipped with MSYS.
libtextstyle.res.lo: $(srcdir)/libtextstyle.rc
	nlinit=`echo 'nl="'; echo '"'`; eval "$$nlinit"; \
	sed_extract_major='/^[0-9]/{'$${nl}'s/^\([0-9]*\).*/\1/p'$${nl}q$${nl}'}'$${nl}'c\'$${nl}0$${nl}q; \
	sed_extract_minor='/^[0-9][0-9]*[.][0-9]/{'$${nl}'s/^[0-9]*[.]\([0-9]*\).*/\1/p'$${nl}q$${nl}'}'$${nl}'c\'$${nl}0$${nl}q; \
	sed_extract_subminor='/^[0-9][0-9]*[.][0-9][0-9]*[.][0-9]/{'$${nl}'s/^[0-9]*[.][0-9]*[.]\([0-9]*\).*/\1/p'$${nl}q$${nl}'}'$${nl}'c\'$${nl}0$${nl}q; \
	$(LIBTOOL) --tag=RC $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=compile $(RC) \
	  "-DPACKAGE_VERSION_STRING=\\\"$(VERSION)\\\"" \
	  "-DPACKAGE_VERSION_MAJOR="`echo '$(VERSION)' | sed -n -e "$$sed_extract_major"` \
	  "-DPACKAGE_VERSION_MINOR="`echo '$(VERSION)' | sed -n -e "$$sed_extract_minor"` \
	  "-DPACKAGE_VERSION_SUBMINOR="`echo '$(VERSION)' | sed -n -e "$$sed_extract_subminor"` \
	  -i $(srcdir)/libtextstyle.rc -o libtextstyle.res.lo --output-format=coff
MOSTLYCLEANFILES += libtextstyle.res.lo
else
WOE32_LIBADD =
endif
libtextstyle_la_LIBADD       += $(WOE32_LIBADD)
libtextstyle_la_DEPENDENCIES += $(WOE32_LIBADD)

# Parametrization of the 'relocatable-lib-lgpl' module.
AM_CPPFLAGS += -DDEPENDS_ON_LIBICONV=1

# Libtool's library version information for libtextstyle.
# See the libtool documentation, section "Library interface versions".
LTV_CURRENT=2
LTV_REVISION=1
LTV_AGE=2

# How to build libtextstyle.la.
libtextstyle_la_LDFLAGS += \
  -no-undefined \
  -export-symbols libtextstyle.sym \
  -version-info $(LTV_CURRENT):$(LTV_REVISION):$(LTV_AGE) \
  -rpath $(libdir)
