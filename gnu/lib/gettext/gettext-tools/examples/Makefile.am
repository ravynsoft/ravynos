## Makefile for the gettext-tools/examples subdirectory of GNU gettext
## Copyright (C) 2001-2007, 2009-2016, 2018-2023 Free Software Foundation, Inc.
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

AUTOMAKE_OPTIONS = 1.2 foreign
ACLOCAL_AMFLAGS = -I ../../gettext-runtime/m4 -I ../../m4
SUBDIRS = po
EXTRA_DIST = \
  HACKING \
  check-examples

examplesdir = $(docdir)/examples

examples_DATA = installpaths

# List of examples files, excluding the localizations.
EXAMPLESFILES = \
  \
  hello-c/INSTALL \
  hello-c/autogen.sh \
  hello-c/autoclean.sh \
  hello-c/hello.c \
  hello-c/Makefile.am \
  hello-c/configure.ac \
  hello-c/m4/Makefile.am \
  hello-c/po/Makevars \
  hello-c/po/POTFILES.in \
  \
  hello-c-gnome/INSTALL \
  hello-c-gnome/autogen.sh \
  hello-c-gnome/autoclean.sh \
  hello-c-gnome/hello.c \
  hello-c-gnome/Makefile.am \
  hello-c-gnome/configure.ac \
  hello-c-gnome/m4/Makefile.am \
  hello-c-gnome/m4/gnome.m4 \
  hello-c-gnome/m4/gnome-gnorba-check.m4 \
  hello-c-gnome/m4/gnome-orbit-check.m4 \
  hello-c-gnome/po/Makevars \
  hello-c-gnome/po/POTFILES.in \
  \
  hello-c-gnome3/INSTALL \
  hello-c-gnome3/autogen.sh \
  hello-c-gnome3/autoclean.sh \
  hello-c-gnome3/hello.c \
  hello-c-gnome3/hello.desktop.in.in \
  hello-c-gnome3/hello.gschema.xml \
  hello-c-gnome3/hello.gresource.xml \
  hello-c-gnome3/hello.ui \
  hello-c-gnome3/Makefile.am \
  hello-c-gnome3/configure.ac \
  hello-c-gnome3/m4/Makefile.am \
  hello-c-gnome3/po/Makevars \
  hello-c-gnome3/po/POTFILES.in \
  \
  hello-c++/INSTALL \
  hello-c++/autogen.sh \
  hello-c++/autoclean.sh \
  hello-c++/hello.cc \
  hello-c++/Makefile.am \
  hello-c++/configure.ac \
  hello-c++/m4/Makefile.am \
  hello-c++/po/Makevars \
  hello-c++/po/POTFILES.in \
  \
  hello-c++20/INSTALL \
  hello-c++20/autogen.sh \
  hello-c++20/autoclean.sh \
  hello-c++20/hello.cc \
  hello-c++20/Makefile.am \
  hello-c++20/configure.ac \
  hello-c++20/m4/Makefile.am \
  hello-c++20/po/Makevars \
  hello-c++20/po/POTFILES.in \
  \
  hello-c++-qt/INSTALL \
  hello-c++-qt/BUGS \
  hello-c++-qt/autogen.sh \
  hello-c++-qt/autoclean.sh \
  hello-c++-qt/hello.cc \
  hello-c++-qt/Makefile.am \
  hello-c++-qt/configure.ac \
  hello-c++-qt/m4/Makefile.am \
  hello-c++-qt/m4/qt.m4 \
  hello-c++-qt/po/Makefile.am \
  \
  hello-c++-kde/INSTALL \
  hello-c++-kde/BUGS \
  hello-c++-kde/autogen.sh \
  hello-c++-kde/autoclean.sh \
  hello-c++-kde/hello.cc \
  hello-c++-kde/hellowindow.h \
  hello-c++-kde/hellowindow.cc \
  hello-c++-kde/Makefile.am \
  hello-c++-kde/configure.in.in \
  hello-c++-kde/m4/Makefile.am \
  hello-c++-kde/admin/acinclude.m4.in \
  hello-c++-kde/admin/am_edit \
  hello-c++-kde/admin/ChangeLog \
  hello-c++-kde/admin/compile \
  hello-c++-kde/admin/conf.change.pl \
  hello-c++-kde/admin/config.guess \
  hello-c++-kde/admin/config.pl \
  hello-c++-kde/admin/config.sub \
  hello-c++-kde/admin/configure.in.bot.end \
  hello-c++-kde/admin/configure.in.min \
  hello-c++-kde/admin/cvs-clean.pl \
  hello-c++-kde/admin/cvs.sh \
  hello-c++-kde/admin/debianrules \
  hello-c++-kde/admin/depcomp \
  hello-c++-kde/admin/detect-autoconf.sh \
  hello-c++-kde/admin/Doxyfile.am \
  hello-c++-kde/admin/Doxyfile.global \
  hello-c++-kde/admin/install-sh \
  hello-c++-kde/admin/libtool.m4.in \
  hello-c++-kde/admin/ltmain.sh \
  hello-c++-kde/admin/Makefile.common \
  hello-c++-kde/admin/missing \
  hello-c++-kde/admin/mkinstalldirs \
  hello-c++-kde/admin/ylwrap \
  hello-c++-kde/po/Makevars \
  hello-c++-kde/po/POTFILES.in \
  \
  hello-c++-gnome/INSTALL \
  hello-c++-gnome/autogen.sh \
  hello-c++-gnome/autoclean.sh \
  hello-c++-gnome/hello.cc \
  hello-c++-gnome/Makefile.am \
  hello-c++-gnome/configure.ac \
  hello-c++-gnome/m4/Makefile.am \
  hello-c++-gnome/m4/gnome.m4 \
  hello-c++-gnome/m4/gnome-gnorba-check.m4 \
  hello-c++-gnome/m4/gnome-orbit-check.m4 \
  hello-c++-gnome/m4/gtk.m4 \
  hello-c++-gnome/m4/gtk--.m4 \
  hello-c++-gnome/po/Makevars \
  hello-c++-gnome/po/POTFILES.in \
  \
  hello-c++-wxwidgets/INSTALL \
  hello-c++-wxwidgets/autogen.sh \
  hello-c++-wxwidgets/autoclean.sh \
  hello-c++-wxwidgets/hello.cc \
  hello-c++-wxwidgets/Makefile.am \
  hello-c++-wxwidgets/configure.ac \
  hello-c++-wxwidgets/m4/Makefile.am \
  hello-c++-wxwidgets/m4/wxwidgets.m4 \
  hello-c++-wxwidgets/po/Makefile.am \
  \
  hello-objc/INSTALL \
  hello-objc/autogen.sh \
  hello-objc/autoclean.sh \
  hello-objc/hello.m \
  hello-objc/Makefile.am \
  hello-objc/configure.ac \
  hello-objc/m4/Makefile.am \
  hello-objc/po/Makevars \
  hello-objc/po/POTFILES.in \
  \
  hello-objc-gnustep/INSTALL \
  hello-objc-gnustep/BUGS \
  hello-objc-gnustep/autogen.sh \
  hello-objc-gnustep/autoclean.sh \
  hello-objc-gnustep/main.m \
  hello-objc-gnustep/AppController.h \
  hello-objc-gnustep/AppController.m \
  hello-objc-gnustep/Hello.h \
  hello-objc-gnustep/Hello.m \
  hello-objc-gnustep/GNUmakefile \
  hello-objc-gnustep/po/GNUmakefile \
  hello-objc-gnustep/po/LocaleAliases \
  \
  hello-objc-gnome/INSTALL \
  hello-objc-gnome/autogen.sh \
  hello-objc-gnome/autoclean.sh \
  hello-objc-gnome/hello.m \
  hello-objc-gnome/Makefile.am \
  hello-objc-gnome/configure.ac \
  hello-objc-gnome/m4/Makefile.am \
  hello-objc-gnome/m4/gnome.m4 \
  hello-objc-gnome/m4/gnome-gnorba-check.m4 \
  hello-objc-gnome/m4/gnome-orbit-check.m4 \
  hello-objc-gnome/po/Makevars \
  hello-objc-gnome/po/POTFILES.in \
  \
  hello-sh/INSTALL \
  hello-sh/autogen.sh \
  hello-sh/autoclean.sh \
  hello-sh/hello.sh \
  hello-sh/Makefile.am \
  hello-sh/configure.ac \
  hello-sh/m4/Makefile.am \
  hello-sh/po/Makefile.am \
  \
  hello-python/INSTALL \
  hello-python/autogen.sh \
  hello-python/autoclean.sh \
  hello-python/hello.py.in \
  hello-python/Makefile.am \
  hello-python/configure.ac \
  hello-python/m4/Makefile.am \
  hello-python/po/Makefile.am \
  \
  hello-clisp/INSTALL \
  hello-clisp/autogen.sh \
  hello-clisp/autoclean.sh \
  hello-clisp/hello.lisp.in \
  hello-clisp/Makefile.am \
  hello-clisp/configure.ac \
  hello-clisp/m4/Makefile.am \
  hello-clisp/po/Makefile.am \
  \
  hello-librep/INSTALL \
  hello-librep/autogen.sh \
  hello-librep/autoclean.sh \
  hello-librep/hello.jl.in \
  hello-librep/Makefile.am \
  hello-librep/configure.ac \
  hello-librep/m4/Makefile.am \
  hello-librep/po/Makefile.am \
  \
  hello-guile/INSTALL \
  hello-guile/autogen.sh \
  hello-guile/autoclean.sh \
  hello-guile/hello.scm \
  hello-guile/Makefile.am \
  hello-guile/configure.ac \
  hello-guile/m4/Makefile.am \
  hello-guile/po/Makefile.am \
  \
  hello-smalltalk/INSTALL \
  hello-smalltalk/autogen.sh \
  hello-smalltalk/autoclean.sh \
  hello-smalltalk/hello.st.in \
  hello-smalltalk/Makefile.am \
  hello-smalltalk/configure.ac \
  hello-smalltalk/m4/Makefile.am \
  hello-smalltalk/po/Makefile.am \
  \
  hello-java/INSTALL \
  hello-java/autogen.sh \
  hello-java/autoclean.sh \
  hello-java/Hello.java \
  hello-java/Makefile.am \
  hello-java/configure.ac \
  hello-java/m4/Makefile.am \
  hello-java/po/Makefile.am \
  \
  hello-java-awt/INSTALL \
  hello-java-awt/BUGS \
  hello-java-awt/autogen.sh \
  hello-java-awt/autoclean.sh \
  hello-java-awt/Hello.java \
  hello-java-awt/Makefile.am \
  hello-java-awt/configure.ac \
  hello-java-awt/m4/Makefile.am \
  hello-java-awt/m4/TestAWT.java \
  hello-java-awt/m4/TestAWT.class \
  hello-java-awt/po/Makefile.am \
  \
  hello-java-swing/INSTALL \
  hello-java-swing/BUGS \
  hello-java-swing/autogen.sh \
  hello-java-swing/autoclean.sh \
  hello-java-swing/Hello.java \
  hello-java-swing/Makefile.am \
  hello-java-swing/configure.ac \
  hello-java-swing/m4/Makefile.am \
  hello-java-swing/m4/TestAWT.java \
  hello-java-swing/m4/TestAWT.class \
  hello-java-swing/po/Makefile.am \
  \
  hello-java-qtjambi/INSTALL \
  hello-java-qtjambi/BUGS \
  hello-java-qtjambi/autogen.sh \
  hello-java-qtjambi/autoclean.sh \
  hello-java-qtjambi/Hello.java \
  hello-java-qtjambi/Makefile.am \
  hello-java-qtjambi/configure.ac \
  hello-java-qtjambi/m4/Makefile.am \
  hello-java-qtjambi/m4/Test15.java \
  hello-java-qtjambi/m4/Test15.class \
  hello-java-qtjambi/po/Makefile.am \
  \
  hello-csharp/INSTALL \
  hello-csharp/autogen.sh \
  hello-csharp/autoclean.sh \
  hello-csharp/hello.cs \
  hello-csharp/Makefile.am \
  hello-csharp/configure.ac \
  hello-csharp/m4/Makefile.am \
  hello-csharp/po/Makefile.am \
  \
  hello-csharp-forms/INSTALL \
  hello-csharp-forms/README \
  hello-csharp-forms/autogen.sh \
  hello-csharp-forms/autoclean.sh \
  hello-csharp-forms/hello.cs \
  hello-csharp-forms/Makefile.am \
  hello-csharp-forms/configure.ac \
  hello-csharp-forms/m4/Makefile.am \
  hello-csharp-forms/po/Makefile.am \
  \
  hello-gawk/INSTALL \
  hello-gawk/autogen.sh \
  hello-gawk/autoclean.sh \
  hello-gawk/hello.awk \
  hello-gawk/Makefile.am \
  hello-gawk/configure.ac \
  hello-gawk/m4/Makefile.am \
  hello-gawk/po/Makefile.am \
  \
  hello-pascal/INSTALL \
  hello-pascal/autogen.sh \
  hello-pascal/autoclean.sh \
  hello-pascal/hello.pas \
  hello-pascal/Makefile.am \
  hello-pascal/configure.ac \
  hello-pascal/m4/Makefile.am \
  hello-pascal/po/Makefile.am \
  \
  hello-ycp/INSTALL \
  hello-ycp/autogen.sh \
  hello-ycp/autoclean.sh \
  hello-ycp/hello.ycp \
  hello-ycp/Makefile.am \
  hello-ycp/configure.ac \
  hello-ycp/m4/Makefile.am \
  hello-ycp/po/Makefile.am \
  \
  hello-tcl/INSTALL \
  hello-tcl/autogen.sh \
  hello-tcl/autoclean.sh \
  hello-tcl/hello.tcl \
  hello-tcl/Makefile.am \
  hello-tcl/configure.ac \
  hello-tcl/m4/Makefile.am \
  hello-tcl/po/Makefile.am \
  \
  hello-tcl-tk/INSTALL \
  hello-tcl-tk/autogen.sh \
  hello-tcl-tk/autoclean.sh \
  hello-tcl-tk/hello.tcl \
  hello-tcl-tk/Makefile.am \
  hello-tcl-tk/configure.ac \
  hello-tcl-tk/m4/Makefile.am \
  hello-tcl-tk/po/Makefile.am \
  \
  hello-perl/INSTALL \
  hello-perl/autogen.sh \
  hello-perl/autoclean.sh \
  hello-perl/hello-1.pl.in \
  hello-perl/hello-2.pl.in \
  hello-perl/Makefile.am \
  hello-perl/configure.ac \
  hello-perl/m4/Makefile.am \
  hello-perl/po/Makefile.am \
  \
  hello-php/INSTALL \
  hello-php/README \
  hello-php/autogen.sh \
  hello-php/autoclean.sh \
  hello-php/hello.php \
  hello-php/Makefile.am \
  hello-php/configure.ac \
  hello-php/m4/Makefile.am \
  hello-php/po/Makefile.am \
  \
  README

