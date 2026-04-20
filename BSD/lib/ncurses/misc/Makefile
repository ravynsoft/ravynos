# $Id: Makefile.in,v 1.88 2025/12/25 22:26:15 tom Exp $
##############################################################################
# Copyright 2018-2024,2025 Thomas E. Dickey                                  #
# Copyright 1998-2016,2017 Free Software Foundation, Inc.                    #
#                                                                            #
# Permission is hereby granted, free of charge, to any person obtaining a    #
# copy of this software and associated documentation files (the "Software"), #
# to deal in the Software without restriction, including without limitation  #
# the rights to use, copy, modify, merge, publish, distribute, distribute    #
# with modifications, sublicense, and/or sell copies of the Software, and to #
# permit persons to whom the Software is furnished to do so, subject to the  #
# following conditions:                                                      #
#                                                                            #
# The above copyright notice and this permission notice shall be included in #
# all copies or substantial portions of the Software.                        #
#                                                                            #
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR #
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   #
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    #
# THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      #
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    #
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        #
# DEALINGS IN THE SOFTWARE.                                                  #
#                                                                            #
# Except as contained in this notice, the name(s) of the above copyright     #
# holders shall not be used in advertising or otherwise to promote the sale, #
# use or other dealings in this Software without prior written               #
# authorization.                                                             #
##############################################################################
#
# Author: Thomas E. Dickey 1996-on
#
# Makefile for ncurses miscellany directory
#
# This makes/installs the terminfo database
#
# The variable 'srcdir' refers to the source-distribution, and can be set with
# the configure script by "--srcdir=DIR".
#
# The rules are organized to produce the libraries for the configured models,
# and the programs with the configured default model.

# turn off _all_ suffix rules; we'll generate our own
.SUFFIXES :

SHELL		= /bin/sh

THIS		= Makefile




top_srcdir	= ..
srcdir		= .
prefix		= /usr
exec_prefix	= ${prefix}
bindir		= ${exec_prefix}/bin
libdir		= ${exec_prefix}/lib
datarootdir	= ${prefix}/share
datadir		= ${datarootdir}
includesubdir	= 

INCLUDEDIR	= $(DESTDIR)$(includedir)$(includesubdir)

tabsetdir	= $(datadir)/tabset
ticdir		= ${datarootdir}/terminfo
ticfile		= $(ticdir).db
source		= ${top_srcdir}/misc/terminfo.src

INSTALL		= /usr/bin/install -c
INSTALL_PROGRAM	= ${INSTALL}
INSTALL_SCRIPT	= ${INSTALL}
INSTALL_DATA	= ${INSTALL} -m 644

PKG_CONFIG_LIBDIR = /usr/lib/pkgconfig

BINDIR		= $(DESTDIR)$(bindir)
DATADIR		= $(DESTDIR)$(datadir)

#INSTALL_PREFIX	= $(DESTDIR)$(prefix)
#LIBDIR		= $(DESTDIR)$(libdir)
#TICDIR		= $(DESTDIR)$(ticdir)
#TICFILE		= $(DESTDIR)$(ticfile)
#TABSETDIR	= $(DESTDIR)$(tabsetdir)

#INSTALL_CONFIG	= $(DESTDIR)$(PKG_CONFIG_LIBDIR)

################################################################################

.PHONY :	all
.PHONY :	check
.PHONY :	clean
.PHONY :	depend
.PHONY :	distclean
.PHONY :	install
.PHONY :	install.data
.PHONY :	install.libs
.PHONY :	libs
.PHONY :	mostlyclean
.PHONY :	realclean
.PHONY :	sources
.PHONY :	uninstall
.PHONY :	uninstall.data
.PHONY :	uninstall.libs

all \
depend \
sources ::

check ::
	@echo "no unit-test implemented"

#all \
#sources :: terminfo.tmp

#install :: install.data

