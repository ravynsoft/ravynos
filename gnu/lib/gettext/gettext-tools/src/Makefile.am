## Makefile for the gettext-tools/src subdirectory of GNU gettext
## Copyright (C) 1995-1998, 2000-2023 Free Software Foundation, Inc.
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

AUTOMAKE_OPTIONS = 1.5 gnits no-dependencies subdir-objects
EXTRA_DIST =
BUILT_SOURCES =
MOSTLYCLEANFILES = core *.stackdump
CLEANFILES =
DISTCLEANFILES =
MAINTAINERCLEANFILES =

RM = rm -f

bin_PROGRAMS = \
msgcmp msgfmt msgmerge msgunfmt xgettext \
msgattrib msgcat msgcomm msgconv msgen msgexec msgfilter msggrep msginit msguniq \
recode-sr-latin

noinst_PROGRAMS = hostname urlget cldr-plurals

if INSTALL_PRIVATE_LIBRARIES
# Specify that libgettextsrc should be installed in $(libdir).
lib_LTLIBRARIES = libgettextsrc.la
else
# Specify that libgettextsrc should not be installed.
noinst_LTLIBRARIES = libgettextsrc.la
endif

noinst_HEADERS = \
  pos.h message.h po-error.h po-xerror.h po-gram.h po-charset.h \
  po-lex.h open-catalog.h read-catalog-abstract.h read-catalog.h \
  read-po.h read-properties.h read-stringtable.h \
  str-list.h \
  write-catalog.h write-po.h write-properties.h write-stringtable.h \
  dir-list.h file-list.h po-gram-gen.h po-gram-gen2.h cldr-plural.h \
  cldr-plural-exp.h locating-rule.h its.h search-path.h \
  msgl-charset.h msgl-equal.h msgl-iconv.h msgl-ascii.h msgl-ofn.h msgl-cat.h \
  msgl-header.h msgl-english.h msgl-check.h msgl-fsearch.h msgfmt.h msgunfmt.h \
  plural-count.h plural-eval.h plural-distrib.h \
  read-mo.h write-mo.h \
  read-java.h write-java.h \
  read-csharp.h write-csharp.h \
  read-resources.h write-resources.h \
  read-tcl.h write-tcl.h \
  write-qt.h \
  read-desktop.h write-desktop.h \
  write-xml.h \
  po-time.h plural-table.h lang-table.h format.h filters.h \
  xgettext.h \
  rc-str-list.h xg-pos.h xg-encoding.h xg-mixed-string.h xg-arglist-context.h \
  xg-arglist-callshape.h xg-arglist-parser.h xg-message.h \
  x-po.h x-properties.h x-stringtable.h \
  x-c.h \
  x-python.h \
  x-java.h \
  x-csharp.h \
  x-javascript.h \
  x-scheme.h \
  x-lisp.h \
  x-elisp.h \
  x-librep.h \
  x-ruby.h \
  x-sh.h \
  x-awk.h \
  x-lua.h \
  x-smalltalk.h \
  x-vala.h \
  x-tcl.h \
  x-perl.h \
  x-php.h \
  x-ycp.h \
  x-rst.h \
  x-desktop.h \
  x-glade.h x-gsettings.h x-appdata.h

EXTRA_DIST += FILES project-id

aliaspath = $(localedir)
jardir = $(datadir)/gettext
pkgdatadir = $(datadir)/gettext
projectsdir = $(pkgdatadir)/projects
pkglibdir = $(libdir)/gettext

AM_CPPFLAGS = \
  -I. -I$(srcdir) \
  -I.. -I$(top_srcdir) \
  -I$(top_srcdir)/libgrep \
  -I../gnulib-lib -I$(top_srcdir)/gnulib-lib \
  -I../../gettext-runtime/intl -I$(top_srcdir)/../gettext-runtime/intl
DEFS = \
  -DLOCALEDIR=$(localedir_c_make) -DBISON_LOCALEDIR=\"$(BISON_LOCALEDIR)\" \
  -DLOCALE_ALIAS_PATH=\"$(aliaspath)\" \
  -DUSEJAVA=$(USEJAVA) \
  -DGETTEXTJAR=\"$(jardir)/gettext.jar\" \
  -DLIBDIR=\"$(libdir)\" \
  -DGETTEXTDATADIR=\"$(pkgdatadir)\" \
  -DPROJECTSDIR=\"$(projectsdir)\" \
  -DEXEEXT=\"@EXEEXT@\" @DEFS@

# Enable more warning options in this directory.
AM_CFLAGS = @WARN_CFLAGS@

# Ensure that <stdint.h> defines SIZE_MAX in C++ mode, like it does in C mode.
AM_CXXFLAGS = -D__STDC_LIMIT_MACROS

LDADD = ../gnulib-lib/libgettextlib.la $(LTLIBUNISTRING) @LTLIBINTL@ $(WOE32_LDADD)
OTHERPROGDEPENDENCIES = ../gnulib-lib/libgettextlib.la $(WOE32_LDADD)

