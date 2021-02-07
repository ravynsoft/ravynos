#  Copyright (C) 2004 Free Software Foundation, Inc.
#
#  Author: Banlu Kemiyatorn <object at gmail dot com>
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

SUBPROJECT_NAME=cairo

# The Objective-C source files to be compiled
cairo_OBJC_FILES = CairoSurface.m \
  CairoFontInfo.m \
  CairoGState.m \
  CairoContext.m \
  CairoFontEnumerator.m \
  CairoFaceInfo.m \
  CairoPSSurface.m \
  CairoPDFSurface.m \
  ../fontconfig/FCFaceInfo.m \
  ../fontconfig/FCFontEnumerator.m \
  ../fontconfig/FCFontInfo.m \


ifeq ($(BUILD_SERVER),x11)
  ifeq ($(WITH_GLITZ),yes)
    cairo_OBJC_FILES += XGCairoGlitzSurface.m
  else
    cairo_OBJC_FILES += XGCairoSurface.m XGCairoXImageSurface.m XGCairoModernSurface.m
  endif
else
  ifeq ($(BUILD_SERVER),wayland)
    cairo_OBJC_FILES += WaylandCairoSurface.m
  else
    ifeq ($(BUILD_GRAPHICS),cairo)
      ifeq ($(WITH_GLITZ),yes)
        cairo_OBJC_FILES += Win32CairoGlitzSurface.m
      else
        cairo_OBJC_FILES += Win32CairoSurface.m Win32CairoGState.m
        # Win32CairoXImageSurface.m 
      endif
    endif
  endif
endif

cairo_OBJC_FILES += 

-include GNUmakefile.preamble

include $(GNUSTEP_MAKEFILES)/subproject.make

-include GNUmakefile.postamble

