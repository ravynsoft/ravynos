#   -*-makefile-gmake*-
#   Instance/Documentation/texi.make
#
#   Instance Makefile rules to build Texinfo documentation.
#
#   Copyright (C) 1998, 2000, 2001, 2002 Free Software Foundation, Inc.
#
#   Author:  Scott Christley <scottc@net-community.com>
#   Author: Nicola Pero <n.pero@mi.flashnet.it> 
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

# To override GNUSTEP_MAKEINFO, define it differently in
# GNUmakefile.preamble
ifeq ($(GNUSTEP_MAKEINFO),)
  GNUSTEP_MAKEINFO = makeinfo
endif

# To override GNUSTEP_MAKEINFO_FLAGS, define it differently in
# GNUmakefile.premable.  To only add new flags to the existing ones,
# set ADDITIONAL_MAKEINFO_FLAGS in GNUmakefile.preamble.
ifeq ($(GNUSTEP_MAKEINFO_FLAGS),)
  GNUSTEP_MAKEINFO_FLAGS = -D NO-TEXI2HTML
endif

ifeq ($(GNUSTEP_MAKETEXT),)
  GNUSTEP_MAKETEXT = makeinfo
endif
ifeq ($(GNUSTEP_MAKETEXT_FLAGS),)
  GNUSTEP_MAKETEXT_FLAGS = -D NO-TEXI2HTML -D TEXT-ONLY --no-header --no-split
endif

ifeq ($(GNUSTEP_TEXI2DVI),)
  GNUSTEP_TEXI2DVI = texi2dvi
endif
ifeq ($(GNUSTEP_TEXI2DVI_FLAGS),)
  GNUSTEP_TEXI2DVI_FLAGS =
endif

ifeq ($(GNUSTEP_TEXI2PDF),)
  GNUSTEP_TEXI2PDF = texi2pdf
endif
ifeq ($(GNUSTEP_TEXI2PDF_FLAGS),)
  GNUSTEP_TEXI2PDF_FLAGS =
endif

ifeq ($(GNUSTEP_TEXI2HTML),)
  GNUSTEP_TEXI2HTML = makeinfo --html
endif
ifeq ($(GNUSTEP_TEXI2HTML_FLAGS),)
  GNUSTEP_TEXI2HTML_FLAGS =
endif

# Extract @setfilename value from the .texi file, if available, and
# set OUTFILE accordingly.  Ignore textdoc targets.
# The @setfilename command may appear more than once in a Texinfo
# file; we only need the first occurrence.  It may have ".info" suffix
# or may be followed by a Texinfo comment.
ifeq ($(TEXT_MAIN),)
  SETFILENAME := $(shell grep ^@setfilename $(GNUSTEP_INSTANCE).texi)
    ifneq ($(SETFILENAME),)
      OUTFILE := $(subst .info,,$(word 2, $(SETFILENAME)))
    else
      OUTFILE := $(GNUSTEP_INSTANCE)
  endif
endif

internal-doc-all_:: $(OUTFILE).info \
                    $(OUTFILE).pdf \
                    $(OUTFILE)/index.html

internal-textdoc-all_:: $(GNUSTEP_INSTANCE)

# If we don't have these programs, just don't build them but don't
# abort the make. This allows projects to automatically build documentation
# without worring that the build will crash if the user doesn't have the
# doc programs. Also don't install them if they haven't been generated.

$(OUTFILE).info: $(TEXI_FILES)
	-$(GNUSTEP_MAKEINFO) $(GNUSTEP_MAKEINFO_FLAGS) $(ADDITIONAL_MAKEINFO_FLAGS) \
		-o $@ $(GNUSTEP_INSTANCE).texi

$(GNUSTEP_INSTANCE).dvi: $(TEXI_FILES)
	-$(GNUSTEP_TEXI2DVI) $(GNUSTEP_TEXI2DVI_FLAGS) $(ADDITIONAL_TEXI2DVI_FLAGS) \
	        $(GNUSTEP_INSTANCE).texi

$(GNUSTEP_INSTANCE).ps: $(GNUSTEP_INSTANCE).dvi
	-$(GNUSTEP_DVIPS) $(GNUSTEP_DVIPS_FLAGS) $(ADDITIONAL_DVIPS_FLAGS) \
		$(GNUSTEP_INSTANCE).dvi -o $@

$(OUTFILE).pdf: $(TEXI_FILES)
	-$(GNUSTEP_TEXI2PDF) $(GNUSTEP_TEXI2PDF_FLAGS) $(ADDITIONAL_TEXI2PDF_FLAGS) \
		$(GNUSTEP_INSTANCE).texi -o $@

# Some versions of texi2html placed the html files in a subdirectory,
# so after running it we try to move any from the subdirectory to
# where they are expected.
$(OUTFILE)/index.html: $(TEXI_FILES)
	-$(GNUSTEP_TEXI2HTML) \
                $(GNUSTEP_TEXI2HTML_FLAGS) $(ADDITIONAL_TEXI2HTML_FLAGS) \
		$(GNUSTEP_INSTANCE).texi; \
                if [ -f $(OUTFILE)/$(OUTFILE)_toc.html ]; \
                then \
                  mv $(OUTFILE)/$(OUTFILE).html .; \
                  mv $(OUTFILE)/$(OUTFILE)_*.html .; \
                  rmdir $(OUTFILE)/$(OUTFILE); \
                fi