SED = sed
BISON = @TOOLS_BISON@
JAR = @JAR@
JAVACOMP = $(SHELL) ../javacomp.sh
CSHARPCOMP = $(SHELL) ../csharpcomp.sh
CSHARPCOMPFLAGS = @CSHARPCOMPFLAGS@


# All programs deal with message lists.
# All programs must read PO files. (msgunfmt also, for read-java.c,
# read-csharp.c and read-resources.c.)
# message.c -> str-list.c.
# (read-catalog-abstract.c <--> po-gram-gen.y <--> po-lex.c) -> str-list.c.
# (read-catalog-abstract.c <--> po-gram-gen.y <--> po-lex.c) -> open-catalog.c -> dir-list.c -> str-list.c.
# (read-catalog-abstract.c <--> po-gram-gen.y <--> po-lex.c) -> po-charset.c.
# (read-catalog-abstract.c <--> po-gram-gen.y <--> po-lex.c) -> message.c -> str-list.c.
COMMON_SOURCE = message.c pos.c po-error.c po-xerror.c \
read-catalog-abstract.c po-lex.c po-gram-gen.y po-charset.c \
read-po.c read-properties.c read-stringtable.c open-catalog.c \
dir-list.c str-list.c

# xgettext and msgfmt deal with format strings.
if !WOE32DLL
FORMAT_SOURCE = format.c
else
FORMAT_SOURCE = ../woe32dll/c++format.cc
endif
FORMAT_SOURCE += \
  format-invalid.h \
  format-c.c format-c-parse.h \
  format-c++-brace.c \
  format-python.c \
  format-python-brace.c \
  format-java.c \
  format-java-printf.c \
  format-csharp.c \
  format-javascript.c \
  format-scheme.c \
  format-lisp.c \
  format-elisp.c \
  format-librep.c \
  format-ruby.c \
  format-sh.c \
  format-awk.c \
  format-lua.c \
  format-pascal.c \
  format-smalltalk.c \
  format-qt.c \
  format-qt-plural.c \
  format-kde.c \
  format-kde-kuit.c \
  format-boost.c \
  format-tcl.c \
  format-perl.c \
  format-perl-brace.c \
  format-php.c \
  format-gcc-internal.c \
  format-gfc-internal.c

# libgettextsrc contains all code that is needed by at least two programs.
libgettextsrc_la_SOURCES = \
  $(COMMON_SOURCE) \
  read-catalog.c \
  write-catalog.c write-properties.c write-stringtable.c write-po.c \
  msgl-ascii.c \
  msgl-ofn.c \
  msgl-iconv.c \
  msgl-equal.c \
  msgl-cat.c \
  msgl-header.c \
  msgl-english.c \
  msgl-check.c \
  file-list.c \
  msgl-charset.c \
  po-time.c \
  plural-exp.c plural-eval.c plural-table.c \
  quote.h \
  sentence.h sentence.c \
  $(FORMAT_SOURCE) \
  read-desktop.c \
  locating-rule.c its.c \
  search-path.c

# msggrep needs pattern matching.
LIBGREP = ../libgrep/libgrep.a

# Source dependencies.
msgcmp_SOURCES = msgcmp.c
msgcmp_SOURCES += msgl-fsearch.c
msgfmt_SOURCES = msgfmt.c
msgfmt_SOURCES += \
  write-mo.c write-java.c write-csharp.c write-resources.c write-tcl.c \
  write-qt.c write-desktop.c write-xml.c \
  ../../gettext-runtime/intl/hash-string.c
if !WOE32DLL
msgmerge_SOURCES = msgmerge.c
else
msgmerge_SOURCES = ../woe32dll/c++msgmerge.cc
endif
msgmerge_SOURCES += msgl-fsearch.c lang-table.c plural-count.c
msgunfmt_SOURCES = msgunfmt.c
msgunfmt_SOURCES += \
  read-mo.c read-java.c read-csharp.c read-resources.c read-tcl.c \
  ../../gettext-runtime/intl/hash-string.c
if !WOE32DLL
xgettext_SOURCES = xgettext.c
else
xgettext_SOURCES = ../woe32dll/c++xgettext.cc
endif
xgettext_SOURCES += \
  xg-pos.c \
  xg-encoding.c \
  xg-mixed-string.c \
  xg-arglist-context.c xg-arglist-callshape.c xg-arglist-parser.c \
  xg-message.c \
  x-po.c \
  x-c.c \
  x-python.c \
  x-java.c \
  x-csharp.c \
  x-javascript.c \
  x-scheme.c \
  x-lisp.c \
  x-elisp.c \
  x-librep.c \
  x-ruby.c \
  x-sh.c ../../gettext-runtime/src/escapes.h \
  x-awk.c \
  x-lua.c \
  x-smalltalk.c \
  x-vala.c \
  x-tcl.c \
  x-perl.c \
  x-php.c \
  x-ycp.c \
  x-rst.c \
  x-desktop.c
