## Makefile.am -- Process this file with automake to produce Makefile.in
##
##   Copyright (C) 2003-2019, 2021-2022 Free Software Foundation, Inc.
##   Written by Gary V. Vaughan, 2003
##
##   This file is part of GNU Libtool.
##
## GNU Libtool is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License as
## published by the Free Software Foundation; either version 2 of
## the License, or (at your option) any later version.
##
## GNU Libtool is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with GNU Libtool; see the file COPYING.  If not, a copy
## can be downloaded from  http://www.gnu.org/licenses/gpl.html,
## or obtained by writing to the Free Software Foundation, Inc.,
## 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#####

ACLOCAL_AMFLAGS		= -I m4

AM_CPPFLAGS		=
AM_LDFLAGS		=

SUBDIRS			= . gnulib-tests
DIST_SUBDIRS		= $(SUBDIRS)
EXTRA_DIST		=

BUILT_SOURCES		= libtool libtoolize

CLEANFILES		=
MOSTLYCLEANFILES	=
DISTCLEANFILES		=
MAINTAINERCLEANFILES	=

include_HEADERS		=
noinst_LTLIBRARIES	=
lib_LTLIBRARIES		=
EXTRA_LTLIBRARIES	=

# Using 'cd' in backquotes may print the directory name, use this instead:
lt__cd		= CDPATH="$${ZSH_VERSION+.}$(PATH_SEPARATOR)" && cd

git_version_gen = '$(SHELL)' '$(aux_dir)/git-version-gen' '--fallback' '$(VERSION)' '.tarball-version'
rebuild = rebuild=:; revision=`$(lt__cd) $(srcdir) && $(git_version_gen) | $(SED) 's|-.*$$||'`


# ---------- #
# Bootstrap. #
# ---------- #

SCRIPT_ENV	= GREP='$(GREP)' SED='$(SED)'

build_scripts	= $(srcdir)/$(aux_dir)/announce-gen \
		  $(srcdir)/$(aux_dir)/do-release-commit-and-tag \
		  $(srcdir)/$(aux_dir)/gendocs.sh \
		  $(srcdir)/$(aux_dir)/git-version-gen \
		  $(srcdir)/$(aux_dir)/gnu-web-doc-update \
		  $(srcdir)/$(aux_dir)/gnupload \
		  $(srcdir)/$(aux_dir)/useless-if-before-free \
		  $(srcdir)/$(aux_dir)/vc-list-files

EXTRA_DIST     += bootstrap bootstrap.conf $(build_scripts) cfg.mk maint.mk \
		  GNUmakefile
CLEANFILES     += libtool libtoolize

## If a file is named several times below, and especially if it
## is a distributed file created during Libtool bootstrap, we
## put it in a variable in accordance with the DRY principle.
## Use '$(srcdir)' for the benefit of non-GNU makes: referring
## to them by their variable makes sure they appear as targets
## and dependencies with '$(srcdir)' consistently.
configure_ac	= $(srcdir)/configure.ac
config_status	= $(top_builddir)/config.status
extract_trace	= $(srcdir)/$(aux_dir)/extract-trace
funclib_sh	= $(srcdir)/$(aux_dir)/funclib.sh
inline_source	= $(srcdir)/$(aux_dir)/inline-source
libtoolize_in	= $(srcdir)/libtoolize.in
ltmain_sh	= $(srcdir)/$(aux_dir)/ltmain.sh
ltmain_in	= $(srcdir)/$(aux_dir)/ltmain.in
libtool_m4	= $(srcdir)/$(macro_dir)/libtool.m4
ltversion_in	= $(srcdir)/$(macro_dir)/ltversion.in
ltversion_m4	= $(srcdir)/$(macro_dir)/ltversion.m4
no_bogus_macros	= $(srcdir)/$(aux_dir)/no-bogus-m4-defines
options_parser	= $(srcdir)/$(aux_dir)/options-parser
u2d_copyright	= $(srcdir)/$(aux_dir)/update-copyright

EXTRA_DIST     += $(extract_trace) $(funclib_sh) $(inline_source) \
		  $(libtoolize_in) $(ltmain_in) $(ltmain_sh) \
		  $(ltversion_in) $(ltversion_m4) $(no_bogus_macros) \
		  $(options_parser) $(u2d_copyright)

## These are the replacements that need to be made at bootstrap time,
## because they must be static in distributed files, and not accidentally
## changed by configure running on the build machine.
bootstrap_edit  = $(SED) \
		  -e 's|@MACRO_VERSION\@|$(VERSION)|g' \
		  -e "s|@MACRO_REVISION\@|$$revision|g" \
		  -e "s|@MACRO_SERIAL\@|$$serial|g" \
		  -e 's|@PACKAGE\@|$(PACKAGE)|g' \
		  -e 's|@PACKAGE_BUGREPORT\@|$(PACKAGE_BUGREPORT)|g' \
		  -e 's|@PACKAGE_URL\@|$(PACKAGE_URL)|g' \
		  -e 's|@PACKAGE_NAME\@|$(PACKAGE_NAME)|g' \
		  -e "s|@package_revision\@|$$revision|g" \
		  -e 's|@PACKAGE_STRING\@|$(PACKAGE_NAME) $(VERSION)|g' \
		  -e 's|@PACKAGE_TARNAME\@|$(PACKAGE)|g' \
		  -e 's|@PACKAGE_VERSION\@|$(VERSION)|g' \
		  -e 's|@VERSION\@|$(VERSION)|g'

