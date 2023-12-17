## Makefile for the toplevel directory of GNU gettext
## Copyright (C) 1995-2023 Free Software Foundation, Inc.
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

AUTOMAKE_OPTIONS = 1.5 gnu no-dependencies
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = gnulib-local gettext-runtime libtextstyle gettext-tools

changelog_etc = \
  gettext-runtime/ChangeLog.0 \
  gettext-runtime/doc/ChangeLog.0 \
  gettext-runtime/intl-csharp/ChangeLog.0 \
  gettext-runtime/intl-java/ChangeLog.0 \
  gettext-runtime/libasprintf/ChangeLog.0 \
  gettext-runtime/m4/ChangeLog.0 \
  gettext-runtime/man/ChangeLog.0 \
  gettext-runtime/src/ChangeLog.0 \
  gettext-runtime/tests/ChangeLog.0 \
  gettext-tools/ChangeLog.0 \
  gettext-tools/doc/ChangeLog.0 \
  gettext-tools/doc/ChangeLog.1 \
  gettext-tools/examples/ChangeLog.0 \
  gettext-tools/gnulib-tests/ChangeLog.0 \
  gettext-tools/libgettextpo/ChangeLog.0 \
  gettext-tools/libgrep/ChangeLog.0 \
  gettext-tools/m4/ChangeLog.0 \
  gettext-tools/man/ChangeLog.0 \
  gettext-tools/man/ChangeLog.1 \
  gettext-tools/misc/ChangeLog.0 \
  gettext-tools/projects/ChangeLog.0 \
  gettext-tools/src/ChangeLog.0 \
  gettext-tools/src/ChangeLog.1 \
  gettext-tools/styles/ChangeLog.0 \
  gettext-tools/tests/ChangeLog.0 \
  gettext-tools/tests/ChangeLog.1 \
  gnulib-local/ChangeLog.0

EXTRA_DIST = \
  $(changelog_etc) DEPENDENCIES PACKAGING HACKING JOIN-GNU ChangeLog.0 \
  autogen.sh \
  check-copyright-headers \
  build-aux/ac-help.sed build-aux/git-version-gen build-aux/texi2html \
  m4/fixautomake.m4 m4/woe32-dll.m4 \
  m4/libtool.m4

# Additional dependencies for configure, due to the use of autoconf --trace.
$(srcdir)/configure: $(srcdir)/gettext-runtime/configure.ac $(srcdir)/gettext-tools/configure.ac 