$(GNUSTEP_INSTANCE): $(TEXI_FILES) $(TEXT_MAIN)
	-$(GNUSTEP_MAKETEXT) $(GNUSTEP_MAKETEXT_FLAGS) $(ADDITIONAL_MAKETEXT_FLAGS) \
		-o $@ $(TEXT_MAIN)

internal-doc-clean::
	-$(ECHO_NOTHING) rm -f $(GNUSTEP_INSTANCE).aux  \
	         $(GNUSTEP_INSTANCE).cp   \
	         $(GNUSTEP_INSTANCE).cps  \
	         $(GNUSTEP_INSTANCE).dvi  \
	         $(GNUSTEP_INSTANCE).fn*  \
	         $(OUTFILE).info* \
	         $(GNUSTEP_INSTANCE).ky   \
	         $(GNUSTEP_INSTANCE).log  \
	         $(GNUSTEP_INSTANCE).pg   \
	         $(GNUSTEP_INSTANCE).ps   \
	         $(OUTFILE).pdf  \
	         $(GNUSTEP_INSTANCE).toc  \
	         $(GNUSTEP_INSTANCE).tp   \
	         $(GNUSTEP_INSTANCE).vr   \
	         $(GNUSTEP_INSTANCE).vrs  \
		 $(GNUSTEP_INSTANCE).html \
		 $(GNUSTEP_INSTANCE)_*.html \
	         $(GNUSTEP_INSTANCE).ps.gz  \
	         $(GNUSTEP_INSTANCE).tar.gz \
	         $(OUTFILE)/*$(END_ECHO)
	-$(ECHO_NOTHING) rmdir $(OUTFILE) $(END_ECHO)

# NB: Only install doc files if they have been generated

# We install all info files in the same directory, which is
# GNUSTEP_DOC_INFO.  TODO: I think we should run
# install-info too - to keep up-to-date the dir index in that
# directory.  
internal-doc-install_:: $(GNUSTEP_DOC_INFO)
	if [ -f $(OUTFILE).pdf ]; then \
	  $(INSTALL_DATA) $(OUTFILE).pdf \
	                $(GNUSTEP_DOC)/$(DOC_INSTALL_DIR); \
	fi
	if [ -f $(OUTFILE).info ]; then \
	  $(INSTALL_DATA) $(OUTFILE).info* $(GNUSTEP_DOC_INFO); \
	fi
	if [ -f i$(OUTFILE)_toc.html ]; then \
	  $(INSTALL_DATA) $(OUTFILE)_*.html \
	                  $(GNUSTEP_DOC)/$(DOC_INSTALL_DIR) || true ; \
	fi
	if [ -f $(OUTFILE)/index.html ]; then \
	  $(MKINSTALLDIRS) $(GNUSTEP_DOC)/$(DOC_INSTALL_DIR)/$(OUTFILE); \
	  $(INSTALL_DATA) $(OUTFILE)/*.html \
	                  $(GNUSTEP_DOC)/$(DOC_INSTALL_DIR)/$(OUTFILE); \
	fi

$(GNUSTEP_DOC_INFO):
	$(ECHO_CREATING)$(MKINSTALLDIRS) $@$(END_ECHO)

internal-doc-uninstall_::
	rm -f \
          $(GNUSTEP_DOC)/$(DOC_INSTALL_DIR)/$(OUTFILE).pdf
	rm -f \
          $(GNUSTEP_DOC_INFO)/$(OUTFILE).info*
	rm -f \
          $(GNUSTEP_DOC)/$(DOC_INSTALL_DIR)/$(OUTFILE)_*.html
	rm -f \
          $(GNUSTEP_DOC)/$(DOC_INSTALL_DIR)/$(OUTFILE).html
	rm -f \
	  $(GNUSTEP_DOC)/$(DOC_INSTALL_DIR)/$(OUTFILE)/*.html

#
# textdoc targets - these should be merged with the doc targets
#
# Make sure we don't install only files that have been generated!
internal-textdoc-install_:: $(GNUSTEP_DOC)/$(DOC_INSTALL_DIR)
	$(ECHO_NOTHING)if [ -f $(GNUSTEP_INSTANCE) ]; then \
	    $(INSTALL_DATA) $(GNUSTEP_INSTANCE) $(GNUSTEP_DOC)/$(DOC_INSTALL_DIR); \
	else \
	  $(ALWAYS_INSIDE_ECHO_MISSING_DOCUMENTATION) \
	fi$(END_ECHO)

internal-textdoc-uninstall_::
	$(ECHO_UNINSTALLING)rm -f \
          $(GNUSTEP_DOC)/$(DOC_INSTALL_DIR)/$(GNUSTEP_INSTANCE)$(END_ECHO)

internal-textdoc-clean::
	$(ECHO_NOTHING) rm -f $(GNUSTEP_INSTANCE) $(END_ECHO)

internal-textdoc-distclean::