## We build ltversion.m4 here, instead of from config.status,
## because config.status is rerun each time one of configure's
## dependencies change and ltversion.m4 happens to be a configure
## dependency.  configure and ltversion.m4 would be rebuilt in
## a loop otherwise.
## We used to do this with a 'stamp-vcl' file, but non-gmake builds
## would rerun configure on every invocation, so now we manually
## check the version numbers from the build rule when necessary.
$(ltversion_m4): $(ltversion_in) $(dotversion)
	@$(rebuild); \
	if test -f '$@'; then \
	  eval `'$(SED)' -n '/^macro_revision=/p' '$@'`; \
	  test "x$$macro_revision" = "x$$revision" && rebuild=false; \
	fi; \
	for prereq in $?; do \
	  case $$prereq in *.version);; *) rebuild=:;; esac; \
	done; \
	if $$rebuild; then \
	  rm -f '$@'; \
	  if test -d '$(srcdir)/.git' && git --version >/dev/null 2>&1; then \
	    $(git_commit_count) > '$(srcdir)/.serial'; \
	  fi; \
	  serial=`cat '$(srcdir)/.serial'`; \
	  if test 0 = '$(AM_DEFAULT_VERBOSITY)' && test 1 != '$(V)'; \
	    then echo "  GEN     " $@; \
	  else echo $(bootstrap_edit) "'$(ltversion_in)' > '$@'"; fi; \
	  $(bootstrap_edit) '$(ltversion_in)' > '$@'; \
	  chmod a-w '$@'; \
	fi

## And for similar reasons, ltmain.sh can't be built from config.status.
## We used to do this with a 'stamp-vcl' file, but non-gmake builds
## would rerun configure on every invocation, so now we manually
## check the version numbers from the build rule when necessary.
## !WARNING! If you edit this rule to change the contents of ltmain.sh,
##           you must 'touch $(aux_dir)/ltmain.in' from the
##           shell if you need ltmain.sh to be regenerated.  Ideally, we
##           should make this rule depend on Makefile but that will break
##           distcheck (at least) by rebuilding ltmain.sh in the source
##           tree whenever config.status regenerates the Makefile.
$(ltmain_sh): $(ltmain_in) $(dotversion)
	@$(rebuild); \
	if test -f '$@'; then \
	  eval `'$(SED)' -n '/^package_revision=/p' '$@'`; \
	  test "x$$package_revision" = "x$$revision" && rebuild=false; \
	fi; \
	for prereq in $?; do \
	  case $$prereq in *.version);; *) rebuild=:;; esac; \
	done; \
	if $$rebuild; then \
	  rm -f '$@'; \
	  if test 0 = '$(AM_DEFAULT_VERBOSITY)' && test 1 != '$(V)'; \
	    then echo "  GEN     " $@; \
	  else echo "$(inline_source) '$(ltmain_in)' |" $(bootstrap_edit) "> '$@'"; fi; \
	  $(SCRIPT_ENV) '$(inline_source)' '$(ltmain_in)' | $(bootstrap_edit) > '$@'; \
	  chmod a-w '$@'; \
	fi

lt_Makefile_am	= $(srcdir)/$(ltdl_dir)/Makefile.am
lt_Makefile_in	= $(srcdir)/$(ltdl_dir)/Makefile.in
ltdl_mk		= $(srcdir)/$(ltdl_dir)/ltdl.mk

$(lt_Makefile_am): $(ltdl_mk)
	$(AM_V_at)rm -f '$@'
	$(AM_V_GEN)( '$(SED)' -n '1,/^.. DO NOT REMOVE THIS LINE -- /p' \
	      '$(ltdl_mk)'; \
	  { echo 'ACLOCAL_AMFLAGS = -I ../m4'; \
	    echo 'AUTOMAKE_OPTIONS = foreign'; \
	    echo 'AM_CPPFLAGS ='; \
	    echo 'AM_LDFLAGS ='; \
	    echo 'BUILT_SOURCES ='; \
	    echo 'include_HEADERS ='; \
	    echo 'noinst_LTLIBRARIES ='; \
	    echo 'lib_LTLIBRARIES ='; \
	    echo 'EXTRA_LTLIBRARIES ='; \
	    echo 'EXTRA_DIST ='; \
	    echo 'CLEANFILES ='; \
	    echo 'MOSTLYCLEANFILES ='; \
	  }; \
	  '$(SED)' -n '/^.. DO NOT REMOVE THIS LINE -- /,$$p' \
	      '$(ltdl_mk)' \
	    |'$(SED)' -e 's|libltdl_||; s|libltdl/||; s|: libltdl/|: |' \
	      -e '/^[	 ]*-I\$$(srcdir)\/libltdl -Ilibltdl \\/d' \
	      -e 's|\$$(libltdl_|$$(|' \
	) |'$(SED)' -e '/^.. DO NOT REMOVE THIS LINE -- /d' \
	      -e '1s,^\(.. Makefile.\)inc.*,\1am -- Process this file with automake to produce Makefile.in,' > '$@'
	$(AM_V_at)chmod a-w '$@'