# List of examples directories containing a po directory.
EXAMPLESDIRS = \
  hello-c \
  hello-c-gnome \
  hello-c-gnome3 \
  hello-c++ \
  hello-c++20 \
  hello-c++-qt \
  hello-c++-kde \
  hello-c++-gnome \
  hello-c++-wxwidgets \
  hello-objc \
  hello-objc-gnustep \
  hello-objc-gnome \
  hello-sh \
  hello-python \
  hello-clisp \
  hello-librep \
  hello-guile \
  hello-smalltalk \
  hello-java \
  hello-java-awt \
  hello-java-swing \
  hello-java-qtjambi \
  hello-csharp \
  hello-csharp-forms \
  hello-gawk \
  hello-pascal \
  hello-ycp \
  hello-tcl \
  hello-tcl-tk \
  hello-perl \
  hello-php

# List of files present in every example's po directory.
EXAMPLESPOFILES = \
  LINGUAS \
  af.po \
  ast.po \
  bg.po \
  ca.po \
  cs.po \
  da.po \
  de.po \
  el.po \
  eo.po \
  es.po \
  fi.po \
  fr.po \
  ga.po \
  gl.po \
  hr.po \
  hu.po \
  id.po \
  it.po \
  ja.po \
  ka.po \
  ky.po \
  lv.po \
  ms.po \
  mt.po \
  nb.po \
  nl.po \
  nn.po \
  pl.po \
  pt.po \
  pt_BR.po \
  ro.po \
  ru.po \
  sk.po \
  sl.po \
  sq.po \
  sr.po \
  sv.po \
  ta.po \
  tr.po \
  uk.po \
  vi.po \
  zh_CN.po \
  zh_HK.po \
  zh_TW.po

