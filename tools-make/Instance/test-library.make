#   -*-makefile-*-
#   Instance/test-library.make
#
#   Instance Makefile rules for test/non-installed libraries
#
#   Copyright (C) 2005 Free Software Foundation, Inc.
#
#   Author:  Adam Fedor <fedor@gnu.org>
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

ifeq ($(RULES_MAKE_LOADED),)
include $(GNUSTEP_MAKEFILES)/rules.make
endif

# Just inherit the build rule from library.make

include $(GNUSTEP_MAKEFILES)/Instance/library.make

internal-test_library-all_:: internal-library-all_
