## Makefile for the gettext-tools/system-tests subdirectory of GNU gettext
## Copyright (C) 1995-2018 Free Software Foundation, Inc.
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

AUTOMAKE_OPTIONS = 1.11 gnits no-dependencies color-tests
EXTRA_DIST =
MOSTLYCLEANFILES = core *.stackdump

EXTRA_DIST += README run-test

# For debugging memory leaks and memory allocation bugs.
# You should build with --disable-shared when using valgrind.
CHECKER =
#CHECKER = valgrind --tool=memcheck --num-callers=20 --leak-check=yes --leak-resolution=high --show-reachable=yes
#CHECKER = valgrind --tool=massif --format=html --depth=10 --stacks=no

TESTS_ENVIRONMENT = top_srcdir=$(top_srcdir) \
		    abs_srcdir=$(abs_srcdir) \
		    abs_top_srcdir=$(abs_top_srcdir) \
		    GETTEXT="$(CHECKER) tstgettext" \
		    NGETTEXT="$(CHECKER) tstngettext" \
		    XGETTEXT="$(CHECKER) xgettext" \
		    MSGATTRIB="$(CHECKER) msgattrib" \
		    MSGCAT="$(CHECKER) msgcat" \
		    MSGCMP="$(CHECKER) msgcmp" \
		    MSGCOMM="$(CHECKER) msgcomm" \
		    MSGCONV="$(CHECKER) msgconv" \
		    MSGEN="$(CHECKER) msgen" \
		    MSGEXEC="$(CHECKER) msgexec" \
		    MSGFILTER="$(CHECKER) msgfilter" \
		    MSGFMT="$(CHECKER) msgfmt" \
		    MSGGREP="$(CHECKER) msggrep" \
		    MSGINIT="$(CHECKER) msginit" \
		    MSGMERGE="$(CHECKER) msgmerge" \
		    MSGUNFMT="$(CHECKER) msgunfmt" \
		    MSGUNIQ="$(CHECKER) msguniq" \
		    RECODE="$(CHECKER) recode" \
		    CONFIG_SHELL='$(SHELL)'

LOG_COMPILER = $(SHELL) $(srcdir)/run-test

# It is important that the gnulib-generated *.h files are not visible here,
# except for config.h.
AM_CPPFLAGS = -I..

# The USE_SYSTEM_LIBINTL flag distinguishes a compilation in the system-tests/
# directory from a compilation in the tests/ directory.
DEFS = -DUSE_SYSTEM_LIBINTL @DEFS@

if TEST_SYSTEM_LIBINTL

TESTS = intl-1 intl-2 intl-3 intl-4 intl-5 \
	intl-setlocale-1 intl-setlocale-2 \
	intl-thread-1 intl-thread-2 intl-thread-3 \
	format-c-3 format-c-4 format-c-5 plural-1
.PHONY: $(TESTS)

# Auxiliary programs used by the tests.
check_PROGRAMS = testlocale intl-1-prg intl-3-prg intl-4-prg intl-5-prg intl-setlocale-1-prg intl-setlocale-2-prg intl-thread-1-prg intl-thread-2-prg intl-thread-3-prg cake fc3 fc4 fc5
testlocale_SOURCES = testlocale.c
intl_1_prg_SOURCES = intl-1-prg.c
intl_3_prg_SOURCES = intl-3-prg.c
intl_4_prg_SOURCES = intl-4-prg.c
intl_5_prg_SOURCES = intl-5-prg.c
intl_setlocale_1_prg_SOURCES = intl-setlocale-1-prg.c
intl_setlocale_2_prg_SOURCES = intl-setlocale-2-prg.c
intl_thread_1_prg_SOURCES = intl-thread-1-prg.c
intl_thread_1_prg_CFLAGS = $(intl_thread_1_prg_CFLAGS_@HAVE_NEWLOCALE@)
intl_thread_1_prg_CFLAGS_1 = -DUSE_POSIX_THREADS
intl_thread_1_prg_LDADD = $(LDADD) $(intl_thread_1_prg_LDADD_@HAVE_NEWLOCALE@)
intl_thread_1_prg_LDADD_1 = -lpthread
intl_thread_2_prg_SOURCES = intl-thread-2-prg.c
intl_thread_2_prg_CFLAGS = $(intl_thread_2_prg_CFLAGS_@HAVE_NEWLOCALE@)
intl_thread_2_prg_CFLAGS_1 = -DUSE_POSIX_THREADS
intl_thread_2_prg_LDADD = $(LDADD) $(intl_thread_2_prg_LDADD_@HAVE_NEWLOCALE@)
intl_thread_2_prg_LDADD_1 = -lpthread
intl_thread_3_prg_SOURCES = intl-thread-3-prg.c
intl_thread_3_prg_CFLAGS = $(intl_thread_3_prg_CFLAGS_@HAVE_NEWLOCALE@)
intl_thread_3_prg_CFLAGS_1 = -DUSE_POSIX_THREADS
intl_thread_3_prg_LDADD = $(LDADD) $(intl_thread_3_prg_LDADD_@HAVE_NEWLOCALE@)
intl_thread_3_prg_LDADD_1 = -lpthread
cake_SOURCES = plural-1-prg.c
fc3_SOURCES = format-c-3-prg.c
fc4_SOURCES = format-c-4-prg.c
fc5_SOURCES = format-c-5-prg.c

endif

# Clean up after Solaris cc.
clean-local:
	rm -rf SunWS_cache
