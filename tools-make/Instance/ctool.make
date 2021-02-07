#   -*-makefile-*-
#   Instance/ctool.make
#
#   Instance Makefile rules to build GNUstep-based command line ctools.
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

#
# The name of the ctools is in the CTOOL_NAME variable.
#
# xxx We need to prefix the target name when cross-compiling
#
# This is the same as a tool, but it is not linked against libobjc and
# it is not linked against the foundation library.  This is good if
# you are not using any Objective-C stuff in here.
#
# PS: this means you must leave the variable xxx_OBJC_FILES (and
# xxx_OBJCC_FILES) empty (if you don't, it won't work).  If you need
# to compile Objective-C stuff, please use tool.make.

ifeq ($(RULES_MAKE_LOADED),)
include $(GNUSTEP_MAKEFILES)/rules.make
endif

# This is the directory where the ctools get installed. If you don't
# specify a directory they will get installed in the GNUstep Local
# root.
ifneq ($($(GNUSTEP_INSTANCE)_INSTALL_DIR),)
  CTOOL_INSTALL_DIR = $($(GNUSTEP_INSTANCE)_INSTALL_DIR)
endif

ifeq ($(CTOOL_INSTALL_DIR),)
  CTOOL_INSTALL_DIR = $(GNUSTEP_TOOLS)
endif

.PHONY: internal-ctool-all_ \
        internal-ctool-install_ \
        internal-ctool-uninstall_ \
	internal-ctool-compile

# Override the default with just the minimal C libs required to link
ALL_LIBS =							\
     $(ADDITIONAL_TOOL_LIBS) $(AUXILIARY_TOOL_LIBS)		\
     $(TARGET_SYSTEM_LIBS)

#
# Compilation targets
#
ifeq ($(GNUSTEP_MAKE_PARALLEL_BUILDING), no)
# Standard building
internal-ctool-all_:: $(GNUSTEP_OBJ_INSTANCE_DIR) \
                      $(OBJ_DIRS_TO_CREATE) \
                     $(GNUSTEP_OBJ_DIR)/$(GNUSTEP_INSTANCE)$(EXEEXT)
else
# Parallel building.  The actual compilation is delegated to a
# sub-make invocation where _GNUSTEP_MAKE_PARALLEL is set to yet.
# That sub-make invocation will compile files in parallel.
internal-ctool-all_:: $(GNUSTEP_OBJ_INSTANCE_DIR) $(OBJ_DIRS_TO_CREATE)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory --no-keep-going \
	internal-ctool-compile \
	GNUSTEP_TYPE=$(GNUSTEP_TYPE) \
	GNUSTEP_INSTANCE=$(GNUSTEP_INSTANCE) \
	GNUSTEP_OPERATION=compile \
	GNUSTEP_BUILD_DIR="$(GNUSTEP_BUILD_DIR)" \
	_GNUSTEP_MAKE_PARALLEL=yes$(END_ECHO_RECURSIVE_MAKE)

internal-ctool-compile: $(GNUSTEP_OBJ_DIR)/$(GNUSTEP_INSTANCE)$(EXEEXT)
endif

$(GNUSTEP_OBJ_DIR)/$(GNUSTEP_INSTANCE)$(EXEEXT): $(OBJ_FILES_TO_LINK)
ifeq ($(OBJ_FILES_TO_LINK),)
	$(WARNING_EMPTY_LINKING)
endif
	$(ECHO_LINKING)$(LD) $(ALL_LDFLAGS) -o $(LDOUT)$@ \
	      $(OBJ_FILES_TO_LINK) \
	      $(ALL_LIB_DIRS) $(ALL_LIBS)$(END_ECHO)

internal-ctool-install_:: $(CTOOL_INSTALL_DIR)/$(GNUSTEP_TARGET_DIR)
	$(ECHO_INSTALLING)$(INSTALL_PROGRAM) -m 0755 \
	                   $(GNUSTEP_OBJ_DIR)/$(GNUSTEP_INSTANCE)$(EXEEXT) \
	                   $(CTOOL_INSTALL_DIR)/$(GNUSTEP_TARGET_DIR)$(END_ECHO)

$(CTOOL_INSTALL_DIR)/$(GNUSTEP_TARGET_DIR):
	$(ECHO_CREATING)$(MKINSTALLDIRS) $@$(END_ECHO)

internal-ctool-uninstall_::
	$(ECHO_UNINSTALLING)rm -f $(CTOOL_INSTALL_DIR)/$(GNUSTEP_TARGET_DIR)/$(GNUSTEP_INSTANCE)$(EXEEXT)$(END_ECHO)

include $(GNUSTEP_MAKEFILES)/Instance/Shared/strings.make