#install.data ::	terminfo.tmp \
#		$(LIBDIR) \
#		$(DATADIR) \
#		$(TABSETDIR)
#	INSTALL_PREFIX=${INSTALL_PREFIX} \
#	TICDIR=${TICDIR} \
#	BINDIR=${BINDIR} \
#	tic_source=terminfo.tmp \
#	$(SHELL) ./run_tic.sh
#	@( cd $(srcdir)/tabset && \
#		$(SHELL) -c 'for i in * ; do \
#			if test -f $$i ; then \
#			echo installing $$i; \
#			$(INSTALL_DATA) $$i $(TABSETDIR)/$$i; \
#			fi; done' )
install.data ::
	@echo "finished $@"

NCURSES_CONFIG	= ncursesw6-config

install \
install.libs :: $(BINDIR) ncurses-config
	$(INSTALL_SCRIPT) ncurses-config $(BINDIR)/$(NCURSES_CONFIG)

# Make a list of the files that gen-pkgconfig might create:
#PC_FILES = \
#	ncursesw.pc \
#	ncursesw.pc \
#	ticw.pc \
#	panelw.pc \
#	menuw.pc \
#	formw.pc \
#	ncurses++w.pc

# some packagers prefer to be able to construct pc-files on servers where
# pkg-config is not installed.  Work around that by creating the library
# directory during this rule:
#install \
#install.libs :: pc-files
#	@$(SHELL) -c 'case "x$(PKG_CONFIG_LIBDIR)" in \
#		x/*|x/*) \
#			mkdir -p $(INSTALL_CONFIG); \
#			for name in $(PC_FILES); do \
#				test -f $$name || continue; \
#				echo installing $$name; \
#				$(INSTALL_DATA) $$name $(INSTALL_CONFIG)/$$name; \
#			done \
#			;; \
#		*) \
#			echo "...skip actual install: no destination was given" ; \
#			;; \
#		esac'

#all \
#sources :: pc-files
#pc-files :
#	$(SHELL) ./gen-pkgconfig
#	-touch $@

terminfo.tmp : run_tic.sed $(source)
	echo '** adjusting tabset paths'
	sed -f run_tic.sed $(source) >terminfo.tmp

run_tic.sed :
	WHICH_XTERM=xterm-new \
	XTERM_KBS=DEL \
	datadir=${datadir} \
	$(SHELL) $(srcdir)/gen_edit.sh >$@

$(BINDIR) \
$(DATADIR) :
	mkdir -p $@

#$(LIBDIR) \
#$(TABSETDIR) :
#	mkdir -p $@

uninstall : uninstall.data uninstall.libs

uninstall.data :
#	-$(SHELL) -c 'if test -d $(TABSETDIR) ; then rm -rf $(TABSETDIR); fi'
#	-$(SHELL) -c 'if test -d $(TICDIR) ; then rm -rf $(TICDIR); fi'
#	-$(SHELL) -c 'if test -f $(TICFILE) ; then rm -f $(TICFILE); fi'

uninstall.libs :
	-rm -f $(BINDIR)/$(NCURSES_CONFIG)
#	@$(SHELL) -c 'case x$(INSTALL_CONFIG) in \
#	x/*|x/*) \
#		for name in $(PC_FILES); do \
#			test -f $$name || continue; \
#			echo uninstalling $$name; \
#			rm -f $(INSTALL_CONFIG)/$$name; \
#		done \
#		;; \
#	esac'

tags :

TAGS :

mostlyclean :
#	-rm -f terminfo.tmp
#	-rm -f run_tic.sed
#	-rm -f pc-files $(PC_FILES)
	-rm -f core tags TAGS *~ *.bak *.ln *.atac trace

clean :: mostlyclean

distclean : clean
#	-rm -f gen-pkgconfig
	-rm -f Makefile run_tic.sh ncurses-config

realclean : distclean

###############################################################################
# The remainder of this file is automatically generated during configuration
###############################################################################