## Document the make macros that are needed to build bootstrap-deps
## dependencies when called from 'bootstrap' (developer's machine),
## where the Makefile.am is fed to make in its raw format before
## 'configure' has found the correct values (on the build machine).
bootstrap_files = \
    $(lt_Makefile_am) \
    $(ltmain_sh) \
    $(ltversion_m4)

.PHONY: bootstrap-deps bootstrap-deps-prep
bootstrap-deps: bootstrap-deps-prep
	$(MAKE) $(bootstrap_files)

bootstrap-deps-prep:
## The following variables are substituted by 'bootstrap-dep-preps'
	@exit_cmd=:; \
	test -z '$(srcdir)' \
	    && echo "ERROR: don't call $(MAKE) with srcdir unset." \
	    && exit_cmd=exit; \
	test -z '$(PACKAGE)' \
	    && echo "ERROR: don't call $(MAKE) with PACKAGE unset." \
	    && exit_cmd=exit; \
	test -z '$(PACKAGE_BUGREPORT)' \
	    && echo "ERROR: don't call $(MAKE) with PACKAGE_BUGREPORT unset." \
	    && exit_cmd=exit; \
	test -z '$(PACKAGE_NAME)' \
	    && echo "ERROR: don't call $(MAKE) with PACKAGE_NAME unset." \
	    && exit_cmd=exit; \
	test -z '$(PACKAGE_URL)' \
	    && echo "ERROR: don't call $(MAKE) with PACKAGE_URL unset." \
	    && exit_cmd=exit; \
	test -z '$(SED)' \
	    && echo "ERROR: don't call $(MAKE) with SED unset." \
	    && exit_cmd=exit; \
	test -z '$(VERSION)' \
	    && echo "ERROR: don't call $(MAKE) with VERSION unset." \
	    && exit_cmd=exit; \
	$$exit_cmd 1
	$(AM_V_at)rm -f $(bootstrap_files)

## Unfortunately, all this bogeyness means that we have to manually
## keep the generated files in libltdl up to date.
LTDL_BOOTSTRAP_DEPS = \
	$(stamp_mk) \
	$(lt_aclocal_m4) \
	$(lt_config_h_in) \
	$(lt_configure)

all-local: $(LTDL_BOOTSTRAP_DEPS)


## ---------------- ##
## Libtool scripts. ##
## ---------------- ##

abs_aux_dir = `$(lt__cd) '$(srcdir)/$(aux_dir)' && pwd`
ltdl_ac_aux_dir = `$(SCRIPT_ENV) $(extract_trace) AC_CONFIG_AUX_DIR $(srcdir)/libltdl/configure.ac`

configure_edit = $(bootstrap_edit) \
	-e '/^\. /s|@auxscriptsdir\@|'$(abs_aux_dir)'|g' \
	-e 's|@aclocaldir\@|$(aclocaldir)|g' \
	-e 's|@aux_dir\@|$(aux_dir)|g' \
	-e 's|@datadir\@|$(datadir)|g' \
	-e 's|@EGREP\@|$(EGREP)|g' \
	-e 's|@FGREP\@|$(FGREP)|g' \
	-e 's|@GREP\@|$(GREP)|g' \
	-e 's|@host_triplet\@|$(host_triplet)|g' \
	-e 's|@LN_S\@|$(LN_S)|g' \
	-e "s|@ltdl_ac_aux_dir\@|$(ltdl_ac_aux_dir)|g" \
	-e 's|@macro_dir\@|$(macro_dir)|g' \
	-e 's|@pkgauxdir\@|$(pkgauxdir)|g' \
	-e 's|@pkgaux_files\@|$(pkgaux_scripts) $(pkgaux_data_files)|g' \
	-e 's|@pkgaux_install_files\@|$(pkgaux_install_files)|g' \
	-e 's|@pkgaux_parent_files\@|$(pkgaux_parent_files)|g' \
	-e 's|@pkgdatadir\@|$(pkgdatadir)|g' \
	-e 's|@pkgltdl_files\@|$(pkgltdl_files)|g' \
	-e 's|@pkgmacro_files\@|$(pkgmacro_files)|g' \
	-e 's|@prefix\@|$(prefix)|g' \
	-e 's|@SED\@|$(SED)|g' \
	-e 's|@srcdir\@|$(srcdir)|g'

# The libtool distributor and the standalone libtool script.
bin_SCRIPTS = libtool

libtoolize: $(libtoolize_in) $(config_status)
	$(AM_V_at)rm -f '$@'
	$(AM_V_GEN)$(configure_edit) '$(libtoolize_in)' > '$@'
	$(AM_V_at)chmod a+x '$@'
	$(AM_V_at)chmod a-w '$@'

# We used to do this with a 'stamp-vcl' file, but non-gmake builds
# would rerun configure on every invocation, so now we manually
# check the version numbers from the build rule when necessary.
libtool: $(ltmain_sh) $(config_status) $(dotversion)
	@$(rebuild); \
	if test -f '$@'; then \
	  eval `'$(SED)' -n '/^package_revision=/p' '$@'`; \
	  test "$$package_revision" = "$$revision" && rebuild=false; \
	fi; \
	for prereq in $?; do \
	  case $$prereq in *.version);; *) rebuild=:;; esac; \
	done; \
	if $$rebuild; then \
	  if test 0 = '$(AM_DEFAULT_VERBOSITY)' && test 1 != '$(V)'; \
	    then echo "  GEN     " $@; \
	  else echo '$(SHELL) $(top_builddir)/config.status "$@"'; fi; \
	  cd '$(top_builddir)' && '$(SHELL)' ./config.status '$@'; \
	fi