if !WOE32DLL
msgattrib_SOURCES = msgattrib.c
else
msgattrib_SOURCES = ../woe32dll/c++msgattrib.cc
endif
if !WOE32DLL
msgcat_SOURCES = msgcat.c
else
msgcat_SOURCES = ../woe32dll/c++msgcat.cc
endif
if !WOE32DLL
msgcomm_SOURCES = msgcomm.c
else
msgcomm_SOURCES = ../woe32dll/c++msgcomm.cc
endif
if !WOE32DLL
msgconv_SOURCES = msgconv.c
else
msgconv_SOURCES = ../woe32dll/c++msgconv.cc
endif
if !WOE32DLL
msgen_SOURCES = msgen.c
else
msgen_SOURCES = ../woe32dll/c++msgen.cc
endif
msgexec_SOURCES = msgexec.c
if !WOE32DLL
msgfilter_SOURCES = msgfilter.c
else
msgfilter_SOURCES = ../woe32dll/c++msgfilter.cc
endif
msgfilter_SOURCES += filter-sr-latin.c
msgfilter_SOURCES += filter-quote.c
if !WOE32DLL
msggrep_SOURCES = msggrep.c
else
msggrep_SOURCES = ../woe32dll/c++msggrep.cc
endif
msginit_SOURCES = msginit.c
msginit_SOURCES += lang-table.c plural-count.c
msginit_SOURCES += ../../gettext-runtime/intl/localealias.c
if !WOE32DLL
msguniq_SOURCES = msguniq.c
else
msguniq_SOURCES = ../woe32dll/c++msguniq.cc
endif
recode_sr_latin_SOURCES = recode-sr-latin.c filter-sr-latin.c
hostname_SOURCES = hostname.c
urlget_SOURCES = urlget.c
cldr_plurals_SOURCES = cldr-plural.y cldr-plural-exp.c cldr-plurals.c
cldr_plurals_CFLAGS = $(AM_CFLAGS) $(INCXML)
cldr_plurals_LDADD = libgettextsrc.la $(LDADD)

if USE_INSTALLED_LIBTEXTSTYLE
LT_LIBTEXTSTYLE = @LTLIBTEXTSTYLE@
else
# How to get the include files of libtextstyle.
textstyle.h textstyle/stdbool.h textstyle/version.h textstyle/woe32dll.h:
	here=`pwd`; \
	cd ../../libtextstyle/lib && \
	  $(MAKE) install-nobase_includeHEADERS install-nobase_nodist_includeHEADERS includedir="$$here"
BUILT_SOURCES    += textstyle.h textstyle/stdbool.h textstyle/version.h textstyle/woe32dll.h
MOSTLYCLEANFILES += textstyle.h textstyle/stdbool.h textstyle/version.h textstyle/woe32dll.h
# Where to find the built libtextstyle library.
LT_LIBTEXTSTYLE = ../../libtextstyle/lib/libtextstyle.la
endif

# How to build libgettextsrc.la.
# Need ../gnulib-lib/libgettextlib.la.
# Need $(LTLIBUNISTRING) because ulc_width_linebreaks, uc_width, etc. may be
# taken from libunistring, when the configure option --with-libunistring-prefix
# was given.
# Need @LTLIBINTL@ because many source files use gettext().
# Need @LTLIBICONV@ because po-charset.c, po-lex.c, msgl-iconv.c, write-po.c
# use iconv().
libgettextsrc_la_LDFLAGS = \
  -release @VERSION@ \
  ../gnulib-lib/libgettextlib.la $(LTLIBUNISTRING) $(LT_LIBTEXTSTYLE) @LTLIBINTL@ @LTLIBICONV@ -lc -no-undefined

# OS/2 does not support a DLL name longer than 8 characters.
if OS2
libgettextsrc_la_LDFLAGS += -os2dllname gtsrc
endif

libgettextsrc_la_CPPFLAGS = $(AM_CPPFLAGS) $(INCXML)

# Tell the mingw or Cygwin linker which symbols to export.
if WOE32DLL
libgettextsrc_la_SOURCES += ../woe32dll/gettextsrc-exports.c
libgettextsrc_la_LDFLAGS += -Wl,--export-all-symbols
libgettextsrc_la_CPPFLAGS += $(GETTEXTLIB_EXPORTS_FLAGS)
endif

# Specify installation directory, for --enable-relocatable.
if RELOCATABLE_VIA_LD
# This is needed, because libgettextsrc depends on libgettextlib and libintl.
libgettextsrc_la_LDFLAGS += `$(RELOCATABLE_LDFLAGS) $(libdir)`
endif

# Compile-time flags for particular source files.
msgmerge_CFLAGS = $(AM_CFLAGS) $(OPENMP_CFLAGS)
msgmerge_CXXFLAGS = $(AM_CXXFLAGS) $(OPENMP_CFLAGS)

