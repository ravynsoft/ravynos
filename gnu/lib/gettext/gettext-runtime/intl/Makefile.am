## Makefile for the gettext-runtime/intl and gettext-tools/intl subdirectories
## of GNU gettext.
## Copyright (C) 1995-1998, 2000-2007, 2009-2023 Free Software Foundation, Inc.
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation; either version 2.1 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this program.  If not, see <https://www.gnu.org/licenses/>.

## Process this file with automake to produce Makefile.in.

AUTOMAKE_OPTIONS = 1.10 gnu no-dependencies
ACLOCAL_AMFLAGS = -I ../../m4 -I ../m4 -I gnulib-m4

SUBDIRS = gnulib-lib

EXTRA_DIST =
BUILT_SOURCES =
MOSTLYCLEANFILES = core *.stackdump
CLEANFILES =
DISTCLEANFILES =
MAINTAINERCLEANFILES =
SUFFIXES =

# -DBUILDING_LIBINTL: Change expansion of LIBINTL_DLL_EXPORTED macro.
# -DBUILDING_DLL: Change expansion of RELOCATABLE_DLL_EXPORTED macro.
# -DBUILDING_DLL: Change expansion of DLL_EXPORTED macro. We're compiling
#                 a library. Exporting the relevant symbols is desired.
AM_CPPFLAGS = \
  -Ignulib-lib -I$(srcdir)/gnulib-lib \
  -DLOCALEDIR=$(localedir_c_make) -DLOCALE_ALIAS_PATH=\"$(aliaspath)\" \
  -DLIBDIR=\"$(libdir)\" -DBUILDING_LIBINTL -DBUILDING_DLL

if WOE32
# On mingw, disable the declarations of *printf functions as aliases to the
# corresponding __mingw_*printf functions, because
# - these functions are useless for i18n purposes (not POSIX/XSI compliant),
# - they pull in a dependency to the libgcc_s_sjlj DLL (through the symbols
#   __udivdi3, __umoddi3).
AM_CPPFLAGS += -D__USE_MINGW_ANSI_STDIO=0
endif

# Enable more warning options in this directory.
AM_CFLAGS = @WARN_CFLAGS@

BISON = @INTLBISON@
BISONFLAGS = --name-prefix=__gettext
# Just to shut up Automake "error: Yacc source seen but 'YACC' is undefined".
YACC = $(BISON) -d

# Tell the ELF linker which symbols to export.
AM_CFLAGS += @CFLAG_VISIBILITY@
# Tell the mingw or Cygwin linker which symbols to export.
if WOE32DLL
AM_LDFLAGS = -Wl,--export-all-symbols
endif

# Support building libintl.a with -fPIC (needed by GCC).
AM_CFLAGS += @PICFLAG@


# Library include file and code.

EXTRA_DIST += \
  gmo.h \
  gettextP.h \
  hash-string.h \
  loadinfo.h \
  plural-exp.h \
  eval-plural.h \
  intl-exports.c os2compat.h os2compat.c \
  libgnuintl.in.h

