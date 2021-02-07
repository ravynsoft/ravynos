#  GNUmakefile
#
#  Copyright (C) 1996 Free Software Foundation, Inc.
#
#  Author: Ovidiu Predescu <ovidiu@net-community.com>
#  Date: November 1997
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
GNUSTEP_LOCAL_ADDITIONAL_MAKEFILES=../gui.make
include $(GNUSTEP_MAKEFILES)/common.make

include ../Version
ifneq ($(GUI_LIB), gnu)
  LIBRARY_NAME = libgmodel
else
  BUNDLE_NAME = libgmodel
endif

ifeq ($(BUILD_NIB2GMODEL), yes)
  TOOL_NAME = nib2gmodel
  ADDITIONAL_TOOL_LIBS += -lgmodel
endif

#APP_NAME = test

libgmodel_OBJC_FILES = IMCustomObject.m IMConnectors.m IMLoading.m GMAppKit.m \
	GMArchiver.m
libgmodel_HEADER_FILES_DIR = ../Headers/Additions/GNUstepGUI
libgmodel_HEADER_FILES_INSTALL_DIR = /GNUstepGUI
libgmodel_HEADER_FILES = IMConnectors.h IMCustomObject.h IMLoading.h \
	GMAppKit.h GMArchiver.h

test_OBJC_FILES = test.m Controller.m
#test_MAIN_MODEL_FILE = test.gmodel
test_RESOURCE_FILES = test.gmodel

nib2gmodel_OBJC_FILES = IBClasses.m Translator.m nib2gmodel.m

# Additional library directories the linker should search
ifneq ($(GUI_LIB), gnu)
  ADDITIONAL_LIB_DIRS += -framework AppKit
endif

# Additional flags to pass to the Objective-C compiler
ADDITIONAL_OBJCFLAGS = -Wall
ADDITIONAL_CPPFLAGS += $(RUNTIME_DEFINE) $(GUI_DEFINE) $(BACKEND_DEFINE)

ADDITIONAL_LIB_DIRS += -L$(GNUSTEP_OBJ_DIR) 
ifeq ($(GUI_LIB), gnu)
  ADDITIONAL_LIB_DIRS += -L../Source/$(GNUSTEP_OBJ_DIR)
  ADDITIONAL_INCLUDE_DIRS += -I../Headers/Additions -I../Headers
else
  ADDITIONAL_INCLUDE_DIRS += -I../Headers/Additions
endif

ADDITIONAL_OBJC_FLAGS += $(BACKEND_DEFINE)

# What are the libraries this library depends upon. This is needed for some
# systems where building a shared library requires to pass to the linker
# all the libraries the target library depends upon.
ifeq ($(GUI_LIB), gnu)
  libgmodel_LIBRARIES_DEPEND_UPON = -lgnustep-gui $(FND_LIBS) $(OBJC_LIBS)
else
  libgmodel_LIBRARIES_DEPEND_UPON = $(FND_LIBS)
endif
libgmodel_BUNDLE_LIBS += $(libgmodel_LIBRARIES_DEPEND_UPON)
nib2gmodel_LIBRARIES_DEPEND_UPON = $(BACKEND_LIBS) $(GUI_LIBS) \
	$(FND_LIBS) $(SYSTEM_LIBS)

ifeq ($(GUI_LIB), gnu)
  include $(GNUSTEP_MAKEFILES)/bundle.make
else
  include $(GNUSTEP_MAKEFILES)/library.make
endif
include $(GNUSTEP_MAKEFILES)/tool.make
include $(GNUSTEP_MAKEFILES)/application.make

include GNUmakefile.postamble