# Link dependencies.
# INTL_MACOSX_LIBS is needed because the programs depend on libintl.la
# but libtool doesn't put -Wl,-framework options into .la files.
# For msginit, it is also needed because of localename.c.
msgcmp_LDADD = libgettextsrc.la @INTL_MACOSX_LIBS@ @MSGMERGE_LIBM@ $(WOE32_LDADD)
msgfmt_LDADD = libgettextsrc.la @INTL_MACOSX_LIBS@ $(WOE32_LDADD)
msgmerge_LDADD = libgettextsrc.la @INTL_MACOSX_LIBS@ @MSGMERGE_LIBM@ $(WOE32_LDADD) $(OPENMP_CFLAGS)
msgunfmt_LDADD = libgettextsrc.la @INTL_MACOSX_LIBS@ $(WOE32_LDADD)
xgettext_LDADD = libgettextsrc.la @INTL_MACOSX_LIBS@ @LTLIBICONV@ $(WOE32_LDADD)
msgattrib_LDADD = libgettextsrc.la @INTL_MACOSX_LIBS@ $(WOE32_LDADD)
msgcat_LDADD = libgettextsrc.la @INTL_MACOSX_LIBS@ $(WOE32_LDADD)
msgcomm_LDADD = libgettextsrc.la @INTL_MACOSX_LIBS@ $(WOE32_LDADD)
msgconv_LDADD = libgettextsrc.la @INTL_MACOSX_LIBS@ $(WOE32_LDADD)
msgen_LDADD = libgettextsrc.la @INTL_MACOSX_LIBS@ $(WOE32_LDADD)
msgexec_LDADD = libgettextsrc.la @INTL_MACOSX_LIBS@ $(WOE32_LDADD)
msgfilter_LDADD = libgettextsrc.la @INTL_MACOSX_LIBS@ $(WOE32_LDADD)
msggrep_LDADD = $(LIBGREP) libgettextsrc.la @INTL_MACOSX_LIBS@ $(WOE32_LDADD)
msginit_LDADD = libgettextsrc.la @INTL_MACOSX_LIBS@ $(WOE32_LDADD)
msguniq_LDADD = libgettextsrc.la @INTL_MACOSX_LIBS@ $(WOE32_LDADD)
hostname_LDADD = $(LDADD) $(GETADDRINFO_LIB)

# Specify when to relink the programs.
msgcmp_DEPENDENCIES = libgettextsrc.la ../gnulib-lib/libgettextlib.la $(WOE32_LDADD)
msgfmt_DEPENDENCIES = libgettextsrc.la ../gnulib-lib/libgettextlib.la $(WOE32_LDADD)
msgmerge_DEPENDENCIES = libgettextsrc.la ../gnulib-lib/libgettextlib.la $(WOE32_LDADD)
msgunfmt_DEPENDENCIES = libgettextsrc.la ../gnulib-lib/libgettextlib.la $(WOE32_LDADD)
xgettext_DEPENDENCIES = libgettextsrc.la ../gnulib-lib/libgettextlib.la $(WOE32_LDADD)
msgattrib_DEPENDENCIES = libgettextsrc.la ../gnulib-lib/libgettextlib.la $(WOE32_LDADD)
msgcat_DEPENDENCIES = libgettextsrc.la ../gnulib-lib/libgettextlib.la $(WOE32_LDADD)
msgcomm_DEPENDENCIES = libgettextsrc.la ../gnulib-lib/libgettextlib.la $(WOE32_LDADD)
msgconv_DEPENDENCIES = libgettextsrc.la ../gnulib-lib/libgettextlib.la $(WOE32_LDADD)
msgen_DEPENDENCIES = libgettextsrc.la ../gnulib-lib/libgettextlib.la $(WOE32_LDADD)
msgexec_DEPENDENCIES = libgettextsrc.la ../gnulib-lib/libgettextlib.la $(WOE32_LDADD)
msgfilter_DEPENDENCIES = libgettextsrc.la ../gnulib-lib/libgettextlib.la $(WOE32_LDADD)
msggrep_DEPENDENCIES = $(LIBGREP) libgettextsrc.la ../gnulib-lib/libgettextlib.la $(WOE32_LDADD)
msginit_DEPENDENCIES = libgettextsrc.la ../gnulib-lib/libgettextlib.la $(WOE32_LDADD)
msguniq_DEPENDENCIES = libgettextsrc.la ../gnulib-lib/libgettextlib.la $(WOE32_LDADD)
recode_sr_latin_DEPENDENCIES = $(OTHERPROGDEPENDENCIES)
hostname_DEPENDENCIES = $(OTHERPROGDEPENDENCIES)
urlget_DEPENDENCIES = $(OTHERPROGDEPENDENCIES)