# Overview of the code
# --------------------
#
# The following files implement public API, declared in libgnuintl.in.h:
#   version.c:          libintl_version
#   gettext.c:          libintl_gettext
#   dgettext.c:         libintl_dgettext
#   dcgettext.c:        libintl_dcgettext
#   ngettext.c:         libintl_ngettext
#   dngettext.c:        libintl_dngettext
#   dcngettext.c:       libintl_dcngettext
#   textdomain.c:       libintl_textdomain
#   bindtextdom.c:      libintl_bindtextdomain
#                       libintl_wbindtextdomain
#                       libintl_bind_textdomain_codeset
#   printf.c:           libintl_fprintf
#                       libintl_vfprintf
#                       libintl_printf
#                       libintl_vprintf
#                       libintl_sprintf
#                       libintl_vsprintf
#                       libintl_snprintf
#                       libintl_vsnprintf
#                       libintl_asprintf
#                       libintl_vasprintf
#                       libintl_fwprintf
#                       libintl_vfwprintf
#                       libintl_wprintf
#                       libintl_vwprintf
#                       libintl_swprintf
#                       libintl_vswprintf
#   localename.c:       libintl_newlocale
#                       libintl_duplocale
#                       libintl_freelocale
#   setlocale.c:        libintl_newlocale
#                       libintl_setlocale
#   relocatable.c:      libintl_set_relocation_prefix
#
# The following files implement private API.
#   gettextP.h:     dcigettext.c:       libintl_dcigettext
#                                       _nl_find_msg
#                                       libintl_nl_default_dirname
#                                       libintl_nl_domain_bindings
#                                       libintl_nl_default_default_domain
#                                       libintl_nl_current_default_domain
#                   localename.c:       _nl_locale_name_canonicalize
#                                       _nl_locale_name_from_win32_LANGID
#                                       _nl_locale_name_from_win32_LCID
#                                       _nl_locale_name_thread_unsafe
#                                       _nl_locale_name_thread
#                                       _nl_locale_name_posix
#                                       _nl_locale_name_environ
#                                       _nl_locale_name_default
#                                       _nl_locale_name
#                   langprefs.c:        _nl_language_preferences_default
#                   finddomain.c:       _nl_find_domain
#                   loadmsgcat.c:       _nl_load_domain
#                                       _nl_msg_cat_cntr
#   loadinfo.h:     l10nflist.c:        _nl_make_l10nflist
#                                       _nl_normalize_codeset
#                   localealias.c:      _nl_expand_alias
#                   explodename.c:      _nl_explode_name
#   plural-exp.h:   plural.y:           libintl_gettext_free_exp
#                                       libintl_gettextparse
#                   plural-exp.c:       libintl_gettext_germanic_plural
#                                       libintl_gettext_extract_plural
#                   eval-plural.h:      plural_eval
#
# Other files:
#   gmo.h:          Describes the GNU MO file format.
#   hash-string.h:  hash-string.c:      libintl_hash_string
#
# Some other files are borrowed from gnulib, some with small modifications.
#
# Overview of invocation hierarchy
# --------------------------------
#
# The main general translation lookup function is libintl_dcigettext.
# libintl_dcigettext
#   \-> guess_category_value
#         \-> _nl_locale_name_thread_unsafe
#         \-> _nl_locale_name_posix
#         \-> _nl_locale_name_default
#         \-> _nl_language_preferences_default
#   \-> _nl_find_domain
#         \-> _nl_make_l10nflist
#         \-> _nl_load_domain
#         \-> _nl_expand_alias
#         \-> _nl_explode_name
#               \-> _nl_normalize_codeset
#   \-> _nl_find_msg
#         \-> _nl_load_domain
#         \-> get_output_charset
#   \-> plural_lookup
#         \-> plural_eval
#
# _nl_load_domain loads a MO file into memory.
# _nl_load_domain
#   \-> _nl_find_msg (for the header entry only)
#   \-> libintl_gettext_extract_plural

LIBINTLSOURCES = \
  bindtextdom.c \
  dcgettext.c \
  dgettext.c \
  gettext.c \
  finddomain.c \
  hash-string.c \
  loadmsgcat.c \
  localealias.c \
  textdomain.c \
  l10nflist.c \
  explodename.c \
  dcigettext.c \
  dcngettext.c \
  dngettext.c \
  ngettext.c \
  plural.y \
  plural-exp.c \
  langprefs.c \
  log.c \
  printf.c \
  setlocale.c \
  version.c \
  osdep.c \
  intl-compat.c

# We must not install the libintl.h/libintl.la files if we are on a
# system which has the GNU gettext() function in its C library or in a
# separate library.
if USE_INCLUDED_LIBINTL
nodist_include_HEADERS = libintl.h
# Note: libintl.la actually gets installed in $(libdir). See below.
noinst_LTLIBRARIES = libintl.la
libintl_la_SOURCES = $(LIBINTLSOURCES)
else
noinst_LTLIBRARIES = libgnuintl.la
libgnuintl_la_SOURCES = $(LIBINTLSOURCES)
endif

# export.h is an auxiliary file used in constructing the .h files.
EXTRA_DIST += export.h

