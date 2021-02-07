#
#  Main makefile for GNUstep WIN32 GUI Backend
#
#  Copyright (C) 2002 Free Software Foundation, Inc.
#
#  Author: Fred Kiefer <FredKiefer@gmx.de>
#
#  This file is part of the GNUstep WIN32 GUI Backend.
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
GNUSTEP_LOCAL_ADDITIONAL_MAKEFILES=../../back.make
include $(GNUSTEP_MAKEFILES)/common.make

include ../../config.make

SUBPROJECT_NAME=winlib

# The C source files to be compiled
winlib_C_FILES = 

# The Objective-C source files to be compiled
winlib_OBJC_FILES = \
WIN32Context.m \
WIN32GState.m \
WIN32FontEnumerator.m \
WIN32FontInfo.m

-include GNUmakefile.preamble

include $(GNUSTEP_MAKEFILES)/subproject.make

-include GNUmakefile.postamble
