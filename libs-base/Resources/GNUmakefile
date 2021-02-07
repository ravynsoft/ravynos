#
#  Resources makefile for GNUstep Base Library
#  Copyright (C) 2000 Free Software Foundation, Inc.
#
#  Written by:	Adam Fedor <fedor@doc.com>
#  Date: Oct 2000
#
#  This file is part of the GNUstep Base Library.
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 3 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
#  Library General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
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
-include ../config.mak

include ../Version
libgnustep-base_INTERFACE_VERSION=$(MAJOR_VERSION).$(MINOR_VERSION)

RESOURCE_SET_NAME = base-resources

ifeq ($(base-resources_INSTALL_DIR),)
base-resources_INSTALL_DIR = $(GNUSTEP_LIBRARY)/Libraries/gnustep-base/Versions/$(libgnustep-base_INTERFACE_VERSION)/Resources
endif

base-resources_LANGUAGES = English German French Italian TraditionalChinese \
Spanish Esperanto Korean Japanese
base-resources_LOCALIZED_RESOURCE_FILES = Localizable.strings
base-resources_RESOURCE_DIRS = Languages GSTLS
base-resources_RESOURCE_FILES = \
        $(LANGUAGE_INSTALL_FILES) \
        $(GSTLS_INSTALL_FILES)

LANGUAGE_FILES = \
Languages/Dutch \
Languages/English \
Languages/French  \
Languages/German  \
Languages/Hungarian  \
Languages/Italian \
Languages/Polish \
Languages/Russian \
Languages/Spanish \
Languages/TraditionalChinese \
Languages/Slovak \
Languages/Ukrainian \
Languages/Esperanto \
Languages/Korean \
Languages/Japanese
LANGUAGE_INSTALL_FILES = $(LANGUAGE_FILES) Languages/Locale.aliases \
	Languages/Locale.encodings Languages/Locale.canonical

GSTLS_INSTALL_FILES = \
        GSTLS/ca-certificates.crt

include $(GNUSTEP_MAKEFILES)/resource-set.make

-include GNUmakefile.postamble