# Specify installation directory, for --enable-relocatable.
msgcmp_CPPFLAGS = $(AM_CPPFLAGS) -DINSTALLDIR=\"$(bindir)\"
msgfmt_CPPFLAGS = $(AM_CPPFLAGS) -DINSTALLDIR=\"$(bindir)\"
msgmerge_CPPFLAGS = $(AM_CPPFLAGS) -DINSTALLDIR=\"$(bindir)\"
msgunfmt_CPPFLAGS = $(AM_CPPFLAGS) -DINSTALLDIR=\"$(bindir)\"
xgettext_CPPFLAGS = $(AM_CPPFLAGS) -DINSTALLDIR=\"$(bindir)\"
msgattrib_CPPFLAGS = $(AM_CPPFLAGS) -DINSTALLDIR=\"$(bindir)\"
msgcat_CPPFLAGS = $(AM_CPPFLAGS) -DINSTALLDIR=\"$(bindir)\"
msgcomm_CPPFLAGS = $(AM_CPPFLAGS) -DINSTALLDIR=\"$(bindir)\"
msgconv_CPPFLAGS = $(AM_CPPFLAGS) -DINSTALLDIR=\"$(bindir)\"
msgen_CPPFLAGS = $(AM_CPPFLAGS) -DINSTALLDIR=\"$(bindir)\"
msgexec_CPPFLAGS = $(AM_CPPFLAGS) -DINSTALLDIR=\"$(bindir)\"
msgfilter_CPPFLAGS = $(AM_CPPFLAGS) -DINSTALLDIR=\"$(bindir)\"
msggrep_CPPFLAGS = $(AM_CPPFLAGS) -DINSTALLDIR=\"$(bindir)\"
msginit_CPPFLAGS = $(AM_CPPFLAGS) -DINSTALLDIR=\"$(bindir)\"
msguniq_CPPFLAGS = $(AM_CPPFLAGS) -DINSTALLDIR=\"$(bindir)\"
recode_sr_latin_CPPFLAGS = $(AM_CPPFLAGS) -DINSTALLDIR=\"$(bindir)\"
hostname_CPPFLAGS = $(AM_CPPFLAGS) -DINSTALLDIR=\"$(pkglibdir)\"
urlget_CPPFLAGS = $(AM_CPPFLAGS) -DINSTALLDIR=\"$(pkglibdir)\"
cldr_plurals_CPPFLAGS = $(AM_CPPFLAGS) -DINSTALLDIR=\"$(pkglibdir)\"
if RELOCATABLE_VIA_LD
msgcmp_LDFLAGS = `$(RELOCATABLE_LDFLAGS) $(bindir)`
msgfmt_LDFLAGS = `$(RELOCATABLE_LDFLAGS) $(bindir)`
msgmerge_LDFLAGS = `$(RELOCATABLE_LDFLAGS) $(bindir)`
msgunfmt_LDFLAGS = `$(RELOCATABLE_LDFLAGS) $(bindir)`
xgettext_LDFLAGS = `$(RELOCATABLE_LDFLAGS) $(bindir)`
msgattrib_LDFLAGS = `$(RELOCATABLE_LDFLAGS) $(bindir)`
msgcat_LDFLAGS = `$(RELOCATABLE_LDFLAGS) $(bindir)`
msgcomm_LDFLAGS = `$(RELOCATABLE_LDFLAGS) $(bindir)`
msgconv_LDFLAGS = `$(RELOCATABLE_LDFLAGS) $(bindir)`
msgen_LDFLAGS = `$(RELOCATABLE_LDFLAGS) $(bindir)`
msgexec_LDFLAGS = `$(RELOCATABLE_LDFLAGS) $(bindir)`
msgfilter_LDFLAGS = `$(RELOCATABLE_LDFLAGS) $(bindir)`
msggrep_LDFLAGS = `$(RELOCATABLE_LDFLAGS) $(bindir)`
msginit_LDFLAGS = `$(RELOCATABLE_LDFLAGS) $(bindir)`
msguniq_LDFLAGS = `$(RELOCATABLE_LDFLAGS) $(bindir)`
recode_sr_latin_LDFLAGS = `$(RELOCATABLE_LDFLAGS) $(bindir)`
hostname_LDFLAGS = `$(RELOCATABLE_LDFLAGS) $(pkglibdir)`
urlget_LDFLAGS = `$(RELOCATABLE_LDFLAGS) $(pkglibdir)`
cldr_plurals_LDFLAGS = `$(RELOCATABLE_LDFLAGS) $(pkglibdir)`
endif

# Linking with C++ libraries is needed _only_ on mingw and Cygwin.
if !WOE32DLL
libgettextsrc_la_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link \
	$(CCLD) $(AM_CFLAGS) $(CFLAGS) $(libgettextsrc_la_LDFLAGS) $(LDFLAGS) \
	-o $@
msgattrib_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link \
	$(CCLD) $(AM_CFLAGS) $(CFLAGS) $(msgattrib_LDFLAGS) $(LDFLAGS) \
	-o $@
msgcat_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link \
	$(CCLD) $(AM_CFLAGS) $(CFLAGS) $(msgcat_LDFLAGS) $(LDFLAGS) \
	-o $@
msgcomm_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link \
	$(CCLD) $(AM_CFLAGS) $(CFLAGS) $(msgcomm_LDFLAGS) $(LDFLAGS) \
	-o $@
msgconv_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link \
	$(CCLD) $(AM_CFLAGS) $(CFLAGS) $(msgconv_LDFLAGS) $(LDFLAGS) \
	-o $@
msgen_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link \
	$(CCLD) $(AM_CFLAGS) $(CFLAGS) $(msgen_LDFLAGS) $(LDFLAGS) \
	-o $@
