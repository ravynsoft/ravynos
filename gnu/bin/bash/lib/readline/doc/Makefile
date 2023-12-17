# Derived by hand from the generated readline-src/doc/Makefile
# This makefile for Readline library documentation is in -*- text -*- mode.
# Emacs likes it that way.

#   Copyright (C) 1996-2002 Free Software Foundation, Inc.
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

topdir = .
srcdir = .
VPATH = .

prefix = /usr/local
infodir = ${prefix}/info

mandir = ${prefix}/man
manpfx = man

man1ext = 1
man1dir = $(mandir)/$(manpfx)$(man1ext)
man3ext = 3
man3dir = $(mandir)/$(manpfx)$(man3ext)

SHELL = /bin/sh
RM = rm -f

INSTALL = /usr/bin/install -c
INSTALL_DATA = ${INSTALL} -m 644

BUILD_DIR = .
TEXINPUTDIR = $(srcdir)

MAKEINFO    = LANGUAGE= makeinfo
TEXI2DVI    = $(srcdir)/texi2dvi
TEXI2HTML   = $(srcdir)/texi2html
QUIETPS     = #set this to -q to shut up dvips
PSDPI       = 300	# I don't have any 600-dpi printers
DVIPS       = dvips -D ${PSDPI} $(QUIETPS) -o $@     # tricky
DVIPDF      = dvipdfm -o $@ -p ${PAPERSIZE}
PSPDF       = gs -sPAPERSIZE=${PAPERSIZE} -sDEVICE=pdfwrite -dNOPAUSE -dBATCH -sOutputFile=$@

RLSRC = $(srcdir)/rlman.texi $(srcdir)/rluser.texi \
	$(srcdir)/rltech.texi $(srcdir)/version.texi \
	$(srcdir)/rluserman.texi
HISTSRC = $(srcdir)/history.texi $(srcdir)/hsuser.texi \
	  $(srcdir)/hstech.texi $(srcdir)/version.texi

# This should be a program that converts troff to an ascii-readable format
NROFF       = groff -Tascii

# This should be a program that converts troff to postscript
GROFF       = groff

DVIOBJ = readline.dvi history.dvi rluserman.dvi
INFOOBJ = readline.info history.info rluserman.info
PSOBJ = readline.ps history.ps rluserman.ps
HTMLOBJ = readline.html history.html rluserman.html
PDFOBJ = readline.pdf history.pdf rluserman.pdf

INTERMEDIATE_OBJ = rlman.dvi

CREATED_DOCS = $(DVIOBJ) $(INFOOBJ) $(PSOBJ) $(HTMLOBJ) $(PDFOBJ)

.SUFFIXES:      .ps .txt .dvi .html .pdf

.ps.pdf:
	$(RM) $@
	-${PSPDF} $<

.dvi.pdf:
	$(RM) $@
	-${DVIPDF} $<

all: info dvi html ps
nodvi: info html
pdf: $(PDFOBJ)

readline.dvi: $(RLSRC)
	TEXINPUTS=.:$(TEXINPUTDIR):$$TEXINPUTS $(TEXI2DVI) $(srcdir)/rlman.texi
	mv rlman.dvi readline.dvi

readline.info: $(RLSRC)
	$(MAKEINFO) --no-split -I $(TEXINPUTDIR) -o $@ $(srcdir)/rlman.texi

rluserman.dvi: $(RLSRC)
	TEXINPUTS=.:$(TEXINPUTDIR):$$TEXINPUTS $(TEXI2DVI) $(srcdir)/rluserman.texi

rluserman.info: $(RLSRC)
	$(MAKEINFO) --no-split -I $(TEXINPUTDIR) -o $@ $(srcdir)/rluserman.texi

history.dvi: ${HISTSRC}
	TEXINPUTS=.:$(TEXINPUTDIR):$$TEXINPUTS $(TEXI2DVI) $(srcdir)/history.texi

history.info: ${HISTSRC}
	$(MAKEINFO) --no-split -I $(TEXINPUTDIR) -o $@ $(srcdir)/history.texi

readline.ps:	readline.dvi
	$(RM) $@
	$(DVIPS) readline.dvi

rluserman.ps:	rluserman.dvi
	$(RM) $@
	$(DVIPS) rluserman.dvi

history.ps:	history.dvi
	$(RM) $@
	$(DVIPS) history.dvi

readline.html:	${RLSRC}
	$(TEXI2HTML) -menu -monolithic -I $(TEXINPUTDIR) $(srcdir)/rlman.texi
	sed -e 's:rlman.html:readline.html:' rlman.html > readline.html
	$(RM) rlman.html

rluserman.html:	${RLSRC}
	$(TEXI2HTML) -menu -monolithic -I $(TEXINPUTDIR) $(srcdir)/rluserman.texi

history.html:	${HISTSRC}
	$(TEXI2HTML) -menu -monolithic -I $(TEXINPUTDIR) $(srcdir)/history.texi

info:	$(INFOOBJ)
dvi:	$(DVIOBJ)
ps:	$(PSOBJ)
html:	$(HTMLOBJ)

readline.pdf:   readline.dvi
history.pdf:    history.dvi
rluserman.pdf:  rluserman.dvi

clean:
	$(RM) *.aux *.cp *.fn *.ky *.log *.pg *.toc *.tp *.vr *.cps *.pgs \
	      *.fns *.kys *.tps *.vrs *.bt *.bts *.o core *.core

distclean: clean
	$(RM) $(CREATED_DOCS)
	$(RM) $(INTERMEDIATE_OBJ)

mostlyclean: clean

maintainer-clean: clean
	$(RM) $(CREATED_DOCS)
	$(RM) $(INTERMEDIATE_OBJ)
	$(RM) Makefile

install:
	@echo "This documentation should not be installed."

uninstall:
