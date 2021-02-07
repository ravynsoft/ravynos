#
#  Makefile for GNUstep base manual
#
#  Copyright (C) 2005 Free Software Foundation, Inc.
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public
#  License as published by the Free Software Foundation; either
#  version 3 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
#  General Public License for more details.
#
#  You should have received a copy of the GNU General Public
#  License along with this library.
#  If not, write to the Free Software Foundation,
#  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

PACKAGE_NAME = gnustep-base

include $(GNUSTEP_MAKEFILES)/common.make
-include ../../config.mak

GNUSTEP_MAKEINFO_FLAGS = -D NO-TEXI2HTML

DOCUMENT_NAME = manual

manual_TEXI_FILES =              \
	foreword.texi            \
	manual.texi              \
	AdvancedMessaging.texi   \
	BaseLibrary.texi         \
	Compliance.texi          \
	DistributedObjects.texi  \
	ExceptionHandling.texi   \
	GNUstepMake.texi         \
	GSDoc.texi               \
	Introduction.texi        \
	ObjcAndJavaC++.texi      \
	ObjcLanguage.texi        \
	JavaAndGuile.texi        \
	WorkingWithObjects.texi  \
	WritingNewClasses.texi

manual_DOC_INSTALL_DIR = Developer/Base/ProgrammingManual

include $(GNUSTEP_MAKEFILES)/documentation.make


