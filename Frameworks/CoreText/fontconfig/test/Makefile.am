# 
#  test/Makefile.am
# 
#  Copyright © 2003 Keith Packard
# 
#  Permission to use, copy, modify, distribute, and sell this software and its
#  documentation for any purpose is hereby granted without fee, provided that
#  the above copyright notice appear in all copies and that both that
#  copyright notice and this permission notice appear in supporting
#  documentation, and that the name of the author(s) not be used in
#  advertising or publicity pertaining to distribution of the software without
#  specific, written prior permission.  The authors make no
#  representations about the suitability of this software for any purpose.  It
#  is provided "as is" without express or implied warranty.
# 
#  THE AUTHOR(S) DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
#  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
#  EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR
#  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
#  DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
#  TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
#  PERFORMANCE OF THIS SOFTWARE.

check_SCRIPTS=run-test.sh
TEST_EXTENSIONS = \
	.sh \
	$(NULL)

AM_TESTS_ENVIRONMENT= \
	src=${srcdir}; export src; \
	EXEEXT=${EXEEXT}; export EXEEXT; \
	LOG_COMPILER=${LOG_COMPILER} ; export LOG_COMPILER; \
	$(NULL)

BUILT_SOURCES = $(builddir)/out.expected

SH_LOG_COMPILER = sh
if OS_WIN32
LOG_COMPILER = ${srcdir}/wrapper-script.sh
endif
TESTS=run-test.sh

TESTDATA =			\
	4x6.pcf			\
	8x16.pcf		\
	fonts.conf.in		\
	test-45-generic.json	\
	test-60-generic.json	\
	test-90-synthetic.json	\
	test-issue-286.json	\
	test-style-match.json	\
	$(NULL)

if FREETYPE_PCF_LONG_FAMILY_NAMES
$(builddir)/out.expected: $(srcdir)/out.expected-long-family-names Makefile
	cp $(srcdir)/out.expected-long-family-names $(builddir)/out.expected
else
$(builddir)/out.expected: $(srcdir)/out.expected-no-long-family-names Makefile
	cp $(srcdir)/out.expected-no-long-family-names $(builddir)/out.expected
endif

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir)

check_PROGRAMS =
if HAVE_PTHREAD
check_PROGRAMS += test-pthread
test_pthread_LDADD = $(top_builddir)/src/libfontconfig.la
# We don't enable this test by default because it will require config and fonts
# to meaningfully test anything, and we are not installed yet.
#TESTS += test-pthread

check_PROGRAMS += test-crbug1004254
test_crbug1004254_LDADD = $(top_builddir)/src/libfontconfig.la
# Disabling this for the same reason as above but trying to run in run-test.sh.
#TESTS += test-crbug1004254
endif
check_PROGRAMS += test-bz89617
test_bz89617_CFLAGS = \
	-DSRCDIR="\"$(abs_srcdir)\""

test_bz89617_LDADD = $(top_builddir)/src/libfontconfig.la
TESTS += test-bz89617

check_PROGRAMS += test-bz131804
test_bz131804_LDADD = $(top_builddir)/src/libfontconfig.la
TESTS += test-bz131804

noinst_PROGRAMS = $(check_PROGRAMS)

if !OS_WIN32
check_PROGRAMS += test-migration
test_migration_LDADD = $(top_builddir)/src/libfontconfig.la
endif

check_PROGRAMS += test-bz96676
test_bz96676_LDADD = $(top_builddir)/src/libfontconfig.la
TESTS += test-bz96676

check_PROGRAMS += test-name-parse
test_name_parse_LDADD = $(top_builddir)/src/libfontconfig.la
TESTS += test-name-parse

if ENABLE_JSONC
check_PROGRAMS += test-conf
test_conf_CFLAGS = $(JSONC_CFLAGS)
test_conf_LDADD = $(top_builddir)/src/libfontconfig.la $(JSONC_LIBS)
endif
TESTS += run-test-conf.sh

check_PROGRAMS += test-bz106618
test_bz106618_LDADD = $(top_builddir)/src/libfontconfig.la

if !OS_WIN32
check_PROGRAMS += test-bz106632
test_bz106632_CFLAGS =					\
	-I$(top_builddir)				\
	-I$(top_builddir)/src				\
	-I$(top_srcdir)					\
	-I$(top_srcdir)/src				\
	-DFONTFILE='"$(abs_top_srcdir)/test/4x6.pcf"'	\
	-DHAVE_CONFIG_H					\
	$(NULL)
test_bz106632_LDADD = $(top_builddir)/src/libfontconfig.la
TESTS += test-bz106632
endif

check_PROGRAMS += test-issue107
test_issue107_LDADD =					\
	$(top_builddir)/src/libfontconfig.la		\
	$(NULL)
TESTS += test-issue107

if !ENABLE_SHARED
if !OS_WIN32
check_PROGRAMS += test-issue110
test_issue110_CFLAGS =					\
	-I$(top_builddir)				\
	-I$(top_builddir)/src				\
	-I$(top_srcdir)					\
	-I$(top_srcdir)/src				\
	-DHAVE_CONFIG_H					\
	-DFONTCONFIG_PATH='"$(BASECONFIGDIR)"'		\
	$(NULL)
test_issue110_LDADD =					\
	$(top_builddir)/src/libfontconfig.la		\
	$(NULL)
TESTS += test-issue110

check_PROGRAMS += test-d1f48f11
test_d1f48f11_CFLAGS =					\
	-I$(top_builddir)				\
	-I$(top_builddir)/src				\
	-I$(top_srcdir)					\
	-I$(top_srcdir)/src				\
	-DHAVE_CONFIG_H					\
	-DFONTCONFIG_PATH='"$(BASECONFIGDIR)"'		\
	$(NULL)
test_d1f48f11_LDADD =					\
	$(top_builddir)/src/libfontconfig.la		\
	$(NULL)
TESTS += test-d1f48f11
endif
endif

check_PROGRAMS += test-bz1744377
test_bz1744377_LDADD = $(top_builddir)/src/libfontconfig.la
TESTS += test-bz1744377

check_PROGRAMS += test-issue180
test_issue180_LDADD = $(top_builddir)/src/libfontconfig.la
TESTS += test-issue180

check_PROGRAMS += test-family-matching
test_family_matching_LDADD = $(top_builddir)/src/libfontconfig.la
TESTS += test-family-matching

EXTRA_DIST=run-test.sh run-test-conf.sh wrapper-script.sh $(TESTDATA) out.expected-long-family-names out.expected-no-long-family-names

CLEANFILES =		\
	fonts.conf	\
	out		\
	out1		\
	out2		\
	out.expected	\
	run*.log	\
	run*.trs	\
	test*.log	\
	test*.trs	\
	$(NULL)

-include $(top_srcdir)/git.mk