## -------- ##
## Libltdl. ##
## -------- ##

include libltdl/ltdl.mk

lt_aclocal_m4	= $(srcdir)/$(ltdl_dir)/aclocal.m4
lt_config_h_in	= $(srcdir)/$(ltdl_dir)/config-h.in
lt_configure	= $(srcdir)/$(ltdl_dir)/configure
lt_configure_ac	= $(srcdir)/$(ltdl_dir)/configure.ac
stamp_mk	= $(srcdir)/$(ltdl_dir)/stamp-mk

lt_obsolete_m4	= $(srcdir)/$(macro_dir)/lt~obsolete.m4

EXTRA_DIST     += $(lt_aclocal_m4) \
		  $(lt_configure) \
		  $(lt_configure_ac) \
		  $(lt_config_h_in) \
		  $(lt_Makefile_am) \
		  $(lt_Makefile_in) \
		  $(lt_obsolete_m4) \
		  $(stamp_mk)

$(lt_Makefile_in): $(lt_Makefile_am) $(lt_aclocal_m4)
	$(AM_V_GEN)cd '$(srcdir)/$(ltdl_dir)' && $(AUTOMAKE) Makefile

# Don't let unused scripts leak into the libltdl Makefile
$(stamp_mk): $(lt_Makefile_in)
	$(AM_V_at)T='$(srcdir)/$(aux_dir)/Makefile.tmp'; \
	for p in $(srcdir)/$(aux_dir)/*; do \
	  test -f "$$p" || continue; \
	  f=`echo "$$p" | $(SED) 's|^$(srcdir)/$(aux_dir)/||'`; \
	  case " $(pkgaux_scripts) ltmain.sh " in \
	    *" $$f "*) ;; \
	    *) '$(SED)' \
		  -e 's|\(\$$([^)]*)/\)*'"$(ltdl_ac_aux_dir)/$$f"'\$$||' \
		  -e 's|\(\$$([^)]*)/\)*'"$(ltdl_ac_aux_dir)/$$f"' ||' \
		  -e '/^[	 ]*\\$$/d' \
		'$(lt_Makefile_in)' > "$$T" \
		  && mv -f "$$T" '$(lt_Makefile_in)';; \
	  esac; \
	done
	$(AM_V_GEN)echo stamp > '$@'

lt_aclocal_m4_deps = \
	$(lt_obsolete_m4) \
	$(ltversion_m4) \
	$(libtool_m4) \
	$(srcdir)/$(macro_dir)/ltargz.m4 \
	$(srcdir)/$(macro_dir)/ltdl.m4 \
	$(srcdir)/$(macro_dir)/ltoptions.m4 \
	$(srcdir)/$(macro_dir)/ltsugar.m4 \
	$(srcdir)/$(macro_dir)/m4.m4 \
	$(srcdir)/$(ltdl_dir)/configure.ac

lt_configure_deps = $(lt_aclocal_m4) $(lt_aclocal_m4_deps)

$(lt_aclocal_m4): $(lt_aclocal_m4_deps)
	$(AM_V_GEN)cd '$(srcdir)/$(ltdl_dir)' && $(ACLOCAL) -I ../m4

$(lt_configure): $(lt_configure_deps)
	$(AM_V_GEN)cd '$(srcdir)/$(ltdl_dir)' && $(AUTOCONF)

$(lt_config_h_in): $(lt_configure_deps)
	$(AM_V_GEN)cd '$(srcdir)/$(ltdl_dir)' && $(AUTOHEADER)
	$(AM_V_at)touch '$@'


## -------------- ##
## Documentation. ##
## -------------- ##

doc_dir		= $(srcdir)/doc

libtool_1	= $(doc_dir)/libtool.1
libtoolize_1	= $(doc_dir)/libtoolize.1
notes_texi	= $(doc_dir)/notes.texi
notes_txt	= $(doc_dir)/notes.txt

EXTRA_DIST	+= $(doc_dir)/gendocs_template

# A bug in automake 1.11.1 (at least) prevents us from using:
#
#   info_TEXINFOS	= $(doc_dir)/libtool.texi
#
# Producing the following error, even though srcdir is implicitly set:
# "cannot open < ./$(srcdir)/doc/libtool.texi: No such file or directory"
info_TEXINFOS		= doc/libtool.texi
doc_libtool_TEXINFOS	= $(doc_dir)/PLATFORMS $(doc_dir)/fdl.texi \
			  $(notes_texi)
EXTRA_DIST	       += $(notes_txt)
## Workaround for a texinfo bug described here:
## http://thread.gmane.org/gmane.comp.tex.texinfo.bugs/3930
DISTCLEANFILES	       += libtool.dvi

all-local: $(notes_txt)

$(notes_txt): $(notes_texi)
	$(AM_V_GEN)$(MAKEINFO) -P '$(srcdir)/doc' --no-headers \
	    $(MAKEINFOFLAGS) -o '$@' '$(notes_texi)'

dist_man1_MANS		= $(libtool_1) $(libtoolize_1)
MAINTAINERCLEANFILES	+= $(dist_man1_MANS)
update_mans = \
  PATH=".$(PATH_SEPARATOR)$$PATH"; export PATH; \
  $(HELP2MAN) --output='$@'

# It's wrong to make distributed files (e.g. $(libtool_1)) rely on
# files created in the build tree, so instead we regenerate the
# manual pages if the sources for the build-tree files we want to
# run have changed.
$(libtool_1): $(ltmain_sh)
	$(AM_V_GEN)$(update_mans) --help-option=--help-all libtool
$(libtoolize_1): $(libtoolize_in)
	$(AM_V_GEN)$(update_mans) libtoolize


## ------------- ##
## Installation. ##
## ------------- ##

ltdldir		= $(srcdir)/libltdl
pkgauxdir	= $(pkgdatadir)/build-aux

# The timestamps on these files must be preserved carefully so we install,
# uninstall and set executable with custom rules here.
pkgaux_parent_files	= ltmain.sh
pkgaux_install_files	= config.guess config.sub install-sh
pkgaux_data_files	= $(pkgaux_parent_files)

# Everything that gets picked up by aclocal is automatically distributed,
# this is the list of macro files we install on the user's system.
pkgmacro_files	= libtool.m4 ltargz.m4 ltdl.m4 ltoptions.m4 ltsugar.m4 \
		  ltversion.m4 lt~obsolete.m4

## These are installed as a subdirectory of pkgdatadir so that
## libtoolize --ltdl can find them later. Note that this list requires
## specific order to avoid unnecessary re-autotooling after libtoolize run.
pkgltdl_files	= COPYING.LIB \
		  Makefile.am \
		  README \
		  configure.ac \
		  aclocal.m4 \
		  Makefile.in \
		  config-h.in \
		  configure \
		  libltdl/lt__alloc.h \
		  libltdl/lt__argz_.h \
		  libltdl/lt__dirent.h \
		  libltdl/lt__glibc.h \
		  libltdl/lt__private.h \
		  libltdl/lt__strl.h \
		  libltdl/lt_dlloader.h \
		  libltdl/lt_error.h \
		  libltdl/lt_system.h \
		  libltdl/slist.h \
		  loaders/dld_link.c \
		  loaders/dlopen.c \
		  loaders/dyld.c \
		  loaders/load_add_on.c \
		  loaders/loadlibrary.c \
		  loaders/preopen.c \
		  loaders/shl_load.c \
		  lt__alloc.c \
		  lt__argz.c \
		  lt__dirent.c \
		  lt__strl.c \
		  lt_dlloader.c \
		  lt_error.c \
		  ltdl.c \
		  ltdl.h \
		  ltdl.mk \
		  slist.c

install-data-local: $(lt_Makefile_in) install-scripts-local
	@$(NORMAL_INSTALL)
## Don't install over the top of an old pkgdatadir
	-rm -rf '$(DESTDIR)$(pkgdatadir)'/*
## First, put a copy of the libtool m4 macros in the aclocal dir
	@list='$(pkgmacro_files)'; for p in $$list; do \
	  d=`echo "$(DESTDIR)$(aclocaldir)/$$p" |$(SED) 's|[^/]*$$||'`; \
	  test -d "$$d" || $(mkinstalldirs) "$$d"; \
	  echo " $(INSTALL_DATA) '$(srcdir)/$(macro_dir)/$$p' '$(DESTDIR)$(aclocaldir)/$$p'"; \
	  $(INSTALL_DATA) "$(srcdir)/$(macro_dir)/$$p" "$(DESTDIR)$(aclocaldir)/$$p"; \
	done
## install the helper scripts
	@list='$(pkgaux_scripts)' && \
	for p in $$list; do \
	  d=`echo "$(DESTDIR)$(pkgauxdir)/$$p" |$(SED) 's|[^/]*$$||'`; \
	  test -d "$$d" || $(mkinstalldirs) "$$d"; \
	  echo " $(INSTALL_SCRIPT) '$(srcdir)/$(aux_dir)/$$p' '$(DESTDIR)$(pkgauxdir)/$$p'"; \
	  $(INSTALL_SCRIPT) "$(srcdir)/$(aux_dir)/$$p" "$(DESTDIR)$(pkgauxdir)/$$p"; \
	done
	@list='$(pkgaux_data_files)' && for p in $$list; do \
	  d=`echo "$(DESTDIR)$(pkgauxdir)/$$p" |$(SED) 's|[^/]*$$||'`; \
	  test -d "$$d" || $(mkinstalldirs) "$$d"; \
	  echo " $(INSTALL_DATA) '$(srcdir)/$(aux_dir)/$$p' '$(DESTDIR)$(pkgauxdir)/$$p'"; \
	  $(INSTALL_DATA) "$(srcdir)/$(aux_dir)/$$p" "$(DESTDIR)$(pkgauxdir)/$$p"; \
	done
## install the libltdl files
	@list='$(pkgltdl_files)' && for p in $$list; do \
	  d=`echo "$(DESTDIR)$(pkgdatadir)/$$p" |$(SED) 's|[^/]*$$||'`; \
	  test -d "$$d" || $(mkinstalldirs) "$$d"; \
	  echo " $(INSTALL_DATA) '$(ltdldir)/$$p' '$(DESTDIR)$(pkgdatadir)/$$p'"; \
	  $(INSTALL_DATA) "$(ltdldir)/$$p" "$(DESTDIR)$(pkgdatadir)/$$p"; \
	done
	chmod a+x '$(DESTDIR)$(pkgdatadir)/configure'

.PHONY: install-scripts-local
install-scripts-local: $(lt_Makefile_in)
## Inline helper-scripts for installed libtoolize script
	@p=`echo libtoolize |sed -e '$(transform)'`; \
	echo " $(SCRIPT_ENV) '$(inline_source)' libtoolize > '$(DESTDIR)$(bindir)/$$p'"; \
	d=`echo "$(DESTDIR)$(bindir)/$$p" |$(SED) 's|[^/]*$$||'`; \
	test -d "$$d" || $(mkinstalldirs) "$$d"; \
	$(SCRIPT_ENV) '$(inline_source)' libtoolize > "$(DESTDIR)$(bindir)/$$p"; \
	chmod a+x "$(DESTDIR)$(bindir)/$$p"


## ------------- ##
## Distribution. ##
## ------------- ##

edit_readme_alpha	= $(srcdir)/$(aux_dir)/edit-readme-alpha
gitlog_to_changelog	= $(srcdir)/$(aux_dir)/gitlog-to-changelog
git_log_fix		= $(srcdir)/$(aux_dir)/git-log-fix
thanks_gen		= $(srcdir)/$(aux_dir)/thanks-gen

dotserial		= $(distdir)/.serial
dotversion		= $(top_srcdir)/.version
tarball_version		= $(distdir)/.tarball-version
readme			= $(distdir)/README
changelog		= $(distdir)/ChangeLog
changelog_old		= $(srcdir)/ChangeLog.old
thanks			= $(distdir)/THANKS
no_thanks		= $(srcdir)/NO-THANKS

# Generate ChangeLog using git log entries for as far back as
# they are in good shape, appending manual records from earlier.
changelog_start_date = 2011-01-01
$(changelog): FORCE
	$(AM_V_GEN)if test -d '$(srcdir)/.git'; then \
	  TZ=UTC $(gitlog_to_changelog) --amend=$(git_log_fix) \
	      --ignore-matching=Omit-from-ChangeLog. \
	      --format='%s%n%n%b%n' \
	      --since=$(changelog_start_date) > '$@T'; \
	  rm -f '$@'; mv '$@T' '$@'; \
	  cat '$(changelog_old)' >> '$@'; \
	fi

# Sort in traditional ASCII order, regardless of the current locale;
# otherwise we may get into trouble with distinct strings that the
# current locale considers to be equal.
ASSORT = LC_ALL=C sort

# Extract all lines up to the first one starting with "##".
prologue = perl -ne '/^\#\#/ and exit; print' $(no_thanks)

# Generate THANKS using git log entries as far as possible, fixing
# up ommisions and errors from NO-THANKS configuration.
$(thanks): FORCE
	$(AM_V_GEN)if test -d '$(srcdir)/.git'; then \
	  { \
	    $(prologue); echo; \
	    { perl -ne '/^$$/.../^$$/ and print' $(no_thanks) \
	      | grep -v '^$$' | perl -pe 's/  +/\0/'; \
	      {  sed -e '1,/\#\# /d' -e '/^\#\# /d' \
		    -e 's|[	][	 ]*|	|' < $(no_thanks) \
		  | tr '\t' '\0'; \
		git log --pretty=format:'%aN%x00%aE'; \
	      } | $(ASSORT) -u; \
	    } | $(thanks_gen) \
	      | LC_ALL=en_US.UTF-8 sort -f; \
	    echo; \
	    printf ';; %s\n' 'Local Variables:' 'coding: utf-8' End:; \
	  } > '$@'; \
	fi

## Arrange so that .version appears only in the distribution
## tarball, and never in a checked-out repository.
EXTRA_DIST += $(dotversion)
BUILT_SOURCES += $(dotversion)
$(dotversion):
	$(AM_V_GEN)echo '$(VERSION)' > '$@T' && mv '$@T' '$@'

## Edit the README file for alpha releases.
EXTRA_DIST += $(edit_readme_alpha)
re_alpha_version = '\([0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*[-\.][-\.0-9a-z]*\)'
$(readme): FORCE
	@if test -n "`expr $(VERSION) : $(re_alpha_version)`"; then \
	  if test 0 = '$(AM_DEFAULT_VERBOSITY)' && test 1 != '$(V)'; \
	    then echo "  GEN     " $@; \
	  else echo "$(SHELL) $(edit_readme_alpha) $@"; fi; \
	  $(SHELL) $(edit_readme_alpha) '$@'; \
	fi

git_commit_count = git log --pretty=oneline |wc -l |$(SED) 's|[	 ]||g'

dist-hook: $(changelog) $(thanks) $(dotversion) $(readme)
## Arrange so that .tarball-version appears only in the distribution
## tarball, and never in a checked-out repository.
	echo '$(VERSION)' > $(tarball_version)
## ...and similarly for .serial.
	$(git_commit_count) > $(dotserial)
## Ensure aclocal has not wrongly picked up old macro definitions.
	$(no_bogus_macros)

distcheck-hook: syntax-check

uninstall-hook:
	@$(NORMAL_UNINSTALL)
	@list='$(pkgltdl_files)'; \
	for f in $$list; do \
	  echo " rm -f '$(DESTDIR)$(pkgdatadir)/$$f'"; \
	  rm -f "$(DESTDIR)$(pkgdatadir)/$$f"; \
	done
	@list='$(pkgaux_scripts) $(pkgaux_data_files)'; \
	for f in $$list; do \
	  echo " rm -f '$(DESTDIR)$(pkgauxdir)/$$f'"; \
	  rm -f "$(DESTDIR)$(pkgauxdir)/$$f"; \
	done
	@for p in $(pkgmacro_files); do \
	  f=`echo "$$p" |$(SED) 's|^.*/||'`; \
	  echo " rm -f '$(DESTDIR)$(aclocaldir)/$$f'"; \
	  rm -f "$(DESTDIR)$(aclocaldir)/$$f"; \
	done
	@p=`echo libtoolize |sed -e '$(transform)'`; \
	echo " rm -f '$(DESTDIR)$(bindir)/$$p'"; \
	rm -f "$(DESTDIR)$(bindir)/$$p"


