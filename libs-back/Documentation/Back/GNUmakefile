#
#  Gui gsdoc makefile for the GNUstep Back Library
#  Copyright (C) 2000 Free Software Foundation, Inc.
#
#  Written by:	Adam Fedor <fedor@gnu.org>
#
#  This file is part of the GNUstep Back Library.
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

# The documents to be generated
DOCUMENT_NAME = General

General_AGSDOC_FILES = \
	Back.gsdoc \
	DefaultsSummary.gsdoc \
	WindowFocus.gsdoc

General_AGSDOC_FLAGS = \
	-Up Back

General_DOC_INSTALL_DIR = Developer/Back

-include GNUmakefile.preamble

include $(GNUSTEP_MAKEFILES)/documentation.make

-include GNUmakefile.postamble
