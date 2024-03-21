# This is a -*-Makefile-*-, or close enough
#
# Copyright (C) 1997-2023 Free Software Foundation, Inc.
# This file is part of GNU Make.
#
# GNU Make is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 3 of the License, or (at your option) any later
# version.
#
# GNU Make is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <https://www.gnu.org/licenses/>.

AUTOMAKE_OPTIONS = dist-lzip silent-rules std-options subdir-objects nostdinc
ACLOCAL_AMFLAGS = -I m4

MAKE_HOST =	@MAKE_HOST@

SUBDIRS =	lib po doc

bin_PROGRAMS =	make
include_HEADERS = src/gnumake.h

man_MANS =	doc/make.1

make_SRCS =	src/ar.c src/arscan.c src/commands.c src/commands.h \
		src/debug.h src/default.c src/dep.h src/dir.c src/expand.c \
		src/file.c src/filedef.h src/function.c src/getopt.c \
		src/getopt.h src/getopt1.c src/gettext.h src/guile.c \
		src/hash.c src/hash.h src/implicit.c src/job.c src/job.h \
		src/load.c src/loadapi.c src/main.c src/makeint.h src/misc.c \
		src/mkcustom.h src/os.h src/output.c src/output.h src/read.c \
		src/remake.c src/rule.c src/rule.h src/shuffle.h src/shuffle.c \
		src/signame.c src/strcache.c src/variable.c src/variable.h \
		src/version.c src/vpath.c

w32_SRCS =	src/w32/pathstuff.c src/w32/w32os.c src/w32/compat/dirent.c \
		src/w32/compat/posixfcn.c src/w32/include/dirent.h \
		src/w32/include/dlfcn.h src/w32/include/pathstuff.h \
		src/w32/include/sub_proc.h src/w32/include/w32err.h \
		src/w32/subproc/misc.c src/w32/subproc/proc.h \
		src/w32/subproc/sub_proc.c src/w32/subproc/w32err.c

vms_SRCS =	src/vms_exit.c src/vms_export_symbol.c src/vms_progname.c \
		src/vmsdir.h src/vmsfunctions.c src/vmsify.c

amiga_SRCS =	src/amiga.c src/amiga.h

glob_SRCS =	lib/fnmatch.c lib/fnmatch.h lib/glob.c lib/glob.h

alloca_SRCS =	lib/alloca.c

loadavg_SRCS =	lib/getloadavg.c

make_SOURCES =	$(make_SRCS)
EXTRA_make_SOURCES = $(amiga_SRCS) $(vms_SRCS)

if HAVE_GUILE
  _GUILE_CFLAGS = $(GUILE_CFLAGS)
  _GUILE_LIBS = $(GUILE_LIBS)
else
  _GUILE_CFLAGS =
  _GUILE_LIBS =
endif

make_LDADD =	$(LIBOBJS) $(_GUILE_LIBS) lib/libgnu.a $(GETLOADAVG_LIBS) \
		@LIBINTL@

localedir =	$(datadir)/locale

AM_CPPFLAGS =	-Isrc -I$(top_srcdir)/src -Ilib -I$(top_srcdir)/lib \
		-DLIBDIR=\"$(libdir)\" -DLOCALEDIR=\"$(localedir)\"

# If prefix is not a standard location, look in prefix as well
if !KNOWN_PREFIX
  AM_CPPFLAGS += -DINCLUDEDIR=\"$(includedir)\"
endif

AM_CFLAGS =	$(_GUILE_CFLAGS)

if WINDOWSENV
  make_SOURCES += $(w32_SRCS)
  AM_CPPFLAGS  += -I $(top_srcdir)/src/w32/include
else
  make_SOURCES += src/posixos.c
endif

if USE_CUSTOMS
  make_SOURCES += src/remote-cstms.c
else
  make_SOURCES += src/remote-stub.c
endif

# Extra stuff to include in the distribution.

mk_FILES =	Basic.mk mk/msdosdjgpp.mk mk/Amiga.mk mk/VMS.mk mk/Windows32.mk
# We don't need this, since the standard automake output will do.
#mk/Posix.mk.in

m4_FILES =	m4/gnulib-cache.m4

test_FILES =	tests/run_make_tests tests/run_make_tests.bat \
		tests/run_make_tests.pl tests/test_driver.pl \
		tests/config-flags.pm.in tests/config_flags_pm.com \
		tests/config-flags.pm.W32 \
		tests/mkshadow tests/thelp.pl tests/guile.supp tests/README
# test/scripts are added via dist-hook below.

EXTRA_DIST =	ChangeLog INSTALL README build.sh build.cfg.in $(man_MANS) \
		src/mkconfig.h README.customs README.OS2 README.zOS \
		README.Amiga SCOPTIONS src/config.ami \
		README.DOS builddos.bat src/configh.dos \
		README.W32 build_w32.bat src/config.h.W32 \
		README.VMS makefile.com src/config.h-vms src/vmsjobs.c \
		vms_export_symbol_test.com \
		src/gmk-default.scm src/gmk-default.h \
		$(mk_FILES) $(m4_FILES) $(test_FILES)

