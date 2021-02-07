#
#  Top level makefile for GNUstep Backend
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
#  Boston, MA 02110-1301, USA.

ifeq ($(GNUSTEP_MAKEFILES),)
 GNUSTEP_MAKEFILES := $(shell gnustep-config --variable=GNUSTEP_MAKEFILES 2>/dev/null)
endif

ifeq ($(GNUSTEP_MAKEFILES),)
  $(error You need to set GNUSTEP_MAKEFILES before compiling!)
endif

PACKAGE_NAME = gnustep-back
export PACKAGE_NAME
RPM_DISABLE_RELOCATABLE=YES
PACKAGE_NEEDS_CONFIGURE = YES

SVN_MODULE_NAME = back
SVN_BASE_URL = svn+ssh://svn.gna.org/svn/gnustep/libs

GNUSTEP_LOCAL_ADDITIONAL_MAKEFILES=back.make
include $(GNUSTEP_MAKEFILES)/common.make

include ./Version

#
# The list of subproject directories
#
SUBPROJECTS = Source Tools

ifneq ($(fonts), no)
SUBPROJECTS += Fonts
endif

-include GNUmakefile.preamble

include $(GNUSTEP_MAKEFILES)/aggregate.make
include $(GNUSTEP_MAKEFILES)/Master/deb.make

-include GNUmakefile.postamble
