#
#  src makefile for the GNUstep Base Library
#
#  Copyright (C) 199a-2010 Free Software Foundation, Inc.
#
#  Written by:	Scott Christley <scottc@net-community.com>
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

SUBPROJECT_NAME = Additions

Additions_C_FILES =\
	GSTypeEncoding.c \

Additions_OBJC_FILES =\
	GSObjCRuntime.m \
	GCObject.m \
	GCArray.m \
	GCDictionary.m \
	GSLock.m \
	GSMime.m \
	GSXML.m \
	GSFunctions.m \
	GSInsensitiveDictionary.m \
	NSArray+GNUstepBase.m \
	NSAttributedString+GNUstepBase.m \
	NSBundle+GNUstepBase.m \
	NSCalendarDate+GNUstepBase.m \
	NSData+GNUstepBase.m \
	NSDebug+GNUstepBase.m \
	NSError+GNUstepBase.m \
	NSHashTable+GNUstepBase.m \
	NSFileHandle+GNUstepBase.m \
	NSLock+GNUstepBase.m \
	NSMutableString+GNUstepBase.m \
	NSNumber+GNUstepBase.m \
	NSObject+GNUstepBase.m \
	NSPropertyList+GNUstepBase.m \
	NSProcessInfo+GNUstepBase.m \
	NSStream+GNUstepBase.m \
	NSString+GNUstepBase.m \
	NSTask+GNUstepBase.m \
	NSThread+GNUstepBase.m \
	NSURL+GNUstepBase.m \

Additions_OBJC_FILES += Unicode.m

-include Makefile.preamble

include $(GNUSTEP_MAKEFILES)/subproject.make

-include Makefile.postamble