## ----------- ##
## Test suite. ##
## ----------- ##

tests_dir	= $(top_srcdir)/tests
package_m4	= $(tests_dir)/package.m4
testsuite	= $(tests_dir)/testsuite

# The testsuite files are evaluated in the order given here. When
# adding new tests, make sure cmdline_wrap.at stays at the end so
# that it can check for previous failures and skip if necessary.
TESTSUITE	= tests/testsuite
TESTSUITE_AT	= tests/testsuite.at \
		  tests/configure-funcs.at \
		  tests/libtoolize.at \
		  tests/libtool.at \
		  tests/demo.at \
		  tests/pic_flag.at \
		  tests/with-pic.at \
		  tests/cdemo.at \
		  tests/convenience.at \
		  tests/depdemo.at \
		  tests/help.at \
		  tests/duplicate_members.at \
		  tests/duplicate_conv.at \
		  tests/duplicate_deps.at \
		  tests/flags.at \
		  tests/inherited_flags.at \
		  tests/link-order.at \
		  tests/link-order2.at \
		  tests/fail.at \
		  tests/shlibpath.at \
		  tests/runpath-in-lalib.at \
		  tests/static.at \
		  tests/export.at \
		  tests/export-def.at \
		  tests/search-path.at \
		  tests/indirect_deps.at \
		  tests/archive-in-archive.at \
		  tests/exeext.at \
		  tests/execute-mode.at \
		  tests/bindir.at \
		  tests/cwrapper.at \
		  tests/deplib-in-subdir.at \
		  tests/infer-tag.at \
		  tests/localization.at \
		  tests/nocase.at \
		  tests/install.at \
		  tests/versioning.at \
		  tests/destdir.at \
		  tests/old-m4-iface.at \
		  tests/am-subdir.at \
		  tests/lt_dlexit.at \
		  tests/lt_dladvise.at \
		  tests/lt_dlopen.at \
		  tests/lt_dlopen_a.at \
		  tests/lt_dlopenext.at \
		  tests/ltdl-libdir.at \
		  tests/ltdl-api.at \
		  tests/dlloader-api.at \
		  tests/loadlibrary.at \
		  tests/lalib-syntax.at \
		  tests/resident.at \
		  tests/slist.at \
		  tests/need_lib_prefix.at \
		  tests/mdemo.at \
		  tests/standalone.at \
		  tests/subproject.at \
		  tests/nonrecursive.at \
		  tests/recursive.at \
		  tests/tagdemo.at \
		  tests/template.at \
		  tests/ctor.at \
		  tests/exceptions.at \
		  tests/early-libtool.at \
		  tests/no-executables.at \
		  tests/deplibs-ident.at \
		  tests/configure-iface.at \
		  tests/f77demo.at \
		  tests/fcdemo.at \
		  tests/darwin.at \
		  tests/dumpbin-symbols.at \
		  tests/deplibs-mingw.at \
		  tests/sysroot.at \
		  tests/stresstest.at \
		  tests/cmdline_wrap.at \
		  $(NOTHING_ELSE)