libgnuintl.h: $(srcdir)/libgnuintl.in.h $(srcdir)/export.h
	sed -e '/IN_LIBGLOCALE/d' \
	    -e 's,@''HAVE_POSIX_PRINTF''@,@HAVE_POSIX_PRINTF@,g' \
	    -e 's,@''HAVE_ASPRINTF''@,@HAVE_ASPRINTF@,g' \
	    -e 's,@''HAVE_SNPRINTF''@,@HAVE_SNPRINTF@,g' \
	    -e 's,@''HAVE_WPRINTF''@,@HAVE_WPRINTF@,g' \
	    -e 's,@''HAVE_NEWLOCALE''@,@HAVE_NEWLOCALE@,g' \
	    -e 's,@''ENHANCE_LOCALE_FUNCS''@,@ENHANCE_LOCALE_FUNCS@,g' \
	  < $(srcdir)/libgnuintl.in.h \
	| if test '@WOE32DLL@' = yes; then \
	    sed -e 's/extern \([^()]*\);/extern __declspec (dllimport) \1;/'; \
	  else \
	    cat; \
	  fi \
	| sed -e 's/extern \([^"]\)/extern LIBINTL_DLL_EXPORTED \1/' \
	      -e "/#define _LIBINTL_H/r $(srcdir)/export.h" \
	| sed -e '/#define _LIBINTL_H/,/[*][/]$$/{/#define _LIBINTL_H/!d;}' \
	| sed -e 's,@''HAVE_VISIBILITY''@,@HAVE_VISIBILITY@,g' \
	  > libgnuintl.h
MOSTLYCLEANFILES += libgnuintl.h

libintl.h: $(srcdir)/libgnuintl.in.h
	sed -e '/IN_LIBGLOCALE/d' \
	    -e 's,@''HAVE_POSIX_PRINTF''@,@HAVE_POSIX_PRINTF@,g' \
	    -e 's,@''HAVE_ASPRINTF''@,@HAVE_ASPRINTF@,g' \
	    -e 's,@''HAVE_SNPRINTF''@,@HAVE_SNPRINTF@,g' \
	    -e 's,@''HAVE_WPRINTF''@,@HAVE_WPRINTF@,g' \
	    -e 's,@''HAVE_NEWLOCALE''@,@HAVE_NEWLOCALE@,g' \
	    -e 's,@''ENHANCE_LOCALE_FUNCS''@,@ENHANCE_LOCALE_FUNCS@,g' \
	  < $(srcdir)/libgnuintl.in.h > libintl.h
MOSTLYCLEANFILES += libintl.h

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
#
# Since this is a rule that produces multiple files, we apply the idiom from
# <https://lists.gnu.org/archive/html/bug-make/2020-09/msg00008.html>, so that
# it works also in parallel 'make'.
generate-plural:
	$(AM_V_YACC)$(BISON) -d $(BISONFLAGS) $(srcdir)/plural.y \
	&& test ':' = '$(BISON)' || { \
	  sed -e 's|".*/plural\.y"|"plural.y"|' \
	      -e 's|"plural\.tab\.c"|"plural.c"|' \
	      -e 's|"plural\.tab\.h"|"plural.h"|' \
	      < plural.tab.c > plural.c-tmp \
	  && sed -e 's|".*/plural\.y"|"plural.y"|' \
	         -e 's|"plural\.tab\.h"|"plural.h"|' \
	         < plural.tab.h > plural.h-tmp \
	  && rm -f plural.tab.c plural.tab.h \
	  && mv plural.c-tmp $(srcdir)/plural.c \
	  && mv plural.h-tmp $(srcdir)/plural.h; \
	}
.PHONY: generate-plural
# The above rule will generate files with time stamp order
# plural.y <= plural.c <= plural.h.
plural.c: plural.y
	@{ test -f $(srcdir)/plural.c && test ! $(srcdir)/plural.c -ot $(srcdir)/plural.y; } || $(MAKE) generate-plural
plural.h: plural.c
	@{ test -f $(srcdir)/plural.h && test ! $(srcdir)/plural.h -ot $(srcdir)/plural.c; } || $(MAKE) generate-plural
BUILT_SOURCES        += plural.c plural.h
MOSTLYCLEANFILES     += plural.tab.c plural.tab.h plural.c-tmp plural.h-tmp
MAINTAINERCLEANFILES += plural.c plural.h
EXTRA_DIST           += plural.c plural.h

