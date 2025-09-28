## Process this file with automake to generate Makefile.in
#
#   Copyright (C) 2021-2023 Free Software Foundation, Inc.
#
# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING3.  If not see
# <http://www.gnu.org/licenses/>.

ACLOCAL_AMFLAGS = -I . -I .. 

AUTOMAKE_OPTIONS = dejagnu foreign

if BUILD_COLLECTOR
    COLLECTOR_SUBDIRS = libcollector
endif
if BUILD_DOC
    DOC_SUBDIR = doc
endif
if BUILD_SRC
    SRC_SUBDIRS = src gp-display-html $(DOC_SUBDIR)
endif
SUBDIRS = $(COLLECTOR_SUBDIRS) $(SRC_SUBDIRS)
DIST_SUBDIRS = libcollector src gp-display-html $(DOC_SUBDIR)

# Setup the testing framework, if you have one
EXPECT = expect
RUNTEST = runtest
RUNTESTFLAGS =

BASEDIR = $(srcdir)/..
BFDDIR = $(BASEDIR)/bfd
jdk_inc = @jdk_inc@
LD_NO_AS_NEEDED = @LD_NO_AS_NEEDED@
GPROFNG_CFLAGS = @GPROFNG_CFLAGS@
GPROFNG_CPPFLAGS = @GPROFNG_CPPFLAGS@
GPROFNG_BROKEN_JAVAC = @GPROFNG_BROKEN_JAVAC@

AM_MAKEFLAGS = \
	jdk_inc="$(jdk_inc)" \
	LD_NO_AS_NEEDED="$(LD_NO_AS_NEEDED)" \
	GPROFNG_CFLAGS="$(GPROFNG_CFLAGS)" \
	GPROFNG_CPPFLAGS="$(GPROFNG_CPPFLAGS)"

if TCL_TRY
.PHONY: check-small check-extra check-install

check-DEJAGNU: check-small

check-small check-extra check-install: site.exp development.exp
	srcroot=`cd $(srcdir) && pwd`; export srcroot; \
	LC_ALL=C; export LC_ALL; \
	EXPECT=$(EXPECT); export EXPECT; \
	jdk_inc="$(jdk_inc)"; export jdk_inc; \
	runtest=$(RUNTEST); \
	if $(SHELL) -c "$$runtest --version" > /dev/null 2>&1; then \
	  $$runtest --tool $(DEJATOOL) --srcdir $${srcroot}/testsuite \
		JDK_INC="$(jdk_inc)" \
		CLOCK_GETTIME_LINK="$(CLOCK_GETTIME_LINK)" \
		CHECK_TARGET=$@ \
		PREFIX="$(prefix)" \
		GPROFNG_BROKEN_JAVAC="$(GPROFNG_BROKEN_JAVAC)" \
		MAKE="$(MAKE)" CC="$(CC)" CFLAGS="$(CFLAGS) $(PTHREAD_CFLAGS)" \
		LDFLAGS="$(LDFLAGS)" LIBS="$(PTHREAD_LIBS) $(LIBS)" \
		BUILDDIR="$(abs_top_builddir)" $(RUNTESTFLAGS); \
	else echo "WARNING: could not find \`runtest'" 1>&2; :;\
	fi

development.exp: $(BFDDIR)/development.sh
	$(EGREP) "(development|experimental)=" $(BFDDIR)/development.sh  \
	  | $(AWK) -F= '{ print "set " $$1 " " $$2 }' > $@

# development.sh is used to determine -Werror default.
CONFIG_STATUS_DEPENDENCIES = $(BFDDIR)/development.sh

EXTRA_DEJAGNU_SITE_CONFIG = development.exp

DISTCLEANFILES = site.exp development.exp
endif

