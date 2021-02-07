#
#  src makefile for the GNUstep Base Library
#
#  Copyright (C) 2010 Free Software Foundation, Inc.
#
#  Written by:	Richard Frith-Macdonald <rfm@gnu.org>
#
#  This file is part of the GNUstep Base Library.
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2 of the License, or (at your option) any later version.
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
#

PACKAGE_NAME = gnustep-base
GNUSTEP_LOCAL_ADDITIONAL_MAKEFILES=../../base.make

include $(GNUSTEP_MAKEFILES)/common.make
include ../../config.mak

SUBPROJECT_NAME = ObjectiveC2

ObjectiveC2_C_FILES =
ObjectiveC2_OBJC_FILES =

ifeq ($(OBJC2RUNTIME),0)

ObjectiveC2_OBJC_FILES += \
	properties.m

ifneq ($(OBJCSYNC), 1)
ObjectiveC2_OBJC_FILES += \
	sync.m
endif

ObjectiveC2_C_FILES += caps.c runtime.c

endif

ifeq ($(HAVE_BLOCKS),0)

ObjectiveC2_OBJC_FILES += \
	blocks_runtime.m\
	NSBlocks.m

endif

ifeq ($(CC), clang)
  ADDITIONAL_OBJCFLAGS = -fblocks
endif

-include Makefile.preamble

include $(GNUSTEP_MAKEFILES)/subproject.make

-include Makefile.postamble

