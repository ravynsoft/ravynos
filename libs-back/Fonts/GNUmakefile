#
#  Resources makefile for GNUstep Back Library
#  Copyright (C) 2005 Free Software Foundation, Inc.
#
#  Written by:	Adam Fedor <fedor@doc.com>
#  Date: Feb 2005
#
#  This file is part of the GNUstep Back Library.
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Library General Public
#  License as published by the Free Software Foundation; either
#  version 2 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
#  Library General Public License for more details.
#
#  You should have received a copy of the GNU Library General Public
#  License along with this library; if not, write to the Free
#  Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111 USA.

PACKAGE_NAME = gnustep-back
GNUSTEP_LOCAL_ADDITIONAL_MAKEFILES=../back.make
include $(GNUSTEP_MAKEFILES)/common.make

#RESOURCE_SET_NAME = back-resources
back-resources_INSTALL_DIR = $(GNUSTEP_LIBRARY)/Fonts
back-resources_RESOURCE_FILES = $(FONT_INSTALL_FILES)

FONT_INSTALL_FILES = Helvetica.nfont

include $(GNUSTEP_MAKEFILES)/rules.make

-include GNUmakefile.postamble

