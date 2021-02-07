#
#  GSCUPS Printing Backend Bundle makefile for GNUstep GUI Library
#
#  Copyright (C) 2004 Free Software Foundation, Inc.
#
#  Author: Chad Hardin <cehardin@mac.com
#
#  Date: October 2004
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

GNUSTEP_INSTALLATION_DIR=$(GNUSTEP_SYSTEM_ROOT)




BUNDLE_NAME=GSCUPS
BUNDLE_INSTALL_DIR=	$(GNUSTEP_BUNDLES)/GSPrinting
GSCUPS_PRINCIPAL_CLASS=GSCUPSPrincipalClass



ifeq ($(GNUSTEP_TARGET_OS),mingw32)
GSCUPS_BUNDLE_LIBS += -lgnustep-gui $(FND_LIBS) $(OBJC_LIBS)
endif
ifeq ($(GNUSTEP_TARGET_OS),cygwin)
GSCUPS_BUNDLE_LIBS += -lgnustep-gui $(FND_LIBS) $(OBJC_LIBS)
endif


GSCUPS_BUNDLE_LIBS += $(GSCUPS_LIBS)

#
# Libraries
#
#GSCUPS_LIBRARIES_DEPEND_UPON +=


#
# Resource files
#
#GSCUPS_RESOURCE_FILES = 


#
# Header files
#
GSCUPS_HEADERS= \
 GSCUPSPrincipalClass.h \
 GSCUPSPrintInfo.h \
 GSCUPSPrintOperation.h \
 GSCUPSPrinter.h


#
# Class files
#
GSCUPS_OBJC_FILES=	\
 GSCUPSPrincipalClass.m \
 GSCUPSPrintInfo.m \
 GSCUPSPrintOperation.m \
 GSCUPSPrinter.m


#
# C files
#
#GSCUPS_C_FILES = 


#
# Languages
#
#GSCUPS_LANGUAGES= \
# English


#
# Localized Resource Files
#
#GSCUPS_LOCALIZED_RESOURCE_FILES= \
# GSCUPSPrintPanel.gorm \
# GSCUPSPageLayout.gorm







#
# Makefiles
#
-include GNUmakefile.preamble
#include $(GNUSTEP_MAKEFILES)/aggregate.make
include $(GNUSTEP_MAKEFILES)/bundle.make
-include GNUmakefile.postamble
