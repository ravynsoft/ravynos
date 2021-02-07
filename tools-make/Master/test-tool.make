#   -*-makefile-*-
#   Master/test-tool.make
#
#   Copyright (C) 1997, 2001 Free Software Foundation, Inc.
#
#   Author:  Scott Christley <scottc@net-community.com>
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

TEST_TOOL_NAME := $(strip $(TEST_TOOL_NAME))

ifeq ($(RULES_MAKE_LOADED),)
include $(GNUSTEP_MAKEFILES)/rules.make
endif

# Building of test tools works as in tool.make, except we don't install them.

ifeq ($(GNUSTEP_MAKE_PARALLEL_BUILDING), no)

internal-all:: $(GNUSTEP_OBJ_DIR) $(TEST_TOOL_NAME:=.all.test-tool.variables)

else

internal-all:: $(GNUSTEP_OBJ_DIR)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory --no-keep-going \
	internal-master-test-tool-all \
	GNUSTEP_BUILD_DIR="$(GNUSTEP_BUILD_DIR)" \
	_GNUSTEP_MAKE_PARALLEL=yes$(END_ECHO_RECURSIVE_MAKE)

.PHONY: internal-master-test-tool-all

internal-master-test-tool-all: $(TEST_TOOL_NAME:=.all.test-tool.variables)

endif

internal-clean::

internal-distclean::

TEST_TOOLS_WITH_SUBPROJECTS = $(strip $(foreach test-tool,$(TEST_TOOL_NAME),$(patsubst %,$(test-tool),$($(test-tool)_SUBPROJECTS))))
ifneq ($(TEST_TOOLS_WITH_SUBPROJECTS),)
internal-clean:: $(TEST_TOOLS_WITH_SUBPROJECTS:=.clean.test-tool.subprojects)
internal-distclean:: $(TEST_TOOLS_WITH_SUBPROJECTS:=.distclean.test-tool.subprojects)
endif

internal-strings:: $(TEST_TOOL_NAME:=.strings.test-tool.variables)

$(TEST_TOOL_NAME): $(GNUSTEP_OBJ_DIR)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory $@.all.test-tool.variables$(END_ECHO_RECURSIVE_MAKE)

internal-install::
	@ echo Skipping installation of test tools...

internal-uninstall::
	@ echo Skipping uninstallation of test tools...