EXTRA_DIST     += $(testsuite) $(TESTSUITE_AT) $(package_m4)

# Be sure to reexport important environment variables:
TESTS_ENVIRONMENT = MAKE="$(MAKE)" CC="$(CC)" CFLAGS="$(CFLAGS)" \
	CPP="$(CPP)" CPPFLAGS="$(CPPFLAGS)" LD="$(LD)" LDFLAGS="$(LDFLAGS)" \
	LIBS="$(LIBS)" LN_S="$(LN_S)" NM="$(NM)" RANLIB="$(RANLIB)" \
	AR="$(AR)" \
	EGREP="$(EGREP)" FGREP="$(FGREP)" GREP="$(GREP)" SED="$(SED)" \
	STRIP="$(STRIP)" lt_INSTALL="$(INSTALL)" \
	MANIFEST_TOOL="$(MANIFEST_TOOL)" \
	OBJEXT="$(OBJEXT)" EXEEXT="$(EXEEXT)" \
	SHELL="$(SHELL)" CONFIG_SHELL="$(SHELL)" \
	CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS)" CXXCPP="$(CXXCPP)" \
	F77="$(F77)" FFLAGS="$(FFLAGS)" \
	FC="$(FC)" FCFLAGS="$(FCFLAGS)" \
	GCJ="$(GCJ)" GCJFLAGS="$(GCJFLAGS)" \
	lt_cv_with_aix_soname="$(with_aix_soname)" \
	lt_cv_to_host_file_cmd="$(to_host_file_cmd)" \
	lt_cv_to_tool_file_cmd="$(to_tool_file_cmd)"