install-data-local: $(EXAMPLESFILES)
	$(MKDIR_P) $(DESTDIR)$(examplesdir)
	for f in $(EXAMPLESFILES); do \
	  d=`echo /$$f | sed -e 's,/[^/]*$$,,'`; \
	  test -z "$$d" || $(MKDIR_P) $(DESTDIR)$(examplesdir)$$d || exit 1; \
	  b=`echo $$f | sed -e 's,^.*/,,'`; \
	  if test $$b = autogen.sh || test $$b = autoclean.sh; then \
	    echo "$(INSTALL_SCRIPT) $(srcdir)/$$f $(DESTDIR)$(examplesdir)/$$f"; \
	    $(INSTALL_SCRIPT) $(srcdir)/$$f $(DESTDIR)$(examplesdir)/$$f; \
	  else \
	    echo "$(INSTALL_DATA) $(srcdir)/$$f $(DESTDIR)$(examplesdir)/$$f"; \
	    $(INSTALL_DATA) $(srcdir)/$$f $(DESTDIR)$(examplesdir)/$$f; \
	  fi; \
	done
	for f1 in $(EXAMPLESDIRS); do \
	  for f2 in $(EXAMPLESPOFILES); do \
	    f=$$f1/po/$$f2; \
	    test -f $(srcdir)/$$f || { \
	      echo "WARNING: $(srcdir)/$$f doesn't exist; do \"make dist\" to prepare" >&2; \
	      break; \
	    }; \
	    d=`echo /$$f | sed -e 's,/[^/]*$$,,'`; \
	    test -z "$$d" || $(MKDIR_P) $(DESTDIR)$(examplesdir)$$d || exit 1; \
	    b=`echo $$f | sed -e 's,^.*/,,'`; \
	    echo "$(INSTALL_DATA) $(srcdir)/$$f $(DESTDIR)$(examplesdir)/$$f"; \
	    $(INSTALL_DATA) $(srcdir)/$$f $(DESTDIR)$(examplesdir)/$$f; \
	  done; \
	done