# --------------- Generate the Guile default module content

src/guile.$(OBJEXT): src/gmk-default.h
src/gmk-default.h: $(top_srcdir)/src/gmk-default.scm
	(echo 'static const char *const GUILE_module_defn = " '\\ \
	  && sed -e 's/;.*//' -e '/^[ \t]*$$/d' -e 's/"/\\"/g' -e 's/$$/ \\/' \
		 $(top_srcdir)/src/gmk-default.scm \
	  && echo '";') > src/gmk-default.h

# --------------- Local DIST Section

# Install the mk and tests subdirectories
#
dist-hook:
	(cd $(top_srcdir); \
	 sub=`find tests/scripts -follow \( -name .git -o -name .deps -o -name work -o -name .gitignore -o -name \*.orig -o -name \*.rej -o -name \*~ -o -name \*.out -o -name Makefile \) -prune -o -type f -print`; \
	 $(AMTAR) chf - $$sub) \
	| (cd $(distdir); $(AMTAR) xfBp -)


# --------------- Local CHECK Section

check-local: check-regression
	@banner=" Regression PASSED: $(PACKAGE_STRING) ($(MAKE_HOST)) built with $(CC) "; \
	dashes=`echo "$$banner" | sed s/./=/g`; \
	echo; \
	echo "$$dashes"; \
	echo "$$banner"; \
	echo "$$dashes"; \
	echo

# > check-regression
#
# Look for the make test suite, and run it if found and we can find perl.
# If we're building outside the tree, we use symlinks to make a local copy of
# the test suite.  Unfortunately the test suite itself isn't localizable yet.
#
MAKETESTFLAGS =

.PHONY: check-regression

GMK_OUTDIR = ..

testlog = test-suite.log
testresult = tests/.test-result
errorpre = makeerror-$(PACKAGE_VERSION)-$(host_triplet)

testfiles = $(testlog) $(testresult) $(errorfile)
MOSTLYCLEANFILES = $(testfiles)

errordetails =	config.status config.log src/config.h src/mkconfig.h \
		$(testlog) tests/work

# Create a 4-letter random sequence
rand_value = c = "abcdefghijklmnopqrstuvwxyz0123456789"
rand_char  = substr(c,int(rand()*36),1)
rand_string = $(AWK) 'BEGIN{srand(); $(rand_value); print $(rand_char) "" $(rand_char) "" $(rand_char) "" $(rand_char);}'

check-regression: tests/config-flags.pm
	@rm -f $(testfiles)
	@if test -f '$(top_srcdir)/tests/run_make_tests'; then \
	  ulimit -n 128; \
	  if $(PERL) -v >/dev/null 2>&1; then \
	    case `cd '$(top_srcdir)'; pwd` in `pwd`) : ;; \
	      *) test -d tests || mkdir tests; \
		 rm -f srctests; \
		 if ln -s '$(top_srcdir)/tests' srctests; then \
		   for f in run_make_tests run_make_tests.pl test_driver.pl scripts thelp.pl; do \
		     rm -f tests/$$f; ln -s ../srctests/$$f tests; \
		   done; fi ;; \
	    esac; \
	    echo "cd tests && $(PERL) $(PERLFLAGS) ./run_make_tests.pl -srcdir $(abs_top_srcdir) -make $(GMK_OUTDIR)/make$(EXEEXT) $(MAKETESTFLAGS)"; \
	    (cd tests && $(PERL) $(PERLFLAGS) ./run_make_tests.pl -srcdir '$(abs_top_srcdir)' -make '$(GMK_OUTDIR)/make$(EXEEXT)' $(MAKETESTFLAGS); echo $$? >.test-result) 2>&1 | tee $(testlog); \
	    er=$$(cat $(testresult)); if test "$$er" -ne 0; then \
	      dirnm="$(errorpre)-$$($(rand_string))"; fnm="$$dirnm.tar.gz"; \
	      rm -rf "$$dirnm"; mkdir "$$dirnm"; \
	      $(AMTAR) chf - $(errordetails) | (cd "$$dirnm"; $(AMTAR) xf -); \
	      $(AMTAR) chf - "$$dirnm" | eval GZIP= gzip $(GZIP_ENV) -c >"$$fnm"; \
	      echo "*** Testing FAILED!  Details: $$fnm"; \
	      echo '*** Please report to <$(PACKAGE_BUGREPORT)>'; echo; \
	      exit $$er; \
	    fi; \
	  else \
	    echo "Can't find a working Perl ($(PERL)); the test suite requires Perl."; \
	  fi; \
	else \
	  echo "Can't find the $(PACKAGE_NAME) test suite ($(top_srcdir)/tests)."; \
	fi


# --------------- Maintainer's Section

# Tell automake that I haven't forgotten about this file and it will be
# created before we build a distribution (see maintMakefile in the Git
# distribution).

README:

@MAINT_MAKEFILE@
