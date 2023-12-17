## Makefile for the gettext-runtime/intl-java subdirectory of GNU gettext
## Copyright (C) 2001-2003, 2006-2007, 2013 Free Software Foundation, Inc.
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

jardir = $(datadir)/gettext

JAR = @JAR@
JAVACOMP = $(SHELL) ../javacomp.sh
JAVADOC2 = test ! -d /usr/lib/jdk1.3.1 || env PATH=/usr/lib/jdk1.3.1/bin:$$PATH javadoc


all-local: all-classes all-javadoc2
install-data-local: install-classes install-javadoc2
installdirs-local: installdirs-classes installdirs-javadoc2
uninstall-local: uninstall-classes uninstall-javadoc2


# Special rules for Java compilation.

all-classes: all-classes-@BUILDJAVA@
all-classes-no:
all-classes-yes: libintl.jar

gnu/gettext/GettextResource.class: $(srcdir)/gnu/gettext/GettextResource.java
	$(JAVACOMP) -d . $(srcdir)/gnu/gettext/GettextResource.java

libintl.jar: gnu/gettext/GettextResource.class
	$(JAR) cf $@ gnu/gettext/GettextResource*.class

EXTRA_DIST += gnu/gettext/GettextResource.java

CLEANFILES += libintl.jar gnu/gettext/*.class

install-classes: install-classes-@BUILDJAVA@
install-classes-no:
	$(MKDIR_P) $(DESTDIR)$(jardir)
install-classes-yes: all-classes-yes
	$(MKDIR_P) $(DESTDIR)$(jardir)
	$(INSTALL_DATA) libintl.jar $(DESTDIR)$(jardir)/libintl.jar

installdirs-classes:
	$(MKDIR_P) $(DESTDIR)$(jardir)

uninstall-classes:
	$(RM) $(DESTDIR)$(jardir)/libintl.jar


# Special rules for Javadoc (JDK 1.2) documentation.

all-javadoc2: $(srcdir)/javadoc2/index.html

$(srcdir)/javadoc2/index.html: $(srcdir)/gnu/gettext/GettextResource.java
	cd $(srcdir) && $(JAVADOC2) -d javadoc2 gnu.gettext gnu/gettext/*.java

JAVADOC2_FILES = \
  javadoc2/index.html \
  javadoc2/allclasses-frame.html \
  javadoc2/overview-tree.html \
  javadoc2/deprecated-list.html \
  javadoc2/serialized-form.html \
  javadoc2/index-all.html \
  javadoc2/help-doc.html \
  javadoc2/packages.html \
  javadoc2/package-list \
  javadoc2/stylesheet.css \
  javadoc2/gnu/gettext/package-summary.html \
  javadoc2/gnu/gettext/package-tree.html \
  javadoc2/gnu/gettext/package-frame.html \
  javadoc2/gnu/gettext/GettextResource.html

EXTRA_DIST += $(JAVADOC2_FILES)

install-javadoc2: all-javadoc2
	$(MKDIR_P) $(DESTDIR)$(htmldir)/javadoc2
	$(MKDIR_P) $(DESTDIR)$(htmldir)/javadoc2/gnu/gettext
	@for f in $(JAVADOC2_FILES); do \
	  echo "$(INSTALL_DATA) $(srcdir)/$$f $(DESTDIR)$(htmldir)/$$f"; \
	  $(INSTALL_DATA) $(srcdir)/$$f $(DESTDIR)$(htmldir)/$$f; \
	done

installdirs-javadoc2:
	$(MKDIR_P) $(DESTDIR)$(htmldir)/javadoc2
	$(MKDIR_P) $(DESTDIR)$(htmldir)/javadoc2/gnu/gettext

uninstall-javadoc2:
	@for f in $(JAVADOC2_FILES); do \
	  echo "$(RM) $(DESTDIR)$(htmldir)/$$f"; \
	  $(RM) $(DESTDIR)$(htmldir)/$$f; \
	done