msgfilter_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link \
	$(CCLD) $(AM_CFLAGS) $(CFLAGS) $(msgfilter_LDFLAGS) $(LDFLAGS) \
	-o $@
msggrep_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link \
	$(CCLD) $(AM_CFLAGS) $(CFLAGS) $(msggrep_LDFLAGS) $(LDFLAGS) \
	-o $@
msgmerge_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link \
	$(CCLD) $(msgmerge_CFLAGS) $(CFLAGS) $(msgmerge_LDFLAGS) $(LDFLAGS) \
	-o $@
msguniq_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link \
	$(CCLD) $(AM_CFLAGS) $(CFLAGS) $(msguniq_LDFLAGS) $(LDFLAGS) \
	-o $@
xgettext_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link \
	$(CCLD) $(AM_CFLAGS) $(CFLAGS) $(xgettext_LDFLAGS) $(LDFLAGS) \
	-o $@
else
libgettextsrc_la_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CXX \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link \
	$(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(libgettextsrc_la_LDFLAGS) $(LDFLAGS) \
	-o $@
msgattrib_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CXX \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link \
	$(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(msgattrib_LDFLAGS) $(LDFLAGS) \
	-o $@
msgcat_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CXX \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link \
	$(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(msgcat_LDFLAGS) $(LDFLAGS) \
	-o $@
msgcomm_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CXX \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link \
	$(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(msgcomm_LDFLAGS) $(LDFLAGS) \
	-o $@
msgconv_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CXX \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link \
	$(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(msgconv_LDFLAGS) $(LDFLAGS) \
	-o $@
msgen_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CXX \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link \
	$(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(msgen_LDFLAGS) $(LDFLAGS) \
	-o $@
msgfilter_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CXX \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link \
	$(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(msgfilter_LDFLAGS) $(LDFLAGS) \
	-o $@
msggrep_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CXX \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link \
	$(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(msggrep_LDFLAGS) $(LDFLAGS) \
	-o $@
msgmerge_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CXX \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link \
	$(CXXLD) $(msgmerge_CXXFLAGS) $(CXXFLAGS) $(msgmerge_LDFLAGS) $(LDFLAGS) \
	-o $@
msguniq_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CXX \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link \
	$(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(msguniq_LDFLAGS) $(LDFLAGS) \
	-o $@
xgettext_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CXX \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link \
	$(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(xgettext_LDFLAGS) $(LDFLAGS) \
	-o $@
endif

# Special rules for bison and flex generated files.

# The Automake generated .y.c rule is broken: When executed in a VPATH build,
#   - The .c file gets generated in the build directory. But since it requires
#     special tools to rebuild it, we need to distribute it in the tarballs,
#     and by the GNU Coding Standards
#     <https://www.gnu.org/prep/standards/html_node/Makefile-Basics.html>
#     the file should be generated in the source directory.
#   - The #line numbers in the .c file refer to a nonexistent file once it
#     has been moved from the build directory to the source directory. This
#     leads to error if 'lcov' is used later.
# Therefore we override this rule.
#
# Also, the ylwrap script is not usable when both a .c and a .h file are to be
# generated from the .y file. The reason is that this script does nothing in a
# situation where the .h file is older than the .y file and the .y file is older
# than the .c file. (This is intentional, see the comment "Do not overwrite
# unchanged header files to avoid useless recompilations.") The effect is that
# during "make dist", a tarball is created where the .h file is older than the
# .y file. This has two negative consequences:
#   - For a user who builds an unmodified tarball, bison will be invoked.
#   - During "make distcheck" the rule
#       FILE.c FILE.h: FILE.y
#     always fires and, since $(srcdir) is read-only, the commands which update
#     (or at least touch) $(srcdir)/FILE.c and $(srcdir)/FILE.h fail.
# Therefore we don't use ylwrap.
#
# Note: There is no point in using Bison's --output option, since we need to
# postprocess the generated files and we don't want that unpostprocessed files
# remain in place if the user interrupts the build through Ctrl-C.

po-gram-gen.c po-gram-gen.h: po-gram-gen.y
	$(AM_V_YACC)$(BISON) -d $(YFLAGS) $(AM_YFLAGS) $(srcdir)/po-gram-gen.y \
	&& test ':' = '$(BISON)' || { \
	  sed -e 's|".*/po-gram-gen\.y"|"po-gram-gen.y"|' \
	      -e 's|"po-gram-gen\.tab\.c"|"po-gram-gen.c"|' \
	      -e 's|"po-gram-gen\.tab\.h"|"po-gram-gen.h"|' \
	      < po-gram-gen.tab.c > po-gram-gen.c-tmp \
	  && sed -e 's|".*/po-gram-gen\.y"|"po-gram-gen.y"|' \
	         -e 's|"po-gram-gen\.tab\.h"|"po-gram-gen.h"|' \
	         < po-gram-gen.tab.h > po-gram-gen.h-tmp \
	  && rm -f po-gram-gen.tab.c po-gram-gen.tab.h \
	  && mv po-gram-gen.c-tmp $(srcdir)/po-gram-gen.c \
	  && mv po-gram-gen.h-tmp $(srcdir)/po-gram-gen.h; \
	}
BUILT_SOURCES += po-gram-gen.c po-gram-gen.h
MOSTLYCLEANFILES += po-gram-gen.tab.c po-gram-gen.tab.h po-gram-gen.c-tmp po-gram-gen.h-tmp
MAINTAINERCLEANFILES += po-gram-gen.c po-gram-gen.h
EXTRA_DIST += po-gram-gen.c po-gram-gen.h


# po-gram-gen2.h is generated from po-gram-gen.h and is also distributed in the
# tarballs (just for convenience).
po-gram-gen2.h: po-gram-gen.h
	$(SED) -e 's/yy/po_gram_/g' -e 's/extern /extern DLL_VARIABLE /' \
	       -e 's|"po-gram-gen\.h"|"po-gram-gen2.h"|' \
	       < $(srcdir)/po-gram-gen.h > po-gram-gen2.h-tmp \
	&& mv po-gram-gen2.h-tmp $(srcdir)/po-gram-gen2.h
BUILT_SOURCES += po-gram-gen2.h
MOSTLYCLEANFILES += po-gram-gen2.h-tmp
MAINTAINERCLEANFILES += po-gram-gen2.h
EXTRA_DIST += po-gram-gen2.h

po-lex.o po-lex.lo: po-gram-gen2.h

cldr-plural.c cldr-plural.h: cldr-plural.y
	$(AM_V_YACC)$(BISON) -d $(YFLAGS) $(AM_YFLAGS) $(srcdir)/cldr-plural.y \
	&& test ':' = '$(BISON)' || { \
	  sed -e 's|".*/cldr-plural\.y"|"cldr-plural.y"|' \
	      -e 's|"cldr-plural\.tab\.c"|"cldr-plural.c"|' \
	      -e 's|"cldr-plural\.tab\.h"|"cldr-plural.h"|' \
	      < cldr-plural.tab.c > cldr-plural.c-tmp \
	  && sed -e 's|".*/cldr-plural\.y"|"cldr-plural.y"|' \
	         -e 's|"cldr-plural\.tab\.h"|"cldr-plural.h"|' \
	         < cldr-plural.tab.h > cldr-plural.h-tmp \
	  && rm -f cldr-plural.tab.c cldr-plural.tab.h \
	  && mv cldr-plural.c-tmp $(srcdir)/cldr-plural.c \
	  && mv cldr-plural.h-tmp $(srcdir)/cldr-plural.h; \
	}
BUILT_SOURCES += cldr-plural.c cldr-plural.h
MOSTLYCLEANFILES += cldr-plural.tab.c cldr-plural.tab.h cldr-plural.c-tmp cldr-plural.h-tmp
MAINTAINERCLEANFILES += cldr-plural.c cldr-plural.h
EXTRA_DIST += cldr-plural.c cldr-plural.h


# Special rules for installation of auxiliary programs.

install-exec-local:
	$(MKDIR_P) $(DESTDIR)$(pkglibdir)
	$(INSTALL_PROGRAM_ENV) $(LIBTOOL) --mode=install $(INSTALL_PROGRAM) hostname$(EXEEXT) $(DESTDIR)$(pkglibdir)/hostname$(EXEEXT)
	$(INSTALL_PROGRAM_ENV) $(LIBTOOL) --mode=install $(INSTALL_PROGRAM) urlget$(EXEEXT) $(DESTDIR)$(pkglibdir)/urlget$(EXEEXT)
	$(INSTALL_PROGRAM_ENV) $(LIBTOOL) --mode=install $(INSTALL_PROGRAM) cldr-plurals$(EXEEXT) $(DESTDIR)$(pkglibdir)/cldr-plurals$(EXEEXT)
	$(INSTALL_SCRIPT) user-email $(DESTDIR)$(pkglibdir)/user-email
	$(INSTALL_SCRIPT) $(srcdir)/project-id $(DESTDIR)$(pkglibdir)/project-id

installdirs-local:
	$(MKDIR_P) $(DESTDIR)$(pkglibdir)

uninstall-local:
	$(RM) $(DESTDIR)$(pkglibdir)/hostname$(EXEEXT)
	$(RM) $(DESTDIR)$(pkglibdir)/urlget$(EXEEXT)
	$(RM) $(DESTDIR)$(pkglibdir)/cldr-plurals$(EXEEXT)
	$(RM) $(DESTDIR)$(pkglibdir)/user-email
	$(RM) $(DESTDIR)$(pkglibdir)/project-id

DISTCLEANFILES += user-email


# Special rules for Java compilation.

USEJAVA = $(USEJAVA_@BUILDJAVA@)
USEJAVA_yes = 1
USEJAVA_no = 0

all-local: all-java-@BUILDJAVA@
all-java-yes: gettext.jar
all-java-no:

gnu/gettext/DumpResource.class: $(srcdir)/gnu/gettext/DumpResource.java
	$(JAVACOMP) -d . $(srcdir)/gnu/gettext/DumpResource.java

gnu/gettext/GetURL.class: $(srcdir)/gnu/gettext/GetURL.java
	$(JAVACOMP) -d . $(srcdir)/gnu/gettext/GetURL.java

gettext.jar: gnu/gettext/DumpResource.class gnu/gettext/GetURL.class
	$(JAR) cf $@ gnu/gettext/DumpResource*.class gnu/gettext/GetURL*.class

EXTRA_DIST += gnu/gettext/DumpResource.java gnu/gettext/GetURL.java

CLEANFILES += gettext.jar gnu/gettext/*.class

install-data-local: install-data-java-@BUILDJAVA@
install-data-java-yes: all-java-yes
	$(INSTALL_DATA) gettext.jar $(DESTDIR)$(jardir)/gettext.jar
install-data-java-no:

installdirs-local: installdirs-java-@BUILDJAVA@
installdirs-java-yes:
	$(MKDIR_P) $(DESTDIR)$(jardir)
installdirs-java-no:

uninstall-local: uninstall-java-@BUILDJAVA@
uninstall-java-yes:
	$(RM) $(DESTDIR)$(jardir)/gettext.jar
uninstall-java-no:


# Special rules for C# auxiliary programs.

EXTRA_DIST += msgfmt.cs msgunfmt.cs

CLEANFILES += \
  msgfmt.net.exe msgfmt.net.exe.mdb \
  msgunfmt.net.exe msgunfmt.net.exe.mdb

all-local: all-csharp-@BUILDCSHARP@
all-csharp-yes: msgfmt.net.exe msgunfmt.net.exe
all-csharp-no:

msgfmt.net.exe: msgfmt.cs
	$(CSHARPCOMP) $(CSHARPCOMPFLAGS) -o $@ $(srcdir)/msgfmt.cs

if USE_INSTALLED_CSHARP_DLL
CSHARP_DLL_DIR = $(libdir)
else
CSHARP_DLL_DIR = ../../gettext-runtime/intl-csharp
endif
msgunfmt.net.exe: msgunfmt.cs
	$(CSHARPCOMP) $(CSHARPCOMPFLAGS) -o $@ -L '$(CSHARP_DLL_DIR)' -l GNU.Gettext $(srcdir)/msgunfmt.cs

install-exec-local: install-exec-csharp-@BUILDCSHARP@
install-exec-csharp-yes: all-csharp-yes
	$(MKDIR_P) $(DESTDIR)$(pkglibdir)
	$(INSTALL_DATA) msgfmt.net.exe $(DESTDIR)$(pkglibdir)/msgfmt.net.exe
	$(INSTALL_DATA) msgunfmt.net.exe $(DESTDIR)$(pkglibdir)/msgunfmt.net.exe
install-exec-csharp-no:
	$(MKDIR_P) $(DESTDIR)$(pkglibdir)

installdirs-local: install-csharp
installdirs-csharp:
	$(MKDIR_P) $(DESTDIR)$(pkglibdir)

uninstall-local: uninstall-csharp-@BUILDCSHARP@
uninstall-csharp-yes: all-csharp-yes
	$(RM) $(DESTDIR)$(pkglibdir)/msgfmt.net.exe
	$(RM) $(DESTDIR)$(pkglibdir)/msgunfmt.net.exe
uninstall-csharp-no:


# Special rules for Tcl auxiliary program.

EXTRA_DIST += msgunfmt.tcl

install-data-local: install-tcl
install-tcl:
	$(MKDIR_P) $(DESTDIR)$(pkgdatadir)
	$(INSTALL_DATA) $(srcdir)/msgunfmt.tcl $(DESTDIR)$(pkgdatadir)/msgunfmt.tcl

installdirs-local: installdirs-tcl
installdirs-tcl:
	$(MKDIR_P) $(DESTDIR)$(pkgdatadir)

uninstall-local: uninstall-tcl
uninstall-tcl:
	$(RM) $(DESTDIR)$(pkgdatadir)/msgunfmt.tcl


# Support for relocatability.
RELOCATABLE_LIBRARY_PATH = $(libdir)
RELOCATABLE_SRC_DIR = $(top_srcdir)/gnulib-lib
RELOCATABLE_BUILD_DIR = ../gnulib-lib
RELOCATABLE_CONFIG_H_DIR = ..
RELOCATABLE_STRIP = :


# Version information according to Woe32 conventions.
if WOE32
WOE32_LDADD = gettext.res
gettext.res : $(top_srcdir)/../windows/gettext.rc
	$(WINDRES) `$(SHELL) $(top_srcdir)/../windows/windres-options --escape $(VERSION)` -i $(top_srcdir)/../windows/gettext.rc -o gettext.res --output-format=coff
MOSTLYCLEANFILES += gettext.res
else
WOE32_LDADD =
endif


# This Makefile contains rules which don't work with parallel make,
# namely those which build textstyle.h, po-gram-gen.c, cldr-plural.c.
# See <https://lists.gnu.org/archive/html/bug-make/2019-05/msg00011.html>.
# So, turn off parallel execution in this Makefile.
.NOTPARALLEL:
