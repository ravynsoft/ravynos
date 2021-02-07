#   -*-makefile-*-
#   Instance/subproject.make
#
#   Instance Makefile rules to build subprojects in GNUstep projects.
#
#   Copyright (C) 1998, 2001, 2010 Free Software Foundation, Inc.
#
#   Author:  Jonathan Gapen <jagapen@whitewater.chem.wisc.edu>
#   Author:  Nicola Pero <nicola.pero@meta-innovation.com>
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

.PHONY: internal-subproject-all_       \
        internal-subproject-install_   \
        internal-subproject-uninstall_ \
	internal-subproject-compile

#
# Compilation targets
#
ifeq ($(GNUSTEP_MAKE_PARALLEL_BUILDING), no)
# Standard building
internal-subproject-all_:: $(GNUSTEP_OBJ_INSTANCE_DIR) \
                           $(OBJ_DIRS_TO_CREATE) \
                           $(GNUSTEP_OBJ_DIR)/$(SUBPROJECT_PRODUCT)
else
# Parallel building.  The actual compilation is delegated to a
# sub-make invocation where _GNUSTEP_MAKE_PARALLEL is set to yet.
# That sub-make invocation will compile files in parallel.
internal-subproject-all_:: $(GNUSTEP_OBJ_INSTANCE_DIR) $(OBJ_DIRS_TO_CREATE)
	$(ECHO_NOTHING_RECURSIVE_MAKE)$(MAKE) -f $(MAKEFILE_NAME) --no-print-directory --no-keep-going \
	internal-subproject-compile \
	GNUSTEP_TYPE=$(GNUSTEP_TYPE) \
	GNUSTEP_INSTANCE=$(GNUSTEP_INSTANCE) \
	GNUSTEP_OPERATION=compile \
	GNUSTEP_BUILD_DIR="$(GNUSTEP_BUILD_DIR)" \
	_GNUSTEP_MAKE_PARALLEL=yes$(END_ECHO_RECURSIVE_MAKE)

internal-subproject-compile: $(GNUSTEP_OBJ_DIR)/$(SUBPROJECT_PRODUCT)
endif

# We need to depend on SUBPROJECT_OBJ_FILES to account for sub-subprojects.
$(GNUSTEP_OBJ_DIR)/$(SUBPROJECT_PRODUCT): $(OBJ_FILES_TO_LINK)
ifeq ($(OBJ_FILES_TO_LINK),)
	$(WARNING_EMPTY_LINKING)
endif
	@echo "$(OBJ_FILES_TO_LINK)" > $(GNUSTEP_OBJ_DIR)/$(SUBPROJECT_PRODUCT)

#
# Build-header target for framework subprojects
#
# If we are called with OWNING_PROJECT_HEADER_DIR_NAME which is not empty,
# we need to copy our headers into that directory during the
# build-headers stage, and to disable installation/uninstallation of
# headers.
#
ifneq ($(OWNING_PROJECT_HEADER_DIR_NAME),)
.PHONY: internal-subproject-build-headers

# NB: See headers.make for an explanation of how HEADER_SUBDIRS is
# computed.
OWNING_PROJECT_HEADER_DIR = $(GNUSTEP_BUILD_DIR)/$(OWNING_PROJECT_HEADER_DIR_NAME)
HEADER_FILES   = $($(GNUSTEP_INSTANCE)_HEADER_FILES)
HEADER_SUBDIRS = $(strip $(filter-out ./,$(sort $(dir $(HEADER_FILES)))))
HEADER_FILES_DIR = $($(GNUSTEP_INSTANCE)_HEADER_FILES_DIR)
ifeq ($(HEADER_FILES_DIR),)
  HEADER_FILES_DIR = .
endif
OWNING_PROJECT_HEADER_FILES   = $(addprefix $(OWNING_PROJECT_HEADER_DIR)/,$(HEADER_FILES))
OWNING_PROJECT_HEADER_SUBDIRS = $(addprefix $(OWNING_PROJECT_HEADER_DIR)/,$(HEADER_SUBDIRS))

# We need to build the OWNING_PROJECT_HEADER_DIR directory here
# because this rule could be executed before the top-level framework
# has built his dirs
internal-subproject-build-headers:: $(OWNING_PROJECT_HEADER_DIR) \
                                    $(OWNING_PROJECT_HEADER_SUBDIRS) \
                                    $(OWNING_PROJECT_HEADER_FILES)

$(OWNING_PROJECT_HEADER_DIR):
	$(ECHO_CREATING)$(MKDIRS) $@$(END_ECHO)

$(OWNING_PROJECT_HEADER_SUBDIRS):
	$(ECHO_CREATING)$(MKDIRS) $@$(END_ECHO)

$(OWNING_PROJECT_HEADER_DIR)/%.h: $(HEADER_FILES_DIR)/%.h
	$(ECHO_CREATING)$(INSTALL_DATA) $< $@$(END_ECHO)

# End FRAMEWORK code
else
# Start no FRAMEWORK code

#
# Installation targets - we only need to install headers and only 
# if this is not in a framework
#

include $(GNUSTEP_MAKEFILES)/Instance/Shared/headers.make

internal-subproject-install_:: shared-instance-headers-install

internal-subproject-uninstall_:: shared-instance-headers-uninstall

endif # no FRAMEWORK


#
# A subproject can have resources, which it stores into the
# Resources/Subproject directory.  If you want your subproject
# to have resources, you need to put
# xxx_HAS_RESOURCE_BUNDLE = yes
# in your GNUmakefile.  The project which owns us can then
# copy recursively this directory into its own Resources directory
# (that is done automatically if the project uses
# Instance/Shared/bundle.make to manage its own resource bundle)
#
ifeq ($($(GNUSTEP_INSTANCE)_HAS_RESOURCE_BUNDLE), yes)

GNUSTEP_SHARED_BUNDLE_RESOURCE_PATH = $(GNUSTEP_BUILD_DIR)/Resources/Subproject
include $(GNUSTEP_MAKEFILES)/Instance/Shared/bundle.make

# Only build, not install
internal-subproject-all_:: shared-instance-bundle-all

endif

include $(GNUSTEP_MAKEFILES)/Instance/Shared/strings.make
