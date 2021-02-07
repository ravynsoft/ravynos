#
#  Top level makefile for GNUstep GUI Library
#
#  Copyright (C) 1997 Free Software Foundation, Inc.
#
#  Author: Scott Christley <scottc@net-community.com>
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

ifeq ($(GNUSTEP_MAKEFILES),)
 GNUSTEP_MAKEFILES := $(shell gnustep-config --variable=GNUSTEP_MAKEFILES 2>/dev/null)
endif

ifeq ($(GNUSTEP_MAKEFILES),)
  $(error You need to set GNUSTEP_MAKEFILES before compiling!)
endif

PACKAGE_NAME = gnustep-gui
export PACKAGE_NAME
RPM_DISABLE_RELOCATABLE=YES
PACKAGE_NEEDS_CONFIGURE = YES

SVN_MODULE_NAME = gui
SVN_BASE_URL = svn+ssh://svn.gna.org/svn/gnustep/libs


GNUSTEP_LOCAL_ADDITIONAL_MAKEFILES=gui.make
include $(GNUSTEP_MAKEFILES)/common.make

include ./Version

# Don't build docs by default
doc=no

#
# The list of subproject directories
#
SUBPROJECTS = \
Source \
Images \
Sounds \
Model \
Tools \
Panels \
PrinterTypes \
TextConverters \
ColorPickers \
KeyBindings \
Resources \
Printing \
Themes \
Tests

# Build and install sounds, if sound support is present.
SUBPROJECTS += $(BUILD_SOUNDS)

ifeq ($(doc), yes)
SUBPROJECTS += Documentation
endif

-include GNUmakefile.preamble

include $(GNUSTEP_MAKEFILES)/aggregate.make
include $(GNUSTEP_MAKEFILES)/Master/deb.make

include GNUmakefile.postamble