installdirs-local:
	$(MKDIR_P) $(DESTDIR)$(examplesdir)
	for f in $(EXAMPLESFILES); do \
	  d=`echo /$$f | sed -e 's,/[^/]*$$,,'`; \
	  test -z "$$d" || $(MKDIR_P) $(DESTDIR)$(examplesdir)$$d || exit 1; \
	done
	for f1 in $(EXAMPLESDIRS); do \
	  for f2 in $(EXAMPLESPOFILES); do \
	    f=$$f1/po/$$f2; \
	    d=`echo /$$f | sed -e 's,/[^/]*$$,,'`; \
	    test -z "$$d" || $(MKDIR_P) $(DESTDIR)$(examplesdir)$$d || exit 1; \
	  done; \
	done

uninstall-local:
	for f in $(EXAMPLESFILES); do \
	  rm -f $(DESTDIR)$(examplesdir)/$$f; \
	done
	for f1 in $(EXAMPLESDIRS); do \
	  for f2 in $(EXAMPLESPOFILES); do \
	    f=$$f1/po/$$f2; \
	    rm -f $(DESTDIR)$(examplesdir)/$$f; \
	  done; \
	done

# Invoked from inside automake's distdir target.
distdir1:
	cd po && $(MAKE) update-po all
	for file in $(EXAMPLESFILES); do \
	  if test -f $$file; then d=.; else d=$(srcdir); fi; \
	  dir=`echo "$$file" | sed -e 's,/[^/]*$$,,'`; \
	  if test "$$dir" != "$$file" && test "$$dir" != "."; then \
	    $(MKDIR_P) "$(distdir)/$$dir"; \
	  fi; \
	  cp -p $$d/$$file $(distdir)/$$file || exit 1; \
	done
	for f1 in $(EXAMPLESDIRS); do \
	  for f2 in $(EXAMPLESPOFILES); do \
	    file=$$f1/po/$$f2; \
	    if test -f $$file; then d=.; else d=$(srcdir); fi; \
	    dir=`echo "$$file" | sed -e 's,/[^/]*$$,,'`; \
	    if test "$$dir" != "$$file" && test "$$dir" != "."; then \
	      $(MKDIR_P) "$(distdir)/$$dir"; \
	    fi; \
	    cp -p $$d/$$file $(distdir)/$$file || exit 1; \
	  done; \
	done

# Temporary directories created by po/Makefile.
clean-local:
	-rm -rf tmp-hello-*

maintainer-update-po: $(top_builddir)/config.status
	$(SHELL) ./config.status --recheck
	$(SHELL) ./config.status po/Makefile
	cd po && $(MAKE) update-po