# Rules for compiling a .c file, that work even without a VPATH variable.
bindtextdom.lo: $(srcdir)/bindtextdom.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/bindtextdom.c
dcgettext.lo: $(srcdir)/dcgettext.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/dcgettext.c
dgettext.lo: $(srcdir)/dgettext.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/dgettext.c
gettext.lo: $(srcdir)/gettext.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/gettext.c
finddomain.lo: $(srcdir)/finddomain.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/finddomain.c
hash-string.lo: $(srcdir)/hash-string.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/hash-string.c
loadmsgcat.lo: $(srcdir)/loadmsgcat.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/loadmsgcat.c
localealias.lo: $(srcdir)/localealias.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/localealias.c
textdomain.lo: $(srcdir)/textdomain.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/textdomain.c
l10nflist.lo: $(srcdir)/l10nflist.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/l10nflist.c
explodename.lo: $(srcdir)/explodename.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/explodename.c
dcigettext.lo: $(srcdir)/dcigettext.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/dcigettext.c
dcngettext.lo: $(srcdir)/dcngettext.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/dcngettext.c
dngettext.lo: $(srcdir)/dngettext.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/dngettext.c
ngettext.lo: $(srcdir)/ngettext.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/ngettext.c
plural.lo: $(srcdir)/plural.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/plural.c
plural-exp.lo: $(srcdir)/plural-exp.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/plural-exp.c
langprefs.lo: $(srcdir)/langprefs.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/langprefs.c
log.lo: $(srcdir)/log.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/log.c
printf.lo: $(srcdir)/printf.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/printf.c
setlocale.lo: $(srcdir)/setlocale.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/setlocale.c
version.lo: $(srcdir)/version.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/version.c
osdep.lo: $(srcdir)/osdep.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/osdep.c
intl-compat.lo: $(srcdir)/intl-compat.c
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(srcdir)/intl-compat.c

# Dependencies.

PLURAL_DEPS =
if USE_INCLUDED_LIBINTL
# A bison-2.1 generated plural.c includes <libintl.h> if ENABLE_NLS.
PLURAL_DEPS += libintl.h
endif

bindtextdom.lo:      ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h
dcgettext.lo:        ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h
dgettext.lo:         ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h
gettext.lo:          ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h
finddomain.lo:       ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h
hash-string.lo:      ../config.h $(srcdir)/hash-string.h
loadmsgcat.lo:       ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h $(srcdir)/hash-string.h $(srcdir)/plural-exp.h
localealias.lo:      ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h
textdomain.lo:       ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h
l10nflist.lo:        ../config.h $(srcdir)/loadinfo.h
explodename.lo:      ../config.h $(srcdir)/loadinfo.h
dcigettext.lo:       ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h $(srcdir)/plural-exp.h $(srcdir)/hash-string.h $(srcdir)/eval-plural.h
dcngettext.lo:       ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h
dngettext.lo:        ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h
ngettext.lo:         ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h
plural.lo:           ../config.h $(srcdir)/plural-exp.h $(PLURAL_DEPS)
plural-exp.lo:       ../config.h $(srcdir)/plural-exp.h
langprefs.lo:        ../config.h
log.lo:              ../config.h
printf.lo:           ../config.h
setlocale.lo:        ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h
version.lo:          ../config.h libgnuintl.h
osdep.lo:            ../config.h $(srcdir)/intl-exports.c $(srcdir)/os2compat.c
intl-compat.lo:      ../config.h $(srcdir)/gettextP.h libgnuintl.h $(srcdir)/gmo.h $(srcdir)/loadinfo.h

