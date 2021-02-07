#
#  Main makefile for GNUstep Backend xlib
#
#  Copyright (C) 1997 Free Software Foundation, Inc.
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
GNUSTEP_LOCAL_ADDITIONAL_MAKEFILES=../../back.make
include $(GNUSTEP_MAKEFILES)/common.make

include ../../config.make

SUBPROJECT_NAME=xlib

# The Objective-C source files to be compiled
xlib_OBJC_FILES = \
XGBitmap.m \
XGCommonFont.m \
XGFont.m \
XGFontManager.m \
XGContext.m \
XGGState.m \
XGGeometry.m \
XGFontSetFontInfo.m \
linking.m

ifeq ($(WITH_XFT),yes)
xlib_OBJC_FILES += GSXftFontInfo.m \
		   ../fontconfig/FCFontInfo.m \
		   ../fontconfig/FCFontEnumerator.m \
		   ../fontconfig/FCFaceInfo.m
endif

-include GNUmakefile.preamble

include $(GNUSTEP_MAKEFILES)/subproject.make

-include GNUmakefile.postamble

