#
#  Top level makefile for GNUstep Backend
#
#  Copyright (C) 2002 Free Software Foundation, Inc.
#
#  Author: Adam Fedor <fedor@gnu.org>
#
#  This file is part of the GNUstep Backend.
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; see the file COPYING.LIB.
#  If not, see <http://www.gnu.org/licenses/> or write to the 
#  Free Software Foundation, 51 Franklin Street, Fifth Floor, 
#  Boston, MA 02110-1301, USA.

PACKAGE_NAME = gnustep-back
include $(GNUSTEP_MAKEFILES)/common.make

include ../config.make

include ../Version


# The library to be compiled, as a library or as a bundle
ifeq ($(BACKEND_BUNDLE),)
  BACKEND_FULL=$(BACKEND_NAME)
  LIBRARY_NAME=libgnustep-$(BACKEND_FULL)
else
  BACKEND_FULL=$(BACKEND_NAME)-$(INTERFACE_VERSION_NUMBER)
  BUNDLE_NAME=libgnustep-$(BACKEND_FULL)
endif

#
# The list of subproject directories
#
SUBPROJECTS = gsc $(BUILD_SERVER) $(BUILD_GRAPHICS)

libgnustep-$(BACKEND_FULL)_SUBPROJECTS=$(SUBPROJECTS)

libgnustep-$(BACKEND_FULL)_OBJC_FILES=GSBackend.m

libgnustep-$(BACKEND_FULL)_PRINCIPAL_CLASS=GSBackend

libgnustep-$(BACKEND_FULL)_LOCALIZED_RESOURCE_FILES=nfontFaceNames.strings

libgnustep-$(BACKEND_FULL)_LANGUAGES=English Swedish

-include GNUmakefile.preamble

ifeq ($(BACKEND_BUNDLE),)
  include $(GNUSTEP_MAKEFILES)/library.make
else
  include $(GNUSTEP_MAKEFILES)/bundle.make
endif

-include GNUmakefile.postamble