distcheck-hook:
# Verify that some files are the same.
	cmp -s $(srcdir)/gettext-runtime/po/Makefile.in.in $(srcdir)/gettext-tools/po/Makefile.in.in
	cmp -s $(srcdir)/gettext-runtime/po/Rules-quot $(srcdir)/gettext-tools/po/Rules-quot
	cmp -s $(srcdir)/gettext-runtime/po/boldquot.sed $(srcdir)/gettext-tools/po/boldquot.sed
	cmp -s $(srcdir)/gettext-runtime/po/quot.sed $(srcdir)/gettext-tools/po/quot.sed
	cmp -s $(srcdir)/gettext-runtime/po/en@quot.header $(srcdir)/gettext-tools/po/en@quot.header
	cmp -s $(srcdir)/gettext-runtime/po/en@boldquot.header $(srcdir)/gettext-tools/po/en@boldquot.header
	cmp -s $(srcdir)/gettext-runtime/po/insert-header.sin $(srcdir)/gettext-tools/po/insert-header.sin
	cmp -s $(srcdir)/gettext-runtime/po/remove-potcdate.sin $(srcdir)/gettext-tools/po/remove-potcdate.sin
	cmp -s $(srcdir)/gettext-runtime/po/remove-potcdate.sin $(srcdir)/gettext-tools/examples/po/remove-potcdate.sin
	cmp -s $(srcdir)/gettext-runtime/m4/build-to-host.m4 $(srcdir)/gettext-tools/gnulib-m4/build-to-host.m4
	cmp -s $(srcdir)/gettext-runtime/m4/fcntl-o.m4 $(srcdir)/gettext-tools/gnulib-m4/fcntl-o.m4
	cmp -s $(srcdir)/gettext-runtime/m4/flexmember.m4 $(srcdir)/gettext-tools/gnulib-m4/flexmember.m4
	cmp -s $(srcdir)/gettext-runtime/m4/gettext.m4 $(srcdir)/gettext-tools/gnulib-m4/gettext.m4
	cmp -s $(srcdir)/gettext-runtime/m4/iconv.m4 $(srcdir)/gettext-tools/gnulib-m4/iconv.m4
	cmp -s $(srcdir)/gettext-runtime/m4/intlmacosx.m4 $(srcdir)/gettext-tools/gnulib-m4/intlmacosx.m4
	cmp -s $(srcdir)/gettext-runtime/m4/inttypes_h.m4 $(srcdir)/gettext-tools/gnulib-m4/inttypes_h.m4
	cmp -s $(srcdir)/gettext-runtime/m4/nls.m4 $(srcdir)/gettext-tools/gnulib-m4/nls.m4
	cmp -s $(srcdir)/gettext-runtime/m4/po.m4 $(srcdir)/gettext-tools/gnulib-m4/po.m4
	cmp -s $(srcdir)/gettext-runtime/m4/progtest.m4 $(srcdir)/gettext-tools/gnulib-m4/progtest.m4
	cmp -s $(srcdir)/gettext-runtime/m4/size_max.m4 $(srcdir)/gettext-tools/gnulib-m4/size_max.m4
	cmp -s $(srcdir)/gettext-runtime/m4/stdint_h.m4 $(srcdir)/gettext-tools/gnulib-m4/stdint_h.m4
	cmp -s $(srcdir)/gettext-runtime/m4/xsize.m4 $(srcdir)/gettext-tools/gnulib-m4/xsize.m4
	cmp -s $(srcdir)/gettext-runtime/man/help2man $(srcdir)/gettext-tools/man/help2man
	cmp -s $(srcdir)/gettext-runtime/man/x-to-1.in $(srcdir)/gettext-tools/man/x-to-1.in
	cmp -s $(srcdir)/libtextstyle/m4/libtextstyle.m4 $(srcdir)/gettext-tools/gnulib-m4/libtextstyle.m4
	cmp -s $(srcdir)/gettext-tools/examples/hello-java-awt/m4/TestAWT.java $(srcdir)/gettext-tools/examples/hello-java-swing/m4/TestAWT.java
	cmp -s $(srcdir)/gettext-tools/examples/hello-java-awt/m4/TestAWT.class $(srcdir)/gettext-tools/examples/hello-java-swing/m4/TestAWT.class
# Verify that all files have appropriate copyright headers.
	if test -d '$(distdir)'; then $(srcdir)/check-copyright-headers '$(distdir)'; fi

# OS/2 port.

EXTRA_DIST += os2/README.OS2 os2/Makefile os2/backward.def os2/configure.awk


# Windows support.

EXTRA_DIST += INSTALL.windows windows/windres-options windows/gettext.rc


# AIX support.

EXTRA_DIST += INSTALL.aix


# Versioning based on Git release tags.

EXTRA_DIST += $(top_srcdir)/.version
BUILT_SOURCES = $(top_srcdir)/.version
$(top_srcdir)/.version:
	echo $(VERSION) > $@-t && mv $@-t $@
dist-hook: gen-ChangeLogs
	echo $(VERSION) > $(distdir)/.tarball-version


# Generate ChangeLog.

gen_start_date = 2015-10-13
.PHONY: gen-ChangeLogs
gen-ChangeLogs:
	$(AM_V_GEN)if test -d .git; then				\
	  log_fix="$(srcdir)/build-aux/git-log-fix";			\
	  test -e "$$log_fix"						\
	    && amend_git_log="--amend=$$log_fix"			\
	    || amend_git_log=;						\
	  for d in . gettext-runtime/intl gettext-runtime/po		\
		gettext-tools/po; do					\
	    test "$$d" = "."						\
	      && cldir="$(distdir)"					\
	      || cldir="$(distdir)/$$d";				\
	    $(top_srcdir)/build-aux/gitlog-to-changelog			\
	      $$amend_git_log --since=$(gen_start_date) -- "$$d"	\
		> "$$cldir/cl-t" &&					\
	    { rm -f "$$cldir/ChangeLog" &&				\
	      mv "$$cldir/cl-t" "$$cldir/ChangeLog"; };			\
	  done;								\
	fi


# PO files update.

maintainer-update-po-local: $(top_builddir)/config.status