BUILDCHECK_ENVIRONMENT = _lt_pkgdatadir="$(abs_top_srcdir)" \
	LIBTOOLIZE="$(abs_top_builddir)/libtoolize" \
	LIBTOOL="$(abs_top_builddir)/libtool" \
	tst_aclocaldir="$(abs_top_srcdir)/m4"

INSTALLCHECK_ENVIRONMENT = \
	LIBTOOLIZE="$(bindir)/`echo libtoolize |$(SED) '$(program_transform_name)'`" \
	LIBTOOL="$(bindir)/`echo libtool |$(SED) '$(program_transform_name)'`" \
	LTDLINCL="-I$(includedir)" \
	LIBLTDL="$(libdir)/libltdl.la" \
	tst_aclocaldir="$(aclocaldir)"

$(testsuite): $(package_m4) $(TESTSUITE_AT) Makefile.am
	$(AM_V_GEN)$(AUTOTEST) -I '$(srcdir)' -I '$(srcdir)/tests' $(TESTSUITE_AT) -o '$@'

$(package_m4): $(dotversion) Makefile.am
	$(AM_V_GEN){ \
	  echo '# Signature of the current package.'; \
	  echo 'm4_define([AT_PACKAGE_NAME],      [$(PACKAGE_NAME)])'; \
	  echo 'm4_define([AT_PACKAGE_TARNAME],   [$(PACKAGE_TARNAME)])'; \
	  echo 'm4_define([AT_PACKAGE_VERSION],   [$(PACKAGE_VERSION)])'; \
	  echo 'm4_define([AT_PACKAGE_STRING],    [$(PACKAGE_STRING)])'; \
	  echo 'm4_define([AT_PACKAGE_BUGREPORT], [$(PACKAGE_BUGREPORT)])'; \
	  echo 'm4_define([AT_PACKAGE_URL],       [$(PACKAGE_URL)])'; \
	} > '$@'

