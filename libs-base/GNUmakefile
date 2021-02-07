#
#  Main Makefile for GNUstep Base Library.
#  
#  Copyright (C) 1997 Free Software Foundation, Inc.
#
#  Written by:	Scott Christley <scottc@net-community.com>
#
#  This file is part of the GNUstep Base Library.
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public
#  License as published by the Free Software Foundation; either
#  version 2 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
#  General Public License for more details.
#
#  You should have received a copy of the GNU General Public
#  License along with this library; if not, write to the Free
#  Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#  Boston, MA 02111 USA
#

ifeq ($(GNUSTEP_MAKEFILES),)
 GNUSTEP_MAKEFILES := $(shell gnustep-config --variable=GNUSTEP_MAKEFILES 2>/dev/null)
  ifeq ($(GNUSTEP_MAKEFILES),)
    $(warning )
    $(warning Unable to obtain GNUSTEP_MAKEFILES setting from gnustep-config!)
    $(warning Perhaps gnustep-make is not properly installed,)
    $(warning so gnustep-config is not in your PATH.)
    $(warning )
    $(warning Your PATH is currently $(PATH))
    $(warning )
  endif
endif

ifeq ($(GNUSTEP_MAKEFILES),)
  $(error You need to set GNUSTEP_MAKEFILES before compiling!)
endif

GNUSTEP_CORE_SOFTWARE = YES
export GNUSTEP_CORE_SOFTWARE
RPM_DISABLE_RELOCATABLE = YES
PACKAGE_NEEDS_CONFIGURE = YES

PACKAGE_NAME = gnustep-base
export PACKAGE_NAME

SVN_MODULE_NAME = base
SVN_BASE_URL = svn+ssh://svn.gna.org/svn/gnustep/libs

#
# Include local (new) configuration - this will prevent the old one 
# (if any) from $(GNUSTEP_MAKEFILES)/Additional/base.make to be included
#
GNUSTEP_LOCAL_ADDITIONAL_MAKEFILES=base.make
include $(GNUSTEP_MAKEFILES)/common.make

include ./Version
-include config.mak

# Helper variable to check if the generated makefiles are present.  If
# they are not, the tree is clean so prevent make from recursing into
# subprojects when clean/distclean is being invoked again.
_have_makefiles := $(shell test -f config.mak -o -f base.make && echo yes)

#
# The list of subproject directories
#
ifeq ($(_have_makefiles),yes)
SUBPROJECTS = Source
SUBPROJECTS += Tools NSTimeZones Resources Tests
endif

-include Makefile.preamble

include $(GNUSTEP_MAKEFILES)/aggregate.make
-include $(GNUSTEP_MAKEFILES)/Master/deb.make

-include Makefile.postamble
