## Makefile for the gettext-runtime/man subdirectory of GNU gettext
## Copyright (C) 2001-2003, 2006, 2009, 2013-2014, 2018-2019 Free Software Foundation,
## Inc.
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

VERSION = @VERSION@

AUTOMAKE_OPTIONS = 1.11 gnits
EXTRA_DIST =

# A manual page for each of the bin_PROGRAMS in src/Makefile.am.

man_aux = gettext.x ngettext.x envsubst.x

# Likewise, plus additional manual pages for the libintl functions.

man_MAN1GEN = gettext.1 ngettext.1
man_MAN1IN = gettext.1.in ngettext.1.in
man_MAN1OTHER = envsubst.1
man_MAN1 = $(man_MAN1GEN) $(man_MAN1OTHER)
man_MAN3 = gettext.3 ngettext.3 \
textdomain.3 bindtextdomain.3 bind_textdomain_codeset.3
man_MAN3IN = gettext.3.in ngettext.3.in \
textdomain.3.in bindtextdomain.3.in bind_textdomain_codeset.3.in
man_MAN3LINK = dgettext.3 dcgettext.3 dngettext.3 dcngettext.3
man_MANS = $(man_MAN1)
notrans_man_MANS = $(man_MAN3) $(man_MAN3LINK)

man_HTML1GEN = gettext.1.html ngettext.1.html
man_HTML1IN = gettext.1.html.in ngettext.1.html.in
man_HTML1OTHER = envsubst.1.html
man_HTML1 = $(man_HTML1GEN) $(man_HTML1OTHER)
man_HTML3 = gettext.3.html ngettext.3.html \
textdomain.3.html bindtextdomain.3.html bind_textdomain_codeset.3.html
man_HTML = $(man_HTML1) $(man_HTML3)

EXTRA_DIST += help2man $(man_aux) $(man_MAN1IN) $(man_MAN1OTHER) \
$(man_MAN3) $(man_MAN3IN) $(man_MAN3LINK) \
$(man_HTML1IN) $(man_HTML1OTHER) $(man_HTML3)
CLEANFILES = $(man_MAN1GEN) $(man_HTML1GEN)
MAINTAINERCLEANFILES = $(man_MAN1IN) $(man_MAN1OTHER) $(man_MAN3) \
$(man_HTML1IN) $(man_HTML1OTHER) $(man_HTML3)

PERL = @PERL@
RM = rm -f

# help2man 1.24 or newer.
HELP2MAN = $(PERL) -w -- $(srcdir)/help2man

# groff 1.17 or newer.
MAN2HTML = groff -mandoc -Thtml
# Some versions of groff -Thtml map '\-' to '&minus;' = U+2212, which yields
# the wrong result when copy&pasting command-line options and such.
# Also remove 'CreationDate:' lines, for reproducible builds.
MAN2HTML_POSTPROCESS = sed -e 's/\&minus;/-/g' -e '/CreationDate:/d'

UPDATEMODE =


gt_man2html = \
	if test -f $${srcdir}$$m; then \
	  $(MAN2HTML) $${srcdir}$$m > t-$@ && \
	  $(MAN2HTML_POSTPROCESS) < t-$@ > t2-$@ && \
	  mv t2-$@ $@; \
	  rm -f t-$@ t2-$@; \
	fi


# We distribute both the man pages and their HTML equivalent.
# The user can generate the parts, via
#   make man
#   make html

all-local: html-local
install-data-local: install-html
installdirs-local: installdirs-html
uninstall-local: uninstall-html


# Man pages.

# The progname.x files contain some extra information not found in the
# "progname --help" output.  Each progname.1 depends on the source,
# not the binary; we don't need to regenerate the binary when any
# source file changes, only the main one.

$(man_MAN1GEN): Makefile
	srcdir=''; \
	test -f ./$@.in || srcdir=$(srcdir)/; \
	if test -f $${srcdir}$@.in; then \
	  sed -e 's|@''localedir''@|$(localedir)|g' $${srcdir}$@.in > t-$@ && \
	  mv t-$@ $@; \
	fi

gettext.1: gettext.1.in
ngettext.1: ngettext.1.in

$(man_MAN1IN) $(man_MAN1OTHER): help2man $(top_srcdir)/../.version
	progname=`echo $@ | sed -e 's/\.in$$//' -e 's/\.1$$//'`; \
	IN_HELP2MAN=1 $(SHELL) x-to-1 $(UPDATEMODE) "$(PERL)" "$(HELP2MAN)" \
	  ../src/$${progname}$(EXEEXT) $(srcdir)/$${progname}.x $@

gettext.1.in: gettext.x ../src/gettext.c
ngettext.1.in: ngettext.x ../src/ngettext.c
envsubst.1: envsubst.x ../src/envsubst.c

$(man_MAN3): $(top_srcdir)/../.version
	sed -e 's/@''VERSION''@/@VERSION@/g' < $(srcdir)/$@.in > t-$@ && \
	mv t-$@ $@

gettext.3: gettext.3.in
ngettext.3: ngettext.3.in
textdomain.3: textdomain.3.in
bindtextdomain.3: bindtextdomain.3.in
bind_textdomain_codeset.3: bind_textdomain_codeset.3.in


# Man pages in HTML format.

html-local: $(man_HTML)

$(man_HTML1GEN): Makefile
	srcdir=''; \
	test -f ./$@.in || srcdir=$(srcdir)/; \
	if test -f $${srcdir}$@.in; then \
	  sed -e 's|@''localedir''@|$(localedir)|g' $${srcdir}$@.in > t-$@ && \
	  mv t-$@ $@; \
	fi

gettext.1.html: gettext.1.html.in
ngettext.1.html: ngettext.1.html.in

$(man_HTML1IN):
	srcdir=''; \
	progname=`echo $@ | sed -e 's/\.1\.html\.in$$//'`; \
	m=$${progname}.1.in; \
	test -f ./$$m || srcdir=$(srcdir)/; \
	$(gt_man2html)

gettext.1.html.in: gettext.1.in
ngettext.1.html.in: ngettext.1.in

$(man_HTML1OTHER):
	srcdir=''; \
	progname=`echo $@ | sed -e 's/\.1\.html$$//'`; \
	m=$${progname}.1; \
	test -f ./$$m || srcdir=$(srcdir)/; \
	$(gt_man2html)

envsubst.1.html: envsubst.1

$(man_HTML3):
	srcdir=''; \
	progname=`echo $@ | sed -e 's/\.3\.html$$//'`; \
	m=$${progname}.3.in; \
	test -f ./$$m || srcdir=$(srcdir)/; \
	$(gt_man2html)

gettext.3.html: gettext.3.in
ngettext.3.html: ngettext.3.in
textdomain.3.html: textdomain.3.in
bindtextdomain.3.html: bindtextdomain.3.in
bind_textdomain_codeset.3.html: bind_textdomain_codeset.3.in

install-html-local:
	$(MKDIR_P) $(DESTDIR)$(htmldir)
	for file in $(man_HTML); do \
	  if test -f $$file; then dir=.; else dir=$(srcdir); fi; \
	  $(INSTALL_DATA) $$dir/$$file $(DESTDIR)$(htmldir)/$$file; \
	done

installdirs-html:
	$(MKDIR_P) $(DESTDIR)$(htmldir)

uninstall-html:
	for file in $(man_HTML); do \
	  $(RM) $(DESTDIR)$(htmldir)/$$file; \
	done
