## Makefile for the gettext-runtime/intl-csharp subdirectory of GNU gettext
## Copyright (C) 2003-2004, 2006, 2013, 2015, 2018 Free Software Foundation, Inc.
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

AUTOMAKE_OPTIONS = 1.2 gnits
EXTRA_DIST =
CLEANFILES =

RM = rm -f

CSHARPCOMP = $(SHELL) ../csharpcomp.sh
CSHARPCOMPFLAGS = @CSHARPCOMPFLAGS@


all-local: all-dll all-doc
install-data-local: install-dll install-doc
installdirs-local: installdirs-dll installdirs-doc
uninstall-local: uninstall-dll uninstall-doc


# Special rules for C# compilation.

all-dll: all-dll-@BUILDCSHARP@
all-dll-no:
all-dll-yes: GNU.Gettext.dll

GNU.Gettext.dll: intl.cs
	$(CSHARPCOMP) $(CSHARPCOMPFLAGS) -o $@ $(srcdir)/intl.cs

EXTRA_DIST += intl.cs

CLEANFILES += GNU.Gettext.dll GNU.Gettext.dll.mdb

install-dll: install-dll-@BUILDCSHARP@
install-dll-no:
	$(MKDIR_P) $(DESTDIR)$(libdir)
install-dll-yes: all-dll-yes
	$(MKDIR_P) $(DESTDIR)$(libdir)
	$(INSTALL_DATA) GNU.Gettext.dll $(DESTDIR)$(libdir)/GNU.Gettext.dll

installdirs-dll:
	$(MKDIR_P) $(DESTDIR)$(libdir)

uninstall-dll:
	$(RM) $(DESTDIR)$(libdir)/GNU.Gettext.dll


# C# reference documentation. Requires the pnet tools.

doc:
	test -d doc || mkdir doc
	csdoc -flibrary-name=GNU.Gettext intl.cs | \
	csdoc2html -o doc -fmulti-file -fframes -fcombine-members -fno-namespace-directories -

intl-csharp.texi: intl.cs
	csdoc -flibrary-name=GNU.Gettext intl.cs | \
	csdoc2texi -fembedded -fparent='C#' -o $@ -

all-doc: $(srcdir)/doc/index.html

DOC_FILES = \
  doc/index.html \
  doc/namespaces.html \
  doc/begin.html \
  doc/GNU_Gettext.html \
  doc/GNU_Gettext_GettextResourceManager.html \
  doc/GNU_Gettext_GettextResourceSet.html

EXTRA_DIST += $(DOC_FILES)

install-doc: all-doc
	$(MKDIR_P) $(DESTDIR)$(htmldir)/csharpdoc
	@for f in $(DOC_FILES); do \
	  echo "$(INSTALL_DATA) $(srcdir)/$$f $(DESTDIR)$(htmldir)/csharpdoc/`basename $$f`"; \
	  $(INSTALL_DATA) $(srcdir)/$$f $(DESTDIR)$(htmldir)/csharpdoc/`basename $$f`; \
	done

installdirs-doc:
	$(MKDIR_P) $(DESTDIR)$(htmldir)/csharpdoc

uninstall-doc:
	@for f in $(DOC_FILES); do \
	  echo "$(RM) $(DESTDIR)$(htmldir)/csharpdoc/`basename $$f`"; \
	  $(RM) $(DESTDIR)$(htmldir)/csharpdoc/`basename $$f`; \
	done
