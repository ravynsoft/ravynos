#
#  Tools level makefile for GNUstep GUI Library
#
#  Copyright (C) 1997 Free Software Foundation, Inc.
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
include ../config.make

include ../Version

SUBPROJECTS = $(BUILD_SPEECH) $(BUILD_SOUND) $(BUILD_SPEECH_RECOGNIZER)
TOOL_NAME = make_services set_show_service gopen gclose gcloseall
SERVICE_NAME = GSspell

# The source files to be compiled
gcloseall_OBJC_FILES = gcloseall.m

gclose_OBJC_FILES = gclose.m

gopen_OBJC_FILES = gopen.m

make_services_OBJC_FILES = make_services.m 

set_show_service_OBJC_FILES = set_show_service.m 

GSspell_OBJC_FILES = GSspell.m

include GNUmakefile.preamble

ifeq ($(CROSS_COMPILING),yes)
  GNUSTEP_MAKE_SERVICES=:
else
  GNUSTEP_MAKE_SERVICES=./$(GNUSTEP_OBJ_DIR)/make_services
endif

include $(GNUSTEP_MAKEFILES)/tool.make
include $(GNUSTEP_MAKEFILES)/service.make
include $(GNUSTEP_MAKEFILES)/aggregate.make

include GNUmakefile.postamble