# Version information according to Woe32 conventions.
EXTRA_DIST += libintl.rc
if WOE32
WOE32_LIBADD = libintl.res.lo
# This rule is executed only on Woe32 systems.
# Use $(RC) with libtool, $(WINDRES) when not using libtool.
# The following sed expressions come from the windres-options script. They are
# inlined here, so that they can be written in a Makefile without requiring a
# temporary file. They must contain literal newlines rather than semicolons,
# so that they work with the sed-3.02 that is shipped with MSYS.
libintl.res.lo: $(srcdir)/libintl.rc
	nlinit=`echo 'nl="'; echo '"'`; eval "$$nlinit"; \
	sed_extract_major='/^[0-9]/{'$${nl}'s/^\([0-9]*\).*/\1/p'$${nl}q$${nl}'}'$${nl}'c\'$${nl}0$${nl}q; \
	sed_extract_minor='/^[0-9][0-9]*[.][0-9]/{'$${nl}'s/^[0-9]*[.]\([0-9]*\).*/\1/p'$${nl}q$${nl}'}'$${nl}'c\'$${nl}0$${nl}q; \
	sed_extract_subminor='/^[0-9][0-9]*[.][0-9][0-9]*[.][0-9]/{'$${nl}'s/^[0-9]*[.][0-9]*[.]\([0-9]*\).*/\1/p'$${nl}q$${nl}'}'$${nl}'c\'$${nl}0$${nl}q; \
	$(LIBTOOL) --tag=RC $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=compile $(RC) \
	  "-DPACKAGE_VERSION_STRING=\\\"$(VERSION)\\\"" \
	  "-DPACKAGE_VERSION_MAJOR="`echo '$(VERSION)' | sed -n -e "$$sed_extract_major"` \
	  "-DPACKAGE_VERSION_MINOR="`echo '$(VERSION)' | sed -n -e "$$sed_extract_minor"` \
	  "-DPACKAGE_VERSION_SUBMINOR="`echo '$(VERSION)' | sed -n -e "$$sed_extract_subminor"` \
	  -i $(srcdir)/libintl.rc -o libintl.res.lo --output-format=coff
MOSTLYCLEANFILES += libintl.res.lo
else
WOE32_LIBADD =
endif

# What object files to include in libintl.la and libgnuintl.la.
libintl_la_LIBADD          = gnulib-lib/libgnu.la $(WOE32_LIBADD)
libintl_la_DEPENDENCIES    = gnulib-lib/libgnu.la $(WOE32_LIBADD)
libgnuintl_la_LIBADD       = gnulib-lib/libgnu.la $(WOE32_LIBADD)
libgnuintl_la_DEPENDENCIES = gnulib-lib/libgnu.la $(WOE32_LIBADD)

# langprefs.c (_nl_language_preferences_win32_95) uses functions from
# advapi32.dll.
if WINDOWS_NATIVE
INTL_WINDOWS_LIBS = -ladvapi32
else
INTL_WINDOWS_LIBS =
endif

# Libtool's library version information for libintl.
# Before making a gettext release, you must change this according to the
# libtool documentation, section "Library interface versions".
LTV_CURRENT=12
LTV_REVISION=0
LTV_AGE=4

# How to build libintl.la and libgnuintl.la.
# Limit the exported symbols: Don't export _libintl_* (from gnulib modules).
OTHER_LDFLAGS = \
  @LTLIBICONV@ @INTL_MACOSX_LIBS@ $(INTL_WINDOWS_LIBS) @LIB_SETLOCALE_NULL@ @LTLIBTHREAD@ \
  -no-undefined \
  -export-symbols-regex '^([^_]|_[^l]|_l[^i]|_li[^b]|_lib[^i]|_libi[^n]|_libin[^t]|_libint[^l]|_libintl[^_]).*' \
  -version-info $(LTV_CURRENT):$(LTV_REVISION):$(LTV_AGE) \
  -rpath $(libdir)
libintl_la_LDFLAGS    = $(AM_LDFLAGS) $(OTHER_LDFLAGS)
libgnuintl_la_LDFLAGS = $(AM_LDFLAGS) $(OTHER_LDFLAGS)


# Installation of libintl.la.

if USE_INCLUDED_LIBINTL

install-exec-local: install-exec-libintl
install-exec-libintl: libintl.la
	$(mkdir_p) $(DESTDIR)$(libdir)
	$(LIBTOOL) --mode=install \
	  $(INSTALL_DATA) libintl.la $(DESTDIR)$(libdir)/libintl.la; \
	if test "@RELOCATABLE@" = yes; then \
	  dependencies=`sed -n -e 's,^dependency_libs=\(.*\),\1,p' < $(DESTDIR)$(libdir)/libintl.la | sed -e "s,^',," -e "s,'\$$,,"`; \
	  if test -n "$$dependencies"; then \
	    rm -f $(DESTDIR)$(libdir)/libintl.la; \
	  fi; \
	fi

