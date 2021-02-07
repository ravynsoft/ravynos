#   -*-makefile-*-
#   Instance/Documentation/latex.make
#
#   Instance Makefile rules to build LaTeX documentation.
#
#   Copyright (C) 1998, 2000, 2001, 2002 Free Software Foundation, Inc.
#
#   Author:  Scott Christley <scottc@net-community.com>
#   Author:  Nicola Pero <n.pero@mi.flashnet.it> 
#
#   This file is part of the GNUstep Makefile Package.
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU General Public License
#   as published by the Free Software Foundation; either version 3
#   of the License, or (at your option) any later version.
#   
#   You should have received a copy of the GNU General Public
#   License along with this library; see the file COPYING.
#   If not, write to the Free Software Foundation,
#   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

$(GNUSTEP_INSTANCE).dvi: $(LATEX_FILES)
	latex $(GNUSTEP_INSTANCE).tex
	latex $(GNUSTEP_INSTANCE).tex

$(GNUSTEP_INSTANCE).ps: $(GNUSTEP_INSTANCE).dvi
	$(GNUSTEP_DVIPS) $(GNUSTEP_DVIPS_FLAGS) $(ADDITIONAL_DVIPS_FLAGS) \
		$(GNUSTEP_INSTANCE).dvi -o $@

$(GNUSTEP_INSTANCE).ps.gz: $(GNUSTEP_INSTANCE).ps 
	gzip $(GNUSTEP_INSTANCE).ps -c > $(GNUSTEP_INSTANCE).ps.gz

internal-doc-all_:: $(GNUSTEP_INSTANCE).ps.gz

internal-doc-install_:: 
	$(INSTALL_DATA) $(GNUSTEP_INSTANCE).ps \
	                $(GNUSTEP_DOC)/$(DOC_INSTALL_DIR)
internal-doc-uninstall_:: 
	$(ECHO_UNINSTALLING)rm -f \
	  $(GNUSTEP_DOC)/$(DOC_INSTALL_DIR)/$(GNUSTEP_INSTANCE).ps$(END_ECHO)

internal-doc-clean::
	-$(ECHO_NOTHING)rm -f $(GNUSTEP_INSTANCE).aux  \
	         $(GNUSTEP_INSTANCE).cp   \
	         $(GNUSTEP_INSTANCE).cps  \
	         $(GNUSTEP_INSTANCE).dvi  \
	         $(GNUSTEP_INSTANCE).fn   \
	         $(GNUSTEP_INSTANCE).info* \
	         $(GNUSTEP_INSTANCE).ky   \
	         $(GNUSTEP_INSTANCE).log  \
	         $(GNUSTEP_INSTANCE).pg   \
	         $(GNUSTEP_INSTANCE).ps   \
	         $(GNUSTEP_INSTANCE).toc  \
	         $(GNUSTEP_INSTANCE).tp   \
	         $(GNUSTEP_INSTANCE).vr   \
	         $(GNUSTEP_INSTANCE).vrs  \
	         $(GNUSTEP_INSTANCE)_*.html \
	         $(GNUSTEP_INSTANCE).ps.gz  \
	         $(GNUSTEP_INSTANCE).tar.gz \
	         $(GNUSTEP_INSTANCE)/* \
	         *.aux$(END_ECHO)

#
# Targets built only if we can find `latex2html'
#

ifneq ($(LATEX2HTML),)
  HAS_LATEX2HTML = yes
endif

ifeq ($(HAS_LATEX2HTML),yes)
internal-doc-all_:: $(GNUSTEP_INSTANCE).tar.gz 

$(GNUSTEP_INSTANCE)/$(GNUSTEP_INSTANCE).html: $(GNUSTEP_INSTANCE).dvi 
	$(LATEX2HTML) $(GNUSTEP_INSTANCE)

$(GNUSTEP_INSTANCE).tar.gz: $(GNUSTEP_INSTANCE)/$(GNUSTEP_INSTANCE).html
	$(TAR) cfzX $(GNUSTEP_INSTANCE).tar.gz $(GNUSTEP_MAKEFILES)/tar-exclude-list $(GNUSTEP_INSTANCE)

internal-doc-install_:: 
	$(INSTALL_DATA) $(GNUSTEP_INSTANCE)/*.html \
	                $(GNUSTEP_DOC)/$(DOC_INSTALL_DIR)
	$(INSTALL_DATA) $(GNUSTEP_INSTANCE)/*.css \
	                $(GNUSTEP_DOC)/$(DOC_INSTALL_DIR)
# Yeah - I know - the following is dangerous if you have misused the 
# DOC_INSTALL_DIR - but it's the only way to do it
internal-doc-uninstall_:: 
	-$(ECHO_UNINSTALLING)rm -f $(GNUSTEP_DOC)/$(DOC_INSTALL_DIR)/*.html; \
	rm -f $(GNUSTEP_DOC)/$(DOC_INSTALL_DIR)/*.css$(END_ECHO)

internal-doc-distclean::
	$(ECHO_NOTHING) if [ -d "$(GNUSTEP_INSTANCE)" ]; then \
	    rm -rf $(GNUSTEP_INSTANCE)/; \
	  fi$(END_ECHO)
endif # LATEX2HTML
