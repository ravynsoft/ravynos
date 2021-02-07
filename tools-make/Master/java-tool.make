#   -*-makefile-*-
#   Master/java-tool.make
#
#   Master Makefile rules to build Java command-line tools.
#
#   Copyright (C) 2001 Free Software Foundation, Inc.
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

# Why using Java if you can use Objective-C ...
# Anyway if you really want it, here we go.

#
# The name of the tools is in the JAVA_TOOL_NAME variable.
# The main class (the one implementing main) is in the
# xxx_PRINCIPAL_CLASS variable.
#

JAVA_TOOL_NAME := $(strip $(JAVA_TOOL_NAME))

ifeq ($(RULES_MAKE_LOADED),)
include $(GNUSTEP_MAKEFILES)/rules.make
endif

# TODO: Parallel building.
internal-all:: $(JAVA_TOOL_NAME:=.all.java_tool.variables)

internal-install:: $(JAVA_TOOL_NAME:=.install.java_tool.variables)

internal-uninstall:: $(JAVA_TOOL_NAME:=.uninstall.java_tool.variables)

internal-clean:: $(JAVA_TOOL_NAME:=.clean.java_tool.variables)

internal-distclean::

JAVA_TOOLS_WITH_SUBPROJECTS = $(strip $(foreach java_tool,$(JAVA_TOOL_NAME),$(patsubst %,$(java_tool),$($(java_tool)_SUBPROJECTS))))
ifneq ($(JAVA_TOOLS_WITH_SUBPROJECTS),)
internal-distclean:: $(JAVA_TOOLS_WITH_SUBPROJECTS:=.distclean.java_tool.subprojects)
endif

$(JAVA_TOOL_NAME):
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory $@.all.java_tool.variables$(END_ECHO_RECURSIVE_MAKE)
