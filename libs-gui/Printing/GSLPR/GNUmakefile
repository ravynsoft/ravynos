#
#  GSLPR Printing Backend Bundle makefile for GNUstep GUI Library
#
#  Copyright (C) 2004 Free Software Foundation, Inc.
#
#  Author: Chad Hardin <cehardin@mac.com
#
#  Date: June 2004
#  
#  This file is part of the GNUstep GUI Library.
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

PACKAGE_NAME = gnustep-gui
GNUSTEP_LOCAL_ADDITIONAL_MAKEFILES=../../gui.make

include $(GNUSTEP_MAKEFILES)/common.make
include ../../config.make
include ../../Version

BUNDLE_NAME=GSLPR
BUNDLE_INSTALL_DIR=	$(GNUSTEP_BUNDLES)/GSPrinting
GSLPR_PRINCIPAL_CLASS=GSLPRPrincipalClass

ifeq ($(GNUSTEP_TARGET_OS),mingw32)
GSLPR_BUNDLE_LIBS += -lgnustep-gui $(FND_LIBS) $(OBJC_LIBS)
endif
ifeq ($(GNUSTEP_TARGET_OS),cygwin)
GSLPR_BUNDLE_LIBS += -lgnustep-gui $(FND_LIBS) $(OBJC_LIBS)
endif

#
# Libraries
#
#GSLPR_LIBRARIES_DEPEND_UPON += 

#
# Resource files
#
#GSLPR_RESOURCE_FILES = 

#
# Header files
#
GSLPR_HEADERS= \
 GSLPRPrincipalClass.h \
 GSLPRPrintInfo.h \
 GSLPRPrintOperation.h \
 GSLPRPrinter.h

#
# Class files
#
GSLPR_OBJC_FILES=	\
 GSLPRPrincipalClass.m \
 GSLPRPrintInfo.m \
 GSLPRPrintOperation.m \
 GSLPRPrinter.m

#
# C files
#
#GSLPR_C_FILES = 

#
# Languages
#
#GSLPR_LANGUAGES= \
# English

#
# Localized Resource Files
#
#GSLPR_LOCALIZED_RESOURCE_FILES= \
# GSLPRPrintPanel.gorm \
# GSLPRPageLayout.gorm

#
# Makefiles
#
-include GNUmakefile.preamble
#include $(GNUSTEP_MAKEFILES)/aggregate.make
include $(GNUSTEP_MAKEFILES)/bundle.make
-include GNUmakefile.postamble
