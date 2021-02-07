#   -*-makefile-*-
#   Instance/test-tool.make
#
#   Copyright (C) 2002 Free Software Foundation, Inc.
#
#   Author:  Nicola Pero <nicola@brainstorm.co.uk>
#
#   This file is part of the GNUstep Makefile Package.
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU General Public License
#   as published by the Free Software Foundation; either version 3
#   of the License, or (at your option) any later version.
#   
#   You should have received a copy of the GNU General Public
#   License along with this library; see the file COPYING.
#   If not, write to the Free Software Foundation,
#   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

# Tools don't link against gui by default
ifeq ($(NEEDS_GUI),)
  NEEDS_GUI = no
endif

ifeq ($(RULES_MAKE_LOADED),)
include $(GNUSTEP_MAKEFILES)/rules.make
endif

# Just inherit the build rule from tool.make

include $(GNUSTEP_MAKEFILES)/Instance/tool.make

internal-test_tool-all_:: internal-tool-all_