installdirs-local: installdirs-libintl
installdirs-libintl:
	$(mkdir_p) $(DESTDIR)$(libdir)

uninstall-local: uninstall-libintl
uninstall-libintl:
	$(LIBTOOL) --mode=uninstall \
	  rm -f $(DESTDIR)$(libdir)/libintl.la

endif


# Installation of preloadable_libintl.so.

if PRELOADABLE_LIBINTL

install-exec-local: install-exec-preloadable
install-exec-preloadable: libgnuintl.la
	$(mkdir_p) $(DESTDIR)$(libdir)
	$(LIBTOOL) --mode=install \
	  $(INSTALL_DATA) libgnuintl.la $(DESTDIR)$(libdir)/libgnuintl.la
	rm -f $(DESTDIR)$(libdir)/preloadable_libintl.so
	$(INSTALL_DATA) $(DESTDIR)$(libdir)/libgnuintl.so $(DESTDIR)$(libdir)/preloadable_libintl.so
	$(LIBTOOL) --mode=uninstall \
	  rm -f $(DESTDIR)$(libdir)/libgnuintl.la

installdirs-local: installdirs-preloadable
installdirs-preloadable:
	$(mkdir_p) $(DESTDIR)$(libdir)

uninstall-local: uninstall-preloadable
uninstall-preloadable:
	rm -f $(DESTDIR)$(libdir)/preloadable_libintl.so

endif


# Installation of locale.alias.

EXTRA_DIST += locale.alias ref-add.sin ref-del.sin

if USE_INCLUDED_LIBINTL

all-local: ref-add.sed ref-del.sed

ref-add.sed: $(srcdir)/ref-add.sin
	sed -e '/^#/d' -e 's/@''PACKAGE''@/@PACKAGE@/g' $(srcdir)/ref-add.sin > t-ref-add.sed
	mv t-ref-add.sed ref-add.sed
MOSTLYCLEANFILES += ref-add.sed

ref-del.sed: $(srcdir)/ref-del.sin
	sed -e '/^#/d' -e 's/@''PACKAGE''@/@PACKAGE@/g' $(srcdir)/ref-del.sin > t-ref-del.sed
	mv t-ref-del.sed ref-del.sed
MOSTLYCLEANFILES += ref-del.sed

install-data-local: install-data-aliasfile
install-data-aliasfile: ref-add.sed
	$(mkdir_p) $(DESTDIR)$(localedir)
	test -f $(DESTDIR)$(localedir)/locale.alias \
	  && orig=$(DESTDIR)$(localedir)/locale.alias \
	  || orig=$(srcdir)/locale.alias; \
	temp=$(DESTDIR)$(localedir)/t-locale.alias; \
	dest=$(DESTDIR)$(localedir)/locale.alias; \
	sed -f ref-add.sed $$orig > $$temp; \
	$(INSTALL_DATA) $$temp $$dest; \
	rm -f $$temp

installdirs-local: installdirs-aliasfile
installdirs-aliasfile:
	$(mkdir_p) $(DESTDIR)$(localedir)

uninstall-local: uninstall-aliasfile
uninstall-aliasfile: ref-del.sed
	if test -f $(DESTDIR)$(localedir)/locale.alias; then \
	  temp=$(DESTDIR)$(localedir)/t-locale.alias; \
	  dest=$(DESTDIR)$(localedir)/locale.alias; \
	  sed -f ref-del.sed $$dest > $$temp; \
	  if grep '^# Packages using this file: $$' $$temp > /dev/null; then \
	    rm -f $$dest; \
	  else \
	    $(INSTALL_DATA) $$temp $$dest; \
	  fi; \
	  rm -f $$temp; \
	fi

endif


# Allow users to use "gnulib-tool --update".
EXTRA_DIST += gnulib-m4/gnulib-cache.m4


# Clean up after Solaris cc.
clean-local:
	rm -rf SunWS_cache


# Windows support.

EXTRA_DIST += INSTALL.windows


# Miscellaneous files.

EXTRA_DIST += COPYING.LIB libintl.glibc
