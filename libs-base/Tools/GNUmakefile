#
#  Tools makefile for GNUstep Base Library
#  Copyright (C) 1997 Free Software Foundation, Inc.
#
#  Written by:	Richard Frith-Macdonald <richard@brainstorm.co.uk>
#
#  This file is part of the GNUstep Base Library.
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public
#  License as published by the Free Software Foundation; either
#  version 2 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
#  General Public License for more details.
#
#  You should have received a copy of the GNU General Public
#  License along with this library; if not, write to the Free
#  Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#  Boston, MA 02111 USA.

ifeq ($(GNUSTEP_MAKEFILES),)
 GNUSTEP_MAKEFILES := $(shell gnustep-config --variable=GNUSTEP_MAKEFILES 2>/dev/null)
  ifeq ($(GNUSTEP_MAKEFILES),)
    $(warning )
    $(warning Unable to obtain GNUSTEP_MAKEFILES setting from gnustep-config!)
    $(warning Perhaps gnustep-make is not properly installed,)
    $(warning so gnustep-config is not in your PATH.)
    $(warning )
    $(warning Your PATH is currently $(PATH))
    $(warning )
  endif
endif

ifeq ($(GNUSTEP_MAKEFILES),)
  $(error You need to set GNUSTEP_MAKEFILES before compiling!)
endif

PACKAGE_NAME = gnustep-base
GNUSTEP_LOCAL_ADDITIONAL_MAKEFILES=../base.make

include $(GNUSTEP_MAKEFILES)/common.make

-include ../Version
-include ../config.mak

ADDITIONAL_CPPFLAGS = -DGNUSTEP_BASE_INTERNAL=1


# DTDs to install
dtddir = $(GNUSTEP_LIBRARY)/DTDs
DTD_FILES = plist-0_9.dtd \
	gsdoc-0_6_5.dtd \
	gsdoc-0_6_6.dtd \
	gsdoc-0_6_7.dtd \
	gsdoc-1_0_0.dtd \
	gsdoc-1_0_1.dtd \
	gsdoc-1_0_2.dtd \
	gsdoc-1_0_3.dtd \
	gsdoc-1_0_4.dtd \
	gsdoc-1_0_1.rnc

# Manual pages to install
MAN1_PAGES = gdnc.1 autogsdoc.1 cvtenc.1 defaults.1 gspath.1 \
  pldes.1 sfparse.1 xmlparse.1 HTMLLinker.1
MAN7_PAGES = gsdoc.7
MAN8_PAGES = gdomap.8

# The application to be compiled
ifeq ($(add),yes)
TOOL_NAME = autogsdoc cvtenc plmerge sfparse xmlparse
else
TOOL_NAME = autogsdoc cvtenc gdnc gspath defaults pl plmerge plutil \
		plparse sfparse pldes plget plser pl2link xmlparse HTMLLinker
ifneq ($(GNUSTEP_TARGET_OS), windows)
ifneq ($(GNUSTEP_TARGET_OS), linux-android)
CTOOL_NAME = gdomap
endif
endif

SUBPROJECTS = make_strings
endif

TEST_TOOL_NAME = locale_alias

# The source files to be compiled
autogsdoc_OBJC_FILES = autogsdoc.m AGSParser.m AGSOutput.m AGSIndex.m AGSHtml.m
cvtenc_OBJC_FILES = cvtenc.m
gdomap_C_FILES = gdomap.c
gdnc_OBJC_FILES = gdnc.m
gspath_OBJC_FILES = gspath.m
defaults_OBJC_FILES = defaults.m
dread_OBJC_FILES = dread.m
dremove_OBJC_FILES = dremove.m
dwrite_OBJC_FILES = dwrite.m
pl_OBJC_FILES = pl.m
pldes_OBJC_FILES = pldes.m
plget_OBJC_FILES = plget.m
plser_OBJC_FILES = plser.m
plmerge_OBJC_FILES = plmerge.m
plparse_OBJC_FILES = plparse.m
plutil_OBJC_FILES = NSPropertyList+PLUtil.m plutil.m
sfparse_OBJC_FILES = sfparse.m
pl2link_OBJC_FILES = pl2link.m
locale_alias_OBJC_FILES = locale_alias.m
xmlparse_OBJC_FILES = xmlparse.m
HTMLLinker_OBJC_FILES = HTMLLinker.m

ifeq ($(OBJC_RUNTIME_LIB), ng)
AGSHtml.m_FILE_FLAGS+= -fobjc-arc
endif

# Reset this variable (defined in config.mak) to avoid useless linkage
# against the libraries gnustep-base uses.
CONFIG_SYSTEM_LIBS :=

DOCUMENT_NAME = autogsdoc

# Documentation rules for autogsdoc
autogsdoc_AGSDOC_FLAGS = -Up autogsdoc
autogsdoc_AGSDOC_FILES = autogsdoc.m \
	AGSParser.h AGSOutput.h AGSIndex.h AGSHtml.h
autogsdoc_DOC_INSTALL_DIR = Developer/Tools

# Use local version of autogsdoc in case it is not installed
AUTOGSDOC=./$(GNUSTEP_OBJ_DIR_NAME)/autogsdoc

include Makefile.preamble

include $(GNUSTEP_MAKEFILES)/tool.make
#include $(GNUSTEP_MAKEFILES)/test-tool.make
include $(GNUSTEP_MAKEFILES)/ctool.make
include $(GNUSTEP_MAKEFILES)/aggregate.make

ifeq ($(GNUSTEP_BASE_HAVE_LIBXML),1)
ifeq ($(doc),yes)
include $(GNUSTEP_MAKEFILES)/documentation.make
endif
endif

include Makefile.postamble

