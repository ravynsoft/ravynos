#
#  Main makefile for GNUstep Backend x11
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

PACKAGE_NAME = gnustep-back
GNUSTEP_LOCAL_ADDITIONAL_MAKEFILES=../../back.make

include $(GNUSTEP_MAKEFILES)/common.make

include ../../config.make

# The library to be compiled, as a library or as a bundle
SUBPROJECT_NAME=x11

# The C source files to be compiled
ifeq ($(WITH_WRASTER),yes)
x11_C_FILES = \
xdnd.c
else
x11_C_FILES = \
context.c \
convert.c \
raster.c \
scale.c \
xdnd.c \
xutil.c
endif

# The Objective-C source files to be compiled
x11_OBJC_FILES = \
XGServer.m \
XGServerEvent.m \
XGServerWindow.m \
XGDragView.m \
XIMInputServer.m \
XWindowBuffer.m\
XGGLFormat.m\
XGGLContext.m

-include GNUmakefile.preamble

include $(GNUSTEP_MAKEFILES)/subproject.make

-include GNUmakefile.postamble

