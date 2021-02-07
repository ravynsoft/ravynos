#   -*-makefile-*-
#   Instance/tool.make
#
#   Instance Makefile rules to build GNUstep-based command line tools.
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

# Tools don't link against gui by default
ifeq ($(NEEDS_GUI),)
  NEEDS_GUI = no
endif

#
# The name of the tools is in the TOOL_NAME variable.
#
# xxx We need to prefix the target name when cross-compiling
#

ifeq ($(RULES_MAKE_LOADED),)
include $(GNUSTEP_MAKEFILES)/rules.make
endif

.PHONY: internal-tool-all_       \
        internal-tool-install_   \
        internal-tool-uninstall_ \
        internal-tool-copy_into_dir \
	internal-tool-compile

# This is the directory where the tools get installed. If you don't specify a
# directory they will get installed in the GNUstep Local Root.
ifneq ($($(GNUSTEP_INSTANCE)_INSTALL_DIR),)
  TOOL_INSTALL_DIR = $($(GNUSTEP_INSTANCE)_INSTALL_DIR)
endif

ifeq ($(TOOL_INSTALL_DIR),)
  TOOL_INSTALL_DIR = $(GNUSTEP_TOOLS)
endif

# This is the 'final' directory in which we copy the tool executable, including
# the target and library-combo paths.  You can override it in special occasions
# (eg, installing an executable into a web server's cgi dir).
ifeq ($(FINAL_TOOL_INSTALL_DIR),)
  FINAL_TOOL_INSTALL_DIR = $(TOOL_INSTALL_DIR)/$(GNUSTEP_TARGET_LDIR)
endif

#
# Compilation targets
#
ifeq ($(GNUSTEP_MAKE_PARALLEL_BUILDING), no)
# Standard building
internal-tool-all_:: $(GNUSTEP_OBJ_INSTANCE_DIR) \
                     $(OBJ_DIRS_TO_CREATE) \
                     $(GNUSTEP_OBJ_DIR)/$(GNUSTEP_INSTANCE)$(EXEEXT)
else
# Parallel building.  The actual compilation is delegated to a
# sub-make invocation where _GNUSTEP_MAKE_PARALLEL is set to yes.
# That sub-make invocation will compile files in parallel.
internal-tool-all_:: $(GNUSTEP_OBJ_INSTANCE_DIR) $(OBJ_DIRS_TO_CREATE)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory --no-keep-going \
	internal-tool-compile \
	GNUSTEP_TYPE=$(GNUSTEP_TYPE) \
	GNUSTEP_INSTANCE=$(GNUSTEP_INSTANCE) \
	GNUSTEP_OPERATION=compile \
	GNUSTEP_BUILD_DIR="$(GNUSTEP_BUILD_DIR)" \
	_GNUSTEP_MAKE_PARALLEL=yes$(END_ECHO_RECURSIVE_MAKE)

internal-tool-compile: $(GNUSTEP_OBJ_DIR)/$(GNUSTEP_INSTANCE)$(EXEEXT)
endif

$(GNUSTEP_OBJ_DIR)/$(GNUSTEP_INSTANCE)$(EXEEXT): $(OBJ_FILES_TO_LINK)
ifeq ($(OBJ_FILES_TO_LINK),)
	$(WARNING_EMPTY_LINKING)
endif
	$(ECHO_LINKING)$(LD) $(ALL_LDFLAGS) $(CC_LDFLAGS) -o $(LDOUT)$@ \
		$(OBJ_FILES_TO_LINK) \
		$(ALL_LIB_DIRS) $(ALL_LIBS)$(END_ECHO)

internal-tool-copy_into_dir::
	$(ECHO_COPYING_INTO_DIR)$(MKDIRS) $(COPY_INTO_DIR)/$(GNUSTEP_TARGET_LDIR);\
	  $(INSTALL_PROGRAM) -m 0755 \
	  $(GNUSTEP_OBJ_DIR)/$(GNUSTEP_INSTANCE)$(EXEEXT) \
	  $(COPY_INTO_DIR)/$(GNUSTEP_TARGET_LDIR)$(END_ECHO)

# This rule runs $(MKDIRS) only if needed
$(FINAL_TOOL_INSTALL_DIR):
	$(ECHO_CREATING)$(MKINSTALLDIRS) $@$(END_ECHO)

internal-tool-install_:: $(FINAL_TOOL_INSTALL_DIR)
	$(ECHO_INSTALLING)$(INSTALL_PROGRAM) -m 0755 \
		$(GNUSTEP_OBJ_DIR)/$(GNUSTEP_INSTANCE)$(EXEEXT) \
		$(FINAL_TOOL_INSTALL_DIR)$(END_ECHO)

internal-tool-uninstall_::
	rm -f $(FINAL_TOOL_INSTALL_DIR)/$(GNUSTEP_INSTANCE)$(EXEEXT)

# NB: We don't have any cleaning targets for tools here, because we
# clean during the Master make invocation.

#
# If the user makefile contains the command
# xxx_HAS_RESOURCE_BUNDLE = yes
# then we need to build a resource bundle for the tool, and install it.
# You can then add resources to the tool, any sort of, with the usual
# xxx_RESOURCE_FILES, xxx_LOCALIZED_RESOURCE_FILES, xxx_LANGUAGES, etc.
# The tool resource bundle (and all resources inside it) can be
# accessed at runtime very comfortably, by using gnustep-base's
# [NSBundle +mainBundle] (exactly as you would do for an application).
#
ifeq ($($(GNUSTEP_INSTANCE)_HAS_RESOURCE_BUNDLE),yes)

# Include the rules to build resource bundles
GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH = $(GNUSTEP_BUILD_DIR)/Resources/$(GNUSTEP_INSTANCE)

GNUSTEP_SHARED_BUNDLE_INSTALL_NAME = $(GNUSTEP_INSTANCE)
GNUSTEP_SHARED_BUNDLE_INSTALL_LOCAL_PATH = Resources
GNUSTEP_SHARED_BUNDLE_INSTALL_PATH = $(GNUSTEP_LIBRARY)/Tools/Resources
include $(GNUSTEP_MAKEFILES)/Instance/Shared/bundle.make

internal-tool-all_:: shared-instance-bundle-all
internal-tool-copy_into_dir:: shared-instance-bundle-copy_into_dir

$(TOOL_INSTALL_DIR):
	$(ECHO_CREATING)$(MKINSTALLDIRS) $@$(END_ECHO)

$(GNUSTEP_LIBRARY)/Tools/Resources:
	$(ECHO_CREATING)$(MKINSTALLDIRS) $@$(END_ECHO)

internal-tool-install_:: $(GNUSTEP_LIBRARY)/Tools/Resources \
                         shared-instance-bundle-install 

internal-tool-uninstall:: shared-instance-bundle-uninstall

endif

include $(GNUSTEP_MAKEFILES)/Instance/Shared/strings.make