tests/atconfig: $(config_status)
	$(AM_V_GEN)$(SHELL) '$(config_status)' '$@'

DISTCLEANFILES += tests/atconfig

CD_TESTDIR	= abs_srcdir=`$(lt__cd) $(srcdir) && pwd`; cd tests

testsuite_deps = tests/atconfig $(srcdir)/$(TESTSUITE)
testsuite_deps_uninstalled = $(testsuite_deps) libltdl/libltdlc.la \
			     $(bin_SCRIPTS) $(LTDL_BOOTSTRAP_DEPS)

# Hook the test suite into the check rule
check-local: $(testsuite_deps_uninstalled)
	$(AM_V_at)$(CD_TESTDIR); \
	CONFIG_SHELL='$(SHELL)' '$(SHELL)' "$$abs_srcdir/$(TESTSUITE)" \
	  $(TESTS_ENVIRONMENT) $(BUILDCHECK_ENVIRONMENT) $(TESTSUITEFLAGS)

# Run the test suite on the *installed* tree.
installcheck-local: $(testsuite_deps)
	$(AM_V_at)$(CD_TESTDIR); \
	CONFIG_SHELL='$(SHELL)' '$(SHELL)' "$$abs_srcdir/$(TESTSUITE)" \
	  $(TESTS_ENVIRONMENT) $(INSTALLCHECK_ENVIRONMENT) $(TESTSUITEFLAGS) \
	  AUTOTEST_PATH='$(exec_prefix)/bin'

.PHONY: check-noninteractive-old
check-noninteractive-old:
	$(AM_V_at)'$(MAKE)' $(AM_MAKEFLAGS) check-TESTS TESTS='$(TESTS)'

# Run only noninteractive parts of the new testsuite.
.PHONY: check-noninteractive-new
check-noninteractive-new: $(testsuite_deps_uninstalled)
	$(AM_V_at)$(CD_TESTDIR); \
	CONFIG_SHELL='$(SHELL)' '$(SHELL)' "$$abs_srcdir/$(TESTSUITE)" \
	  $(TESTS_ENVIRONMENT) $(BUILDCHECK_ENVIRONMENT) \
	  -k '!interactive' INNER_TESTSUITEFLAGS=',!interactive' \
	  $(TESTSUITEFLAGS)

# Run only interactive parts of the new testsuite.
.PHONY: check-interactive
check-interactive: $(testsuite_deps_uninstalled)
	$(AM_V_at)$(CD_TESTDIR); \
	CONFIG_SHELL='$(SHELL)' '$(SHELL)' "$$abs_srcdir/$(TESTSUITE)" \
	  $(TESTS_ENVIRONMENT) $(BUILDCHECK_ENVIRONMENT) \
	  -k interactive -k recursive INNER_TESTSUITEFLAGS=',interactive' \
	  $(TESTSUITEFLAGS)

.PHONY: check-noninteractive
check-noninteractive: check-noninteractive-old check-noninteractive-new

# We need to remove any file droppings left behind by testsuite
clean-local:
	-$(CD_TESTDIR); \
	test -f "$$abs_srcdir/$(TESTSUITE)" && \
	    '$(SHELL)' "$$abs_srcdir/$(TESTSUITE)" --clean

## An empty target to depend on when a rule needs to always run
## whenever it is visited.
FORCE:
